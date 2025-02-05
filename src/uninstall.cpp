#define SDL_MAIN_HANDLED
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include "../ui/installer/app.hpp"

static std::string g_OldLauncherVersion;

void parseArguments(int argc, char *argv[], std::string &action, std::string &path, std::string &userpath) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        // Default
        if (arg.find("--workdir=") == 0)
        {
            path = arg.substr(10);
        }

        // Default
        // Get the vortex version pools from user configs
        if (arg.find("--userdir=") == 0)
        {
            userpath = arg.substr(10);
        }

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
    g_InstallerData = std::make_shared<VortexInstallerData>();
    g_InstallerData->g_WorkingPath = g_InstallerData->g_DefaultInstallPath;

    g_InstallerData->g_Action = "uninstall";
    parseArguments(argc, argv, g_InstallerData->g_Action, g_InstallerData->g_WorkingPath, g_InstallerData->g_HomeDirectory);

    if (g_InstallerData->g_WorkingPath.empty())
    {
        g_InstallerData->g_WorkingPath = CookPath("");
        std::cout << "Path derived from executable: " << g_InstallerData->g_WorkingPath << std::endl;
    }

    std::cout << "Action: " << g_InstallerData->g_Action << std::endl;
    std::cout << "Path: " << g_InstallerData->g_WorkingPath << std::endl;

     std::string manifestPath = findManifestJson(g_InstallerData->g_WorkingPath);
    if (!manifestPath.empty())
    {
        std::cout << "Found manifest.json at: " << manifestPath << std::endl;
    g_InstallerData->g_WorkingPath = manifestPath.substr(0, manifestPath.find_last_of("/\\"));
       
    }
    else
    {
        std::cerr << "manifest.json not found!" << std::endl;
    }

    std::thread mainThread([&]()
                           { Cherry::Main(argc, argv); });

    while (g_ApplicationRunning)
    {
    }

    mainThread.join();

    return 0;
}
