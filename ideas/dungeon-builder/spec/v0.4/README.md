# Dungeon YAML Spec v0.4 — context and rationale

`spec.md` in this directory is the normative contract, **PROPOSED 2026-08-08, not yet
ratified.** This file is everything that isn't: what the spec is for, why it says what
it says, the alternatives it weighed and rejected, and the open points Kirk and the
platform team need to rule on before `spec.md`'s header can flip PROPOSED → RATIFIED.
Read `spec.md` to implement against (once ratified); read this file to understand why
`spec.md` looks the way it does.

## What this is

The versioned contract between the builder side (`rpg-dnd5e-web`'s `TARGET-YAML.md`/
specimens, which authors the dialect ahead of what the platform compiles) and the
platform side (`rpg-toolkit`/`rpg-api-protos`/`rpg-api`). `../v0.3/spec.md` is
RATIFIED and, as of this writing, **fully implemented server-side** — Wave 0 (canvas)
is LIVE VERIFIED and Wave 1 (regions) has an exploratory posture settled on
rpg-project#200. Per the cadence `../v0.3/README.md` names ("the builder authors spec
v*N*+1 while the platform implements spec v*N*"), this document is that next cut: the
builder authors v0.4 while the platform builds out v0.3.

`spec.md` is the **proposed contract baseline** for the next level; it is not itself a
request for platform implementation, and it claims no implementation, test, or merge
evidence. It is a PR review surface — see "How this gets ratified," below.

## Kirk's ruling — the headline, verbatim

2026-08-08, in response to the rim-edge/canvas-floor gap this document exists to
close:

> "I think the floor is the region and we can see what is inside the walls. I think
> the canvas is available space in the builder but only regions carry into the game."

Two sentences, two distinct claims, both load-bearing:

1. **"The floor is the region"** — canvas-mode structural floor stops being the whole
   canvas rectangle and becomes the union of whatever cells are painted into declared
   regions. `spec.md` §4.5.4.
2. **"The canvas is available space in the builder but only regions carry into the
   game"** — the canvas keeps its authoring-workspace role (bounds, coordinate
   legality, the outer extent a builder UI renders) but stops being a game-facing
   geometry claim on its own. `spec.md` §4.5.4/§4.5.6 draws exactly this line: canvas
   dimensions still bound *where a coordinate is legal to write*; they no longer
   determine *what exists as floor*.

Everything else in this document — the envelope model, the doorway question, the
`spec:` marker — is this ruling worked through to its consequences, not a separate
set of decisions layered on top of it.

## Why this was needed: the rim-edge story

`../v0.3/spec.md` §4.7.2 requires both endpoints of an authored `walls:` edge to be
hex-adjacent floor cells. Under v0.3's canvas-mode floor rule (the full
`[0,width) × [0,height)` rectangle), that requirement quietly makes an entire class of
walls **inexpressible**: a wall on the very rim of the canvas has no floor cell on its
outward side to be the edge's other endpoint — there's nothing there to be adjacent
to, floor or otherwise.

The smallest exhibit makes this exact and countable rather than hand-wavy: take a
**1×1 canvas** (`canvas: {width: 1, height: 1}`), the minimal legal canvas under
`../v0.3/spec.md` §4.5 (positive dimensions, nothing more required). Its single floor
cell is a hexagon with exactly **six edges**. Every one of those six edges has void on
the far side — there is no second cell, anywhere, for any of them to be adjacent to.
All six fail `walls:`'s endpoint-floor-membership rule identically; none can be
authored, not even to say "wall here" on a boundary an author can plainly see needs
one. Scale the canvas up and the count of *rim* edges (as opposed to interior ones)
grows with the perimeter, not the area — but the 1×1 case shows the whole class in one
cell, with a number small enough to state without hedging: **6 rim edges, 0
authorable.**

This isn't a corner case nobody would hit. Every canvas-mode document has a rim, and
under v0.3's rectangle-floor rule the rim is exactly the boundary a real dungeon most
wants to wall — the edge of the playable space. v0.3 shipped this gap knowingly
(`../v0.3/spec.md` §2 lists `wallLines:` as unfiled specifically because "its footprint
rule has no issue to land in yet," and the rim problem was never separately named as
its own gap) rather than as an oversight; this document is where it gets a name and a
fix.

## The fix: envelope walls, generalized from a mechanism that already exists

Room-chain mode never had this problem. `../v0.3/spec.md` §1(b) lists "generated
wall/door truth" (rpg-api#769/rpg-toolkit#881) as already real: a room chain's
generator produces a perimeter of solid edges around its rectangle automatically —
nobody authors the outer wall of a room, it's just there, generated from the room's
own geometry. The rim-edge problem was never "canvas mode has no envelope
mechanism" — it's "canvas mode's floor was a rectangle with no generated envelope
wrapped around it at all," which is a gap in what ran, not in what the mechanism could
do.

`spec.md` §4.5.12's envelope-walls rule is that same mechanism, pointed at a
different, and now non-rectangular, floor shape: **every hex edge with floor on
exactly one side is an implicit solid edge.** For a room-chain rectangle that's a
perimeter. For a v0.4 region-union floor it's the boundary of whatever shape the
author painted — which may be one blob, several disconnected blobs, or a shape with
holes, and the rule needs no special case for any of them, because "floor on exactly
one side" is already shape-agnostic. This is why the ruling above states "we can see
what is inside the walls" almost as a throwaway: once floor is a real, possibly
irregular shape instead of a rectangle, wrapping it in walls is the SAME operation
room-chain mode has been doing all along, not a new one.

The practical payoff: **rim walls stop being authored at all**, in either mode, which
dissolves the inexpressibility problem above rather than patching around it. The 1×1
canvas's six edges need no author action under v0.4 — they're simply there, the moment
that single cell is claimed by a region.

## Doorways: why there isn't one through the envelope

An envelope edge (by the rule above) always has void on one side. A door — in every
existing sense this dialect uses the word (`../v0.3/spec.md` §4.7.4's symmetric
interaction model, §4.10.4's region-attachment convenience) — is an opening that lets
something walk from one floor cell into another. An opening in the envelope would let
something walk from a floor cell into void, which isn't an opening onto anywhere; it's
a hole in the world. This document does not propose one.

Two ways to build a "passage" through the envelope were weighed and rejected before
landing on the answer `spec.md` §4.5.12 states:

- **A door-like construct that opens onto void and creates a floor stub on the other
  side automatically.** Rejected: it would silently create floor an author never
  declared, contradicting "the floor is the region" at the exact moment it's supposed
  to be governing — the new floor cell has no owning region, so it would need its own
  ad-hoc rule for which region (if any) owns it, undoing the clean containment model
  §4.10.2 already has.
- **A special `corridor`-archetype connectivity rule** — since `archetype` already has
  a `corridor` value (`../v0.3/spec.md` §4.10.1), overload it to mean "this region
  auto-connects to whatever it's nearest." Rejected: `archetype` is currently a pure
  label with zero connectivity behavior anywhere in the model (`../v0.3/spec.md`
  §4.10.2.7); giving one specific value a side effect the others don't have breaks
  that uniformity for a case the existing mechanism below already covers for free.

**The answer that needed no new mechanism at all**: `../v0.3/spec.md` §4.10.2.4
already establishes that a region boundary is semantic-only and implies no edge.
That rule was written for region-to-region boundaries generally, before v0.4 existed —
but it already says exactly what's needed here: **two regions whose cells are
hex-adjacent connect openly by default.** An author who wants two floor areas
connected draws (or extends) a region so their cells touch; nothing else is required.
An author who wants that connection *gated* draws an ordinary `walls:` door edge on
the shared boundary, exactly as they would gate any other interior passage. "Floor
bridging," in the terms this document's brief used to frame the question, isn't a new
capability layered onto the envelope model — it's the v0.3 model, unchanged, simply
being the ONLY way connectivity across the envelope now works, because there is no
other way. The simplest coherent rule won by already existing.

## `FloorPlan` projection: explicit, not client-derived

`spec.md` §4.5.10 requires envelope edges to be projected on the wire as ordinary
canonical edges, the same as any other generated or authored edge — not left for a
client to re-derive from the floor-cell set it already has. This wasn't the only
option: a client could, in principle, compute "floor cell with a non-floor neighbor"
itself and draw the implied wall without the server ever saying so.

Rejected anyway, for a reason this codebase already paid for once: `Space.walls`
(the old flat runtime wall list) was retired in favor of `HexRecord.edges`
specifically to have ONE canonical source of wall truth instead of a client
re-deriving geometry from something else and risking drift (`../v0.3/spec.md` §4.7.5).
Deriving envelope edges client-side from floor membership would reopen exactly that
seam — a second, client-computed notion of "where the walls are" running alongside the
server's canonical `HexRecord.edges`, for envelope edges specifically, the moment
regions get repainted and the boundary moves. Projecting them explicitly costs nothing
new on the wire (it's the same `HexRecord.edges` record every other edge already
uses) and keeps the single-source-of-truth property the walls-from-truth migration
already established.

## Why `spec:`, not `version: 2`, not a break-in-place

CLAUDE.md's proto-versioning section sets a real bar for when to version instead of
break: "an outside consumer, a deployed client we cannot update in lockstep, or a
migration too large for one sitting." A blunt reading might say this doesn't clear
that bar — everything's in this workspace, and the fix could ship as an in-place break
the way Fog of War did (`../v0.3/README.md` doesn't cite that precedent, but CLAUDE.md
does, at length).

**This case is different in a way that matters: an in-place break here wouldn't fail
loudly, it would silently reinterpret already-persisted content.** Fog of War's break
changed decode shape — old callers got a clear rejection, not a wrong answer. This
change, done naively, would not: every already-saved v0.3 canvas document decodes
exactly as before under v0.4's rules, just with a *different meaning* — its floor
silently shrinks from the whole rectangle to whatever its (possibly nonexistent)
`regions:` declares, which for most existing documents is empty. A previously-working
dungeon would silently have no floor at all, no error raised anywhere. That's a worse
failure mode than a hard break, not a milder one — CLAUDE.md's own trigger for
versioning ("a deployed client we cannot update in lockstep") is really gesturing at
exactly this risk: content that exists and can't be waved through a single-sitting
fix-up. A `PutDungeon` call for a canvas document can't tell, from the document alone,
whether "no `regions:`" means "hasn't been migrated yet" or "genuinely has no floor,
deliberately."

A real `version: 2` bump was considered and rejected too — not because the change
isn't real, but because `version:` (`spec.md` §3.2, `Ground rules` §1) governs DECODE
shape (`KnownFields`, structural acceptance), and nothing about this change touches
decode shape at all: every v0.4 field is additive, every existing field keeps its
existing type. What changes is a SEMANTIC interpretation of an existing field
(`canvas:`) under a new condition. `version:` is the wrong lever for a semantic
branch; `spec:` — decode-orthogonal, purely a behavior discriminator — is the right
one, and it lets both meanings coexist server-side for exactly as long as the
transition needs, with zero migration required for anything already saved.

## Reconciliation notes — section by section against v0.3

1. **Canvas floor source (`../v0.3/spec.md` §4.5.4 → `spec.md` §4.5.4).** Rectangle →
   region union, gated on `spec:`. Both live server-side during the transition; see
   `spec.md` §4.14.
2. **Root scope semantics (`../v0.3/spec.md` §4.10.2.1 → `spec.md` §4.10.2.1).**
   v0.3: unpainted cells belong to root. v0.4 (`spec: "0.4"` only): there are no
   unpainted FLOOR cells to belong to root at all — an unpainted cell isn't floor.
   Root survives purely as the hierarchy root for top-level declared regions, not as
   a cell owner. `spec: "0.3"` documents keep v0.3's rule unchanged.
3. **Rim-edge inexpressibility — dissolved, not patched.** See "The fix," above.
   No wall-authoring rule changed (`spec.md` §4.7 is byte-identical to
   `../v0.3/spec.md` §4.7); rim walls simply stopped needing to be authored.
4. **`wallLines:` (`../v0.3/spec.md` §2's "unfiled" → `spec.md` §4.13's permanent
   rejection).** Not a status downgrade — a resolution. v0.3 left it unfiled because
   "its footprint rule has no issue to land in yet"; this document gives it a
   permanent, explicit answer: never a wire construct, always compiled down to
   `walls:` client-side, with the compilation contract now stated normatively so a
   platform implementer knows what a dense, line-derived `walls:` payload looks like
   and why.
5. **`lighting:` (rpg-project#190, doc-level only → `spec.md` §4.12, doc-level +
   region-scoped).** The document-level `ambient` knob (#190's original scope) is
   unchanged in shape; this document adds region-level `lighting`, resolved through
   the SAME per-property innermost-outward chain `../v0.3/spec.md` §4.10.2.5 already
   reserved for exactly this kind of future extension — the seam gets its first real
   tenant.
6. **`height`/`offset` (rpg-project#188, entirely above v0.3 → `spec.md` §4.11,
   partially promoted).** Only the decor-grade, mechanically-inert slice graduates:
   `height` (unclamped, decoupled from `mount`, per Kirk's 2026-08-02 "any placement
   may carry height... mount:wall remains the wall-flush case") and a new `offset`
   for intra-hex fine positioning. `mount: wall`'s own edge-selection geometry stays
   above v0.4 — #188 remains open for that half.
7. **`spec:` — genuinely new, no v0.3 analog.** The first document-level field whose
   entire purpose is discriminating floor SEMANTICS rather than describing content.

## Alternatives considered, collected

Several sections above already name and reject a specific alternative inline; this
section collects the ones that don't fit naturally into the narrative above, for a
reviewer scanning for "what else was on the table."

- **Zero-region canvas validation outcome** (`spec.md` §4.5.13). Considered: reject a
  `spec: "0.4"` canvas document outright if `regions:` is empty (or resolves to an
  empty union) — "a document with no floor is not a valid dungeon." Rejected in favor
  of valid-empty: the entire capability-probed authoring loop TARGET-YAML.md
  describes (`capabilityProbe.ts`, every-keystroke `validate_only` calls) depends on
  a from-scratch document validating successfully at the moment authoring BEGINS, not
  only once it's complete — the same reason a from-scratch `rooms: []` canvas
  document was accepted in v0.3 despite compiling to nothing playable yet. Rejecting
  outright would mean the very first `validate_only` call on a blank v0.4 canvas
  fails, breaking the live-preview loop before an author paints a single cell.
  Downstream constructs (`start:`, `place:`, `walls:`) already reject against an
  empty floor through their own existing rules — no separate blanket rejection earns
  its complexity.
- **`regions:` requiring `canvas:` to be present** (`spec.md` §4.5.9/§4.10.3.7). Not
  previously stated anywhere — v0.3 left this genuinely unaddressed (a document with
  `regions:` but neither `rooms:` nor `canvas:` was simply never considered). This
  document states it as a MUST because, under `spec: "0.4"`, regions ARE the floor,
  and floor needs a coordinate-legality bound (`canvas.width`/`height`) to validate
  cells against — without one, "cell outside structural floor" (`../v0.3/spec.md`
  §4.10.3.1) has no upper bound to check against. Flagged as a genuine ratification
  point below, since it's new rather than a restatement.
- **Coverage-based (fractional) standability as a server concept.** Considered
  seriously, since the underlying geometric test (`FOOTPRINT_EPSILON`,
  Cyrus-Beck half-plane clipping — `../v0.3/README.md`'s `wallLines:` note) is already
  fully specified and implemented client-side; porting it server-side was weighed as
  "the toolkit already has everything but the code." Rejected: it would introduce a
  SECOND geometric-truth model (continuous, coverage-based) running alongside the
  discrete edge/floor-cell model `HexRecord.edges` already is — exactly the
  duplicated-truth risk the walls-from-truth migration eliminated once already.
  `spec.md` §4.13.3 states this as a permanent rejection, not a deferral.

## How this gets ratified

Per the process lesson v0.3 itself paid for (`../v0.3/README.md`'s Ratification
record — six points marked OPEN, resolved in a single 2026-08-05 pass after review):
**this PR stays open through review.** Reviewers — Kirk and platform — comment ON the
PR; rulings land as commits that edit `spec.md`'s OPEN-flagged text directly (there is
no OPEN marker syntax reused from v0.3 in this draft — every open point below is
instead stated as this document's own RECOMMENDATION, explicit enough to implement
against, but named as a recommendation rather than baked in as unconditional fact);
the header flips PROPOSED → RATIFIED in-thread; there is exactly ONE merge, at the
end, once every point below is settled. Opening a second PR to "fix" a ratification
finding — the mistake `../v0.3/README.md` doesn't itself narrate but this repo's own
CLAUDE.md names generally ("a repo gets ONE branch for a wave, not one per bug found
along the way") — is the failure mode this discipline exists to prevent.

### Open ratification points

1. **Zero-region/zero-floor `spec: "0.4"` canvas document** — recommend **valid,
   empty floor**, not a hard rejection. `spec.md` §4.5.13.
2. **The `spec:` marker's value set and defaulting** — recommend exactly two
   server-known values (`"0.3"`, `"0.4"`), omission defaulting to `"0.3"`, and
   `"draft"` as a client-local-only sentinel that never reaches the wire.
   `spec.md` §4.14.
3. **`wallLines:` wire support** — recommend **never**; permanent client-side
   projection to `walls:`, with the compilation contract stated normatively.
   `spec.md` §4.13.
4. **Coverage-based standability as a server concept** — recommend **rejected
   permanently**; traversability stays edge/floor-cell based only, never fractional.
   `spec.md` §4.13.3.
5. **Envelope edges on the wire: projected explicitly, or client-derived from floor
   membership** — recommend **projected explicitly**, reusing `HexRecord.edges`
   verbatim, per the walls-from-truth precedent. `spec.md` §4.5.10.
6. **`regions:` requiring `canvas:` present** — recommend **yes, reject otherwise**;
   this is new rather than a v0.3 restatement and deserves an explicit look.
   `spec.md` §4.5.9/§4.10.3.7.
7. **`spec: "0.3"` support's end date** — **not recommended here at all**, flagged
   as a platform/Kirk call for later: this document proposes indefinite dual support
   during "the transition" without naming when the transition ends. `spec.md` §4.14.4.
8. **`height`/`offset` scope** — recommend the narrow, decor-only slice `spec.md`
   §4.11 states (mechanically inert, non-monster floor placements only), explicitly
   NOT resolving `mount: wall`'s own edge-selection geometry (rpg-project#188 stays
   open for that). Confirm this narrower scope is the one wanted before #188 forks
   into two trackers.
9. **Region-level `lighting:` resolution mechanics** — recommend reusing
   `archetype`'s existing innermost-outward walk verbatim, with document-level
   `lighting:` read as root's own entry in that same chain (not a separate
   mechanism). `spec.md` §4.10.2.5/§4.12.

## Specimens — the executable half

Per `../v0.3/README.md`'s own precedent: `rpg-dnd5e-web`'s specimen pack
(`src/concepts/dungeon-builder/specimens/`) regenerating to this cut is explicit
follow-up work in a different repository, **not done as part of this PR** — matching
the same "pack vN+1 regenerates once vN+1 is approved" discipline v0.3's own README
states for its own pack. Until then, the current pack predates this document's
region-as-floor model entirely (it authors `regions:` as v0.3's non-floor-defining
scopes) and should not be read as demonstrating v0.4 semantics.

## Pointers

- `../v0.3/{spec,README}.md` — the ratified baseline this document is a delta on.
- `ideas/dungeon-builder/{design,plan}.md` — the platform's Wave 0/Wave 1 delivery
  docs this proposal does not edit; a v0.4 wave doc is future work once ratified,
  same "not created by this dispatch" discipline those files already name for
  themselves.
- rpg-project#180 — Wave 1 tracker; rpg-project#200 — the exploratory-scope course
  correction this document's onboarding read before drafting.
- rpg-project#186 (`end:`), #187 (`orientation:`), #188 (`mount`/`height`), #190
  (`lighting:`), #191 (`targeting:`) — the future-construct queue `spec.md` §2 still
  points at; #188 and #190 are partially absorbed by this cut, named above.
- `rpg-dnd5e-web` `src/concepts/dungeon-builder/TARGET-YAML.md` — the dialect source
  of truth this document promotes `height`/`offset`-adjacent fields and the
  `wallLines:` contract from.
