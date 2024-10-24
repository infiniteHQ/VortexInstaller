#pragma once
#define CHERRY_V1
#include "../../lib/cherry/cherry.hpp"

#include "src/static/install/install.hpp"
#include "src/static/update/update.hpp"
#include "src/static/uninstall/uninstall.hpp"

#include "../../lib/restcpp/include/restclient-cpp/restclient.h"
#include "../../lib/restcpp/include/restclient-cpp/connection.h"
#include "../../src/base.hpp"

#include <thread>
#include <memory>

#include <cstdlib>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#else
#include <unistd.h>
#endif

#if defined(VXI_LOGS) 
#define VXI_LOG(log) std::cout << log << std::endl;
#else
#define VXI_LOG(log)
#endif 

static std::shared_ptr<VortexInstallerData> g_InstallerData = nullptr;

using namespace VortexInstaller;

class Layer : public Cherry::Layer
{
public:
  Layer() {};
};

class Launcher
{
public:
  Launcher(const std::shared_ptr<VortexInstallerData> &data) : installer_data(data)
  {
    if (installer_data->g_Action == "install")
    {
      install_window = InstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(install_window->GetAppWindow());
    }
    else if (installer_data->g_Action == "update")
    {
      update_window = UpdateAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(update_window->GetAppWindow());
    }
    else if (installer_data->g_Action == "uninstall")
    {
      uninstall_window = UninstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(uninstall_window->GetAppWindow());
    }
  };

private:
  std::shared_ptr<InstallAppWindow> install_window;
  std::shared_ptr<UpdateAppWindow> update_window;
  std::shared_ptr<UninstallAppWindow> uninstall_window;
  std::shared_ptr<VortexInstallerData> installer_data;
};

static std::shared_ptr<Launcher> c_Launcher;

void DetectPlatform()
{
#if defined(_WIN32) || defined(_WIN64)
  g_InstallerData->g_Platform = "windows";
  g_InstallerData->g_DefaultInstallPath = "C:/Program Files/VortexLauncher";
  g_InstallerData->g_DefaultInstallPath = "C:/Program Files/Vortex";
  g_InstallerData->g_VortexDataPath = GetHomeDirectory() + "/.vx";
#elif defined(__APPLE__) && defined(__MACH__)
  g_InstallerData->g_Platform = "macos";
  g_InstallerData->g_DefaultInstallPath = "/Applications/VortexLauncher";
  g_InstallerData->g_VortexPath = "/Applications/Vortex";
  g_InstallerData->g_VortexDataPath = GetHomeDirectory() + "/.vx";
#elif defined(__linux__)
  g_InstallerData->g_Platform = "linux";
  g_InstallerData->g_DefaultInstallPath = "/opt/VortexLauncher";
  g_InstallerData->g_VortexPath = "/opt/Vortex";
  g_InstallerData->g_VortexDataPath = GetHomeDirectory() + "/.vx";
#elif defined(__FreeBSD__)
  g_InstallerData->g_Platform = "freebsd";
  g_InstallerData->g_DefaultInstallPath = "/opt/VortexLauncher";
  g_InstallerData->g_VortexPath = "/opt/Vortex";
  g_InstallerData->g_VortexDataPath = GetHomeDirectory() + "/.vx";
#else
  g_InstallerData->g_Platform = "unknown";
  g_InstallerData->g_DefaultInstallPath = "?";
  g_InstallerData->g_VortexPath = "?";
  g_InstallerData->g_VortexDataPath = "?";
#endif
}

void DetectArch()
{
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

bool FileExists(const std::string &path)
{
    return std::filesystem::exists(path);
}

void DeleteFileCrossPlatform(const std::string &path)
{
#ifdef _WIN32
  DeleteFileA(path.c_str());
#else
  std::filesystem::remove_all(path);
#endif
}

bool DownloadFile(const std::string &url, const std::string &outputPath)
{
#ifdef _WIN32
    HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), outputPath.c_str(), 0, NULL);
    if (hr != S_OK) {
      VXI_LOG("url : " + url);
       // std::cerr << "Error downloading file: " << hr); // Affiche l'erreur
    }
    return hr == S_OK;
#else
    std::string downloadCommand = "curl -o " + outputPath + " " + url;
    return system(downloadCommand.c_str()) == 0;
#endif
}

void CleanUpTemporaryDirectory(const std::string &tempDir)
{
  if (std::filesystem::exists(tempDir))
  {
    DeleteFileCrossPlatform(tempDir);
  }
}

std::string getManifestVersion(const std::string &manifestPath)
{
  std::ifstream manifestFile(manifestPath);
  if (!manifestFile.is_open())
  {
    //std::cerr << "Failed to open " << manifestPath);
    return "";
  }

  nlohmann::json manifestJson;
  try
  {
    manifestFile >> manifestJson;
  }
  catch (const std::exception &e)
  {
    //std::cerr << "Failed to parse JSON: " << e.what());
    return "";
  }

  manifestFile.close();

  if (manifestJson.contains("version") && manifestJson["version"].is_string())
  {
    return manifestJson["version"];
  }
  else
  {
    //std::cerr << "\"version\" field not found or not a string");
    return "";
  }
}
std::string RevertOldVortexLauncher(const std::string &path)
{
  auto &installerData = *g_InstallerData;
  std::string installPath = path;
  std::string manifestPath = installPath + "/manifest.json";

  auto endsWith = [](const std::string &str, const std::string &suffix) -> bool
  {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  };

  std::string newPath = "none";


  if (endsWith(installPath, "VortexLauncherOld") || endsWith(installPath, "VortexLauncherOld/"))
  {
    if (std::filesystem::exists(manifestPath))
    {
      VXI_LOG("manifest.json finded at : " + manifestPath);

      newPath = installPath.substr(0, installPath.find_last_of("/")) + "/VortexLauncher";

      try
      {
        std::filesystem::rename(installPath, newPath);
      }
      catch (const std::filesystem::filesystem_error &e)
      {
        //std::cerr << "Error while renaming : " << e.what());
      }
    }
    else
    {
      //std::cerr << "Error : manifest.json not found in " << installPath);
    }
  }
  else
  {
    //std::cerr << "Error : This directory not end with VortexLauncher or VortexLauncher/.");
  }

  return newPath;
}

// 3 Steps
std::string MakeVortexLauncherFolderOld(const std::string &path)
{
  auto &installerData = *g_InstallerData;
  std::string installPath = path;
  std::string manifestPath = installPath + "/manifest.json";

  auto endsWith = [](const std::string &str, const std::string &suffix) -> bool
  {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  };

  std::string newPath = "none";

  installerData.state_n++;
  installerData.state = "Checking path...";

  if (endsWith(installPath, "VortexLauncher") || endsWith(installPath, "VortexLauncher/"))
  {

    if (std::filesystem::exists(manifestPath))
    {

      installerData.state_n++;
      installerData.state = "Checking if path exist...";

      VXI_LOG("manifest.json finded at : " + manifestPath);

      newPath = installPath.substr(0, installPath.find_last_of("/")) + "/VortexLauncherOld";

      try
      {
        installerData.state_n++;
        installerData.state = "Rename path to old...";
        std::filesystem::rename(installPath, newPath);
        VXI_LOG("Folder renamed to : " + newPath);
      }
      catch (const std::filesystem::filesystem_error &e)
      {
       VXI_LOG("Error while renaming folder : " << e.what());
      }
    }
    else
    {
      VXI_LOG("manifest.json not found " << installPath);
    }
  }
  else
  {
    VXI_LOG("Error : This directory not end with VortexLauncher or VortexLauncher/.");
  }

  return newPath;
}

// 2 Check
void DeleteOldVortexLauncher(const std::string &path)
{
  auto &installerData = *g_InstallerData;
  std::string installPath = path;
  std::string manifestPath = installPath + "/manifest.json";

        installerData.state_n++;
        installerData.state = "Check old vortex folder...";
  if (std::filesystem::exists(manifestPath))
  {
    VXI_LOG("Found manifest.json at: " + manifestPath);
    VXI_LOG("Deleting folder: " + installPath);
    try
    {
        installerData.state_n++;
        installerData.state = "Delete old vortex folder...";
      std::filesystem::remove_all(installPath);
      VXI_LOG("Successfully deleted the folder: "<< installPath);
    }
    catch (const std::filesystem::filesystem_error &e)
    {
      VXI_LOG("Error deleting the folder: " << e.what());
    }
  }
  else
  {
    VXI_LOG("Error: manifest.json not found in " << installPath);
  }
}

void DeleteVortexLauncher(const bool &vxlauncher, const bool &vx, const bool &vxdatas)
{
  auto &installerData = *g_InstallerData;
  bool failed = false;

  DetectPlatform();

  auto isValidPath = [](const std::string &path) -> bool
  {
    return !path.empty() && path != "/" && std::filesystem::exists(path);
  };

  if (vxlauncher)
  {
    std::string installPath = installerData.g_WorkingPath;
    std::string manifestPath = installPath + "/manifest.json";

    installerData.state_n++;
    installerData.state = "Vérification du Vortex Launcher...";
    if (std::filesystem::exists(manifestPath))
    {
      VXI_LOG("Fichier manifest.json finded at : " << manifestPath);
      VXI_LOG("Deleting folder : " << installPath);

      installerData.state_n++;
      installerData.state = "Deleting Vortex Launcher...";
      try
      {
        if (isValidPath(installPath))
        {
          std::filesystem::remove_all(installPath);
          VXI_LOG("Folder deleted : " << installPath);
        }
        else
        {
          throw std::filesystem::filesystem_error("Chemin invalide", std::error_code());
        }
      }
      catch (const std::filesystem::filesystem_error &e)
      {
        installerData.result = "fail";
        failed = true;
        installerData.state = "Error while deleting the Vortex Launcher folder";
      }
    }
    else
    {
      //std::cerr << "manifest.json not found " << installPath);
      installerData.result = "fail";
      failed = true;
      installerData.state = "Error: Path to Vortex Launcher invalid !";
    }
  }

  if (vx)
  {
    std::string installPath = installerData.g_VortexPath;
    installerData.state_n++;
    installerData.state = "Deleting Vortex...";

    if (isValidPath(installPath))
    {
      try
      {
        std::filesystem::remove_all(installPath);
        VXI_LOG("Deleted with success :  " << installPath);
      }
      catch (const std::filesystem::filesystem_error &e)
      {
        //std::cerr << "Error while deleting Vortex Launcher folder : " << e.what());
        installerData.result = "fail";
        failed = true;
        installerData.state = "Failed while deleting the Vortex Launcher folder !";
      }
    }
    else
    {
      //std::cerr << "Vortex Launcher not found at path : " << installPath);
      installerData.result = "fail";
      failed = true;
      installerData.state = "Failed ! Vortex Launcher path is invalid !";
    }
  }

  if (vxdatas)
  {
    std::string installPath = installerData.g_VortexDataPath;
    installerData.state_n++;
    installerData.state = "Deleting Vortex datas...";

    if (isValidPath(installPath))
    {
      try
      {
        std::filesystem::remove_all(installPath);
        VXI_LOG("Deleted with success :  " << installPath);
      }
      catch (const std::filesystem::filesystem_error &e)
      {
        installerData.result = "fail";
        failed = true;
        installerData.state = "Failed while deleting Vortex datas !";
      }
    }
    else
    {
      installerData.result = "fail";
      failed = true;
      installerData.state = "Path to Vortex datas is invalid";
    }
  }

  if (!failed)
  {
    installerData.state_n++;
    installerData.state = "Deleted succefully !";
    installerData.result = "success";
  }
}

// 5 Steps
bool InstallVortexLauncher()
{
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

  if (installerData.g_UseNet)
  {
    installerData.state_n++;
    installerData.state = "Downloading files...";

    std::string dlpath = installerData.g_RequestTarballPath;
    std::string sumpath = installerData.g_RequestSumPath;
    std::string installPath = installerData.g_DefaultInstallPath;


    std::string tarballFile = tempDir + "/" + dlpath.substr(dlpath.find_last_of("/\\") + 1);
    std::string sumFile = tempDir + "/" + sumpath.substr(sumpath.find_last_of("/\\") + 1);

    if (!DownloadFile(dlpath, tarballFile) || !DownloadFile(sumpath, sumFile))
    {
      installerData.result = "fail";
      installerData.state = "Error: Failed to download files.";
      CleanUpTemporaryDirectory(tempDir);
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
    if (system(checkSumCommand.c_str()) != 0)
    {
      installerData.result = "fail";
      installerData.state = "Error: Integrity check failed.";
      CleanUpTemporaryDirectory(tempDir);
      return false;
    }

    installerData.state_n++;
    installerData.state = "Extracting files...";

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
    }

std::string uncompressCommand;
#ifdef _WIN32
    uncompressCommand = "cmd /C \"\"tar\" -xzf \"" + tarballFile +
                        "\" --strip-components=1 -C \"" + installPath + "\" dist/\"";
#else
    uncompressCommand = "tar -xzf " + tarballFile + " --strip-components=1 -C " + installPath + " dist/";
#endif

    if (system(uncompressCommand.c_str()) != 0)
    {
      installerData.result = "fail";
      installerData.state = "Error: Failed to extract tarball.";
      CleanUpTemporaryDirectory(tempDir);
      return false;
    }

    installerData.state_n++;
    installerData.state = "Running vortex_launcher test...";

    std::string testLauncher;
#ifdef _WIN32
    testLauncher = installPath + "\\bin\\vortex_launcher.exe --test";
#else
    testLauncher = installPath + "/bin/vortex_launcher --test";
#endif
    if (system(testLauncher.c_str()) != 0)
    {
      installerData.result = "fail";
      installerData.state = "Error: Launcher test failed.";
      CleanUpTemporaryDirectory(tempDir);
      return false;
    }

    installerData.state_n++;
    installerData.state = "Installation completed successfully.";
    installerData.result = "success";
  }
  else
  {
    installerData.result = "fail";
    installerData.state = "Error: Network usage is disabled. Cannot proceed with installation.";
  }

  CleanUpTemporaryDirectory(tempDir);
  return true;
}

void UpdateVortexLauncher()
{
  auto &installerData = *g_InstallerData;

  installerData.state_n = 0;
  installerData.state = "Initialization...";

  // Rename old vortex
  std::string old_vx_path = MakeVortexLauncherFolderOld(installerData.g_DefaultInstallPath);

  // Install new vortex
  bool result = InstallVortexLauncher();

  // Delete old vortex
  if (result)
  {
    DeleteOldVortexLauncher(old_vx_path);
  }
  else
  {
    RevertOldVortexLauncher(old_vx_path);
  }
}

Cherry::Application *Cherry::CreateApplication(int argc, char **argv)
{
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

  if (g_InstallerData->g_Action == "install")
  {
    std::string name = "Installer";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("ressources/imgs/icon_install.png");
    spec.FavIconPath = Cherry::GetPath("ressources/imgs/icon_install.png");
  }
  else if (g_InstallerData->g_Action == "update")
  {
    std::string name = "Vortex Updater";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("ressources/imgs/icon_update.png");
    spec.FavIconPath = Cherry::GetPath("ressources/imgs/icon_update.png");
  }
  else if (g_InstallerData->g_Action == "uninstall")
  {
    std::string name = "Uninstaller";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("ressources/imgs/icon_crash.png");
    spec.FavIconPath = Cherry::GetPath("ressources/imgs/icon_crash.png");
  }

  g_InstallerData->m_InstallCallback = InstallVortexLauncher;
  g_InstallerData->m_DeleteCallback = DeleteVortexLauncher;
  g_InstallerData->m_UpdateCallback = UpdateVortexLauncher;

  spec.WindowSaves = false;

  Cherry::Application *app = new Cherry::Application(spec);
  app->SetFavIconPath(Cherry::GetPath("ressources/imgs/favicon.png"));
  app->AddFont("Consola", Cherry::GetPath("ressources/fonts/consola.ttf"), 17.0f);

  app->AddLocale("fr", Cherry::GetPath("ressources/locales/fr.json"));
  app->AddLocale("en", Cherry::GetPath("ressources/locales/en.json"));
  app->SetDefaultLocale("en");
  app->SetLocale("fr");

  app->PushLayer(layer);
  app->SetMenubarCallback([app, layer]() {

  });

  c_Launcher = std::make_shared<Launcher>(g_InstallerData);
  return app;
}