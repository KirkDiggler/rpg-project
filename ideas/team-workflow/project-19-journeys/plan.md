# Project 19 Global Journey Starters Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Execute shared Project 19 mutations inline and sequentially; do not fan live board writes out to subagents.

**Goal:** Make useful journey starters discoverable across Initiative boundaries, seed four approved player-promise journeys, and configure the existing views for global pickup and parent-journey navigation.

**Architecture:** GitHub remains the sole durable collaboration state. Preserve the existing Project schema, change only three view filters plus the Project README, use top-level Shaping journeys for unadopted capabilities, and parent only the two already required by initiative #231. Every logical live-resource batch receives a direct readback before work moves to the next resource.

**Tech Stack:** GitHub Projects v2 GraphQL API, GitHub Issues REST/GraphQL APIs, `gh`, `jq`, Python 3

**Spec:** `ideas/team-workflow/project-19-journeys/design.md`

**Completed predecessor:** `ideas/team-workflow/project-19-journeys/pilot-plan.md`

## Global Constraints

- Project 19 (`PVT_kwHOAASbwc4Bcj4v`) remains the sole durable board.
- Initiative is a priority lens, not a permission boundary.
- Do not add fields, field options, views, roadmap dates, automation, or gameplay implementation.
- Create exactly four journey issues; create no child slices during this rollout.
- Existing repository trackers remain linked evidence. Do not reparent, rewrite, or close them.
- Set Status `Todo`, Readiness `Shaping`, Team `Cross-team`, and no assignee on all four starters.
- Set Area `Class Kits` on resources, leveling, and spellcasting; set Area `The Dungeon` on dungeon danger.
- Assign Initiative `Four-player Level-3 Dungeon` and parent #231 only to resources and leveling.
- Keep Kind blank on all journey issues.
- PR #230 is the merged predecessor. Keep reopened rpg-project issue #229 and follow-up PR #245 open through a real collaborator checkpoint; Kirk alone merges #245.
- The active implementation path remains `rpg-toolkit → rpg-api → rpg-dnd5e-web`, with `rpg-api-protos` only when a wire change is earned.
- `dnd-bot-discord` is archival and supplies no current requirement.
- Agent-authored GitHub comments end with `— asset-pipeline agent, on behalf of KirkDiggler` until the team-workflow signature design in PR #238 is ratified and implemented.
- Stop on a rate-limit warning, mutation error, or mismatched readback. Do not continue through partial state.

## Live IDs

```text
Project:     PVT_kwHOAASbwc4Bcj4v
Status:      PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM
  Todo:      397864df
Area:        PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s
  Class Kits: 5cbbab59
  The Dungeon: 0e68c572
Team:        PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs
  Cross-team: a24c4f05
Initiative:  PVTSSF_lAHOAASbwc4Bcj4vzhf2z3s
  Four-player Level-3 Dungeon: 20c80cbf
Readiness:   PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y
  Shaping:   9f972e92
Kind:        PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w

View 5 Current Initiative:      PVTV_lAHOAASbwc4Bcj4vzgLXUnU
View 6 Ready Journeys:          PVTV_lAHOAASbwc4Bcj4vzgLXUno
View 7 Active Journeys:         PVTV_lAHOAASbwc4Bcj4vzgLXUns
View 8 Discovery and Concepts:  PVTV_lAHOAASbwc4Bcj4vzgLXUnw
View 9 Shelf:                   PVTV_lAHOAASbwc4Bcj4vzgLXUn0
```

## Rollback boundary

The baseline files are the rollback authority. If Kirk chooses rollback after a
partial mutation:

- restore README and filters from `project-before.json` / `views-before.json`;
- use **Slice by → No slicing** on views 7 and 9 through the browser;
- restore #201 and #231 bodies from their `issue-*-before.json` files;
- remove the two adopted starters from #231 with `removeSubIssue`, then remove
  all four new Project items;
- close the four issues as `NOT_PLANNED` with a signed rollback explanation;
  GitHub issues are never deleted; and
- verify the restored state before resuming any forward step.

Do not initiate rollback merely because a later step stopped; preserve the
partial state and evidence until Kirk chooses forward repair or rollback.

---

### Task 1: Snapshot and revise the existing contributor views

**Live resources:** Project 19 README and views 5–9.

**Produces:** Global Ready/Discovery filters, child-slice Active filter, revised contributor contract, and persisted Slice-by settings verified by browser refresh; the public-API observability gap is captured as evidence.

- [x] **Step 1: Verify quota and capture a durable pre-mutation snapshot**

```bash
set -euo pipefail
cache="$HOME/.cache/pi/project19-journey-starters"
mkdir -p "$cache"

gh api rate_limit > "$cache/rate-before.json"
jq -e '.resources.graphql.remaining >= 2000' "$cache/rate-before.json" >/dev/null

gh project view 19 --owner KirkDiggler --format json > "$cache/project-before.json"
gh project field-list 19 --owner KirkDiggler --format json > "$cache/fields-before.json"
gh project item-list 19 --owner KirkDiggler --limit 1000 --format json > "$cache/items-before.json"
gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2{views(first:100){nodes{id number name layout filter verticalGroupByFields(first:10){nodes{... on ProjectV2FieldCommon{id name}}} groupByFields(first:10){nodes{... on ProjectV2FieldCommon{id name}}} configuration{visibleFields(first:50){nodes{... on ProjectV2FieldCommon{id name}}}}}}}}}' -f id='PVT_kwHOAASbwc4Bcj4v' > "$cache/views-before.json"
for n in 201 229 231; do gh api "repos/KirkDiggler/rpg-project/issues/$n" > "$cache/issue-$n-before.json"; done
gh api repos/KirkDiggler/rpg-project/pulls/230 > "$cache/pr-230-before.json"
sha256sum "$cache"/*-before.json > "$cache/baseline.sha256"
```

Expected: the quota assertion passes, every file is non-empty, and `baseline.sha256` records all captured inputs.

- [x] **Step 2: Prove the live schema IDs still match the design**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
jq -e '
  (.fields[]|select(.name=="Status")|.id)=="PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM" and
  (.fields[]|select(.name=="Area")|.id)=="PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s" and
  (.fields[]|select(.name=="Team")|.id)=="PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs" and
  (.fields[]|select(.name=="Initiative")|.id)=="PVTSSF_lAHOAASbwc4Bcj4vzhf2z3s" and
  (.fields[]|select(.name=="Readiness")|.id)=="PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y" and
  (.fields[]|select(.name=="Kind")|.id)=="PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w" and
  ((.fields[]|select(.name=="Status")|.options[]|select(.name=="Todo")|.id)=="397864df") and
  ((.fields[]|select(.name=="Area")|.options[]|select(.name=="Class Kits")|.id)=="5cbbab59") and
  ((.fields[]|select(.name=="Area")|.options[]|select(.name=="The Dungeon")|.id)=="0e68c572") and
  ((.fields[]|select(.name=="Team")|.options[]|select(.name=="Cross-team")|.id)=="a24c4f05") and
  ((.fields[]|select(.name=="Initiative")|.options[]|select(.name=="Four-player Level-3 Dungeon")|.id)=="20c80cbf") and
  ((.fields[]|select(.name=="Readiness")|.options[]|select(.name=="Shaping")|.id)=="9f972e92")
' "$cache/fields-before.json" >/dev/null
```

Expected: exit 0. Stop rather than resolving replacement IDs silently.

- [x] **Step 3: Write the revised Project README payload**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
cat > "$cache/project-readme-expanded.md" <<'EOF'
# The evolving game — how Project 19 is organized

Project 19 is the durable board for game development. The current initiative is **Four-player Level-3 Dungeon**, but Initiative is a priority lens rather than a permission boundary. Useful capabilities may be shaped, proven, or built before an Initiative adopts them.

## Work shapes

Adopted work uses **Initiative → Journey → Slice issue → linked PR**. An unadopted capability may remain a top-level Journey starter and gain a concrete Slice when evidence earns one.

- **Initiative** — an adopted strategic outcome.
- **Journey** — a player-visible or operator-visible multi-PR capability.
- **Slice** — one coherent merge unit in its owning repository, with exactly one Kind.

The journey assignee holds the baton. A merged slice does not automatically finish its journey.

## Finding work

- **Ready Journeys** is the global pickup view: Todo, Ready, and unassigned, regardless of Initiative.
- **Current Initiative** shows the journeys prioritized by the adopted north star.
- **Active Journeys** shows current child slices; use the Parent issue rail to select a journey.
- **Discovery and Concepts** shows Learn, Decide, and Concept slices globally.
- **Shelf** keeps work with no Initiative visible; use the Readiness rail to find Shaping journey starters.

A blank Initiative means “not currently prioritized,” not “invalid” or “forbidden.”

## Journey maturity

A Shaping starter contains a player promise, rough shape, verified current reality, ways in, a possibility shelf, and a learning log. Ways in are invitations, not promised child issues. A possibility is not accepted policy.

Ready means one immediate proof is trustworthy and claimable. Shaping work may still be claimed or gain a concrete Learn, Decide, Concept, Build, Fix, or Verify slice.

## Fields

- **Initiative** — current strategic priority.
- **Area** — broad product area; a filter, not a schedule.
- **Team** — outcome perspective and ownership lens.
- **Kind** — Learn / Decide / Concept / Build / Fix / Verify on slices.
- **Readiness** — journey-only Shaping / Ready / Blocked.
- **Status** — Todo / In Progress / In Review / Done.

`Area = Shelf` is legacy. Do not assign it to new work.

## Evidence

Toolkit tests can prove a toolkit slice. A Concept proves a desired experience or consumer contract, not production integration. A gameplay journey closes only when its behavior is observed through its named real game path.

Canonical design and review: [rpg-project PR #230](https://github.com/KirkDiggler/rpg-project/pull/230), `ideas/team-workflow/project-19-journeys/design.md`.
EOF
```

Expected: the README describes global pickup, Shaping starters, and Initiative as priority.

- [x] **Step 4: Update the README and read it back immediately**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
gh api graphql \
  -f query='mutation($project:ID!,$readme:String!){updateProjectV2(input:{projectId:$project,readme:$readme}){projectV2{id readme}}}' \
  -f project='PVT_kwHOAASbwc4Bcj4v' \
  -F readme=@"$cache/project-readme-expanded.md" \
  > "$cache/readme-update.json"

python3 - <<'PY'
import json
from pathlib import Path
cache = Path.home()/'.cache/pi/project19-journey-starters'
actual = json.loads((cache/'readme-update.json').read_text())['data']['updateProjectV2']['projectV2']['readme']
expected = (cache/'project-readme-expanded.md').read_text()
assert actual.rstrip('\n') == expected.rstrip('\n')
PY
```

Expected: exact equality after normalizing GitHub's terminal newline behavior.

- [x] **Step 5: Update the three changed filters sequentially**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
update_view() {
  local id="$1" filter="$2" out="$3"
  gh api graphql \
    -f query='mutation($id:ID!,$filter:String!){updateProjectV2View(input:{viewId:$id,filter:$filter}){projectV2View{id name filter}}}' \
    -f id="$id" -f filter="$filter" > "$out"
  test "$(jq -r '.data.updateProjectV2View.projectV2View.filter' "$out")" = "$filter"
}

update_view 'PVTV_lAHOAASbwc4Bcj4vzgLXUno' \
  'label:journey status:Todo readiness:Ready no:assignee' \
  "$cache/view-6-update.json"
update_view 'PVTV_lAHOAASbwc4Bcj4vzgLXUns' \
  'has:parent-issue kind:Build,Fix,Verify,Learn,Decide,Concept -status:Done' \
  "$cache/view-7-update.json"
update_view 'PVTV_lAHOAASbwc4Bcj4vzgLXUnw' \
  'kind:Learn,Decide,Concept -status:Done' \
  "$cache/view-8-update.json"
```

Expected: each mutation returns its exact requested filter before the next mutation runs.

- [x] **Step 6: Verify all five filters and unchanged visible fields**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2{views(first:100){nodes{number name layout filter verticalGroupByFields(first:10){nodes{... on ProjectV2FieldCommon{name}}} configuration{visibleFields(first:50){nodes{... on ProjectV2FieldCommon{name}}}}}}}}}' -f id='PVT_kwHOAASbwc4Bcj4v' > "$cache/views-filter-readback.json"

jq -e '
  ([.data.node.views.nodes[]|select(.number==5)|.filter]==["initiative:\"Four-player Level-3 Dungeon\" label:journey"]) and
  ([.data.node.views.nodes[]|select(.number==6)|.filter]==["label:journey status:Todo readiness:Ready no:assignee"]) and
  ([.data.node.views.nodes[]|select(.number==7)|.filter]==["has:parent-issue kind:Build,Fix,Verify,Learn,Decide,Concept -status:Done"]) and
  ([.data.node.views.nodes[]|select(.number==8)|.filter]==["kind:Learn,Decide,Concept -status:Done"]) and
  ([.data.node.views.nodes[]|select(.number==9)|.filter]==["no:initiative -status:Done"]) and
  all(.data.node.views.nodes[]|select(.number>=5 and .number<=9);
    .layout=="TABLE_LAYOUT" and
    ([.configuration.visibleFields.nodes[].name]|contains(["Title","Assignees","Status","Linked pull requests","Parent issue","Sub-issues progress","Area","Kind","Team","Initiative","Readiness"])))
' "$cache/views-filter-readback.json" >/dev/null
```

Expected: exit 0 and no visible field disappears.

- [x] **Step 7: Set the two Slice-by fields in the GitHub browser UI**

The public `UpdateProjectV2ViewInput` has no Slice-by property; its nested configuration input exposes only `visibleFieldIds`. In Project 19:

1. Open **Active Journeys** (`/users/KirkDiggler/projects/19/views/7`).
2. Open the view menu, choose **Slice by**, choose **Parent issue**, and save the view.
3. Open **Shelf** (`/users/KirkDiggler/projects/19/views/9`).
4. Open the view menu, choose **Slice by**, choose **Readiness**, and save the view.

Expected: Active Journeys gains a Parent issue rail; Shelf gains a Readiness rail. Stop until Kirk confirms both browser actions.

- [x] **Step 8: Capture browser persistence and the public-API observability gap**

Kirk confirmed both settings remained after refresh on 2026-08-21. Reordering the tabs changed only their positions: Active Journeys is first and Shelf is fourth; their stable IDs and names are unchanged.

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2{views(first:100){nodes{id number name updatedAt}}}} viewType:__type(name:"ProjectV2View"){fields{name}} updateConfigurationType:__type(name:"ProjectV2ViewConfigurationInput"){inputFields{name}}}' -f id='PVT_kwHOAASbwc4Bcj4v' > "$cache/views-slice-api-evidence.json"

jq -e '
  ([.data.node.views.nodes[]|select(.id=="PVTV_lAHOAASbwc4Bcj4vzgLXUns")|.name]==["Active Journeys"]) and
  ([.data.node.views.nodes[]|select(.id=="PVTV_lAHOAASbwc4Bcj4vzgLXUn0")|.name]==["Shelf"]) and
  ([.data.viewType.fields[].name|select(test("slice";"i"))]|length)==0 and
  ([.data.updateConfigurationType.inputFields[].name]==["visibleFieldIds"])
' "$cache/views-slice-api-evidence.json" >/dev/null

cat > "$cache/views-slice-browser-verification.txt" <<'EOF'
2026-08-21 — Kirk set Active Journeys to Slice by Parent issue and Shelf to Slice by Readiness, saved both, and confirmed both persisted after refresh. Active is first and Shelf fourth after tab reordering. GitHub's public ProjectV2View schema exposes no Slice-by field; groupByFields and verticalGroupByFields are different settings and remain empty.
EOF
```

Expected: stable view identities and the API gap are captured; Kirk's persisted-after-refresh confirmation is the Slice-by acceptance evidence.

---

### Task 2: Create the two current-Initiative journey starters

**Live resources:** New rpg-project issues, Project items, initiative #231.

**Consumes:** Task 1's verified field IDs and contributor contract.

**Produces:** Two Todo/Shaping/Cross-team/Class Kits journeys parented under #231, with no assignee, Kind, or child slices.

- [x] **Step 1: Write the class-resource journey body**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
cat > "$cache/resources-journey.md" <<'EOF'
## Player promise / Outcome

As a player with limited class capabilities, I can see the resources available to me, spend them through every supported ability that uses them, receive a clear refusal when a pool is empty, and see recovery reflected through the real game path.

## Why now

The Four-player Level-3 Dungeon requires every supported level 1–3 ability for Barbarian/Berserker, Fighter/Champion, Monk/Open Hand, and Rogue/Thief to be usable. Monk Ki is a shared pool consumed by several abilities, making resource truth a current Initiative capability rather than a UI ornament.

## Rough shape

- One authoritative current/maximum value is shared by every consumer of a named resource.
- Ability activation checks and spends that value; the client renders resulting truth rather than calculating it.
- Recovery follows an explicit game boundary and appears without reconnect tricks.
- Spell slots are a future seam, not a decision this journey must make.

## Current reality

- Rage is a working baseline in rpg-toolkit: activation refuses an empty pool, consumes one `rage_charges`, persists the decrement, and emits `ResourceChanged`.
- rpg-toolkit#795 restored tracked pools at new-encounter seating for the arcade loop.
- rpg-toolkit#1087 made economy and pool mutation mark the character sheet dirty so session write-back does not discard a spend.
- rpg-toolkit#453 contains useful Monk level-2 intent but predates the current grants, resource, encounter, and session composition; its implementation claims must be rechecked.
- rpg-toolkit#39 is historical restoration design input, not an accepted migration plan.

## Ways in

- Trace Rage's current/maximum value and `ResourceChanged` from toolkit through API projection to the game UI, recording the first missing seam.
- Reconcile one level-2 Monk ability against the current grants and session/resolution path, then prove two Ki consumers observe the same pool.
- Build a fixture-driven resource-display concept that distinguishes available, spent, exhausted, and recovered states without inventing client-side rules.

## Possibility shelf

- whether spell slots eventually share the generic resource substrate;
- short-rest and long-rest interactions beyond the arcade run boundary;
- authored recovery triggers or items that restore part of a pool; and
- resource history, warnings, or planning UI beyond current/maximum truth.

## Learning log

- 2026-08-21 — Created as a Shaping journey. Rage is evidence; Ki is the first shared-pool example. No implementation child was created.

Board model: rpg-project#229 / PR #230. Initiative: rpg-project#231.

— asset-pipeline agent, on behalf of KirkDiggler
EOF
```

Expected: the body offers several ways in without selecting a child slice.

- [x] **Step 2: Create and read back the resource issue**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
jq -n --rawfile body "$cache/resources-journey.md" \
  '{title:"Journey: See, Spend, and Recover Class Resources",body:$body,labels:["journey"]}' \
  | gh api -X POST repos/KirkDiggler/rpg-project/issues --input - \
  > "$cache/resources-created.json"

jq -e '.state=="open" and .title=="Journey: See, Spend, and Recover Class Resources" and ([.labels[].name]|contains(["journey"]))' "$cache/resources-created.json" >/dev/null
RESOURCE_NUMBER=$(jq -r .number "$cache/resources-created.json")
RESOURCE_URL=$(jq -r .html_url "$cache/resources-created.json")
printf 'RESOURCE_NUMBER=%q\nRESOURCE_URL=%q\n' "$RESOURCE_NUMBER" "$RESOURCE_URL" > "$cache/resources.env"
```

Expected: one open journey-labeled issue and a persisted number/URL.

- [x] **Step 3: Board and parent the resource journey**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/resources.env"
gh project item-add 19 --owner KirkDiggler --url "$RESOURCE_URL" --format json > "$cache/resources-item.json"
RESOURCE_ITEM=$(jq -r .id "$cache/resources-item.json")

set_select() {
  gh project item-edit --id "$RESOURCE_ITEM" --project-id 'PVT_kwHOAASbwc4Bcj4v' \
    --field-id "$1" --single-select-option-id "$2" >/dev/null
}
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM' '397864df'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s' '5cbbab59'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs' 'a24c4f05'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhf2z3s' '20c80cbf'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y' '9f972e92'

gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2Item{fieldValues(first:50){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2FieldCommon{name}}}}}}}}' -f id="$RESOURCE_ITEM" > "$cache/resources-fields-readback.json"
jq -e '([.data.node.fieldValues.nodes[]|select(.field.name? != null)|{key:.field.name,value:.name}]|from_entries) as $f | $f.Status=="Todo" and $f.Area=="Class Kits" and $f.Team=="Cross-team" and $f.Initiative=="Four-player Level-3 Dungeon" and $f.Readiness=="Shaping" and ($f.Kind//null)==null' "$cache/resources-fields-readback.json" >/dev/null

PARENT_ID=$(jq -r .node_id "$cache/issue-231-before.json")
gh api graphql \
  -f query='mutation($parent:ID!,$url:String!){addSubIssue(input:{issueId:$parent,subIssueUrl:$url}){issue{number} subIssue{number parent{number}}}}' \
  -f parent="$PARENT_ID" -f url="$RESOURCE_URL" \
  > "$cache/resources-parented.json"
jq -e '.data.addSubIssue.issue.number==231 and .data.addSubIssue.subIssue.parent.number==231' "$cache/resources-parented.json" >/dev/null
```

Expected: all five select mutations succeed in order and direct GraphQL reports parent #231.

- [x] **Step 4: Write and create the leveling journey**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
cat > "$cache/leveling-journey.md" <<'EOF'
## Player promise / Outcome

As a player whose character has earned enough persisted XP, I can see that a level is available on the start screen, open the existing Character Sheet, complete the required choices, and return later with the new level and grants intact.

## Why now

The Four-player Level-3 Dungeon explicitly requires persistent monster-kill XP and an outside-dungeon path through level 3 for Barbarian/Berserker, Fighter/Champion, Monk/Open Hand, and Rogue/Thief.

## Rough shape

- XP is authoritative and persists across dungeon runs.
- The selected-character panel advertises `Level Up Available` and leads to the existing Character Sheet.
- Leveling is an explicit between-run choice; an eligible character may keep playing without leveling.
- The toolkit owns thresholds, grants, and choice validity; API stores/projects; web guides and renders.
- Reloading proves the transition rather than relying on in-memory UI state.

## Current reality

- Current web `origin/dev` already has Home → select character → View Sheet. The sheet displays level and XP but no level-up interaction.
- rpg-toolkit#428 records the broad XP/leveling idea but predates the composable session and current class-grant architecture.
- rpg-api#79 is a stale cross-repository umbrella and references archival dnd-bot-discord; it supplies history, not requirements.
- rpg-dnd5e-web#31 proposes a much larger progression product, including trees, timelines, and multiclass planning that are not required here.
- rpg-api-protos#183 records a character-data projection decision relevant to where level/XP truth is fetched, but does not settle this flow.

## Ways in

- Trace the authoritative XP, level, and class-choice state across toolkit, API storage, and the existing Character Sheet; record the first absent transition.
- Create a fixture-driven concept for the start-screen eligibility signal and one simple level transition.
- Reconcile one level-1-to-2 path for a supported class against current grants before generalizing through level 3.

## Possibility shelf

- equal versus divided monster XP;
- authored fight-clear, objective, or dungeon-completion bonuses;
- average versus rolled HP gain;
- richer progression trees or character history;
- multiclassing; and
- warnings or recommendations tied to dungeon danger.

## Learning log

- 2026-08-21 — Created as a Shaping journey. The existing Character Sheet is the chosen home; leveling remains optional and between runs.

Board model: rpg-project#229 / PR #230. Initiative: rpg-project#231.

— asset-pipeline agent, on behalf of KirkDiggler
EOF

jq -n --rawfile body "$cache/leveling-journey.md" \
  '{title:"Journey: Earn XP and Level Up Between Runs",body:$body,labels:["journey"]}' \
  | gh api -X POST repos/KirkDiggler/rpg-project/issues --input - \
  > "$cache/leveling-created.json"

jq -e '.state=="open" and .title=="Journey: Earn XP and Level Up Between Runs" and ([.labels[].name]|contains(["journey"]))' "$cache/leveling-created.json" >/dev/null
LEVELING_NUMBER=$(jq -r .number "$cache/leveling-created.json")
LEVELING_URL=$(jq -r .html_url "$cache/leveling-created.json")
printf 'LEVELING_NUMBER=%q\nLEVELING_URL=%q\n' "$LEVELING_NUMBER" "$LEVELING_URL" > "$cache/leveling.env"
```

Expected: one open journey-labeled issue with the approved optional between-run flow.

- [x] **Step 5: Board and parent the leveling journey**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/leveling.env"
gh project item-add 19 --owner KirkDiggler --url "$LEVELING_URL" --format json > "$cache/leveling-item.json"
LEVELING_ITEM=$(jq -r .id "$cache/leveling-item.json")

set_select() {
  gh project item-edit --id "$LEVELING_ITEM" --project-id 'PVT_kwHOAASbwc4Bcj4v' \
    --field-id "$1" --single-select-option-id "$2" >/dev/null
}
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM' '397864df'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s' '5cbbab59'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs' 'a24c4f05'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhf2z3s' '20c80cbf'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y' '9f972e92'

gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2Item{fieldValues(first:50){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2FieldCommon{name}}}}}}}}' -f id="$LEVELING_ITEM" > "$cache/leveling-fields-readback.json"
jq -e '([.data.node.fieldValues.nodes[]|select(.field.name? != null)|{key:.field.name,value:.name}]|from_entries) as $f | $f.Status=="Todo" and $f.Area=="Class Kits" and $f.Team=="Cross-team" and $f.Initiative=="Four-player Level-3 Dungeon" and $f.Readiness=="Shaping" and ($f.Kind//null)==null' "$cache/leveling-fields-readback.json" >/dev/null

PARENT_ID=$(jq -r .node_id "$cache/issue-231-before.json")
gh api graphql \
  -f query='mutation($parent:ID!,$url:String!){addSubIssue(input:{issueId:$parent,subIssueUrl:$url}){issue{number} subIssue{number parent{number}}}}' \
  -f parent="$PARENT_ID" -f url="$LEVELING_URL" \
  > "$cache/leveling-parented.json"
jq -e '.data.addSubIssue.issue.number==231 and .data.addSubIssue.subIssue.parent.number==231' "$cache/leveling-parented.json" >/dev/null
```

Expected: direct GraphQL reports parent #231.

- [x] **Step 6: Update initiative #231's child list without replacing other content**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/resources.env"
source "$cache/leveling.env"
gh api repos/KirkDiggler/rpg-project/issues/231 --jq .body > "$cache/initiative-231-current.md"

RESOURCE_NUMBER="$RESOURCE_NUMBER" LEVELING_NUMBER="$LEVELING_NUMBER" python3 - <<'PY'
import os
from pathlib import Path
cache = Path.home()/'.cache/pi/project19-journey-starters'
body = (cache/'initiative-231-current.md').read_text()
old = '''## Pilot journeys

- [ ] Composable Dungeon Builder
- [ ] Composable Attack Damage
- [ ] Monster Behavior in the Local Dungeon

Additional journeys are added only after current-architecture reconciliation.'''
new = f'''## Journey children

- [ ] #169 — Composable Dungeon Builder
- [ ] #232 — Composable Attack Damage
- [ ] #201 — Monster Behavior in the Local Dungeon
- [ ] #236 — Multi-contributor agent workspace
- [ ] #{os.environ["RESOURCE_NUMBER"]} — See, Spend, and Recover Class Resources
- [ ] #{os.environ["LEVELING_NUMBER"]} — Earn XP and Level Up Between Runs

Additional journeys are adopted only after current-reality reconciliation.'''
assert body.count(old) == 1
body = body.replace(old, new)
assert body.count('- a dungeon-completion XP reward;') == 1
body = body.replace('- a dungeon-completion XP reward;', '- fight-clear or dungeon-completion XP rewards;')
log = '- 2026-08-20 — Initiative adopted. Local dev is the completion environment; hosted dev remains a later journey.'
assert body.count(log) == 1
body = body.replace(log, log + '\n- 2026-08-21 — Added class-resource and between-run leveling Shaping journeys; no child slices were predicted.')
(cache/'initiative-231-expanded.md').write_text(body)
PY

gh api -X PATCH repos/KirkDiggler/rpg-project/issues/231 \
  -F body=@"$cache/initiative-231-expanded.md" \
  > "$cache/initiative-231-update.json"
test "$(gh api repos/KirkDiggler/rpg-project/issues/231 --jq .body)" = "$(< "$cache/initiative-231-expanded.md")"
```

Expected: only the child list, XP exclusion wording, and Learning log gain approved changes.

- [x] **Step 7: Verify hierarchy, board fields, assignments, and Initiative preservation**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/resources.env"
source "$cache/leveling.env"
gh api graphql \
  -F resource="$RESOURCE_NUMBER" -F leveling="$LEVELING_NUMBER" \
  -f query='query($resource:Int!,$leveling:Int!){repository(owner:"KirkDiggler",name:"rpg-project"){initiative:issue(number:231){subIssues(first:20){totalCount nodes{number}}} resource:issue(number:$resource){number title parent{number} subIssues(first:10){totalCount} assignees(first:10){nodes{login}} labels(first:10){nodes{name}}} leveling:issue(number:$leveling){number title parent{number} subIssues(first:10){totalCount} assignees(first:10){nodes{login}} labels(first:10){nodes{name}}}}}' \
  > "$cache/current-starters-hierarchy.json"

gh project item-list 19 --owner KirkDiggler --limit 1000 --format json > "$cache/items-after-current-starters.json"

jq -e --argjson r "$RESOURCE_NUMBER" --argjson l "$LEVELING_NUMBER" '
  (([169,201,232,236,$r,$l] - [.data.repository.initiative.subIssues.nodes[].number])|length)==0 and
  .data.repository.resource.parent.number==231 and
  .data.repository.leveling.parent.number==231 and
  .data.repository.resource.subIssues.totalCount==0 and
  .data.repository.leveling.subIssues.totalCount==0 and
  (.data.repository.resource.assignees.nodes|length)==0 and
  (.data.repository.leveling.assignees.nodes|length)==0 and
  ([.data.repository.resource.labels.nodes[].name]|contains(["journey"])) and
  ([.data.repository.leveling.labels.nodes[].name]|contains(["journey"]))
' "$cache/current-starters-hierarchy.json" >/dev/null

jq -e --argjson r "$RESOURCE_NUMBER" --argjson l "$LEVELING_NUMBER" '
  ([.items[]|select(.content.repository=="KirkDiggler/rpg-project" and (.content.number==$r or .content.number==$l))]|length)==2 and
  all(.items[]|select(.content.repository=="KirkDiggler/rpg-project" and (.content.number==$r or .content.number==$l));
    .status=="Todo" and .readiness=="Shaping" and .team=="Cross-team" and .area=="Class Kits" and
    .initiative=="Four-player Level-3 Dungeon" and (.kind//null)==null and (.assignees|length)==0)
' "$cache/items-after-current-starters.json" >/dev/null
```

Expected: both new journeys are correct, the original pilot children and concurrent #236 remain present, and unrelated Initiative additions do not make this targeted check fail.

---

### Task 3: Create the two unadopted journey starters

**Live resources:** Two new top-level rpg-project issues and Project items.

**Consumes:** Task 1's global views and Task 2's established issue-body pattern.

**Produces:** Spellcasting and dungeon-danger starters with no Initiative, parent, assignee, Kind, or children.

- [x] **Step 1: Write and create the spellcasting starter**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
cat > "$cache/spellcasting-journey.md" <<'EOF'
## Player promise / Outcome

As a player with a spellcasting capability, I can choose a supported spell, choose valid targets, and see its rules-authored result through the real game path so caster and spell-using subclass options can open incrementally.

## Rough shape

- The first promising vertical proof is one at-will cantrip moving through selection, targeting, resolution, and a visible result.
- The toolkit owns spell rules and outcomes, the API projects authoritative state, and the web renders choices and results.
- The journey proves a reusable casting path, not a complete class catalog.
- Spell-slot architecture is deliberately not required by the cantrip proof.

## Current reality

- rpg-toolkit#146 records old known-spell/cantrip processing assumptions that must be compared with current character grants and persistence.
- rpg-toolkit#431 describes broad combat spellcasting but predates the composable session and resolution architecture.
- rpg-toolkit#799 is newer evidence that finalized spell slots are orphaned: they are persisted but have no live spend/recovery path. Its proposed migration is not yet an accepted decision.
- rpg-api-protos#30 and rpg-api#120 predict comprehensive spell-management RPCs and service behavior; that breadth is evidence, not the required first shape.
- rpg-dnd5e-web#95 and rpg-dnd5e-web#99 record spell-selection UX intent from the older choice flow.

## Ways in

- Inventory the current spell, cantrip, action, target, and resolution representations and record which old trackers remain true.
- Express one at-will cantrip against the current session/resolution seams with an executable toolkit contract.
- Build a fixture-driven concept for choosing a cantrip and valid target while consuming server-authored outcomes.

## Possibility shelf

- whether spell slots share the class-resource substrate or remain distinct;
- slot spending and recovery;
- upcasting;
- concentration and interruption;
- areas of effect;
- prepared versus known spell management;
- spellbooks and full class lists; and
- additional spellcasting classes and subclasses.

## Learning log

- 2026-08-21 — Created as an unadopted Shaping journey. One cantrip is the promising way in; slot architecture remains open.

Board model: rpg-project#229 / PR #230. No Initiative is currently assigned.

— asset-pipeline agent, on behalf of KirkDiggler
EOF

jq -n --rawfile body "$cache/spellcasting-journey.md" \
  '{title:"Journey: Cast a Spell in Play",body:$body,labels:["journey"]}' \
  | gh api -X POST repos/KirkDiggler/rpg-project/issues --input - \
  > "$cache/spellcasting-created.json"

jq -e '.state=="open" and .title=="Journey: Cast a Spell in Play" and ([.labels[].name]|contains(["journey"]))' "$cache/spellcasting-created.json" >/dev/null
SPELL_NUMBER=$(jq -r .number "$cache/spellcasting-created.json")
SPELL_URL=$(jq -r .html_url "$cache/spellcasting-created.json")
printf 'SPELL_NUMBER=%q\nSPELL_URL=%q\n' "$SPELL_NUMBER" "$SPELL_URL" > "$cache/spellcasting.env"
```

Expected: the issue is useful without accepting a slot model or creating a child.

- [x] **Step 2: Board spellcasting without assigning Initiative or parent**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/spellcasting.env"
gh project item-add 19 --owner KirkDiggler --url "$SPELL_URL" --format json > "$cache/spellcasting-item.json"
SPELL_ITEM=$(jq -r .id "$cache/spellcasting-item.json")

set_select() {
  gh project item-edit --id "$SPELL_ITEM" --project-id 'PVT_kwHOAASbwc4Bcj4v' \
    --field-id "$1" --single-select-option-id "$2" >/dev/null
}
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM' '397864df'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s' '5cbbab59'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs' 'a24c4f05'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y' '9f972e92'

gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2Item{fieldValues(first:50){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2FieldCommon{name}}}}}}}}' -f id="$SPELL_ITEM" > "$cache/spellcasting-fields-readback.json"
jq -e '([.data.node.fieldValues.nodes[]|select(.field.name? != null)|{key:.field.name,value:.name}]|from_entries) as $f | $f.Status=="Todo" and $f.Area=="Class Kits" and $f.Team=="Cross-team" and $f.Readiness=="Shaping" and ($f.Initiative//null)==null and ($f.Kind//null)==null' "$cache/spellcasting-fields-readback.json" >/dev/null
```

Expected: four select mutations only; direct readback shows no Initiative or Kind.

- [x] **Step 3: Write and create the dungeon-danger starter**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
cat > "$cache/dungeon-danger-journey.md" <<'EOF'
## Player promise / Outcome

As a party, we can understand how dangerous available dungeons are for our current group and freely choose an easier, appropriate, harder, or deadly adventure without canonical monsters secretly changing underneath us.

## Rough shape

- A goblin remains the same goblin at every party level.
- Danger guidance is relative to the current party and advisory, never a lock.
- Stronger progression comes from choosing more dangerous dungeons and from encounter composition, layout, bosses, placement, ambushes, and tactics rather than hidden stat inflation.
- Easy play is supported intentionally; rewards from canonical monsters remain truthful even when a party chooses a low-risk run.

## Current reality

- rpg-toolkit#545 contains an older broad dungeon-generator proposal with themes and CR-based composition; its architecture and scope are not adopted.
- rpg-api#295 implemented an earlier API-owned CR-scaling direction that conflicts with the current toolkit-owned rules/content boundary.
- rpg-api#689 delivered deterministic per-region archetype seeding and explicitly deferred CR budgeting, difficulty scaling, and monster pools.
- Current authored dungeon and spawn work provides evidence and seams, but no canonical relative-danger contract exists.

## Ways in

- Inventory the current dungeon-selection surface and the party facts available before a run.
- Build a fixture-driven selection concept showing the same fixed dungeon as relatively different for two party compositions without disabling either choice.
- Prove a deterministic toolkit composition seam can choose different canonical monster rosters without altering a monster stat block.

## Possibility shelf

- labels such as Very Easy, Appropriate, Hard, and Deadly;
- the exact party-strength or encounter-budget formula;
- authored versus generated danger bands;
- monster count, archetype, boss, and room-budget composition;
- Intel/clock-driven ambushes and hidden monsters from the Monster Behavior journey;
- recommended-level warnings; and
- whether optional fight or objective XP rewards communicate danger.

## Learning log

- 2026-08-21 — Created as an unadopted Shaping journey. Player choice and canonical monster identity are fixed; the difficulty formula is not.

Board model: rpg-project#229 / PR #230. No Initiative is currently assigned.

— asset-pipeline agent, on behalf of KirkDiggler
EOF

jq -n --rawfile body "$cache/dungeon-danger-journey.md" \
  '{title:"Journey: Choose a Dungeon by Danger",body:$body,labels:["journey"]}' \
  | gh api -X POST repos/KirkDiggler/rpg-project/issues --input - \
  > "$cache/dungeon-danger-created.json"

jq -e '.state=="open" and .title=="Journey: Choose a Dungeon by Danger" and ([.labels[].name]|contains(["journey"]))' "$cache/dungeon-danger-created.json" >/dev/null
DANGER_NUMBER=$(jq -r .number "$cache/dungeon-danger-created.json")
DANGER_URL=$(jq -r .html_url "$cache/dungeon-danger-created.json")
printf 'DANGER_NUMBER=%q\nDANGER_URL=%q\n' "$DANGER_NUMBER" "$DANGER_URL" > "$cache/dungeon-danger.env"
```

Expected: the issue fixes player choice and canonical monster identity while shelving formulas.

- [x] **Step 4: Board dungeon danger without assigning Initiative or parent**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/dungeon-danger.env"
gh project item-add 19 --owner KirkDiggler --url "$DANGER_URL" --format json > "$cache/dungeon-danger-item.json"
DANGER_ITEM=$(jq -r .id "$cache/dungeon-danger-item.json")

set_select() {
  gh project item-edit --id "$DANGER_ITEM" --project-id 'PVT_kwHOAASbwc4Bcj4v' \
    --field-id "$1" --single-select-option-id "$2" >/dev/null
}
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM' '397864df'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s' '0e68c572'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs' 'a24c4f05'
set_select 'PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y' '9f972e92'

gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2Item{fieldValues(first:50){nodes{... on ProjectV2ItemFieldSingleSelectValue{name field{... on ProjectV2FieldCommon{name}}}}}}}}' -f id="$DANGER_ITEM" > "$cache/dungeon-danger-fields-readback.json"
jq -e '([.data.node.fieldValues.nodes[]|select(.field.name? != null)|{key:.field.name,value:.name}]|from_entries) as $f | $f.Status=="Todo" and $f.Area=="The Dungeon" and $f.Team=="Cross-team" and $f.Readiness=="Shaping" and ($f.Initiative//null)==null and ($f.Kind//null)==null' "$cache/dungeon-danger-fields-readback.json" >/dev/null
```

Expected: four select mutations only; direct readback shows no Initiative or Kind.

- [x] **Step 5: Verify both starters are top-level and correctly shelved**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/spellcasting.env"
source "$cache/dungeon-danger.env"
gh api graphql \
  -F spell="$SPELL_NUMBER" -F danger="$DANGER_NUMBER" \
  -f query='query($spell:Int!,$danger:Int!){repository(owner:"KirkDiggler",name:"rpg-project"){spell:issue(number:$spell){number title parent{number} subIssues(first:10){totalCount} assignees(first:10){nodes{login}} labels(first:10){nodes{name}}} danger:issue(number:$danger){number title parent{number} subIssues(first:10){totalCount} assignees(first:10){nodes{login}} labels(first:10){nodes{name}}}}}' \
  > "$cache/unadopted-starters-hierarchy.json"

gh project item-list 19 --owner KirkDiggler --limit 1000 --format json > "$cache/items-after-unadopted-starters.json"

jq -e '
  .data.repository.spell.parent==null and .data.repository.danger.parent==null and
  .data.repository.spell.subIssues.totalCount==0 and .data.repository.danger.subIssues.totalCount==0 and
  (.data.repository.spell.assignees.nodes|length)==0 and (.data.repository.danger.assignees.nodes|length)==0 and
  ([.data.repository.spell.labels.nodes[].name]|contains(["journey"])) and
  ([.data.repository.danger.labels.nodes[].name]|contains(["journey"]))
' "$cache/unadopted-starters-hierarchy.json" >/dev/null

jq -e --argjson s "$SPELL_NUMBER" --argjson d "$DANGER_NUMBER" '
  ([.items[]|select(.content.repository=="KirkDiggler/rpg-project" and (.content.number==$s or .content.number==$d))]|length)==2 and
  all(.items[]|select(.content.repository=="KirkDiggler/rpg-project" and (.content.number==$s or .content.number==$d));
    .status=="Todo" and .readiness=="Shaping" and .team=="Cross-team" and
    (.initiative//null)==null and (.kind//null)==null and (.assignees|length)==0) and
  ([.items[]|select(.content.repository=="KirkDiggler/rpg-project" and .content.number==$s)|.area]==["Class Kits"]) and
  ([.items[]|select(.content.repository=="KirkDiggler/rpg-project" and .content.number==$d)|.area]==["The Dungeon"])
' "$cache/items-after-unadopted-starters.json" >/dev/null
```

Expected: both have null parents and Initiatives; unrelated concurrent Project additions are preserved.

---

### Task 4: Add ambush and hiding possibilities to Monster Behavior

**Live resource:** rpg-project issue #201.

**Produces:** Updated possibility shelf only; current Next proof, status, hierarchy, and assignment remain unchanged.

- [x] **Step 1: Replace the current shelf block and append one Learning log entry**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
gh api repos/KirkDiggler/rpg-project/issues/201 --jq .body > "$cache/monster-201-current.md"

python3 - <<'PY'
from pathlib import Path
cache = Path.home()/'.cache/pi/project19-journey-starters'
body = (cache/'monster-201-current.md').read_text()
old = '''## Not now / shelves

- a real-time simulation server;
- a complete catalog of personalities;
- patrol, fleeing, hiding, and every targeting strategy in one wave; or
- spellcasting as a prerequisite.'''
new = '''## Possibility shelf

- monsters hiding from party Intel until discovered;
- authored ambushes that use Intel changes and the appropriate world/fight clocks;
- patrol, fleeing, alert, and other intentional behaviors;
- dungeon-authored behavior hints or dispositions; and
- local presentation that lets players understand why a monster made a choice.

These are player-facing possibilities, not a predetermined child backlog.

## Not now

- a real-time simulation server;
- a complete catalog of personalities in one wave; or
- spellcasting as a prerequisite.'''
assert body.count(old) == 1
body = body.replace(old, new)
log = '- 2026-08-20 — Preserved the vision, returned the technical shape to Shaping, and explicitly retired “modes on two clocks” as implementation instruction.'
assert body.count(log) == 1
body = body.replace(log, log + '\n- 2026-08-21 — Added hiding and ambushes through Intel/clocks as possibilities without changing the current reconciliation proof.')
(cache/'monster-201-expanded.md').write_text(body)
PY

gh api -X PATCH repos/KirkDiggler/rpg-project/issues/201 \
  -F body=@"$cache/monster-201-expanded.md" \
  > "$cache/monster-201-update.json"
test "$(gh api repos/KirkDiggler/rpg-project/issues/201 --jq .body)" = "$(< "$cache/monster-201-expanded.md")"
```

Expected: exact body equality and the existing Next proof remains untouched.

- [x] **Step 2: Verify #201's hierarchy and board state did not drift**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
gh api graphql -f query='query{repository(owner:"KirkDiggler",name:"rpg-project"){issue(number:201){number title parent{number} subIssues(first:10){totalCount} assignees(first:10){nodes{login}} labels(first:10){nodes{name}} body}}}' > "$cache/monster-201-readback.json"
gh project item-list 19 --owner KirkDiggler --limit 1000 --format json > "$cache/items-after-monster-update.json"

jq -e '
  .data.repository.issue.parent.number==231 and
  .data.repository.issue.subIssues.totalCount==0 and
  (.data.repository.issue.assignees.nodes|length)==0 and
  ([.data.repository.issue.labels.nodes[].name]|contains(["journey"])) and
  (.data.repository.issue.body|contains("monsters hiding from party Intel") and contains("authored ambushes") and contains("## Next proof"))
' "$cache/monster-201-readback.json" >/dev/null

jq -e '
  [.items[]|select(.content.repository=="KirkDiggler/rpg-project" and .content.number==201)|{status,readiness,team,area,initiative,kind:(.kind//null),assignees}] ==
  [{status:"Todo",readiness:"Shaping",team:"Monster AI",area:"The Dungeon",initiative:"Four-player Level-3 Dungeon",kind:null,assignees:[]}]
' "$cache/items-after-monster-update.json" >/dev/null
```

Expected: #201 remains Todo/Shaping/unassigned under #231 with no children.

---

### Task 5: Ratify documentation and publish the expansion checkpoint

**Files:**
- Modify: `CLAUDE.md`
- Modify: `ideas/team-workflow/project-19-journeys/design.md`
- Modify: `ideas/team-workflow/project-19-journeys/plan.md`
- Modify: `sessions/active.md`

**Produces:** Canonical docs matching live state, final evidence manifest, signed checkpoints, a reopened pilot issue, and an open follow-up Pilot Active PR.

- [x] **Step 1: Update the canonical Project Board pointer semantics**

In `CLAUDE.md`, replace:

```markdown
The current initiative is **Four-player Level-3 Dungeon**. Work is organized as
**Initiative → Journey → Slice**: an initiative is the strategic outcome, a
journey is a coherent multi-PR capability, and a slice is one owning-repository
issue with its linked PR. **Ready Journeys** is the contributor entry point. A
blank Initiative means the item has not been reconciled against the current
architecture; Todo alone does not make it ready.
```

with:

```markdown
The current initiative is **Four-player Level-3 Dungeon**. Adopted work is
organized as **Initiative → Journey → Slice**; an unadopted capability may remain
a top-level Shaping journey until an initiative adopts it. Initiative is a
priority lens, not a permission boundary.

**Ready Journeys** is the global contributor entry point. Use **Current
Initiative** for prioritized outcomes, **Shelf → Shaping** for rough journey
starters, and the **Active Journeys** Parent issue rail for work already sliced
beneath a journey. Todo alone does not make a journey Ready.
```

Expected: `CLAUDE.md` links the design at `ideas/team-workflow/project-19-journeys/design.md` and no longer says blank Initiative means unready.

- [x] **Step 2: Mark the design expansion live and refresh the active handoff**

Change the design frontmatter status from:

```text
status: pilot active; journey-starter expansion approved by Kirk 2026-08-20; expansion implementation planned
```

to:

```text
status: pilot active; journey-starter expansion live 2026-08-21
```

Replace the existing Project 19 bullet in `sessions/active.md` with a concise handoff containing:

```markdown
- **Project 19 initiative/journey pilot (#229 / PR #245; merged predecessor PR #230) — starter expansion live, pilot active.** Ready Journeys is global; Active Journeys is sliced by Parent issue; Shelf is sliced by Readiness. Initiative #231 includes #169, #232, #201, #236, plus #241 (class resources) and #242 (between-run leveling). #243 (spellcasting) and #244 (dungeon danger) are top-level Shaping starters with no Initiative. #201 shelves Intel/clock-driven hiding and ambushes. No new starter has a predicted child slice. Keep PR #245 open until a real collaborator checkpoint exercises discovery, claim, and handoff.
```

Expected: volatile board facts have owning issue/PR pointers and no gameplay implementation is claimed.

- [x] **Step 3: Capture and assert the complete final live state**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/resources.env"
source "$cache/leveling.env"
source "$cache/spellcasting.env"
source "$cache/dungeon-danger.env"

gh project view 19 --owner KirkDiggler --format json > "$cache/project-final.json"
gh project field-list 19 --owner KirkDiggler --format json > "$cache/fields-final.json"
gh project item-list 19 --owner KirkDiggler --limit 1000 --format json > "$cache/items-final.json"
gh api graphql -F resource="$RESOURCE_NUMBER" -F leveling="$LEVELING_NUMBER" -F spell="$SPELL_NUMBER" -F danger="$DANGER_NUMBER" -f query='query($resource:Int!,$leveling:Int!,$spell:Int!,$danger:Int!){repository(owner:"KirkDiggler",name:"rpg-project"){initiative:issue(number:231){subIssues(first:20){totalCount nodes{number title}}} resource:issue(number:$resource){parent{number} subIssues(first:10){totalCount}} leveling:issue(number:$leveling){parent{number} subIssues(first:10){totalCount}} spell:issue(number:$spell){parent{number} subIssues(first:10){totalCount}} danger:issue(number:$danger){parent{number} subIssues(first:10){totalCount}} monster:issue(number:201){body parent{number}}}}' > "$cache/hierarchy-final.json"
gh api graphql -f query='query($id:ID!){node(id:$id){... on ProjectV2{views(first:100){nodes{id number name filter updatedAt}}}} viewType:__type(name:"ProjectV2View"){fields{name}}}' -f id='PVT_kwHOAASbwc4Bcj4v' > "$cache/views-final.json"

diff <(jq -S '.fields' "$cache/fields-before.json") <(jq -S '.fields' "$cache/fields-final.json")
jq -e '.readme|contains("Initiative is a priority lens") and contains("Ready Journeys") and contains("ideas/team-workflow/project-19-journeys/design.md")' "$cache/project-final.json" >/dev/null

jq -e --argjson r "$RESOURCE_NUMBER" --argjson l "$LEVELING_NUMBER" '
  (([169,201,232,236,$r,$l] - [.data.repository.initiative.subIssues.nodes[].number])|length)==0 and
  .data.repository.resource.parent.number==231 and .data.repository.leveling.parent.number==231 and
  .data.repository.spell.parent==null and .data.repository.danger.parent==null and
  .data.repository.resource.subIssues.totalCount==0 and .data.repository.leveling.subIssues.totalCount==0 and
  .data.repository.spell.subIssues.totalCount==0 and .data.repository.danger.subIssues.totalCount==0 and
  (.data.repository.monster.body|contains("authored ambushes") and contains("monsters hiding from party Intel"))
' "$cache/hierarchy-final.json" >/dev/null

jq -e --argjson r "$RESOURCE_NUMBER" --argjson l "$LEVELING_NUMBER" --argjson s "$SPELL_NUMBER" --argjson d "$DANGER_NUMBER" '
  ([.items[]|select(.content.repository=="KirkDiggler/rpg-project" and (.content.number==$r or .content.number==$l or .content.number==$s or .content.number==$d))]|length)==4 and
  all(.items[]|select(.content.repository=="KirkDiggler/rpg-project" and (.content.number==$r or .content.number==$l or .content.number==$s or .content.number==$d));
    .status=="Todo" and .readiness=="Shaping" and .team=="Cross-team" and (.kind//null)==null and (.assignees|length)==0) and
  all(.items[]|select(.content.repository=="KirkDiggler/rpg-project" and (.content.number==$r or .content.number==$l));
    .area=="Class Kits" and .initiative=="Four-player Level-3 Dungeon") and
  ([.items[]|select(.content.repository=="KirkDiggler/rpg-project" and .content.number==$s)|{area,initiative:(.initiative//null)}]==[{area:"Class Kits",initiative:null}]) and
  ([.items[]|select(.content.repository=="KirkDiggler/rpg-project" and .content.number==$d)|{area,initiative:(.initiative//null)}]==[{area:"The Dungeon",initiative:null}])
' "$cache/items-final.json" >/dev/null

jq -e '
  ([.data.node.views.nodes[]|select(.id=="PVTV_lAHOAASbwc4Bcj4vzgLXUno")|.filter]==["label:journey status:Todo readiness:Ready no:assignee"]) and
  ([.data.node.views.nodes[]|select(.id=="PVTV_lAHOAASbwc4Bcj4vzgLXUns")|.filter]==["has:parent-issue kind:Build,Fix,Verify,Learn,Decide,Concept -status:Done"]) and
  ([.data.node.views.nodes[]|select(.id=="PVTV_lAHOAASbwc4Bcj4vzgLXUnw")|.filter]==["kind:Learn,Decide,Concept -status:Done"]) and
  ([.data.node.views.nodes[]|select(.id=="PVTV_lAHOAASbwc4Bcj4vzgLXUn0")|.filter]==["no:initiative -status:Done"]) and
  ([.data.viewType.fields[].name|select(test("slice";"i"))]|length)==0
' "$cache/views-final.json" >/dev/null
test -s "$cache/views-slice-browser-verification.txt"

jq -n \
  --slurpfile project "$cache/project-final.json" \
  --slurpfile fields "$cache/fields-final.json" \
  --slurpfile items "$cache/items-final.json" \
  --slurpfile hierarchy "$cache/hierarchy-final.json" \
  --slurpfile views "$cache/views-final.json" \
  --rawfile browserSlice "$cache/views-slice-browser-verification.txt" \
  '{captured_at:(now|todate),phase:"journey-starter expansion live; retro not yet earned",project:$project[0],fields:$fields[0],items:$items[0],hierarchy:$hierarchy[0],views:$views[0],browser_slice_verification:$browserSlice}' \
  > "$cache/final-state.json"
sha256sum "$cache/final-state.json" > "$cache/final-state.sha256"
```

Expected: every assertion exits 0 and `final-state.sha256` records the final evidence artifact.

Execution lifecycle note: Kirk merged predecessor PR #230 at 2026-08-21T11:11:52Z while the live rollout was in progress, and GitHub deleted its branch. Kirk then approved a fresh follow-up. The preserved documentation patch was applied cleanly to `docs/229-global-journey-starters` from `origin/main` at `0c017bb`; the merged branch was not recreated.

- [x] **Step 4: Run documentation self-review and publish the first follow-up commit**

```bash
git diff --check
python3 - <<'PY'
from pathlib import Path
root = Path('.')
files = [
    root/'CLAUDE.md',
    root/'ideas/team-workflow/project-19-journeys/design.md',
    root/'ideas/team-workflow/project-19-journeys/plan.md',
    root/'sessions/active.md',
]
contents = {p: p.read_text() for p in files}
text = '\n'.join(contents.values())
for marker in ['T'+'BD', 'T'+'ODO', 'F'+'IXME', 'PLACE'+'HOLDER', '?'*3]:
    assert marker not in text, marker
assert ('ideas/' + 'project-19-journeys') not in text
claude = contents[root/'CLAUDE.md']
assert 'Initiative is a priority lens' in ' '.join(claude.split())
assert 'Ready Journeys' in claude
assert 'ideas/team-workflow/project-19-journeys/design.md' in claude
PY

git add CLAUDE.md ideas/team-workflow/project-19-journeys/ sessions/active.md
git diff --cached --check
git commit -m 'docs: activate global journey starters'
git push -u origin docs/229-global-journey-starters
```

Expected: clean checks and a pushed first follow-up commit containing the live docs plus completed rollout steps.

- [x] **Step 5: Open the follow-up PR, reopen #229, and publish signed checkpoints**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/resources.env"
source "$cache/leveling.env"
source "$cache/spellcasting.env"
source "$cache/dungeon-danger.env"

STATE_SHA=$(awk '{print $1}' "$cache/final-state.sha256")
INITIATIVE_CHILDREN=$(jq -r '[.data.repository.initiative.subIssues.nodes[].number|"#\(.)"]|join(", ")' "$cache/hierarchy-final.json")
cat > "$cache/followup-pr-body.md" <<EOF
## Review phase

**Pilot Active; journey-starter expansion live.** The revised global views, four journey starters, #201 possibility shelf, and canonical documentation are live. This follow-up stays open through a real collaborator checkpoint and retro, so setup completion is not yet a success verdict or MERGE-READY call.

## Summary

- make Ready Journeys and Discovery global while keeping Current Initiative as the priority lens
- slice Active Journeys by Parent issue and Shelf by Readiness
- add #241 and #242 beneath initiative #231
- add top-level Shaping starters #243 and #244 without Initiative
- preserve concurrent #236 and its slices
- record the public API gap for browser-only Slice-by settings

## Evidence

- final-state SHA-256: \`$STATE_SHA\`
- initiative child readback: $INITIATIVE_CHILDREN
- persistent local evidence: \`~/.cache/pi/project19-journey-starters/final-state.json\`

## Lifecycle

PR #230 is the merged design/pilot predecessor. Kirk approved this fresh follow-up after #230 merged and its branch was deleted during rollout.

Tracks #229.

— asset-pipeline agent, on behalf of KirkDiggler
EOF

FOLLOWUP_URL=$(gh pr create --repo KirkDiggler/rpg-project \
  --base main --head docs/229-global-journey-starters \
  --title 'docs: activate global journey starters' \
  --body-file "$cache/followup-pr-body.md")
FOLLOWUP_NUMBER=${FOLLOWUP_URL##*/}
printf 'FOLLOWUP_NUMBER=%q\nFOLLOWUP_URL=%q\n' "$FOLLOWUP_NUMBER" "$FOLLOWUP_URL" > "$cache/followup.env"
gh pr view "$FOLLOWUP_NUMBER" --repo KirkDiggler/rpg-project --json number,state,baseRefName,headRefName,url,body > "$cache/followup-created.json"
jq -e '.state=="OPEN" and .baseRefName=="main" and .headRefName=="docs/229-global-journey-starters" and (.body|contains("Pilot Active; journey-starter expansion live") and contains("Tracks #229"))' "$cache/followup-created.json" >/dev/null

gh api repos/KirkDiggler/rpg-project/issues/229 --jq .body > "$cache/issue-229-plan-ready.md"
FOLLOWUP_URL="$FOLLOWUP_URL" python3 - <<'PY'
import os
from pathlib import Path
cache = Path.home()/'.cache/pi/project19-journey-starters'
body = (cache/'issue-229-plan-ready.md').read_text()
old = 'The three-journey pilot is active. Kirk approved the journey-starter expansion on 2026-08-20; the implementation plan is committed and execution is pending.'
new = 'The journey pilot and starter expansion are live. The board remains in Pilot Active state until a real collaborator checkpoint and retro.'
assert body.count(old) == 1
body = body.replace(old, new)
anchor = '- `ideas/team-workflow/project-19-journeys/pilot-plan.md` — completed initial rollout'
assert body.count(anchor) == 1
body = body.replace(anchor, anchor + f'\n- Follow-up rollout review: {os.environ["FOLLOWUP_URL"]}')
old_taxonomy = 'The stable-domain location follows the taxonomy designed in PR #238.'
new_taxonomy = 'The idea remains on its own review line; PR #238 was consulted only for the `ideas/team-workflow/` directory convention.'
assert body.count(old_taxonomy) == 1
body = body.replace(old_taxonomy, new_taxonomy)
(cache/'issue-229-live.md').write_text(body)
PY
gh api -X PATCH repos/KirkDiggler/rpg-project/issues/229 -f state=open -F body=@"$cache/issue-229-live.md" > "$cache/issue-229-live-update.json"
test "$(gh api repos/KirkDiggler/rpg-project/issues/229 --jq .body)" = "$(< "$cache/issue-229-live.md")"
jq -e '.state=="open" and (.body|contains("Follow-up rollout review"))' "$cache/issue-229-live-update.json" >/dev/null

STATE_SHA=$(awk '{print $1}' "$cache/final-state.sha256")
INITIATIVE_CHILDREN=$(jq -r '[.data.repository.initiative.subIssues.nodes[].number|"#\(.)"]|join(", ")' "$cache/hierarchy-final.json")
cat > "$cache/expansion-live-checkpoint.md" <<EOF
## Journey-starter expansion live — retro not yet earned

### Views

- Ready Journeys: \`label:journey status:Todo readiness:Ready no:assignee\`
- Active Journeys: \`has:parent-issue kind:Build,Fix,Verify,Learn,Decide,Concept -status:Done\`; Slice by **Parent issue**
- Discovery and Concepts: \`kind:Learn,Decide,Concept -status:Done\`
- Shelf: \`no:initiative -status:Done\`; Slice by **Readiness**

### Journey state

Initiative #231's current GraphQL child readback is: $INITIATIVE_CHILDREN. This preserves the concurrent multi-contributor workspace journey alongside this expansion.

- $RESOURCE_URL — Todo / Shaping / Cross-team / Class Kits / unassigned; parent #231
- $LEVELING_URL — Todo / Shaping / Cross-team / Class Kits / unassigned; parent #231
- $SPELL_URL — Todo / Shaping / Cross-team / Class Kits / unassigned; no Initiative or parent
- $DANGER_URL — Todo / Shaping / Cross-team / The Dungeon / unassigned; no Initiative or parent

All four have blank Kind and zero child slices. #201 now shelves Intel/clock-driven hiding and ambushes without changing its immediate reconciliation proof.

### Evidence and deviation

- Final-state SHA-256: \`$STATE_SHA\`
- Persistent evidence: \`~/.cache/pi/project19-journey-starters/final-state.json\`
- Follow-up review: $FOLLOWUP_URL; merged predecessor: PR #230
- GitHub's public ProjectV2View schema has no Slice-by field. Kirk set both values in the browser and confirmed they persisted after refresh; the API limitation is captured in the final evidence.

This does not pass the pilot by itself. The next acceptance checkpoint is a collaborator finding, claiming, or shaping one journey from shared GitHub state without private assignment.

— asset-pipeline agent, on behalf of KirkDiggler
EOF

gh api -X POST repos/KirkDiggler/rpg-project/issues/229/comments -F body=@"$cache/expansion-live-checkpoint.md" > "$cache/checkpoint-229.json"
gh api -X POST "repos/KirkDiggler/rpg-project/issues/$FOLLOWUP_NUMBER/comments" -F body=@"$cache/expansion-live-checkpoint.md" > "$cache/checkpoint-followup.json"
cat > "$cache/predecessor-redirect.md" <<EOF
The journey-starter expansion continued in $FOLLOWUP_URL after this design/pilot predecessor merged. The live checkpoint and retro gate are carried there and on #229.

— asset-pipeline agent, on behalf of KirkDiggler
EOF
gh api -X POST repos/KirkDiggler/rpg-project/issues/230/comments -F body=@"$cache/predecessor-redirect.md" > "$cache/checkpoint-230.json"
jq -e '.html_url|contains("/issues/229#issuecomment-")' "$cache/checkpoint-229.json" >/dev/null
jq -e --arg n "$FOLLOWUP_NUMBER" '.html_url|contains("/pull/\($n)#issuecomment-")' "$cache/checkpoint-followup.json" >/dev/null
jq -e '.html_url|contains("/pull/230#issuecomment-")' "$cache/checkpoint-230.json" >/dev/null
jq -e '.state=="open"' "$cache/issue-229-live-update.json" >/dev/null
jq -e '.state=="OPEN"' "$cache/followup-created.json" >/dev/null
gh api repos/KirkDiggler/rpg-project/pulls/230 --jq '.merged_at != null' | grep -qx true
```

Expected: #229 is reopened, the follow-up PR is open, all three signed comments have durable URLs, and merged PR #230 points forward.

- [x] **Step 6: Verify final Git and GitHub lifecycle state**

```bash
cache="$HOME/.cache/pi/project19-journey-starters"
source "$cache/followup.env"
test -z "$(git status --porcelain)"
test "$(git rev-parse HEAD)" = "$(git rev-parse origin/docs/229-global-journey-starters)"
gh api repos/KirkDiggler/rpg-project/issues/229 --jq '.state' | grep -qx open
gh api "repos/KirkDiggler/rpg-project/pulls/$FOLLOWUP_NUMBER" --jq '.state' | grep -qx open
gh api "repos/KirkDiggler/rpg-project/pulls/$FOLLOWUP_NUMBER" --jq '.body' | grep -Fq 'journey-starter expansion live'
gh api repos/KirkDiggler/rpg-project/pulls/230 --jq '.merged_at != null' | grep -qx true
! gh api "repos/KirkDiggler/rpg-project/pulls/$FOLLOWUP_NUMBER" --jq '.body' | grep -Fq 'retro passed'
```

Expected: clean synchronized follow-up branch, reopened issue #229, open follow-up PR, merged predecessor #230, and no claim that the pilot retro has passed.
