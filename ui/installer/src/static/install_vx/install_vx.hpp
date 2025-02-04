#pragma once

#ifndef VXINSTALL_WINDOW_H
#define VXINSTALL_WINDOW_H

#define CHERRY_V1
#include "../../../../../lib/cherry/cherry.hpp"
#include "../../../../../src/base.hpp"

namespace VortexInstaller
{
    struct VortexInstallAppChild
    {
        std::function<void()> m_Foo;
        bool m_Finished;

        VortexInstallAppChild(const std::function<void()>& foo, const bool& finished):
        m_Foo(foo),
        m_Finished(finished)
        {};
    VortexInstallAppChild() : m_Foo(nullptr), m_Finished(false) {};
    };

    // This window can be a "subappwindow" of a parent if you use the constructor with parent parameter.
    class VortexInstallAppWindow : public std::enable_shared_from_this<VortexInstallAppWindow>
    {
    public:
        VortexInstallAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData>& data);

        void AddChild(const std::string &child_name, const std::function<void()> &child);
        void SetChildState(const std::string &child_name, const bool& state);
        void RemoveChild(const std::string &child_name);
        std::function<void()> GetChild(const std::string &child_name);

        std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
        static std::shared_ptr<VortexInstallAppWindow> Create(const std::string &name,  const std::shared_ptr<VortexInstallerData>&data);
        void SetupRenderCallback();
        void Render();

        void RenderInstallVortex();
        void RenderLicenseAgreement();
        void RenderSummary();
        void RenderInstall();

        std::unordered_map<std::string, VortexInstallAppChild> m_Childs;

        std::string m_SelectedChildName;
        std::shared_ptr<VortexInstallerData> m_Data;

        std::shared_ptr<Cherry::AppWindow> m_AppWindow;
    };
}

#endif // VXINSTALL_WINDOW_H