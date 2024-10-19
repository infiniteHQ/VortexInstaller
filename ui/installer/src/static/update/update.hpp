#pragma once

#ifndef UPDATE_WINDOW_H
#define UPDATE_WINDOW_H

#define CHERRY_V1
#include "../../../../../lib/cherry/cherry.hpp"

namespace VortexInstaller
{
    // This window can be a "subappwindow" of a parent if you use the constructor with parent parameter.
    class UpdateAppWindow : public std::enable_shared_from_this<UpdateAppWindow>
    {
    public:
        UpdateAppWindow(const std::string &name);

        void AddChild(const std::string &child_name, const std::function<void()> &child);
        void RemoveChild(const std::string &child_name);
        std::function<void()> GetChild(const std::string &child_name);

        std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
        static std::shared_ptr<UpdateAppWindow> Create(const std::string &name);
        void SetupRenderCallback();
        void Render();

        std::unordered_map<std::string, std::function<void()>> m_Childs;

        std::string m_SelectedChildName;

        std::shared_ptr<Cherry::AppWindow> m_AppWindow;
    };
}

#endif // UPDATE_WINDOW_H