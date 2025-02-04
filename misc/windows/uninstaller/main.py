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
            # Prépare les arguments pour relancer le script avec des droits d'administrateur
            params = " ".join(f'"{arg}"' for arg in sys.argv)
            ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, params, None, 1)
        except Exception as e:
            print(f"Error while trying to elevate privileges: {e}")
        sys.exit()

    # Détermine le chemin de l'application
    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
        executable_path = sys.executable
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))
        executable_path = os.path.abspath(sys.argv[0])

    # Détermine le dossier personnel de l'utilisateur
    user_home = os.path.expanduser("~")

    # Chemin de l'exécutable
    exe_path = os.path.join(app_path, "vortex_uninstall.exe")

    # Prépare les arguments pour l'exécutable
    updater_args = [
        exe_path,
        f"--workdir={executable_path}",
        f"--userdir={user_home}"
    ] + sys.argv[1:]

    try:
        # Lance l'exécutable avec les paramètres
        subprocess.run(updater_args, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Uninstaller: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Executable file not found: {exe_path}")
    except Exception as e:
        print(f"Unexpected error: {e}")

if __name__ == "__main__":
    main()
