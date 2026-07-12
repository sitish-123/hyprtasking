#pragma once

#include <hyprland/src/desktop/DesktopTypes.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/helpers/time/Time.hpp>
#include <hyprutils/math/Box.hpp>
#include <hyprland/src/render/Framebuffer.hpp>

void render_window_at_box(PHLWINDOW window, PHLMONITOR monitor, const Time::steady_tp& time, CBox box);

// EXPERIMENTAL (plugin:hyprtasking:experimental:snapshots). Render a workspace ONCE into the
// offscreen framebuffer `fb` (monitor-sized, pixel space) via the PUBLIC beginFullFakeRender
// path, so the overview can composite the cached texture into tiles instead of re-rendering
// every workspace every frame through the active-workspace swap. NOTE: this nests an
// offscreen render inside the monitor render — the render-pass save/restore behaviour is the
// main thing to verify live; callers snapshot up-front (before building the composite pass).
void render_workspace_to_fb(SP<Render::IFramebuffer> fb, PHLMONITOR monitor, PHLWORKSPACE workspace, const Time::steady_tp& time);

// Render a workspace's contents scaled into `box` (monitor-local, relative to (0,0)).
// Handles the active-workspace swap Hyprland requires to fully render a non-active
// workspace; the caller must restore monitor->m_activeWorkspace afterwards.
void render_workspace_at_box(PHLMONITOR monitor, PHLWORKSPACE workspace, const Time::steady_tp& time, CBox box);
