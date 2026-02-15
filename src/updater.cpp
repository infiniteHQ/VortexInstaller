#define SDL_MAIN_HANDLED
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "./ui/app.hpp"

namespace fs = std::filesystem;

void parseArguments(int argc, char *argv[], std::string &action, std::string &path) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg.find("--path=") == 0) {
      VortexInstaller::GetContext()->g_DefaultInstallPath = arg.substr(7);
    }

    if (arg.find("--dist=") == 0) {
      VortexInstaller::GetContext()->g_Distribution = arg.substr(7);
    }

    if (arg.find("--workdir=") == 0) {
      path = arg.substr(10);
    }
  }

  if (path.empty()) {
    std::cout << "No path provided, using default install path: " << path << std::endl;
  }
}

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
std::string normalizeVersion(const std::string &version) {
  std::vector<int> versionParts = SeparateVersion(version);

  std::stringstream ss;
  ss << versionParts[0] << "." << versionParts[1] << "." << versionParts[2];

  return ss.str();
}

int main(int argc, char *argv[]) {
  VortexInstaller::CreateContext();

  VortexInstaller::GetContext()->g_Action = "update";

  parseArguments(argc, argv, VortexInstaller::GetContext()->g_Action, VortexInstaller::GetContext()->g_WorkingPath);

  VortexInstaller::DetectPlatform();
  VortexInstaller::DetectArch();

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

        if (body.empty()) {
          VortexInstaller::GetContext()->g_Request = false;
        } else {
          VortexInstaller::GetContext()->g_Request = true;
          try {
            VortexInstaller::GetContext()->jsonResponse = nlohmann::json::parse(body);

            if (!VortexInstaller::GetContext()->jsonResponse.empty() &&
                VortexInstaller::GetContext()->jsonResponse.is_array()) {
              std::string values_str = VortexInstaller::GetContext()->jsonResponse[0]["values"];

              VortexInstaller::GetContext()->g_RequestValues = nlohmann::json::parse(values_str);

              if (VortexInstaller::GetContext()->g_RequestValues.contains("path") &&
                  VortexInstaller::GetContext()->g_RequestValues["path"].is_string()) {
                VortexInstaller::GetContext()->g_RequestTarballPath = VortexInstaller::GetContext()->g_RequestValues["path"];
                std::cout << "Tarball Path: " << VortexInstaller::GetContext()->g_RequestTarballPath << std::endl;
              } else {
                std::cout << "Error: 'path' key missing or not a string" << std::endl;
              }

              if (VortexInstaller::GetContext()->g_RequestValues.contains("sum") &&
                  VortexInstaller::GetContext()->g_RequestValues["sum"].is_string()) {
                VortexInstaller::GetContext()->g_RequestSumPath = VortexInstaller::GetContext()->g_RequestValues["sum"];
                std::cout << "Sum Path: " << VortexInstaller::GetContext()->g_RequestSumPath << std::endl;
              } else {
                std::cout << "Error: 'sum' key missing or not a string" << std::endl;
              }

              if (VortexInstaller::GetContext()->g_RequestValues.contains("version") &&
                  VortexInstaller::GetContext()->g_RequestValues["version"].is_string()) {
                VortexInstaller::GetContext()->g_RequestVersion = VortexInstaller::GetContext()->g_RequestValues["version"];
                std::cout << "Version: " << VortexInstaller::GetContext()->g_RequestVersion << std::endl;
              } else {
                std::cout << "Error: 'version' key missing or not a string" << std::endl;
              }
            } else {
              std::cout << "Unexpected JSON format or empty response." << std::endl;
            }
          } catch (nlohmann::json::parse_error &e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
          }
        }

        if (VortexInstaller::GetContext()->g_Request) {
          std::cout << body << std::endl;
        }

        std::string manifestPath = VortexInstaller::FindManifestJson(VortexInstaller::GetContext()->g_WorkingPath);
        if (!manifestPath.empty()) {
          std::cout << "Found manifest.json at: " << manifestPath << std::endl;

          VortexInstaller::GetContext()->g_ManifestVersion = VortexInstaller::GetManifestVersion(manifestPath);
          if (!VortexInstaller::GetContext()->g_ManifestVersion.empty()) {
            VortexInstaller::GetContext()->g_ManifestVersion =
                normalizeVersion(VortexInstaller::GetContext()->g_ManifestVersion);
            std::string requestVersion = normalizeVersion(VortexInstaller::GetContext()->g_RequestVersion);

            std::cout << "Manifest version: " << VortexInstaller::GetContext()->g_ManifestVersion << std::endl;
            std::cout << "Requested version: " << requestVersion << std::endl;

            if (requestVersion == VortexInstaller::GetContext()->g_ManifestVersion) {
              VortexInstaller::GetContext()->g_VortexLauncherOutdated = false;
            } else {
              bool isGreater = CompareVersions(VortexInstaller::GetContext()->g_ManifestVersion, requestVersion);
              if (isGreater) {
                VortexInstaller::GetContext()->g_VortexLauncherOutdated = true;
                std::cout << "Requested version is greater than manifest version. VortexLauncher is outdated." << std::endl;
              } else {
                VortexInstaller::GetContext()->g_VortexLauncherOutdated = false;
                std::cout << "Requested version is not greater than manifest version. VortexLauncher is up to date."
                          << std::endl;
              }
            }

          } else {
            std::cerr << "Failed to extract version from manifest.json!" << std::endl;
          }
        } else {
          std::cerr << "manifest.json not found!" << std::endl;
        }

        VortexInstaller::GetContext()->g_NetFetched = true;
      }
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  }).detach();

  if (VortexInstaller::GetContext()->g_WorkingPath.empty()) {
    VortexInstaller::GetContext()->g_WorkingPath = VortexInstaller::CookPath("");
    std::cout << "Path derived from executable: " << VortexInstaller::GetContext()->g_WorkingPath << std::endl;
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
