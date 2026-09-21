#define SDL_MAIN_HANDLED
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "./ui/app.hpp"

#ifdef __APPLE__
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <sstream>

#define VXI_MAC_LOG(x) (std::cerr << "[vortex-mac] " << x << std::endl)
#endif

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
  VortexInstaller::CreateContext();

  VortexInstaller::GetContext()->g_WorkingPath = VortexInstaller::GetContext()->g_DefaultInstallPath;
  VortexInstaller::GetContext()->g_Action = "install";

  VortexInstaller::DetectPlatform();
  VortexInstaller::DetectArch();

#ifdef __APPLE__
  {
    std::shared_ptr<VortexInstallerData> ctx = VortexInstaller::GetContext();
    if (const char *v = std::getenv("VORTEX_PLATFORM")) {
      ctx->g_Platform = v;
    } else if (ctx->g_Platform.empty() || ctx->g_Platform == "linux") {
      ctx->g_Platform = "macos";
    }

    if (const char *v = std::getenv("VORTEX_ARCH")) {
      ctx->g_Arch = v;
    } else if (ctx->g_Arch.empty()) {
#if defined(__aarch64__) || defined(__arm64__)
      ctx->g_Arch = "arm64";
#else
      ctx->g_Arch = "x86_64";
#endif
    }
    VXI_MAC_LOG("platform=" << ctx->g_Platform << " arch=" << ctx->g_Arch << " distribution='" << ctx->g_Distribution << "'");
  }
#endif

  std::thread([=]() {
    if (VortexInstaller::GetContext()->net.CheckNet()) {
      VortexInstaller::GetContext()->g_Request = true;
    }
#ifdef __APPLE__
    else {
      VXI_MAC_LOG("CheckNet() failed: no network detected");
    }
#endif
  }).detach();

  std::thread([=]() {
#ifdef __APPLE__
    int mac_attempts = 0;
#endif
    while (!VortexInstaller::GetContext()->g_NetFetched) {
      if (VortexInstaller::GetContext()->g_Request) {
        std::string dist = VortexInstaller::GetContext()->g_Distribution + "_" + VortexInstaller::GetContext()->g_Platform;
#ifdef __APPLE__
        if (const char *d = std::getenv("VORTEX_DIST")) {
          dist = d;
        } else if (VortexInstaller::GetContext()->g_Distribution.empty()) {
          dist = VortexInstaller::GetContext()->g_Platform;
        }
#endif
        std::string url = "https://api.infinite.si/api/vortexupdates/get_vl_versions?dist=" + dist +
                          "&arch=" + VortexInstaller::GetContext()->g_Arch;

#ifdef __APPLE__
        VXI_MAC_LOG("calling net.GET(" << url << ") attempt " << (mac_attempts + 1));
        auto mac_t0 = std::chrono::steady_clock::now();
#endif
        std::string body = VortexInstaller::GetContext()->net.GET(url);

#ifdef __APPLE__
        VXI_MAC_LOG(
            "net.GET returned " << body.size() << " bytes in "
                                << std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::steady_clock::now() - mac_t0)
                                       .count()
                                << " ms: " << body.substr(0, 300));
        if (body.empty() && ++mac_attempts < 3) {
          std::this_thread::sleep_for(std::chrono::seconds(2));
          continue;
        }
#endif

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
#ifdef __APPLE__
        VXI_MAC_LOG(
            "parsed: tarball='" << VortexInstaller::GetContext()->g_RequestTarballPath << "' sum='"
                                << VortexInstaller::GetContext()->g_RequestSumPath << "' version='"
                                << VortexInstaller::GetContext()->g_RequestVersion
                                << "' UseNet=" << VortexInstaller::GetContext()->g_UseNet
                                << " builtinExist=" << VortexInstaller::GetContext()->m_BuiltinLauncherExist);
#endif
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
#ifdef __APPLE__
    std::cerr << "Looked for: " << builtin_manifest << std::endl;
#endif
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