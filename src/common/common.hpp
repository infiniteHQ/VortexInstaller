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
  bool g_PollkitApproved = false;

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

  // UI -> Backend
  void PatchFromJson(const nlohmann::json &patch) {
    std::lock_guard<std::mutex> lock(mutex);
#define PATCH(field)          \
  if (patch.contains(#field)) \
    field = patch[#field];

    PATCH(g_PollkitApproved)

    // --- Paths & env ---
    PATCH(g_VortexDataPath)
    PATCH(g_VortexPath)
    PATCH(g_DefaultInstallPath)
    PATCH(g_WorkingPath)
    PATCH(g_HomeDirectory)

    // --- Install context ---
    PATCH(g_Distribution)
    PATCH(g_Action)
    PATCH(g_Platform)
    PATCH(g_Arch)

    // --- Versions / manifest ---
    PATCH(g_ManifestVersion)
    PATCH(g_RequestVersion)
    PATCH(g_RequestTarballPath)
    PATCH(g_RequestSumPath)

    // --- Localization ---
    PATCH(g_PreviousSelectedLanguage)
    PATCH(g_SelectedLanguage)

    // --- State ---
    PATCH(state)
    PATCH(result)
    PATCH(state_n)
    PATCH(finished)

    // --- Network ---
    PATCH(g_Request)
    PATCH(g_NetFetched)
    PATCH(g_NetAvailable)
    PATCH(g_UseNet)

    // --- JSON raw ---
    if (patch.contains("jsonResponse") && patch["jsonResponse"].is_object())
      jsonResponse = patch["jsonResponse"];
    if (patch.contains("g_RequestValues") && patch["g_RequestValues"].is_object())
      g_RequestValues = patch["g_RequestValues"];

    // --- Version logic ---
    PATCH(g_VortexLauncherOutdated)
    PATCH(m_BuiltinLauncherExist)
    PATCH(m_BuiltinLauncherNewer)
    PATCH(m_NetLauncherNewer)
    PATCH(m_FolderAlreadyExist)

    // --- Pools ---
    if (patch.contains("m_VortexPools") && patch["m_VortexPools"].is_array())
      m_VortexPools = patch["m_VortexPools"].get<std::vector<std::string>>();

    // --- Complex objects ---
    if (patch.contains("m_SelectedVortexVersion") && patch["m_SelectedVortexVersion"].is_object()) {
      const auto &v = patch["m_SelectedVortexVersion"];

#define PATCH_V(field)    \
  if (v.contains(#field)) \
    m_SelectedVortexVersion.field = v[#field];

      PATCH_V(id)
      PATCH_V(version)
      PATCH_V(name)
      PATCH_V(arch)
      PATCH_V(dist)
      PATCH_V(path)
      PATCH_V(sum)
      PATCH_V(platform)
      PATCH_V(date)
      PATCH_V(banner)
      PATCH_V(proper_name)
      PATCH_V(created_at)

#undef PATCH_V
    }

    if (patch.contains("m_BuiltinLauncher") && patch["m_BuiltinLauncher"].is_object()) {
      const auto &inner = patch["m_BuiltinLauncher"];
#define PATCH_INNER(field)    \
  if (inner.contains(#field)) \
    m_BuiltinLauncher.field = inner[#field];

      PATCH_INNER(version)
      PATCH_INNER(arch)
      PATCH_INNER(platform)
      PATCH_INNER(tarball)
      PATCH_INNER(sum)

#undef PATCH_INNER
    }

#undef PATCH
  }

  mutable std::mutex mutex;

  nlohmann::json ToJson() const {
    std::lock_guard<std::mutex> lock(mutex);
    return { { "g_PollkitApproved", g_PollkitApproved },
             // --- Paths & env ---
             { "g_VortexDataPath", g_VortexDataPath },
             { "g_VortexPath", g_VortexPath },
             { "g_DefaultInstallPath", g_DefaultInstallPath },
             { "g_WorkingPath", g_WorkingPath },
             { "g_HomeDirectory", g_HomeDirectory },

             // --- Install context ---
             { "g_Distribution", g_Distribution },
             { "g_Action", g_Action },
             { "g_Platform", g_Platform },
             { "g_Arch", g_Arch },

             // --- Versions / manifest ---
             { "g_ManifestVersion", g_ManifestVersion },
             { "g_RequestVersion", g_RequestVersion },
             { "g_RequestTarballPath", g_RequestTarballPath },
             { "g_RequestSumPath", g_RequestSumPath },

             // --- Localization ---
             { "g_PreviousSelectedLanguage", g_PreviousSelectedLanguage },
             { "g_SelectedLanguage", g_SelectedLanguage },

             // --- State ---
             { "state", state },
             { "result", result },
             { "state_n", state_n },
             { "finished", finished },

             // --- Network ---
             { "g_Request", g_Request },
             { "g_NetFetched", g_NetFetched },
             { "g_NetAvailable", g_NetAvailable },
             { "g_UseNet", g_UseNet },

             // --- JSON raw ---
             { "jsonResponse", jsonResponse },
             { "g_RequestValues", g_RequestValues },

             // --- Version logic ---
             { "g_VortexLauncherOutdated", g_VortexLauncherOutdated },
             { "m_BuiltinLauncherExist", m_BuiltinLauncherExist },
             { "m_BuiltinLauncherNewer", m_BuiltinLauncherNewer },
             { "m_NetLauncherNewer", m_NetLauncherNewer },
             { "m_FolderAlreadyExist", m_FolderAlreadyExist },

             // --- Pools ---
             { "m_VortexPools", m_VortexPools },

             // --- Complex objects ---
             { "m_SelectedVortexVersion",
               { { "id", m_SelectedVortexVersion.id },
                 { "version", m_SelectedVortexVersion.version },
                 { "name", m_SelectedVortexVersion.name },
                 { "arch", m_SelectedVortexVersion.arch },
                 { "dist", m_SelectedVortexVersion.dist },
                 { "path", m_SelectedVortexVersion.path },
                 { "sum", m_SelectedVortexVersion.sum },
                 { "platform", m_SelectedVortexVersion.platform },
                 { "date", m_SelectedVortexVersion.date },
                 { "banner", m_SelectedVortexVersion.banner },
                 { "proper_name", m_SelectedVortexVersion.proper_name },
                 { "created_at", m_SelectedVortexVersion.created_at } } },
             { "m_BuiltinLauncher",
               { { "version", m_BuiltinLauncher.version },
                 { "arch", m_BuiltinLauncher.arch },
                 { "platform", m_BuiltinLauncher.platform },
                 { "tarball", m_BuiltinLauncher.tarball },
                 { "sum", m_BuiltinLauncher.sum } } } };
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
