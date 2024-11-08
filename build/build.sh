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

cp ../misc/linux/installer/icon.png build/bin/
cp ../misc/linux/installer/main.py build/bin/

# TODO : fix the vx installer (dl and decompression), uninstaller, contents managment + modules/templates/plugins clarifications !

cd build/bin
pyinstaller --onefile --name VortexInstaller --icon=icon.png \
    --add-data "vortex_installer:." \
    --add-data "ressources:ressources" \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
    main.py

rm VortexInstaller.spec
rm icon.png main.py
cd ../..

cp ../misc/linux/updater/icon.png build/bin/
cp ../misc/linux/updater/main.py build/bin/
cp ../misc/linux/updater/start_vortex.sh build/bin/
cp ../misc/linux/updater/org.vortex.updater.policy build/bin/
cd build/bin
chmod +x start_vortex.sh

pyinstaller --onefile --name VortexUpdater --icon=icon.png \
    --add-data "vortex_update:." \
    --add-data "start_vortex.sh:." \
    --add-data "ressources:ressources" \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
    main.py

rm icon.png main.py
cd ../..

#cp build/bin/dist/* build/bin
#rm -rf build/bin/build
#rm -rf build/bin/dist
#rm build/bin/vortex_installer
#rm build/bin/vortex_update
#rm build/bin/org.vortex.vxinstaller.policy
#rm build/bin/VortexUpdater.spec
#rm build/bin/VxInstaller.spec

mkdir -p shipping/linux

cp build/bin/dist/VortexInstaller shipping/linux/