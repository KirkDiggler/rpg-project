#!/bin/bash
# Compiles ONE generated .cc to $OUT/obj/<flattened>.o (incremental).
# Called by build.sh through xargs; FLAGS / INC / CXX / GEN / OUT come from env.
set -euo pipefail
src=$1
obj="$OUT/obj/$(echo "${src#"$GEN"/}" | tr '/' '_' | sed 's/\.cc$/.o/')"
if [ -f "$obj" ] && [ ! "$src" -nt "$obj" ]; then
  exit 0
fi
# shellcheck disable=SC2086  # word splitting is the point
"$CXX" $FLAGS $INC -c "$src" -o "$obj.tmp" 2>"$obj.log"
status=$?
if [ $status -ne 0 ]; then
  echo "FAILED: $src (see $obj.log)" >&2
  head -20 "$obj.log" >&2
  exit $status
fi
rm -f "$obj.log"
mv "$obj.tmp" "$obj"
