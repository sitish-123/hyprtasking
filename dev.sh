#!/usr/bin/env bash
# hyprtasking FORK dev helper — test the fork build without touching daily config.
#
#   ./dev.sh build     just compile the fork
#   ./dev.sh test      build + load the FORK build, adaptive grid ON (this session only)
#   ./dev.sh restore   go back to the daily fork build (delegates to overview-switch.sh)
#
# The fork-only `grid:adaptive` key is enabled at RUNTIME here (not written into
# ~/.config/hypr/custom/general.lua), so the daily/stock build never sees an
# unknown key and never throws a config-error overlay.
set -uo pipefail

DIR="$HOME/Projects/hyprtasking"
DEV_SO="$DIR/build/libhyprtasking.so"
CACHE="/var/cache/hyprpm/$USER"
STOCK_SO="$CACHE/hyprtasking/hyprtasking.so"
SCROLL_SO="$CACHE/hyprland-scroll-overview/scrolloverview.so"
STATE="$HOME/.config/hypr/custom/overview.state"

# --reconfigure first so a newly-added src/*.cpp (meson.build globs sources at CONFIGURE
# time) is actually picked up — a bare `meson compile` silently misses new files.
build() { meson setup --reconfigure "$DIR/build" >/dev/null 2>&1 || true; meson compile -C "$DIR/build"; }

unload_overviews() {
  hyprctl plugin unload "$STOCK_SO"  >/dev/null 2>&1 || true
  hyprctl plugin unload "$DEV_SO"    >/dev/null 2>&1 || true
  hyprctl plugin unload "$SCROLL_SO" >/dev/null 2>&1 || true
}

case "${1:-test}" in
  build) build ;;

  test|load|reload)
    build || { echo "build failed — not loading"; exit 1; }
    echo hyprtasking > "$STATE"
    unload_overviews; sleep 0.3
    hyprctl plugin load "$DEV_SO" || { echo "load failed"; exit 1; }
    hyprctl reload >/dev/null 2>&1
    # fork-only adaptive, runtime only. `hyprctl keyword` is rejected in Lua mode;
    # hl.config() via dispatch applies it (the dispatch-return error is harmless).
    hyprctl dispatch 'hl.config({ plugin = { hyprtasking = { grid = { adaptive = true } } } })' >/dev/null 2>&1
    notify-send -t 2500 "hyprtasking fork" "Dev build loaded — adaptive ON (test session)" 2>/dev/null || true
    echo "FORK loaded, adaptive ON. Open with 4-finger up. Run './dev.sh restore' when done."
    ;;

  restore)
    # The fork is now the daily driver and its settings live in hyprland/hyprtasking.lua.
    # Stock hyprtasking does NOT register the fork-only config keys that lua sets, so loading
    # it triggers Hyprland's uncatchable config-error overlay. So 'restore' returns to the
    # daily FORK build via the canonical switcher instead of loading stock.
    exec "$HOME/.config/hypr/custom/scripts/overview-switch.sh" restore
    ;;

  *) echo "usage: dev.sh {build|test|restore}"; exit 1 ;;
esac
