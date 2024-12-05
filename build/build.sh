#!/bin/sh
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

cp -r ../ui/installer/assets/ressources ./build/bin/ressources
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

pyinstaller --onefile --name VortexInstaller --icon=icon.png \
    --add-data "vortex_installer:." \
    --add-data "ressources:ressources" \
    $BUILTIN_FLAG \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
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
    --add-data "ressources:ressources" \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
    main.py

rm icon.png main.py org.vortex.updater.policy
cd ../..

cp ../misc/linux/vxuninstaller/icon.png build/bin/
cp ../misc/linux/vxuninstaller/main.py build/bin/
cd build/bin

pyinstaller --onefile --name VersionUninstaller --icon=icon.png \
    --add-data "vxuninstall:." \
    --add-data "ressources:ressources" \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
    main.py

rm icon.png main.py
cd ../..


cp ../misc/linux/uninstaller/icon.png build/bin/
cp ../misc/linux/uninstaller/main.py build/bin/
cd build/bin

pyinstaller --onefile --name VortexUninstaller --icon=icon.png \
    --add-data "vortex_uninstaller:." \
    --add-data "ressources:ressources" \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
    main.py

rm icon.png main.py
cd ../..


cp ../misc/linux/vxinstaller/icon.png build/bin/
cp ../misc/linux/vxinstaller/main.py build/bin/
cd build/bin

pyinstaller --onefile --name VersionInstaller --icon=icon.png \
    --add-data "vxinstaller:." \
    --add-data "ressources:ressources" \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
    main.py

rm icon.png main.py
cd ../..

cp build/bin/dist/* build/bin
rm -rf build/bin/build
rm -rf build/bin/dist
rm build/bin/vortex_installer
rm build/bin/vortex_update
rm build/bin/org.vortex.vxinstaller.policy
rm build/bin/VortexUpdater.spec
rm build/bin/VersionInstaller.spec
rm build/bin/VersionUninstaller.spec

mkdir -p shipping/linux

cp build/bin/dist/VortexInstaller shipping/linux/