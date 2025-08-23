#include "./install.hpp"

#include <future>
#include <thread>

namespace VortexInstaller {
  void InstallAppWindow::RenderInstallVortex() {
    {
      float x = ImGui::GetContentRegionAvail().x;
      float y = x / 4.726f;
      CherryKit::ImageLocalCentered(Cherry::GetPath("resources/imgs/vortex_banner.png"), x, y);
      CherryStyle::AddMarginX(10.0f);
      CherryStyle::RemoveMarginY(40.0f);
      Cherry::PushFont("ClashMedium");
      CherryNextProp("color_text", "#FFFFFF");
      CherryKit::TitleOne(Cherry::GetLocale("loc.welcome"));
      Cherry::PopFont();
    }

    Space(10.0f);
    CherryNextProp("color_text", "#777777FF");
    CherryKit::TextWrapped(Cherry::GetLocale("loc.description"));

    Space(20.0f);
    /*auto val = std::make_shared<std::string>(m_Data->g_DefaultInstallPath);
    auto input = std::make_shared<Cherry::SimpleStringInput>("Select", val,
    "Select install path");

    Cherry::TitleFiveColored("Please select the installation path", "#787878FF");
    {
        input->Render("__blank");
    }*/

    CherryNextComponent.SetProperty("color_text", "#777777FF");
    CherryKit::TextWrapped("Please select the installation path");
    CherryNextComponent.SetProperty("size_x", std::to_string(CherryGUI::GetContentRegionAvail().x - 6.0f));
    CherryKit::InputString("", &m_Data->g_DefaultInstallPath);

    std::string text = CherryApp.GetLocale("loc.continue") + CherryApp.GetLocale("loc.close");
    ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
    CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
    CherryNextProp("color", "#222222");
    CherryKit::Separator();
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 40);

    CherryNextProp("color_text", "#B1FF31");
    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }

    CherryGUI::SameLine();

    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.continue")).GetData("isClicked") == "true") {
      m_SelectedChildName = "?loc:loc.child.license";
      this->SetChildState("?loc:loc.child.main", true);

      CheckExistingInstallation(m_Data);
      if (m_Data->m_FolderAlreadyExist) {
        CanInstall = false;
      }
    }

    {
      /*auto accept = std::make_shared<Cherry::CustomButtonSimple>("Contdinue",
      "Continue"); accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("sec"))
      {
          m_Data->g_DefaultInstallPath = input.GetData("value");
          m_SelectedChildName = "?loc:loc.child.license";
          this->SetChildState("?loc:loc.child.main", true);

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

    CherryGUI::PushStyleColor(ImGuiCol_ChildBg, Cherry::HexToImU32("#121212"));
    CherryGUI::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    CherryGUI::BeginChild("TextZone", ImVec2(0, available_height), false, ImGuiWindowFlags_HorizontalScrollbar);
    CherryGUI::TextUnformatted(file_content.c_str());
    CherryGUI::EndChild();
    CherryGUI::PopStyleColor();
    CherryGUI::PopStyleVar();
    /*{
      auto deny = std::make_shared<Cherry::CustomButtonSimple>("Deny", "Deny");
      if (deny->Render()) {
        Cherry::Application().Get().Close();
      }
    }*/

    std::string text = CherryApp.GetLocale("loc.accept") + CherryApp.GetLocale("loc.close");
    ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
    CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
    CherryNextProp("color", "#222222");
    CherryKit::Separator();
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 40);

    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }

    CherryGUI::SameLine();

    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.accept")).GetData("isClicked") == "true") {
      m_SelectedChildName = "?loc:loc.child.summary";
    }

    /*{
      auto accept = std::make_shared<Cherry::CustomButtonSimple>("Accept",
    "Accept"); accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("__another")) {
        m_SelectedChildName = "?loc:loc.child.summary";
      }
      CherryGUI::PopStyleColor();
    }*/
  }

  void InstallAppWindow::RenderSummary() {
    Cherry::SetNextComponentProperty("color_text", "#AAAAAAFF");
    CherryKit::TitleThree("Configure your installation");

    /*Cherry::TitleThreeColored("Select your packages to install", "#AAAAAAFF");
     */

    CherryKit::SeparatorText("Install Vortex Launcher from repository, or from this local installer");

    if (m_Data->m_BuiltinLauncherNewer && m_Data->g_Request && m_Data->m_BuiltinLauncherExist) {
      Cherry::SetNextComponentProperty("color_text", "#555555");
      CherryKit::TextWrapped(
          "The built-in launcher version is newer. We "
          "recommend installing it for better performance.");
    } else if (!m_Data->m_BuiltinLauncherNewer && m_Data->g_Request && m_Data->m_BuiltinLauncherExist) {
      Cherry::SetNextComponentProperty("color_text", "#555555");
      CherryKit::TextWrapped(
          "The built-in launcher version is older. We recommend using internet "
          "to download the latest version.");
    }

    ImVec2 available_size = CherryGUI::GetContentRegionAvail();
    float button_width = (available_size.x - CherryGUI::GetStyle().ItemSpacing.x) / 2;

    if (!m_Data->g_Request) {
      m_Data->g_UseNet = false;
      CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), "Offline mode, the web installation is not available.");
    }
    if (!m_Data->g_NetFetched && m_Data->g_Request) {
      m_Data->g_UseNet = false;
      CherryGUI::TextColored(
          Cherry::HexToRGBA("#555555FF"), "Waiting to fetch online Vortex Launcher versions, please wait.");
    }

    if (!m_Data->m_BuiltinLauncherExist) {
      m_Data->g_UseNet = true;
      CherryGUI::TextColored(Cherry::HexToRGBA("#FF3535FF"), "No builtin launcher, you need to have internet connexion.");
    }

    if (!m_Data->m_BuiltinLauncherExist && !m_Data->g_Request) {
      m_Data->result = "fail";
      m_Data->state =
          "Error: Network usage is disabled and there no builtin "
          "launcher. Cannot proceed with installation.";
      m_SelectedChildName = "?loc:loc.child.installation";
    }

    if (m_Data->g_NetFetched) {
      bool clicked = false;

      std::string label = "Download and Install from net \n(VortexLauncher " + m_Data->g_RequestVersion + ")";
      CherryGUI::PushTextWrapPos(CherryGUI::GetCursorPos().x + button_width);

      // auto btn = std::make_shared<Cherry::ImageTextButtonSimple>(label, label,
      // Cherry::GetPath("resources/imgs/net.png"));

      if (m_Data->g_UseNet) {
        // btn->SetBorderColorIdle("#B1FF31FF");
        Cherry::SetNextComponentProperty("color_border", "#B1FF31FF");
      }

      /*if (btn->Render("__1")) {
        m_Data->g_UseNet = true;
      }*/
      CherryNextComponent.SetProperty("size_x", button_width);
      if (CherryKit::ButtonImageText(label, Cherry::GetPath("resources/imgs/net.png")).GetData("isClicked") == "true") {
        m_Data->g_UseNet = true;
      }

      CherryGUI::PopTextWrapPos();
    } else {
      CherryGUI::BeginDisabled();

      std::string label = "Online version unknow";
      CherryGUI::PushTextWrapPos(CherryGUI::GetCursorPos().x + button_width);

      // auto btn = std::make_shared<Cherry::ImageTextButtonSimple>(label, label,
      // Cherry::GetPath("resources/imgs/net.png"));

      if (m_Data->g_UseNet) {
        // btn->SetBorderColorIdle("#B1FF31FF");
        Cherry::SetNextComponentProperty("color_border", "#B1FF31FF");
      }

      /*if (btn->Render("__1")) {
        m_Data->g_UseNet = true;
      }*/

      CherryNextComponent.SetProperty("size_x", button_width);
      if (CherryKit::ButtonImageText(label, Cherry::GetPath("resources/imgs/net.png")).GetData("isClicked") == "true") {
      }

      CherryGUI::PopTextWrapPos();
      CherryGUI::EndDisabled();
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

      CherryNextComponent.SetProperty("size_x", button_width);
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

    static bool package_base = true;
    static bool package_one = false;
    static bool package_two = false;

    std::vector<Cherry::Component> blocks;
    CherryNextComponent.SetRenderMode(Cherry::RenderMode::CreateOnly);
    if (package_base) {
      CherryNextComponent.SetProperty("block_border_color", "#B1FF31");
    }
    blocks.push_back(
        CherryKit::BlockVerticalCustom(
            [=]() {},
            155.0f,
            85.0f,
            {
                [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/install_banner.png"), 150, 50); },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryKit::TitleSix("Base");
                },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryStyle::RemoveMarginY(5.0f);
                  CherryStyle::PushFontSize(0.70f);
                  if (package_base) {
                    CherryNextComponent.SetProperty("color_text", "#B1FF31");
                    CherryKit::TextSimple("Selected");
                  }
                  CherryStyle::PopFontSize();
                },
            },
            1));

    CherryNextComponent.SetRenderMode(Cherry::RenderMode::CreateOnly);
    if (package_one) {
      CherryNextComponent.SetProperty("block_border_color", "#B1FF31");
    }
    blocks.push_back(
        CherryKit::BlockVerticalCustom(
            [=]() { package_one = !package_one; },
            155.0f,
            85.0f,
            {
                [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/install_banner.png"), 150, 50); },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryKit::TitleSix("System tools");
                },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryStyle::RemoveMarginY(5.0f);
                  CherryStyle::PushFontSize(0.70f);
                  if (package_one) {
                    CherryNextComponent.SetProperty("color_text", "#B1FF31");
                    CherryKit::TextSimple("Selected");
                  }
                  CherryStyle::PopFontSize();
                },
            },
            2));
    CherryNextComponent.SetRenderMode(Cherry::RenderMode::CreateOnly);
    blocks.push_back(
        CherryKit::BlockVerticalCustom(
            [=]() {},
            155.0f,
            85.0f,
            {
                [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/install_banner.png"), 150, 50); },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryKit::TitleSix("Electronic tools");
                },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryStyle::RemoveMarginY(5.0f);
                  CherryStyle::PushFontSize(0.70f);
                  CherryKit::TextSimple(":fh");
                  CherryStyle::PopFontSize();
                },
            },
            3));
    CherryNextComponent.SetRenderMode(Cherry::RenderMode::CreateOnly);
    blocks.push_back(
        CherryKit::BlockVerticalCustom(
            [=]() {},
            155.0f,
            85.0f,
            {
                [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/install_banner.png"), 150, 50); },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryKit::TitleSix("Debug and Hacking");
                },
                [=]() {
                  CherryStyle::AddMarginX(5.0f);
                  CherryStyle::RemoveMarginY(5.0f);
                  CherryStyle::PushFontSize(0.70f);
                  CherryKit::TextSimple(":fh");
                  CherryStyle::PopFontSize();
                },
            },
            4));

    CherryGUI::BeginChild("agfsgd", ImVec2(0, 150), false);
    CherryKit::GridSimple(155.0f, 155.0f, blocks);
    CherryGUI::EndChild();

    CherryKit::Separator();

    std::string label_install = "The installation path is : " + m_Data->g_DefaultInstallPath;
    CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), label_install.c_str());

    if (m_Data->m_FolderAlreadyExist) {
      CherryGUI::Separator();
      CherryGUI::TextColored(
          Cherry::HexToRGBA("#FF3535FF"),
          "CAUTION : The installation directory already "
          "exist. This installation will erase everythinh !");
      CherryGUI::Checkbox("Erase the folder and install Vortex", &CanInstall);
      CherryGUI::Separator();
    }
    std::string text = CherryApp.GetLocale("loc.confirg_install") + CherryApp.GetLocale("loc.close");
    ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
    CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
    CherryNextProp("color", "#222222");
    CherryKit::Separator();
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 40);

    {
      // auto accept = std::make_shared<Cherry::CustomButtonSimple>("Confirm",
      // "Confirm and install !"); accept->SetProperty("color_bg", "#B1FF31FF");
      // accept->SetProperty("color_bg_hovered", "#C3FF53FF");

      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }

      CherryGUI::SameLine();

      if (!CanInstall) {
        CherryGUI::BeginDisabled();
      }

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

      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.confirg_install")).GetData("isClicked") == "true") {
        m_SelectedChildName = "?loc:loc.child.installation";

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
      if (!CanInstall) {
        CherryGUI::EndDisabled();
      }
    }
  }

  void InstallAppWindow::RenderInstall() {
    float progress = static_cast<float>(m_Data->state_n) / 5.0f;
    ImVec4 progressBarColor = (m_Data->result == "success" || m_Data->result == "processing")
                                  ? Cherry::HexToRGBA("#B1FF31FF")
                                  : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);

    CherryKit::Space(40.0f);
    if (m_Data->result == "processing") {
      CherryKit::TitleTwo(CherryApp.GetLocale("loc.install.result_processing"));
    } else if (m_Data->result == "success") {
      CherryKit::TitleTwo(CherryApp.GetLocale("loc.install.result_success"));
    } else if (m_Data->result == "fail") {
      CherryKit::TitleTwo(CherryApp.GetLocale("loc.install.result_failed"));
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

      /*auto accept = std::make_shared<Cherry::CustomButtonSimple>("Finish",
      "Finish"); accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("__finish")) {
        Cherry::Application().Get().Close();
      }
      CherryGUI::PopStyleColor();*/
    }

    if (m_Data->result == "processing") {
      CherryGUI::BeginDisabled();
      std::string text = CherryApp.GetLocale("loc.close");
      ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
      CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
      CherryNextProp("color", "#222222");
      CherryKit::Separator();
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 30);

      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
        m_SelectedChildName = "?loc:loc.child.license";
        this->SetChildState("?loc:loc.child.main", true);

        CheckExistingInstallation(m_Data);
        if (m_Data->m_FolderAlreadyExist) {
          CanInstall = false;
        }
      }
      CherryGUI::EndDisabled();

    } else if (m_Data->result == "success") {
      std::string text = CherryApp.GetLocale("loc.launch_launcher") + CherryApp.GetLocale("loc.close");
      ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
      CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
      CherryNextProp("color", "#222222");
      CherryKit::Separator();
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 40);

      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }

      CherryGUI::SameLine();

      Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
      Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
      Cherry::SetNextComponentProperty("color_text", "#121212FF");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.launch_launcher")).GetData("isClicked") == "true") {
        // Start launcher
      }

    } else if (m_Data->result == "fail") {
      std::string text = CherryApp.GetLocale("loc.close");
      ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
      CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
      CherryNextProp("color", "#222222");
      CherryKit::Separator();
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 30);

      Cherry::SetNextComponentProperty("color_bg", "#ed5247");
      Cherry::SetNextComponentProperty("color_bg_hovered", "#eda49f");
      Cherry::SetNextComponentProperty("color_text", "#121212FF");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }
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

    m_SelectedChildName = "?loc:loc.child.main";

    this->AddChild("?loc:loc.child.installation", [this]() { RenderInstall(); });

    this->AddChild("?loc:loc.child.summary", [this]() { RenderSummary(); });

    this->AddChild("?loc:loc.child.license", [this]() { RenderLicenseAgreement(); });

    this->AddChild("?loc:loc.child.main", [this]() { RenderInstallVortex(); });

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
    // CherryKit::TitleThree("Vortex Installer");
    Cherry::PopFont();

    // Cherry::TitleThreeColored("Vortex Installer", "#B1FF31FF");
    Space(20.0f);

    for (const auto &child : m_Childs) {
      if (child.first == m_SelectedChildName) {
        CherryGUI::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
      } else {
        CherryGUI::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
      }
      std::string child_name;

      if (child.first.rfind("?loc:", 0) == 0) {
        std::string localeName = child.first.substr(5);
        child_name = Cherry::GetLocale(localeName);
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
