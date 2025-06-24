import os
import sys
import subprocess
import shlex

def is_admin():
    return os.geteuid() == 0

def main():
    if not is_admin():
        print("This application needs to run as administrator to install the Vortex Launcher.")
        try:
            command = f'pkexec env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY XDG_RUNTIME_DIR=/run/user/$(id -u) {shlex.quote(sys.executable)} {" ".join(map(shlex.quote, sys.argv))}'
            subprocess.run(command, shell=True, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error while executing this as admin: {e}")
            sys.exit(1)
        sys.exit()

    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
        executable_path = sys.executable
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))
        executable_path = os.path.abspath(sys.argv[0])

    exe_path = os.path.join(app_path, "vortex_install")

    try:
        subprocess.run([exe_path, f"--workdir={executable_path}"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Installer: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Exec file not found: {exe_path}")

if __name__ == "__main__":
    main()
