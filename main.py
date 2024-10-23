import os
import subprocess
import sys

def main():
    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))

    exe_path = os.path.join(app_path, "vortex_installer.exe")

    try:
        subprocess.run([exe_path], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing the Vortex Installer :( {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Executable not found: {exe_path}")

if __name__ == "__main__":
    main()
