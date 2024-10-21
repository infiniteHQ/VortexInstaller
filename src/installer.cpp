#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include "../ui/installer/installer.hpp"

void parseArguments(int argc, char *argv[], std::string &action, std::string &path)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.find("--path=") == 0)
        {
            path = arg.substr(7);
        }
    }

    if (path.empty())
    {
        path = g_InstallerData->g_DefaultInstallPath;
    }
}

int main(int argc, char *argv[])
{
    g_InstallerData = std::make_shared<VortexInstallerData>();

    g_InstallerData->g_WorkingPath = g_InstallerData->g_DefaultInstallPath;

    g_InstallerData->g_Action = "install";

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

    parseArguments(argc, argv, g_InstallerData->g_Action, g_InstallerData->g_WorkingPath);

    std::cout << "Action: " << g_InstallerData->g_Action << std::endl;
    std::cout << "Path: " << g_InstallerData->g_WorkingPath << std::endl;

    std::thread mainThread([&]()
                           { Cherry::Main(argc, argv); });

    while (g_ApplicationRunning)
    {
    }

    mainThread.join();

    return 0;
}
