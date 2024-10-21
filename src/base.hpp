#pragma once

#ifndef BASE_VORTEXINSTALLER_H
#define BASE_VORTEXINSTALLER_H

#include "../lib/json/single_include/nlohmann/json.hpp"

struct VortexInstallerData
{
    std::string g_DefaultInstallPath = "???";
    std::string g_WorkingPath = "";
    std::string g_Action = "install";
    std::string g_Platform = ""; // windows, linux, macos
    std::string g_Arch = "";     // x86_64, arm64, riscv, etc...
    bool g_Request = false;
    bool g_UseNet = true;
    nlohmann::json jsonResponse;
    nlohmann::json g_RequestValues;
    std::string g_RequestVersion = "";
    std::string g_RequestTarballPath = "";
    std::string g_RequestSumPath = "";

    int state_n = 0;
    std::string state = "Unknown";
    std::string result = "processing";
    bool finished = false;

    std::function<void()> m_InstallCallback;
};

#endif // BASE_VORTEXINSTALLER_H