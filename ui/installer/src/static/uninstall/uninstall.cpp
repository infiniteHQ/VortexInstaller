#include "./uninstall.hpp"

#include <future>
#include <thread>

namespace VortexInstaller {
void UninstallAppWindow::RenderUninstallVortex() {
  Space(30.0f);

  Cherry::SetNextComponentProperty("color_text", "#AAAAAAFF");
  CherryKit::TitleThree(Cherry::GetLocale("loc.uninstallation"));

  CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
  CherryGUI::TextWrapped(Cherry::GetLocale("loc.uninstall.summary").c_str());

  CherryGUI::PopStyleColor();

  Space(10.0f);

  {
    CherryKit::Separator();
    CherryKit::Space(5.0f);
    CherryGUI::Checkbox(
        Cherry::GetLocale("loc.uninstall.checkbox_uninstall_launcher").c_str(),
        &this->m_DeleteVortexLauncher);
    std::string label =
        Cherry::GetLocale("loc.uninstall.checkbox_uninstall_launcher_summary") +
        " \"" + m_Data->g_DefaultInstallPath + "\"";
    CherryGUI::TextWrapped(label.c_str());
    CherryKit::Space(5.0f);
    // Cherry::MenuItemTextSeparator("");
    CherryKit::Separator();
  }

  {
    CherryKit::Space(5.0f);
    CherryGUI::Checkbox(
        Cherry::GetLocale("loc.uninstall.checkbox_uninstall_versions").c_str(),
        &this->m_DeleteVortex);
    std::string label =
        Cherry::GetLocale("loc.uninstall.checkbox_uninstall_launcher_summary") +
        " \"" + m_Data->g_VortexPath + "\"";
    CherryGUI::TextWrapped(label.c_str());
    // Cherry::MenuItemTextSeparator("");
    CherryKit::Space(5.0f);
    CherryKit::Separator();
  }

  {
    CherryKit::Space(5.0f);
    CherryGUI::Checkbox(
        Cherry::GetLocale("loc.uninstall.checkbox_uninstall_datas").c_str(),
        &this->m_DeleteVortexDatas);
    std::string label =
        Cherry::GetLocale("loc.uninstall.checkbox_uninstall_datas_summary");
    CherryGUI::TextWrapped(label.c_str());
    CherryKit::Space(5.0f);
    CherryKit::Separator();
  }

  std::string text =
      CherryApp.GetLocale("loc.continue") + CherryApp.GetLocale("loc.close");
  ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                           50);
  CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
  CherryNextProp("color", "#222222");
  CherryKit::Separator();
  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                           40);

  CherryNextProp("color_text", "#B1FF31");
  if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close"))
          .GetData("isClicked") == "true") {
    Cherry::Application().Get().Close();
  }

  CherryGUI::SameLine();

  Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
  Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
  Cherry::SetNextComponentProperty("color_text", "#121212FF");
  if (CherryKit::ButtonText(CherryApp.GetLocale("loc.continue"))
          .GetData("isClicked") == "true") {
    m_SelectedChildName = "?loc:loc.child.confirm_action";
    this->SetChildState("?loc:loc.child.confirm_action", true);
  }
}

void UninstallAppWindow::RenderConfirmAction() {
  const float button_height = 30.0f;
  const float spacing = 10.0f;

  float available_height =
      CherryGUI::GetContentRegionAvail().y - button_height - spacing;

  Space(30.0f);

  Cherry::SetNextComponentProperty("color_text", "#FA1212FF");
  CherryKit::TitleOne(Cherry::GetLocale("loc.warning"));

  if (this->m_DeleteVortexLauncher) {
    CherryKit::TitleSix(Cherry::GetLocale("loc.uninstall.launcher_deletion"));
  }

  if (this->m_DeleteVortex) {
    CherryKit::TitleSix(Cherry::GetLocale("loc.uninstall.versions_deletion"));
  }

  if (this->m_DeleteVortexDatas) {
    CherryKit::TitleSix(Cherry::GetLocale("loc.uninstall.datas_deletion"));
  }

  std::string text =
      CherryApp.GetLocale("loc.continue") + CherryApp.GetLocale("loc.close");
  ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                           50);
  CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
  CherryNextProp("color", "#222222");
  CherryKit::Separator();
  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                           40);

  CherryNextProp("color_text", "#B1FF31");
  if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close"))
          .GetData("isClicked") == "true") {
    Cherry::Application().Get().Close();
  }

  CherryGUI::SameLine();

  Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
  Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
  Cherry::SetNextComponentProperty("color_text", "#121212FF");
  if (CherryKit::ButtonText(CherryApp.GetLocale("loc.continue"))
          .GetData("isClicked") == "true") {
    m_SelectedChildName = "?loc:loc.child.uninstallation";
    std::thread mainThread([this]() {
      if (m_Data->m_DeleteCallback) {
        m_Data->m_DeleteCallback(this->m_DeleteVortexLauncher,
                                 this->m_DeleteVortex,
                                 this->m_DeleteVortexDatas);
      }
    });

    mainThread.detach();
  }
}

void UninstallAppWindow::RenderUninstallation() {
  float total = 1.0f;

  if (this->m_DeleteVortexLauncher) {
    total += 2.0f;
  }

  float progress = static_cast<float>(m_Data->state_n) / total;
  ImVec4 progressBarColor =
      (m_Data->result == "success" || m_Data->result == "processing")
          ? Cherry::HexToRGBA("#B1FF31FF")
          : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);

  if (m_Data->result == "processing") {
    CherryKit::TitleTwo(CherryApp.GetLocale("loc.uninstall.processing"));
  } else if (m_Data->result == "success") {
    CherryKit::TitleTwo(CherryApp.GetLocale("loc.uninstall.success"));
  } else if (m_Data->result == "fail") {
    CherryKit::TitleTwo(CherryApp.GetLocale("loc.uninstall.fail"));
  }

  CherryGUI::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
  CherryGUI::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
  CherryGUI::Text(m_Data->state.c_str());
  CherryGUI::PopStyleColor();

  if (m_Data->state_n == 3) {
    std::string text = CherryApp.GetLocale("loc.finish");
    ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                             50);
    CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
    CherryNextProp("color", "#222222");
    CherryKit::Separator();
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                             40);

    CherryNextProp("color_text", "#B1FF31");
    if (CherryKit::ButtonText(CherryApp.GetLocale("loc.finish"))
            .GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }
  } else {
    CherryGUI::BeginDisabled();
    std::string text = CherryApp.GetLocale("loc.finish");
    ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                             50);
    CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
    CherryNextProp("color", "#222222");
    CherryKit::Separator();
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                             40);

    CherryNextProp("color_text", "#B1FF31");
    CherryKit::ButtonText(CherryApp.GetLocale("loc.finish"));
    CherryGUI::EndDisabled();
  }
}

UninstallAppWindow::UninstallAppWindow(
    const std::string &name, const std::shared_ptr<VortexInstallerData> &data)
    : m_Data(data) {
  m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
  m_AppWindow->SetIcon(
      Cherry::GetPath("resources/imgs/icons/misc/icon_home.png"));
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

  m_SelectedChildName = "?loc:loc.child.uninstall_vortex";

  this->AddChild("?loc:loc.child.uninstallation",
                 [this]() { RenderUninstallation(); });

  this->AddChild("?loc:loc.child.confirm_action",
                 [this]() { RenderConfirmAction(); });

  this->AddChild("?loc:loc.child.uninstall_vortex",
                 [this]() { RenderUninstallVortex(); });

  std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
}

void UninstallAppWindow::AddChild(const std::string &child_name,
                                  const std::function<void()> &child) {
  m_Childs[child_name] = UninstallAppChild(child, false);
}

void UninstallAppWindow::RemoveChild(const std::string &child_name) {
  auto it = m_Childs.find(child_name);
  if (it != m_Childs.end()) {
    m_Childs.erase(it);
  }
}

void UninstallAppWindow::SetChildState(const std::string &child_name,
                                       const bool &state) {
  auto it = m_Childs.find(child_name);
  if (it != m_Childs.end()) {
    it->second.m_Finished = state;
  }
}

std::shared_ptr<Cherry::AppWindow> &UninstallAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<UninstallAppWindow>
UninstallAppWindow::Create(const std::string &name,
                           const std::shared_ptr<VortexInstallerData> &data) {
  auto instance =
      std::shared_ptr<UninstallAppWindow>(new UninstallAppWindow(name, data));
  instance->SetupRenderCallback();
  return instance;
}

void UninstallAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->Render();
    }
  });
}

std::function<void()>
UninstallAppWindow::GetChild(const std::string &child_name) {
  auto it = m_Childs.find(child_name);
  if (it != m_Childs.end()) {
    return it->second.m_Foo;
  }
  return nullptr;
}

void UninstallAppWindow::Render() {
  static float leftPaneWidth = 300.0f;
  const float minPaneWidth = 50.0f;
  const float splitterWidth = 1.5f;
  static int selected;

  CherryGUI::PushStyleColor(ImGuiCol_ChildBg, Cherry::HexToRGBA("#111111FF"));
  CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#0000000"));
  CherryGUI::BeginChild("left_pane", ImVec2(leftPaneWidth, 0), true);

  Space(25.0f);

  // Cherry::TitleThreeColored("Vortex Uninstaller", "#B1FF31FF");
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

    CustomCheckbox(child_name.c_str(), &activated, 7.0f, "#FF3423");
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

} // namespace VortexInstaller
