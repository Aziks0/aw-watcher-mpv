/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#pragma once

#include "common.hpp"

namespace utils {

#ifdef _WIN32

/**
 * @brief Convert a wide string to a UTF-8 string.
 *
 * @param wstr The wide string to convert.
 * @returns The converted UTF-8 string.
 */
std::string wstring_to_string(const std::wstring &wstr);

#endif

/**
 * @brief Get the hostname of the current machine.
 *
 * @throws std::system_error If the hostname cannot be retrieved, altough it
 * should not happen.
 *
 * @returns The hostname.
 */
std::string get_hostname();

} // namespace utils
