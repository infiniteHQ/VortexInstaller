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
    if (g_InstallerData->g_Action != "uninstall" && g_InstallerData->g_Action != "vxuninstall") {
      if (!g_InstallerData->g_Request) {
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

    if (g_InstallerData) {
      switch (selectedIndex) {
        case 0: g_InstallerData->g_SelectedLanguage = "en"; break;
        case 1: g_InstallerData->g_SelectedLanguage = "fr"; break;
        case 2: g_InstallerData->g_SelectedLanguage = "es"; break;
        case 3: g_InstallerData->g_SelectedLanguage = "de"; break;
        case 4: g_InstallerData->g_SelectedLanguage = "it"; break;
        case 5: g_InstallerData->g_SelectedLanguage = "pt"; break;
        case 6: g_InstallerData->g_SelectedLanguage = "sv"; break;
        case 7: g_InstallerData->g_SelectedLanguage = "fi"; break;
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

  // g_InstallerData->m_InstallCallback = InstallVortexLauncher;
  // g_InstallerData->m_DeleteCallback = DeleteVortexLauncher;
  //// g_InstallerData->m_UpdateCallback = UpdateVortexLauncher;
  // g_InstallerData->m_InstallVortexCallback = InstallVortexVersion;
  // g_InstallerData->m_UninstallVortexCallback = DeleteVortexVersion;

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

  return app;
}