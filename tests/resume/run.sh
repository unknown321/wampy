#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
out=${1:-/tmp/wampy-resume-tests}
mkdir -p "$out"
for test in policy io sequence late_socket font gate_stress; do
 g++ -std=c++11 -Wall -Wextra -Werror -Wno-misleading-indentation -O1 -g -pthread -fsanitize=address,undefined -fno-omit-frame-pointer tests/resume/${test}_test.cpp -o "$out/$test"
 ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0} "$out/$test"
done
g++ -std=c++11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer tests/resume/utf8_test.cpp src/wstring.cpp -o "$out/utf8"
ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0} "$out/utf8"
if [ -n "${PROTOBUF_HOST_LIB:-}" ]; then
 g++ -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter -O1 -g -pthread -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Ilibs/protobuf/src -Iserver/armv5-unknown-linux-gnueabihf tests/resume/adapter_test.cpp \
  server/armv5-unknown-linux-gnueabihf/command.pb.cc "$PROTOBUF_HOST_LIB" -o "$out/adapter"
 ASAN_OPTIONS=${ASAN_OPTIONS:-detect_leaks=0} "$out/adapter"
else
 printf '%s\n' 'SKIP: adapter requires generated command.pb.cc and PROTOBUF_HOST_LIB (3.21.12)'
fi
