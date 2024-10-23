#!/bin/sh
mkdir build_spdlog
mkdir build

cd build_spdlog && cmake ../../lib/spdlog && make -j$(nproc) install
cd ../build && cmake ../.. && make -j$(nproc) install
cd ..

# TODO : Make dist and package the installer with pyinstaller
# Cpy into shipping/linux

sudo chown root:root ./build/bin/vortex_installer
sudo chown root:root ./build/bin/vortex_uninstall
sudo chown root:root ./build/bin/vortex_update

sudo chmod u+s ./build/bin/vortex_installer
sudo chmod u+s ./build/bin/vortex_uninstall
sudo chmod u+s ./build/bin/vortex_update