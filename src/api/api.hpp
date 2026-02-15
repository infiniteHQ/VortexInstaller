#pragma once
#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <thread>

#ifndef VORTEXINSTALLER_API
#define VORTEXINSTALLER_API

#include "../common/common.hpp"

#if (!defined(PATH_MAX) || PATH_MAX < 1024)
#undef PATH_MAX
#define PATH_MAX 1024
#endif

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFString.h>
#endif

#if defined(VXI_LOGS)
#define VXI_LOG(log) std::cout << log << std::endl;
#else
#define VXI_LOG(log)
#endif

namespace VortexInstaller {
  void CreateContext();
  std::shared_ptr<VortexInstallerData> GetContext();

  // Main installer api
  bool InstallVortexLauncher();
  void UpdateVortexLauncher();
  bool InstallVortexVersion();
  void DeleteVortexVersion();
  void DeleteVortexLauncher(const bool &vxlauncher, const bool &vx, const bool &vxdatas);
  std::string MakeVortexLauncherFolderOld(const std::string &path);
  void DeleteOldVortexLauncher(const std::string &path);
  std::string RevertOldVortexLauncher(const std::string &path);
  void PatchData();

  // Utilities
  std::string CookPath(std::string_view input_path);
#ifdef _WIN32
  std::string convertPathToWindowsStyle(const std::string &path);
#endif
  std::string GetPath(const std::string &path);
  std::string GetManifestVersion(const std::string &manifestPath);
  bool DownloadFile(const std::string &url, const std::string &outputPath);
  void CleanUpTemporaryDirectory(const std::string &tempDir);
  bool FileExists(const std::string &path);
  std::string NormalizePath(const std::string &path);
  void DetectPlatform();
  void DetectArch();
  std::string GetTopLevelDir(const std::string &tarballFile);
  std::string GetFinalLink(const std::string &tarballFile, const std::string &installPath, int strip = 1);

  bool IsSafePath(const std::filesystem::path &path);
  std::string GetUncompressCommand(const std::string &tarballFile, const std::string &installPath, int strip = 1);

  bool IsCurlAvailable();
  static void CreateFolder(const std::string &path);

  std::string ReplaceSpacesWithUnderscores(const std::string &str);
  bool CreateShortcut(
      const std::string &name,
      const std::string &targetPath,
      const std::string &shortcutPath,
      const std::string &description,
      const std::string &iconPath);

#if defined(_WIN32)
  bool InstallRedistributable(const std::wstring &filePath);
#endif  // _WIN32

  int GetDefaultSelectedLanguage();
  std::string DetectSystemLanguage();

  std::string FindManifestJson(const std::filesystem::path &startPath, int maxDepth = 10);

}  // namespace VortexInstaller

#endif  // VORTEXINSTALLER_API