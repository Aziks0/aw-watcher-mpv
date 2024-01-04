/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <system_error>

#include "utils.hpp"

#ifdef _WIN32
#include <windows.h>
#else // UNIX
#include <errno.h>
#include <unistd.h>
#endif

#ifdef _WIN32 // WINDOWS IMPLEMENTATIONS

std::string get_hostname_impl() {
    std::vector<WCHAR> buffer(MAX_COMPUTERNAME_LENGTH, 0);
    DWORD size = MAX_COMPUTERNAME_LENGTH;
    if (GetComputerNameExW(ComputerNameDnsHostname, &buffer[0], &size)) {
        const std::wstring w_hostname(&buffer[0], size);
        return utils::wstring_to_string(w_hostname);
    }

    // `GetComputerNameExW()` can return ERROR_MORE_DATA if the buffer is too
    // small and `size` is then set to the buffer size that we need.
    const DWORD error = GetLastError();
    if (error != ERROR_MORE_DATA) {
        throw std::system_error(error, std::system_category(),
                                "GetComputerNameExW");
    }
    if (size == 0) {
        return "";
    }

    buffer.resize(size, 0);
    if (!GetComputerNameExW(ComputerNameDnsHostname, &buffer[0], &size)) {
        throw std::system_error(GetLastError(), std::system_category(),
                                "GetComputerNameExW");
    }
    const std::wstring w_hostname(&buffer[0], size);
    return utils::wstring_to_string(w_hostname);
}

#else // UNIX IMPLEMENTATIONS

std::string get_hostname_impl() {
    std::vector<char> buffer(1024, 0);
    if (gethostname(&buffer[0], buffer.size()) == -1) {
        throw std::system_error(errno, std::system_category(), "gethostname");
    }
    return std::string(&buffer[0]);
}

#endif

namespace utils {

#ifdef _WIN32

std::string wstring_to_string(const std::wstring &wstr) {
    if (wstr.empty()) {
        return std::string();
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(),
                                         (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0],
                        size, NULL, NULL);
    return str;
}

#endif

std::string get_hostname() { return get_hostname_impl(); }

} // namespace utils
