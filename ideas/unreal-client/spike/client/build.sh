#!/bin/bash
# Builds the spike's headless client INSIDE the toolchain container
# (docker/toolchain.Dockerfile; clang + libc++, no host installs).
#
# Flags match how TurboLink's Linux libs were built: clang, libc++,
# -fno-exceptions, -fno-rtti. The same generated code (gen/cpp) is what the
# UE host will compile, against these same static libraries.
set -euo pipefail

SPIKE=${SPIKE:-/spike}
TL=$SPIKE/vendor/Source/ThirdParty
GEN=$SPIKE/gen/cpp
OUT=$SPIKE/build
JOBS=${JOBS:-$(nproc)}
CXX=${CXX:-clang++}

# Space-separated on purpose: compile_one.sh word-splits them.
export CXX
export FLAGS="-std=c++17 -stdlib=libc++ -fno-exceptions -fno-rtti -O1 -g0 -DNDEBUG -DGOOGLE_PROTOBUF_NO_RTTI -Wno-deprecated-declarations -Wno-unused-parameter"
export INC="-I$GEN -I$TL/protobuf/include -I$TL/grpc/include -I$TL/abseil/include -I$TL/re2/include"
export GEN OUT

echo "== runtime pins =="
grep -m1 'define PROTOBUF_VERSION ' "$TL/protobuf/include/google/protobuf/port_def.inc" | sed 's/^/   /'
grep -m3 -E 'define GRPC_CPP_VERSION_(MAJOR|MINOR|PATCH) ' "$TL/grpc/include/grpcpp/version_info.h" | tr '\n' ' ' | sed 's/^/   /'; echo
"$CXX" --version | head -1 | sed 's/^/   /'

echo "== compiling generated code (36 pb.cc + 36 grpc.pb.cc), $JOBS at a time =="
mkdir -p "$OUT/obj"
chmod +x "$SPIKE/client/compile_one.sh"
SECONDS=0
find "$GEN" -name '*.cc' | sort | xargs -P "$JOBS" -n 1 "$SPIKE/client/compile_one.sh"
echo "   generated objects: $(ls "$OUT/obj" | wc -l) in ${SECONDS}s"

echo "== compiling + linking the client =="
"$CXX" $FLAGS $INC "$SPIKE/client/main.cc" "$SPIKE/client/openssl3_shim.cc" "$OUT"/obj/*.o \
  -Wl,--start-group \
  "$TL"/protobuf/lib/linux/Release/libprotobuf.a \
  "$TL"/protobuf/lib/linux/Release/libutf8_range.a \
  "$TL"/protobuf/lib/linux/Release/libutf8_validity.a \
  "$TL"/grpc/lib/linux/Release/libgrpc++.a \
  "$TL"/grpc/lib/linux/Release/libgrpc.a \
  "$TL"/grpc/lib/linux/Release/libgpr.a \
  "$TL"/grpc/lib/linux/Release/libupb.a \
  "$TL"/grpc/lib/linux/Release/libaddress_sorting.a \
  "$TL"/re2/lib/linux/Release/libre2.a \
  "$TL"/abseil/lib/linux/Release/*.a \
  -Wl,--end-group \
  -lcares -lssl -lcrypto -lz -lpthread -ldl -lm -lrt \
  -o "$OUT/record_client"
ls -la "$OUT/record_client"
echo "== done =="
