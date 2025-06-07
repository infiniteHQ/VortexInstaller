#include "./install_vx.hpp"

#include <future>
#include <thread>

namespace VortexInstaller {
  static void MyButton(const std::string &name, int w, int h) {
    ImVec2 squareSize(w, h);
    ImVec2 totalSize(squareSize.x, squareSize.y + 5);
    ImVec2 cursorPos = CherryGUI::GetCursorScreenPos();

    std::string button_id = name + "squareButtonWithText";
    if (CherryGUI::InvisibleButton(button_id.c_str(), totalSize)) {
    }

    if (CherryGUI::IsItemHovered()) {
      CherryGUI::SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    ImDrawList *drawList = CherryGUI::GetWindowDrawList();

    if (!name.empty()) {
      drawList->AddImage(
          Cherry::GetTexture(name), cursorPos, ImVec2(cursorPos.x + squareSize.x, cursorPos.y + squareSize.y));
    } else {
      drawList->AddImage(
          Cherry::GetTexture(Cherry::GetPath("resources/imgs/icons/misc/icon_vortex_default.png")),
          cursorPos,
          ImVec2(cursorPos.x + squareSize.x, cursorPos.y + squareSize.y));
    }

    ImVec2 smallRectSize(40, 20);
    ImVec2 smallRectPos(cursorPos.x + squareSize.x - smallRectSize.x - 5, cursorPos.y + squareSize.y - smallRectSize.y - 5);

    float windowVisibleX2 = CherryGUI::GetWindowPos().x + CherryGUI::GetWindowContentRegionMax().x;
    if (cursorPos.x + totalSize.x < windowVisibleX2)
      CherryGUI::SameLine();
  }

  void VortexInstallAppWindow::RenderInstallVortex() {
    Space(30.0f);

    Cherry::SetNextComponentProperty("color_text", "#FFFFFF");
    CherryKit::TitleThree("Install Vortex Editor");

    CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
    CherryGUI::TextWrapped(
        "This utility allows you to install a version of Vortex so you can open your projects and run your plugins and "
        "modules. Once installed, you will be able to launch any project for the following version:");

    if (m_Data->m_SelectedVortexVersion.version != "") {
      Cherry::SetNextComponentProperty("color_text", "#797979FF");
      CherryKit::SeparatorText("Available version :");
      {
        // LOGO Section
        CherryGUI::BeginChild("LOGO_", ImVec2(120, 40), false, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
        MyButton(Cherry::GetHttpPath(m_Data->m_SelectedVortexVersion.banner), 120, 40);
        CherryGUI::EndChild();
        CherryGUI::SameLine();
      }
      {
        // Project Info Section
        ImGuiID _id = CherryGUI::GetID("INFO_PANEL");
        CherryGUI::BeginChild(_id, ImVec2(0, 50), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
        CherryGUI::SetCursorPosY(CherryGUI::GetStyle().ItemSpacing.y);
        {
          float fontScale = 0.9f;
          float oldFontSize = CherryGUI::GetFont()->Scale;
          CherryGUI::GetFont()->Scale = fontScale;
          CherryGUI::PushFont(CherryGUI::GetFont());

          CherryGUI::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.9f), m_Data->m_SelectedVortexVersion.name.c_str());

          CherryGUI::GetFont()->Scale = oldFontSize;
          CherryGUI::PopFont();
        }

        // Space(2.0f);
        {
          float fontScale = 0.8f;
          float oldFontSize = CherryGUI::GetFont()->Scale;
          CherryGUI::GetFont()->Scale = fontScale;
          CherryGUI::PushFont(CherryGUI::GetFont());

          CherryGUI::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.5f), "Version: ");
          CherryGUI::SameLine();
          CherryGUI::TextColored(ImVec4(1.0f, 0.8f, 0.8f, 0.8f), m_Data->m_SelectedVortexVersion.version.c_str());

          CherryGUI::GetFont()->Scale = oldFontSize;
          CherryGUI::PopFont();
        }

        CherryGUI::EndChild();
      }

      CherryKit::Separator();
    }

    CherryGUI::PopStyleColor();

    Space(20.0f);
    std::shared_ptr<std::string> val = std::make_shared<std::string>(m_Data->g_DefaultInstallPath);

    CherryGUI::TextWrapped(
        "Please choose an installation location from the folders that can contain Vortex versions on your system.",
        "#787878FF");

    int selected = std::stoi(CherryKit::ComboText("", &m_Data->m_VortexPools, 0)->GetProperty("selected"));

    /*static auto cp_ComboOne = std::make_shared<Cherry::ComboSimple>("combo_1", "###SuperCombo", m_Data->m_VortexPools,
    0); CherryGUI::SetNextItemWidth(300); cp_ComboOne->Render("combo_1"); m_Data->g_DefaultInstallPath =
    cp_ComboOne->GetData("selected_string");*/

    ImVec2 to_remove = CherryGUI::CalcTextSize("DenyAccept");
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);

    /*{
      auto deny = std::make_shared<Cherry::CustomButtonSimple>("Close", "Close");
      if (deny->Render()) {
        Cherry::Application().Get().Close();
      }
    }*/

    if (CherryKit::ButtonText("Close")->GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }

    CherryGUI::SameLine();

    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText("Continue")->GetData("isClicked") == "true") {
      if (selected <= m_Data->m_VortexPools.size()) {
        m_Data->g_DefaultInstallPath = m_Data->m_VortexPools[selected];
      }

      m_SelectedChildName = "Accept Licence Agreement";
      this->SetChildState("Install Vortex", true);
    }

    /*{
      auto accept = std::make_shared<Cherry::CustomButtonSimple>("Continue", "Continue");
      accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("sec")) {
        m_SelectedChildName = "Accept Licence Agreement";
        this->SetChildState("Install Vortex", true);
      }
      CherryGUI::PopStyleColor();
    }*/
  }

  void VortexInstallAppWindow::RenderLicenseAgreement() {
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

    if (CherryKit::ButtonText("Deny")->GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }

    CherryGUI::SameLine();

    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText("Accept")->GetData("isClicked") == "true") {
      std::thread([this]() {
        if (m_Data->m_InstallVortexCallback) {
          m_Data->m_InstallVortexCallback();
        }
      }).detach();
      m_SelectedChildName = "Installation";
    }

    /*{
      auto accept = std::make_shared<Cherry::CustomButtonSimple>("Accept", "Accept");
      accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("__another")) {
        std::thread([this]() {
          if (m_Data->m_InstallVortexCallback) {
            m_Data->m_InstallVortexCallback();
          }
        }).detach();
        m_SelectedChildName = "Installation";
      }
      CherryGUI::PopStyleColor();
    }*/
  }

  void VortexInstallAppWindow::RenderInstall() {
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
      if (CherryKit::ButtonText("Finish")->GetData("isClicked") == "true") {
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

  VortexInstallAppWindow::VortexInstallAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData> &data)
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

    this->AddChild("Accept Licence Agreement", [this]() { RenderLicenseAgreement(); });

    this->AddChild("Install Vortex", [this]() { RenderInstallVortex(); });

    std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
  }

  void VortexInstallAppWindow::AddChild(const std::string &child_name, const std::function<void()> &child) {
    m_Childs[child_name] = VortexInstallAppChild(child, false);
  }

  void VortexInstallAppWindow::RemoveChild(const std::string &child_name) {
    auto it = m_Childs.find(child_name);
    if (it != m_Childs.end()) {
      m_Childs.erase(it);
    }
  }

  void VortexInstallAppWindow::SetChildState(const std::string &child_name, const bool &state) {
    auto it = m_Childs.find(child_name);
    if (it != m_Childs.end()) {
      it->second.m_Finished = state;
    }
  }

  std::shared_ptr<Cherry::AppWindow> &VortexInstallAppWindow::GetAppWindow() {
    return m_AppWindow;
  }

  std::shared_ptr<VortexInstallAppWindow> VortexInstallAppWindow::Create(
      const std::string &name,
      const std::shared_ptr<VortexInstallerData> &data) {
    auto instance = std::shared_ptr<VortexInstallAppWindow>(new VortexInstallAppWindow(name, data));
    instance->SetupRenderCallback();
    return instance;
  }

  void VortexInstallAppWindow::SetupRenderCallback() {
    auto self = shared_from_this();
    m_AppWindow->SetRenderCallback([self]() {
      if (self) {
        self->Render();
      }
    });
  }

  std::function<void()> VortexInstallAppWindow::GetChild(const std::string &child_name) {
    auto it = m_Childs.find(child_name);
    if (it != m_Childs.end()) {
      return it->second.m_Foo;
    }
    return nullptr;
  }

  void VortexInstallAppWindow::Render() {
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
