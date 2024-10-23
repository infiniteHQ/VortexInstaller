@echo off

mkdir build_spdlog
mkdir build

cd build_spdlog
cmake -G "MinGW Makefiles" ..\..\lib\spdlog
mingw32-make.exe -j%NUMBER_OF_PROCESSORS%

cd ..\build
cmake -G "MinGW Makefiles" ..\..
mingw32-make.exe -j%NUMBER_OF_PROCESSORS%

cd ..

mkdir build\dist
copy ..\misc\windows\icon.png build\bin\
copy ..\misc\windows\main.py build\bin\
copy ..\misc\windows\admin_manifest.xml build\bin\

cd build\bin

pyinstaller --onefile --name VortexInstaller --icon=icon.png --add-data "vortex_installer.exe;." --add-data "ressources;ressources" --add-binary "libglm_shared.dll;." --add-binary "librestclient-cpp.dll;." --add-binary "libspdlog.dll;." --add-binary "SDL2.dll;." --manifest=admin_manifest.xml main.py

cd ..
cd ..

mkdir shipping\windows

copy build\bin\dist\VortexInstaller.exe shipping\windows\

:: TODO : "Publish" the app on the system