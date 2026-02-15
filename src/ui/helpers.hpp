#pragma once

#ifndef BASE_VORTEXINSTALLER_H
#define BASE_VORTEXINSTALLER_H

#include <iostream>

#include "../api/api.hpp"

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif

static void Space(const float &space) {
  CherryGUI::BeginDisabled();
  CherryGUI::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
  CherryGUI::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
  CherryGUI::Button("", ImVec2(0, space));
  CherryGUI::PopStyleColor(2);
  CherryGUI::EndDisabled();
}

static std::string ReadFile(const std::string &file_path) {
  std::ifstream file(file_path);
  if (!file.is_open())
    return "Erreur : Unable to open file";

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

static void
CustomCheckbox(const std::string &label, bool *activated, float radius = 7.0f, const std::string hex = "#B1FF31FF") {
  ImVec2 pos = CherryGUI::GetCursorScreenPos();
  ImDrawList *draw_list = CherryGUI::GetWindowDrawList();

  ImVec2 text_size = CherryGUI::CalcTextSize(label.c_str());

  float offset_y = (text_size.y - radius * 2.0f) * 0.5f;

  ImU32 color_bg = activated ? IM_COL32(70, 70, 70, 255) : IM_COL32(50, 50, 50, 255);

  ImU32 color_check = Cherry::HexToImU32(hex);

  draw_list->AddCircleFilled(ImVec2(pos.x + radius, pos.y + radius + offset_y), radius, color_bg, 16);

  if (activated) {
    draw_list->AddCircleFilled(ImVec2(pos.x + radius, pos.y + radius + offset_y), radius * 0.6f, color_check, 16);
  }

  CherryGUI::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));

  CherryGUI::SameLine();
  CherryGUI::TextUnformatted(label.c_str());
}

static std::string CookPath(const std::string input_path) {
  std::string output_path;
  std::string root_path;

#ifdef _WIN32
  char result[MAX_PATH];
  if (GetModuleFileNameA(NULL, result, MAX_PATH) != 0) {
    root_path = result;
  } else {
    std::cerr << "Failed while getting the root path" << std::endl;
  }
#else
  static std::mutex path_mutex;
  char result[PATH_MAX];
  {
    std::lock_guard<std::mutex> lock(path_mutex);
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
      result[count] = '\0';
      root_path = result;
    } else {
      std::cerr << "Failed while getting the root path" << std::endl;
    }
  }
#endif

  root_path = std::filesystem::path(root_path).parent_path().string();

  if (!input_path.empty() && input_path.front() == '/') {
    output_path = input_path;
  } else {
    output_path = root_path + "/" + input_path;
  }

  return output_path;
}

static void printManifest(const std::string &manifestPath) {
  std::ifstream manifestFile(manifestPath);
  if (!manifestFile.is_open()) {
    std::cerr << "Failed to open " << manifestPath << std::endl;
    return;
  }

  std::string line;
  while (std::getline(manifestFile, line)) {
    std::cout << line << std::endl;
  }
  manifestFile.close();
}

#endif  // BASE_VORTEXINSTALLER_H