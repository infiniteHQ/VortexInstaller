#pragma once

#ifndef BASE_VORTEXINSTALLER_H
#define BASE_VORTEXINSTALLER_H

#include "../lib/json/single_include/nlohmann/json.hpp"

static void Space(const float &space)
{
    ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
    ImGui::Button("", ImVec2(0, space));
    ImGui::PopStyleColor(2);
    ImGui::EndDisabled();
}

static std::string ReadFile(const std::string &file_path)
{
    std::ifstream file(file_path);
    if (!file.is_open())
        return "Erreur : Unable to open file";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void CustomCheckbox(const std::string &label, bool activated, float radius = 7.0f)
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

struct VortexInstallerData
{
    std::string g_DefaultInstallPath = "";
    std::string g_WorkingPath = "";
    std::string g_Distribution = "stable";
    std::string g_Action = "install";
    std::string g_Platform = "";
    std::string g_Arch = ""; 
    bool g_Request = false;
    bool g_UseNet = true;
    nlohmann::json jsonResponse;
    std::string g_ManifestVersion = "";
    nlohmann::json g_RequestValues;
    std::string g_RequestVersion = "";
    std::string g_RequestTarballPath = "";
    std::string g_RequestSumPath = "";

    bool g_VortexLauncherOutdated = false;

    int state_n = 0;
    std::string state = "Unknown";
    std::string result = "processing";
    bool finished = false;

    std::function<void()> m_InstallCallback;
};

#endif // BASE_VORTEXINSTALLER_H