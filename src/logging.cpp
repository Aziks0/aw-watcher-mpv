/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include "logging.hpp"

namespace logging {

Logger::Logger(std::string prefix) : level(LEVEL_ERROR) {
    this->prefix = std::format("[{}] ", prefix);
}

Logger::Logger(std::string prefix, std::string level) : Logger(prefix) {
    this->set_level(level);
}

void Logger::set_level(std::string level) {
    auto it = level_map.find(level);
    if (it == level_map.end()) {
        this->level = LEVEL_ERROR;
        this->error("Unknown log level: {}. Using 'error' instead.", level);
        return;
    }

    this->level = it->second;
    this->info("Log level set to: {}.", level);
}

} // namespace logging
