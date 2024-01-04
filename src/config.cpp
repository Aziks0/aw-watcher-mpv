/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <fstream>
#include <filesystem>
#include <system_error>
#include <stdexcept>

#include "config.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32 // WINDOWS IMPLEMENTATIONS

#define REAL_MAX_PATH                                                          \
    1024 // Recent Windows versions don't limit the path length

std::filesystem::path get_exe_directory() {
    char buffer[REAL_MAX_PATH];
    DWORD res = GetModuleFileNameA(NULL, buffer, REAL_MAX_PATH);
    if (res == 0 || res == REAL_MAX_PATH) {
        throw std::system_error(GetLastError(), std::system_category(),
                                "GetModuleFileNameA");
    }
    return std::filesystem::path(buffer).parent_path();
}

std::filesystem::path get_config_dir_impl() {
    // First, check if config is portable
    const std::filesystem::path exe_dir = get_exe_directory();
    if (std::filesystem::exists(exe_dir / "portable_config")) {
        return exe_dir / "portable_config" / "script-opts";
    }

    // Then, check for %MPV_HOME%
    const char *mpv_home = std::getenv("MPV_HOME");
    if (mpv_home) {
        return std::filesystem::path(mpv_home) / "script-opts";
    }

    // Finally, default to %APPDATA%/mpv
    const char *appdata = std::getenv("APPDATA");
    if (appdata == nullptr) {
        throw std::runtime_error("APPDATA env doesn't exist???");
    }
    return std::filesystem::path(appdata) / "mpv" / "script-opts";
}

#else // UNIX IMPLEMENTATIONS

std::filesystem::path get_config_dir_impl() {
    const char *mpv_home = std::getenv("MPV_HOME");
    if (mpv_home) {
        return std::filesystem::path(mpv_home) / "script-opts";
    }

    const char *xdg_config_home = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config_home) {
        return std::filesystem::path(xdg_config_home) / "mpv" / "script-opts";
    }

    return std::filesystem::path("~/.config/mpv/script-opts");
}

#endif

namespace config {

std::filesystem::path get_config_dir() { return get_config_dir_impl(); }

// TODO: add logging
Config get_config(std::string filename) {
    std::filesystem::path config_path = get_config_dir() / (filename + ".json");
    if (!std::filesystem::exists(config_path)) {
        return Config();
    }

    std::ifstream config_file(config_path);
    json config;
    config_file >> config;
    return config.get<Config>();
}

} // namespace config
