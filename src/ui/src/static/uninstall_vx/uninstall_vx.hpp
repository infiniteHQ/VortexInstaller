#pragma once

#ifndef UNINSTALLVX_WINDOW_H
#define UNINSTALLVX_WINDOW_H

#define CHERRY_V1
#include "../../../../../lib/cherry/cherry.hpp"
#include "../../../../common/common.hpp"

namespace VortexInstaller {
  struct UninstallVortexAppChild {
    std::function<void()> m_Foo;
    bool m_Finished;

    UninstallVortexAppChild(const std::function<void()> &foo, const bool &finished) : m_Foo(foo), m_Finished(finished) { };
    UninstallVortexAppChild() : m_Foo(nullptr), m_Finished(false) { };
  };

  // This window can be a "subappwindow" of a parent if you use the constructor
  // with parent parameter.
  class VortexUninstallAppWindow : public std::enable_shared_from_this<VortexUninstallAppWindow> {
   public:
    VortexUninstallAppWindow(const std::string &name, const std::shared_ptr<VortexInstallerData> &data);

    void AddChild(const std::string &child_name, const std::function<void()> &child);
    void SetChildState(const std::string &child_name, const bool &state);
    void RemoveChild(const std::string &child_name);
    std::function<void()> GetChild(const std::string &child_name);

    std::shared_ptr<Cherry::AppWindow> &GetAppWindow();
    static std::shared_ptr<VortexUninstallAppWindow> Create(
        const std::string &name,
        const std::shared_ptr<VortexInstallerData> &data);
    void SetupRenderCallback();
    void Render();

    void RenderUninstallVortex();
    void RenderUninstallation();

    bool m_DeleteVortexLauncher = true;
    bool m_DeleteVortex = false;
    bool m_DeleteVortexDatas = false;

    std::unordered_map<std::string, UninstallVortexAppChild> m_Childs;

    std::string m_SelectedChildName;
    std::shared_ptr<VortexInstallerData> m_Data;

    std::shared_ptr<Cherry::AppWindow> m_AppWindow;
  };
}  // namespace VortexInstaller

#endif  // UNINSTALLVX_WINDOW_H