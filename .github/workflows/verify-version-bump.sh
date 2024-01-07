#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2024 Benjamin DIDIER <contact@aziks.aleeas.com>
#
# SPDX-License-Identifier: MPL-2.0

set -e

# Get version from CMakeLists.txt
current_version=$(grep 'project(' CMakeLists.txt | sed -nre 's/.*VERSION (([0-9]+\.){2}[0-9]+).*/\1/p')

# Get last version from git tag
last_version=$(git tag --sort=taggerdate | tail -1)

# Get the lesser version between the two
lesser_version=$(printf "%s\n%s" "$current_version" "$last_version" | sort -V | head -n1)

# `exit 1` if `current_version` <= `last_version`
if [[ "$current_version" == "$lesser_version" ]]; then
  echo "Project version has not been bumped."
  exit 1
fi

exit 0
