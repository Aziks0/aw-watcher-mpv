/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#pragma once

#include "common.hpp"

namespace config {

class Config {
  public:
    /// @brief How often we send heartbeats, in seconds.
    unsigned int poll_time = 5;

    /// @brief Maximum time for merging heartbeats, in seconds.
    unsigned int pulse_time = 11;

    /// @brief The URL of the Activity Watch API.
    std::string url = "http://127.0.0.1:5600/api/0";

    /// @brief List of properties to send with each heartbeat.
    properties_t properties = {"filename", "media-title"};

    std::string log_level = "error";

    Config() = default;

    Config(unsigned int poll_time, unsigned int pulse_time, std::string url,
           std::string log_level, properties_t properties)
        : poll_time(poll_time), pulse_time(pulse_time), url(std::move(url)),
          log_level(std::move(log_level)), properties(std::move(properties)) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Config, poll_time, pulse_time,
                                                url, log_level, properties)
};

/**
 * @brief Get the mpv plugin config.
 *
 * @param filename Name of the config file.
 * @returns The config.
 */
Config get_config(std::string filename);

} // namespace config
