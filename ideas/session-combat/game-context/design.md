# Game Context — one read channel, one write channel, one door (v1)

**Status:** Design — **decided** (Kirk, 2026-08-28: "I approve and would like
to proceed"). Plan in [plan.md](./plan.md) — an agent-handoff artifact,
auto-approved as long as it honestly represents this design (Kirk's plan
ruling, same day). Reasoning and rejected alternatives in
[brainstorm.md](./brainstorm.md). This PR stays open
through implementation; an `implementation.md` lands beside this file before
merge — the mini retro: what we thought we were building vs. what we found
(Kirk, 2026-08-28).
**Journey:** rpg-project#253 · **Umbrella:** `ideas/session-combat/` ·
**Supersedes:** the two-channel spine of
[effect-context/design.md](../effect-context/design.md) §"The spine" (own-sheet
reads move from injection to the cast; write channel becomes normative).

---

## Context — what exists today

Verified 2026-08-28 against `rpg-toolkit` `origin/main` @ `a2f91b2` (#1283);
toolkit#1284 open.

Three data channels reach conditions, with no rule for which applies:
event payload (incl. own-sheet facts compiled on: `DamageChainEvent.
OffHandWeaponRef`), gamectx (Sneak Attack, Pack Tactics, Prone, OA), and the
`OwnerAware` handle (Unarmored Defense / Martial Arts / Unarmored Movement
reads; `MarkDirty` + purse writes). The handle is wired twice, bespoke
(`character/load.go:152`, `monstertraits/loader.go:271` — dropped in #1281,
restored in #1283); nil-owner branches no-op silently. gamectx tenants (room,
cast, readiness) are installed unconditionally in `resolution/resolve.go`
(`:347`, `:374`, `:395`), each held by a `TestNoCodePathProduces*lessInteraction`
test. Sheet keepers already apply `ConditionApplied`/`ConditionRemoved`/
`HealingReceived` and mark dirty.

## The two channels

- **R1 — Reads come from game context.** An effect reads the world, the cast,
  and **itself** through `gamectx`. Self is a member:
  `CastOf(ctx).Member(ownID)`. Facts *of the interaction being resolved* (who
  swung, with what, advantage sources) ride the event payload; facts that are
  *state* do not get compiled onto events.
- **R2 — Writes are requests on the bus.** An effect never mutates a sheet it
  does not own. It publishes; the owning sheet keeper applies and marks dirty.
  Context is never mutable.

## The two stages

- **R3 — The seam fetches records and installs the truth it loaded.**
  Session verbs **load-install-act-save** every participant by ID from
  repositories (load-everything), calling the door (R5) immediately after
  load — so every downstream computation, inside an interaction or not,
  inherits an installed context (D1). The only place data is fetched. No
  rules live here.
- **R4 — Resolution derives truth.** Everything ambient is a derivation of
  what the interaction already holds: room ← encounter canvas; cast ←
  attached participants; readiness ← cast.

## The one door

- **R5** — Exactly one function installs game context (working name
  `installTruth(ctx, in, cast) ctx`, in `resolution`). No other code calls a
  `gamectx.With*`. Its body is plain sequential code; dependency order is the
  documentation.
- **R6** — It runs on **every** path that folds a chain or attaches an
  effect. A path that skips the door is a bug, not a mode (see D1).

## Chain composition

- **R7 — Chains compose by sequencing, never by nesting mid-fold.** A fold
  that needs another chain's answer takes it as an **input**, folded before
  it fires (the strike folds the target's AC chain while assembling the
  attack event — "ask the AC before we fire the chain"); a handler that
  wants consequences publishes a **request** the machine answers after the
  fold completes (movement buffers reaction triggers, then runs each strike
  as its own resolution). Contributing handlers stay pure: append a piece or
  publish a request — never fold. The bus permits reentrancy (handlers are
  invoked outside its lock); determinism and resumability are why this law
  exists, not mechanics.

## Tenant admission (MUST, checked at review, not by the compiler)

- **M1** — Owned by nobody. Anything with an owner arrives by event or is
  read off a member.
- **M2** — Derived, never fetched. If it needs a repository, it is a record
  (R3), not context.
- **M3** — Mandatory and singular: installed by the one door on every path,
  one installer per fact, pinned by a structural test.
- **M4** — Absent value states the author's intent (fail-closed with the
  reason readable at the point of failure).
- **M5** — Admission is a ruling. A new tenant is a design decision brought
  to Kirk before code — never follow-the-pattern.

Tenants today: room, cast, reaction readiness. Named candidate: stance table
(rpg-project#315), entering as data behind the cast's existing questions.

## Migration (ordered so the fail-silent surface shrinks each step)

1. Preflight/standing fold paths go through the door (per D1's ruling).
2. Unarmored Defense, Martial Arts, Unarmored Movement: owner-handle reads →
   `Member(ownID)` reads. Delete the per-condition structural owner
   interfaces.
3. Write side per D3/D4: reaction spend and dirty-marking become request
   events; OA drops its direct purse/markDirty reach-through.
4. Delete `OwnerAware`/`SetOwner` machinery when its last user is gone; both
   bespoke loader wirings go with it.
5. Docs follow truth the same day: `gamectx/doc.go`, `cast.go`,
   effect-context design gets a superseded pointer, ADR noting the channel
   law if ruled.

## Decisions — ruled (Kirk, 2026-08-28)

- **D1 — cast-less paths:** verb-level install, folded into R3
  (load-install-act-save). How this landed — three reformulations, all from
  Kirk's probes — lives in this PR's commit history and brainstorm.md.
- **D2 — in-flight work:** toolkit#1284 and #1285 land as-is; migration
  happens here, not by reworking open PRs.
- **D3 — MarkDirty:** becomes a request event applied by the sheet keeper;
  the owner handle dies completely.
- **D4 — OA's writes:** fully request-shaped — publish trigger, publish
  spend; the keeper meters and dirties, keeping the character/monster
  asymmetry a keeper concern.
- **D5 — member surface:** the cast hands out the read-facing surface of
  the live object; mutation is request-only by construction.

## Done when

- Zero non-test references to `OwnerAware`/`SetOwner`.
- A new condition reading its own sheet, the world, and an enemy uses one
  channel with zero wiring changes; a structural test pins that no fold path
  skips the door.
- The three migrated conditions produce identical folds before/after
  (pixel-formula-style pinned, not round-trip-only).
- OA fires end-to-end through request-shaped writes; sheets dirty exactly as
  today.

## Non-goals

Allegiance semantics (#315) — the stance table's *content* is its own design.
Reactions opt-in UX (wave 5). Perception limiting (pre-v1 law). The
open-set installer interface and generic `Of[T]` — shelved in brainstorm.md.
