#define SDL_MAIN_HANDLED
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "./ui/app.hpp"

static std::string version_name;
static std::string dist;
static std::string arch;
static std::string platform;
static std::string home_dir;
static std::vector<std::string> version_pools;

void RefreshEnvironmentVortexVersionsPools() {
  std::string path;
#if defined(_WIN32) || defined(_WIN64)
  path = VortexInstaller::GetContext()->g_HomeDirectory + "\\.vx\\configs\\";
#endif

#if defined(__linux__) || defined(__APPLE__)
  path = VortexInstaller::GetContext()->g_HomeDirectory + "/.vx/configs/";
#endif

  std::string json_file;
#if defined(_WIN32) || defined(_WIN64)
  json_file = path + "\\vortex_versions_pools.json";
#endif

#if defined(__linux__) || defined(__APPLE__)
  json_file = path + "/vortex_versions_pools.json";
#endif

  try {
    nlohmann::json json_data;

    if (!std::filesystem::exists(json_file)) {
      //
    }

    std::ifstream readed_file(json_file);

    if (!readed_file.is_open()) {
      //
    }

    readed_file >> json_data;

    for (auto pool : json_data["vortex_versions_pools"]) {
      version_pools.push_back(pool);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}

void parseArguments(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg.find("--arch=") == 0) {
      arch = arg.substr(7);
    }

    if (arg.find("--platform=") == 0) {
      platform = arg.substr(11);
    }

    if (arg.find("--dist=") == 0) {
      dist = arg.substr(7);
    }

    if (arg.find("--version_name=") == 0) {
      version_name = arg.substr(15);
    }

    if (arg.find("--home=") == 0) {
      VortexInstaller::GetContext()->g_HomeDirectory = arg.substr(7);
    }
  }
}

int main(int argc, char *argv[]) {
  VortexInstaller::CreateContext();

  VortexInstaller::GetContext()->g_WorkingPath = VortexInstaller::GetContext()->g_DefaultInstallPath;
  parseArguments(argc, argv);

  VortexInstaller::GetContext()->g_Action = "vxinstall";

  RefreshEnvironmentVortexVersionsPools();
  VortexInstaller::GetContext()->m_VortexPools = version_pools;
  std::thread([=]() {
    if (VortexInstaller::GetContext()->net.CheckNet()) {
      VortexInstaller::GetContext()->g_Request = true;
    }
  }).detach();

  std::thread([=]() {
    while (!VortexInstaller::GetContext()->g_NetFetched) {
      if (VortexInstaller::GetContext()->g_Request) {
        std::string url =
            "http://api.infinite.si:9000/api/vortexupdates/"
            "get_filtered_v_versions?platform=" +
            platform + "&dist=" + dist + "&arch=" + arch + "&name=" + version_name;

        std::string body = VortexInstaller::GetContext()->net.GET(url);
        auto json_response = nlohmann::json::parse(body);
        std::vector<VortexVersion> versions;

        for (const auto &item : json_response) {
          VortexVersion v;
          v.id = item["id"].get<int>();
          v.version = item["version"].get<std::string>();
          v.name = item["name"].get<std::string>();
          v.arch = item["arch"].get<std::string>();
          v.dist = item["dist"].get<std::string>();
          v.path = item["path"].get<std::string>();
          v.sum = item["sum"].get<std::string>();
          VortexInstaller::GetContext()->g_RequestTarballPath = v.path;
          VortexInstaller::GetContext()->g_RequestSumPath = v.sum;  // TODO
          v.platform = item["platform"].get<std::string>();
          v.date = item["date"].get<std::string>();
          v.created_at = item["created_at"].get<std::string>();

          if (item.contains("values")) {
            auto values = nlohmann::json::parse(item["values"].get<std::string>());
            if (values.contains("image")) {
              v.banner = values["image"].get<std::string>();
            }
            if (values.contains("proper_name")) {
              v.proper_name = values["proper_name"].get<std::string>();
            }
          }

          versions.push_back(v);
        }

        for (auto &v : versions) {
          if (v.name == version_name && v.arch == arch) {
            VortexInstaller::GetContext()->m_SelectedVortexVersion = v;
            break;
          }
        }
        VortexInstaller::GetContext()->g_NetFetched = true;
      }

      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  }).detach();

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
