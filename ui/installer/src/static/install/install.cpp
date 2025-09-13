#include "./install.hpp"

#include <future>
#include <thread>

namespace VortexInstaller {

  static bool ShowPackagesSelection = false;

  void InstallAppWindow::RenderInstallVortex() {
    {
      float x = ImGui::GetContentRegionAvail().x;
      float y = x / 4.726f;
      CherryKit::ImageLocalCentered(Cherry::GetPath("resources/imgs/vx_banner.png"), x, y);
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

    CherryNextComponent.SetProperty("color_text", "#777777FF");
    CherryKit::TextWrapped(Cherry::GetLocale("loc.install.install_path"));
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
      m_SelectedChildName = "?loc:loc.child.summary";
    }
  }

  void InstallAppWindow::RenderSummary() {
    Cherry::SetNextComponentProperty("color_text", "#AAAAAAFF");
    CherryKit::TitleThree("Configure your installation");
    CherryGUI::SameLine();

    CherryNextComponent.SetProperty("padding_y", 3.0f);
    CherryNextComponent.SetProperty("padding_x", 3.0f);

    if (CherryKit::ButtonImage(Cherry::GetPath("resources/imgs/icons/misc/icon_settings.png"))
            .GetDataAs<bool>("isClicked")) {
      ShowPackagesSelection = !ShowPackagesSelection;
    }

    CherryKit::SeparatorText(Cherry::GetLocale("loc.install.install_summary"));

    if (m_Data->m_BuiltinLauncherNewer && m_Data->g_Request && m_Data->m_BuiltinLauncherExist) {
      Cherry::SetNextComponentProperty("color_text", "#555555");
      CherryKit::TextWrapped(Cherry::GetLocale("loc.install.builder_is_newer"));
    } else if (!m_Data->m_BuiltinLauncherNewer && m_Data->g_Request && m_Data->m_BuiltinLauncherExist) {
      Cherry::SetNextComponentProperty("color_text", "#555555");
      CherryKit::TextWrapped(Cherry::GetLocale("loc.install.builder_is_older"));
    }

    ImVec2 available_size = CherryGUI::GetContentRegionAvail();
    float button_width = (available_size.x - CherryGUI::GetStyle().ItemSpacing.x) / 2 - 42.0f;

    if (!m_Data->g_Request) {
      m_Data->g_UseNet = false;
      CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), Cherry::GetLocale("loc.install.offline_mode").c_str());
    }
    if (!m_Data->g_NetFetched && m_Data->g_Request) {
      m_Data->g_UseNet = false;
      CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), Cherry::GetLocale("loc.install.wait_fetch").c_str());
    }

    if (!m_Data->m_BuiltinLauncherExist && !m_Data->g_Request) {
      m_Data->g_UseNet = true;
      CherryGUI::TextColored(Cherry::HexToRGBA("#FF3535FF"), Cherry::GetLocale("loc.install.no_builtin").c_str());
    }

    if (!m_Data->m_BuiltinLauncherExist && !m_Data->g_Request) {
      m_Data->result = "fail";
      m_Data->state = Cherry::GetLocale("loc.error.no_internet_and_builtin");
      m_SelectedChildName = "?loc:loc.child.installation";
    }

    if (m_Data->g_NetFetched) {
      bool clicked = false;

      std::string label =
          Cherry::GetLocale("loc.install.net_download") + "\n(VortexLauncher " + m_Data->g_RequestVersion + ")";
      // CherryGUI::PushTextWrapPos(CherryGUI::GetCursorPos().x + button_width);

      // auto btn = std::make_shared<Cherry::ImageTextButtonSimple>(label, label,
      // Cherry::GetPath("resources/imgs/net.png"));

      /*if (m_Data->g_UseNet) {
        // btn->SetBorderColorIdle("#B1FF31FF");
        Cherry::SetNextComponentProperty("color_border", "#B1FF31FF");
      }*/

      /*if (btn->Render("__1")) {
        m_Data->g_UseNet = true;
      }*/
      /*CherryNextComponent.SetProperty("size_x", button_width);
      if (CherryKit::ButtonImageText(label,
      Cherry::GetPath("resources/imgs/net.png")).GetData("isClicked") == "true") {
        m_Data->g_UseNet = true;
      }*/

      if (m_Data->g_UseNet) {
        CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#B1FF31FF"));
      } else {
        CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#343434"));
      }

      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#CDCDCD"));
      CherryGUI::PushStyleColor(ImGuiCol_Button, Cherry::HexToRGBA("#232323"));
      CherryGUI::PushStyleColor(ImGuiCol_ButtonHovered, Cherry::HexToRGBA("#343434"));
      CherryGUI::PushStyleColor(ImGuiCol_ButtonActive, Cherry::HexToRGBA("#454545"));

      CherryStyle::AddMarginX(6.0f);
      if (CherryGUI::ImageSizeButtonWithText(
              Cherry::GetTexture(Cherry::GetPath("resources/imgs/net.png")),
              button_width,
              label.c_str(),
              ImVec2(-FLT_MIN, 0.0f),
              ImVec2(0, 0),
              ImVec2(1, 1),
              -1,
              ImVec4(0, 0, 0, 0),
              ImVec4(1, 1, 1, 1))) {
        m_Data->g_UseNet = true;
      }
      CherryGUI::PopStyleColor(5);

      CherryGUI::PopTextWrapPos();
    } else {
      CherryGUI::BeginDisabled();

      std::string label = "Online version unknow";
      CherryGUI::PushTextWrapPos(CherryGUI::GetCursorPos().x + button_width);

      // auto btn = std::make_shared<Cherry::ImageTextButtonSimple>(label, label,
      // Cherry::GetPath("resources/imgs/net.png"));

      /*if (m_Data->g_UseNet) {
        // btn->SetBorderColorIdle("#B1FF31FF");
        Cherry::SetNextComponentProperty("color_border", "#B1FF31FF");
      }

      if (btn->Render("__1")) {
        m_Data->g_UseNet = true;
      }

      CherryNextComponent.SetProperty("size_x", button_width);
      if (CherryKit::ButtonImageText(label,
      Cherry::GetPath("resources/imgs/net.png")).GetData("isClicked") == "true") {
      }*/

      if (m_Data->g_UseNet) {
        CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#B1FF31FF"));
      } else {
        CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#343434"));
      }

      CherryGUI::PushStyleColor(ImGuiCol_Button, Cherry::HexToRGBA("#232323"));
      CherryGUI::PushStyleColor(ImGuiCol_ButtonHovered, Cherry::HexToRGBA("#343434"));
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#CDCDCD"));
      CherryGUI::PushStyleColor(ImGuiCol_ButtonActive, Cherry::HexToRGBA("#454545"));

      CherryStyle::AddMarginX(6.0f);
      if (CherryGUI::ImageSizeButtonWithText(
              Cherry::GetTexture(Cherry::GetPath("resources/imgs/net.png")),
              button_width,
              label.c_str(),
              ImVec2(-FLT_MIN, 0.0f),
              ImVec2(0, 0),
              ImVec2(1, 1),
              -1,
              ImVec4(0, 0, 0, 0),
              ImVec4(1, 1, 1, 1))) {
        m_Data->g_UseNet = true;
      }
      CherryGUI::PopStyleColor(5);

      CherryGUI::PopTextWrapPos();
      CherryGUI::EndDisabled();
    }

    CherryGUI::SameLine();

    {
      bool clicked = false;

      std::string label = Cherry::GetLocale("loc.install.no_local_launcher");

      if (m_Data->m_BuiltinLauncherExist) {
        label = Cherry::GetLocale("loc.install.builtin_install") + "\n(Latest " + m_Data->m_BuiltinLauncher.platform +
                " w/ " + m_Data->m_BuiltinLauncher.arch + " v. " + m_Data->m_BuiltinLauncher.version + ")";
      }

      CherryGUI::PushTextWrapPos(CherryGUI::GetCursorPos().x + button_width);

      if (!m_Data->m_BuiltinLauncherExist) {
        CherryGUI::BeginDisabled();
      }

      if (!m_Data->g_UseNet && m_Data->m_BuiltinLauncherExist) {
        CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#B1FF31FF"));
      } else {
        CherryGUI::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#343434"));
      }

      CherryGUI::PushStyleColor(ImGuiCol_Button, Cherry::HexToRGBA("#232323"));
      CherryGUI::PushStyleColor(ImGuiCol_ButtonHovered, Cherry::HexToRGBA("#343434"));
      CherryGUI::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#CDCDCD"));
      CherryGUI::PushStyleColor(ImGuiCol_ButtonActive, Cherry::HexToRGBA("#454545"));

      CherryStyle::AddMarginX(6.0f);
      if (CherryGUI::ImageSizeButtonWithText(
              Cherry::GetTexture(Cherry::GetPath("resources/imgs/install.png")),
              button_width,
              label.c_str(),
              ImVec2(-FLT_MIN, 0.0f),
              ImVec2(0, 0),
              ImVec2(1, 1),
              -1,
              ImVec4(0, 0, 0, 0),
              ImVec4(1, 1, 1, 1))) {
        m_Data->g_UseNet = false;
      }
      CherryGUI::PopStyleColor(5);

      if (!m_Data->m_BuiltinLauncherExist) {
        CherryGUI::EndDisabled();
      }
      CherryGUI::PopTextWrapPos();
    }

    if (ShowPackagesSelection) {
      CherryKit::SeparatorText(Cherry::GetLocale("loc.install.select_packages"));

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
                  [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/package_base.png"), 155, 50); },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryKit::TitleSix("Base");
                  },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryStyle::PushFontSize(0.70f);
                    if (package_base) {
                      CherryNextComponent.SetProperty("color_text", "#B1FF31");
                      CherryKit::TextSimple(Cherry::GetLocale("loc.required"));
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
                  [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/package_sys.png"), 155, 50); },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryNextComponent.SetProperty("color_text", "#898989");
                    CherryKit::TitleSix("System development");
                  },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryStyle::PushFontSize(0.70f);

                    CherryNextComponent.SetProperty("color_text", "#FFB11188");
                    CherryKit::TextSimple(Cherry::GetLocale("loc.available_soon"));

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
                  [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/package_tools.png"), 155, 50); },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryNextComponent.SetProperty("color_text", "#898989");
                    CherryKit::TitleSix("Debug and tools");
                  },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryStyle::PushFontSize(0.70f);

                    CherryNextComponent.SetProperty("color_text", "#FFB11188");
                    CherryKit::TextSimple(Cherry::GetLocale("loc.available_soon"));

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
                  [=]() { CherryKit::ImageLocal(Cherry::GetPath("resources/imgs/package_em.png"), 155, 50); },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryNextComponent.SetProperty("color_text", "#898989");
                    CherryKit::TitleSix("Embedded dev");
                  },
                  [=]() {
                    CherryStyle::AddMarginX(5.0f);
                    CherryStyle::RemoveMarginY(5.0f);
                    CherryStyle::PushFontSize(0.70f);

                    CherryNextComponent.SetProperty("color_text", "#FFB11188");
                    CherryKit::TextSimple(Cherry::GetLocale("loc.available_soon"));

                    CherryStyle::PopFontSize();
                  },
              },
              4));
      CherryNextComponent.SetRenderMode(Cherry::RenderMode::CreateOnly);
      blocks.push_back(
          CherryKit::BlockVerticalCustom(
              [=]() {},
              155.0f,
              85.0f,
              {
                  [=]() {
                    CherryStyle::AddMarginY(40.0f);
                    CherryKit::TextCenter(Cherry::GetLocale("loc.comming_soon"));
                  },
              },
              5));
      CherryNextComponent.SetRenderMode(Cherry::RenderMode::CreateOnly);
      blocks.push_back(
          CherryKit::BlockVerticalCustom(
              [=]() {},
              155.0f,
              85.0f,
              {
                  [=]() {
                    CherryStyle::AddMarginY(40.0f);
                    CherryKit::TextCenter(Cherry::GetLocale("loc.comming_soon"));
                  },
              },
              6));

      CherryGUI::BeginChild("agfsgd", ImVec2(0, 150), false);
      CherryKit::GridSimple(155.0f, 155.0f, blocks);
      CherryGUI::EndChild();
    }

    CherryKit::Separator();

    if (m_Data->m_FolderAlreadyExist) {
      std::string label_install = Cherry::GetLocale("loc.install.install_path_is") + " : " + m_Data->g_DefaultInstallPath;
      CherryGUI::TextColored(Cherry::HexToRGBA("#555555FF"), label_install.c_str());
      CherryGUI::Separator();
      CherryGUI::TextColored(
          Cherry::HexToRGBA("#FF3535FF"), Cherry::GetLocale("loc.install.install_path_already_exist").c_str());
      CherryGUI::Checkbox(Cherry::GetLocale("loc.install.erase_and_install").c_str(), &CanInstall);
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
      CherryNextProp("color_text", "#B1FF31");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }

      CherryGUI::SameLine();

      if (!CanInstall) {
        CherryGUI::BeginDisabled();
      }

      if (!CanInstall) {
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
      ImVec2 buttonSize = CherryGUI::CalcTextSize(CherryApp.GetLocale("loc.finish").c_str());

      CherryGUI::SetCursorPosX(CherryGUI::GetContentRegionMax().x - buttonSize.x - 50);

      Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
      Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
      Cherry::SetNextComponentProperty("color_text", "#121212FF");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.finish")).GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }
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

      CherryNextProp("color_text", "#B1FF31");
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

      CherryNextProp("color_text", "#B1FF31");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.close")).GetData("isClicked") == "true") {
        Cherry::Application().Get().Close();
      }

      CherryGUI::SameLine();

      Cherry::SetNextComponentProperty("color_bg", "#B1FF31FF");
      Cherry::SetNextComponentProperty("color_bg_hovered", "#C3FF53FF");
      Cherry::SetNextComponentProperty("color_text", "#121212FF");
      if (CherryKit::ButtonText(CherryApp.GetLocale("loc.launch_launcher")).GetData("isClicked") == "true") {
        // Start launcher
        std::thread([=]() {
          std::string installPath = m_Data->g_DefaultInstallPath;

#if defined(_WIN32)
          std::string cmd = "\"" + installPath + "\\bin\\vortex_launcher.exe\"";
#else
          std::string cmd = installPath + "/bin/vortex_launcher";
#endif

          std::system(cmd.c_str());
        }).detach();

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
