import ctypes
import os
import sys
import subprocess

def is_admin():
    """Check if the script is running with administrator privileges."""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def main():
    if not is_admin():
        print("This application needs to run as administrator to update Vortex.")
        try:
            params = " ".join(f'"{arg}"' for arg in sys.argv)
            ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, params, None, 1)
        except Exception as e:
            print(f"Error while trying to elevate privileges: {e}")
        sys.exit()

    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
        executable_path = sys.executable
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))
        executable_path = os.path.abspath(sys.argv[0])

    user_home = os.path.expanduser("~")

    exe_path = os.path.join(app_path, "vortex_uninstall.exe")

    updater_args = [
        exe_path,
        f"--workdir={executable_path}",
        f"--userdir={user_home}"
    ] + sys.argv[1:]

    try:
        subprocess.run(updater_args, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Uninstaller: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Executable file not found: {exe_path}")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main()
