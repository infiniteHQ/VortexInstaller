#include "./install_vx.hpp"
#include <thread>
#include <future>

namespace VortexInstaller
{
    void VortexInstallAppWindow::RenderInstallVortex()
    {
        Space(30.0f);
        Cherry::TitleThreeColored("Install Vortex", "#AAAAAAFF");
        ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
        ImGui::TextWrapped("Let's install a new version of Vortex");

        ImGui::PopStyleColor();

        Space(20.0f);
        std::shared_ptr<std::string> val = std::make_shared<std::string>(m_Data->g_DefaultInstallPath);

        Cherry::TitleFiveColored("Choose a pool for Vortex", "#787878FF");
        {
            static auto cp_ComboOne = std::make_shared<Cherry::ComboSimple>("combo_1", "SuperCombo", m_Data->m_VortexPools, 0);
            ImGui::SetNextItemWidth(300);
            cp_ComboOne->Render("combo_1");
            m_Data->g_DefaultInstallPath = cp_ComboOne->GetData("selected_string");
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
                m_SelectedChildName = "Accept Licence Agreement";
                this->SetChildState("Install Vortex", true);
            }
            ImGui::PopStyleColor();
        }
    }

    void VortexInstallAppWindow::RenderLicenseAgreement()
    {
        static std::string file_content = ReadFile(Cherry::GetPath("ressources/license/apache2.txt"));

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
                std::thread([this](){
                if(m_Data->m_InstallVortexCallback)
                {
                    m_Data->m_InstallVortexCallback();
                }   
                }).detach();
                m_SelectedChildName = "Installation";
            }
            ImGui::PopStyleColor();
        }
    }

    void VortexInstallAppWindow::RenderInstall()
    {
        float progress = static_cast<float>(m_Data->state_n) / 5.0f;                          
    ImVec4 progressBarColor = (m_Data->result == "success" || m_Data->result == "processing") 
                                ? Cherry::HexToRGBA("#B1FF31FF") 
                                : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);
        ImGui::Text("Installation Progress:");

        if(m_Data->result == "processing")
        {
            Cherry::TitleTwo("Installation of Vortex Launcher");
        }
        else if(m_Data->result == "success")
        {
            Cherry::TitleTwo("Vortex Launcher is now installed !");
        }
        else if(m_Data->result == "fail")
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

    VortexInstallAppWindow::VortexInstallAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData> &data) : m_Data(data)
    {
        m_AppWindow = std::make_shared<Cherry::AppWindow>(name, name);
        m_AppWindow->SetIcon(Cherry::GetPath("ressources/imgs/icons/misc/icon_home.png"));
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

        this->AddChild("Accept Licence Agreement", [this]()
                       { RenderLicenseAgreement(); });

        this->AddChild("Install Vortex", [this]()
                       { RenderInstallVortex(); });

        std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
    }

    void VortexInstallAppWindow::AddChild(const std::string &child_name, const std::function<void()> &child)
    {
        m_Childs[child_name] = VortexInstallAppChild(child, false);
    }

    void VortexInstallAppWindow::RemoveChild(const std::string &child_name)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            m_Childs.erase(it);
        }
    }

    void VortexInstallAppWindow::SetChildState(const std::string &child_name, const bool &state)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            it->second.m_Finished = state;
        }
    }

    std::shared_ptr<Cherry::AppWindow> &VortexInstallAppWindow::GetAppWindow()
    {
        return m_AppWindow;
    }

    std::shared_ptr<VortexInstallAppWindow> VortexInstallAppWindow::Create(const std::string &name, const std::shared_ptr<VortexInstallerData> &data)
    {
        auto instance = std::shared_ptr<VortexInstallAppWindow>(new VortexInstallAppWindow(name, data));
        instance->SetupRenderCallback();
        return instance;
    }

    void VortexInstallAppWindow::SetupRenderCallback()
    {
        auto self = shared_from_this();
        m_AppWindow->SetRenderCallback([self]()
                                       {
            if (self) {
                self->Render();
            } });
    }

    std::function<void()> VortexInstallAppWindow::GetChild(const std::string &child_name)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            return it->second.m_Foo;
        }
        return nullptr;
    }

    void VortexInstallAppWindow::Render()
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
