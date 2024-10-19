#pragma once
#define CHERRY_V1
#include "../../lib/cherry/cherry.hpp"

#include "src/static/welcome/welcome.hpp"

#include <thread>
#include <memory>

using namespace VortexLauncher;

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
    welcome_window = WelcomeWindow::Create("?loc:loc.window_names.welcome");
    Cherry::AddAppWindow(welcome_window->GetAppWindow());    
    
  };


private:
  std::shared_ptr<WelcomeWindow> welcome_window;
};

static std::shared_ptr<Launcher> c_Launcher;

Cherry::Application *Cherry::CreateApplication(int argc, char **argv)
{
                        std::cout << "QD3"<< std::endl;
  Cherry::ApplicationSpecification spec;
  std::shared_ptr<Layer> layer = std::make_shared<Layer>();

  std::string name = "Vortex Installer";
  spec.Name = name;
  spec.Height = 500;
  spec.Width = 800;
  spec.WindowResizeable = false;
  spec.CustomTitlebar = true;
  spec.DisableWindowManagerTitleBar = true;
  spec.WindowOnlyClosable = true;
  spec.RenderMode = WindowRenderingMethod::SimpleWindow;
  spec.UniqueAppWindowName = "?loc:loc.window_names.welcome";

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
  app->SetMenubarCallback([app, layer]()
                          {
                            
                          });

  c_Launcher = std::make_shared<Launcher>();
  return app;
}