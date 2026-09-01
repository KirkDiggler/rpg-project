---
name: Living World — entering the game
status: design round 2026-09-01 with Kirk; authoring model ruled, slice 1 ruled; plan.md follows design approval on the same PR
purpose: how living-world capabilities reach the live game — the builder as author, scenarios as forms, and the concealed door as slice 1
---

# Living World — entering the game

*Companion to brainstorm.md (§21 two-era scenarios, §22 the witness seam),
use-cases.md UC-4, and integration.md (the rungs). Journey: rpg-project#326.*

## What changed since integration.md was recorded

- **The freeze is released** — Billy, rpg-project#201, 2026-08-31, after
  encounter v0.40.0 tagged.
- **Rung 1 is merged** — rpg-toolkit#1352: `castView` reads a `world/graph`
  fold, seeded from the Input, zero behavior change. Resolution pins
  `world v0.3.0`.
- **The kernel the game needs is published**: world v0.1.0 (kernel),
  v0.2.0 (the Witness capability), v0.3.0 (concealment as declared state).
  The tomb (UC-4) proved all of it at examples scale.

Nothing structural gates the climb anymore. This design is the Rung-5
round integration.md promised.

## The authoring model (ruled 2026-09-01, in session)

**The builder is the author, and the author owns structure.** A scenario
owns its quest contract, its verbs, and the minimal entity bindings the
quest cannot work without. Everything else — placement, layout, and
whether anything sits behind a concealed door, or behind two of them —
is the author's compositional freedom, not a form field. Rooms carry no
scenario roles; what matters is what is *in* them. (This retires the
spike's fixed cast: `BossRoom`/`HiddenRoom` were scaffolding, not
contract.)

**Concealed doors are a general builder capability, not scenario
machinery.** The builder already authors doors; concealment is one more
property on a door declaration, carrying its own find check and open
check. Any scenario — or no scenario at all — can sit behind one. §22's
"concealment is content-declared" still holds: in the builder era the
builder *is* the content author.

**Scenarios arrive as forms, over an RPC.** The builder calls
`ListScenarios`; the toolkit exports, per scenario package, a form
descriptor — field key, label, type, and guidance text (the constructor
refusal messages, already written for the form-filler; §21 addendum 2
closing its loop). rpg-api translates verbatim and never learns what a
captain does. The builder renders a picker per field type. Submitting
the filled form validates through `New(cfg)` itself — the refusals come
back as inline form errors; there is no second validator to drift.

- Field-type vocabulary starts minimal: `entity_ref(kind)` and `check`
  (approach + difficulty). It grows only when a scenario demands a third
  shape — the second-instance law applied to form fields.
- The checks for a concealed door live on the **door declaration** in
  the builder, not on the scenario form — they belong to the door,
  wherever the author puts it.
- **Pinning test per scenario**: descriptor and `Config` struct must
  agree both ways (F20's two-lists shape, designed out on day one).
- The dungeon spec stores `{scenario_id, bindings}` as pure references;
  composition at run start is `New()` per placed scenario, then
  `world.Compose`, whose collision refusals are already builder-readable.

**Two spike couplings the generic tomb must undo** (found by
interrogating UC-4 against this model):

1. **Recovery is currently raised by the door opening**
   (`Raise{On: FactDoorOpened, Flag: Recovered}`). The real shape is a
   *recover/take* verb on the artifact itself, so the quest completes
   whether the artifact sits behind zero doors or two.
2. **The captain's knowledge of the door is hardcoded.** Generically it
   is an authorable **knowledge link** — "this entity knows this
   secret" — seeded by the author when placing a captain near a
   concealed door. Named here; deferred until a slice needs it. Default:
   nobody knows, search can find.

**Capabilities before whole functionality** — ruled. The game adopts
living-world capabilities one at a time; the full artifact-recovery
scenario composes out of capabilities that each earned their place.

## Slice 1 — the concealed door

*Journey #326's secret door, arriving as a capability rather than a
whole scenario. No scenario package, no quest, no RPC — just the door.*

**Done-when (observed through the named local dev path):** a
builder-authored dungeon holds a concealed door with a find check and an
open check. In play: a player declares **search**; on success the door
appears for that player alone — party-mates see nothing. A knower
succeeding the **open** check reveals the open door to everyone present.
A party that never searches finishes the run without ever seeing it.

**Journey reconciliation:** #326's outcome text says the first
capability is *passive* Perception. UC-4 and §22 later ruled explicit
search first, with passive reveal as polish reserved for the sight-seam
design round. The journey text should be updated to match the rulings —
slice 1 is explicit search.

**What it forces, per layer** (develop outside-in, merge inside-out):

- **web — builder**: door declaration grows `concealed` plus the two
  checks. **web — game**: the search verb on the player's action
  surface, targeting the room they stand in; per-player door-reveal
  beats rendered; a concealed, unknown door absent from the party's
  view.
- **protos**: dungeonspec door fields; a room-targeted search
  intent and the open intent; detection
  beats addressed per recipient. The wire is ready — `Event.recipient`
  and broker routing are live end to end, and per-player detection
  beats from birth is the standing ruling (doors/traps are exactly the
  named case).
- **rpg-api / session**: the host composes its **first world** — a
  minimal one: journal + graph, the authored dungeon's concealed
  structure as graph declarations, search/open as world verbs. The
  session's existing sight (squareSeam LOS, the shared 120ft ruling)
  implements the **Witness** capability — §22's game rung, verbatim.
- **toolkit**: whatever friction adoption surfaces — a finding, not a
  license to grow the kernel. The tomb example is precedent, not a
  dependency.

**Ruled on review (2026-09-01, Kirk):**

- **The dungeon run is the world.** One `World` per run, composed at
  run start; journal facts round-trip as data on Input/Output exactly
  the way `EncounterData` does (Rung 2's persistence shape arriving
  with its first passenger).
- **Search is a player verb, and it searches a room.** The verb is
  given back to the players — offered on the action surface, declared
  intent, universally attemptable, no prerequisites — and its target
  is a **room**, never a door: a player cannot target structure they
  do not know exists. Searching sweeps the concealed structure the
  targeted room holds (v1: the room the searcher occupies — presence
  is the host's truth), rolling the find check each concealed
  declaration carries; success writes the location fact with audience
  = the searcher alone. A room with nothing hidden resolves the same
  way as a failed check — the answer never leaks the question.

**Still open** (recommendation attached, Kirk rules on the PR):

1. **Who resolves the search check.** Recommended: the humble rung
   first — a dnd5e resolver in the session in the shape the examples'
   `dnd5eresolver` proved, using the character's real skills.
   `resolution.Resolve` becoming the Resolver's realest rung
   (integration.md Rung 5) is its own later slice.

## Later slices (named, not designed)

- **Slice 2 — the artifact**: recover verb + quest contract + the
  `ListScenarios` RPC; artifact recovery becomes the first placeable
  scenario, binding an artifact (item) and a captain (monster).
- **Knowledge links**: authorable who-knows-what seeding.
- **Passive perception**: the sight-seam design round (reserved, with
  Kirk).
- **Guild-scope world and goals**: Rungs 5–6 in full — tenancy, needles,
  deadlines, the seat.

## Boundaries held

No passive detection, no model, no seat, no tenancy. No change to the
`world` module's API unless slice-1 friction forces one — and that is a
finding to bring back here first.
