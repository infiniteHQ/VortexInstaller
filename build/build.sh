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

rm icon.png main.py
cd ../..

cp ../misc/linux/vxinstaller/icon.png build/bin/
cp ../misc/linux/vxinstaller/main.py build/bin/
cp ../misc/linux/vxinstaller/org.vortex.vxinstaller.policy build/bin/
cd build/bin
pyinstaller --onefile --name VxInstaller --icon=icon.png \
    --add-data "vxinstaller:." \
    --add-data "ressources:ressources" --add-data "org.vortex.vxinstaller.policy:." \
    --add-binary "../ui_installer_build/cherry_build/lib/glm/glm/libglm_shared.so:." \
    --add-binary "../restcpp_build/librestclient-cpp.so:." \
    --add-binary "../ui_installer_build/cherry_build/lib/spdlog/libspdlog.so:." \
    main.py

rm icon.png main.py
cd ../..

mkdir -p shipping/linux

cp build/bin/dist/VortexInstaller shipping/linux/