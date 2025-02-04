import os
import sys
import subprocess
import ctypes

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def main():
    if not is_admin():
        print("This application needs to run as administrator to install the Vortex Launcher.")
        try:
            params = " ".join(f'"{arg}"' for arg in sys.argv)
            subprocess.run(
                ["powershell", "-Command", f"Start-Process '{sys.executable}' -ArgumentList '{params}' -Verb RunAs"],
                check=True
            )
        except subprocess.CalledProcessError as e:
            print(f"Error while executing this as admin: {e}")
            sys.exit(1)
        except Exception as e:
            print(f"Unexpected error: {e}")
            sys.exit(1)
        sys.exit()

    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
        executable_path = sys.executable
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))
        executable_path = os.path.abspath(sys.argv[0])

    exe_path = os.path.join(app_path, "vortex_update.exe")

    try:
        subprocess.run([exe_path, f"--workdir={executable_path}"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Installer: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Exec file not found: {exe_path}")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main()
