#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include "../ui/installer/installer.hpp"

static std::string g_OldLauncherVersion;

void parseArguments(int argc, char *argv[], std::string &action, std::string &path) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.find("--path=") == 0) {
            path = arg.substr(7);
        }
        
        if (arg.find("--oldv=") == 0) {
            g_OldLauncherVersion = arg.substr(7);
        }
    }

    if (path.empty()) {
        path = g_InstallerData->g_DefaultInstallPath;
    }
}

int main(int argc, char *argv[])
{
    g_InstallerData->g_WorkingPath = g_InstallerData->g_DefaultInstallPath;

            g_InstallerData->g_Action = "update";
    parseArguments(argc, argv, g_InstallerData->g_Action, g_InstallerData->g_WorkingPath);

    std::cout << "Action: " << g_InstallerData->g_Action << std::endl;
    std::cout << "Path: " << g_InstallerData->g_WorkingPath << std::endl;

    std::thread mainThread([&]()
                           { Cherry::Main(argc, argv); });

    while (g_ApplicationRunning)
    {
    }

    mainThread.join();

    return 0;
}
