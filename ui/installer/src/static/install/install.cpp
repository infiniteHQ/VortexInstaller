#include "./install.hpp"
#include <thread>
#include <future>

namespace VortexInstaller
{
    void InstallAppWindow::RenderInstallVortex()
    {
        Space(30.0f);
        Cherry::TitleThreeColored("Welcome to Vortex !", "#AAAAAAFF");
        ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
        ImGui::TextWrapped("We re glad to see you here. This software is an installation wizard that can help you install the Vortex Launcher. The Vortex Launcher is a powerful tool designed to assist you in managing projects, modules, plugins, and content. After installation, you will be able to create your projects and explore all the amazing features and capabilities that Vortex provides. Enjoy your experience!");

        ImGui::PopStyleColor();

        Space(20.0f);
        auto val = std::make_shared<std::string>(m_Data->g_DefaultInstallPath);
        auto input = std::make_shared<Cherry::SimpleStringInput>("Select", val, "Select install path");

        Cherry::TitleFiveColored("Please select the installation path", "#787878FF");
        {
            input->Render("__blank");
        }

        ImVec2 to_remove = ImGui::CalcTextSize("DenyAccept");
        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - to_remove.x - 50);

        {
            auto deny = std::make_shared<Cherry::CustomButtonSimple>("Close", "Close");
            if (deny->Render())
            {
                Cherry::Application().Get().Close();
            }
        }

        ImGui::SameLine();

        {
            auto accept = std::make_shared<Cherry::CustomButtonSimple>("Contdinue", "Continue");
            accept->SetProperty("bg", "#B1FF31FF");
            accept->SetProperty("bg_hovered", "#C3FF53FF");
            ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
            if (accept->Render("sec"))
            {
                m_Data->g_DefaultInstallPath = input->GetData("value");
                m_SelectedChildName = "Accept Licence Agreement";
                this->SetChildState("Install Vortex", true);

                CheckExistingInstallation(m_Data);
                if (m_Data->m_FolderAlreadyExist)
                {
                    CanInstall = false;
                }
            }
            ImGui::PopStyleColor();
        }
    }

    void InstallAppWindow::RenderLicenseAgreement()
    {
        static std::string file_content = ReadFile(Cherry::GetPath("resources/license/apache2.txt"));

        const float button_height = 30.0f;
        const float spacing = 10.0f;

        float available_height = ImGui::GetContentRegionAvail().y - button_height - spacing;

        ImGui::BeginChild("TextZone", ImVec2(0, available_height), true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(file_content.c_str());
        ImGui::EndChild();

        ImVec2 to_remove = ImGui::CalcTextSize("DenyAccept");

        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - to_remove.x - 50);

        {
            auto deny = std::make_shared<Cherry::CustomButtonSimple>("Deny", "Deny");
            if (deny->Render())
            {
                Cherry::Application().Get().Close();
            }
        }

        ImGui::SameLine();

        {
            auto accept = std::make_shared<Cherry::CustomButtonSimple>("Accept", "Accept");
            accept->SetProperty("bg", "#B1FF31FF");
            accept->SetProperty("bg_hovered", "#C3FF53FF");
            ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
            if (accept->Render("__another"))
            {
                m_SelectedChildName = "Summary";
            }
            ImGui::PopStyleColor();
        }
    }

    void InstallAppWindow::RenderSummary()
    {
        Cherry::TitleThreeColored("Select your packages to install", "#AAAAAAFF");
        ImVec2 to_remove = ImGui::CalcTextSize("Confirm and install !");

        Cherry::MenuItemTextSeparator("Install VortexLauncher from repository, or from this local installer");

        if (m_Data->m_BuiltinLauncherExist)
        {
            ImGui::TextColored(Cherry::HexToRGBA("#555555FF"), "The built-in launcher version is newer. We recommend installing it for better performance.");
        }

        ImVec2 available_size = ImGui::GetContentRegionAvail();
        float button_width = (available_size.x - ImGui::GetStyle().ItemSpacing.x) / 2;

        if (!m_Data->g_Request)
        {
            m_Data->g_UseNet = false;
            ImGui::TextColored(Cherry::HexToRGBA("#555555FF"), "Offline mode, the web installation is not available.");
        }

        if (!m_Data->m_BuiltinLauncherExist)
        {
            m_Data->g_UseNet = true;
            ImGui::TextColored(Cherry::HexToRGBA("#FF3535FF"), "No builtin launcher, you need to have internet connexion.");
        }

        if (!m_Data->m_BuiltinLauncherExist && !m_Data->g_Request)
        {
            m_Data->result = "fail";
            m_Data->state = "Error: Network usage is disabled and there no builtin launcher. Cannot proceed with installation.";
            m_SelectedChildName = "Installation";
        }

        {
            bool clicked = false;

            std::string label = "Download and Install from net \n(VortexLauncher " + m_Data->g_RequestVersion + ")";
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + button_width);

            auto btn = std::make_shared<Cherry::ImageTextButtonSimple>(label, label, Cherry::GetPath("resources/imgs/net.png"));

            if (m_Data->g_UseNet)
            {
                btn->SetBorderColorIdle("#B1FF31FF");
            }

            if (btn->Render("__1"))
            {
                m_Data->g_UseNet = true;
            }

            ImGui::PopTextWrapPos();
        }

        ImGui::SameLine();

        {
            bool clicked = false;

            std::string label = "No local Launcher to install...";

            if (m_Data->m_BuiltinLauncherExist)
            {
                label = "Install from this installer \n(Latest " + m_Data->m_BuiltinLauncher.platform + " w/ " + m_Data->m_BuiltinLauncher.arch + " v. " + m_Data->m_BuiltinLauncher.version + ")";
            }

            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + button_width);

            if (!m_Data->m_BuiltinLauncherExist)
            {
                ImGui::BeginDisabled();
            }

            auto btn = std::make_shared<Cherry::ImageTextButtonSimple>(label, label, Cherry::GetPath("resources/imgs/install.png"));

            if (!m_Data->g_UseNet && m_Data->m_BuiltinLauncherExist)
            {
                btn->SetBorderColorIdle("#B1FF31FF");
            }

            if (btn->Render())
            {
                m_Data->g_UseNet = false;
            }

            if (!m_Data->m_BuiltinLauncherExist)
            {
                ImGui::EndDisabled();
            }
            ImGui::PopTextWrapPos();
        }

        Cherry::MenuItemTextSeparator("Select packages");
        ImGui::TextColored(Cherry::HexToRGBA("#555555FF"), "Installer prebuilt packages will be available soon...");
        std::string label_install = "The installation path is : " + m_Data->g_DefaultInstallPath;
        ImGui::TextColored(Cherry::HexToRGBA("#555555FF"), label_install.c_str());

        if (m_Data->m_FolderAlreadyExist)
        {
            ImGui::Separator();
            ImGui::TextColored(Cherry::HexToRGBA("#FF3535FF"), "CAUTION : The installation directory already exist. This installation will erase everythinh !");
            ImGui::Checkbox("Erase the folder and install Vortex", &CanInstall);
            ImGui::Separator();
        }

        if (!CanInstall)
        {
            ImGui::BeginDisabled();
        }

        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - to_remove.x - 50);
        {
            auto accept = std::make_shared<Cherry::CustomButtonSimple>("Confirm", "Confirm and install !");
            accept->SetProperty("bg", "#B1FF31FF");
            accept->SetProperty("bg_hovered", "#C3FF53FF");

            if (!CanInstall)
            {
                accept->SetProperty("bg", "#353535FF");
                accept->SetProperty("bg_hovered", "#353535FF");
            }

            ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
            if (accept->Render("__confirm"))
            {
                m_SelectedChildName = "Installation";

                std::thread mainThread([this]()
                                       { 
        if (m_Data->m_InstallCallback)
        {
            m_Data->m_InstallCallback();
        } });

                mainThread.detach();
            }
            ImGui::PopStyleColor();
        }

        if (!CanInstall)
        {
            ImGui::EndDisabled();
        }
    }

    void InstallAppWindow::RenderInstall()
    {
        float progress = static_cast<float>(m_Data->state_n) / 5.0f;
        ImVec4 progressBarColor = (m_Data->result == "success" || m_Data->result == "processing")
                                      ? Cherry::HexToRGBA("#B1FF31FF")
                                      : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);
        ImGui::Text("Installation Progress:");

        if (m_Data->result == "processing")
        {
            Cherry::TitleTwo("Installation of Vortex Launcher");
        }
        else if (m_Data->result == "success")
        {
            Cherry::TitleTwo("Vortex Launcher is now installed !");
        }
        else if (m_Data->result == "fail")
        {
            Cherry::TitleTwo("Oups, an error was occured");
        }

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, progressBarColor);
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");
        ImGui::Text(m_Data->state.c_str());
        ImGui::PopStyleColor();

        if (m_Data->state_n == 5)
        {
            ImVec2 buttonSize = ImGui::CalcTextSize("Finish");

            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - buttonSize.x - 50);

            auto accept = std::make_shared<Cherry::CustomButtonSimple>("Finish", "Finish");
            accept->SetProperty("bg", "#B1FF31FF");
            accept->SetProperty("bg_hovered", "#C3FF53FF");
            ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
            if (accept->Render("__finish"))
            {
                Cherry::Application().Get().Close();
            }
            ImGui::PopStyleColor();
        }
    }

    void InstallAppWindow::CheckExistingInstallation(const std::shared_ptr<VortexInstallerData> &data)
    {
        std::string path = data->g_DefaultInstallPath;

        std::filesystem::path installPath(path);

        if (std::filesystem::exists(installPath))
        {
            data->m_FolderAlreadyExist = true;
        }
        else
        {
            data->m_FolderAlreadyExist = false;
        }
    }

    InstallAppWindow::InstallAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData> &data) : m_Data(data)
    {
        m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
        m_AppWindow->SetIcon(Cherry::GetPath("resources/imgs/icons/misc/icon_home.png"));
        m_AppWindow->SetClosable(false);

        m_AppWindow->m_TabMenuCallback = []()
        {
            ImVec4 grayColor = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
            ImVec4 graySeparatorColor = ImVec4(0.4f, 0.4f, 0.4f, 0.5f);
            ImVec4 darkBackgroundColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
            ImVec4 lightBorderColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, grayColor);
                ImGui::Text("Main stuff");
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Separator, graySeparatorColor);
                ImGui::Separator();
                ImGui::PopStyleColor();

                if (ImGui::MenuItem("Logs Utility", "Overview of all logs"))
                {
                }

                if (ImGui::MenuItem("Logs2 Utility", "Overview of all logs"))
                {
                }

                ImGui::EndMenu();
            }
        };

        m_AppWindow->SetInternalPaddingX(30.0f);
        m_AppWindow->SetInternalPaddingY(10.0f);

        m_SelectedChildName = "Install Vortex";

        this->AddChild("Installation", [this]()
                       { RenderInstall(); });

        this->AddChild("Summary", [this]()
                       { RenderSummary(); });

        this->AddChild("Accept Licence Agreement", [this]()
                       { RenderLicenseAgreement(); });

        this->AddChild("Install Vortex", [this]()
                       { RenderInstallVortex(); });

        std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
    }

    void InstallAppWindow::AddChild(const std::string &child_name, const std::function<void()> &child)
    {
        m_Childs[child_name] = AppChild(child, false);
    }

    void InstallAppWindow::RemoveChild(const std::string &child_name)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            m_Childs.erase(it);
        }
    }

    void InstallAppWindow::SetChildState(const std::string &child_name, const bool &state)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            it->second.m_Finished = state;
        }
    }

    std::shared_ptr<Cherry::AppWindow> &InstallAppWindow::GetAppWindow()
    {
        return m_AppWindow;
    }

    std::shared_ptr<InstallAppWindow> InstallAppWindow::Create(const std::string &name, const std::shared_ptr<VortexInstallerData> &data)
    {
        auto instance = std::shared_ptr<InstallAppWindow>(new InstallAppWindow(name, data));
        instance->SetupRenderCallback();
        return instance;
    }

    void InstallAppWindow::SetupRenderCallback()
    {
        auto self = shared_from_this();
        m_AppWindow->SetRenderCallback([self]()
                                       {
            if (self) {
                self->Render();
            } });
    }

    std::function<void()> InstallAppWindow::GetChild(const std::string &child_name)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            return it->second.m_Foo;
        }
        return nullptr;
    }

    void InstallAppWindow::Render()
    {
        static float leftPaneWidth = 300.0f;
        const float minPaneWidth = 50.0f;
        const float splitterWidth = 1.5f;
        static int selected;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, Cherry::HexToRGBA("#111111FF"));
        ImGui::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#0000000"));
        ImGui::BeginChild("left_pane", ImVec2(leftPaneWidth, 0), true);

        Space(25.0f);
        Cherry::TitleThreeColored("Vortex Installer", "#B1FF31FF");
        Space(10.0f);

        for (const auto &child : m_Childs)
        {
            if (child.first == m_SelectedChildName)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            std::string child_name;

            if (child.first.rfind("?loc:", 0) == 0)
            {
                std::string localeName = child.first.substr(5);
                child_name = Cherry::GetLocale(localeName) + "####" + localeName;
            }
            else
            {
                child_name = child.first;
            }

            Cherry::TextButtonUnderlineOptions opt;
            opt.y_margin = 5;

            bool activated = child.second.m_Finished;
            std::string label = "###" + child_name;

            CustomCheckbox(child_name.c_str(), &activated);
            Space(10.0f);

            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);

        ImGui::SameLine();
        ImGui::BeginGroup();

        if (!m_SelectedChildName.empty())
        {
            std::function<void()> pannel_render = GetChild(m_SelectedChildName);
            if (pannel_render)
            {
                pannel_render();
            }
        }

        ImGui::EndGroup();
    }

}
