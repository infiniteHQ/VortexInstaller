#pragma once
#define CHERRY_V1
#include "../../lib/cherry/cherry.hpp"

#include "src/static/install/install.hpp"
#include "src/static/update/update.hpp"

#include <thread>
#include <memory>

static std::string g_DefaultPath = "/default/path/to";
static std::string g_WorkingPath = "";
static std::string g_Action = "install";

using namespace VortexInstaller;

class Layer : public Cherry::Layer
{
public:
  Layer() {};
};

class Launcher
{
public:
  Launcher()
  {
    if (g_Action == "install")
    {
      install_window = InstallAppWindow::Create("?loc:loc.window_names.welcome");
      Cherry::AddAppWindow(install_window->GetAppWindow());
    }
    else if (g_Action == "update")
    {
      update_window = UpdateAppWindow::Create("?loc:loc.window_names.welcome");
      Cherry::AddAppWindow(update_window->GetAppWindow());
    }
  };

private:
  std::shared_ptr<InstallAppWindow> install_window;
  std::shared_ptr<UpdateAppWindow> update_window;
};

static std::shared_ptr<Launcher> c_Launcher;

void FetchVortexInstallerUpdates()
{
  // Call the vortex updates api and get updates/paths
}

void InstallVortexInstaller()
{
  // IF LOCAL
  // Get the path
  // Get the builtin VortexInstaller tarball
  // Install to path

  // IF DOWNLOAD
  // Get the path
  // Get latest version and assiociated path
  // Download latest version to temp
  // Install to path
  // Copy this VortexInstaller dist into the VortexInstaller installation path
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

  if (g_Action == "install")
  {
    std::string name = "Vortex Installer";
    spec.Name = name;
  }
  else if (g_Action == "update")
  {
    std::string name = "Vortex Updater";
    spec.Name = name;
  }

  spec.WindowSaves = false;
  spec.IconPath = Cherry::GetPath("ressources/imgs/icon_update.png");
  spec.FavIconPath = Cherry::GetPath("ressources/imgs/icon_update.png");

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

  c_Launcher = std::make_shared<Launcher>();
  return app;
}