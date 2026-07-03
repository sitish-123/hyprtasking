#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/config/ConfigValue.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/config/values/ConfigValues.hpp>
#include <hyprutils/math/Vector2D.hpp>

#include "globals.hpp"

using namespace Config::Values;

namespace HTConfig {

template<typename T>
inline T value(std::string config) {
    static std::unordered_map<std::string, CConfigValue<T>> cache;

    // Single hashed lookup on the hot path (was count() + operator[] = two hashes).
    // This runs dozens of times per frame during animations/gestures.
    auto it = cache.find(config);
    if (it == cache.end()) {
        CConfigValue<T> val("plugin:hyprtasking:" + config);
        it = cache.emplace(std::move(config), std::move(val)).first;
    }

    return *it->second;
}

} // namespace HTConfig
