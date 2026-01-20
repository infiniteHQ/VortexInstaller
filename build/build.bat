@echo off
setlocal

mkdir build_spdlog
mkdir build

cd build
cmake -G "Visual Studio 17" -A x64 ..\..

for /f %%i in ('powershell -command "(Get-WmiObject -Class Win32_Processor).NumberOfLogicalProcessors"') do set THREADS=%%i

cmake --build . --config Release -- /m:%THREADS%

xcopy /Y /E /I .\bin\Release\* .\bin

rmdir /S /Q .\bin\Release

cd ..
mkdir build\dist

xcopy /E /I /Y ..\ui\installer\assets\resources .\build\bin\resources
xcopy /E /I /Y ..\ui\installer\assets\resources .\build\bin\resources_nodeps
rmdir /S /Q .\build\bin\resources_nodeps\deps

xcopy /E /I /Y ..\ui\installer\assets\builtin .\build\bin\builtin
copy ..\misc\windows\installer\icon.png .\build\bin\
copy ..\misc\windows\installer\main.py .\build\bin\
copy ..\misc\windows\installer\admin_manifest.xml build\bin\
cd build\bin

set BUILTIN_FLAG=
if exist builtin (
    set BUILTIN_FLAG=--add-data "builtin;builtin"
)

set MANIFEST_FLAG=
if exist manifest.json (
    set MANIFEST_FLAG=--add-data "manifest.json;."
)

call pyinstaller --noconsole --onefile --name VortexInstaller --icon=icon.png ^
    --add-data "vortex_installer.exe;." ^
    --add-data "resources;resources" ^
    %BUILTIN_FLAG% ^
    --manifest=admin_manifest.xml ^
    main.py

del VortexInstaller.spec
del icon.png main.py
cd ..\..

copy ..\misc\windows\updater\icon.png build\bin\
copy ..\misc\windows\updater\main.py build\bin\
copy ..\misc\windows\updater\admin_manifest.xml build\bin\
cd build\bin

call pyinstaller --onefile --name VortexUpdater --icon=icon.png ^
    --add-data "vortex_update.exe;." ^
    --add-data "resources_nodeps;resources" ^
    %MANIFEST_FLAG% ^
    --manifest=admin_manifest.xml ^
    main.py

del icon.png main.py org.vortex.updater.policy
cd ..\..

copy ..\misc\windows\vxuninstaller\icon.png build\bin\
copy ..\misc\windows\vxuninstaller\main.py build\bin\
copy ..\misc\windows\vxuninstaller\admin_manifest.xml build\bin\
cd build\bin

call pyinstaller --onefile --name VersionUninstaller --icon=icon.png ^
    --add-data "vxuninstall.exe;." ^
    --add-data "resources_nodeps;resources" ^
    %MANIFEST_FLAG% ^
    --manifest=admin_manifest.xml ^
    main.py

del icon.png main.py
cd ..\..

copy ..\misc\windows\uninstaller\icon.png build\bin\
copy ..\misc\windows\uninstaller\main.py build\bin\
copy ..\misc\windows\uninstaller\admin_manifest.xml build\bin\
cd build\bin

call pyinstaller --onefile --name VortexUninstaller --icon=icon.png ^
    --add-data "vortex_uninstall.exe;." ^
    --add-data "resources_nodeps;resources" ^
    %MANIFEST_FLAG% ^
    --manifest=admin_manifest.xml ^
    main.py

del icon.png main.py
cd ..\..

copy ..\misc\windows\vxinstaller\icon.png build\bin\
copy ..\misc\windows\vxinstaller\main.py build\bin\
copy ..\misc\windows\vxinstaller\admin_manifest.xml build\bin\
cd build\bin

call pyinstaller --onefile --name VersionInstaller --icon=icon.png ^
    --add-data "vxinstaller.exe;." ^
    --add-data "resources_nodeps;resources" ^
    %MANIFEST_FLAG% ^
    --manifest=admin_manifest.xml ^
    main.py

del icon.png main.py
cd ..\..

copy build\bin\dist\* build\bin
rd /s /q build\bin\build
rd /s /q build\bin\dist
rd /s /q build\bin\resources
rd /s /q build\bin\resources_nodeps
del build\bin\vortex_installer
del build\bin\vortex_update
del build\bin\org.vortex.vxinstaller.policy
del build\bin\VortexUpdater.spec
del build\bin\VersionInstaller.spec
del build\bin\VersionUninstaller.spec

mkdir shipping\linux
copy build\bin\dist\VortexInstaller shipping\windows\

endlocal