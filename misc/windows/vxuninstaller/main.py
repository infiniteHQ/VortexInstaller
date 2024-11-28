import ctypes
import os
import sys
import subprocess

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def main():
    if not is_admin():
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, __file__, None, 1)
        sys.exit()

    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))

    exe_path = os.path.join(app_path, "vxuninstaller.exe")

    try:
        subprocess.run([exe_path], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing the Vortex Version Uninstaller: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Executable not found: {exe_path}")

if __name__ == "__main__":
    main()
