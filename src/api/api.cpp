#include "api.hpp"

// 5 Steps
bool VortexInstaller::InstallVortexLauncher() {
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

#if defined(_WIN32)
  std::string pathStr = CherryPath("resources/deps/VC_redist.x64.exe");

  std::wstring redistPath(pathStr.begin(), pathStr.end());

  installerData.state = "Installing Visual C++ Redistributables...";

  if (!InstallRedistributable(redistPath)) {
    installerData.result = "fail";
    installerData.state = "Failed while installing Visual C++ Redistributables...";
    return false;
  }
#endif

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

    if (std::filesystem::exists(installPath) &&
    std::filesystem::is_directory(installPath))
    {
      for (const auto &entry : std::filesystem::directory_iterator(installPath))
      {
        DeleteFileCrossPlatform(entry.path().string());
      }
    }*/

    std::string uncompressCommand;
#ifdef _WIN32
    uncompressCommand = "cmd /C \"tar -xzf \"" + tarballFile + "\" --strip-components=1 -C \"" + installPath + "\"\"";
#else
    // uncompressCommand = "tar -xzf " + tarballFile + " --strip-components=1 -C
    // " + installPath + " dist/";
    GetFinalLink(tarballFile, installPath);
#endif

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
    tarballFile = VortexInstaller::GetPath("builtin\\" + installerData.m_BuiltinLauncher.tarball);
#else
    tarballFile = VortexInstaller::GetPath("builtin/" + installerData.m_BuiltinLauncher.tarball);
#endif

#ifdef _WIN32
    sumFile = VortexInstaller::GetPath("builtin\\" + installerData.m_BuiltinLauncher.sum);
#else
    sumPath = VortexInstaller::GetPath("builtin/");
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
        std::cerr << "Error: Sum file does not exist at " << sumFile <<
    std::endl; installerData.result = "fail"; installerData.state = "Error:
    Missing sum file."; return false;
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
    // uncompressCommand = "tar -xzf " + tarballFile + " --strip-components=1 -C
    // " + installPath + " dist/";
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
    // installerData.state = "Error: Network usage is disabled. Cannot proceed
    // with installation.";
  }

  std::string installPath = installerData.g_DefaultInstallPath;
#ifdef _WIN32
  {
    std::string shortcutPath =
        "C:\\ProgramData\\Microsoft\\Windows\\Start "
        "Menu\\Programs\\Vortex Launcher.lnk";
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
    std::string shortcutPath =
        "C:\\ProgramData\\Microsoft\\Windows\\Start "
        "Menu\\Programs\\Vortex Updater.lnk";
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
    std::string shortcutPath =
        "C:\\ProgramData\\Microsoft\\Windows\\Start "
        "Menu\\Programs\\Uninstall Vortex.lnk";
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

void VortexInstaller::UpdateVortexLauncher() {
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

bool VortexInstaller::InstallVortexVersion() {
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
    installPath = installerData.g_DefaultInstallPath + "\\" + installerData.m_SelectedVortexVersion.name;
#else
    installPath = installerData.g_DefaultInstallPath + "/" + installerData.m_SelectedVortexVersion.name;
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
        "Error: Network usage is disabled and there no "
        "builtin launcher. Cannot proceed with installation.";
  }

  CleanUpTemporaryDirectory(tempDir);
  return true;
}

void VortexInstaller::DeleteVortexVersion() {
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

void VortexInstaller::DeleteVortexLauncher(const bool &vxlauncher, const bool &vx, const bool &vxdatas) {
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
        // std::cerr << "Error while deleting Vortex Launcher folder : " <<
        // e.what());
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

std::string VortexInstaller::GetManifestVersion(const std::string &manifestPath) {
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

bool VortexInstaller::DownloadFile(const std::string &url, const std::string &outputPath) {
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

void VortexInstaller::CleanUpTemporaryDirectory(const std::string &tempDir) {
  if (std::filesystem::exists(tempDir)) {
    if (!IsSafePath(tempDir)) {
      std::cerr << "Cannot delete this path: " << tempDir << std::endl;
      return;
    }

    std::string command = "rm -rf " + tempDir;
    system(command.c_str());
  }
}

bool VortexInstaller::FileExists(const std::string &path) {
  return std::filesystem::exists(path);
}

std::string VortexInstaller::NormalizePath(const std::string &path) {
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

void VortexInstaller::DetectPlatform() {
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

void VortexInstaller::DetectArch() {
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

std::string VortexInstaller::GetTopLevelDir(const std::string &tarballFile) {
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

std::string VortexInstaller::GetFinalLink(const std::string &tarballFile, const std::string &installPath, int strip) {
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

bool VortexInstaller::IsSafePath(const std::filesystem::path &path) {
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

std::string
VortexInstaller::GetUncompressCommand(const std::string &tarballFile, const std::string &installPath, int strip) {
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

bool VortexInstaller::IsCurlAvailable() {
#ifdef _WIN32
  return true;
#else
  return system("which curl >/dev/null 2>&1") == 0;
#endif
}

void VortexInstaller::CreateFolder(const std::string &path) {
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

bool VortexInstaller::CreateShortcut(
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

bool VortexInstaller::CreateShortcut(
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

std::string VortexInstaller::ReplaceSpacesWithUnderscores(const std::string &str) {
  std::string result = str;
  std::replace(result.begin(), result.end(), ' ', '_');
  return result;
}

#if defined(_WIN32)

bool VortexInstaller::InstallRedistributable(const std::wstring &filePath) {
  std::wstring arguments = L" /install /quiet /norestart";
  std::wstring commandLine = L"\"" + filePath + L"\"" + arguments;

  STARTUPINFOW si = { 0 };
  si.cb = sizeof(si);
  PROCESS_INFORMATION pi = { 0 };

  if (CreateProcessW(NULL, const_cast<LPWSTR>(commandLine.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);

      // 0: Success, 1638: Already installed, 3010: Success but reboot required
      return (exitCode == 0 || exitCode == 1638 || exitCode == 3010);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }

  return false;
}
#endif

std::string VortexInstaller::DetectSystemLanguage() {
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

int VortexInstaller::GetDefaultSelectedLanguage() {
  std::string sysLang = DetectSystemLanguage();
  std::transform(sysLang.begin(), sysLang.end(), sysLang.begin(), ::tolower);

  if (sysLang.find("fr") != std::string::npos)
    return 1;
  else if (sysLang.find("es") != std::string::npos)
    return 2;
  else if (sysLang.find("de") != std::string::npos)
    return 3;
  else if (sysLang.find("it") != std::string::npos)
    return 4;
  else if (sysLang.find("pt") != std::string::npos)
    return 5;
  else if (sysLang.find("sv") != std::string::npos)
    return 6;
  else if (sysLang.find("fi") != std::string::npos)
    return 7;

  return 0;  // en
}

std::string VortexInstaller::MakeVortexLauncherFolderOld(const std::string &path) {
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
      VXI_LOG("manifest.json found at : " + manifestPath);

      newPath = installPath.substr(0, installPath.find_last_of("/")) + "/VortexLauncherOld";

      try {
        if (std::filesystem::exists(newPath)) {
          installerData.state_n++;
          installerData.state = "Removing existing old folder...";
          VXI_LOG("Removing existing folder: " + newPath);
          std::filesystem::remove_all(newPath);
        }

        installerData.state_n++;
        installerData.state = "Rename path to old...";
        std::filesystem::rename(installPath, newPath);
        VXI_LOG("Folder renamed to : " + newPath);

      } catch (const std::filesystem::filesystem_error &e) {
        VXI_LOG("Filesystem error : " << e.what());
        newPath = "none";
      }
    } else {
      VXI_LOG("manifest.json not found in " << installPath);
    }
  } else {
    VXI_LOG("Error : This directory does not end with VortexLauncher.");
  }

  return newPath;
}

void VortexInstaller::DeleteOldVortexLauncher(const std::string &path) {
  auto &installerData = *g_InstallerData;

  if (path.empty() || path == "none") {
    VXI_LOG("DeleteOldVortexLauncher aborted: invalid path");
    return;
  }

  std::filesystem::path folderPath(path);
  std::string folderName = folderPath.filename().string();

  installerData.state_n++;
  installerData.state = "Check old vortex folder...";

  if (folderName != "VortexLauncherOld") {
    VXI_LOG("DeleteOldVortexLauncher aborted: not a VortexLauncherOld folder -> " << path);
    return;
  }

  if (!std::filesystem::exists(folderPath)) {
    VXI_LOG("DeleteOldVortexLauncher aborted: folder does not exist -> " << path);
    return;
  }

  VXI_LOG("Deleting old vortex folder: " << folderPath.string());

  try {
    installerData.state_n++;
    installerData.state = "Delete old vortex folder...";

    std::error_code ec;
    if (std::filesystem::remove_all(folderPath, ec) == static_cast<std::uintmax_t>(-1)) {
      VXI_LOG("Error during deletion: " << ec.message());
    } else {
      VXI_LOG("Successfully deleted old vortex folder");
    }

  } catch (const std::exception &e) {
    VXI_LOG("Exception while deleting folder: " << e.what());
  }
}

std::string VortexInstaller::RevertOldVortexLauncher(const std::string &path) {
  auto &installerData = *g_InstallerData;
  std::string installPath = path;

  auto endsWith = [](const std::string &str, const std::string &suffix) -> bool {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  };

  std::string originalPath = "none";

  if (endsWith(installPath, "VortexLauncherOld") || endsWith(installPath, "VortexLauncherOld/")) {
    originalPath = installPath.substr(0, installPath.find_last_of("/")) + "/VortexLauncher";

    try {
      if (std::filesystem::exists(originalPath)) {
        VXI_LOG("Removing failed installation attempt at: " + originalPath);
        std::filesystem::remove_all(originalPath);
      }

      if (std::filesystem::exists(installPath)) {
        VXI_LOG("Reverting: Renaming " + installPath + " back to " + originalPath);
        std::filesystem::rename(installPath, originalPath);
        VXI_LOG("Revert successful.");
      } else {
        VXI_LOG("Revert failed: Backup folder not found.");
      }
    } catch (const std::filesystem::filesystem_error &e) {
      VXI_LOG("Critical error during revert: " << e.what());
    }
  } else {
    VXI_LOG("Revert aborted: Path is not a backup folder.");
  }

  return originalPath;
}

std::string VortexInstaller::CookPath(std::string_view input_path) {
  static const std::string root_path = []() {
    std::string path;

#ifdef _WIN32
    char result[MAX_PATH];
    if (GetModuleFileNameA(nullptr, result, MAX_PATH)) {
      path = std::filesystem::path(result).parent_path().string();
    } else {
      std::cerr << "Failed to get root path (Windows)\n";
    }

#elif defined(__APPLE__)
    char result[PATH_MAX];
    uint32_t size = sizeof(result);
    if (_NSGetExecutablePath(result, &size) == 0) {
      path = std::filesystem::path(result).parent_path().string();
    } else {
      std::cerr << "Failed to get root path (macOS)\n";
    }

#else  // Linux
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, sizeof(result) - 1);
    if (count != -1) {
      result[count] = '\0';
      path = std::filesystem::path(result).parent_path().string();
    } else {
      std::cerr << "Failed to get root path (Linux)\n";
    }
#endif

    return path;
  }();

  return (input_path.empty() || input_path.front() == '/') ? std::string(input_path)
                                                           : root_path + "/" + std::string(input_path);
}

#ifdef _WIN32
std::string VortexInstaller::convertPathToWindowsStyle(const std::string &path) {
  std::string windowsPath = path;
  std::replace(windowsPath.begin(), windowsPath.end(), '/', '\\');
  return windowsPath;
}
#endif

std::string VortexInstaller::GetPath(const std::string &path) {
#ifdef _WIN32
  return convertPathToWindowsStyle(Application::CookPath(path));
#else
  return VortexInstaller::CookPath(path);
#endif
}

std::string VortexInstaller::FindManifestJson(const std::filesystem::path &startPath, int maxDepth) {
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