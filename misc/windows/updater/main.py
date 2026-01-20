import os
import sys
import subprocess
import ctypes
import tempfile
import shutil
import uuid

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def get_embedded_path(rel_path: str) -> str:
    """
    Return embedded file
    """
    if getattr(sys, "frozen", False):
        base = sys._MEIPASS
    else:
        base = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(base, rel_path)

def launch_updater_embedded(exe_name_in_package, target_install_dir):
    """
   Copy updater
    """
    src_exe = get_embedded_path(exe_name_in_package)
    if not os.path.exists(src_exe):
        raise FileNotFoundError(src_exe)

    tmp_root = tempfile.gettempdir()
    runner_dir = os.path.join(
        tmp_root,
        f"vortex_updater_runner_{uuid.uuid4().hex[:8]}"
    )
    os.makedirs(runner_dir, exist_ok=True)

    runner_exe = os.path.join(runner_dir, "vortex_update.exe")
    shutil.copy2(src_exe, runner_exe)

    src_resources = get_embedded_path("resources")
    dst_resources = os.path.join(runner_dir, "resources")
    if os.path.isdir(src_resources):
        shutil.copytree(src_resources, dst_resources, dirs_exist_ok=True)

    args = [
        runner_exe,
        f"--workdir={target_install_dir}"
    ] + sys.argv[1:]

    DETACHED = 0x00000008
    NEW_PROCESS_GROUP = 0x00000200
    creation_flags = DETACHED | NEW_PROCESS_GROUP

    subprocess.Popen(
        args,
        close_fds=True,
        creationflags=creation_flags,
        cwd=runner_dir
    )

def main():
    if not is_admin():
        params = " ".join(f'"{arg}"' for arg in sys.argv)
        ctypes.windll.shell32.ShellExecuteW(
            None,
            "runas",
            sys.executable,
            params,
            None,
            1
        )
        return

    if getattr(sys, 'frozen', False):
        executable_path = sys.executable
    else:
        executable_path = os.path.abspath(sys.argv[0])

    install_folder = os.path.dirname(executable_path)

    try:
        launch_updater_embedded(
            exe_name_in_package="vortex_update.exe",
            target_install_dir=install_folder
        )
    except Exception as e:
        print("Updater launch error:", e)
        sys.exit(1)

    sys.exit(0)

if __name__ == "__main__":
    main()
