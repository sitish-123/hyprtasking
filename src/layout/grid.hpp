#pragma once

#include <hyprland/src/helpers/AnimatedVariable.hpp>
#include <hyprland/src/helpers/time/Time.hpp>
#include <hyprland/src/render/Framebuffer.hpp>
#include <unordered_map>
#include <unordered_set>

#include "../types.hpp"
#include "layout_base.hpp"

struct HTGridSlot {
    int layer;
    int x;
    int y;
};

class HTLayoutGrid: public HTLayoutBase {
  private:
    PHLANIMVAR<float> scale;
    PHLANIMVAR<Vector2D> offset;

    // Survives workspace destruction so a slot stays sticky for an empty ws.
    std::unordered_map<WORKSPACEID, HTGridSlot> ws_slot_cache;
    std::unordered_map<long long, WORKSPACEID> slot_ws_cache;

    // Layout cache. build_overview_layout() runs on EVERY render frame — including the
    // ~71fps of content-driven re-renders when the overview sits open over a live window.
    // The result only depends on (stage, offset, scale, dims, monitor size), so cache it
    // and skip the rebuild + the per-frame rawMonitorFocus save/restore when unchanged.
    bool m_layout_cached = false;
    HTViewStage m_cached_stage {};
    Vector2D m_cached_offset {};
    float m_cached_scale = 0;
    int m_cached_rows = 0;
    int m_cached_cols = 0;
    Vector2D m_cached_mon_size {};

    // EXPERIMENTAL snapshot cache (plugin:hyprtasking:experimental:snapshots): workspace id →
    // offscreen framebuffer + validity. Filled on demand by snapshot_for(); invalidated on
    // open / move / workspace change so it re-renders.
    struct HTSnapshot {
        SP<Render::IFramebuffer> fb;
        bool                     valid = false;
    };
    std::unordered_map<WORKSPACEID, HTSnapshot> m_snapshots;

    static long long pack_slot(int layer, int x, int y);

  public:
    HTLayoutGrid(VIEWID view_id);
    virtual ~HTLayoutGrid() = default;

    virtual std::string layout_name();

    virtual bool is_animating();

    virtual CBox calculate_ws_box(int x, int y, HTViewStage stage);

    virtual void close_open_lerp(float perc);
    virtual void on_show(CallbackFun on_complete);
    virtual void on_hide(CallbackFun on_complete);
    virtual void on_move(WORKSPACEID old_id, WORKSPACEID new_id, CallbackFun on_complete);
    virtual void on_move_swipe(Vector2D delta);
    virtual WORKSPACEID on_move_swipe_end();

    virtual WORKSPACEID get_ws_id_in_direction(int x, int y, std::string& direction);

    virtual bool should_render_window(PHLWINDOW window);
    virtual float drag_window_scale();
    virtual void init_position();
    virtual void build_overview_layout(HTViewStage stage);
    virtual void render();

    // EXPERIMENTAL snapshots. snapshot_for renders `workspace` into its cached FBO if the
    // cache entry is invalid, and returns it; invalidate_snapshots forces a re-render.
    SP<Render::IFramebuffer> snapshot_for(PHLMONITOR monitor, PHLWORKSPACE workspace, const Time::steady_tp& time);
    void invalidate_snapshots() { for (auto& [id, s] : m_snapshots) s.valid = false; }
    // Invalidate ONE workspace's snapshot (live-content damage → re-render just that tile).
    void invalidate_snapshot(WORKSPACEID id) {
        const auto it = m_snapshots.find(id);
        if (it != m_snapshots.end())
            it->second.valid = false;
    }

    void get_grid_dims(int& rows, int& cols);
    void refresh_grid_dims();
    int m_grid_rows = 3;
    int m_grid_cols = 3;
    void refresh_workspace_cache(const std::unordered_set<WORKSPACEID>& extra_off_limits = {});
    WORKSPACEID slot_workspace(int layer, int x, int y);

    const std::unordered_map<WORKSPACEID, HTGridSlot>& cache() const { return ws_slot_cache; }
};
