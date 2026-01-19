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
    {
      float x = ImGui::GetContentRegionAvail().x;
      float y = x / 4.726f;
      CherryKit::ImageLocalCentered(Cherry::GetPath("resources/imgs/update_banner.png"), x, y);
      CherryStyle::AddMarginX(10.0f);
      CherryStyle::RemoveMarginY(40.0f);
      Cherry::PushFont("ClashMedium");
      CherryNextProp("color_text", "#FFFFFF");
      CherryKit::TitleOne(Cherry::GetLocale("loc.vortex_editor_install"));
      Cherry::PopFont();
    }

    Space(10.0f);
    CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
    CherryGUI::TextWrapped(Cherry::GetLocale("loc.vxinstall.summary").c_str());

    Cherry::SetNextComponentProperty("color_text", "#797979FF");
    CherryKit::SeparatorText(Cherry::GetLocale("loc.vxinstall.available_version") + " :");
    if (m_Data->m_SelectedVortexVersion.version != "") {
      CherryKit::BlockVerticalCustom(
          [=]() {},
          155.0f,
          85.0f,
          {
              [=]() { CherryKit::ImageLocal(Cherry::GetHttpPath(m_Data->m_SelectedVortexVersion.banner), 155, 50); },
              [=]() {
                CherryStyle::AddMarginX(5.0f);
                CherryStyle::RemoveMarginY(5.0f);
                CherryKit::TitleSix(m_Data->m_SelectedVortexVersion.name);
              },
              [=]() {
                CherryStyle::AddMarginX(5.0f);
                CherryStyle::RemoveMarginY(5.0f);
                CherryStyle::PushFontSize(0.70f);
                CherryNextComponent.SetProperty("color_text", "#ABABAB");
                CherryKit::TextSimple(m_Data->m_SelectedVortexVersion.version);
                CherryGUI::SameLine();
                CherryNextComponent.SetProperty("color_text", "#565656");
                CherryKit::TextSimple("-");
                CherryGUI::SameLine();
                CherryNextComponent.SetProperty("color_text", "#9A9A9A");
                CherryKit::TextSimple(m_Data->m_SelectedVortexVersion.platform);
                CherryStyle::PopFontSize();
              },
          });
    } else {
      CherryKit::TextSimple("Loading...");
    }

    CherryGUI::PopStyleColor();

    std::string text = CherryApp.GetLocale("loc.continue") + CherryApp.GetLocale("loc.close");
    ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
    CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 100.0f);

    CherryGUI::TextWrapped(Cherry::GetLocale("loc.vxinstall.choose_target_folder").c_str(), "#787878FF");

    CherryNextComponent.SetProperty("size_x", CherryGUI::GetContentRegionAvail().x);
    int selected = std::stoi(CherryKit::ComboText("", &m_Data->m_VortexPools, 0).GetProperty("selected"));

    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);

    CherryNextProp("color", "#222222");
    CherryKit::Separator();
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 40);

    CherryNextProp("color_text", "#B1FF31");
    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }

    CherryGUI::SameLine();

    if (m_Data->m_SelectedVortexVersion.version == "") {
      CherryGUI::BeginDisabled();
    }
    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.continue")).GetData("isClicked") == "true") {
      if (selected <= m_Data->m_VortexPools.size()) {
        m_Data->g_DefaultInstallPath = m_Data->m_VortexPools[selected];
      }

      m_SelectedChildName = "?loc:loc.child.license";
      this->SetChildState("?loc:loc.child.license", true);
    }
    if (m_Data->m_SelectedVortexVersion.version == "") {
      CherryGUI::EndDisabled();
    }
  }

  void VortexInstallAppWindow::RenderLicenseAgreement() {
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

    std::string text = CherryApp.GetLocale("loc.accept") + CherryApp.GetLocale("loc.close");
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
    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.accept")).GetData("isClicked") == "true") {
      std::thread([this]() {
        if (m_Data->m_InstallVortexCallback) {
          m_Data->m_InstallVortexCallback();
        }
      }).detach();
      m_SelectedChildName = "?loc:loc.child.installation";
    }
  }

  void VortexInstallAppWindow::RenderInstall() {
    float progress = static_cast<float>(m_Data->state_n) / 5.0f;
    ImVec4 progressBarColor = (m_Data->result == "success" || m_Data->result == "processing")
                                  ? Cherry::HexToRGBA("#B1FF31FF")
                                  : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);

    CherryKit::Space(40.0f);
    if (m_Data->result == "processing") {
      CherryKit::TitleTwo(Cherry::GetLocale("loc.vxinstall.processing"));
    } else if (m_Data->result == "success") {
      CherryKit::TitleTwo(Cherry::GetLocale("loc.vxinstall.success"));
    } else if (m_Data->result == "fail") {
      CherryKit::TitleTwo(Cherry::GetLocale("loc.vxinstall.fail"));
    }

    CherryGUI::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
    CherryGUI::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
    CherryGUI::Text(m_Data->state.c_str());
    CherryGUI::PopStyleColor();

    if (m_Data->result == "processing") {
      CherryGUI::BeginDisabled();
      std::string text = CherryApp.GetLocale("loc.close");
      ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
      CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
      CherryNextProp("color", "#222222");
      CherryKit::Separator();
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 30);

      CherryNextProp("color_text", "#B1FF31");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
      }
      CherryGUI::EndDisabled();

    } else if (m_Data->result == "success") {
      std::string text = CherryApp.GetLocale("loc.close");
      ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 50);
      CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
      CherryNextProp("color", "#222222");
      CherryKit::Separator();
      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x - 40);

      Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
      Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
      Cherry::SetNextComponentProperty("color_text", "#121212FF");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
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

    m_SelectedChildName = "?loc:loc.child.installation_vortex_editor";

    this->AddChild("?loc:loc.child.installation", [this]() { RenderInstall(); });

    this->AddChild("?loc:loc.child.license", [this]() { RenderLicenseAgreement(); });

    this->AddChild("?loc:loc.child.installation_vortex_editor", [this]() { RenderInstallVortex(); });

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
    CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#00000000"));
    CherryGUI::BeginChild("left_pane", ImVec2(leftPaneWidth, 0), true);

    Space(25.0f);

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
