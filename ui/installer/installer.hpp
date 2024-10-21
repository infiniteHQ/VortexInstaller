#pragma once
#define CHERRY_V1
#include "../../lib/cherry/cherry.hpp"

#include "src/static/install/install.hpp"
#include "src/static/update/update.hpp"

#include "../../lib/restcpp/include/restclient-cpp/restclient.h"
#include "../../src/base.hpp"

#include <thread>
#include <memory>

#include <cstdlib>    // For system() command
#include <filesystem> // For file system operations
#ifdef _WIN32
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib") // For URLDownloadToFileA
#else
#include <unistd.h> // For access() on Unix
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
      update_window = UpdateAppWindow::Create("?loc:loc.window_names.welcome");
      Cherry::AddAppWindow(update_window->GetAppWindow());
    }
  };

private:
  std::shared_ptr<InstallAppWindow> install_window;
  std::shared_ptr<UpdateAppWindow> update_window;
  std::shared_ptr<VortexInstallerData> installer_data;
};

static std::shared_ptr<Launcher> c_Launcher;

void DetectPlatform()
{
#if defined(_WIN32) || defined(_WIN64)
  g_InstallerData->g_Platform = "windows";
  g_InstallerData->g_DefaultInstallPath = "C:\\Program Files\\VortexLauncher";
#elif defined(__APPLE__) && defined(__MACH__)
  g_InstallerData->g_Platform = "macos";
  g_InstallerData->g_DefaultInstallPath = "/Applications/VortexLauncher";
#elif defined(__linux__)
  g_InstallerData->g_Platform = "linux";
  g_InstallerData->g_DefaultInstallPath = "/opt/VortexLauncher";
#elif defined(__FreeBSD__)
  g_InstallerData->g_Platform = "freebsd";
  g_InstallerData->g_DefaultInstallPath = "/opt/VortexLauncher";
#else
  g_InstallerData->g_Platform = "unknown";
  g_InstallerData->g_DefaultInstallPath = "?";
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

void UpdateVortexInstaller()
{
  // IF LOCAL
  // Get the path
  // Get the builtin VortexInstaller tarball
  // Delete old VortexInstaller
  // Install to path

  // IF DOWNLOAD
  // Get the path
  // Get latest version and assiociated path
  // Delete old VortexInstaller
  // Download latest version to temp
  // Install to path
  // Copy this VortexInstaller dist into the VortexInstaller installation path
}


// Cross-platform file existence check
bool FileExists(const std::string &path)
{
#ifdef _WIN32
  return PathFileExistsA(path.c_str());
#else
  return access(path.c_str(), F_OK) != -1;
#endif
}

// Cross-platform function to delete files (Windows vs Unix)
void DeleteFileCrossPlatform(const std::string &path)
{
#ifdef _WIN32
  DeleteFileA(path.c_str());
#else
  std::filesystem::remove_all(path);
#endif
}

// Cross-platform function to download files (using Wininet on Windows)
bool DownloadFile(const std::string &url, const std::string &outputPath)
{
#ifdef _WIN32
  HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), outputPath.c_str(), 0, NULL);
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

void InstallVortexLauncher()
{
  auto &installerData = *g_InstallerData;

  installerData.state_n = 0;
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
    installerData.state_n = 1;
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
      return;
    }

    installerData.state_n = 2;
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
      return;
    }

    installerData.state_n = 3;
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
uncompressCommand = "tar -xzf " + tarballFile + " --strip-components=1 -C " + installPath + " dist/";
#else
uncompressCommand = "tar -xzf " + tarballFile + " --strip-components=1 -C " + installPath + " dist/";
#endif
if (system(uncompressCommand.c_str()) != 0)
{
  installerData.result = "fail";
  installerData.state = "Error: Failed to extract tarball.";
  CleanUpTemporaryDirectory(tempDir);
  return;
}


    installerData.state_n = 4;
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
      return;
    }

    installerData.state_n = 5;
    installerData.state = "Installation completed successfully.";
    installerData.result = "success";
  }
  else
  {
    installerData.result = "fail";
    installerData.state = "Error: Network usage is disabled. Cannot proceed with installation.";
  }

  CleanUpTemporaryDirectory(tempDir);
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
  }

  g_InstallerData->m_InstallCallback = InstallVortexLauncher;

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

  DetectPlatform();
  DetectArch();

  std::cout << "Arch" << g_InstallerData->g_Arch << std::endl;
  std::cout << "Platform : " << g_InstallerData->g_Platform << std::endl;

  std::string dist = "stable_" + g_InstallerData->g_Platform;

  std::string url = "https://api.infinite.si/api/vortexupdates/get_vl_versions?dist=" + dist + "&arch=" + g_InstallerData->g_Arch;

  std::cout << url << std::endl;
  RestClient::Response r = RestClient::get(url);

  if (r.code != 200)
  {
    g_InstallerData->g_Request = false;
  }
  else
  {
    g_InstallerData->g_Request = true;
    try
    {
      g_InstallerData->jsonResponse = nlohmann::json::parse(r.body);

      if (!g_InstallerData->jsonResponse.empty() && g_InstallerData->jsonResponse.is_array())
      {
        // Extract the "values" field from the first element
        std::string values_str = g_InstallerData->jsonResponse[0]["values"];

        // Parse "values" string into a JSON object
        g_InstallerData->g_RequestValues = nlohmann::json::parse(values_str);

        std::cout << "Formatted JSON Values:\n"
                  << g_InstallerData->g_RequestValues.dump(4) << std::endl;

        // Ensure the keys exist and assign them to your strings
        if (g_InstallerData->g_RequestValues.contains("path") && g_InstallerData->g_RequestValues["path"].is_string())
        {
          g_InstallerData->g_RequestTarballPath = g_InstallerData->g_RequestValues["path"];
          std::cout << "Tarball Path: " << g_InstallerData->g_RequestTarballPath << std::endl;
        }
        else
        {
          std::cout << "Error: 'path' key missing or not a string" << std::endl;
        }

        if (g_InstallerData->g_RequestValues.contains("sum") && g_InstallerData->g_RequestValues["sum"].is_string())
        {
          g_InstallerData->g_RequestSumPath = g_InstallerData->g_RequestValues["sum"];
          std::cout << "Sum Path: " << g_InstallerData->g_RequestSumPath << std::endl;
        }
        else
        {
          std::cout << "Error: 'sum' key missing or not a string" << std::endl;
        }

        if (g_InstallerData->g_RequestValues.contains("version") && g_InstallerData->g_RequestValues["version"].is_string())
        {
          g_InstallerData->g_RequestVersion = g_InstallerData->g_RequestValues["version"];
          std::cout << "Version: " << g_InstallerData->g_RequestVersion << std::endl;
        }
        else
        {
          std::cout << "Error: 'version' key missing or not a string" << std::endl;
        }
      }
      else
      {
        std::cout << "Unexpected JSON format or empty response." << std::endl;
      }
    }
    catch (nlohmann::json::parse_error &e)
    {
      std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    }
  }

  if (g_InstallerData->g_Request)
  {
    std::cout << r.body << std::endl;
  }
  else
  {

    std::cout << "OUPSIII" << std::endl;
  }

  c_Launcher = std::make_shared<Launcher>(g_InstallerData);
  return app;
}