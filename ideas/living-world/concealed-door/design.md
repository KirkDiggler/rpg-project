---
name: Living World — entering the game
status: design round 2026-09-01 with Kirk; authoring model ruled, slice 1 ruled; plan.md follows design approval on the same PR
purpose: how living-world capabilities reach the live game — the builder as author, scenarios as forms, and the concealed door as slice 1
---

# Living World — entering the game

> Team explainer with diagrams: [how-it-works.md](how-it-works.md) — the
> machine end to end. This file stays the normative record.

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

- **The resolver and the no-bus law** (narrowed during the session
  wave, 2026-09-01): the session's ratified structural pin — no events
  bus in the seam, ever (the gamectx slice's TestNoBusLivesInThisModule)
  — holds. The humble resolver is therefore the first production
  CALLER of dnd5e's check machinery, not its first bus subscriber: real
  skills with proficiency and expertise, zero dice arithmetic at the
  seam. AMENDED same day, Kirk's ruling: the nil-bus parameter itself
  is the dark-times shape (an absent value that does not say what the
  author meant — the day a condition subscribes, every nil-bus call
  site becomes a silent rules bug). The fix is two total functions
  instead of one partial one (#1357): \`MakeAbilityCheck\` REQUIRES the
  bus — a full check consults the chain, period — and a separate,
  honestly-named unaided variant carries roll + modifier + DC with no
  bus parameter at all. The session's resolver calls the unaided one by
  name; absence is a choice made in the open, never a nil. The chain
  going LIVE is the resolution rung's opening move — that slice
  inherits this line. Sweep rider: saves' chain entry points audited
  for the same shape.

- **Who resolves the search check** — RE-RULED 2026-09-01, late, by
  Kirk, overriding the humble-rung ruling above after seeing where it
  led: **the session asks resolution.** The seam's own ratified pin
  said it all along ("this seam loads no sheets and folds no chains:
  ask resolution instead"), and the deeper reason is unprovability —
  for a real character we can never know that no condition applies, so
  an unaided check is a claim nobody can stand behind; the blinded
  searcher's penalty must have somewhere to land from day one. The
  resolution rung's opening move therefore arrives IN slice 1: a check
  entry in resolution (toolkit#1380) loads the character with their
  conditions, fires AbilityCheckChain through resolution's lawful bus
  (the chain's first live audience), and selects the best listed
  approach — approach selection is rules and moves off the seam. The
  session's CheckResolver capability wraps that entry: records in,
  answers out. The earlier humble-rung text stands above as the record
  of a ruling that did not survive contact — rulings carry their
  scope, and this one's scope ended the day the seam had to hold a
  sheet to honor it.

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
  here, not a validator bug. Two additions from wave 1b's build
  (toolkit#1373): **a concealed door never rides the shared moved
  beat**, found or unfound — one shared payload cannot tell knowers a
  secret without telling everyone, so the crosser learns through their
  own recipient-scoped reveal and the roster's moved beat stays silent
  about the door (an unruled leak found and closed during the build);
  and **the graph is never persisted** — only journal facts ride the
  blob, the graph reseeds from the authored field at every load, so
  who-knows-what cannot drift from the dungeon that minted it.
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

## Ruled 2026-09-01, second round (the session-wave gates)

- **Movement is sight-scoped.** Kirk: "if Finch is not in view for Bram
  then they should not get their movement events... like monsters, I
  have a last-known-location ghost but know nothing more." The intel
  model monsters already live by becomes symmetric for party members;
  the story tab may stay loose. Slice 1 implements the
  concealment-forced minimum on the engine branch: a step inside a
  region not revealed to a recipient is not delivered to that recipient
  — the trail stops at the frontier — and a witnessed crossing reveals
  by the existing perceive-the-open-door mechanism. Full sight-scoped
  movement with last-known ghosts is a named follow-up (it narrows
  full-data-until-v1.0 for movement beats and wants intel's ghost
  machinery).
- **Closing re-conceals; knowledge is permanent.** Concealment never
  globally ends. Opening makes the door perceivable and every perceiver
  learns it forever — "they saw it open and close to disappear; they
  know a door is there" — and once shut it is a wall again to
  strangers. State is reversible; knowledge is not. The engine's
  per-member facts already carry these semantics; the wave pins a
  close-re-conceals scene both ways (stranger-after-close sees the
  twin's wall; a knower keeps a shut door).
- **The gap oracle: per-recipient dense numbering** (mechanism ruled
  with work shown, inside two standing constraints — the secrecy
  principle and the wire's gapless contract; flag if wrong): each
  member's stream numbers its own deliveries densely at the session
  seam, so no member ever observes a hole; the protos comment amendment
  (seq is per-recipient) rides the api wave.

## Found by the session wave (2026-09-01, PR toolkit#1377)

- **Combat cannot form on a concealed dungeon until resolution accepts
  the capabilities** — fight formation reloads the world without
  CheckResolver/Witness and a concealed field rightly refuses. Fails
  closed and loudly today, pinned with a self-deleting test; the
  plumbing follow-up is toolkit#1378 and it GATES the done-when for any
  dungeon where a fight can start.
- **The probe law went structural at the seam**: Unlock reads the lock
  through DoorsFor(member), so no die is ever rolled against a lock the
  member has not found — enforced by shape, pinned with a dice counter.
- **Retention is storage-only** (ruled disposition for the review's
  C1 on toolkit#1377): trim may never affect delivery or numbering — a
  member's dense stream numbers what was delivered to them, the
  retained story governs only what is re-readable later, and a verb
  minting more beats than the retention window numbers and delivers
  every one before anything is trimmed. Found empirically: a 40-cell
  free-roam walk trimmed its own beats past every cursor mid-verb and
  could never commit.
- **Per-recipient numbering is cursor-based** (one persisted cursor per
  ever-member, advanced with the beats in one persist) because
  retention trims are the norm — counting from 1 was never an option; a
  cursor the trim outran fails closed by name.
- **A mid-verb read must not pass the storage boundary** (found by PR
  toolkit#1384's review, 2026-09-01): once retention enforces at
  `ToData`, that call both trims and snapshots — but session also uses
  it as its only world-snapshot read, four times mid-verb, and persist
  calls it before projection reads the story. On bump, a verb bigger
  than the window would trim past the projection baseline *through a
  read* and silently deliver zero events — the storage-only ruling
  defeated by a read-shaped call. Encounter grows a pure snapshot read
  (no trim, no floor advance; named so it cannot be mistaken for the
  blob you persist) — toolkit#1385; the session follow-through migrates
  every mid-verb `ToData` call site to it, not just the numbering
  order.
- **The split-save crash window narrows to the big-verb arm, honestly
  wedged** (rebind review of toolkit#1377, 2026-09-01; recorded, not
  patched): with numbering and delivery built before the save-point
  trim, a crash between the encounter save and the session save loses
  the cursors — for a normal verb the next load re-derives them; for a
  verb that outgrew the retention window the persisted floor has
  passed every cursor and the session refuses every subsequent verb,
  permanently, by the trim-outran guard. Fail-closed and truthful (no
  beat was delivered that wasn't saved), strictly better than the
  pre-fix behavior, and unhealable by retry — a remediation path is a
  named shelf, not slice work. The module's save-ordering doc carries
  the same admission.

## The toolkit side, complete (2026-09-01)

Every ruling above is code. Merged in dependency order, each through a
fresh-session review at zero criticals: the required bus and the
cancelled unaided variant (toolkit#1382, dnd5e v0.126.1); retention at
the storage boundary (#1384) and the pure world read that keeps a
mid-verb reader off it (#1386, encounter v0.43.0); resolution's check
entry with the fold-once reconciliation (#1387, resolution v0.27.0);
the session concealment seam — per-member reads, Search, per-recipient
dense numbering, the frontier-scoped movement minimum (#1377, session
v0.43.0); and the capability plumbing that lets a fight form on a
concealed dungeon (#1393 + #1397, closing #1378, session v0.44.0).

One pin arrived late and is now in. PR #1397's review measured that
only the fight-formation site failed when its supplied capabilities
were dropped — striker, attack and activate survived mutation, so
three of the four mid-fight reloads were supplied yet unproven; the
fold-in was lost when its builder ran out of budget and the PR merged
without it. Carried as toolkit#1398 rather than counted as done, and
closed by #1399 (session v0.44.1): the formed fight is now fought in —
the finder swings, a non-knower Dodges, the zombie takes its turn —
and all three mutants die. The scene ends where the retired tripwire
began: the finder still holds her door, and the non-knower's atlas is
byte-identical to the never-authored answer he held before the fight
existed.

**The lesson, kept:** supplied is not proven. A capability threaded
through a call site that no test drives is indistinguishable from one
that was never threaded, and the suite stays green either way. The
check that found it — mutate the site, watch what fails — is cheap
enough to be the default whenever a wave adds a capability to more
call sites than its scenes exercise.

## Walked in multiplayer, 2026-09-02 — the slice is real

Kirk drove the local dev path with two players and confirmed the centre of
the done-when: **a player searches, the door appears for that player alone,
and opening it shows the door to both.** Per-player reveal, the search verb,
and the shared reveal on open all behaved as ruled, against a
builder-authored dungeon on the merged stack.

What the walk cost, and what it bought — every one of these was invisible to
CI and only a used environment could show them:

- **Authoring a concealed dungeon was impossible.** The compile path built a
  world without CheckResolver/Witness, so `PutDungeon` refused before it ever
  wrote the file — no concealed dungeon had ever reached disk, though saves
  appeared to succeed. rpg-api#887.
- **A stale authored dungeon stops the server booting**, because one file
  that will not compile fails the whole content registry. The dialect change
  orphaned four of seven authored dungeons. rpg-api#886 — the blast radius
  should be the dungeon, not the service.
- **A concealed door defaults to OPEN.** `closed: true` is opt-in, so the
  first authored secret door stood open and revealed itself, and everyone
  present — skeletons included — learned it at first light. Working as ruled,
  and a trap the tool set for the author.
- **The frontier rule is load-bearing and the builder cannot satisfy it.**
  `hiddenFrom` has exactly one caller, the atlas projection; movement never
  consults concealment. So the validator is the only thing preventing a
  walk-in, and the author has no way to draw the walls it demands.
  web#890, toolkit#1405.
- **A latent fail-open**: `AtlasFor` returns the complete unfiltered atlas
  when `e.world` is nil. On a concealed field that is the one thing that must
  never be nil, and if it ever is, everyone sees everything, silently.

Ruled during the walk: **concealment links to the door** — the hidden space
is what lies behind a concealed door, derived rather than declared twice, so
the passage and the room cannot disagree. And the **masquerade wall extends
to every withheld boundary facing concealed space**: a wall is unremarkable,
but floor that stops in mid-air and refuses to be crossed is the tell. The
never-authored yardstick governs space and contents; it is the wrong test for
the boundary that disguises them.

## Later slices (named, not designed)

- **Door-property shelves** (Kirk, 2026-09-01, while ruling the
  witness questions — named for future door kinds; nothing built).
  Today's constants, confirmed ruled: a concealed door that is OPEN is
  known by those who witness it, and a found door never re-makes its
  find check — knowledge is permanent, opening is governed only by the
  door's own state and lock. Two shelves where those constants become
  per-door properties:
  1. *Witnessing is not always learning.* A magic door is the wall even
     while someone steps through it — comprehending what you just saw
     could carry its own check (Arcana), making the reveal a witness
     receives a door property instead of a constant. Without the
     check, you hold at most "something happened here" — a last-known
     ghost at a blank wall — not the door.
  2. *Per-passage checks.* A door kind whose crossing itself demands a
     check each time — following someone through the magic door takes
     your own Arcana — distinct from find (once, permanent) and from
     the lock (state). The approach-list vocabulary every door already
     carries is the shape both shelves would reuse.
- **The sheet-declared check** (Kirk, 2026-09-01, reading #1373): the
  successor to Search, expected soon — "ok to start here but I do not
  think we will stay here long." The player loads their sheet and
  declares intent FROM it — check with Perception, check with
  Investigation — and what they learn may differ by skill. Search
  survives as the special case (possibly the no-roll case); the
  general tool is a check that works with or without a skill roll.
  This is the postponed approach-choice ruling and the sheet-era
  dispatcher converging into one verb design round.


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
