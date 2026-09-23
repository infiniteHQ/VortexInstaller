#!/usr/bin/env python3
"""Vortex Installer launcher. On macOS: UI runs as user, backend elevation goes through a pkexec shim."""
import os
import pwd
import shlex
import shutil
import signal
import stat
import subprocess
import sys
import tempfile
import time

UI_NAME = "vxuninstall"

ELEVATION_SHIM = os.environ.get("VORTEX_ELEVATION_SHIM", "1") != "0"

# Optional: launcher starts the backend itself as root (macOS only)
BACKEND_NAME = os.environ.get("VORTEX_BACKEND_NAME") or None
BACKEND_ARGS = shlex.split(os.environ.get("VORTEX_BACKEND_ARGS", ""))
UI_ARGS = shlex.split(os.environ.get("VORTEX_UI_ARGS", ""))
AUTH_TIMEOUT_S = 180


PKEXEC_SHIM = r'''#!/bin/bash
# pkexec replacement for macOS: runs the command as root via the native admin prompt,
# relaying stdin/stdout/stderr.
[ -n "$VORTEX_DEBUG" ] && printf 'pkexec %s\n' "$*" >> "${TMPDIR:-/tmp}/vortex-pkexec.log"

# Background jobs would get /dev/null as stdin, so keep the original one on fd 3
exec 3<&0

while [ $# -gt 0 ]; do
  case "$1" in
    --user) shift 2 ;;
    --) shift; break ;;
    -*) shift ;;
    *) break ;;
  esac
done
if [ $# -eq 0 ]; then
  echo "pkexec: no program specified" >&2
  exit 127
fi

quote() {
  printf "'%s'" "$(printf '%s' "$1" | sed "s/'/'\\\\''/g")"
}

SHIM_PID=$$
PARENT_PID=${VORTEX_PARENT_PID:-$PPID}
UID_N=$(id -u); USER_N=$(id -un); GID_N=$(id -g)

D=$(mktemp -d "${TMPDIR:-/tmp}/vortex-elev.XXXXXX") || exit 1
mkfifo "$D/in" "$D/out" "$D/err" || { rm -rf "$D"; exit 1; }

P1=""; P2=""; P3=""; OSA=""
cleanup() {
  local pids="$P1 $P2 $P3 $OSA"
  [ -n "${pids// /}" ] && kill $pids 2>/dev/null
  rm -rf "$D"
}
trap cleanup EXIT
trap 'exit 143' INT TERM HUP

CMD=""
for a in "$@"; do CMD="$CMD $(quote "$a")"; done

# Root side: kill the command if this shim or the launcher goes away
WATCH='B=$!; while kill -0 '"$SHIM_PID"' 2>/dev/null && kill -0 '"$PARENT_PID"' 2>/dev/null && kill -0 $B 2>/dev/null; do sleep 1; done; if kill -0 $B 2>/dev/null; then kill $B; wait $B; exit 0; fi; wait $B'
INNER="PKEXEC_UID=$UID_N SUDO_USER=$USER_N SUDO_UID=$UID_N SUDO_GID=$GID_N$CMD < $(quote "$D/in") > $(quote "$D/out") 2> $(quote "$D/err") & $WATCH"
ESC=$(printf '%s' "$INNER" | sed 's/\\/\\\\/g; s/"/\\"/g')

cat <&3 > "$D/in" & P1=$!
cat "$D/out" & P2=$!
cat "$D/err" >&2 & P3=$!

osascript -e "do shell script \"$ESC\" with administrator privileges" >"$D/osa.out" 2>"$D/osa.err" &
OSA=$!
wait $OSA
RC=$?

ERR=$(cat "$D/osa.err" 2>/dev/null)
STATUS=$(printf '%s' "$ERR" | sed -n 's/.*(\(-*[0-9][0-9]*\))[[:space:]]*$/\1/p' | tail -n 1)

RAN=0
if [ "$RC" -eq 0 ]; then
  RAN=1; EXIT=0
elif [ -n "$STATUS" ] && [ "$STATUS" -ge 1 ] 2>/dev/null && [ "$STATUS" -le 255 ]; then
  RAN=1; EXIT=$STATUS
elif [ "$STATUS" = "-128" ]; then
  EXIT=126
else
  EXIT=127
  [ -n "$ERR" ] && echo "pkexec: $ERR" >&2
fi

if [ "$RAN" -eq 1 ]; then
  i=0
  while [ $i -lt 100 ] && { kill -0 $P2 2>/dev/null || kill -0 $P3 2>/dev/null; }; do
    sleep 0.1; i=$((i+1))
  done
fi
exit $EXIT
'''


def install_elevation_shim() -> str:
    shim_dir = tempfile.mkdtemp(prefix="vortex-shim-")
    path = os.path.join(shim_dir, "pkexec")
    with open(path, "w") as f:
        f.write(PKEXEC_SHIM)
    os.chmod(path, 0o755)
    return shim_dir


def applescript_escape(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')


def show_error(message: str) -> None:
    print(message, file=sys.stderr)
    if sys.platform != "darwin":
        return
    script = (
        f'display dialog "{applescript_escape(message)}" '
        f'with title "Vortex Installer" buttons {{"OK"}} default button "OK" '
        f"with icon stop"
    )
    try:
        subprocess.run(["/usr/bin/osascript", "-e", script], check=False)
    except Exception:
        pass


def ensure_executable(path: str) -> None:
    try:
        mode = os.stat(path).st_mode
        if not mode & stat.S_IXUSR:
            os.chmod(path, mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
    except OSError:
        pass


def missing_file_message(path: str, app_path: str) -> str:
    try:
        names = sorted(
            n for n in os.listdir(app_path)
            if os.path.isfile(os.path.join(app_path, n))
            and os.access(os.path.join(app_path, n), os.X_OK)
            and not n.endswith((".dylib", ".so", ".zip", ".pyc", ".py"))
        )[:15]
    except OSError:
        names = []
    msg = f"Exec file not found: {path}"
    if names:
        msg += "\n\nExecutables in bundle:\n  - " + "\n  - ".join(names)
    return msg


def is_auth_cancelled(stderr: str) -> bool:
    return "-128" in stderr or "User canceled" in stderr


def create_ipc(executable_path: str):
    ipc_dir = tempfile.mkdtemp(prefix="vortex-ipc-")
    to_backend = os.path.join(ipc_dir, "ui_to_backend.fifo")
    from_backend = os.path.join(ipc_dir, "backend_to_ui.fifo")
    os.mkfifo(to_backend, 0o600)
    os.mkfifo(from_backend, 0o600)
    ready_file = os.path.join(ipc_dir, "backend.started")
    ipc_env = {
        "VORTEX_PIPE_TO_BACKEND": to_backend,
        "VORTEX_PIPE_FROM_BACKEND": from_backend,
        "VORTEX_WORKDIR": executable_path,
    }
    return ipc_dir, ipc_env, ready_file


def start_backend_as_root(backend_cmd: list, env_pairs: list, ready_file: str):
    if os.geteuid() == 0:
        env = dict(os.environ)
        env.update(p.split("=", 1) for p in env_pairs)
        open(ready_file, "w").close()
        return subprocess.Popen(backend_cmd, env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, text=True)

    payload = shlex.join(["/usr/bin/env", *env_pairs, *backend_cmd])
    inner = (
        f"touch {shlex.quote(ready_file)}; "
        f"{payload} & B=$!; "
        f"while kill -0 {os.getpid()} 2>/dev/null && kill -0 $B 2>/dev/null; do sleep 1; done; "
        f"if kill -0 $B 2>/dev/null; then kill $B; wait $B; exit 0; fi; "
        f"wait $B"
    )
    script = f'do shell script "{applescript_escape(inner)}" with administrator privileges'
    return subprocess.Popen(
        ["/usr/bin/osascript", "-e", script],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
    )


def run_macos(app_path: str, executable_path: str, exe_path: str, forwarded_args: list) -> int:
    if not os.path.isfile(exe_path):
        show_error(missing_file_message(exe_path, app_path))
        return 1
    ensure_executable(exe_path)

    ui_cmd = [exe_path, f"--workdir={executable_path}", *UI_ARGS, *forwarded_args]
    ui_env = dict(os.environ)
    ui_env["VORTEX_PARENT_PID"] = str(os.getpid())

    signal.signal(signal.SIGTERM, lambda *_: sys.exit(143))
    signal.signal(signal.SIGHUP, lambda *_: sys.exit(129))

    shim_dir = ipc_dir = backend = ui = None
    ui_killed_by_us = False
    try:
        if ELEVATION_SHIM:
            shim_dir = install_elevation_shim()
            ui_env["PATH"] = shim_dir + os.pathsep + ui_env.get("PATH", "/usr/bin:/bin:/usr/sbin:/sbin")

        if BACKEND_NAME:
            backend_path = os.path.join(app_path, BACKEND_NAME)
            if not os.path.isfile(backend_path):
                show_error(missing_file_message(backend_path, app_path))
                return 1
            ensure_executable(backend_path)

            ipc_dir, ipc_env, ready_file = create_ipc(executable_path)
            user = pwd.getpwuid(os.getuid())
            root_env = [
                f"SUDO_USER={user.pw_name}",
                f"SUDO_UID={user.pw_uid}",
                f"SUDO_GID={user.pw_gid}",
                f"VORTEX_PARENT_PID={os.getpid()}",
                *[f"{k}={v}" for k, v in ipc_env.items()],
            ]
            backend = start_backend_as_root([backend_path, *BACKEND_ARGS], root_env, ready_file)

            deadline = time.time() + AUTH_TIMEOUT_S
            while not os.path.exists(ready_file):
                rc = backend.poll()
                if rc is not None:
                    _, err = backend.communicate()
                    err = (err or "").strip()
                    if is_auth_cancelled(err):
                        print("Authentication cancelled.", file=sys.stderr)
                        return 130
                    show_error(f"Backend failed to start: {backend_path}\n\n{err or rc}")
                    return rc or 1
                if time.time() > deadline:
                    show_error("Authentication timed out.")
                    return 1
                time.sleep(0.1)
            ui_env.update(ipc_env)

        try:
            ui = subprocess.Popen(ui_cmd, env=ui_env)
        except FileNotFoundError:
            show_error(f"Exec file not found: {exe_path}")
            return 1
        except OSError as e:
            show_error(f"Error while executing Vortex Installer: {exe_path}: {e}")
            return 1

        backend_reported = False
        while ui.poll() is None:
            if backend is not None and not backend_reported:
                rc = backend.poll()
                if rc is not None:
                    backend_reported = True
                    _, err = backend.communicate()
                    if rc != 0:
                        show_error(f"Backend exited with code {rc}.\n\n{(err or '').strip()}")
                        ui_killed_by_us = True
                        ui.terminate()
                        break
            time.sleep(0.2)

        try:
            ui.wait(timeout=5)
        except subprocess.TimeoutExpired:
            ui.kill()

        if ui.returncode != 0 and not ui_killed_by_us:
            show_error(f"Error while executing Vortex Installer: {exe_path}: exit code {ui.returncode}")
        return ui.returncode or 0

    except KeyboardInterrupt:
        return 130

    finally:
        if ui is not None and ui.poll() is None:
            ui.terminate()
        if backend is not None and backend.poll() is None:
            try:
                backend.wait(timeout=10)
            except subprocess.TimeoutExpired:
                backend.terminate()
        for d in (ipc_dir, shim_dir):
            if d:
                shutil.rmtree(d, ignore_errors=True)


def main():
    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
        executable_path = sys.executable
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))
        executable_path = os.path.abspath(sys.argv[0])

    exe_path = os.path.join(app_path, UI_NAME)

    forwarded_args = sys.argv[1:]

    if sys.platform == "darwin":
        return run_macos(app_path, executable_path, exe_path, forwarded_args)

    vxinstaller_cmd = [
        exe_path,
        f"--workdir={executable_path}",
        *forwarded_args
    ]

    try:
        subprocess.run(vxinstaller_cmd, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Installer: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Exec file not found: {exe_path}")


if __name__ == "__main__":
    sys.exit(main() or 0)