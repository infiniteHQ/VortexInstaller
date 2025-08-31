#include "./update.hpp"

#include <future>
#include <thread>

namespace VortexInstaller {

void UpdateAppWindow::RenderUpdateVortex() {
  {
    float x = ImGui::GetContentRegionAvail().x;
    float y = x / 4.726f;
    CherryKit::ImageLocalCentered(
        Cherry::GetPath("resources/imgs/vortex_banner.png"), x, y);
    CherryStyle::AddMarginX(10.0f);
    CherryStyle::RemoveMarginY(40.0f);
    Cherry::PushFont("ClashMedium");
    CherryNextProp("color_text", "#FFFFFF");
    if (!m_Data->g_Request) {
      CherryKit::TitleOne(CherryApp.GetLocale("loc.no_internet"));
    } else {
      if (!m_Data->g_VortexLauncherOutdated) {
        CherryKit::TitleOne(CherryApp.GetLocale("loc.update.is_updated"));
      } else {
        CherryKit::TitleOne(CherryApp.GetLocale("loc.update.can_update"));
      }
    }

    Cherry::PopFont();
  }

  CherryKit::Space(20.0f);

  if (!m_Data->g_Request) {
    CherryKit::TextWrapped(
        CherryApp.GetLocale("loc.update.explain_no_internet"));
  } else {
    if (!m_Data->g_VortexLauncherOutdated) {
      CherryKit::TextWrapped(
          CherryApp.GetLocale("loc.update.explain_is_updated"));
    } else {
      CherryKit::TextWrapped(
          CherryApp.GetLocale("loc.update.explain_can_update"));
    }
  }

  /*{
    std::string label = "Current version : " + m_Data->g_ManifestVersion;
    CherryGUI::Text(label.c_str());
  }*/

  std::string text =
      CherryApp.GetLocale("loc.update") + CherryApp.GetLocale("loc.close");

  if (!m_Data->g_VortexLauncherOutdated) {
    text = CherryApp.GetLocale("loc.close");
  } else {
    text = CherryApp.GetLocale("loc.update") + CherryApp.GetLocale("loc.close");
  }

  ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
  CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 52.0f);
  Cherry::SetNextComponentProperty("color_text", "#767676");
  CherryKit::TextSimple("Distribution : " + m_Data->g_Distribution);

  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
                           50);
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

  if (!m_Data->g_VortexLauncherOutdated) {
    CherryGUI::BeginDisabled();
  }

  if (m_Data->g_Request) {
    if (m_Data->g_VortexLauncherOutdated) {
      Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
      Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
      Cherry::SetNextComponentProperty("color_text", "#121212FF");
      if (CherryKit::ButtonText("Update").GetData("isClicked") == "true") {
        m_SelectedChildName = "?loc:loc.child.update";

        std::thread mainThread([this]() {
          if (m_Data->m_UpdateCallback) {
            m_Data->m_UpdateCallback();
          }
        });

        mainThread.detach();
      }
    }
  }

  if (!m_Data->g_VortexLauncherOutdated) {
    CherryGUI::EndDisabled();
  }
  /*std::string label = "Update now !";
  ImVec2 to_remove = CherryGUI::CalcTextSize(label.c_str());

  if (m_Data->g_VortexLauncherOutdated) {
    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
50);
    /*{
      auto accept = std::make_shared<Cherry::CustomButtonSimple>(label.c_str(),
label.c_str()); accept->SetProperty("color_bg", "#B1FF31FF");
      accept->SetProperty("color_bg_hovered", "#C3FF53FF");
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
      if (accept->Render("__confirm")) {
        m_SelectedChildName = "Update";

        std::thread mainThread([this]() {
          if (m_Data->m_UpdateCallback) {
            m_Data->m_UpdateCallback();
          }
        });

        mainThread.detach();
      }
      CherryGUI::PopStyleColor();
    }*

  Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
  Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
  Cherry::SetNextComponentProperty("color_text", "#121212FF");
  if (CherryKit::ButtonText(label).GetData("isClicked") == "true") {
    m_SelectedChildName = "Update";

    std::thread mainThread([this]() {
      if (m_Data->m_UpdateCallback) {
        m_Data->m_UpdateCallback();
      }
    });

    mainThread.detach();
  }
}
else {
  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
50);
  /*{
    auto accept = std::make_shared<Cherry::CustomButtonSimple>("Confirm",
"Close"); accept->SetProperty("color_bg", "#B1FF31FF");
    accept->SetProperty("color_bg_hovered", "#C3FF53FF");
    CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
    if (accept->Render("__confirm")) {
      Cherry::Application::Get().Close();
    }
    CherryGUI::PopStyleColor();
  }*
  std::string text = CherryApp.GetLocale("loc.quit");
  ImVec2 to_remove = CherryGUI::CalcTextSize(text.c_str());
  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
50); CherryGUI::SetCursorPosY(CherryGUI::GetContentRegionMax().y - 35.0f);
  CherryNextProp("color", "#222222");
  CherryKit::Separator();
  CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - to_remove.x -
40);

  CherryNextProp("color_text", "#B1FF31");
  if
(CherryKit::ButtonText(CherryApp.GetLocale("loc.quit")).GetData("isClicked") ==
"true") { Cherry::Application::Get().Close();
  }
}
*/
}

void UpdateAppWindow::RenderUpdate() {
  float progress = static_cast<float>(m_Data->state_n) / 10.0f;
  ImVec4 progressBarColor =
      (m_Data->result == "success" || m_Data->result == "processing")
          ? Cherry::HexToRGBA("#B1FF31FF")
          : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);

  if (m_Data->result == "processing") {
    CherryKit::TitleTwo(Cherry::GetLocale("loc.update.result_processing"));
  } else if (m_Data->result == "success") {
    CherryKit::TitleTwo(Cherry::GetLocale("loc.update.result_success"));
  } else if (m_Data->result == "fail") {
    CherryKit::TitleTwo(Cherry::GetLocale("loc.update.result_failed"));
  }

  CherryGUI::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
  CherryGUI::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
  CherryGUI::Text(m_Data->state.c_str());
  CherryGUI::PopStyleColor();

  if (m_Data->finished) {
    ImVec2 buttonSize = CherryGUI::CalcTextSize("Finish");

    CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - buttonSize.x -
                             50);

    /*auto accept = std::make_shared<Cherry::CustomButtonSimple>("Finish",
    "Finish"); accept->SetProperty("color_bg", "#B1FF31FF");
    accept->SetProperty("color_bg_hovered", "#C3FF53FF");
    CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
    if (accept->Render("__finish")) {
      Cherry::Application().Get().Close();
    }
    CherryGUI::PopStyleColor();*/

    Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
    Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
    Cherry::SetNextComponentProperty("color_text", "#121212FF");
    if (CherryKit::ButtonText(Cherry::GetLocale("loc.finish"))
            .GetData("isClicked") == "true") {
      Cherry::Application().Get().Close();
    }
  }
}

UpdateAppWindow::UpdateAppWindow(
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

  m_SelectedChildName = "?loc:loc.child.update_vortex";

  this->AddChild("?loc:loc.child.update", [this]() { RenderUpdate(); });

  this->AddChild("?loc:loc.child.update_vortex",
                 [this]() { RenderUpdateVortex(); });

  std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
}

void UpdateAppWindow::AddChild(const std::string &child_name,
                               const std::function<void()> &child) {
  m_Childs[child_name] = UpdateAppChild(child, false);
}

void UpdateAppWindow::RemoveChild(const std::string &child_name) {
  auto it = m_Childs.find(child_name);
  if (it != m_Childs.end()) {
    m_Childs.erase(it);
  }
}

void UpdateAppWindow::SetChildState(const std::string &child_name,
                                    const bool &state) {
  auto it = m_Childs.find(child_name);
  if (it != m_Childs.end()) {
    it->second.m_Finished = state;
  }
}

std::shared_ptr<Cherry::AppWindow> &UpdateAppWindow::GetAppWindow() {
  return m_AppWindow;
}

std::shared_ptr<UpdateAppWindow>
UpdateAppWindow::Create(const std::string &name,
                        const std::shared_ptr<VortexInstallerData> &data) {
  auto instance =
      std::shared_ptr<UpdateAppWindow>(new UpdateAppWindow(name, data));
  instance->SetupRenderCallback();
  return instance;
}

void UpdateAppWindow::SetupRenderCallback() {
  auto self = shared_from_this();
  m_AppWindow->SetRenderCallback([self]() {
    if (self) {
      self->Render();
    }
  });
}

std::function<void()> UpdateAppWindow::GetChild(const std::string &child_name) {
  auto it = m_Childs.find(child_name);
  if (it != m_Childs.end()) {
    return it->second.m_Foo;
  }
  return nullptr;
}

void UpdateAppWindow::Render() {
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
  Cherry::PopFont();

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

} // namespace VortexInstaller
