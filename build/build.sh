#!/bin/bash

NO_INSTALLER=false
while [[ "$#" -gt 0 ]]; do
  case $1 in
    -ni|--no_installer)
      NO_INSTALLER=true
      shift
      ;;
    *)
      echo "Unknown option : $1"
      exit 1
      ;;
  esac
done

python3 -m venv venv_build
source venv_build/bin/activate
pip install --upgrade pip
pip install pyinstaller

mkdir -p build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release ../..
make -j$(nproc)
cd ..

mkdir -p build/dist

cp -r ../src/ui/assets/resources ./build/bin/resources
rm -rf ./build/bin/resources/deps
cp -r ../src/ui/assets/builtin ./build/bin/builtin
cp ../misc/linux/installer/icon.png ./build/bin/
cp ../misc/linux/installer/main.py ./build/bin/
cp ../misc/linux/installer/admin_manifest.xml build/bin/

cd build/bin

BUILTIN_FLAG=""
if [[ -d "builtin" ]]; then
    BUILTIN_FLAG="--add-data builtin:./builtin"
fi

PY_OPTS="--onefile --clean --upx-dir /usr/bin"

if [ "$NO_INSTALLER" = false ]; then

    # VortexInstaller
    pyinstaller $PY_OPTS --name VortexInstaller --icon=icon.png \
        --add-data "vortex_installer:." \
        --add-data "resources:resources" \
        $BUILTIN_FLAG \
        main.py
    rm VortexInstaller.spec icon.png main.py

    # VortexUpdater
    cd ../..
    cp ../misc/linux/updater/icon.png build/bin/
    cp ../misc/linux/updater/main.py build/bin/
    cp ../misc/linux/updater/org.vortex.updater.policy build/bin/
    cd build/bin
    pyinstaller $PY_OPTS --name VortexUpdater --icon=icon.png \
        --add-data "vortex_update:." \
        --add-data "resources:resources" \
        main.py
    rm VortexUpdater.spec icon.png main.py org.vortex.updater.policy

    # VersionUninstaller
    cd ../..
    cp ../misc/linux/vxuninstaller/icon.png build/bin/
    cp ../misc/linux/vxuninstaller/main.py build/bin/
    cd build/bin
    pyinstaller $PY_OPTS --name VersionUninstaller --icon=icon.png \
        --add-data "vxuninstall:." \
        --add-data "resources:resources" \
        main.py
    rm VersionUninstaller.spec icon.png main.py

    # VortexUninstaller
    cd ../..
    cp ../misc/linux/uninstaller/icon.png build/bin/
    cp ../misc/linux/uninstaller/main.py build/bin/
    cd build/bin
    pyinstaller $PY_OPTS --name VortexUninstaller --icon=icon.png \
        --add-data "vortex_uninstall:." \
        --add-data "resources:resources" \
        main.py
    rm VortexUninstaller.spec icon.png main.py

    # VersionInstaller
    cd ../..
    cp ../misc/linux/vxinstaller/icon.png build/bin/
    cp ../misc/linux/vxinstaller/main.py build/bin/
    cd build/bin
    pyinstaller $PY_OPTS --name VersionInstaller --icon=icon.png \
        --add-data "vxinstaller:." \
        --add-data "resources:resources" \
        main.py
    rm VersionInstaller.spec icon.png main.py
    
    cd ../..
fi

cp build/bin/dist/* build/bin
rm -rf build/bin/build
rm -rf build/bin/dist
rm -rf build/bin/resources
rm -rf build/bin/builtin

mkdir -p shipping/linux
cp build/bin/VortexInstaller shipping/linux/

deactivate
rm -rf venv_build
