# SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
#
# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.14)

include(FetchContent)

project(aw_watcher_mpv VERSION 1.0.0 LANGUAGES CXX)

# We need c++20 for `std::jthread`, `std::format` and `std::chrono::utc_clock`.
# It is supported by GCC 13, Clang 18 and MSVC 19.29.
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
        message(FATAL_ERROR "GCC 13 is the minimum required version")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18)
        message(FATAL_ERROR "Clang 18 is the minimum required version")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.29)
        message(FATAL_ERROR "MSVC 19.29 is the minimum required version")
    endif()
else()
    message(FATAL_ERROR "Unsupported compiler")
endif()

# We want static linking on windows
if(WIN32)
    set(BUILD_SHARED_LIBS OFF)
endif()

# Build libraries only if they are not installed
FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8 # Commit hash for 1.10.5
)
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(nlohmann_json cpr)

add_library(aw_watcher_mpv SHARED
    src/aw_client.cpp
    src/utils.cpp
    src/logging.cpp
    src/config.cpp
    src/main.cpp
)

# We could use `find_package` for mpv but it's a pain on windows
target_include_directories(aw_watcher_mpv PRIVATE
    src
    third_party/mpv/include
    third_party/outcome/include
)
target_link_libraries(aw_watcher_mpv PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
)

# https://github.com/mpv-player/mpv/blob/28b21e4ab7ca00aecc4246d9185bf77f92db98d2/DOCS/man/libmpv.rst?plain=1#L63
target_compile_definitions(aw_watcher_mpv PRIVATE
    $<$<BOOL:${WIN32}>:MPV_CPLUGIN_DYNAMIC_SYM>
)
