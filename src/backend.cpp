#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "./api/api.hpp"
#include "./common/common.hpp"

using json = nlohmann::json;

static bool is_only_whitespace(const std::string& s) {
  return s.find_first_not_of(" \t\r\n") == std::string::npos;
}

int main() {
  VortexInstaller::CreateContext();

  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.empty() || is_only_whitespace(line))
      continue;

    try {
      json msg = json::parse(line);

      if (!msg.is_object())
        continue;

      if (!msg.contains("type") || !msg["type"].is_string())
        continue;

      const std::string type = msg["type"];

      if (type == "patch") {
        if (msg.contains("data"))
          VortexInstaller::GetContext()->PatchFromJson(msg["data"]);

        VortexInstaller::GetContext()->g_PollkitApproved = true;
        nlohmann::json out;
        out["type"] = "refresh";
        out["data"] = VortexInstaller::GetContext()->ToJson();
        std::cout << out.dump() << std::endl;

      } else if (type == "command") {
        if (msg.contains("name") && msg["name"].is_string()) {
          const std::string cmd = msg["name"];
          if (cmd == "InstallVortexLauncher") {
            VortexInstaller::InstallVortexLauncher();
          }
        }
      }

      json out;
      out["type"] = "refresh";
      out["data"] = VortexInstaller::GetContext()->ToJson();
      std::cout << out.dump() << std::endl;

    } catch (const json::exception&) {
      continue;
    }
  }

  return 0;
}
