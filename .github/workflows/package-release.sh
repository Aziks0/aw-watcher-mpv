#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
#
# SPDX-License-Identifier: MPL-2.0

set -e

mkdir release
cd artifacts

for directory in aw-watcher-mpv-*; do
  cp ../{README.md,LICENSE} "$directory"
  if [[ -n $(find "$directory" -name "*.dll") ]]; then
    zip -r "../release/$directory.zip" "$directory"
  else
    tar czf "../release/$directory.tar.gz" "$directory"
  fi
done
