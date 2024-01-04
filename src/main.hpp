/**
 * SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#pragma once

#include "common.hpp"
#include "mpv/client.h"

extern "C" MPV_EXPORT int mpv_open_cplugin(mpv_handle *mpv);
