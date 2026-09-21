import os
import sys
import subprocess

def main():
    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
        executable_path = sys.executable
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))
        executable_path = os.path.abspath(sys.argv[0])

    exe_path = os.path.join(app_path, "vxinstaller")

    forwarded_args = sys.argv[1:]

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
    main()
