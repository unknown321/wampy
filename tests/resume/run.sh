#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
out=${1:-/tmp/wampy-resume-tests}
mkdir -p "$out"
for test in policy io; do
 g++ -std=c++11 -Wall -Wextra -Werror -Wno-misleading-indentation -O1 -g -pthread -fsanitize=address,undefined -fno-omit-frame-pointer tests/resume/${test}_test.cpp -o "$out/$test"
 ASAN_OPTIONS=detect_leaks=0 "$out/$test"
done
if [ -n "${PROTOBUF_HOST_LIB:-}" ]; then
 g++ -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter -O1 -g -pthread -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Ilibs/protobuf/src -Iserver/armv5-unknown-linux-gnueabihf tests/resume/adapter_test.cpp \
  server/armv5-unknown-linux-gnueabihf/command.pb.cc "$PROTOBUF_HOST_LIB" -o "$out/adapter"
 ASAN_OPTIONS=detect_leaks=0 "$out/adapter"
else
 printf '%s\n' 'SKIP: adapter requires generated command.pb.cc and PROTOBUF_HOST_LIB (3.21.12)'
fi
