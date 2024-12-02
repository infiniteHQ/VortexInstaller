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

static std::string CookPath(const std::string input_path)
{
    std::string output_path;
    std::string root_path;

#ifdef _WIN32
    char result[MAX_PATH];
    if (GetModuleFileNameA(NULL, result, MAX_PATH) != 0)
    {
        root_path = result;
    }
    else
    {
        std::cerr << "Failed while getting the root path" << std::endl;
    }
#else
    static std::mutex path_mutex;
    char result[PATH_MAX];
    {
        std::lock_guard<std::mutex> lock(path_mutex);
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        if (count != -1)
        {
            result[count] = '\0';
            root_path = result;
        }
        else
        {
            std::cerr << "Failed while getting the root path" << std::endl;
        }
    }
#endif

    root_path = std::filesystem::path(root_path).parent_path().string();

    if (!input_path.empty() && input_path.front() == '/')
    {
        output_path = input_path;
    }
    else
    {
        output_path = root_path + "/" + input_path;
    }

    return output_path;
}

static void printManifest(const std::string &manifestPath)
{
    std::ifstream manifestFile(manifestPath);
    if (!manifestFile.is_open())
    {
        std::cerr << "Failed to open " << manifestPath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(manifestFile, line))
    {
        std::cout << line << std::endl;
    }
    manifestFile.close();
}

static std::string findManifestJson(const std::filesystem::path &startPath)
{
    std::filesystem::path currentPath = startPath;

    while (!currentPath.empty())
    {
        std::filesystem::path manifestPath = currentPath / "manifest.json";

        if (std::filesystem::exists(manifestPath))
        {
            return manifestPath.string();
        }

        currentPath = currentPath.parent_path();
    }

    return "";
}

struct VortexVersion
{
    int id;
    std::string version;
    std::string name;
    std::string arch;
    std::string dist;
    std::string path;
    std::string sum;
    std::string platform;
    std::string date;
    std::string created_at;
};

struct VortexBuiltinLauncher
{
    std::string version;
    std::string arch;
    std::string platform;
    std::string tarball;
    std::string sum;
};

struct VortexInstallerData
{
    std::string g_VortexDataPath = "";
    std::string g_VortexPath = "";
    std::string g_DefaultInstallPath = "";
    std::string g_WorkingPath = "";
    std::string g_HomeDirectory = "";
    std::string g_Distribution = "stable";
    std::string g_Action = "install";
    std::string g_Platform = "";
    std::string g_Arch = "";
    std::string g_ManifestVersion = "";
    std::string g_RequestVersion = "";
    std::string g_RequestTarballPath = "";
    std::string g_RequestSumPath = "";
    std::string state = "Unknown";
    std::string result = "processing";
    nlohmann::json jsonResponse;
    nlohmann::json g_RequestValues;
    bool g_Request = false;
    bool g_UseNet = true;
    bool g_VortexLauncherOutdated = false;
    bool finished = false;
    int state_n = 0;
    std::function<void()> m_InstallCallback;
    std::function<void()> m_UpdateCallback;
    std::function<void()> m_DowngradeCallback;
    std::function<void()> m_InstallVortexCallback;
    std::function<void()> m_UninstallVortexCallback;
    std::function<void(const bool &vxlauncher, const bool &vx, const bool &vxdata)> m_DeleteCallback;

    VortexVersion m_SelectedVortexVersion;
    VortexBuiltinLauncher m_BuiltinLauncher;
    bool m_BuiltinLauncherExist = false;
    bool m_FolderAlreadyExist;
    std::vector<std::string> m_VortexPools;
};

#endif // BASE_VORTEXINSTALLER_H