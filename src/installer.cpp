#define SDL_MAIN_HANDLED
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

    RestClient::init();

    std::string dist = g_InstallerData->g_Distribution + "_" + g_InstallerData->g_Platform;
    std::string url = "https://api.infinite.si";

    RestClient::Connection *conn = new RestClient::Connection(url);

    conn->SetTimeout(5);
    conn->SetUserAgent("foo/cool");

    RestClient::HeaderFields headers;
    headers["Accept"] = "application/json";
    conn->SetHeaders(headers);

    conn->SetVerifyPeer(false);
    conn->SetCAInfoFilePath("non-existent file");

    conn->FollowRedirects(true);
    conn->FollowRedirects(true, 3);

    RestClient::Response r = conn->get("/api/vortexupdates/get_vl_versions?dist=" + dist + "&arch=" + g_InstallerData->g_Arch);
    if (r.code != 200)
    {
        VXI_LOG("Error: " << r.code << " - " << r.body);
    }

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
                    VXI_LOG("Tarball Path: " << g_InstallerData->g_RequestTarballPath);
                }
                else
                {
                    VXI_LOG("Error: 'path' key missing or not a string");
                }

                if (g_InstallerData->g_RequestValues.contains("sum") && g_InstallerData->g_RequestValues["sum"].is_string())
                {
                    g_InstallerData->g_RequestSumPath = g_InstallerData->g_RequestValues["sum"];
                    VXI_LOG("Sum Path: " << g_InstallerData->g_RequestSumPath);
                }
                else
                {
                    VXI_LOG("Error: 'sum' key missing or not a string");
                }

                if (g_InstallerData->g_RequestValues.contains("version") && g_InstallerData->g_RequestValues["version"].is_string())
                {
                    g_InstallerData->g_RequestVersion = g_InstallerData->g_RequestValues["version"];
                    VXI_LOG("Version: " << g_InstallerData->g_RequestVersion);
                }
                else
                {
                    VXI_LOG("Error: 'version' key missing or not a string");
                }
            }
            else
            {
                VXI_LOG("Unexpected JSON format or empty response.");
            }
        }
        catch (nlohmann::json::parse_error &e)
        {
            VXI_LOG("JSON Parse Error: " << e.what());
        }
    }

    if (g_InstallerData->g_Request)
    {
        VXI_LOG(r.body);
    }
    else
    {
        //
    }

    parseArguments(argc, argv, g_InstallerData->g_Action, g_InstallerData->g_WorkingPath);

    VXI_LOG("Action: " << g_InstallerData->g_Action);
    VXI_LOG("Path: " << g_InstallerData->g_WorkingPath);

    std::thread mainThread([&]()
                           { Cherry::Main(argc, argv); });

    while (g_ApplicationRunning)
    {
    }

    mainThread.join();
    delete conn;
    return 0;
}
