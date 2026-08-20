# Project 19 Journey Pilot Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to execute this plan. This is one bounded live-board migration; execute inline and do not fan shared Project state out to subagents.

**Goal:** Configure Project 19 around the approved initiative/journey model and instantiate the three pilot journeys without migrating the wider backlog.

**Architecture:** GitHub remains the only durable state. Apply three read-after-write batches—schema, views, pilot hierarchy—then update the canonical pointer. Capture the live state before mutation because Project field-option updates are replacement operations, not patches.

**Tech Stack:** GitHub Projects v2 GraphQL API, GitHub Issues REST/GraphQL APIs, `gh`, `jq`

**Spec:** `ideas/project-19-journeys/design.md`

## Global Constraints

- Project 19 (`PVT_kwHOAASbwc4Bcj4v`) remains the sole board.
- Do not bulk-edit existing items or assign the new Initiative outside the pilot hierarchy.
- Preserve every current single-select option ID when adding `Kind = Concept`.
- Stop on any failed or mismatched readback; do not continue through a partial migration.
- Keep rpg-project issue #229 and PR #230 open through the pilot. Kirk alone merges.
- Do not add roadmap dates, hosted deployment, spell work, custom automation, or dnd-bot-discord items.
- End agent-authored GitHub comments with `— asset-pipeline agent, on behalf of KirkDiggler`.

---

### Task 1: Snapshot and configure the schema

**Live resources:** Project 19 fields and `rpg-project` labels.

- [x] Capture rollback evidence before mutation. The initial snapshot SHA-256 (`83ef4592…b7b4bd`) is recorded on #229/#230; persistent recovery manifests live under `~/.cache/pi/project19-journey-pilot/`. Inputs:
  - `gh project view 19 --owner KirkDiggler --format json`;
  - `gh project field-list 19 --owner KirkDiggler --format json`;
  - `gh project item-list 19 --owner KirkDiggler --limit 1000 --format json`;
  - GraphQL `ProjectV2.views`;
  - rpg-project issues #169, #201, and #229;
  - rpg-toolkit issues #927 and #979; and
  - rpg-toolkit PR #1126.

- [x] Rename field `PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s` from `Feature` to `Area` with `updateProjectV2Field`. Read back all seven existing option IDs unchanged.

- [x] Add `Concept` to field `PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w` with `updateProjectV2Field`, submitting the complete ordered option set:

  `Build (ea162471), Fix (38b89d82), Verify (ab287333), Learn (55d8baa4), Decide (2ccd98be), Concept (new)`.

  Use color `PURPLE` and description `Fixture-driven proof of the desired experience or consumer contract` for Concept. Read back six options; stop if any existing ID changed or disappeared.

- [x] Create `Initiative` with the sole option `Four-player Level-3 Dungeon`:

  ```bash
  gh project field-create 19 --owner KirkDiggler --name Initiative \
    --data-type SINGLE_SELECT \
    --single-select-options 'Four-player Level-3 Dungeon' --format json
  ```

- [x] Create `Readiness` with `Shaping,Ready,Blocked` using the same command shape.

- [x] Create or update structural labels idempotently:

  ```bash
  gh label create initiative --repo KirkDiggler/rpg-project --color 5319E7 \
    --description 'Adopted strategic outcome on Project 19' --force
  gh label create journey --repo KirkDiggler/rpg-project --color 1D76DB \
    --description 'Multi-PR outcome within an adopted initiative' --force
  ```

- [x] Verify field names are exactly `Area`, `Kind`, `Initiative`, and `Readiness`; `Feature` is absent; Kind has six options; Initiative has one; Readiness has three.

---

### Task 2: Create focused views and replace the stale board README

**Live resources:** Project 19 views and README.

Create each view as `TABLE_LAYOUT` with `createProjectV2View`, then set its filter with `updateProjectV2View`:

| View | Exact filter |
|---|---|
| Current Initiative | `initiative:"Four-player Level-3 Dungeon" label:journey` |
| Ready Journeys | `initiative:"Four-player Level-3 Dungeon" label:journey status:Todo readiness:Ready no:assignee` |
| Active Journeys | `initiative:"Four-player Level-3 Dungeon" -status:Done` |
| Discovery and Concepts | `initiative:"Four-player Level-3 Dungeon" kind:Learn,Decide,Concept -status:Done` |
| Shelf | `no:initiative -status:Done` |

- [x] Resolve live IDs and make these fields visible in every view: Title, Assignees, Status, Linked pull requests, Parent issue, Sub-issues progress, Area, Kind, Team, Initiative, Readiness.

- [x] Create the five views and read back exact names, layouts, filters, and visible fields. The public API cannot set group-by configuration; Parent issue and Sub-issues progress are the pilot's verifiable hierarchy surface.

- [x] Replace the Project README with a concise contract that states:
  - Project 19 is the durable game board;
  - hierarchy is Initiative → Journey → Slice;
  - `Ready Journeys` is the contributor front door;
  - blank Initiative means not reconciled, not ready;
  - Area, Team, Kind, Readiness, and Status meanings; and
  - toolkit tests prove slices while local-dev evidence closes gameplay journeys.

- [x] Read the README and all five views back through GraphQL/`gh project view`. Stop if any filter or field is missing.

---

### Task 3: Instantiate the pilot hierarchy

**Live resources:** one initiative, three journey parents, and one current Decide slice.

- [x] Create and board `rpg-project` issue **Initiative: Four-player Level-3 Dungeon** from design §4. Label `initiative`; set Status `In Progress`, Team `Cross-team`, Area `Capstone`, and Initiative `Four-player Level-3 Dungeon`.

- [x] Reconcile `rpg-project#169` as **Journey: Composable Dungeon Builder**:
  - preserve its old body under `Historical design input`;
  - use the journey sections from design §5;
  - record canonical editable YAML, multi-room/props/lighting, local play, and round-trip meaning as the outcome;
  - name compilation of authored YAML into the composable world model as the next proof;
  - label `journey`, assign `KirkDiggler`, retain Status `In Progress`, set Team `Cross-team`, Area `The Dungeon`, and the current Initiative; and
  - add it beneath the initiative with `addSubIssue`.

- [x] Create and board **Journey: Composable Attack Damage**:
  - outcome and proof come from design §12;
  - label `journey`, assign `dammitbilly0ne`, set Status `In Progress`, Team `Platform`, Area `The Dungeon`, and the current Initiative; and
  - add it beneath the initiative.

- [x] Reconcile `rpg-toolkit#979` as PR #1126's Decide slice:
  - preserve its prior body in history;
  - replace stale selective-critical requirements with the accepted #1126 review contract;
  - add it to Project 19 with Status `In Review`, Kind `Decide`, Team `Platform`, Area `The Dungeon`, and the current Initiative;
  - add it beneath the attack journey; and
  - append a signed `Journey tracking` section containing `Closes #979` to PR #1126 without replacing its existing body.

- [x] Link #927's still-useful context from the attack journey/#979, post a signed supersession comment, then close #927 as superseded by #1126.

- [x] Reconcile `rpg-project#201` as **Journey: Monster Behavior in the Local Dungeon**:
  - retain its product vision;
  - mark `modes on two clocks` as historical input rather than current instruction;
  - set the next proof to current-architecture reconciliation against the composable encounter/session and resolution machines;
  - label `journey`, leave unassigned, set Status `Todo`, Readiness `Shaping`, Team `Monster AI`, Area `The Dungeon`, and the current Initiative; and
  - add it beneath the initiative without creating implementation children.

- [x] Verify through GraphQL:
  - the initiative has exactly the three pilot journeys;
  - #169 and #201 retain their issue identities;
  - #979's parent is the attack journey and PR #1126 closes #979;
  - builder is assigned to Kirk;
  - attack damage is assigned to dammitbilly0ne;
  - monster behavior is unassigned and Shaping; and
  - no unrelated item has the current Initiative value.

Execution note: direct Issue/ProjectV2Item GraphQL is the hierarchy authority.
The linked-item Title snapshots for #169/#201 did not follow their issue renames,
so their Project memberships were refreshed and all fields reapplied. The CLI
still omitted its Parent issue projection; direct GraphQL verified those links.

---

### Task 4: Update the canonical pointer and publish the checkpoint

**Files:**
- Modify: `CLAUDE.md` — replace only the stale Project Board section.

- [x] Point `CLAUDE.md` at `https://github.com/users/KirkDiggler/projects/19`, name `Ready Journeys` as the contributor entry, define Initiative → Journey → Slice, retain the one-issue/PR rules, and link `ideas/project-19-journeys/design.md` instead of duplicating it.

- [x] Verify and commit the documentation amendment:

  ```bash
  git diff --check
  rg -n 'projects/19|Ready Journeys|Initiative.*Journey.*Slice|One issue per PR' CLAUDE.md
  git add CLAUDE.md ideas/project-19-journeys/
  git commit -m 'docs: ratify Project 19 journey rollout'
  git push
  ```

- [ ] Post signed checkpoints on issue #229 and PR #230 containing field IDs, view URLs/filters, initiative/journey URLs, #979/#1126 disposition, verification results, deviations, and the real checkpoint that will trigger the pilot retro.

- [ ] Change PR #230's phase to **Pilot Active** with `gh api -X PATCH`. Keep #229 and #230 open/In Progress. Do not claim the model successful until a real journey checkpoint or handoff exercises the retro criteria in design §13.
