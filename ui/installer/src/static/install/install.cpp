#include "./install.hpp"

#include <future>
#include <thread>

namespace VortexInstaller {
  void InstallAppWindow::RenderInstallVortex() {
    Space(30.0f);
    Cherry::SetNextComponentProperty("color_text", "#B1FF31");
    CherryKit::TitleThree("Welcome to Vortex !");
    CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
    CherryGUI::TextWrapped(
        "We re glad to see you here. This software is an installation wizard that can help you install the Vortex Launcher. "
        "The Vortex Launcher is a powerful tool designed to assist you in managing projects, modules, plugins, and content. "
        "After installation, you will be able to create your projects and explore all the amazing features and capabilities "
        "that Vortex provides. Enjoy your experience!");

    CherryGUI::PopStyleColor();

    Space(20.0f);
    /*auto val = std::make_shared<std::string>(m_Data->g_DefaultInstallPath);
    auto input = std::make_shared<Cherry::SimpleStringInput>("Select", val, "Select install path");

    Cherry::TitleFiveColored("Please select the installation path", "#787878FF");
    {
        input->Render("__blank");
    }*/

    CherryNextComponent.SetProperty("color_text", "#777777FF");
    CherryKit::TextWrapped("Please select the installation path");
    CherryNextComponent.SetProperty("size_x", "400.0f");
    CherryKit::InputString("", &m_Data->g_DefaultInstallPath);

    ImVec2 to_remove = CherryGUI::CalcTextSize("DenyAccept");
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);

    /*{
        auto deny = std::make_shared<Cherry::CustomButtonSimple>("Close", "Close");
        if (deny->Render())
        {
            Cherry::Application().Get().Close();
        }
    }*/

    if (CherryKit::ButtonText("Close").GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }

    CherryGUI::SameLine();

    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText("Continue").GetData("isClicked") == "true") {
      m_SelectedChildName = "Accept Licence Agreement";
      this->SetChildState("Install Vortex", true);

      CheckExistingInstallation(m_Data);
      if (m_Data->m_FolderAlreadyExist) {
        CanInstall = false;
      }
    }

    {
      /*auto accept = std::make_shared<Cherry::CustomButtonSimple>("Contdinue", "Continue");
      accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("sec"))
      {
          m_Data->g_DefaultInstallPath = input.GetData("value");
          m_SelectedChildName = "Accept Licence Agreement";
          this->SetChildState("Install Vortex", true);

          CheckExistingInstallation(m_Data);
          if (m_Data->m_FolderAlreadyExist)
          {
              CanInstall = false;
          }
      }
      CherryGUI::PopStyleColor();*/
    }
  }

  void InstallAppWindow::RenderLicenseAgreement() {
    static std::string file_content = ReadFile(Cherry::GetPath("resources/license/apache2.txt"));

    const float button_height = 30.0f;
    const float spacing = 10.0f;

    float available_height = CherryGUI::GetContentRegionAvail().y - button_height - spacing;

    CherryGUI::BeginChild("TextZone", ImVec2(0, available_height), true, ImGuiWindowFlags_HorizontalScrollbar);
    CherryGUI::TextUnformatted(file_content.c_str());
    CherryGUI::EndChild();

    ImVec2 to_remove = CherryGUI::CalcTextSize("DenyAccept");

    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);

    /*{
      auto deny = std::make_shared<Cherry::CustomButtonSimple>("Deny", "Deny");
      if (deny->Render()) {
        Cherry::Application().Get().Close();
      }
    }*/

    if (CherryKit::ButtonText("Close").GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }

    CherryGUI::SameLine();

    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText("Continue").GetData("isClicked") == "true") {
      m_SelectedChildName = "Summary";
    }

    /*{
      auto accept = std::make_shared<Cherry::CustomButtonSimple>("Accept", "Accept");
      accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("__another")) {
        m_SelectedChildName = "Summary";
      }
      CherryGUI::PopStyleColor();
    }*/
  }

  void InstallAppWindow::RenderSummary() {
    Cherry::SetNextComponentProperty("color_text", "#AAAAAAFF");
    CherryKit::TitleThree("Select your packages to install");
    ImVec2 to_remove = CherryGUI::CalcTextSize("Confirm and install !");

    /*Cherry::TitleThreeColored("Select your packages to install", "#AAAAAAFF");
     */

    CherryKit::SeparatorText("Install VortexLauncher from repository, or from this local installer");

    if (m_Data->m_BuiltinLauncherNewer && m_Data->g_Request) {
      Cherry::SetNextComponentProperty("color_text", "#555555");
      CherryKit::TextWrapped("The built-in launcher version is newer. We recommend installing it for better performance.");
    } else if (!m_Data->m_BuiltinLauncherNewer && m_Data->g_Request) {
      Cherry::SetNextComponentProperty("color_text", "#555555");
      CherryKit::TextWrapped(
          "The built-in launcher version is older. We recommend using internet to download the latest version.");
    }

    ImVec2 available_size = CherryGUI::GetContentRegionAvail();
    float button_width = (available_size.x - CherryGUI::GetStyle().ItemSpacing.x) / 2;

    if (!m_Data->g_Request) {
      m_Data->g_UseNet = false;
      CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), "Offline mode, the web installation is not available.");
    }

    if (!m_Data->m_BuiltinLauncherExist) {
      m_Data->g_UseNet = true;
      CherryGUI::TextColored(Cherry::HexToRGBA("#FF3535FF"), "No builtin launcher, you need to have internet connexion.");
    }

    if (!m_Data->m_BuiltinLauncherExist && !m_Data->g_Request) {
      m_Data->result = "fail";
      m_Data->state = "Error: Network usage is disabled and there no builtin launcher. Cannot proceed with installation.";
      m_SelectedChildName = "Installation";
    }

    {
      bool clicked = false;

      std::string label = "Download and Install from net \n(VortexLauncher " + m_Data->g_RequestVersion + ")";
      CherryGUI::PushTextWrapPos(CherryGUI::GetCursorPos().x + button_width);

      // auto btn = std::make_shared<Cherry::ImageTextButtonSimple>(label, label, Cherry::GetPath("resources/imgs/net.png"));

      if (m_Data->g_UseNet) {
        // btn->SetBorderColorIdle("#B1FF31FF");
        Cherry::SetNextComponentProperty("color_border", "#B1FF31FF");
      }

      /*if (btn->Render("__1")) {
        m_Data->g_UseNet = true;
      }*/

      if (CherryKit::ButtonImageText(label, Cherry::GetPath("resources/imgs/net.png")).GetData("isClicked") == "true") {
        m_Data->g_UseNet = true;
      }

      CherryGUI::PopTextWrapPos();
    }

    CherryGUI::SameLine();

    {
      bool clicked = false;

      std::string label = "No local Launcher to install...";

      if (m_Data->m_BuiltinLauncherExist) {
        label = "Install from this installer \n(Latest " + m_Data->m_BuiltinLauncher.platform + " w/ " +
                m_Data->m_BuiltinLauncher.arch + " v. " + m_Data->m_BuiltinLauncher.version + ")";
      }

      CherryGUI::PushTextWrapPos(CherryGUI::GetCursorPos().x + button_width);

      if (!m_Data->m_BuiltinLauncherExist) {
        CherryGUI::BeginDisabled();
      }

      // auto btn =std::make_shared<Cherry::ImageTextButtonSimple>(label, label,
      // Cherry::GetPath("resources/imgs/install.png"));

      if (!m_Data->g_UseNet && m_Data->m_BuiltinLauncherExist) {
        // btn->SetBorderColorIdle("#B1FF31FF");
        Cherry::SetNextComponentProperty("color_border", "#B1FF31FF");
      }

      if (CherryKit::ButtonImageText(label, Cherry::GetPath("resources/imgs/install.png")).GetData("isClicked") == "true") {
        m_Data->g_UseNet = false;
      }

      /*if (btn->Render()) {
        m_Data->g_UseNet = false;
      }*/

      if (!m_Data->m_BuiltinLauncherExist) {
        CherryGUI::EndDisabled();
      }
      CherryGUI::PopTextWrapPos();
    }

    CherryKit::SeparatorText("Select packages");
    CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), "Installer prebuilt packages will be available soon...");

    CherryKit::Separator();

    std::string label_install = "The installation path is : " + m_Data->g_DefaultInstallPath;
    CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), label_install.c_str());

    if (m_Data->m_FolderAlreadyExist) {
      CherryGUI::Separator();
      CherryGUI::TextColored(
          Cherry::HexToRGBA("#FF3535FF"),
          "CAUTION : The installation directory already exist. This installation will erase everythinh !");
      CherryGUI::Checkbox("Erase the folder and install Vortex", &CanInstall);
      CherryGUI::Separator();
    }

    if (!CanInstall) {
      CherryGUI::BeginDisabled();
    }

    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
    {
      // auto accept = std::make_shared<Cherry::CustomButtonSimple>("Confirm", "Confirm and install !");
      // accept->SetProperty("color_bg", "#B1FF31FF");
      // accept->SetProperty("color_bg_hovered", "#C3FF53FF");

      if (CherryKit::ButtonText("Close").GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }

      CherryGUI::SameLine();

      if (!CanInstall) {
        // accept->SetProperty("color_bg", "#353535FF");
        // accept->SetProperty("color_bg_hovered", "#353535FF");
        Cherry::SetNextComponentProperty("color_bg", "#353535FF");
        Cherry::SetNextComponentProperty("color_bg_hovered", "#353535FF");
      } else {
        Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
        Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
      }
      Cherry::SetNextComponentProperty("color_text", "#121212FF");

      if (CherryKit::ButtonText("Confirm and install !").GetData("isClicked") == "true") {
        m_SelectedChildName = "Installation";

        std::thread mainThread([this]() {
          if (m_Data->m_InstallCallback) {
            m_Data->m_InstallCallback();
          }
        });

        mainThread.detach();
      }

      /*CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("__confirm")) {
        m_SelectedChildName = "Installation";

        std::thread mainThread([this]() {
          if (m_Data->m_InstallCallback) {
            m_Data->m_InstallCallback();
          }
        });

        mainThread.detach();
      }
      CherryGUI::PopStyleColor();*/
    }

    if (!CanInstall) {
      CherryGUI::EndDisabled();
    }
  }

  void InstallAppWindow::RenderInstall() {
    float progress = static_cast<float>(m_Data->state_n) / 5.0f;
    ImVec4 progressBarColor = (m_Data->result == "success" || m_Data->result == "processing")
                                  ? Cherry::HexToRGBA("#B1FF31FF")
                                  : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);

    if (m_Data->result == "processing") {
      CherryKit::TitleTwo("Installation of Vortex Launcher");
    } else if (m_Data->result == "success") {
      CherryKit::TitleTwo("Vortex Launcher is now installed !");
    } else if (m_Data->result == "fail") {
      CherryKit::TitleTwo("Oups, an error was occured");
    }

    CherryGUI::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
    CherryGUI::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
    CherryGUI::Text(m_Data->state.c_str());
    CherryGUI::PopStyleColor();

    if (m_Data->state_n == 5) {
      ImVec2 buttonSize = CherryGUI::CalcTextSize("Finish");

      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - buttonSize.x - 50);

      Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
      Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
      Cherry::SetNextComponentProperty("color_text", "#121212FF");
      if (CherryKit::ButtonText("Finish").GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }

      /*auto accept = std::make_shared<Cherry::CustomButtonSimple>("Finish", "Finish");
      accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("__finish")) {
        Cherry::Application().Get().Close();
      }
      CherryGUI::PopStyleColor();*/
    }
  }

  void InstallAppWindow::CheckExistingInstallation(const std::shared_ptr<VortexInstallerData> &data) {
    std::string path = data->g_DefaultInstallPath;

    std::filesystem::path installPath(path);

    if (std::filesystem::exists(installPath)) {
      data->m_FolderAlreadyExist = true;
    } else {
      data->m_FolderAlreadyExist = false;
    }
  }

  InstallAppWindow::InstallAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData> &data)
      : m_Data(data) {
    m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
    m_AppWindow->SetIcon(Cherry::GetPath("resources/imgs/icons/misc/icon_home.png"));
    m_AppWindow->SetClosable(false);

    m_AppWindow->m_TabMenuCallback = []() {
      ImVec4 grayColor = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
      ImVec4 graySeparatorColor = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
      ImVec4 darkBackgroundColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
      ImVec4 lightBorderColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
      if (CherryGUI::BeginMenu("Edit")) {
        CherryGUI::PushStyleColor(ImGuiCol_Text, grayColor);
        CherryGUI::Text("Main stuff");
        CherryGUI::PopStyleColor();

        CherryGUI::PushStyleColor(ImGuiCol_Separator, graySeparatorColor);
        CherryGUI::Separator();
        CherryGUI::PopStyleColor();

        if (CherryGUI::MenuItem("Logs Utility", "Overview of all logs")) {
        }

        if (CherryGUI::MenuItem("Logs2 Utility", "Overview of all logs")) {
        }

        CherryGUI::EndMenu();
      }
    };

    m_AppWindow->SetInternalPaddingX(30.0f);
    m_AppWindow->SetInternalPaddingY(10.0f);

    m_SelectedChildName = "Install Vortex";

    this->AddChild("Installation", [this]() { RenderInstall(); });

    this->AddChild("Summary", [this]() { RenderSummary(); });

    this->AddChild("Accept Licence Agreement", [this]() { RenderLicenseAgreement(); });

    this->AddChild("Install Vortex", [this]() { RenderInstallVortex(); });

    std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
  }

  void InstallAppWindow::AddChild(const std::string &child_name, const std::function<void()> &child) {
    m_Childs[child_name] = AppChild(child, false);
  }

  void InstallAppWindow::RemoveChild(const std::string &child_name) {
    auto it = m_Childs.find(child_name);
    if (it != m_Childs.end()) {
      m_Childs.erase(it);
    }
  }

  void InstallAppWindow::SetChildState(const std::string &child_name, const bool &state) {
    auto it = m_Childs.find(child_name);
    if (it != m_Childs.end()) {
      it->second.m_Finished = state;
    }
  }

  std::shared_ptr<Cherry::AppWindow> &InstallAppWindow::GetAppWindow() {
    return m_AppWindow;
  }

  std::shared_ptr<InstallAppWindow> InstallAppWindow::Create(
      const std::string &name,
      const std::shared_ptr<VortexInstallerData> &data) {
    auto instance = std::shared_ptr<InstallAppWindow>(new InstallAppWindow(name, data));
    instance->SetupRenderCallback();
    return instance;
  }

  void InstallAppWindow::SetupRenderCallback() {
    auto self = shared_from_this();
    m_AppWindow->SetRenderCallback([self]() {
      if (self) {
        self->Render();
      }
    });
  }

  std::function<void()> InstallAppWindow::GetChild(const std::string &child_name) {
    auto it = m_Childs.find(child_name);
    if (it != m_Childs.end()) {
      return it->second.m_Foo;
    }
    return nullptr;
  }

  void InstallAppWindow::Render() {
    static float leftPaneWidth = 300.0f;
    const float minPaneWidth = 50.0f;
    const float splitterWidth = 1.5f;
    static int selected;

    CherryGUI::PushStyleColor(ImGuiCol_ChildBg, Cherry::HexToRGBA("#111111FF"));
    CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#0000000"));
    CherryGUI::BeginChild("left_pane", ImVec2(leftPaneWidth, 0), true);

    Space(25.0f);

    Cherry::SetNextComponentProperty("color_text", "#B1FF31");
    Cherry::PushFont("ClashBold");
    CherryKit::TitleThree("Vortex Installer");
    Cherry::PopFont();

    // Cherry::TitleThreeColored("Vortex Installer", "#B1FF31FF");
    Space(10.0f);

    for (const auto &child : m_Childs) {
      if (child.first == m_SelectedChildName) {
        CherryGUI::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
      } else {
        CherryGUI::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
      }
      std::string child_name;

      if (child.first.rfind("?loc:", 0) == 0) {
        std::string localeName = child.first.substr(5);
        child_name = Cherry::GetLocale(localeName) + "####" + localeName;
      } else {
        child_name = child.first;
      }

      /*Cherry::TextButtonUnderlineOptions opt;
      opt.y_margin = 5;*/

      bool activated = child.second.m_Finished;
      std::string label = "###" + child_name;

      CustomCheckbox(child_name.c_str(), &activated);
      Space(10.0f);

      CherryGUI::PopStyleColor();
    }
    CherryGUI::EndChild();
    CherryGUI::PopStyleColor(2);

    CherryGUI::SameLine();
    CherryGUI::BeginGroup();

    if (!m_SelectedChildName.empty()) {
      std::function<void()> pannel_render = GetChild(m_SelectedChildName);
      if (pannel_render) {
        pannel_render();
      }
    }

    CherryGUI::EndGroup();
  }

}  // namespace VortexInstaller
