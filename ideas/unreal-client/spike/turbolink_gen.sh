#!/bin/bash
# Step 3: runs TurboLink's code generator over ALL our protos, the way the
# author's Tools/generate_code.cmd does (--turbolink_out, GenerateJsonCode=true),
# but on Linux: protoc from Debian, the prebuilt .NET exe under mono.
# Inside docker/mono.Dockerfile, with the protos checkout mounted read-only.
set -euo pipefail

PROTOS=${PROTOS:-/protos}
SPIKE=${SPIKE:-/spike}
OUT=$SPIKE/tlgen

rm -rf "$OUT" && mkdir -p "$OUT"
cat > /tmp/protoc-gen-turbolink <<'EOF'
#!/bin/bash
exec mono /spike/vendor/Tools/protoc-gen-turbolink.exe
EOF
chmod +x /tmp/protoc-gen-turbolink

echo "== tools =="
protoc --version | sed 's/^/   /'
mono --version | head -1 | sed 's/^/   /'
ls -la /spike/vendor/Tools/protoc-gen-turbolink.exe | sed 's/^/   /'

cd "$PROTOS"
FILES=$(find . -name '*.proto' -not -path './.worktrees/*' -not -path './.claude/*' -not -path './gen/*' -not -path './node_modules/*' | sort)
echo "== $(echo "$FILES" | wc -l) proto files =="

SECONDS=0
# shellcheck disable=SC2086  # one file per argument is the point
protoc --proto_path=. --proto_path=/usr/include $FILES \
  --plugin=protoc-gen-turbolink=/tmp/protoc-gen-turbolink \
  --turbolink_out="$OUT" --turbolink_opt=GenerateJsonCode=true
echo "== generator finished in ${SECONDS}s, exit $? =="

echo "== output =="
find "$OUT" -type f | sed "s#$OUT/##" | sort | head -80
echo "   files: $(find "$OUT" -type f | wc -l)   LOC: $(find "$OUT" -type f \( -name '*.h' -o -name '*.cpp' \) | xargs cat | wc -l)   size: $(du -sh "$OUT" | cut -f1)"
