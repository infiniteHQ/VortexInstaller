#define SDL_MAIN_HANDLED
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "./ui/app.hpp"

static std::string g_OldLauncherVersion;

void parseArguments(int argc, char *argv[], std::string &action, std::string &path, std::string &userpath) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    // Default
    if (arg.find("--workdir=") == 0) {
      path = arg.substr(10);
    }

    // Default
    // Get the vortex version pools from user configs
    if (arg.find("--userdir=") == 0) {
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
    path = VortexInstaller::GetContext()->g_DefaultInstallPath;
  }
}

int main(int argc, char *argv[]) {
  VortexInstaller::CreateContext();
  VortexInstaller::GetContext()->g_WorkingPath = VortexInstaller::GetContext()->g_DefaultInstallPath;

  VortexInstaller::GetContext()->g_Action = "uninstall";
  parseArguments(
      argc,
      argv,
      VortexInstaller::GetContext()->g_Action,
      VortexInstaller::GetContext()->g_WorkingPath,
      VortexInstaller::GetContext()->g_HomeDirectory);

  if (VortexInstaller::GetContext()->g_WorkingPath.empty()) {
    // TODO Clarify and secure this.
    VortexInstaller::GetContext()->g_WorkingPath = VortexInstaller::CookPath("");
    std::cout << "Path derived from executable: " << VortexInstaller::GetContext()->g_WorkingPath << std::endl;
  }

  std::cout << "Action: " << VortexInstaller::GetContext()->g_Action << std::endl;
  std::cout << "Path: " << VortexInstaller::GetContext()->g_WorkingPath << std::endl;

  std::string manifestPath = VortexInstaller::FindManifestJson(VortexInstaller::GetContext()->g_WorkingPath);
  if (!manifestPath.empty()) {
    std::cout << "Found manifest.json at: " << manifestPath << std::endl;
    VortexInstaller::GetContext()->g_WorkingPath = manifestPath.substr(0, manifestPath.find_last_of("/\\"));

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
