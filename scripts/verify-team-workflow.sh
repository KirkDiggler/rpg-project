#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }

[ -L "$ROOT/AGENTS.md" ] || fail "AGENTS.md must remain a symlink"
[ "$(readlink "$ROOT/AGENTS.md")" = "CLAUDE.md" ] || fail "AGENTS.md must point to CLAUDE.md"
git -C "$ROOT" check-ignore -q --no-index active.md || fail "active.md must be ignored"

for heading in 'Operator:' '## User direction' '## Current focus' '## Local work' '## Last observed' '## Next' '## Open questions'; do
  grep -Fq "$heading" "$ROOT/docs/templates/local-active.md" || fail "local active template lacks $heading"
done

for team in platform ui-ux assets monster-ai cross-team; do
  prompt="$ROOT/docs/teams/roles/$team/prompt.md"
  [ -f "$prompt" ] || fail "missing Team charter $team"
  grep -Fq "— $team agent, on behalf of <github-login>" "$prompt" || fail "wrong signature contract for $team"
done

[ -f "$ROOT/.agents/skills/README.md" ] || fail "clean skill catalog README is missing"
if find "$ROOT/.agents/skills" -mindepth 2 -name SKILL.md -print -quit | grep -q .; then
  fail "pilot skill catalog must contain no SKILL.md"
fi

for pointer in 'gh api user' 'rpg-project/active.md' 'docs/teams/roles/' '.agents/skills/' "owning repository's AGENTS.md"; do
  grep -Fq "$pointer" "$ROOT/CLAUDE.md" || fail "CLAUDE.md lacks startup pointer $pointer"
done

printf 'PASS: additive multi-contributor foundation verified\n'
