import ctypes
import os
import sys
import subprocess
import tempfile

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def main():
    if not is_admin():
        params = " ".join(f'"{arg}"' for arg in sys.argv[1:])
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, params, None, 1)
        sys.exit(0)

    install_dir = None
    for a in sys.argv[1:]:
        if a.startswith("--path="):
            install_dir = a.split("=", 1)[1]
            break


    if not install_dir:
        print("Install dir unknown -> fallback: ask your app to pass --path=... or derive it from a stable source")

    if getattr(sys, 'frozen', False):
        app_path = sys._MEIPASS 
    else:
        app_path = os.path.dirname(os.path.abspath(__file__))

    exe_path = os.path.join(app_path, "vortex_uninstall.exe")

    args = [exe_path]
    if install_dir:
        args.append(f'--path={install_dir}')
    safe_cwd = tempfile.gettempdir()
    try:
        subprocess.run(args, check=True, cwd=safe_cwd)
    except Exception as e:
        print(f"Uninstaller failed: {e}")

if __name__ == "__main__":
    main()
