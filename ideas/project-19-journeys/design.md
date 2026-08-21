---
name: Project 19 Initiative and Journey Model
tracking: https://github.com/KirkDiggler/rpg-project/issues/229
status: pilot active; journey-starter expansion approved by Kirk 2026-08-20; expansion implementation pending
---

# Project 19 Initiative and Journey Model

## 1. Purpose

Project 19 MUST remain the durable project board for the evolving game. Saved
views MUST make the adopted initiative, globally claimable journeys, and useful
Shaping journey starters visible without creating a new project for each
predicted phase of development.

Initiative is a priority lens, not a permission boundary. A capability MAY be
shaped, proven, or built without belonging to the current initiative. The
current initiative is **Four-player Level-3 Dungeon**. Its completion proof
MUST run through the local dev game. A hosted dev deployment is not required.

## 2. Work hierarchy

Project 19 MUST use these related shapes:

```text
Adopted work                     Unadopted capability
Initiative                       Journey starter
└── Journey                      └── Slice issue (when earned)
    └── Slice issue                  └── Linked pull request
        └── Linked pull request
```

- An **Initiative** is an adopted strategic outcome containing multiple
  independently useful journeys.
- A **Journey** is one player-visible or operator-visible capability that may
  span multiple PRs while retaining one stable outcome. It MAY remain a
  top-level starter until an initiative adopts it.
- A **Slice** is one coherent PR-sized merge unit in its owning repository.
- The **Baton** is the journey's current assignee and MAY transfer after any
  proven slice.
- An **Area** is a broad product classification; it is not a journey or a
  schedule.

The canonical initiative and journey records MUST be issues in `rpg-project`.
Slice issues MUST live in the repository that owns their implementation.
GitHub sub-issue relationships MUST always connect journey → slice and MUST
also connect initiative → journey when that journey is adopted. Every slice PR
MUST close or link its own slice issue; an implementation PR MUST NOT reuse the
initiative or journey issue as its PR issue.

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
same Initiative value so saved views can include the full hierarchy. A blank
Initiative means only that no adopted initiative currently prioritizes the
item; it does not mean invalid, forbidden, or unready.

`Readiness` MUST be a single-select field used on journey issues only:

- `Shaping` — the player promise is useful, but a trustworthy immediate proof
  may not exist yet;
- `Ready` — one immediate proof is defined and claimable; and
- `Blocked` — a hard blocker is linked explicitly.

Readiness MUST remain empty on slice issues. A Shaping journey MAY be claimed
and MAY gain a concrete slice; Readiness communicates maturity rather than
enforcing which Kind of work is allowed. A `Ready` journey MUST be Todo and
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

A fight-clear or dungeon-completion XP reward, hosted dev deployment,
additional subclasses, and spellcasting are not initial completion
requirements. They MAY remain possibilities or become later journeys without
being adopted into this initiative.

## 5. Journey contract

Every Shaping journey starter MUST contain these sections:

1. **Player promise / Outcome** — what a player or operator should be able to
   experience, optionally written as “As a…, I can…, so that…”.
2. **Rough shape** — enough boundary to distinguish the capability from nearby
   journeys without pretending the implementation is known.
3. **Current reality** — verified facts plus links to relevant historical
   trackers, whose claims remain evidence until rechecked.
4. **Ways in** — useful investigations, concepts, decisions, or thin builds
   that could help a collaborator begin.
5. **Possibility shelf** — plausible mechanics or future seams that are not
   accepted policy or promised work.
6. **Learning log** — concise checkpoint decisions and evidence.

An adopted journey MUST also explain **Why now** in terms of its Initiative.
Before a journey becomes Ready, it MUST name one concrete **Next proof** and
its known hard **Dependencies**. Detailed conditions of acceptance and the
closing proof MAY evolve as evidence arrives, but **Done when** MUST be explicit
before the journey can close.

Ways in are invitations, not pre-approved child issues. A possibility-shelf
entry is not a decision. A journey MUST NOT begin with a speculative complete
child tree; it SHOULD carry only the next proven slice or two. Later children
are added when a consumer, concept, or implementation result makes them
concrete.

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

Every slice created or reconciled beneath a journey under this model MUST carry
exactly one Kind; Initiative and Journey issues SHOULD leave Kind blank. No
Kind is restricted by Initiative membership. Learn, Concept, and Build slices
can establish what the platform can do; a later Decide slice can settle whether
or how the game should use that capability.

Toolkit tests MAY prove a toolkit slice. They MUST NOT by themselves close a
player-visible journey. A gameplay journey MUST close with local evidence
through the real dev game path. A concept proves desired behavior or a consumer
contract; it MUST NOT be represented as production integration.

## 7. Lifecycle

1. Reconcile or create the journey starter around a player or operator promise.
2. Set Status Todo, Area, Team, and Readiness. Set Initiative and an initiative
   parent only when the journey is adopted.
3. Link verified current behavior and historical trackers as evidence, not as
   automatic children.
4. A collaborator MAY claim a Shaping journey, take its baton, and create a
   concrete slice when they find a useful way in.
5. Before an unassigned journey enters Ready, name its Next proof and hard
   dependencies.
6. Create each earned slice in its owning repository, assign one Kind, and add
   it beneath the journey.
7. Link the slice PR and move the slice through normal review and verification.
8. After the slice, update Current reality and the Learning log.
9. Continue with a newly earned slice, hand off the baton, block explicitly, or
   execute the journey's closing verification.
10. Mark the journey Done only after its explicit closing evidence exists.

A merge MUST NOT automatically mark its journey Done. A journey MAY remain In
Progress across several merged slice PRs. Adopting a top-level journey MUST set
both its initiative parent and Initiative field; those two signals MUST NOT
drift apart.

## 8. Saved views

### Current Initiative

MUST use `initiative:"Four-player Level-3 Dungeon" label:journey` and show
Status, Readiness, assignee, Team, Area, and Sub-issues progress.

### Ready Journeys

MUST use `label:journey status:Todo readiness:Ready no:assignee`. This global
contributor front door MUST NOT require Initiative membership.

### Active Journeys

MUST show non-Done slice work with a parent and a Kind, using
`has:parent-issue kind:Build,Fix,Verify,Learn,Decide,Concept -status:Done`.
The view SHOULD use **Slice by → Parent issue** so journey names appear in the
left navigation and their current slices appear in the main pane. A journey
with no child slices will not appear in this view; it remains discoverable in
Current Initiative, Ready Journeys, or Shelf.

### Discovery and Concepts

MUST use `kind:Learn,Decide,Concept -status:Done` globally. It MUST keep those
proofs visible without presenting them as shipped production behavior.

### Shelf

MUST use `no:initiative -status:Done` and SHOULD use **Slice by → Readiness**.
Selecting Shaping or Ready exposes unadopted journey starters without requiring
a speculative future Initiative. Legacy `Area = Shelf` items MAY remain during
the pilot but MUST NOT be copied into the current initiative without
reconciliation.

Existing Team views MAY remain as secondary lenses over the same source items.
They MUST NOT replace Ready Journeys as the contributor entry point.

## 9. Current-architecture gate

An existing item MUST NOT receive the current Initiative value until its
outcome and verified current reality are reconciled with the composable
encounter/session architecture. Adoption does not require the journey to have
reached Ready.

Reconciliation MUST result in one of these dispositions:

1. adopt the issue as a valid journey or slice;
2. rewrite it around the preserved outcome and current architecture;
3. supersede or combine it under one canonical issue; or
4. leave it outside the initiative.

This gate MUST be applied journey by journey. The rollout MUST NOT block on a
full audit of every non-Done board item. It governs Initiative adoption, not
permission to shape or implement useful capability outside the Initiative.
Production slices MUST still target the current architecture, and historical
trackers MUST remain evidence until a collaborator deliberately reconciles the
one needed next.

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
- A possibility-shelf entry MUST NOT be treated as accepted game policy.
- Initiative membership MUST NOT restrict whether a concrete Learn, Decide,
  Concept, Build, Fix, or Verify slice can be undertaken.
- A handoff MUST leave Current reality, blockers, evidence, and Next proof when
  one exists current before changing the journey assignee.

## 11. Roadmap compatibility

Journey issues MUST record hard dependencies now. They MUST NOT encode
sequence through title numbering or speculative dates.

A future roadmap MAY add optional Start date and Target date fields. Dates MUST
apply only to initiatives and journeys. PR slices MUST remain in active-work
views rather than becoming roadmap bars.

## 12. Pilot and approved journey starters

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
- Ambushes and hidden monsters using Intel and clocks MUST be recorded as
  player-facing possibilities, not predetermined implementation tasks.
- It MUST become Ready only after one immediate proof is defined.
- Done when: toolkit tests discriminate the behavior and intentional monster
  decisions are observed in the local dungeon.

### Journey-starter expansion

The following unassigned journey stubs MUST begin as Todo + Shaping and MUST
NOT receive speculative child trees:

| Journey | Initiative parent | Area | Team |
|---|---|---|---|
| See, Spend, and Recover Class Resources | rpg-project#231 | Class Kits | Cross-team |
| Earn XP and Level Up Between Runs | rpg-project#231 | Class Kits | Cross-team |
| Cast a Spell in Play | none | Class Kits | Cross-team |
| Choose a Dungeon by Danger | none | The Dungeon | Cross-team |

For **class resources**, Rage is verified current reality: activation checks
availability, consumes a charge, persists the decrement, and publishes a
resource change. Ki is the first shared-pool example because several abilities
consume it. Spell slots remain a possibility seam rather than a required
unification decision. Known evidence includes rpg-toolkit#39,
rpg-toolkit#453, rpg-toolkit#795, and rpg-toolkit#1087; their historical scope
MUST be rechecked before reuse.

For **XP and leveling**, monster kills remain the required initial XP source.
A start-screen character card SHOULD reveal that leveling is available and
lead to the existing Character Sheet. Leveling MUST remain optional and occur
between runs. Fight-clear, objective, and dungeon-completion bonuses belong on
the possibility shelf until deliberately chosen. Known historical trackers
include rpg-toolkit#428, rpg-api#79, rpg-api-protos#183, and
rpg-dnd5e-web#31.

For **spellcasting**, one at-will cantrip moving through selection, targeting,
resolution, and visible result is the promising first vertical proof. Spell
slots, spending and recovery, upcasting, concentration, areas, and full spell
lists remain shelved possibilities. rpg-toolkit#146, rpg-toolkit#431,
rpg-toolkit#799, rpg-api-protos#30, rpg-api#120, rpg-dnd5e-web#95, and
rpg-dnd5e-web#99 are evidence, not accepted architecture.

For **dungeon danger**, monster stat blocks MUST remain canonical: a goblin is
a goblin. The party SHOULD see relative guidance and remain free to choose an
easier or more dangerous dungeon. Difficulty MAY come from dungeon selection,
monster composition, layout, bosses, ambushes, or tactics; the journey MUST NOT
preselect a hidden stat-scaling formula. rpg-toolkit#545 and completed
rpg-api#295 and rpg-api#689 are historical starting points, not an adopted
implementation plan.

## 13. Pilot acceptance and retro

The pilot succeeds only if:

- a teammate can find a meaningful unassigned journey without requesting an
  assignment;
- its maturity is understandable from GitHub state: Shaping provides useful
  ways in, while Ready provides one immediate proof;
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
- implement spellcasting or additional subclasses merely because a journey
  starter exists;
- decide spell-slot architecture, XP-bonus policy, or a difficulty formula;
- force an eligible character to level before playing;
- scale canonical monster statistics secretly;
- revive work from dnd-bot-discord; or
- add custom board automation.
