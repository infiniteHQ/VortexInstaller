import os
import sys
import subprocess

def is_admin():
    return os.geteuid() == 0

def main():
    if not is_admin():
        print("This application needs to run as administrator to install the Vortex Launcher.")
        try:
            subprocess.run(f'pkexec env DISPLAY=$DISPLAY XAUTHORITY=$XAUTHORITY XDG_RUNTIME_DIR=/run/user/$(id -u) {sys.executable} {" ".join(sys.argv)}', shell=True, check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error while executing this as admin: {e}")
            sys.exit(1)
        sys.exit()

    # Déterminer le chemin de l'exécutable VortexUpdater
    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS
        executable_path = sys.executable  # Chemin de VortexUpdater (exécutable PyInstaller)
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))
        executable_path = os.path.abspath(sys.argv[0])  # Chemin de VortexUpdater en développement

    exe_path = os.path.join(app_path, "vortex_update")

    try:
        # Formate le paramètre avec --workdir=/chemin/vers/VortexUpdater
        subprocess.run([exe_path, f"--workdir={executable_path}"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Installer: {exe_path}: {e}")
    except FileNotFoundError:
        print(f"Exec file not found: {exe_path}")

if __name__ == "__main__":
    main()
