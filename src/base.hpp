#pragma once

#ifndef BASE_VORTEXINSTALLER_H
#define BASE_VORTEXINSTALLER_H

#include <iostream>

#include "../lib/cherry/cherry.hpp"
#include "../lib/httpcl/httpcl.h"
#include "../lib/json/single_include/nlohmann/json.hpp"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

static void Space(const float &space) {
  CherryGUI::BeginDisabled();
  CherryGUI::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
  CherryGUI::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
  CherryGUI::Button("", ImVec2(0, space));
  CherryGUI::PopStyleColor(2);
  CherryGUI::EndDisabled();
}

static std::string ReadFile(const std::string &file_path) {
  std::ifstream file(file_path);
  if (!file.is_open())
    return "Erreur : Unable to open file";

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

static void CustomCheckbox(const std::string &label, bool *activated,
                           float radius = 7.0f,
                           const std::string hex = "#B1FF31FF") {
  ImVec2 pos = CherryGUI::GetCursorScreenPos();
  ImDrawList *draw_list = CherryGUI::GetWindowDrawList();

  ImVec2 text_size = CherryGUI::CalcTextSize(label.c_str());

  float offset_y = (text_size.y - radius * 2.0f) * 0.5f;

  ImU32 color_bg =
      activated ? IM_COL32(70, 70, 70, 255) : IM_COL32(50, 50, 50, 255);

  ImU32 color_check = Cherry::HexToImU32(hex);

  draw_list->AddCircleFilled(ImVec2(pos.x + radius, pos.y + radius + offset_y),
                             radius, color_bg, 16);

  if (activated) {
    draw_list->AddCircleFilled(
        ImVec2(pos.x + radius, pos.y + radius + offset_y), radius * 0.6f,
        color_check, 16);
  }

  CherryGUI::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));

  CherryGUI::SameLine();
  CherryGUI::TextUnformatted(label.c_str());
}

static std::string CookPath(const std::string input_path) {
  std::string output_path;
  std::string root_path;

#ifdef _WIN32
  char result[MAX_PATH];
  if (GetModuleFileNameA(NULL, result, MAX_PATH) != 0) {
    root_path = result;
  } else {
    std::cerr << "Failed while getting the root path" << std::endl;
  }
#else
  static std::mutex path_mutex;
  char result[PATH_MAX];
  {
    std::lock_guard<std::mutex> lock(path_mutex);
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
      result[count] = '\0';
      root_path = result;
    } else {
      std::cerr << "Failed while getting the root path" << std::endl;
    }
  }
#endif

  root_path = std::filesystem::path(root_path).parent_path().string();

  if (!input_path.empty() && input_path.front() == '/') {
    output_path = input_path;
  } else {
    output_path = root_path + "/" + input_path;
  }

  return output_path;
}

static void printManifest(const std::string &manifestPath) {
  std::ifstream manifestFile(manifestPath);
  if (!manifestFile.is_open()) {
    std::cerr << "Failed to open " << manifestPath << std::endl;
    return;
  }

  std::string line;
  while (std::getline(manifestFile, line)) {
    std::cout << line << std::endl;
  }
  manifestFile.close();
}

static std::string findManifestJson(const std::filesystem::path &startPath,
                                    int maxDepth = 10) {
  std::filesystem::path currentPath = startPath;
  int depth = 0;

  while (!currentPath.empty() && depth < maxDepth) {
    std::filesystem::path manifestPath = currentPath / "manifest.json";

    if (std::filesystem::exists(manifestPath)) {
      return manifestPath.string();
    }

    currentPath = currentPath.parent_path();
    ++depth;
  }

  return "";
}

struct VortexVersion {
  int id;
  std::string version;
  std::string name;
  std::string arch;
  std::string dist;
  std::string path;
  std::string sum;
  std::string platform;
  std::string date;
  std::string banner;
  std::string created_at;
};

struct VortexBuiltinLauncher {
  std::string version;
  std::string arch;
  std::string platform;
  std::string tarball;
  std::string sum;
};

class VortexInstallerNet {
public:
  VortexInstallerNet() { naettInit(nullptr); }
  ~VortexInstallerNet() {}

  bool CheckNet();
  std::string GET(const std::string &url) { return Request(url, "GET"); }
  std::string POST(const std::string &url, const std::string &body,
                   const std::string &contentType = "application/json") {
    return Request(url, "POST", body, contentType);
  }

private:
  std::string Request(const std::string &url, const std::string &method,
                      const std::string &body = "",
                      const std::string &contentType = "") {
    naettReq *req = nullptr;

    if (method == "GET") {
      std::cout << "GET request\n";

      const char *URL = url.c_str();
      req = naettRequest_va(URL, 2, naettMethod("GET"),
                            naettHeader("accept", "*/*"));

    } else if (method == "POST") {
      std::cout << "POST request\n";

    } else {
      std::cerr << "Unsupported HTTP method: " << method << std::endl;
      return "";
    }

    if (!req) {
      std::cerr << "Failed to create request\n";
      return "";
    }

    naettRes *res = naettMake(req);
    if (!res) {
      std::cerr << "Failed to make request\n";
      naettFree(req);
      return "";
    }

    while (!naettComplete(res)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    int status = naettGetStatus(res);
    if (status < 0) {
      std::cerr << "Request failed with status: " << status << std::endl;
      naettClose(res);
      naettFree(req);
      return "";
    }

    int length = 0;
    const char *responseBody =
        static_cast<const char *>(naettGetBody(res, &length));
    if (!responseBody || length == 0) {
      std::cerr << "Empty response body\n";
      naettClose(res);
      naettFree(req);
      return "";
    }

    std::string result(responseBody, length);

    naettClose(res);
    naettFree(req);

    return result;
  }
};

struct VortexInstallerData {
  std::string g_VortexDataPath = "";
  std::string g_VortexPath = "";
  std::string g_DefaultInstallPath = "";
  std::string g_WorkingPath = "";
  std::string g_HomeDirectory = "";
  std::string g_Distribution = "stable";
  std::string g_Action = "install";
  std::string g_Platform = "";
  std::string g_Arch = "";
  std::string g_ManifestVersion = "";
  std::string g_RequestVersion = "";
  std::string g_RequestTarballPath = "";
  std::string g_RequestSumPath = "";
  std::string g_PreviousSelectedLanguage = "en";
  std::string g_SelectedLanguage = "en";
  std::string state = "Unknown";
  std::string result = "processing";
  nlohmann::json jsonResponse;
  nlohmann::json g_RequestValues;
  bool g_Request = false;
  bool g_NetFetched = false;
  bool g_NetAvailable = true;
  bool g_UseNet = true;
  bool g_VortexLauncherOutdated = false;
  bool finished = false;
  int state_n = 0;
  std::function<void()> m_InstallCallback;
  std::function<void()> m_UpdateCallback;
  std::function<void()> m_DowngradeCallback;
  std::function<void()> m_InstallVortexCallback;
  std::function<void()> m_UninstallVortexCallback;
  std::function<void(const bool &vxlauncher, const bool &vx,
                     const bool &vxdata)>
      m_DeleteCallback;

  VortexInstallerNet net;

  VortexVersion m_SelectedVortexVersion;
  VortexBuiltinLauncher m_BuiltinLauncher;
  bool m_BuiltinLauncherExist = false;
  bool m_BuiltinLauncherNewer = false;
  bool m_FolderAlreadyExist;
  std::vector<std::string> m_VortexPools;
};

#endif // BASE_VORTEXINSTALLER_H