#define SDL_MAIN_HANDLED
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "./ui/app.hpp"

static std::string g_OldLauncherVersion;

void parseArguments(int argc, char *argv[], std::string &path) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg.find("--path=") == 0) {
      path = arg.substr(7);
    }
  }

  if (path.empty()) {
    path = g_InstallerData->g_DefaultInstallPath;
  }
}

int main(int argc, char *argv[]) {
  g_InstallerData = std::make_shared<VortexInstallerData>();
  g_InstallerData->g_WorkingPath = g_InstallerData->g_DefaultInstallPath;

  g_InstallerData->g_Action = "vxuninstall";
  parseArguments(argc, argv, g_InstallerData->g_WorkingPath);

  if (g_InstallerData->g_WorkingPath.empty()) {
    g_InstallerData->g_WorkingPath = VortexInstaller::CookPath("");
    std::cout << "Path derived from executable: " << g_InstallerData->g_WorkingPath << std::endl;
  }

  std::cout << "Action: " << g_InstallerData->g_Action << std::endl;
  std::cout << "Path: " << g_InstallerData->g_WorkingPath << std::endl;

  std::string manifestPath = VortexInstaller::FindManifestJson(g_InstallerData->g_WorkingPath);
  if (!manifestPath.empty()) {
    std::cout << "Found manifest.json at: " << manifestPath << std::endl;

    g_InstallerData->g_WorkingPath = manifestPath.substr(0, manifestPath.find_last_of("/\\"));

  } else {
    std::cerr << "manifest.json not found!" << std::endl;
  }

  CherryRun(argc, argv);

  return 0;
}

#ifdef _WIN32
#include <windows.h>

extern int main(int argc, char *argv[]);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  return main(__argc, __argv);
}
#endif
