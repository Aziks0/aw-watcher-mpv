#pragma once

#include <algorithm>
#include <chrono>
#include <format>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

typedef std::vector<std::string> properties_t;
