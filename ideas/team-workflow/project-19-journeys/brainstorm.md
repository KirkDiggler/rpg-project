---
name: Project 19 Initiative and Journey Model — Brainstorm
tracking: https://github.com/KirkDiggler/rpg-project/issues/229
status: conversation-approved 2026-08-20; written review pending
---

# Project 19 Initiative and Journey Model — Brainstorm

## Why this exists

Project 19 classifies work well but does not yet help a contributor choose and
carry a meaningful feature. At the start of this design:

- the board contained 515 items;
- 211 were not Done, including 165 Todo items;
- Team and Feature identified ownership and product area;
- none of the items used the exposed Parent issue or Sub-issues progress fields;
  and
- an active design such as rpg-toolkit PR #1126 described a coherent multi-PR
  capability without exposing that larger capability as a board unit.

The missing answer was not another team lane. It was: **what player-visible
outcome are we advancing, what is the next proof, and who currently holds the
baton?**

## Current north star

The adopted initiative is a four-player multiplayer dungeon in which the four
selected classes can progress through level 3 and use all of their supported
features.

The initial class/subclass scope is the SRD 5.1 set:

- Barbarian + Berserker;
- Fighter + Champion;
- Monk + Open Hand; and
- Rogue + Thief.

Characters earn XP from monster kills, with a dungeon-completion reward left as
an optional later slice. Crossing an XP threshold does not mutate a character
inside the dungeon. The player performs a level-up flow after leaving the
dungeon.

The initiative's current final proof runs through the local dev game. A hosted
dev deployment is desirable later but does not block the initiative.

The active runtime path is rpg-toolkit → rpg-api → rpg-dnd5e-web.
rpg-api-protos participates when the wire contract changes. dnd-bot-discord is
a dead project and supplies neither current requirements nor roadmap work.

## Vocabulary reached in discussion

- **Initiative:** the large destination containing several independently useful
  capabilities. Current example: the four-player level-3 dungeon.
- **Journey:** one coherent, observable capability within an initiative, usually
  requiring multiple PRs. A journey keeps a stable outcome while its route may
  change as implementation teaches us.
- **Slice:** one coherent issue/PR-sized merge unit beneath a journey.
- **Baton:** current ownership of a journey. A contributor may continue after a
  proven slice or hand it off cleanly.
- **Area:** the broad product classification currently called Feature on the
  board, such as The Dungeon or Class Kits.

"Journey" was chosen to avoid confusing a multi-PR feature with the board's
existing broad Feature field. Renaming that field to Area makes the distinction
explicit.

## Approaches considered

### Chosen: one durable board with initiative-focused views

The game evolves inside one Project 19 board. An Initiative field and saved
views provide focus. Journey parent issues expose multi-PR outcomes; child
issues retain repository ownership and one-issue-per-PR discipline.

This keeps history and future work discoverable without forcing contributors to
search a series of predicted project boards.

### Rejected: one project board per initiative

A fresh board would look clean initially but fragment durable game history and
require deciding future project boundaries before the product earns them.

### Rejected: retain a flat board and add filters only

More filters would reduce visible volume but would not reveal why several PRs
belong together, what closes the feature, or who owns its continuation.

## Stable outcome, rolling route

A journey is intentionally not a complete task tree. It records:

- the player-visible outcome;
- why it matters to the current initiative;
- what is already true;
- one immediate proof;
- its observable done condition;
- hard dependencies;
- future shelves; and
- short learning updates after each slice.

Only the next proven slice or two become child issues. Later slices are created
when the consumer or implementation makes them real. This avoids false roadmap
precision and preserves the project's outside-in development rule.

A plan task is not automatically a board issue. Child issues represent coherent
merge units. Work discovered inside the same module and coherent wave stays on
the current branch; a genuinely separate module or repository leg gets another
child.

## Evidence ladder

Different work modes are legitimate but prove different claims:

- **Learn:** answers a feasibility or architecture question with recorded
  evidence.
- **Decide:** settles a boundary or contract.
- **Concept:** proves the desired experience, commonly with fixture data in the
  web's `/concepts` surface.
- **Build:** creates production behavior.
- **Fix:** closes a discovered defect or gap.
- **Verify:** observes the capability through the intended real path.

Toolkit tests can prove a toolkit slice. They cannot close a player-visible
journey by themselves. A gameplay journey closes only after its behavior runs
locally through the real dev game path.

## Long-lived research and future architecture

The board must retain R&D and prototypes without presenting them as committed
production scope. Work with no current initiative remains outside initiative
views. A future possibility is first recorded on a journey's shelf; it becomes
a child only when a concrete use case needs it.

Limited spells are such a shelf today. The new resolution machines may make a
small spell proof cheaper than expected, but the initial four SRD subclasses do
not require spellcasting. A concrete spell case can later earn a Learn or
Concept slice and, if useful, expand the supported subclass list.

## Roadmap posture

A future roadmap should display initiatives and journeys, not PR slices.
Journey parents record hard dependencies now. Start and target dates are added
later only when scheduling becomes useful and honest. Sequence is not encoded
through title numbering or speculative dates.

## Planning discovery: Project view API

GitHub's current GraphQL API can create and update Project 19 fields, views,
filters, visible columns, and cross-repository sub-issue relationships. Its
writable view configuration does not expose group-by fields. The pilot therefore
uses API-verifiable table views with Parent issue and Sub-issues progress
visible; visual parent grouping remains an optional later refinement rather
than a browser-only setup requirement.

## Current-architecture reconciliation

Project 19 contains current work, but some issue premises predate the composable
encounter/session architecture. The new Initiative field acts as a trust
boundary: an item receives the current initiative only after its outcome and
next proof have been checked against today's architecture.

Reconciliation is journey-local rather than a 211-item audit. Related items are:

1. adopted as children when still valid;
2. rewritten or superseded when their old-stack premise is dead;
3. combined under one canonical issue when duplicated; or
4. left uncommitted when they do not yet serve the initiative.

The outcome survives when useful even if the implementation prescription does
not.

## Known journey candidates

The first initiative is expected to grow journeys around:

- the new dungeon on the composable encounter stack;
- four-player session continuity;
- the complete dungeon lifecycle;
- monster behavior visible in play;
- composable attack damage;
- XP awards and persistence;
- the outside-dungeon level-up flow;
- one level 1–3 journey for each of the four class/subclass pairs; and
- a final integrated four-player level-3 proof.

These are an outcome map, not a committed sequence. A candidate becomes Ready
only after reconciliation and definition of its immediate proof.

## Approved pilot

### Composable Dungeon Builder

Kirk holds the baton. The canonical artifact is editable YAML; the builder is a
visual producer and editor of that same format, not a separate dungeon dialect.

Journey proof: create, reopen, and edit a canonical multi-room dungeon with
props and lighting, then launch and play it locally without losing authored
meaning. Existing builder, YAML, and world-model issues are candidate slices,
not an automatically accepted backlog.

### Composable Attack Damage

The teammate working on rpg-toolkit PR #1126 holds the baton. The design PR is
a Decide slice. Issues #979 and #927 require reconciliation into one canonical
journey before implementation children are declared Ready.

Journey proof: toolkit tests retain the typed per-pool evidence, and an attack
using multiple typed pools resolves correctly through the local game.

### Monster Behavior

This begins unassigned and Shaping. rpg-project #201 preserves a useful product
vision but must be reconciled with the newer composable encounter/session and
resolution-machine model before becoming Ready.

Journey proof: behavior is first discriminated by toolkit tests and then seen as
intentional monster decisions in the local dungeon.

## What the pilot must teach us

After one complete checkpoint cycle, the retro asks whether:

- a teammate found meaningful work without requesting an assignment;
- the outcome and immediate proof were understandable;
- stale assumptions surfaced before implementation;
- the baton continued or transferred cleanly;
- Journey Done remained distinct from a green toolkit PR; and
- maintaining hierarchy was lighter than interpreting the flat board.

Custom automation waits until this human workflow demonstrates a stable,
repetitive need.
