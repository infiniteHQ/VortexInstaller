#pragma once

#ifndef UPDATE_WINDOW_H
#define UPDATE_WINDOW_H

#define CHERRY_V1
#include "../../../../../lib/cherry/cherry.hpp"
#include "../../../../../src/base.hpp"

namespace VortexInstaller
{
    struct UpdateAppChild
    {
        std::function<void()> m_Foo;
        bool m_Finished;

        UpdateAppChild(const std::function<void()>& foo, const bool& finished):
        m_Foo(foo),
        m_Finished(finished)
        {};
    UpdateAppChild() : m_Foo(nullptr), m_Finished(false) {};
    };

    // This window can be a "subappwindow" of a parent if you use the constructor with parent parameter.
    class UpdateAppWindow : public std::enable_shared_from_this<UpdateAppWindow>
    {
    public:
        UpdateAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData>& data);

        void AddChild(const std::string &child_name, const std::function<void()> &child);
        void SetChildState(const std::string &child_name, const bool& state);
        void RemoveChild(const std::string &child_name);
        std::function<void()> GetChild(const std::string &child_name);

        std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
        static std::shared_ptr<UpdateAppWindow> Create(const std::string &name,  const std::shared_ptr<VortexInstallerData>&data);
        void SetupRenderCallback();
        void Render();

        void RenderUpdateVortex();
        void RenderUpdate();

        std::unordered_map<std::string, UpdateAppChild> m_Childs;

        std::string m_SelectedChildName;
        std::shared_ptr<VortexInstallerData> m_Data;

        std::shared_ptr<Cherry::AppWindow> m_AppWindow;
    };
}

#endif // UPDATE_WINDOW_H