#pragma once
#define CHERRY_V1
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

#include "../../lib/cherry/cherry.hpp"
#include "../../src/base.hpp"
#include "src/static/install/install.hpp"
#include "src/static/install_vx/install_vx.hpp"
#include "src/static/uninstall/uninstall.hpp"
#include "src/static/uninstall_vx/uninstall_vx.hpp"
#include "src/static/update/update.hpp"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFString.h>
#endif

static std::string DetectSystemLanguage() {
#if defined(_WIN32)
  wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
  if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH)) {
    char buffer[LOCALE_NAME_MAX_LENGTH];
    wcstombs(buffer, localeName, LOCALE_NAME_MAX_LENGTH);
    std::string lang(buffer);

    auto pos = lang.find('-');
    if (pos != std::string::npos)
      lang = lang.substr(0, pos);

    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);
    return lang;
  }
#else
  const char *envVars[] = { "LC_ALL", "LANG", "LC_MESSAGES" };
  for (auto var : envVars) {
    const char *val = std::getenv(var);
    if (val && *val) {
      std::string s(val);

      auto pos = s.find('_');
      if (pos != std::string::npos)
        s = s.substr(0, pos);
      pos = s.find('.');
      if (pos != std::string::npos)
        s = s.substr(0, pos);

      std::transform(s.begin(), s.end(), s.begin(), ::tolower);
      return s;
    }
  }
#endif
  return "en";
}

static int GetDefaultSelectedLanguage() {
  std::string sysLang = DetectSystemLanguage();
  std::transform(sysLang.begin(), sysLang.end(), sysLang.begin(), ::tolower);

  if (sysLang.find("fr") != std::string::npos) {
    return 1;
  } else if (sysLang.find("es") != std::string::npos) {
    return 2;
  }
  return 0;
}

#ifdef _WIN32
#include <fileapi.h>
#include <urlmon.h>
#include <windows.h>
#pragma comment(lib, "urlmon.lib")

#include <objbase.h>
#include <shlguid.h>
#include <shobjidl.h>

#include <iostream>
#include <string>

bool CreateShortcut(
    const std::string &targetPath,
    const std::string &shortcutPath,
    const std::string &description,
    const std::string &iconPath,
    int iconIndex = 0) {
  HRESULT hres;
  IShellLinkW *pShellLink = nullptr;

  hres = CoInitialize(NULL);
  if (FAILED(hres)) {
    return false;
  }

  hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **)&pShellLink);
  if (SUCCEEDED(hres)) {
    IPersistFile *pPersistFile = nullptr;

    wchar_t wszTargetPath[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, targetPath.c_str(), -1, wszTargetPath, MAX_PATH);

    pShellLink->SetPath(wszTargetPath);

    wchar_t wszDescription[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, description.c_str(), -1, wszDescription, MAX_PATH);
    pShellLink->SetDescription(wszDescription);

    if (!iconPath.empty()) {
      wchar_t wszIconPath[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, iconPath.c_str(), -1, wszIconPath, MAX_PATH);
      pShellLink->SetIconLocation(wszIconPath, iconIndex);
    }

    hres = pShellLink->QueryInterface(IID_IPersistFile, (void **)&pPersistFile);
    if (SUCCEEDED(hres)) {
      wchar_t wsz[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, shortcutPath.c_str(), -1, wsz, MAX_PATH);

      hres = pPersistFile->Save(wsz, TRUE);
      pPersistFile->Release();
    }
    pShellLink->Release();
  }
  CoUninitialize();
  return SUCCEEDED(hres);
}

#else
#include <unistd.h>

std::string ReplaceSpacesWithUnderscores(const std::string &str) {
  std::string result = str;
  std::replace(result.begin(), result.end(), ' ', '_');
  return result;
}

bool CreateShortcut(
    const std::string &name,
    const std::string &targetPath,
    const std::string &shortcutPath,
    const std::string &description,
    const std::string &iconPath) {
  std::string fileName = ReplaceSpacesWithUnderscores(name) + ".desktop";
  std::string desktopFilePath = shortcutPath + "/" + fileName;

  if (std::filesystem::exists(desktopFilePath)) {
    if (!std::filesystem::remove(desktopFilePath)) {
      std::cerr << "Error while deleting the old shorcut." << std::endl;
      return false;
    }
    std::cout << "Old shortcut deleted" << std::endl;
  }

  std::string content =
      "[Desktop Entry]\n"
      "Name=" +
      name +
      "\n"
      "Exec=" +
      targetPath +
      "\n"
      "Icon=" +
      iconPath +
      "\n"
      "Terminal=false\n"
      "Type=Application\n"
      "Categories=Utility;Development;\n";

  std::ofstream desktopFile(desktopFilePath);
  if (!desktopFile) {
    std::cerr << "Error while deleting : " << desktopFilePath << std::endl;
    return false;
  }

  desktopFile << content;
  desktopFile.close();

  std::string chmodCommand = "chmod +x " + desktopFilePath;
  if (std::system(chmodCommand.c_str()) != 0) {
    return false;
  }

  return true;
}

#endif

#if defined(VXI_LOGS)
#define VXI_LOG(log) std::cout << log << std::endl;
#else
#define VXI_LOG(log)
#endif

bool IsCurlAvailable() {
#ifdef _WIN32
  return true;
#else
  return system("which curl >/dev/null 2>&1") == 0;
#endif
}

static void CreateFolder(const std::string &path) {
  if (!std::filesystem::exists(path)) {
    try {
      std::string cmd = "mkdir " + path;
      system(cmd.c_str());
    } catch (const std::exception &ex) {
      std::cout << "Failed to create the folder : " << ex.what() << std::endl;
    }
  } else {
    std::cout << "Path already exist : " << path << std::endl;
  }
}

static std::shared_ptr<VortexInstallerData> g_InstallerData = nullptr;

using namespace VortexInstaller;

bool IsSafePath(const std::filesystem::path &path) {
  const std::vector<std::filesystem::path> dangerous_paths = { "/",    "/bin",  "/boot", "/dev",  "/etc", "/lib", "/lib64",
                                                               "/opt", "/proc", "/root", "/sbin", "/sys", "/usr", "/var" };

  for (const auto &dangerous_path : dangerous_paths) {
    if (std::filesystem::exists(path) && std::filesystem::exists(dangerous_path)) {
      if (std::filesystem::equivalent(path, dangerous_path)) {
        std::cerr << "Cannot delete this path : " << path << std::endl;
        return false;
      }
    }
  }

  return true;
}

std::string GetUncompressCommand(const std::string &tarballFile, const std::string &installPath, int strip = 1) {
  std::string command;

#ifdef _WIN32
  command = "cmd /C \"rd /s /q \"" + installPath + "\" && tar -xzf \"" + tarballFile + "\" --strip-components=1 -C \"" +
            installPath + "\" dist\\\"";
#else
  std::filesystem::path homeDir = std::getenv("HOME");
  std::filesystem::path destPath = installPath;

  if (!std::filesystem::exists(destPath)) {
    std::cerr << "Path does not exist: " << installPath << std::endl;
    return "";
  }

  if (!IsSafePath(destPath)) {
    std::cerr << "Cannot delete this path: " << installPath << std::endl;
    return "";
  }

  if (destPath.string().find(homeDir.string()) == 0) {
    command =
        "tar -xzf " + tarballFile + " --strip-components=" + std::to_string(strip) + " -C \"" + installPath + "\" dist";
  } else {
    command =
        "tar -xzf " + tarballFile + " --strip-components=" + std::to_string(strip) + " -C \"" + installPath + "\" dist";
  }
#endif

  return command;
}

std::string GetTopLevelDir(const std::string &tarballFile) {
  std::string command = "tar -tzf " + tarballFile;
  std::array<char, 128> buffer;
  std::string line;

  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    std::cerr << "Failed to open pipe for command." << std::endl;
    return "";
  }

  std::regex distDirPattern(R"(^dist/([^/]+)/)");
  std::smatch match;

  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    line = buffer.data();
    if (std::regex_search(line, match, distDirPattern)) {
      pclose(pipe);
      return match.str(1);
    }
  }

  pclose(pipe);
  std::cerr << "No top-level directory found under 'dist/' in the tarball." << std::endl;
  return "";
}

std::string GetFinalLink(const std::string &tarballFile, const std::string &installPath, int strip = 1) {
  std::string topLevelDir = GetTopLevelDir(tarballFile);
  if (topLevelDir.empty()) {
    std::cerr << "Failed to find top-level directory in tarball." << std::endl;
    return "";
  }

  std::string finalPath = installPath;

  std::string command = GetUncompressCommand(tarballFile, installPath, strip);

  int result = std::system(command.c_str());

  if (result != 0) {
    std::cerr << "Failed to uncompress tarball." << std::endl;
    return "";
  }

  if (std::filesystem::exists(finalPath) && std::filesystem::is_directory(finalPath)) {
    std::cout << "Successfully uncompressed to: " << finalPath << std::endl;
    return finalPath;
  } else {
    std::cerr << "Uncompression failed or destination path does not exist." << std::endl;
    return "";
  }
}

class Layer : public Cherry::Layer {
 public:
  Layer() { };
};

class Launcher {
 public:
  Launcher(const std::shared_ptr<VortexInstallerData> &data) : installer_data(data) {
    if (installer_data->g_Action == "install") {
      install_window = InstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(install_window->GetAppWindow());
    } else if (installer_data->g_Action == "vxinstall") {
      installer_vx = VortexInstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(installer_vx->GetAppWindow());
    } else if (installer_data->g_Action == "vxuninstall") {
      uninstaller_vx = VortexUninstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(uninstaller_vx->GetAppWindow());
    } else if (installer_data->g_Action == "update") {
      update_window = UpdateAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(update_window->GetAppWindow());
    } else if (installer_data->g_Action == "uninstall") {
      uninstall_window = UninstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(uninstall_window->GetAppWindow());
    }
  };

 private:
  std::shared_ptr<InstallAppWindow> install_window;
  std::shared_ptr<UpdateAppWindow> update_window;
  std::shared_ptr<UninstallAppWindow> uninstall_window;
  std::shared_ptr<VortexInstallAppWindow> installer_vx;
  std::shared_ptr<VortexUninstallAppWindow> uninstaller_vx;
  std::shared_ptr<VortexInstallerData> installer_data;
};

static std::shared_ptr<Launcher> c_Launcher;

void DetectPlatform() {
#if defined(_WIN32) || defined(_WIN64)
  g_InstallerData->g_Platform = "windows";
  g_InstallerData->g_DefaultInstallPath = "C:/Program Files/VortexLauncher";
  g_InstallerData->g_VortexPath = "C:/Program Files/Vortex";
  g_InstallerData->g_VortexDataPath = g_InstallerData->g_HomeDirectory + "/.vx";
#elif defined(__APPLE__) && defined(__MACH__)
  g_InstallerData->g_Platform = "macos";
  g_InstallerData->g_DefaultInstallPath = "/Applications/VortexLauncher";
  g_InstallerData->g_VortexPath = "/Applications/Vortex";
  g_InstallerData->g_VortexDataPath = g_InstallerData->g_HomeDirectory + "/.vx";
#elif defined(__linux__)
  g_InstallerData->g_Platform = "linux";
  g_InstallerData->g_DefaultInstallPath = "/opt/VortexLauncher";
  g_InstallerData->g_VortexPath = "/opt/Vortex";  // TODO : Pools
  g_InstallerData->g_VortexDataPath = g_InstallerData->g_HomeDirectory + "/.vx";
#elif defined(__FreeBSD__)
  g_InstallerData->g_Platform = "freebsd";
  g_InstallerData->g_DefaultInstallPath = "/opt/VortexLauncher";
  g_InstallerData->g_VortexPath = "/opt/Vortex";
  g_InstallerData->g_VortexDataPath = g_InstallerData->g_HomeDirectory + "/.vx";
#else
  g_InstallerData->g_Platform = "unknown";
  g_InstallerData->g_DefaultInstallPath = "?";
  g_InstallerData->g_VortexPath = "?";
  g_InstallerData->g_VortexDataPath = "?";
#endif
}

void DetectArch() {
#if defined(__x86_64__) || defined(_M_X64)
  g_InstallerData->g_Arch = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
  g_InstallerData->g_Arch = "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
  g_InstallerData->g_Arch = "arm64";
#elif defined(__arm__) || defined(_M_ARM)
  g_InstallerData->g_Arch = "arm";
#elif defined(__riscv)
  g_InstallerData->g_Arch = "riscv";
#elif defined(__ppc64__)
  g_InstallerData->g_Arch = "ppc64";
#else
  g_InstallerData->g_Arch = "unknown";
#endif
}

bool FileExists(const std::string &path) {
  return std::filesystem::exists(path);
}

std::string NormalizePath(const std::string &path) {
#ifdef _WIN32
  std::string normalizedPath = path;
  std::replace(normalizedPath.begin(), normalizedPath.end(), '/', '\\');
  return normalizedPath;
#else
  std::string normalizedPath = path;
  std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
  return normalizedPath;
#endif
}

bool DownloadFile(const std::string &url, const std::string &outputPath) {
  std::string normalizedOutputPath = NormalizePath(outputPath);
#ifdef _WIN32
  HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), normalizedOutputPath.c_str(), 0, NULL);
  if (hr != S_OK) {
    std::cerr << "Error downloading: " << url << std::endl;
  }
  return hr == S_OK;
#else
  std::string downloadCommand = "curl -o " + normalizedOutputPath + " " + url;
  return system(downloadCommand.c_str()) == 0;
#endif
}

void CleanUpTemporaryDirectory(const std::string &tempDir) {
  if (std::filesystem::exists(tempDir)) {
    if (!IsSafePath(tempDir)) {
      std::cerr << "Cannot delete this path: " << tempDir << std::endl;
      return;
    }

    std::string command = "rm -rf " + tempDir;
    system(command.c_str());
  }
}

std::string getManifestVersion(const std::string &manifestPath) {
  std::ifstream manifestFile(manifestPath);
  if (!manifestFile.is_open()) {
    // std::cerr << "Failed to open " << manifestPath);
    return "";
  }

  nlohmann::json manifestJson;
  try {
    manifestFile >> manifestJson;
  } catch (const std::exception &e) {
    // std::cerr << "Failed to parse JSON: " << e.what());
    return "";
  }

  manifestFile.close();

  if (manifestJson.contains("version") && manifestJson["version"].is_string()) {
    return manifestJson["version"];
  } else {
    // std::cerr << "\"version\" field not found or not a string");
    return "";
  }
}
std::string RevertOldVortexLauncher(const std::string &path) {
  auto &installerData = *g_InstallerData;
  std::string installPath = path;
  std::string manifestPath = installPath + "/manifest.json";

  auto endsWith = [](const std::string &str, const std::string &suffix) -> bool {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  };

  std::string newPath = "none";

  if (endsWith(installPath, "VortexLauncherOld") || endsWith(installPath, "VortexLauncherOld/")) {
    if (std::filesystem::exists(manifestPath)) {
      VXI_LOG("manifest.json finded at : " + manifestPath);

      newPath = installPath.substr(0, installPath.find_last_of("/")) + "/VortexLauncher";

      try {
        std::filesystem::rename(installPath, newPath);
      } catch (const std::filesystem::filesystem_error &e) {
        // std::cerr << "Error while renaming : " << e.what());
      }
    } else {
      // std::cerr << "Error : manifest.json not found in " << installPath);
    }
  } else {
    // std::cerr << "Error : This directory not end with VortexLauncher or VortexLauncher/.");
  }

  return newPath;
}

// 3 Steps
std::string MakeVortexLauncherFolderOld(const std::string &path) {
  auto &installerData = *g_InstallerData;
  std::string installPath = path;
  std::string manifestPath = installPath + "/manifest.json";

  auto endsWith = [](const std::string &str, const std::string &suffix) -> bool {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  };

  std::string newPath = "none";

  installerData.state_n++;
  installerData.state = "Checking path...";

  if (endsWith(installPath, "VortexLauncher") || endsWith(installPath, "VortexLauncher/")) {
    if (std::filesystem::exists(manifestPath)) {
      installerData.state_n++;
      installerData.state = "Checking if path exist...";

      VXI_LOG("manifest.json finded at : " + manifestPath);

      newPath = installPath.substr(0, installPath.find_last_of("/")) + "/VortexLauncherOld";

      try {
        installerData.state_n++;
        installerData.state = "Rename path to old...";
        std::filesystem::rename(installPath, newPath);
        VXI_LOG("Folder renamed to : " + newPath);
      } catch (const std::filesystem::filesystem_error &e) {
        VXI_LOG("Error while renaming folder : " << e.what());
      }
    } else {
      VXI_LOG("manifest.json not found " << installPath);
    }
  } else {
    VXI_LOG("Error : This directory not end with VortexLauncher or VortexLauncher/.");
  }

  return newPath;
}

// 2 Check
void DeleteOldVortexLauncher(const std::string &path) {
  auto &installerData = *g_InstallerData;
  std::string installPath = path;
  std::string manifestPath = installPath + "/manifest.json";

  installerData.state_n++;
  installerData.state = "Check old vortex folder...";
  if (std::filesystem::exists(manifestPath)) {
    VXI_LOG("Found manifest.json at: " + manifestPath);
    VXI_LOG("Deleting folder: " + installPath);
    try {
      installerData.state_n++;
      installerData.state = "Delete old vortex folder...";
      std::filesystem::remove_all(installPath);
      VXI_LOG("Successfully deleted the folder: " << installPath);
    } catch (const std::filesystem::filesystem_error &e) {
      VXI_LOG("Error deleting the folder: " << e.what());
    }
  } else {
    VXI_LOG("Error: manifest.json not found in " << installPath);
  }
}

void DeleteVortexLauncher(const bool &vxlauncher, const bool &vx, const bool &vxdatas) {
  auto &installerData = *g_InstallerData;
  bool failed = false;

  DetectPlatform();

  auto isValidPath = [](const std::string &path) -> bool {
#ifdef _WIN32
    return !path.empty() && path != "\\" && std::filesystem::exists(path);
#else
    return !path.empty() && path != "/" && std::filesystem::exists(path);
#endif
  };

  if (vxlauncher) {
    std::string installPath = installerData.g_WorkingPath;
    std::string manifestPath;

#ifdef _WIN32
    manifestPath = installPath + "\\manifest.json";
#else
    manifestPath = installPath + "/manifest.json";
#endif

    installerData.state_n++;
    installerData.state = "Vérification du Vortex Launcher...";
    if (std::filesystem::exists(manifestPath)) {
      VXI_LOG("Fichier manifest.json finded at : " << manifestPath);
      VXI_LOG("Deleting folder : " << installPath);

      installerData.state_n++;
      installerData.state = "Deleting Vortex Launcher...";
      try {
        if (isValidPath(installPath)) {
#if defined(_WIN32) || defined(_WIN64)
          std::string command = "rmdir /S /Q \"" + installPath + "\"";
#else
          std::string command = "rm -rf \"" + installPath + "\"";
#endif
          int result = std::system(command.c_str());

          if (result == 0) {
            std::cout << "Folder deleted: " << installPath << std::endl;
          } else {
            throw std::runtime_error("Failed to delete folder using command: " + command);
          }
        } else {
          throw std::filesystem::filesystem_error("Bad directory", std::error_code());
        }
      } catch (const std::exception &e) {
        std::cerr << "ZE: " << e.what() << std::endl;
        installerData.result = "fail";
        failed = true;
        installerData.state = "Error while deleting the Vortex Launcher folder";
      }
    } else {
      installerData.result = "fail";
      failed = true;
      installerData.state = "Error: Path to Vortex Launcher invalid !";
    }
  }

  if (vx) {
    std::string installPath = installerData.g_VortexPath;
    installerData.state_n++;
    installerData.state = "Deleting Vortex...";

    if (isValidPath(installPath)) {
      try {
        std::filesystem::remove_all(installPath);
        VXI_LOG("Deleted with success :  " << installPath);
      } catch (const std::filesystem::filesystem_error &e) {
        // std::cerr << "Error while deleting Vortex Launcher folder : " << e.what());
        installerData.result = "fail";
        failed = true;
        installerData.state = "Failed while deleting the Vortex Launcher folder !";
      }
    } else {
      // std::cerr << "Vortex Launcher not found at path : " << installPath);
      installerData.result = "fail";
      failed = true;
      installerData.state = "Failed ! Vortex Launcher path is invalid !";
    }
  }

  if (vxdatas) {
    std::string installPath = installerData.g_VortexDataPath;
    installerData.state_n++;
    installerData.state = "Deleting Vortex datas...";

    if (isValidPath(installPath)) {
      try {
        std::filesystem::remove_all(installPath);
        VXI_LOG("Deleted with success :  " << installPath);
      } catch (const std::filesystem::filesystem_error &e) {
        installerData.result = "fail";
        failed = true;
        installerData.state = "Failed while deleting Vortex datas !";
      }
    } else {
      installerData.result = "fail";
      failed = true;
      installerData.state = "Path to Vortex datas is invalid";
    }
  }

  if (!failed) {
    installerData.state_n++;
    installerData.state = "Deleted succefully !";
    installerData.result = "success";
  }
}

void DeleteVortexVersion() {
  auto &installerData = *g_InstallerData;
  bool failed = false;

  DetectPlatform();

  auto isValidPath = [](const std::string &path) -> bool {
    return !path.empty() && path != "/" && std::filesystem::exists(path);
  };

  std::string installPath = installerData.g_WorkingPath;
  std::string manifestPath = installPath + "/manifest.json";
  std::cout << installPath << std::endl;

  installerData.state_n++;
  installerData.state = "Verify Vortex Launcher...";
  if (std::filesystem::exists(manifestPath)) {
    installerData.state_n++;
    installerData.state = "Deleting Vortex Launcher...";
    try {
      if (isValidPath(installPath)) {
        VXI_LOG("Folder deletion attempt: " << installPath);

        if (IsSafePath(installPath)) {
          try {
            // Use std::filesystem for cross-platform folder deletion
            std::filesystem::remove_all(installPath);
            VXI_LOG("Folder deleted successfully: " << installPath);
          } catch (const std::filesystem::filesystem_error &e) {
            installerData.result = "fail";
            failed = true;
            installerData.state = "Error while deleting the folder: " + std::string(e.what());
            VXI_LOG("Error: " << e.what());
          }
        } else {
          VXI_LOG("Unsafe path detected, skipping deletion: " << path);
        }
      } else {
        throw std::filesystem::filesystem_error("Invalid path", std::error_code());
      }
    } catch (const std::filesystem::filesystem_error &e) {
      installerData.result = "fail";
      failed = true;
      installerData.state = "Error while deleting the Vortex Launcher folder";
    }
  } else {
    installerData.result = "fail";
    failed = true;
    installerData.state = "Error: Path to Vortex Launcher invalid !";
  }

  if (!failed) {
    installerData.state_n++;
    installerData.state = "Deleted succefully !";
    installerData.result = "success";
  }
}

// 5 Steps
bool InstallVortexLauncher() {
  auto &installerData = *g_InstallerData;

  // installerData.state_n++;
  installerData.state = "Initialization...";

  std::string tempDir;
#ifdef _WIN32
  tempDir = std::filesystem::temp_directory_path().string() + "\\vortex_installer";
#else
  tempDir = "/tmp/vortex_installer";
#endif
  std::filesystem::create_directories(tempDir);

  std::cout << "FQ" << std::endl;

  if (installerData.g_UseNet) {
    installerData.state_n++;
    installerData.state = "Downloading files...";

    std::string dlpath = installerData.g_RequestTarballPath;
    std::string sumpath = installerData.g_RequestSumPath;
    std::string installPath = installerData.g_DefaultInstallPath;

    std::string tarballFile = tempDir + "/" + dlpath.substr(dlpath.find_last_of("/\\") + 1);
    std::string sumFile = tempDir + "/" + sumpath.substr(sumpath.find_last_of("/\\") + 1);

    if (!IsCurlAvailable()) {
      std::cerr << "Error: curl is not installed on this system." << std::endl;
      return false;
    }

    if (!DownloadFile(dlpath, NormalizePath(tarballFile)) || !DownloadFile(sumpath, NormalizePath(sumFile))) {
      std::cerr << "Error: Failed to download files." << std::endl;
      return false;
    }

    installerData.state_n++;
    installerData.state = "Verifying integrity...";

    std::filesystem::current_path(tempDir);

    std::string checkSumCommand;
#ifdef _WIN32
    checkSumCommand = "CertUtil -hashfile " + tarballFile + " SHA256";
#else
    checkSumCommand = "sha256sum -c " + sumFile;
#endif
    if (system(checkSumCommand.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Integrity check failed.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Ensure clean install path...";
    std::filesystem::path path(installPath);

    if (std::filesystem::exists(path)) {
      if (!IsSafePath(path)) {
        std::cerr << "Cannot delete this safe path : " << installPath << std::endl;
        return false;
      }

      try {
        std::filesystem::remove_all(path);
        std::cout << "Folder deleted : " << installPath << std::endl;
      } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Error deleting folder: " << e.what() << std::endl;
        installerData.result = "fail";
        installerData.state = "Error: Failed to prepare installation folder.";
        return false;
      }
    }

    try {
      std::filesystem::create_directories(path);
      std::cout << "New folder created : " << installPath << std::endl;
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "Error creating folder: " << e.what() << std::endl;
      installerData.result = "fail";
      installerData.state = "Error: Failed to prepare installation folder.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Extracting files...";

    /* TODO : Link this suppr with uncompress cmd

    if (!std::filesystem::exists(installPath))
    {
      std::filesystem::create_directories(installPath);
    }

    if (std::filesystem::exists(installPath) && std::filesystem::is_directory(installPath))
    {
      for (const auto &entry : std::filesystem::directory_iterator(installPath))
      {
        DeleteFileCrossPlatform(entry.path().string());
      }
    }*/

    std::cout << "FQ88" << std::endl;

    std::string uncompressCommand;
#ifdef _WIN32
    uncompressCommand = "cmd /C \"tar -xzf \"" + tarballFile + "\" --strip-components=1 -C \"" + installPath + "\"\"";
#else
    // uncompressCommand = "tar -xzf " + tarballFile + " --strip-components=1 -C " + installPath + " dist/";
    GetFinalLink(tarballFile, installPath);
#endif
    std::cout << "FQ55" << std::endl;

    std::cout << "INSTALL PATH : " << installPath << std::endl;
    if (system(uncompressCommand.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to extract tarball.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Running vortex_launcher test...";

    std::string testLauncher;
#ifdef _WIN32
    testLauncher = "cd \"" + installPath + "\\bin\" && vortex_launcher.exe --test";
#else
    testLauncher = installPath + "/bin/vortex_launcher --test";
#endif
    std::cout << "FQ2" << std::endl;
    if (system(testLauncher.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Launcher test failed.";
      return false;
    }
    std::cout << "FQ3" << std::endl;

    installerData.state_n++;
    installerData.state = "Installation completed successfully.";
    installerData.result = "success";
  } else {
    std::string dlpath = installerData.g_RequestTarballPath;
    std::string installPath = installerData.g_DefaultInstallPath;

    std::string tarballFile;
    std::string sumFile;
    std::string sumPath;

#ifdef _WIN32
    tarballFile = Cherry::GetPath("builtin\\" + installerData.m_BuiltinLauncher.tarball);
#else
    tarballFile = Cherry::GetPath("builtin/" + installerData.m_BuiltinLauncher.tarball);
#endif

#ifdef _WIN32
    sumFile = Cherry::GetPath("builtin\\" + installerData.m_BuiltinLauncher.sum);
#else
    sumPath = Cherry::GetPath("builtin/");
    sumFile = installerData.m_BuiltinLauncher.sum;
#endif

    installerData.state_n++;
    installerData.state = "Verifying integrity...";

    std::filesystem::current_path(tempDir);

    if (!std::filesystem::exists(tarballFile)) {
      std::cerr << "Error: Tarball file does not exist at " << tarballFile << std::endl;
      installerData.result = "fail";
      installerData.state = "Error: Missing tarball file.";
      return false;
    }

    /*if (!std::filesystem::exists(sumFile)) {
        std::cerr << "Error: Sum file does not exist at " << sumFile << std::endl;
        installerData.result = "fail";
        installerData.state = "Error: Missing sum file.";
        return false;
    }*/

    std::string checkSumCommand;
#ifdef _WIN32
    checkSumCommand = "CertUtil -hashfile " + tarballFile + " SHA256";
#else
    checkSumCommand = "cd " + sumPath + " && sha256sum -c " + sumFile;
#endif
    if (system(checkSumCommand.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Integrity check failed.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Ensure clean install path...";
    std::filesystem::path path(installPath);

    if (std::filesystem::exists(path)) {
      if (!IsSafePath(path)) {
        std::cerr << "Cannot delete this safe path : " << installPath << std::endl;
        return false;
      }

      try {
        std::filesystem::remove_all(path);
        std::cout << "Folder deleted : " << installPath << std::endl;
      } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Error deleting folder: " << e.what() << std::endl;
        installerData.result = "fail";
        installerData.state = "Error: Failed to prepare installation folder.";
        return false;
      }
    }

    try {
      std::filesystem::create_directories(path);
      std::cout << "New folder created : " << installPath << std::endl;
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "Error creating folder: " << e.what() << std::endl;
      installerData.result = "fail";
      installerData.state = "Error: Failed to prepare installation folder.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Extracting files...";

    std::string uncompressCommand;
#ifdef _WIN32
    uncompressCommand = "cmd /C \"tar -xzf \"" + tarballFile + "\" --strip-components=1 -C \"" + installPath + "\"\"";
#else
    // uncompressCommand = "tar -xzf " + tarballFile + " --strip-components=1 -C " + installPath + " dist/";
    GetFinalLink(tarballFile, installPath);
#endif
    std::cout << "FQ55" << std::endl;

    std::cout << "INSTALL PATH : " << installPath << std::endl;
    if (system(uncompressCommand.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to extract tarball.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Running vortex_launcher test...";

    std::string testLauncher;
#ifdef _WIN32
    testLauncher = "cd \"" + installPath + "\\bin\" && vortex_launcher.exe --test";
#else
    testLauncher = installPath + "/bin/vortex_launcher --test";
#endif
    std::cout << "FQ2" << testLauncher << std::endl;
    if (system(testLauncher.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Launcher test failed.";
      return false;
    }
    std::cout << "FQ3" << std::endl;

    installerData.state_n++;
    installerData.state = "Installation completed successfully.";
    installerData.result = "success";

    // installerData.result = "fail";
    // installerData.state = "Error: Network usage is disabled. Cannot proceed with installation.";
  }

  std::string installPath = installerData.g_DefaultInstallPath;
#ifdef _WIN32
  {
    std::string shortcutPath = "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Vortex Launcher.lnk";
    if (!CreateShortcut(
            installPath + "\\bin\\vortex_launcher.exe",
            shortcutPath,
            "The Vortex creation platform",
            installPath + "\\bin\\resources\\imgs\\favicon.ico")) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to create Start Menu shortcut.";
      return false;
    }
  }

  {
    std::string shortcutPath = "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Vortex Updater.lnk";
    if (!CreateShortcut(
            installPath + "\\bin\\VortexUpdater.exe",
            shortcutPath,
            "The Vortex creation platform",
            installPath + "\\bin\\resources\\imgs\\favicon_updater.ico")) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to create Start Menu shortcut.";
      return false;
    }
  }

  {
    std::string shortcutPath = "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Uninstall Vortex.lnk";
    if (!CreateShortcut(
            installPath + "\\bin\\VortexUninstaller.exe",
            shortcutPath,
            "The Vortex creation platform",
            installPath + "\\bin\\resources\\imgs\\favicon_uninstaller.ico")) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to create Start Menu shortcut.";
      return false;
    }
  }
#else
  {
    std::string shortcutPath = "/usr/share/applications";
    if (!CreateShortcut(
            "Vortex Launcher",
            installPath + "/bin/vortex_launcher",
            shortcutPath,
            "The Vortex creation platform",
            installPath + "/bin/resources/imgs/icon.png")) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to create Start Menu shortcut.";
      return false;
    }
  }
  {
    std::string shortcutPath = "/usr/share/applications";
    if (!CreateShortcut(
            "Update Vortex",
            installPath + "/bin/VortexUpdater",
            shortcutPath,
            "Update Vortex to the latest version",
            installPath + "/bin/resources/imgs/icon_update.png")) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to create Start Menu shortcut.";
      return false;
    }
  }
  {
    std::string shortcutPath = "/usr/share/applications";
    if (!CreateShortcut(
            "Uninstall Vortex",
            installPath + "/bin/VortexUninstaller",
            shortcutPath,
            "Uninstall and delete Vortex",
            installPath + "/bin/resources/imgs/icon_crash.png")) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to create Start Menu shortcut.";
      return false;
    }
  }

#endif

  // Create default Vortex version folder
  std::string def_vx_path;

#if defined(_WIN32) || defined(_WIN64)
  def_vx_path = "C:/Program Files/Vortex";
#else
  def_vx_path = "/opt/Vortex/";
#endif
  CreateFolder(def_vx_path);

  CleanUpTemporaryDirectory(tempDir);
  return true;
}

// 5 Steps
bool InstallVortexVersion() {
  auto &installerData = *g_InstallerData;

  // installerData.state_n++;
  installerData.state = "Initialization...";

  std::string tempDir;
#ifdef _WIN32
  tempDir = std::filesystem::temp_directory_path().string() + "\\vx_installer";
#else
  tempDir = "/tmp/vx_installer";
#endif
  std::filesystem::create_directories(tempDir);

  if (installerData.g_UseNet) {
    installerData.state_n++;
    installerData.state = "Downloading files...";

    std::string dlpath = installerData.g_RequestTarballPath;
    std::string sumpath = installerData.g_RequestSumPath;
    std::string installPath;

#ifdef _WIN32
    installPath = installerData.g_DefaultInstallPath + "\\" + installerData.m_SelectedVortexVersion.version;
#else
    installPath = installerData.g_DefaultInstallPath + "/" + installerData.m_SelectedVortexVersion.version;
#endif

    std::string tarballFile = tempDir + "/" + dlpath.substr(dlpath.find_last_of("/\\") + 1);
    std::string sumFile = tempDir + "/" + sumpath.substr(sumpath.find_last_of("/\\") + 1);

    if (!IsCurlAvailable()) {
      std::cerr << "Error: curl is not installed on this system." << std::endl;
      return false;
    }

    if (!DownloadFile(dlpath, NormalizePath(tarballFile)) || !DownloadFile(sumpath, NormalizePath(sumFile))) {
      std::cerr << "Error: Failed to download files." << std::endl;
      installerData.result = "fail";
      installerData.state = "Error: Failed to download files.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Verifying integrity...";

    std::filesystem::current_path(tempDir);

    std::string checkSumCommand;
#ifdef _WIN32
    checkSumCommand = "CertUtil -hashfile " + tarballFile + " SHA256";
#else
    checkSumCommand = "sha256sum -c " + sumFile;
#endif
    if (system(checkSumCommand.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Integrity check failed.";
      return false;
    }
    installerData.state_n++;
    installerData.state = "Ensure clean install path...";

    std::filesystem::path path(installPath);
    try {
      std::filesystem::create_directories(path);
      std::cout << "New folder created : " << installPath << std::endl;
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "Error creating folder: " << e.what() << std::endl;
      installerData.result = "fail";
      installerData.state = "Error: Failed to prepare installation folder.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Extracting files...";

    std::string finalLink;
    std::string uncompressCommand;
#ifdef _WIN32
    uncompressCommand = "cmd /C tar -xzf \"" + tarballFile + "\" --strip-components=2 -C \"" + installPath + "\"";
#else
    finalLink = GetFinalLink(tarballFile, installPath, 2);
#endif

    if (system(uncompressCommand.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Failed to extract tarball.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Running vortex test...";

    std::string testLauncher;
#ifdef _WIN32
    testLauncher = installPath + "\\bin\\vortex.exe --test";
#else
    testLauncher = "cd \"" + finalLink + "\" && ./bin/vortex -test";
#endif
    if (system(testLauncher.c_str()) != 0) {
      installerData.result = "fail";
      installerData.state = "Error: Launcher test failed.";
      return false;
    }

    installerData.state_n++;
    installerData.state = "Installation completed successfully.";
    installerData.result = "success";
  } else {
    if (installerData.m_BuiltinLauncherExist)
      installerData.result = "fail";
    installerData.state =
        "Error: Network usage is disabled and there no builtin launcher. Cannot proceed with installation.";
  }

  CleanUpTemporaryDirectory(tempDir);
  return true;
}

void UpdateVortexLauncher() {
  auto &installerData = *g_InstallerData;

  installerData.state_n = 0;
  installerData.state = "Initialization...";

  // Rename old vortex
  std::string old_vx_path = MakeVortexLauncherFolderOld(installerData.g_DefaultInstallPath);

  // Install new vortex
  bool result = InstallVortexLauncher();

  // Delete old vortex
  if (result) {
    DeleteOldVortexLauncher(old_vx_path);
  } else {
    RevertOldVortexLauncher(old_vx_path);
  }

  installerData.finished = true;
}

Cherry::Application *Cherry::CreateApplication(int argc, char **argv) {
  Cherry::ApplicationSpecification spec;
  std::shared_ptr<Layer> layer = std::make_shared<Layer>();

  spec.Height = 500;
  spec.Width = 800;
  spec.WindowResizeable = false;
  spec.CustomTitlebar = true;
  spec.DisableWindowManagerTitleBar = true;
  spec.WindowOnlyClosable = true;
  spec.RenderMode = WindowRenderingMethod::SimpleWindow;
  spec.UniqueAppWindowName = "?loc:loc.window_names.welcome";

  spec.SetFramebarCallback([]() {
    CherryStyle::AddMarginY(10.0f);
    CherryStyle::AddMarginY(3.0f);
    if (!g_InstallerData->g_Request) {
      CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/icons/misc/icon_disconnected.png"), 15.0f, 15.0f);
    } else {
      CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/icons/misc/icon_connected.png"), 15.0f, 15.0f);
    }
    CherryStyle::RemoveMarginY(3.0f);

    CherryNextComponent.SetProperty("size_x", 50.0f);
    CherryNextComponent.SetProperty("padding_x", 3.0f);
    CherryNextComponent.SetProperty("padding_y", 3.0f);

    CherryStyle::AddMarginX(20.0f);
    CherryGUI::BeginGroup();

    static int default_selected = GetDefaultSelectedLanguage();

    auto &langSelector = CherryKit::ComboImageText(
        CherryID("language_selector"),
        "",
        {
            { "English", Cherry::GetPath("resources/imgs/icons/flags/us.png") },
            { "Français", Cherry::GetPath("resources/imgs/icons/flags/fr.png") },
            { "Español", Cherry::GetPath("resources/imgs/icons/flags/es.png") },
        },
        default_selected);

    int selectedIndex = langSelector.GetPropertyAs<int>("selected");

    if (g_InstallerData) {
      switch (selectedIndex) {
        case 0: g_InstallerData->g_SelectedLanguage = "en"; break;
        case 1: g_InstallerData->g_SelectedLanguage = "fr"; break;
        case 2: g_InstallerData->g_SelectedLanguage = "es"; break;
        default: g_InstallerData->g_SelectedLanguage = "en"; break;
      }

      if (g_InstallerData->g_SelectedLanguage != g_InstallerData->g_PreviousSelectedLanguage) {
        CherryApp.SetLocale(g_InstallerData->g_SelectedLanguage);
        g_InstallerData->g_PreviousSelectedLanguage = g_InstallerData->g_SelectedLanguage;
      }
    }

    CherryGUI::EndGroup();
  });

  if (g_InstallerData->g_Action == "install") {
    std::string name = "Installer";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon_install.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon_install.png");
  } else if (g_InstallerData->g_Action == "vxinstall") {
    std::string name = "Vortex Installer";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon.png");
  } else if (g_InstallerData->g_Action == "vxuninstall") {
    std::string name = "Vortex Uninstaller";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon.png");
  } else if (g_InstallerData->g_Action == "update") {
    std::string name = "Vortex Updater";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon_update.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon_update.png");
  } else if (g_InstallerData->g_Action == "uninstall") {
    std::string name = "Uninstaller";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon_crash.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon_crash.png");
  }

  g_InstallerData->m_InstallCallback = InstallVortexLauncher;
  g_InstallerData->m_DeleteCallback = DeleteVortexLauncher;
  g_InstallerData->m_UpdateCallback = UpdateVortexLauncher;
  g_InstallerData->m_InstallVortexCallback = InstallVortexVersion;
  g_InstallerData->m_UninstallVortexCallback = DeleteVortexVersion;

  spec.WindowSaves = false;

  Cherry::Application *app = new Cherry::Application(spec);
  app->SetFavIconPath(Cherry::GetPath("resources/imgs/favicon.png"));
  app->AddFont("Consola", Cherry::GetPath("resources/fonts/consola.ttf"), 17.0f);
  app->AddFont("ClashBold", Cherry::GetPath("resources/fonts/ClashDisplay-Semibold.ttf"), 20.0f);

  app->AddLocale("fr", Cherry::GetPath("resources/locales/fr.json"));
  app->AddLocale("en", Cherry::GetPath("resources/locales/en.json"));
  app->SetDefaultLocale("en");
  app->SetLocale("en");

  app->PushLayer(layer);
  app->SetMenubarCallback([app, layer]() {

  });

  c_Launcher = std::make_shared<Launcher>(g_InstallerData);
  return app;
}