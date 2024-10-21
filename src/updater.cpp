#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <filesystem>
#include <memory>
#include <mutex>

#include "../ui/installer/installer.hpp"

namespace fs = std::filesystem;

void printManifest(const std::string &manifestPath)
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

std::string findManifestJson(const fs::path &startPath)
{
    fs::path currentPath = startPath;

    while (!currentPath.empty())
    {
        fs::path manifestPath = currentPath / "manifest.json";

        if (fs::exists(manifestPath))
        {
            return manifestPath.string();
        }

        currentPath = currentPath.parent_path();
    }

    return "";
}

std::string CookPath(const std::string input_path)
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

    root_path = fs::path(root_path).parent_path().string();

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

void parseArguments(int argc, char *argv[], std::string &action, std::string &path)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.find("--path=") == 0)
        {
            g_InstallerData->g_DefaultInstallPath = arg.substr(7);
        }

        if (arg.find("--dist=") == 0)
        {
            g_InstallerData->g_Distribution = arg.substr(7);
        }
    }

    if (path.empty())
    {
        std::cout << "No path provided, using default install path: " << path << std::endl;
    }
}

std::vector<int> splitVersion(const std::string &version)
{
    std::vector<int> versionParts;
    std::stringstream ss(version);
    std::string part;

    while (std::getline(ss, part, '.'))
    {
        versionParts.push_back(std::stoi(part));
    }

    while (versionParts.size() < 3)
    {
        versionParts.push_back(0);
    }

    return versionParts;
}

bool isVersionGreater(const std::string &manifestVersion, const std::string &requestVersion)
{
    std::vector<int> v1 = splitVersion(manifestVersion);
    std::vector<int> v2 = splitVersion(requestVersion);

    for (size_t i = 0; i < 3; ++i)
    {
        if (v2[i] > v1[i])
        {
            return true;
        }
        else if (v2[i] < v1[i])
        {
            return false;
        }
    }

    return false;
}

std::string normalizeVersion(const std::string &version)
{
    std::vector<int> versionParts = splitVersion(version);

    std::stringstream ss;
    ss << versionParts[0] << "." << versionParts[1] << "." << versionParts[2];

    return ss.str();
}

int main(int argc, char *argv[])
{
    g_InstallerData = std::make_shared<VortexInstallerData>();

    g_InstallerData->g_Action = "update";

    parseArguments(argc, argv, g_InstallerData->g_Action, g_InstallerData->g_WorkingPath);

    DetectPlatform();
    DetectArch();

    std::string dist = g_InstallerData->g_Distribution + "_" + g_InstallerData->g_Platform;
    std::string url = "https://api.infinite.si/api/vortexupdates/get_vl_versions?dist=" + dist + "&arch=" + g_InstallerData->g_Arch;

    std::cout << url << std::endl;
    RestClient::Response r = RestClient::get(url);

    if (r.code != 200)
    {
        g_InstallerData->g_Request = false;
    }
    else
    {
        g_InstallerData->g_Request = true;
        try
        {
            g_InstallerData->jsonResponse = nlohmann::json::parse(r.body);

            if (!g_InstallerData->jsonResponse.empty() && g_InstallerData->jsonResponse.is_array())
            {
                std::string values_str = g_InstallerData->jsonResponse[0]["values"];

                g_InstallerData->g_RequestValues = nlohmann::json::parse(values_str);

                if (g_InstallerData->g_RequestValues.contains("path") && g_InstallerData->g_RequestValues["path"].is_string())
                {
                    g_InstallerData->g_RequestTarballPath = g_InstallerData->g_RequestValues["path"];
                    std::cout << "Tarball Path: " << g_InstallerData->g_RequestTarballPath << std::endl;
                }
                else
                {
                    std::cout << "Error: 'path' key missing or not a string" << std::endl;
                }

                if (g_InstallerData->g_RequestValues.contains("sum") && g_InstallerData->g_RequestValues["sum"].is_string())
                {
                    g_InstallerData->g_RequestSumPath = g_InstallerData->g_RequestValues["sum"];
                    std::cout << "Sum Path: " << g_InstallerData->g_RequestSumPath << std::endl;
                }
                else
                {
                    std::cout << "Error: 'sum' key missing or not a string" << std::endl;
                }

                if (g_InstallerData->g_RequestValues.contains("version") && g_InstallerData->g_RequestValues["version"].is_string())
                {
                    g_InstallerData->g_RequestVersion = g_InstallerData->g_RequestValues["version"];
                    std::cout << "Version: " << g_InstallerData->g_RequestVersion << std::endl;
                }
                else
                {
                    std::cout << "Error: 'version' key missing or not a string" << std::endl;
                }
            }
            else
            {
                std::cout << "Unexpected JSON format or empty response." << std::endl;
            }
        }
        catch (nlohmann::json::parse_error &e)
        {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        }
    }

    if (g_InstallerData->g_Request)
    {
        std::cout << r.body << std::endl;
    }
    else
    {
        //
    }

    if (g_InstallerData->g_WorkingPath.empty())
    {
        g_InstallerData->g_WorkingPath = CookPath("");
        std::cout << "Path derived from executable: " << g_InstallerData->g_WorkingPath << std::endl;
    }

    std::string manifestPath = findManifestJson(g_InstallerData->g_WorkingPath);
    if (!manifestPath.empty())
    {
        std::cout << "Found manifest.json at: " << manifestPath << std::endl;

        g_InstallerData->g_ManifestVersion = getManifestVersion(manifestPath);
        if (!g_InstallerData->g_ManifestVersion.empty())
        {
            g_InstallerData->g_ManifestVersion = normalizeVersion(g_InstallerData->g_ManifestVersion);
            std::string requestVersion = normalizeVersion(g_InstallerData->g_RequestVersion);

            std::cout << "Manifest version: " << g_InstallerData->g_ManifestVersion << std::endl;
            std::cout << "Requested version: " << requestVersion << std::endl;

            bool isGreater = isVersionGreater(g_InstallerData->g_ManifestVersion, requestVersion);
            if (isGreater)
            {
                g_InstallerData->g_VortexLauncherOutdated = true;
                std::cout << "Requested version is greater than manifest version. VortexLauncher is outdated." << std::endl;
            }
            else
            {
                g_InstallerData->g_VortexLauncherOutdated = false;
                std::cout << "Requested version is not greater than manifest version. VortexLauncher is up to date." << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to extract version from manifest.json!" << std::endl;
        }
    }
    else
    {
        std::cerr << "manifest.json not found!" << std::endl;
    }

    std::thread mainThread([&]()
                           { Cherry::Main(argc, argv); });

    while (g_ApplicationRunning)
    {
        //
    }

    mainThread.join();

    return 0;
}