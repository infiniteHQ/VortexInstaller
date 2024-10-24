import os
import sys
import subprocess

def is_admin():
    return os.geteuid() == 0

def main():
    if not is_admin():
        print("This application needs to run as administrator to install the Vortex Launcher.")
        try:
            subprocess.run(['sudo', sys.executable] + sys.argv, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error while executing this as admin : {e}")
            sys.exit(1)
        sys.exit()

    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))

    exe_path = os.path.join(app_path, "vortex_installer")

    try:
        subprocess.run([exe_path], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Installer: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Exec file not found : {exe_path}")

if __name__ == "__main__":
    main()
