#define SDL_MAIN_HANDLED
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "./ui/app.hpp"

std::vector<int> SeparateVersion(const std::string &version) {
  std::vector<int> versionParts;
  std::stringstream ss(version);
  std::string part;

  while (std::getline(ss, part, '.')) {
    versionParts.push_back(std::stoi(part));
  }

  while (versionParts.size() < 3) {
    versionParts.push_back(0);
  }

  return versionParts;
}

bool CompareVersions(const std::string &version, const std::string &comparate_version, bool strict = false) {
  std::vector<int> v1 = SeparateVersion(version);
  std::vector<int> v2 = SeparateVersion(comparate_version);

  for (size_t i = 0; i < 3; ++i) {
    if (v2[i] > v1[i]) {
      return true;
    } else if (v2[i] < v1[i]) {
      return false;
    }
  }

  return !strict;
}

void parseArguments(int argc, char *argv[], std::string &action, std::string &path, std::string &home) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg.find("--path=") == 0) {
      path = arg.substr(7);
    }

    if (arg.find("--home=") == 0) {
      home = arg.substr(7);
    }
  }

  if (path.empty()) {
    path = VortexInstaller::GetContext()->g_DefaultInstallPath;
  }
}

int main(int argc, char *argv[]) {
  std::cout << "SDFSD" << std::endl;
  VortexInstaller::CreateContext();

  std::cout << "333" << std::endl;
  VortexInstaller::GetContext()->g_WorkingPath = VortexInstaller::GetContext()->g_DefaultInstallPath;

  std::cout << "555" << std::endl;
  VortexInstaller::GetContext()->g_Action = "install";

  std::cout << "666" << std::endl;
  VortexInstaller::DetectPlatform();
  VortexInstaller::DetectArch();

  std::cout << "345345" << std::endl;
  std::thread([=]() {
    if (VortexInstaller::GetContext()->net.CheckNet()) {
      VortexInstaller::GetContext()->g_Request = true;
    }
  }).detach();

  std::thread([=]() {
    while (!VortexInstaller::GetContext()->g_NetFetched) {
      if (VortexInstaller::GetContext()->g_Request) {
        std::string dist = VortexInstaller::GetContext()->g_Distribution + "_" + VortexInstaller::GetContext()->g_Platform;
        std::string url = "https://api.infinite.si/api/vortexupdates/get_vl_versions?dist=" + dist +
                          "&arch=" + VortexInstaller::GetContext()->g_Arch;

        std::string body = VortexInstaller::GetContext()->net.GET(url);

        try {
          VortexInstaller::GetContext()->jsonResponse = nlohmann::json::parse(body);

          if (!VortexInstaller::GetContext()->jsonResponse.empty() &&
              VortexInstaller::GetContext()->jsonResponse.is_array()) {
            std::string values_str = VortexInstaller::GetContext()->jsonResponse[0]["values"];

            VortexInstaller::GetContext()->g_RequestValues = nlohmann::json::parse(values_str);

            if (VortexInstaller::GetContext()->g_RequestValues.contains("path") &&
                VortexInstaller::GetContext()->g_RequestValues["path"].is_string()) {
              VortexInstaller::GetContext()->g_RequestTarballPath = VortexInstaller::GetContext()->g_RequestValues["path"];
              VXI_LOG("Tarball Path: " << VortexInstaller::GetContext()->g_RequestTarballPath);
            } else {
              VXI_LOG("Error: 'path' key missing or not a string");
            }

            if (VortexInstaller::GetContext()->g_RequestValues.contains("sum") &&
                VortexInstaller::GetContext()->g_RequestValues["sum"].is_string()) {
              VortexInstaller::GetContext()->g_RequestSumPath = VortexInstaller::GetContext()->g_RequestValues["sum"];
              VXI_LOG("Sum Path: " << VortexInstaller::GetContext()->g_RequestSumPath);
            } else {
              VXI_LOG("Error: 'sum' key missing or not a string");
            }

            if (VortexInstaller::GetContext()->g_RequestValues.contains("version") &&
                VortexInstaller::GetContext()->g_RequestValues["version"].is_string()) {
              VortexInstaller::GetContext()->g_RequestVersion = VortexInstaller::GetContext()->g_RequestValues["version"];
              VXI_LOG("Version: " << VortexInstaller::GetContext()->g_RequestVersion);
            } else {
              VXI_LOG("Error: 'version' key missing or not a string");
            }
          } else {
            VXI_LOG("Unexpected JSON format or empty response.");
          }
        } catch (nlohmann::json::parse_error &e) {
          VXI_LOG("JSON Parse Error: " << e.what());
        }

        // Check if the local builtin launcher is equals or higher to the net
        if (VortexInstaller::GetContext()->m_BuiltinLauncherExist)
          if (CompareVersions(
                  VortexInstaller::GetContext()->g_RequestVersion,
                  VortexInstaller::GetContext()->m_BuiltinLauncher.version)) {
            VortexInstaller::GetContext()->g_UseNet = false;
            VortexInstaller::GetContext()->m_BuiltinLauncherNewer = true;
          } else {
            VortexInstaller::GetContext()->m_NetLauncherNewer = true;
          }

        if (VortexInstaller::GetContext()->m_BuiltinLauncher.version == VortexInstaller::GetContext()->g_RequestVersion) {
          VortexInstaller::GetContext()->g_UseNet = false;
          VortexInstaller::GetContext()->m_BuiltinLauncherNewer = true;
        }
        VortexInstaller::GetContext()->g_NetFetched = true;
      }
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  }).detach();

  std::string builtin_manifest = Cherry::GetPath("builtin/manifest.json");

  if (std::filesystem::exists(builtin_manifest)) {
    std::ifstream manifest_file(builtin_manifest);
    nlohmann::json manifest_json;

    try {
      manifest_file >> manifest_json;

      VortexBuiltinLauncher launcher;
      launcher.version = manifest_json.at("version").get<std::string>();
      launcher.arch = manifest_json.at("arch").get<std::string>();
      launcher.platform = manifest_json.at("platform").get<std::string>();
      launcher.tarball = manifest_json.at("tarball").get<std::string>();
      launcher.sum = manifest_json.at("sum").get<std::string>();

      VortexInstaller::GetContext()->m_BuiltinLauncherExist = true;
      VortexInstaller::GetContext()->m_BuiltinLauncher = launcher;

      std::cout << "Manifest loaded successfully!" << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "Error reading manifest: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "Manifest file does not exist!" << std::endl;
  }

  parseArguments(
      argc,
      argv,
      VortexInstaller::GetContext()->g_Action,
      VortexInstaller::GetContext()->g_WorkingPath,
      VortexInstaller::GetContext()->g_HomeDirectory);

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
