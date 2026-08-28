# Game Context — one read channel, one write channel, one door (v1)

**Status:** Design — **decided** (Kirk, 2026-08-28: "I approve and would like
to proceed"). Plan in [plan.md](./plan.md) — an agent-handoff artifact,
auto-approved as long as it honestly represents this design (Kirk's plan
ruling, same day). Reasoning and rejected alternatives in
[brainstorm.md](./brainstorm.md). This PR stays open
through implementation; an `implementation.md` lands beside this file before
merge — the mini retro: what we thought we were building vs. what we found
(Kirk, 2026-08-28).
**Final form (Kirk, 2026-08-28):** *"having session do loading that should be
happening in resolution is not a solid foundation we can build on"* — session
installs nothing; folds live in resolution. D1/D6/D8 below carry the ruled
forms.
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

- **R3 — The seam fetches records, and only records.** Session verbs
  load-act-save every participant by ID from repositories (load-everything).
  The only place data is fetched; no rules live here. Session holds *records
  of* the world and the participants — truth (a live room, a live cast)
  never exists above resolution, so session installs nothing and never calls
  gamectx. Charter check: a cast is live runtime objects — session
  installing one fails session's own law that no runtime object crosses the
  boundary.
- **R4 — Resolution derives truth.** Everything ambient is a derivation of
  what the interaction already holds: room ← encounter canvas; cast ←
  attached participants; readiness ← cast.

## Who holds what

| Truth | Held by | Installed by | Charter check |
|---|---|---|---|
| world / participant **records** | session (repositories) | never — records are not truth | verbs take IDs, repos are key-value ✓ |
| room (live world) | the encounter's canvas | the door, in resolution | encounter stays ctx-free ✓ (D8) |
| cast (live sheets) | resolution's `attachAll` | the door | session→cast fails "no runtime object crosses the boundary" → sealed (D6) |
| reaction readiness | derived from the cast | the door | derived, never fetched (M2) ✓ |

## The one door

- **R5** — Exactly one function installs game context (working name
  `installTruth(ctx, in, cast) ctx`, in `resolution`). No other code calls a
  `gamectx.With*`. Its body is plain sequential code; dependency order is the
  documentation.
- **R6 — Folds live in resolution.** When a computation needs truth, the
  computation comes to resolution — never the truth to the computation. A
  chain folded outside resolution is the bug, not a mode. The one live
  outside fold today (Join's AC read) moves inside via the projection entry
  (D6).

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

1. The projection entry in resolution; `Join` reroutes its AC read through
   it (the base-AC-barbarian pin).
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

- **D1 — cast-less paths (final form):** session installs nothing; folds
  live in resolution (R6). Four reformulations, every one from a Kirk probe,
  each smaller than the last — the record lives in this PR's commits and
  brainstorm.md. The last word: "having session do loading that should be
  happening in resolution is not a solid foundation we can build on." 
- **D2 — in-flight work:** toolkit#1284 and #1285 land as-is; migration
  happens here, not by reworking open PRs.
- **D3 — MarkDirty:** becomes a request event applied by the sheet keeper;
  the owner handle dies completely.
- **D4 — OA's writes:** fully request-shaped — publish trigger, publish
  spend; the keeper meters and dirties, keeping the character/monster
  asymmetry a keeper concern.
- **D5 — member surface:** the cast hands out the read-facing surface of
  the live object; mutation is request-only by construction.
- **D6 — the cast stays sealed:** `attachAll`/`Participants` remain internal
  to resolution; nothing is exported to session. Join's AC read becomes a
  small exported resolution **projection entry** (attach the one character,
  install the truth, fold, tear down). `compileResolutionCast` — session's
  hand-rolled preflight attach — is a named open question attached to Phase
  3, decided on evidence when that phase touches it.
- **D7 — the member surface carries `HasShieldEquipped`:** the monster
  answer is false — a monster's shield is baked into its stat-block AC.
- **D8 — the encounter stays ctx-free, no asterisk:** context never flows
  from above; resolution installs at its own boundary, the only boundary
  that matters.
- **D9 — Fighting Style Protection joins D4's family:** shield read via the
  member surface (D7), reaction spend as a request event.
- **D10 — fail loudly means observable, not refused** (Kirk, 2026-08-29, on
  the Join strictness tripwire): a character carrying a condition this build
  cannot parse still enters — "in the event we had a real bug that would
  make the character unplayable… my fail loudly is really about
  observability. it should be obvious that something failed. maybe come out
  the combat log: condition initialization failed. if we become a real game
  this is where the error code would go." Cash-out: READ entries (the
  projection) load leniently and REPORT what they dropped — the report rides
  the entry's output and session announces it (combat log is the product
  home; an error code when we are a real game). WRITE entries stay strict —
  #948's no-clobber holds: never persist a sheet that silently dropped a
  condition. Strictness is a property of what the entry does, not of
  loading; one attach mechanism, policy per entry. The two session pins that
  fired survive. **Scoped down (Kirk, 2026-08-29): for now the lenient drop
  site fires a warning log and nothing more** — "it is premature to think how
  we can get this data out cleanly. we can postpone these decisions until we
  have the structure we want setup." SHELF (named, empty): the clean data-out
  — a report type on the loader, the report riding the projection's output,
  the combat-log announcement at Join, conditions-only-vs-all-drop-sites.
  Carved when the structure exists; the warning log marks every site the
  shelf will serve.

## Done when

- Zero non-test references to `OwnerAware`/`SetOwner`.
- A new condition reading its own sheet, the world, and an enemy uses one
  channel with zero wiring changes; a structural test pins that folds live
  in resolution.
- The three migrated conditions produce identical folds before/after
  (pixel-formula-style pinned, not round-trip-only).
- OA fires end-to-end through request-shaped writes; sheets dirty exactly as
  today.

## Non-goals

Allegiance semantics (#315) — the stance table's *content* is its own design.
Reactions opt-in UX (wave 5). Perception limiting (pre-v1 law). The
open-set installer interface and generic `Of[T]` — shelved in brainstorm.md.

Shelf: `Unlock`'s ability check folds an empty chain today (bus-free sheet,
no subscribers). The day lock-picking should feel a condition, that check
becomes a resolution machine under R6 — named, empty, not built.
