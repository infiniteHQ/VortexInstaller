#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include "./ui/installer/installer.hpp"

void parseArguments(int argc, char *argv[], std::string &action, std::string &path) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "update") {
            g_Action = "update";
        } else if (arg == "install") {
            g_Action = "install";
        }

        if (arg.find("--path=") == 0) {
            path = arg.substr(7);
        }
    }

    if (path.empty()) {
        path = g_DefaultPath;
    }
}

int main(int argc, char *argv[])
{
    g_WorkingPath = g_DefaultPath;

    parseArguments(argc, argv, g_Action, g_WorkingPath);

    std::cout << "Action: " << g_Action << std::endl;
    std::cout << "Path: " << g_WorkingPath << std::endl;

    std::thread mainThread([&]()
                           { Cherry::Main(argc, argv); });

    while (g_ApplicationRunning)
    {
    }

    mainThread.join();

    return 0;
}
