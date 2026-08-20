---
name: Project 19 Initiative and Journey Model
tracking: https://github.com/KirkDiggler/rpg-project/issues/229
status: approved by Kirk 2026-08-20; pilot implementation pending
---

# Project 19 Initiative and Journey Model

## 1. Purpose

Project 19 MUST remain the durable project board for the evolving game. Saved
views MUST make the adopted initiative and claimable multi-PR features visible
without creating a new project for each predicted phase of development.

The current initiative is **Four-player Level-3 Dungeon**. Its completion proof
MUST run through the local dev game. A hosted dev deployment is not required.

## 2. Work hierarchy

Project 19 MUST use this hierarchy:

```text
Initiative
└── Journey
    └── Slice issue
        └── Linked pull request
```

- An **Initiative** is an adopted strategic outcome containing multiple
  independently useful journeys.
- A **Journey** is one player-visible or operator-visible capability that may
  span multiple PRs while retaining one stable outcome.
- A **Slice** is one coherent PR-sized merge unit in its owning repository.
- The **Baton** is the journey's current assignee and MAY transfer after any
  proven slice.
- An **Area** is a broad product classification; it is not a journey or a
  schedule.

The canonical initiative and journey records MUST be issues in `rpg-project`.
Slice issues MUST live in the repository that owns their implementation.
GitHub sub-issue relationships MUST connect initiative → journey → slice.
Every slice PR MUST close or link its own slice issue; an implementation PR
MUST NOT reuse the initiative or journey issue as its PR issue.

## 3. Board fields

### 3.1 Existing fields

- `Status` MUST retain `Todo`, `In Progress`, `In Review`, and `Done`.
- `Team` MUST retain the current ownership values.
- The current `Feature` field MUST be renamed `Area`. Existing product-area
  values MAY remain during the pilot.
- `Kind` MUST retain `Build`, `Fix`, `Verify`, `Learn`, and `Decide`, and MUST
  add `Concept`.

`Shelf` is a legacy non-area value. New work MUST NOT use `Area = Shelf`.
Uncommitted work is kept outside active initiative views. Retirement or
migration of the legacy value waits for the pilot retro.

### 3.2 New fields

`Initiative` MUST be a single-select field. The pilot MUST add exactly one
adopted value:

- `Four-player Level-3 Dungeon`

A new value MUST NOT be added until its initiative is actually adopted. Every
initiative issue, journey, and slice selected for an initiative MUST carry the
same Initiative value so saved views can include the full hierarchy.

`Readiness` MUST be a single-select field used on journey issues only:

- `Shaping` — the outcome is useful but the immediate proof is not ready;
- `Ready` — one immediate proof is defined and claimable; and
- `Blocked` — a hard blocker is linked explicitly.

Readiness MUST remain empty on slice issues. A `Ready` journey MUST be Todo and
unblocked. A claimed journey MUST become In Progress; it MAY clear Readiness.
An active journey that becomes blocked MUST use `Blocked` until the blocker is
resolved. Done journeys MUST have no active Readiness value.

### 3.3 Structural labels and assignees

Canonical `initiative` and `journey` labels MUST identify hierarchy levels in
`rpg-project` saved views. These labels are structural, not priority labels.

The assignee on a journey issue is the current baton holder. Slice assignees
own only that slice. Clearing or changing the journey assignee is an explicit
handoff and MUST include a current checkpoint on the journey.

## 4. Initiative contract

An initiative issue MUST state:

1. the player-visible north star;
2. its completion proof;
3. adopted scope and explicit exclusions;
4. its journey children; and
5. the final integrated verification story.

For the current initiative:

- four players MUST be able to play the new dungeon through the local dev
  stack;
- characters MUST be able to earn persisted XP from monster kills;
- level changes MUST occur through an outside-dungeon flow;
- supported class content through level 3 MUST include Barbarian/Berserker,
  Fighter/Champion, Monk/Open Hand, and Rogue/Thief; and
- all supported level 1–3 abilities MUST be usable through their intended game
  paths before the initiative closes.

A dungeon-completion XP reward, hosted dev deployment, additional subclasses,
and spellcasting are not initial completion requirements. They MAY become later
journeys when adopted.

## 5. Journey contract

Every journey issue MUST contain these sections:

1. **Outcome** — an observable capability, not an implementation deliverable.
2. **Why now** — its connection to the adopted initiative.
3. **Current reality** — verified facts only.
4. **Next proof** — one concrete, immediate proof.
5. **Done when** — the real path that closes the journey.
6. **Dependencies** — hard prerequisites only.
7. **Not now / shelves** — useful future seams without promised work.
8. **Learning log** — concise checkpoint decisions and evidence.

A journey MUST NOT begin with a speculative complete child tree. It SHOULD
carry only the next proven slice or two. Later children are added when a
consumer, concept, or implementation result makes them concrete.

A plan task or checkpoint MUST NOT automatically become a slice issue. Work in
the same coherent module/wave MUST remain in one branch and PR. Work requiring
a separate owning repository or genuinely separate module MAY become another
slice.

## 6. Work kinds and proof

| Kind | Required completion evidence |
|---|---|
| Learn | Recorded answer and evidence for the investigated question |
| Decide | Explicit accepted decision, trade-offs, and superseded alternatives |
| Concept | Runnable desired experience, normally fixture-driven in the web `/concepts` surface |
| Build | Production behavior plus repository tests |
| Fix | Reproduction, correction, and regression proof |
| Verify | Evidence from the intended real path |

Toolkit tests MAY prove a toolkit slice. They MUST NOT by themselves close a
player-visible journey. A gameplay journey MUST close with local evidence
through the real dev game path. A concept proves desired behavior or a consumer
contract; it MUST NOT be represented as production integration.

## 7. Lifecycle

1. Reconcile or create the journey parent.
2. Assign Initiative, Area, Team, Kind where applicable, and Readiness.
3. Define the immediate proof and create its owning-repository slice issue.
4. Assign the journey baton and current slice.
5. Link the slice PR and move the slice through normal review and verification.
6. After the slice, update Current reality and the Learning log.
7. Continue with a newly earned slice, hand off the baton, block explicitly, or
   execute the journey's closing verification.
8. Mark the journey Done only after its Done-when evidence exists.

A merge MUST NOT automatically mark its journey Done. A journey MAY remain In
Progress across several merged slice PRs.

## 8. Saved views

### Current Initiative

MUST show journey issues for `Initiative = Four-player Level-3 Dungeon` with
Status, Readiness, assignee, Team, Area, and Sub-issues progress.

### Ready Journeys

MUST show journey issues where:

- Initiative is the current initiative;
- Status is Todo;
- Readiness is Ready; and
- no assignee holds the baton.

This is the contributor front door.

### Active Journeys

MUST show non-Done current-initiative journeys and their slice children with
Parent issue and Sub-issues progress visible. It SHOULD group by parent journey
when the available Project view API can configure that grouping. API-writable
filters and visible hierarchy MUST take precedence over a browser-only grouping
step during the pilot.

### Discovery and Concepts

MUST show current work whose Kind is Learn, Decide, or Concept. It MUST keep
those proofs visible without presenting them as shipped production behavior.

### Shelf

MUST keep uncommitted work searchable while excluding it from current
initiative and Ready views. Legacy `Area = Shelf` items MAY remain during the
pilot but MUST NOT be copied into the current initiative without
reconciliation.

Existing Team views MAY remain as secondary lenses over the same source items.
They MUST NOT replace Ready Journeys as the contributor entry point.

## 9. Current-architecture gate

An existing item MUST NOT receive the current Initiative value until its
outcome and immediate proof are reconciled with the composable
encounter/session architecture.

Reconciliation MUST result in one of these dispositions:

1. adopt the issue as a valid journey or slice;
2. rewrite it around the preserved outcome and current architecture;
3. supersede or combine it under one canonical issue; or
4. leave it outside the initiative.

This gate MUST be applied journey by journey. The rollout MUST NOT block on a
full audit of every non-Done board item.

The active runtime path for this initiative is rpg-toolkit → rpg-api →
rpg-dnd5e-web. rpg-api-protos participates when a wire change requires it.
dnd-bot-discord is archival and MUST NOT supply current requirements or work.

## 10. Blockers, discoveries, and handoffs

- A stale technical premise discovered before work starts MUST return the
  journey to Shaping.
- A hard dependency MUST set Readiness to Blocked and link the blocking issue.
- A newly discovered change in the current coherent module/wave MUST stay in
  the current slice.
- A genuinely separate module or repository change MUST receive its own slice.
- Duplicate issues MUST collapse to one canonical child; superseded issues
  MUST link to it.
- A future possibility without a current use case MUST be recorded on a shelf,
  not created as promised implementation work.
- A handoff MUST leave Current reality, Next proof, blockers, and evidence
  current before changing the journey assignee.

## 11. Roadmap compatibility

Journey issues MUST record hard dependencies now. They MUST NOT encode
sequence through title numbering or speculative dates.

A future roadmap MAY add optional Start date and Target date fields. Dates MUST
apply only to initiatives and journeys. PR slices MUST remain in active-work
views rather than becoming roadmap bars.

## 12. Pilot journeys

### Composable Dungeon Builder

- Baton: Kirk.
- Initial status: In Progress.
- Canonical artifact: editable dungeon YAML.
- The builder MUST produce and reopen that same format rather than a parallel
  dialect.
- Done when: an author can create, reopen, and edit a multi-room dungeon with
  props and lighting, then launch and play it locally without losing authored
  meaning.

### Composable Attack Damage

- Baton: the contributor on rpg-toolkit PR #1126.
- Initial status: In Progress.
- PR #1126 MUST be represented as a Decide slice.
- rpg-toolkit issues #979 and #927 MUST be reconciled into one canonical
  journey before implementation slices are marked Ready.
- Done when: toolkit tests retain typed per-pool evidence and an attack using
  multiple typed pools resolves correctly through the local game.

### Monster Behavior

- Baton: initially unassigned.
- Initial status: Todo + Shaping.
- rpg-project issue #201's outcome MUST be reconciled with the current
  composable encounter/session and resolution-machine architecture.
- It MUST become Ready only after one immediate proof is defined.
- Done when: toolkit tests discriminate the behavior and intentional monster
  decisions are observed in the local dungeon.

## 13. Pilot acceptance and retro

The pilot succeeds only if:

- a teammate can find a meaningful unassigned journey without requesting an
  assignment;
- its outcome and immediate proof are understandable from GitHub state;
- stale assumptions are exposed before implementation;
- the baton can continue or transfer at a proven checkpoint;
- journey completion remains distinct from a green toolkit PR; and
- the hierarchy costs less effort than interpreting the flat board.

The retro MUST decide whether to retain, rename, simplify, or remove fields and
views before wider migration. Custom automation MUST NOT be built until the
manual pilot reveals a stable repetitive action worth automating.

## 14. Non-goals

The pilot does not:

- create another GitHub project;
- migrate or audit all historical board items;
- define a complete implementation tree for the current initiative;
- schedule speculative roadmap dates;
- implement hosted dev deployment;
- add spellcasting or additional subclasses;
- revive work from dnd-bot-discord; or
- add custom board automation.
