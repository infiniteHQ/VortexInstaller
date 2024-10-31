#define SDL_MAIN_HANDLED
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include "../ui/installer/installer.hpp"

static std::string version;
static std::string dist;
static std::string arch;
static std::string platform;
static std::vector<std::string> version_pools;

std::string getHomeDirectory()
{
#if defined(__linux__) || defined(__APPLE__)
    const char *homePath = std::getenv("HOME");
    if (homePath == nullptr)
    {
        throw std::runtime_error("HOME environment variable not set");
    }
    return std::string(homePath);
#endif

#if defined(_WIN32) || defined(_WIN64)
    const char *homePath = std::getenv("USERPROFILE");
    if (homePath == nullptr)
    {
        const char *homeDrive = std::getenv("HOMEDRIVE");
        const char *homePathEnv = std::getenv("HOMEPATH");
        if (homeDrive == nullptr || homePathEnv == nullptr)
        {
            throw std::runtime_error("HOME environment variables not set");
        }
        return std::string(homeDrive) + std::string(homePathEnv);
    }
    return std::string(homePath);
#endif

    throw std::runtime_error("Unknown platform: Unable to determine home directory");
}

void RefreshEnvironmentVortexVersionsPools()
{
    std::string path;
#if defined(_WIN32) || defined(_WIN64)
    path = getHomeDirectory() + "\\.vx\\configs\\";
#endif

#if defined(__linux__) || defined(__APPLE__)
    path = getHomeDirectory() + "/.vx/configs/";
#endif

    std::string json_file;
#if defined(_WIN32) || defined(_WIN64)
    json_file = path + "\\vortex_versions_pools.json";
#endif

#if defined(__linux__) || defined(__APPLE__)
    json_file = path + "/vortex_versions_pools.json";
#endif

    try
    {
        nlohmann::json json_data;

        if (!std::filesystem::exists(json_file))
        {
            //
        }

        std::ifstream readed_file(json_file);

        if (!readed_file.is_open())
        {
            //
        }

        readed_file >> json_data;

        for (auto pool : json_data["vortex_versions_pools"])
        {
            version_pools.push_back(pool);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void parseArguments(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.find("--arch=") == 0)
        {
            arch = arg.substr(7);
        }

        if (arg.find("--platform=") == 0)
        {
            platform = arg.substr(11);
        }

        if (arg.find("--dist=") == 0)
        {
            dist = arg.substr(7);
        }

        if (arg.find("--version=") == 0)
        {
            version = arg.substr(10);
        }
    }
}

int main(int argc, char *argv[])
{
    g_InstallerData = std::make_shared<VortexInstallerData>();

    g_InstallerData->g_WorkingPath = g_InstallerData->g_DefaultInstallPath;
    parseArguments(argc, argv);

    g_InstallerData->g_Action = "vxinstall";

    RefreshEnvironmentVortexVersionsPools();
    g_InstallerData->m_VortexPools = version_pools;

    std::string url = "http://api.infinite.si:9000/api/vortexupdates/get_filtered_v_versions?platform=" + platform + "&dist=" + dist + "&arch=" + arch + "&version=" + version ;
    RestClient::Response r = RestClient::get(url);

    if (r.code != 200)
    {
        //
    }

    auto json_response = nlohmann::json::parse(r.body);
    std::vector<VortexVersion> versions;

    for (const auto &item : json_response)
    {
        VortexVersion v;
        v.id = item["id"].get<int>();
        v.version = item["version"].get<std::string>();
        v.name = item["name"].get<std::string>();
        v.arch = item["arch"].get<std::string>();
        v.dist = item["dist"].get<std::string>();
        v.link = item["link"].get<std::string>();
        g_InstallerData->g_RequestTarballPath = v.link;
        g_InstallerData->g_RequestSumPath = v.link; // TODO
        v.platform = item["platform"].get<std::string>();
        v.date = item["date"].get<std::string>();
        v.created_at = item["created_at"].get<std::string>();
        versions.push_back(v);
    }

    for (auto& v : versions) {
        if (v.version == version && v.arch == arch) {
            g_InstallerData->m_SelectedVortexVersion = v;
            break;
        }
    }

    std::thread mainThread([&]()
                           { Cherry::Main(argc, argv); });

    while (g_ApplicationRunning)
    {
    }

    mainThread.join();
    return 0;
}
