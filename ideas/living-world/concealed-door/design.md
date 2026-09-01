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
  (one or more accepted approaches, each with its own difficulty — see
  the multi-approach ruling below). It grows only when a scenario
  demands a third shape — the second-instance law applied to form
  fields.
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
  is the host's truth), rolling each concealed
  declaration's find check (any of its listed approaches — see the
  multi-approach ruling); success writes the location fact with
  audience = the searcher alone. A room with nothing hidden resolves the same
  way as a failed check — the answer never leaks the question.

- **Checks accept multiple approaches** (ruled 2026-09-01). A check is
  not one ability and one DC; it is a set of accepted approaches, each
  carrying its own difficulty — a locked door forced with Strength *or*
  finessed with Dexterity and tools; a concealed door spotted with
  Perception *or* reasoned out with Investigation. Success by any
  listed approach; the author prices each route separately. **Who picks the
  approach: postponed** (Kirk, 2026-09-01). Slice 1 pushes no choice to
  the player — search is just search, open is just open, and the
  resolver applies the character's best listed approach. The long-term
  shape, named without being adopted: the character has their sheet and
  *chooses a skill* to act on a room — approaches chosen from the
  sheet, never pushed as options. That is its own later design round;
  nothing in slice 1 forecloses it, since the checks already carry the
  approach lists that round would read.

- **Who resolves the search check** (ruled 2026-09-01, Kirk agreeing
  with the recommendation): the humble rung first — a dnd5e resolver in
  the session in the shape the examples' `dnd5eresolver` proved, using
  the character's real skills. `resolution.Resolve` becoming the
  Resolver's realest rung (integration.md Rung 5) is its own later
  slice.

The original questions are all ruled. One new question from the
independent review of the wire is open below (the hidden room's
telegraph); plan.md sits beside this file on the same PR.

## Review findings ratified into the contract (2026-09-01)

The independent review of rpg-api-protos#267 surfaced these; the first
three are mechanism rulings inside the already-ruled principle, made
with work shown.

- **The masquerade wall.** "Absent from the wire" cannot mean a hole:
  boundaries are authored walls only and a door's edges carry no
  boundary, so pure omission leaves a non-knower a visible gap in the
  wall run exactly where the secret is — the absence itself leaks. The
  fiction already says what belongs there: a concealed door IS a wall
  until found. So a non-knower's atlas presents a synthetic ordinary
  boundary at the door's edges (indistinguishable from an authored
  wall), and DoorRevealed's boundaries replace the mask with the truth.
  Refined statement of the absence law: the door is absent from every
  door-list; its geometry is masked as wall. The mask is not a flag —
  nothing marks it, and no message shape changes.
- **The probe law.** Everywhere a door id is spoken (OpenDoor, Unlock),
  a concealed unfound door answers NOT_FOUND, byte-identical to a door
  that does not exist — a DC-naming refusal would confirm existence to
  a guessed id.
- **Reveal causes are exemplary, not exhaustive.** A member who enters
  after the door was opened perceives present state (§22's truth
  grain) and gets their reveal then — the enumerated causes (own
  search, opened in presence) are examples of knowledge arriving, not
  the closed set.
- **Wave 1b pins:** a Move refused at a concealed unfound edge must be
  byte-identical to the ordinary no-crossing refusal; and the synthetic
  mask boundary must match the NEIGHBOURING AUTHORED RUN's height
  (walls carry per-edge height) — a standard-height mask inside a
  height-2 run is a visible notch exactly where the secret is. And
  should adjacent concealed regions ever become authorable: a boundary
  shared with a still-hidden neighbour stays withheld on reveal — the
  member-scoped answer governs, not a literal every-touching-boundary
  sweep (review round 4's interpretation pin). **Presence pierces**: a
  member standing inside a concealed region perceives it — you cannot
  occupy a secret you do not know exists — so a dungeon whose party
  start sits in a concealed room is legal authoring (the occupants
  begin knowing, everyone else begins blind), and the projection must
  reveal-by-occupancy from the first frame. And a concealed region with
  zero entrances stays as legal as any unreachable plain room — dead
  content is the author's own business, not a coherence violation. One
  expressiveness limit accepted with the frontier invariant (noted at
  #1370's final review, no severity): a whole dungeon behind a hidden
  START is only writable all-concealed — visible space seeds at the
  start, so unconcealed regions beyond a hidden start refuse; if an
  author ever legitimately wants that shape, it is a ruling to revisit
  here, not a validator bug.
- **RULED (Kirk, 2026-09-01): the room hides with its door.** Client
  fog does not hide unexplored space, so the map itself must keep the
  secret — the room "appears to be a wall unless it is found."
  Concealment extends to regions, and the law is the never-authored
  yardstick (refined in review round 3, which caught the first
  formulation leaking): a non-knower's atlas is BYTE-IDENTICAL to an
  atlas where the region was never authored — its cells, its region
  entry, its props, and EVERY boundary touching its cells, border walls
  included, all withheld. "Interior boundaries" was the leak: a border
  wall left behind has one endpoint on no visible floor, an
  impossible-in-an-honest-atlas signature certifying hidden space. With
  the whole border withheld, the door's edge already reads as solid
  mass like the rest of it — no mask is needed or wanted there (a
  synthetic wall touching void would carry the same self-marking
  signature). The masquerade wall stays confined to its own case: a
  concealed door between two spaces the member can see, where omission
  would show as a hole in a visible run. The region reveal's boundaries
  carry every boundary touching the region's cells. One accepted
  disclosure, named so it is never mistaken for a bug: a found door's
  doorways name one cell of hidden floor per entrance — knowing where
  a door is includes knowing it leads somewhere. Two knowledge moments,
  deliberately distinct: finding the door reveals the DOOR alone
  (knowing where a door is is not seeing what is behind it); the room's
  atlas slice arrives by a recipient-scoped region reveal when the
  recipient perceives the door OPEN — present at the opening, or
  walking up later (present state, §22's truth grain).
- **Authoring coherence (with the region ruling; reformulated after
  #1370's review round caught the first sentence direction-blind).**
  Regions gain a concealed marker, declared — no cascade from the door,
  per the kernel's own second-instance law. The invariant is
  boundary-shaped, not entrance-local: **the frontier between visible
  and hidden space consists of concealed doors and nothing else, and
  visible space is connected from the party start.** Two refusals,
  worded for the form-filler: an unconcealed region unreachable from
  the start without crossing hidden space refuses ("this room can only
  be entered through a concealed door — conceal the room too, or give
  it another way in"); a crossing from visible space into a concealed
  region that is not a concealed door refuses ("a walk-in room cannot
  be a secret"). Everything wholly inside hidden space — interior
  doors, open passages between two concealed rooms of one secret suite
  — is nobody's business, and everything wholly inside visible space is
  free. The entrance-local first draft refused the minimal honest
  dungeon (a visible start room whose only crossing is the one
  concealed door) and made secret suites unauthorable; the frontier
  form keeps every true refusal and drops the false ones. A room with
  one open door and one concealed shortcut stays legal — the room is no
  secret, the shortcut is; a concealed start region stays legal —
  presence pierces at runtime.

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
