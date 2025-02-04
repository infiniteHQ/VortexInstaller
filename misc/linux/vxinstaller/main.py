import os
import sys
import subprocess

def is_admin():
    return os.geteuid() == 0

def main():
    if not is_admin():
        print("This application needs to run as administrator to install a version of Vortex.")
        try:
            subprocess.run(f'pkexec env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY XDG_RUNTIME_DIR=/run/user/$(id -u) {sys.executable} {" ".join(sys.argv)}', shell=True, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error while executing this as admin: {e}")
            sys.exit(1)
        sys.exit()

    # Determine the path to the `vxinstaller` executable
    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))

    exe_path = os.path.join(app_path, "vxinstaller")

    # Prepare to forward the arguments
    vxinstaller_args = [exe_path] + sys.argv[1:]  # Append all arguments passed to the Python script

    try:
        # Run `vxinstaller` with all arguments passed to this Python script
        subprocess.run(vxinstaller_args, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Installer: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Executable file not found: {exe_path}")

if __name__ == "__main__":
    main()
