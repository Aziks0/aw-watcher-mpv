<!--
SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>

SPDX-License-Identifier: CC-BY-4.0
-->

# aw-watcher-mpv

An [ActivityWatch](https://activitywatch.net/) watcher for [mpv](https://mpv.io/), as a [`C`
plugin](https://mpv.io/manual/stable/#c-plugins).

## Features

- There is only **one** utility, no need for a `logger` and a `scanner`
- It only runs when mpv is running
- You can choose which properties you want to send on heartbeats

> [!NOTE]
> Heartbeats are only sent when something is playing. If your media is paused, no heartbeats are sent.

## Installation

### Windows

Download `aw-watcher-mpv-windows.zip` from the latest release
[here](https://github.com/Aziks0/aw-watcher-mpv/releases/latest).

Place `aw-watcher-mpv.dll` in your mpv `scripts` folder. By default, it is located in `%APPDATA%/mpv`. Check out the
[mpv documentation](https://mpv.io/manual/stable/#files-on-windows) if you can't find it.

### Linux

> [!IMPORTANT]
> `mpv` needs to be compiled with the `cplugins` feature, otherwise the plugin won't be loaded.
>
> `openssl-3` and `libcurl` need to be installed on your system.

Download `aw-watcher-mpv-linux.tar.gz` from the latest release
[here](https://github.com/Aziks0/aw-watcher-mpv/releases/latest).

Place `aw-watcher-mpv.so` in your mpv `scripts` folder. If you don't know where to find it, check out the [mpv
documentation](https://mpv.io/manual/stable/#files) on the matter.

## Configuration

You can configure the behavior of `aw-watcher-mpv` by creating a JSON file in your mpv `script-opts` folder _(refer to
[installation section](#installation) to find your mpv folder)_. The file needs to have **the same name** as the dll
you've put in `scripts` (if you haven't changed the dll name, it should be named `aw-watcher-mpv.json`).

### Options available

| Option | Description |
| --- | --- |
| `url` | The URL of the Activity Watch API. |
| `poll_time` | How often heartbeats are sent, in **whole seconds** (no float). |
| `pulse_time` | Maximum time between 2 heartbeats to be merged, in **whole seconds** (no float). |
| `log_level` | Log level. See its [own section](#log_level). |
| `properties` | List of properties to send with each heartbeat. See its [own section](#properties). |

#### `log_level`

Log level available:

- `no`
- `fatal`
- `error`
- `warn`
- `info`
- `debug`

#### `properties`

You can choose which mpv properties you want to send by writing them in a JSON array. Take a look at the [default
configuration](#default-configuration) for an exemple.

A list of available properties can be found [here](https://mpv.io/manual/stable/#property-list) or you can call `mpv
--list-properties` in a shell.

> [!NOTE]
> Heartbeats are only sent when the property [`core-idle`](https://mpv.io/manual/stable/#command-interface-core-idle)
> is `false`.

### Default configuration

```json
{
    "url": "http://127.0.0.1:5600/api/0",
    "poll_time": 5,
    "pulse_time": 11,
    "log_level": "error",
    "properties": [
        "filename",
        "media-title"
    ]
}
```

## Credits

- [RundownRhino/aw-watcher-mpv-sender](https://github.com/RundownRhino/aw-watcher-mpv-sender) — for the idea
- [nlohmann/json](https://github.com/nlohmann/json) — JSON for Modern C++
- [cpr](https://github.com/libcpr/cpr) — C++ Requests: Curl for People
- [outcome](https://github.com/ned14/outcome) — Provides very lightweight outcome\<T> and result\<T>
