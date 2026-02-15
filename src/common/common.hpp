#pragma once
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../../lib/httpcl/httpcl.h"
#include "../../lib/json/single_include/nlohmann/json.hpp"

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
  std::string proper_name;
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
  VortexInstallerNet() {
    naettInit(nullptr);
  }
  ~VortexInstallerNet() {
  }

  bool CheckNet();
  std::string GET(const std::string &url) {
    return Request(url, "GET");
  }
  std::string POST(const std::string &url, const std::string &body, const std::string &contentType = "application/json") {
    return Request(url, "POST", body, contentType);
  }

 private:
  std::string Request(
      const std::string &url,
      const std::string &method,
      const std::string &body = "",
      const std::string &contentType = "") {
    naettReq *req = nullptr;

    if (method == "GET") {
      std::cerr << "GET request\n";

      const char *URL = url.c_str();
      req = naettRequest_va(URL, 2, naettMethod("GET"), naettHeader("accept", "*/*"));

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
    const char *responseBody = static_cast<const char *>(naettGetBody(res, &length));
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
  // --- Paths & env ---
  std::string g_VortexDataPath = "";
  std::string g_VortexPath = "";
  std::string g_DefaultInstallPath = "";
  std::string g_WorkingPath = "";
  std::string g_HomeDirectory = "";

  // --- Install context ---
  std::string g_Distribution = "stable";
  std::string g_Action = "install";
  std::string g_Platform = "";
  std::string g_Arch = "";

  // --- Versions / manifest ---
  std::string g_ManifestVersion = "";
  std::string g_RequestVersion = "";
  std::string g_RequestTarballPath = "";
  std::string g_RequestSumPath = "";

  // --- Localization ---
  std::string g_PreviousSelectedLanguage = "en";
  std::string g_SelectedLanguage = "en";

  // --- State ---
  std::string state = "Unknown";
  std::string result = "processing";
  int state_n = 0;
  bool finished = false;

  // --- Network ---
  bool g_Request = false;
  bool g_NetFetched = false;
  bool g_NetAvailable = true;
  bool g_UseNet = true;

  // --- JSON raw ---
  nlohmann::json jsonResponse;
  nlohmann::json g_RequestValues;

  // --- Version logic ---
  bool g_VortexLauncherOutdated = false;
  bool m_BuiltinLauncherExist = false;
  bool m_BuiltinLauncherNewer = false;
  bool m_NetLauncherNewer = false;
  bool m_FolderAlreadyExist = false;

  // --- Pools ---
  std::vector<std::string> m_VortexPools;

  // --- Complex objects (NOT serialized directly) ---
  VortexInstallerNet net;
  VortexVersion m_SelectedVortexVersion;
  VortexBuiltinLauncher m_BuiltinLauncher;

  /* ===================== IPC / SYNC ===================== */

  // UI -> Backend (patch partiel)
  void PatchFromJson(const nlohmann::json &patch) {
#define PATCH(field)          \
  if (patch.contains(#field)) \
    field = patch[#field];

    PATCH(g_WorkingPath)
    PATCH(g_DefaultInstallPath)
    PATCH(g_SelectedLanguage)
    PATCH(g_Action)
    PATCH(g_Distribution)
    PATCH(g_UseNet)
    PATCH(state)

#undef PATCH
  }

  mutable std::mutex mutex;

  nlohmann::json ToJson() const {
    std::lock_guard<std::mutex> lock(mutex);
    return { { "state", state }, { "state_n", state_n }, { "result", result }, { "finished", finished } };
  }
};

#ifndef _WIN32
#include <netdb.h>
#include <unistd.h>
#endif

inline bool VortexInstallerNet::CheckNet() {
#ifdef _WIN32
  int rc = std::system("ping -n 1 -w 2000 1.1.1.1 >NUL 2>&1");
  if (rc != 0)
    rc = std::system("ping -n 1 -w 2000 8.8.8.8 >NUL 2>&1");
  return rc == 0;
#else
  addrinfo hints{}, *res = nullptr;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo("infinite.si", "80", &hints, &res);

  if (res)
    freeaddrinfo(res);

  return (status == 0);
#endif
}
