/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

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

#include <outcome.hpp>

namespace outcome = OUTCOME_V2_NAMESPACE;

typedef std::vector<std::string> properties_t;
