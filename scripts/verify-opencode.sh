#!/usr/bin/env bash
set -euo pipefail

fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }
require_command() { command -v "$1" >/dev/null 2>&1 || fail "$1 is required"; }
require_file() { [ -f "$1" ] || fail "$1 is missing"; }

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

require_command opencode
require_command jq
require_file opencode.jsonc
[ -L AGENTS.md ] || fail "AGENTS.md must be a symlink"
[ "$(readlink AGENTS.md)" = "CLAUDE.md" ] || fail "AGENTS.md must point to CLAUDE.md"

tmpfiles=()
cleanup() { [ "${#tmpfiles[@]}" -eq 0 ] || rm -f "${tmpfiles[@]}"; }
trap cleanup EXIT

resolved="$(mktemp)"
tmpfiles+=("$resolved")
opencode debug config >"$resolved"

jq -e '
  .model == "openai/gpt-5.6-sol-fast" and
  .small_model == "openai/gpt-5.6-luna" and
  .default_agent == "platform-lead" and
  .enabled_providers == ["openai"]
' "$resolved" >/dev/null || fail "root model profile is wrong"
jq -e '
  (.agent.build.disable == true) and
  (.agent.plan.disable == true) and
  (.agent.general.mode == "subagent") and
  (.agent.general.model == "openai/gpt-5.6-terra") and
  (.agent.general.variant == "high") and
  (.agent.title | (keys | sort) == ["model", "variant"]) and
  (.agent.summary | (keys | sort) == ["model", "variant"]) and
  (.agent.compaction | (keys | sort) == ["model", "variant"])
' opencode.jsonc >/dev/null || fail "runtime overrides are wrong"
superpowers_plugin="superpowers@git+https://github.com/obra/superpowers.git"
jq -e --arg plugin "$superpowers_plugin" '.plugin == [$plugin]' opencode.jsonc >/dev/null || fail "raw project config must declare the exact Superpowers plugin spec"
jq -e --arg plugin "$superpowers_plugin" '([.plugin[]? | select(. == $plugin)] | length) == 1' "$resolved" >/dev/null || fail "resolved config must contain the exact Superpowers plugin spec once"

general_json="$(mktemp)"
tmpfiles+=("$general_json")
opencode debug agent general >"$general_json" || fail "general agent does not resolve"
jq -e '
  .mode == "subagent" and
  .model.providerID == "openai" and
  .model.modelID == "gpt-5.6-terra" and
  .variant == "high"
' "$general_json" >/dev/null || fail "resolved general profile is wrong"

references=(rpg-toolkit rpg-api rpg-api-protos rpg-dnd5e-web rpg-deployment rpg-game-assets)
jq -e --argjson aliases '["rpg-toolkit","rpg-api","rpg-api-protos","rpg-dnd5e-web","rpg-deployment","rpg-game-assets"]' '
  . as $config |
  ($config.references | keys | sort) == ($aliases | sort) and
  all($aliases[] as $alias | ($config.references[$alias].path == ("../" + $alias)) and
                            ($config.references[$alias].description | type == "string" and length > 0))
' "$resolved" >/dev/null || fail "references must use aliases with exact paths and descriptions"
for server in hub-tools hub-tools-staging linear goat-drops; do
  jq -e --arg server "$server" '.mcp[$server].enabled == false' "$resolved" >/dev/null || fail "MCP $server is not disabled"
done
jq -e '.mcp["chrome-devtools"].enabled == true and .mcp["chrome-devtools"].type == "local" and .mcp["chrome-devtools"].command == ["npx", "-y", "chrome-devtools-mcp@latest", "--browserUrl=http://127.0.0.1:9222", "--no-usage-statistics"]' "$resolved" >/dev/null || fail "chrome-devtools configuration is wrong"

assert_permission_rule() {
  local file="$1" permission="$2" action="$3" pattern="$4"
  jq -e --arg permission "$permission" --arg action "$action" --arg pattern "$pattern" '
    any(.permission[]; .permission == $permission and .action == $action and .pattern == $pattern)
  ' "$file" >/dev/null || fail "missing resolved $permission $action $pattern rule"
}

bash_action() {
  local file="$1" command="$2" action pattern resolved=""
  while IFS=$'\t' read -r action pattern; do
    case "$command" in
      $pattern) resolved="$action" ;;
    esac
  done < <(jq -r '.permission[] | select(.permission == "bash") | [.action, .pattern] | @tsv' "$file")
  printf '%s\n' "$resolved"
}

agents=(ui-lead platform-lead assets-lead rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member game-dev-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer independent-gate explore janitor)
declare -A expected_mode expected_model expected_variant
for agent in ui-lead platform-lead assets-lead; do
  expected_mode[$agent]=primary
  expected_model[$agent]=gpt-5.6-sol-fast
  expected_variant[$agent]=xhigh
done
for agent in rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member game-dev-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer; do
  expected_mode[$agent]=subagent
  expected_model[$agent]=gpt-5.6-terra
  expected_variant[$agent]=high
done
expected_mode[independent-gate]=subagent
expected_model[independent-gate]=gpt-5.6-sol
expected_variant[independent-gate]=max
expected_mode[explore]=subagent
expected_model[explore]=gpt-5.6-luna
expected_variant[explore]=medium
expected_mode[janitor]=subagent
expected_model[janitor]=gpt-5.6-luna
expected_variant[janitor]=low

for agent in "${agents[@]}"; do
  source=".opencode/agents/$agent.md"
  require_file "$source"
  agent_json="$(mktemp)"
  tmpfiles+=("$agent_json")
  opencode debug agent "$agent" >"$agent_json" || fail "agent $agent does not resolve"
  jq -e --arg mode "${expected_mode[$agent]}" --arg model "${expected_model[$agent]}" --arg variant "${expected_variant[$agent]}" '
    .mode == $mode and
    .model.providerID == "openai" and
    .model.modelID == $model and
    .variant == $variant
  ' "$agent_json" >/dev/null || fail "resolved adapter profile is wrong for $agent"

  case "$agent" in
    ui-lead) paths=(docs/teams/roles/director/prompt.md docs/teams/roles/director/field-notes.md docs/teams/roles/director/overlays/ui-ux.md) ;;
    platform-lead) paths=(docs/teams/roles/director/prompt.md docs/teams/roles/director/field-notes.md docs/teams/roles/director/overlays/platform.md) ;;
    assets-lead) paths=(docs/teams/roles/director/prompt.md docs/teams/roles/director/field-notes.md docs/teams/roles/director/overlays/assets.md) ;;
    rpg-toolkit-member) paths=(docs/teams/roles/rpg-toolkit-member/prompt.md) ;;
    rpg-api-member) paths=(docs/teams/roles/rpg-api-member/prompt.md) ;;
    rpg-api-protos-member) paths=(docs/teams/roles/rpg-api-protos-member/prompt.md) ;;
    rpg-deployment-member) paths=(docs/teams/roles/rpg-deployment-member/prompt.md) ;;
    rpg-game-assets-member) paths=(docs/teams/roles/rpg-game-assets-member/prompt.md) ;;
    game-dev-member) paths=(docs/teams/roles/game-dev-member/prompt.md) ;;
    ui-web-member) paths=(docs/teams/roles/rpg-dnd5e-web-member/prompt.md docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md) ;;
    assets-web-member) paths=(docs/teams/roles/rpg-dnd5e-web-member/prompt.md docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md) ;;
    toolkit-fixer) paths=(docs/teams/roles/toolkit-fixer/prompt.md) ;;
    api-fixer) paths=(docs/teams/roles/api-fixer/prompt.md) ;;
    web-fixer) paths=(docs/teams/roles/web-fixer/prompt.md) ;;
    independent-gate) paths=(docs/teams/roles/independent-gate/prompt.md) ;;
    explore) paths=(docs/teams/roles/explore/prompt.md) ;;
    janitor) paths=(docs/teams/roles/janitor/prompt.md) ;;
  esac
  for path in "${paths[@]}"; do
    grep -Fqx -- "- \`$path\`" "$source" || fail "literal canonical pointer $path is missing from $agent"
  done

  if [[ "$agent" == ui-lead || "$agent" == platform-lead || "$agent" == assets-lead ]]; then
    assert_permission_rule "$agent_json" edit deny "*"
    assert_permission_rule "$agent_json" bash deny "*"
    assert_permission_rule "$agent_json" bash allow "gh *"
    assert_permission_rule "$agent_json" bash deny "gh pr merge*"
    assert_permission_rule "$agent_json" bash deny "gh * pr merge*"
    jq -e '
      [.permission | to_entries[] | select(.value.permission == "bash") | {index: .key, action: .value.action, pattern: .value.pattern}] as $rules |
      ([ $rules[] | select(.action == "deny" and .pattern == "*") | .index ] | max) as $broad_deny |
      ([ $rules[] | select(.action == "allow" and .pattern == "gh *") | .index ] | max) as $gh_allow |
      any($rules[]; .action == "allow" and .pattern == "gh *" and .index > $broad_deny) and
      any($rules[]; .action == "deny" and .pattern == "gh pr merge*" and .index > $gh_allow) and
      any($rules[]; .action == "deny" and .pattern == "gh * pr merge*" and .index > $gh_allow)
    ' "$agent_json" >/dev/null || fail "lead gh merge deny must follow broad gh allow"
    [ "$(bash_action "$agent_json" "gh issue view 101")" = allow ] || fail "lead gh coordination must remain allowed"
    [ "$(bash_action "$agent_json" "gh pr merge 103")" = deny ] || fail "lead gh pr merge must resolve to the trailing deny"
    [ "$(bash_action "$agent_json" "gh -R KirkDiggler/rpg-project pr merge 103")" = deny ] || fail "lead repo-flag gh pr merge must resolve to the trailing deny"
    case "$agent" in
      ui-lead) task_allowlist=(ui-web-member web-fixer explore independent-gate janitor) ;;
      platform-lead) task_allowlist=(general rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member toolkit-fixer api-fixer explore independent-gate janitor) ;;
      assets-lead) task_allowlist=(rpg-game-assets-member assets-web-member web-fixer explore independent-gate janitor) ;;
    esac
    task_allowlist_json="$(printf '%s\n' "${task_allowlist[@]}" | jq -R . | jq -s .)"
    jq -e --argjson allowlist "$task_allowlist_json" '
      [.permission[] | select(.permission == "task") | {action: .action, pattern: .pattern}] ==
      ([{action: "deny", pattern: "*"}] + [$allowlist[] | {action: "allow", pattern: .}])
    ' "$agent_json" >/dev/null || fail "lead task permissions must be broad deny followed by the exact manifest allowlist"
  else
    assert_permission_rule "$agent_json" task deny "*"
  fi
done

gate_json="$(mktemp)"
tmpfiles+=("$gate_json")
opencode debug agent independent-gate >"$gate_json"
assert_permission_rule "$gate_json" edit allow "*"
assert_permission_rule "$gate_json" task deny "*"
assert_permission_rule "$gate_json" bash allow "*"
for pattern in 'git *commit*' 'git *push*' 'git *merge*' 'gh pr merge*' 'gh * pr merge*'; do
  assert_permission_rule "$gate_json" bash deny "$pattern"
  jq -e --arg pattern "$pattern" '
    [.permission | to_entries[] | select(.value.permission == "bash") | {index: .key, action: .value.action, pattern: .value.pattern}] as $rules |
    ([ $rules[] | select(.action == "allow" and .pattern == "*") | .index ] | max) as $broad_allow |
    any($rules[]; .action == "deny" and .pattern == $pattern and .index > $broad_allow)
  ' "$gate_json" >/dev/null || fail "gate deny $pattern must follow broad allow"
done
[ "$(bash_action "$gate_json" "gh -R KirkDiggler/rpg-project pr merge 103")" = deny ] || fail "gate repo-flag gh pr merge must resolve to the trailing deny"

explore_json="$(mktemp)"
tmpfiles+=("$explore_json")
opencode debug agent explore >"$explore_json"
assert_permission_rule "$explore_json" edit deny "*"

board_skill=".opencode/skills/project-board-workflow/SKILL.md"
require_file "$board_skill"
grep -Fqx -- '- Same-repository backing issue: `Closes #<issue-number>`.' "$board_skill" || fail "board skill lacks the same-repository closing form"
grep -Fqx -- '- Cross-repository backing issue: `Closes <owner>/<repository>#<issue-number>`.' "$board_skill" || fail "board skill lacks the cross-repository closing form"
grep -Fqx -- 'After a human merge, wait for GitHub to close the backing issue and Project 19' "$board_skill" || fail "board skill lacks the wait-before-reconcile rule"
grep -Fqx -- 'automation to move the item to Done before manually reconciling fields. Repair' "$board_skill" || fail "board skill lacks the automation completion rule"
grep -Fqx -- 'only fields automation failed to reconcile; do not preempt or duplicate the' "$board_skill" || fail "board skill lacks the limited manual-repair rule"
grep -Fqx -- 'automated transition.' "$board_skill" || fail "board skill lacks the no-preemption rule"
[ "$(grep -Fxc -- 'only fields automation failed to reconcile; do not preempt or duplicate the' "$board_skill")" -eq 1 ] || fail "board skill duplicates the manual-repair rule"
same_repo_keyword='Closes #106'
cross_repo_keyword='Closes KirkDiggler/rpg-api#682'
invalid_same_repo_keyword='Closes KirkDiggler/rpg-project#106'
invalid_cross_repo_keyword='Closes #106'
[[ "$same_repo_keyword" =~ ^Closes\ #[1-9][0-9]*$ ]] || fail "same-repository closing keyword does not discriminate"
[[ "$cross_repo_keyword" =~ ^Closes\ [A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+#[1-9][0-9]*$ ]] || fail "cross-repository closing keyword does not discriminate"
[[ ! "$invalid_same_repo_keyword" =~ ^Closes\ #[1-9][0-9]*$ ]] || fail "same-repository keyword accepts cross-repository form"
[[ ! "$invalid_cross_repo_keyword" =~ ^Closes\ [A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+#[1-9][0-9]*$ ]] || fail "cross-repository keyword accepts same-repository form"

printf "PASS: OpenCode project configuration verified without a model call\n"
