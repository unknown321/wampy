#!/bin/sh
# GLFW_SOURCE must contain the pinned submodule with libs/glfw.patch applied.
set -eu
root=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
out=${1:-/tmp/wampy-input-tests}
glfw=${GLFW_SOURCE:-$root/libs/glfw}
mkdir -p "$out"
flags="-std=c99 -D_GNU_SOURCE -g -O1 -ffunction-sections -fdata-sections -fsanitize=address,undefined -fno-omit-frame-pointer"
${CC:-cc} $flags -I"$glfw/src" -I"$glfw/include" -c "$glfw/src/null_window.c" -o "$out/null_window.o"
${CC:-cc} $flags -I"$glfw/src" -I"$glfw/include" "$root/tests/input/null_events.c" "$out/null_window.o" \
    -Wl,--gc-sections -Wl,--wrap=ioctl -pthread -o "$out/null_events"
"$out/null_events"
