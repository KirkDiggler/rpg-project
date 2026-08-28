# Game Context — one read channel, one write channel, one door (v1)

**Status:** PROPOSED — awaiting Kirk's ruling on this PR. Reasoning and
rejected alternatives in [brainstorm.md](./brainstorm.md).
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

- **R3 — The seam fetches records.** Session verbs load-act-save every
  participant by ID from repositories (load-everything). The only place data
  is fetched. No rules live here.
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

## Decisions for Kirk

- **D1 — the cast-less paths.** Some folds run outside any interaction —
  the concrete case is AC derived on read at session join (#1279), where
  Unarmored Defense contributes with no `Resolve` running, so no cast is
  installed. Delete the handle and read self via `Member(ownID)`, and on
  that path the condition takes its "cannot answer" branch: base-AC
  barbarian again, guaranteed by architecture. Recommended: **every fold is
  an interaction** — the join-time AC read attaches that one character and
  builds a cast of one through the same `attachAll` and door. Only this
  version yields R6 with no asterisk and one loading mechanism engine-wide.
  Cost: ctx plumbing on the paths #1276/#1279 just made fallible — priced by
  a spike during step 1, evidence back to Kirk before committing. The honest
  fallback is weaker than this doc first claimed (corrected 2026-08-28 after
  Kirk's probe): a `MarkDirty`-only remnant saves nothing here, because the
  ghost is the READ side — the handle would have to keep its read methods
  for every condition that fires on an undoored path, and R1 gains a
  permanent asterisk. Reformulated twice on
  2026-08-28, both times by Kirk's probes, landing on his form: **"the world
  loads them into game context, then we fire the chain" holds on every
  path** — the session verb pattern becomes load-**install**-act-save, one
  door call right after load, so everything downstream (a Resolve, or an
  on-read AC computation at join) inherits an installed context and no
  computation needs to know whether an interaction is running. Resolve keeps
  its own door call for the interaction-derived truths (attached cast,
  readiness) — both installs are the same function, both held structurally.
  The spike confirms the verb-pattern change is the one-line-per-verb it
  looks like.
- **D5 — the member surface.** `combat.Combatant` exposes mutators
  (`ApplyDamage`, `MarkClean`), so handing the live object through the cast
  leaves R2 enforced by discipline, not by the interface. Recommended: the
  cast hands out the read-facing surface of the object (narrow interface or
  wrapper); mutation stays request-only by construction. Alternative:
  accept discipline + review, revisit on first violation.
- **D2 — in-flight work.** Recommended: land toolkit#1284 as-is
  (grant-at-attach answers *who carries*, unaffected by channel law) and
  toolkit#1285 as-is (record truth is per-change). Migration happens here,
  not by reworking open PRs.
- **D3 — MarkDirty.** Recommended: a request event ("my serialized state
  changed", ref-addressed), applied by the sheet keeper — the handle dies
  completely. Alternative if D1 falls back: keep the one-method handle.
- **D4 — OA's write idiom.** Recommended: fully request-shaped — publish
  trigger, publish spend; keeper meters and dirties. Character/monster
  asymmetry (purse vs `UsedThisTurn`) stays a keeper concern, not a
  condition concern.

## Done when

- Zero non-test references to `OwnerAware`/`SetOwner` (or, on D1 fallback,
  exactly one one-method interface).
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
