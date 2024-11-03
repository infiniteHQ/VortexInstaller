import os
import sys
import subprocess
import shutil

# Définir XDG_RUNTIME_DIR
if 'XDG_RUNTIME_DIR' not in os.environ:
    os.environ['XDG_RUNTIME_DIR'] = '/run/user/' + str(os.getuid())

def copy_policy_file():
    # Chemin du fichier .policy local et emplacement cible
    policy_src = os.path.join(os.path.dirname(os.path.abspath(__file__)), "org.vortex.vxinstaller.policy")
    policy_dest = "/usr/share/polkit-1/actions/org.vortex.vxinstaller.policy"
    
    if not os.path.exists(policy_dest):
        try:
            # Utilisation de pkexec pour copier le fichier avec les privilèges nécessaires
            print(f"Attempting to copy policy file to {policy_dest} with elevated privileges.")
            subprocess.run(['pkexec', 'cp', policy_src, policy_dest], check=True)
            print(f"Policy file successfully copied to {policy_dest}")
        except subprocess.CalledProcessError as e:
            print(f"Failed to copy policy file: {e}")
            sys.exit(1)

def run_installer_with_privileges():
    # Chemin vers l'exécutable de l'installateur
    app_path = os.path.dirname(os.path.abspath(__file__))
    exe_path = os.path.join(app_path, "vxinstaller")
    
    if not os.path.isfile(exe_path):
        print(f"Executable not found: {exe_path}")
        sys.exit(1)

    try:
        # Exécuter vxinstaller avec pkexec pour obtenir les privilèges nécessaires
        subprocess.run(['pkexec', exe_path] + sys.argv[1:], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error while executing Vortex Installer with admin privileges: {e}")
        sys.exit(1)

def main():
    # Copier le fichier policy pour autoriser vxinstaller à s'exécuter avec les privilèges
    copy_policy_file()
    
    # Lancer vxinstaller avec les privilèges nécessaires tout en restant sous l'utilisateur actuel
    run_installer_with_privileges()

if __name__ == "__main__":
    main()
