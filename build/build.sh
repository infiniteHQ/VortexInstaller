#!/bin/sh

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

mkdir build_spdlog
mkdir build

cd build_spdlog
cmake ../../lib/spdlog
make -j$(nproc)

cd ../build
cmake ../.. 
make -j$(nproc)

cd ..

mkdir build/dist

cp -r ../ui/installer/assets/resources ./build/bin/resources
cp -r ../ui/installer/assets/builtin ./build/bin/builtin
cp ../misc/linux/installer/icon.png ./build/bin/
cp ../misc/linux/installer/main.py ./build/bin/
cp ../misc/linux/installer/admin_manifest.xml build/bin/
cd build/bin

BUILTIN_FLAG=""
if [[ -d "builtin" ]]; then
    BUILTIN_FLAG="--add-data builtin:./builtin"
fi

MANIFEST_FLAG=""
if [[ -f "manifest.json" ]]; then
    MANIFEST_FLAG="--add-data manifest.json:."
fi


if [ "$NO_INSTALLER" = false ]; then

pyinstaller --onefile --name VortexInstaller --icon=icon.png \
    --add-data "vortex_installer:." \
    --add-data "resources:resources" \
    $BUILTIN_FLAG \
    main.py

rm VortexInstaller.spec
rm icon.png main.py
cd ../..

cp ../misc/linux/updater/icon.png build/bin/
cp ../misc/linux/updater/main.py build/bin/
cp ../misc/linux/updater/org.vortex.updater.policy build/bin/
cd build/bin

pyinstaller --onefile --name VortexUpdater --icon=icon.png \
    --add-data "vortex_update:." \
    --add-data "resources:resources" \
    main.py

rm icon.png main.py org.vortex.updater.policy
cd ../..

cp ../misc/linux/vxuninstaller/icon.png build/bin/
cp ../misc/linux/vxuninstaller/main.py build/bin/
cd build/bin

pyinstaller --onefile --name VersionUninstaller --icon=icon.png \
    --add-data "vxuninstall:." \
    --add-data "resources:resources" \
    main.py

rm icon.png main.py
cd ../..


cp ../misc/linux/uninstaller/icon.png build/bin/
cp ../misc/linux/uninstaller/main.py build/bin/
cd build/bin

pyinstaller --onefile --name VortexUninstaller --icon=icon.png \
    --add-data "vortex_uninstall:." \
    --add-data "resources:resources" \
    main.py

rm icon.png main.py
cd ../..


cp ../misc/linux/vxinstaller/icon.png build/bin/
cp ../misc/linux/vxinstaller/main.py build/bin/
cd build/bin

pyinstaller --onefile --name VersionInstaller --icon=icon.png \
    --add-data "vxinstaller:." \
    --add-data "resources:resources" \
    main.py

rm icon.png main.py
cd ../..
fi


cp build/bin/dist/* build/bin
rm -rf build/bin/build
rm -rf build/bin/dist

mkdir -p shipping/linux

cp build/bin/dist/VortexInstaller shipping/linux/