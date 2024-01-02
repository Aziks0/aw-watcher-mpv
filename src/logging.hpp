#pragma once

#include <iostream>

#include "common.hpp"

namespace logging {

namespace {

enum Level {
    LEVEL_NO,
    LEVEL_FATAL,
    LEVEL_ERROR,
    LEVEL_WARN,
    LEVEL_INFO,
    LEVEL_DEBUG,
};

std::map<std::string, Level> level_map = {
    {"no", LEVEL_NO},     {"fatal", LEVEL_FATAL}, {"error", LEVEL_ERROR},
    {"warn", LEVEL_WARN}, {"info", LEVEL_INFO},   {"debug", LEVEL_DEBUG},
};

} // namespace

class Logger {
  private:
    Level level;
    std::string prefix;

  public:
    Logger(std::string prefix);

    Logger(std::string prefix, std::string level);

    void set_level(std::string level);

    template <typename... Args> void fatal(const char *format, Args &&...args) {
        if (this->level < LEVEL_FATAL)
            return;

        std::clog << this->prefix << "FATAL ERROR: "
                  << std::vformat(format, std::make_format_args(args...))
                  << std::endl;
    }

    template <typename... Args> void error(const char *format, Args &&...args) {
        if (this->level < LEVEL_ERROR)
            return;

        std::clog << this->prefix
                  << std::vformat(format, std::make_format_args(args...))
                  << std::endl;
    }

    template <typename... Args> void warn(const char *format, Args &&...args) {
        if (this->level < LEVEL_WARN)
            return;

        std::cout << this->prefix
                  << std::vformat(format, std::make_format_args(args...))
                  << std::endl;
    }

    template <typename... Args> void info(const char *format, Args &&...args) {
        if (this->level < LEVEL_INFO)
            return;

        std::cout << this->prefix
                  << std::vformat(format, std::make_format_args(args...))
                  << std::endl;
    }

    template <typename... Args> void debug(const char *format, Args &&...args) {
        if (this->level < LEVEL_DEBUG)
            return;

        std::cout << this->prefix
                  << std::vformat(format, std::make_format_args(args...))
                  << std::endl;
    }
};

} // namespace logging
