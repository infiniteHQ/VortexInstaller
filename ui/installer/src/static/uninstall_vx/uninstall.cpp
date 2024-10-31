#include "./uninstall.hpp"
#include <thread>
#include <future>

namespace VortexInstaller
{
    void UninstallAppWindow::RenderUninstallVortex()
    {
        Space(30.0f);
        Cherry::TitleThreeColored("Vortex uninstallation wizard", "#AAAAAAFF");
        ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
        ImGui::TextWrapped("This software will help you to uninstall vortex (and optionnally Vortex local data).");

        ImGui::PopStyleColor();

        Space(20.0f);

        {
            ImGui::Checkbox("Uninstall Vortex Launcher", &this->m_DeleteVortexLauncher);
            std::string label = "This action will delete the current Vortex Launcher regitered at \"" + m_Data->g_DefaultInstallPath + "\"";
            ImGui::Text(label.c_str());
            Cherry::MenuItemTextSeparator("");
        }

        {
            ImGui::Checkbox("Uninstall Vortex versions", &this->m_DeleteVortex);
            std::string label = "This action will delete the current Vortex versions installed and regitered by default at \"/opt/Vortex/...\"";
            ImGui::Text(label.c_str());
            Cherry::MenuItemTextSeparator("");
        }

        {
            ImGui::Checkbox("Uninstall Vortex datas", &this->m_DeleteVortexDatas);
            std::string label = "This action will delete all data from your .vx folder (WARNING: this will delete all modules, plugins, contents & configs registered here.)";
            ImGui::Text(label.c_str());
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
                m_SelectedChildName = "Confirm action";
                this->SetChildState("Confirm action", true);
            }
            ImGui::PopStyleColor();
        }
    }

    void UninstallAppWindow::RenderConfirmAction()
    {
        const float button_height = 30.0f;
        const float spacing = 10.0f;

        float available_height = ImGui::GetContentRegionAvail().y - button_height - spacing;

        Space(30.0f);

        Cherry::TitleOneColored("WARNING!", "#FA1212FF");
        if (this->m_DeleteVortexLauncher)
        {
            Cherry::TitleSix("This launcher will be deleted.");
        }

        if (this->m_DeleteVortex)
        {
            Cherry::TitleSix("All versions of Vortex will be deleted.");
        }

        if (this->m_DeleteVortexDatas)
        {
            Cherry::TitleSix("All datas of Vortex will be deleted.");
        }

        ImVec2 to_remove = ImGui::CalcTextSize("DenyAccept");

        ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - to_remove.x - 50);

        {
            auto deny = std::make_shared<Cherry::CustomButtonSimple>("Back", "Back");
            if (deny->Render())
            {
                m_SelectedChildName = "Uninstall Vortex";
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
                m_SelectedChildName = "Uninstallation";
                std::thread mainThread([this]()
                                       { 
        if (m_Data->m_DeleteCallback)
        {
            m_Data->m_DeleteCallback(this->m_DeleteVortexLauncher, this->m_DeleteVortex, this->m_DeleteVortexDatas);
        } });

                mainThread.detach();
            }
            ImGui::PopStyleColor();
        }
    }

    void UninstallAppWindow::RenderUninstallation()
    {  
        float total = 1.0f;

        if(this->m_DeleteVortexLauncher)
        {
            total += 2.0f;
        }
        

        float progress = static_cast<float>(m_Data->state_n) /total;                          
    ImVec4 progressBarColor = (m_Data->result == "success" || m_Data->result == "processing") 
                                ? Cherry::HexToRGBA("#B1FF31FF") 
                                : ImVec4(0.8f, 0.18f, 0.18f, 1.0f);
        ImGui::Text("Installation Progress:");

        if(m_Data->result == "processing")
        {
            Cherry::TitleTwo("Uninstallation...");
        }
        else if(m_Data->result == "success")
        {
            Cherry::TitleTwo("Uninstallation well done !");
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

    UninstallAppWindow::UninstallAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData> &data) : m_Data(data)
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

        m_SelectedChildName = "Uninstall Vortex";

        this->AddChild("Uninstallation", [this]()
                       { RenderUninstallation(); });

        this->AddChild("Confirm action", [this]()
                       { RenderConfirmAction(); });

        this->AddChild("Uninstall Vortex", [this]()
                       { RenderUninstallVortex(); });

        std::shared_ptr<Cherry::AppWindow> win = m_AppWindow;
    }

    void UninstallAppWindow::AddChild(const std::string &child_name, const std::function<void()> &child)
    {
        m_Childs[child_name] = UninstallAppChild(child, false);
    }

    void UninstallAppWindow::RemoveChild(const std::string &child_name)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            m_Childs.erase(it);
        }
    }

    void UninstallAppWindow::SetChildState(const std::string &child_name, const bool &state)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            it->second.m_Finished = state;
        }
    }

    std::shared_ptr<Cherry::AppWindow> &UninstallAppWindow::GetAppWindow()
    {
        return m_AppWindow;
    }

    std::shared_ptr<UninstallAppWindow> UninstallAppWindow::Create(const std::string &name, const std::shared_ptr<VortexInstallerData> &data)
    {
        auto instance = std::shared_ptr<UninstallAppWindow>(new UninstallAppWindow(name, data));
        instance->SetupRenderCallback();
        return instance;
    }

    void UninstallAppWindow::SetupRenderCallback()
    {
        auto self = shared_from_this();
        m_AppWindow->SetRenderCallback([self]()
                                       {
            if (self) {
                self->Render();
            } });
    }

    std::function<void()> UninstallAppWindow::GetChild(const std::string &child_name)
    {
        auto it = m_Childs.find(child_name);
        if (it != m_Childs.end())
        {
            return it->second.m_Foo;
        }
        return nullptr;
    }

    void UninstallAppWindow::Render()
    {
        static float leftPaneWidth = 300.0f;
        const float minPaneWidth = 50.0f;
        const float splitterWidth = 1.5f;
        static int selected;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, Cherry::HexToRGBA("#111111FF"));
        ImGui::PushStyleColor(ImGuiCol_Border, Cherry::HexToRGBA("#0000000"));
        ImGui::BeginChild("left_pane", ImVec2(leftPaneWidth, 0), true);

        Space(25.0f);
        Cherry::TitleThreeColored("Vortex Uninstaller", "#B1FF31FF");
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
