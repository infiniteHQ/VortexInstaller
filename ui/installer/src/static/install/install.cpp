#include "./install.hpp"

namespace VortexInstaller
{
    void Space(const float &space)
    {
        ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
        ImGui::Button("", ImVec2(0, space));
        ImGui::PopStyleColor(2);
        ImGui::EndDisabled();
    }

    std::string ReadFile(const std::string &file_path)
    {
        std::ifstream file(file_path);
        if (!file.is_open())
            return "Erreur : Unable to open file";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    InstallAppWindow::InstallAppWindow(const std::string &name)
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
                       {
    {
    ImVec2 to_remove = ImGui::CalcTextSize("Finish");

    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - to_remove.x - 50);

        auto accept = std::make_shared<Cherry::CustomButtonSimple>("Finish","Finish");
            accept->SetProperty("bg", "#B1FF31FF");
             accept->SetProperty("bg_hovered", "#C3FF53FF");
             ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
        if(accept->Render("__finish"))
        {
            Cherry::Application().Get().Close();
        }
        ImGui::PopStyleColor();

    } });

        this->AddChild("Summary", [this]()
                       {

    ImVec2 to_remove = ImGui::CalcTextSize("Confirm and install !");

    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - to_remove.x - 50);

    {
        auto accept = std::make_shared<Cherry::CustomButtonSimple>("Confirm","Confirm and install !");
            accept->SetProperty("bg", "#B1FF31FF");
             accept->SetProperty("bg_hovered", "#C3FF53FF");
             ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
        if(accept->Render("__confirm"))
        {
             m_SelectedChildName = "Installation";
        }
        ImGui::PopStyleColor();

    } });

        this->AddChild("Accept Licence Agreement", [this]()
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
        auto deny = std::make_shared<Cherry::CustomButtonSimple>("Deny","Deny");
        if(deny->Render())
        {
            Cherry::Application().Get().Close();
        }
    }

    ImGui::SameLine();

    {
        auto accept = std::make_shared<Cherry::CustomButtonSimple>("Accept","Accept");
            accept->SetProperty("bg", "#B1FF31FF");
             accept->SetProperty("bg_hovered", "#C3FF53FF");
             ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
        if(accept->Render("__another"))
        {
             m_SelectedChildName = "Summary";
        }
        ImGui::PopStyleColor();

    } });

        this->AddChild("Install Vortex", [this]()
                       { 
                        Space(30.0f);
Cherry::TitleThreeColored("Welcome to Vortex !", "#AAAAAAFF");
ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#777777FF"));
ImGui::TextWrapped("We re glad to see you here. This software is an installation wizard that can help you install the Vortex Launcher. The Vortex Launcher is a powerful tool designed to assist you in managing projects, modules, plugins, and content. After installation, you will be able to create your projects and explore all the amazing features and capabilities that Vortex provides. Enjoy your experience!");
                       
                        ImGui::PopStyleColor();

                         Space(20.0f);
    std::shared_ptr<std::string> val = std::make_shared<std::string>("C:/Program Files/VortexLauncher");

Cherry::TitleFiveColored("Please select the installation path", "#787878FF");
    {
        auto input = std::make_shared<Cherry::SimpleStringInput>("Select", val, "Select install path");
        input->Render("__blank");
    }


    ImVec2 to_remove = ImGui::CalcTextSize("DenyAccept");
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - to_remove.x - 50);

    {
        auto deny = std::make_shared<Cherry::CustomButtonSimple>("Close","Close");
        if(deny->Render())
        {
            Cherry::Application().Get().Close();
        }
    }

    ImGui::SameLine();
    
    {
        auto accept = std::make_shared<Cherry::CustomButtonSimple>("Contdinue","Continue");
            accept->SetProperty("bg", "#B1FF31FF");
             accept->SetProperty("bg_hovered", "#C3FF53FF");
             ImGui::PushStyleColor(ImGuiCol_Text, Cherry::HexToRGBA("#121212FF"));
        if(accept->Render("sec"))
        {

        m_SelectedChildName = "Accept Licence Agreement";
            this->SetChildState("Install Vortex", true);
        }
        ImGui::PopStyleColor();

    } });
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

    std::shared_ptr<InstallAppWindow> InstallAppWindow::Create(const std::string &name)
    {
        auto instance = std::shared_ptr<InstallAppWindow>(new InstallAppWindow(name));
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

    void CustomCheckbox(const std::string &label, bool activated, float radius = 7.0f)
    {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        ImVec2 text_size = ImGui::CalcTextSize(label.c_str());

        float offset_y = (text_size.y - radius * 2.0f) * 0.5f;

        ImU32 color_bg = activated ? IM_COL32(70, 70, 70, 255) : IM_COL32(50, 50, 50, 255);

        ImU32 color_check = Cherry::HexToImU32("#B1FF31FF");

        draw_list->AddCircleFilled(ImVec2(pos.x + radius, pos.y + radius + offset_y), radius, color_bg, 16);

        if (activated)
        {
            draw_list->AddCircleFilled(ImVec2(pos.x + radius, pos.y + radius + offset_y), radius * 0.6f, color_check, 16);
        }

        ImGui::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));

        ImGui::SameLine();
        ImGui::TextUnformatted(label.c_str());
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
