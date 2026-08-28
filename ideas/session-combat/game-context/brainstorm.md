# Game Context — brainstorm: one read channel, one write channel, one door

**Status:** Brainstorm — feeding [design.md](./design.md) (PROPOSED).
**Journey:** rpg-project#253 · **Umbrella:** `ideas/session-combat/` — successor
to [effect-context](../effect-context/brainstorm.md), which this re-opens
deliberately, with new knowledge, per the 2026-08-28 working method: a scoped
ruling does not extend itself, and a fix that settles an open question is
evidence for the design conversation, not the resolution of it.

---

## The moment that raised it

Kirk, 2026-08-28, reviewing the conversion of the old encounter behavior onto
the resolution stack:

> "game context might be the answer. it seems like a clean way to get the data
> that's necessary to the conditions… the context carries runtime information
> and that's exactly what this is."

> "what felt off to me is we had this owner handle and I can't decide why that
> would be necessary if the game context had everything necessary."

> "the elegance I see missing is the methods they use to load them. load all,
> get everything on the bus kind of thing."

gamectx had been ruled on 2026-08-26 — but *scoped*: "Right now. we need the
functionality of the game context where conditions can look up the state of the
world and Mark things dirty." The pattern then accreted past its ruling: a
sixth tenant (reaction readiness, an explicit non-goal of the ruled design)
installed in toolkit#1282, movement and grant-at-attach riding the momentum.
Nobody decided gamectx was *the architecture*; every PR acted as if someone
had. This document is the conversation that was skipped.

## The owner-handle post-mortem

The split — "own sheet by injection, world by context" — was ruled 2026-08-26
because ambient state was **sometimes absent** back then: five registries,
one ever installed, a barbarian at 10+DEX with Unarmored Defense attached and
nothing logged. Injection at attach was the escape from sometimes-absent.

That disease is cured by different means now: room, cast, and readiness are
installed unconditionally on every resolve path, held structurally by the
`TestNoCodePathProduces*lessInteraction` family. The reason for the split is
dead — and the injection channel has meanwhile demonstrated the *same*
disease it was built to escape:

- `SetOwner` is wired **twice, bespoke** — `character/load.go` and
  `monstertraits/loader.go` each do their own dance. The monster half was
  removed as "unreachable" in #1281 and had to be restored in #1283. Silent,
  per-kind, discovered later: the five-registries failure shape, on the
  channel that was supposed to be the fix.
- Every nil-owner branch is fail-silent-shaped: Unarmored Defense with no
  owner quietly no-ops (`unarmored_defense.go:225`).
- The reads it carries are small: three conditions (Unarmored Defense and
  Martial Arts want ability scores; Unarmored Movement wants "am I holding a
  shield"). Everything else is writes: `MarkDirty` and the reaction purse.

Self is just a member. `gamectx.CastOf(ctx).Member(myID)` already returns the
full combat-facing sheet — the same door any condition can walk through about
its *target*. The read half of the owner handle is scar tissue from a bug we
have since fixed properly.

## The request pattern was already invented here

Kirk: "we used to mutate it in place but that felt very wrong. context is
read-only typically so we made the request pattern — the condition can request
something happen, and the character listens."

The new stack already half-runs on it: the **sheet keepers** subscribe to
`ConditionApplied` / `ConditionRemoved` / `HealingReceived`, apply the change,
and mark the sheet dirty. No condition holds a mutable character. The write
side of the owner handle is the part of the old mutate-in-place instinct that
survived the rebuild — and Opportunity Attack is the tell that we are
half-and-half: it *publishes* its trigger like a request, then reaches through
the handle to spend the purse and mark dirty like an owner.

## The elegance, named: derived, never fetched

Nothing in gamectx is fetched. The room is a read-only view of the encounter
the interaction already holds; the cast is a projection over the participants
`attachAll` already attached; readiness is arithmetic over that cast. So the
loading story is two stages with a clean boundary:

1. **The seam fetches records.** The session verb load-acts-saves, pulls every
   participant by ID from the repository — the load-everything law — and hands
   them to `Resolve`. The only place data is *fetched*; the host's job.
2. **Resolution derives truth.** One door installs the context: a single
   function that takes what the interaction holds and derives every ambient
   fact from it. Nothing else may call a `With*`.

This is "load all, get everything on the bus" applied to the data channel:
the bus and the context are loaded by the same attach, through one door.

Sanity checks against known futures: the **stance table** (allegiance,
rpg-project#315) passes — authored data fetched by the seam, derived into the
cast's `IsHostile`/`IsAllied` answers at the door. **Light / terrain /
difficult ground** are properties of the room and ride the canvas; the door
does not grow. **Turn order** is the clock's and reaches rules as events; it
stays out.

## Rejected alternatives (with the reasoning, so they stay rejected)

**A loader interface — "implement this, you get on" (Kirk's noodle,
2026-08-28, withdrawn same conversation).** The deciding law: **open sets get
interfaces, closed sets get a function.** The bus side already IS the
interface (`ConditionBehavior`: implement it, `attachAll` seats you) and is
correct there, because conditions are an open set the engine shouldn't
enumerate. gamectx is the opposite: a closed, ruled set — three tenants,
maybe four — where admission is a design moment that must come to Kirk. An
interface optimizes for frictionless anonymous admission; its success mode is
the accretion this document exists to stop. Technically it also flattens real
dependency order (room before attach; cast only after `attachAll`; readiness
from cast) into registration ceremony — a DI framework in embryo replacing
four honest sequential lines. Kirk's own close: "I already knew it's a closed
set… bringing in a new type is deliberate and functional."

**Generic `Of[T]`/`With[T]` keys (Go 1.27).** Would delete the per-fact
key+With+accessor triple. Rejected for now: the named accessors are where each
fact's semantics live in prose — `IsReactionReady`'s doc IS the fail-closed
ruling. Three facts' boilerplate is cheap rent for three load-bearing doc
comments.

**Explicit parameters on `Apply`/handlers.** Compile-checked, but ripples
through every handler signature and trends toward Frankenstein events. The
ambient channel's known risk (invisibility) is already mitigated by the
structural-test regime, which is where the five-registries lesson actually
landed.

## The ghost: paths where no interaction is running

`cast.go`'s own docs: session's standing and preflight attaches raise no cast.
Collapse self-reads into the cast without touching those paths and Unarmored
Defense inside an AC fold on a read path takes its "cannot answer" branch —
the AC-11 barbarian, re-created *by architecture*. The design's D1 prices the
two exits: every fold becomes an interaction (cast of one, same door), or a
one-method residual handle. See design.md.

## Shelves (named, empty on purpose)

- **Open-set installer interface.** If gamectx ever serves an open set —
  another rulebook contributing ambient facts, plugin-shaped — the interface
  becomes the right tool and the door is the seam it slots into. Not before.
- **Stance table as tenant four** (rpg-project#315) — enters through the door
  when allegiance is designed; the cast's questions don't change shape.
- **`HasCondition` on Cast** — stays parked as effect-context left it:
  arrives with the first predicate that needs it.

## Open questions (ledgered for the design PR)

1. D1 — every-fold-is-an-interaction vs residual `MarkDirty` handle (priced
   in design.md).
2. Whether `MarkDirty` itself becomes a request event or dies with the handle
   (D3).
3. OA's write idiom after migration (D4).
4. Sequencing of in-flight work — toolkit#1284, #1285 (D2).
