#pragma once
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

#include "../../lib/cherry/cherry.hpp"
#include "../api/api.hpp"
#include "./src/static/install/install.hpp"
#include "./src/static/install_vx/install_vx.hpp"
#include "./src/static/uninstall/uninstall.hpp"
#include "./src/static/uninstall_vx/uninstall_vx.hpp"
#include "./src/static/update/update.hpp"

class InstallerInterface {
 public:
  InstallerInterface(const std::shared_ptr<VortexInstallerData> &data) : installer_data(data) {
    if (installer_data->g_Action == "install") {
      install_window = VortexInstaller::InstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(install_window->GetAppWindow());
    } else if (installer_data->g_Action == "vxinstall") {
      installer_vx = VortexInstaller::VortexInstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(installer_vx->GetAppWindow());
    } else if (installer_data->g_Action == "vxuninstall") {
      uninstaller_vx = VortexInstaller::VortexUninstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(uninstaller_vx->GetAppWindow());
    } else if (installer_data->g_Action == "update") {
      update_window = VortexInstaller::UpdateAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(update_window->GetAppWindow());
    } else if (installer_data->g_Action == "uninstall") {
      uninstall_window = VortexInstaller::UninstallAppWindow::Create("?loc:loc.window_names.welcome", installer_data);
      Cherry::AddAppWindow(uninstall_window->GetAppWindow());
    }
  };

 private:
  std::shared_ptr<VortexInstaller::InstallAppWindow> install_window;
  std::shared_ptr<VortexInstaller::UpdateAppWindow> update_window;
  std::shared_ptr<VortexInstaller::UninstallAppWindow> uninstall_window;
  std::shared_ptr<VortexInstaller::VortexInstallAppWindow> installer_vx;
  std::shared_ptr<VortexInstaller::VortexUninstallAppWindow> uninstaller_vx;
  std::shared_ptr<VortexInstallerData> installer_data;
};

static std::shared_ptr<InstallerInterface> c_InstallerInterface;

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
    CherryStyle::AddMarginY(3.0f);
    if (VortexInstaller::GetContext()->g_Action != "uninstall" && VortexInstaller::GetContext()->g_Action != "vxuninstall") {
      if (!VortexInstaller::GetContext()->g_Request) {
        CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/icons/misc/icon_disconnected.png"), 15.0f, 15.0f);
      } else {
        CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/icons/misc/icon_connected.png"), 15.0f, 15.0f);
      }
    }

    CherryNextComponent.SetProperty("size_x", 50.0f);
    CherryNextComponent.SetProperty("padding_x", 3.0f);
    CherryNextComponent.SetProperty("padding_y", 3.0f);

    CherryStyle::AddMarginX(7.0f);
    CherryStyle::RemoveMarginY(4.0f);
    CherryGUI::BeginGroup();

    static int default_selected = VortexInstaller::GetDefaultSelectedLanguage();
    CherryNextComponent.SetProperty("disable_text", "true");

    auto &langSelector = CherryKit::ComboImageText(
        CherryID("language_selector"),
        "",
        {
            { "English", Cherry::GetPath("resources/imgs/icons/flags/us.png") },    // 0
            { "Français", Cherry::GetPath("resources/imgs/icons/flags/fr.png") },   // 1
            { "Español", Cherry::GetPath("resources/imgs/icons/flags/es.png") },    // 2
            { "Deutsch", Cherry::GetPath("resources/imgs/icons/flags/de.png") },    // 3
            { "Italiano", Cherry::GetPath("resources/imgs/icons/flags/it.png") },   // 4
            { "Português", Cherry::GetPath("resources/imgs/icons/flags/pt.png") },  // 5
            { "Svenska", Cherry::GetPath("resources/imgs/icons/flags/se.png") },    // 6
            { "Suomi", Cherry::GetPath("resources/imgs/icons/flags/fi.png") },      // 7
        },
        default_selected);

    int selectedIndex = langSelector.GetPropertyAs<int>("selected");

    if (VortexInstaller::GetContext()) {
      switch (selectedIndex) {
        case 0: VortexInstaller::GetContext()->g_SelectedLanguage = "en"; break;
        case 1: VortexInstaller::GetContext()->g_SelectedLanguage = "fr"; break;
        case 2: VortexInstaller::GetContext()->g_SelectedLanguage = "es"; break;
        case 3: VortexInstaller::GetContext()->g_SelectedLanguage = "de"; break;
        case 4: VortexInstaller::GetContext()->g_SelectedLanguage = "it"; break;
        case 5: VortexInstaller::GetContext()->g_SelectedLanguage = "pt"; break;
        case 6: VortexInstaller::GetContext()->g_SelectedLanguage = "sv"; break;
        case 7: VortexInstaller::GetContext()->g_SelectedLanguage = "fi"; break;
        default: VortexInstaller::GetContext()->g_SelectedLanguage = "en"; break;
      }

      if (VortexInstaller::GetContext()->g_SelectedLanguage != VortexInstaller::GetContext()->g_PreviousSelectedLanguage) {
        CherryApp.SetLocale(VortexInstaller::GetContext()->g_SelectedLanguage);
        VortexInstaller::GetContext()->g_PreviousSelectedLanguage = VortexInstaller::GetContext()->g_SelectedLanguage;
      }
    }
    CherryGUI::EndGroup();
  });

  if (VortexInstaller::GetContext()->g_Action == "install") {
    std::string name = "Installer";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon_install.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon_install.png");
  } else if (VortexInstaller::GetContext()->g_Action == "vxinstall") {
    std::string name = "Vortex Installer";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon.png");
  } else if (VortexInstaller::GetContext()->g_Action == "vxuninstall") {
    std::string name = "Vortex Uninstaller";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon.png");
  } else if (VortexInstaller::GetContext()->g_Action == "update") {
    std::string name = "Vortex Updater";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon_update.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon_update.png");
  } else if (VortexInstaller::GetContext()->g_Action == "uninstall") {
    std::string name = "Uninstaller";
    spec.Name = name;
    spec.IconPath = Cherry::GetPath("resources/imgs/icon_crash.png");
    spec.FavIconPath = Cherry::GetPath("resources/imgs/icon_crash.png");
  }

  // VortexInstaller::GetContext()->m_InstallCallback = InstallVortexLauncher;
  // VortexInstaller::GetContext()->m_DeleteCallback = DeleteVortexLauncher;
  //// VortexInstaller::GetContext()->m_UpdateCallback = UpdateVortexLauncher;
  // VortexInstaller::GetContext()->m_InstallVortexCallback = InstallVortexVersion;
  // VortexInstaller::GetContext()->m_UninstallVortexCallback = DeleteVortexVersion;

  spec.WindowSaves = false;

  Cherry::Application *app = new Cherry::Application(spec);
  app->SetFavIconPath(Cherry::GetPath("resources/imgs/favicon.png"));
  app->AddFont("Consola", Cherry::GetPath("resources/fonts/consola.ttf"), 17.0f);
  app->AddFont("ClashBold", Cherry::GetPath("resources/fonts/ClashDisplay-Semibold.ttf"), 20.0f);

  app->AddLocale("en", Cherry::GetPath("resources/locales/EN.json"));
  app->AddLocale("fr", Cherry::GetPath("resources/locales/FR.json"));
  app->AddLocale("es", Cherry::GetPath("resources/locales/ES.json"));
  app->AddLocale("de", Cherry::GetPath("resources/locales/DE.json"));
  app->AddLocale("it", Cherry::GetPath("resources/locales/IT.json"));
  app->AddLocale("pt", Cherry::GetPath("resources/locales/PT.json"));
  app->AddLocale("sv", Cherry::GetPath("resources/locales/SV.json"));
  app->AddLocale("fi", Cherry::GetPath("resources/locales/FI.json"));

  app->SetDefaultLocale("en");

  app->SetLocale("en");

  app->PushLayer(layer);
  app->SetMenubarCallback([app, layer]() {

  });

  c_InstallerInterface = std::make_shared<InstallerInterface>(VortexInstaller::GetContext());

  return app;
}