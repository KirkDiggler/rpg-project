# Dungeon YAML Spec v0.3 — context and rationale

`spec.md` in this directory is the normative contract, **RATIFIED 2026-08-05**. This
file is everything that isn't: what the spec is for, why it says what it says, what
disagreements it found between sources, the ratification record, and where the
executable examples live. Read `spec.md` to implement against; read this file to
understand why `spec.md` looks the way it does.

## What this is

This is the versioned contract between the builder side (which authors the dialect
ahead of what the platform compiles — `rpg-dnd5e-web`'s `TARGET-YAML.md`/specimens)
and the platform side (`rpg-toolkit`/`rpg-api-protos`/`rpg-api`, which implements
against a ratified level). Kirk's ruling (verbatim, rpg-project session): "a
sustainable way forward is for us to create the 0.2 spec and then they can build for
that... let's get a solid 0.2 or 0.3 spec and start from there."

`spec.md` is the **contract baseline**; it is not itself a request for platform
implementation, and it does not claim any implementation, test, or merge evidence.

## Cadence, and document version vs. spec level

The builder authors spec v*N*+1 while the platform implements spec v*N*. Proposals for
the next level amend into a `vNext` draft. Version skew between what the builder has
authored and what the platform has shipped is the **designed steady state**, not a
defect to close — see `TARGET-YAML.md`'s "this gap is permanent, not a phase."

**Do not conflate the dungeon YAML document's `version:` field with the spec level.**
The document's own `version:` stays `1` for every construct in this spec, including
everything newly promoted in the v0.3 cut. Per Kirk's settled additive model
(rpg-project#175), a new optional field does not bump the document version; only a
real incompatible/breaking change would, and none is proposed here. Spec levels (v0.1,
v0.2, v0.3, ...) are contract/pack versions — they track how much of the dialect a
conforming implementation is expected to accept, entirely independent of the one YAML
`version:` integer a document carries. `spec.md`'s Ground Rules state the actual
document-level requirement (`version:` must be exactly `1`); this paragraph is only
explaining why that requirement doesn't change as the spec itself moves from v0.2 to
v0.3 to v0.4.

Aligns with, and adopts the version number of, `rpg-dnd5e-web`'s specimen pack
(`src/concepts/dungeon-builder/specimens/`, currently v0.3) — going forward, spec vN
and web-pack vN name the same contract level.

## How to read `spec.md`

`spec.md` is table- and rule-first: YAML shape blocks, field tables, numbered MUST/MUST
NOT requirements, and acceptance criteria per construct. It carries no narrative, no
"why," and no history. Before ratification, six questions genuinely weren't settled,
and `spec.md` marked each **OPEN — pending ratification** with a one-line pointer into
this file's Reconciliation notes rather than picking an answer. Kirk ratified all six
on 2026-08-05 (Ratification record, next section) — `spec.md` now states every one of
them as a normative rule directly, with no OPEN markers remaining. Code citations
(`file.go:line`, function names, literal error strings) live in `spec.md` because they
pin exact required behavior; GitHub comment/issue provenance ("per the 19:27 comment,"
"the checkpoint says") lives here instead, because it explains where a rule came from
rather than stating the rule itself.

## Ratification record — 2026-08-05

Kirk ratified all six points from `spec.md`'s pre-ratification decision surface.
Answers below; `spec.md` states each as a normative rule directly.

1. **v0.3 level cut** — ratified as drafted. No change to the construct groupings in
   `spec.md` §1.
2. **`cells:` ruling + #175 Specimen Pack v0.2 supersession** — ratified; already
   normative in `spec.md` §4.10.1. The named follow-up debt (`design.md:5`/
   `plan.md:7` still pointing at the superseded comment) was paid separately —
   rpg-project#195 / PR #196.
3. **Canvas + non-empty `rooms:` in one document** — ratified **reject**, adopting
   this spec's own recommendation verbatim. `spec.md` §4.5 states it as a MUST-reject
   rule with an acceptance criterion.
4. **`rooms:` + `regions:` in one document** — ratified **reject**, adopting this
   spec's own recommendation verbatim. `spec.md` §4.10.3 states it as a MUST-reject
   rule. Kirk's rationale, verbatim: "this is what I originally had in my head when
   we came up with regions — I saw them as rooms in toolkit... being able to relax it
   and not create something new if it ever comes up makes this an easy call." A
   **future-relaxation path is named, not ruled out**: authored regions layered over
   generated/chain content is a legitimate vNext proposal if a real use case appears,
   and a chain-to-canvas converter (letting an author "promote" a `rooms:` document
   into `canvas:` + `regions:` losslessly) is the tool that would make that relaxation
   cheap — not a chain/region cross-validator.
5. **Authoring-projection scope** — ratified **adopt the fuller projection**, adopting
   this spec's own recommendation verbatim. `spec.md` §4.5 now requires `FloorPlan` to
   project canonical structural-floor cells for a canvas document; `spec.md` §4.10.4
   now requires the authoring wire to carry region extents and a toolkit-derived
   parent id. The former "not required by this list" hedge is removed from both
   sections' acceptance criteria.
6. **Canvas-mode top-level `facing:`** — ratified **accepted**, under the same
   conditions `spec.md` §4.9 already applies to a room-scoped entry (non-monster,
   `mount: floor`). This spec gave no recommendation; the platform team's rationale:
   the capability belongs to a floor prop, not to room ownership — a placement's
   entitlement to `facing:` was never really about which list it lives in.
   `specimens/canvas.yaml:14`'s top-level altar (`facing: W`) is now conforming.

## Supersession: the #175 Specimen Pack v0.2 lineage

This spec supersedes the [Specimen Pack v0.2 comment](https://github.com/KirkDiggler/rpg-project/issues/175#issuecomment-5185751479)
lineage on rpg-project#175 as grammar/acceptance authority for `regions:` — most
concretely, its `extent: {min,max}` rectangle grammar, which Kirk has ruled out.
`cells:` (painted cell lists) is the wire representation, matching `rpg-dnd5e-web`'s
`TARGET-YAML.md` and its live specimen pack. Full discussion, including the
`design.md`/`plan.md` follow-up-edit debt this supersession created, is Reconciliation
notes item 1 below — that debt was paid separately, rpg-project#195 / PR #196.

## The two-floor-source finding

`spec.md` §4.5 states, as bare rules, which validators canvas mode must skip entirely
and which must switch their floor source. The underlying discovery is worth recording
here because it's counterintuitive and easy to re-break: `dungeonspec.Validate` (v1,
room-chain path) contains **two independently-implemented, room-only floor-derivation
functions**, not one — `semanticFloorCells` (`validate.go:546-568`, used by
`validateWalls`/`validateWallEndpoint`) and, in a different package entirely,
`semanticDungeonFloorHexes` (`authored_edges.go:118-138`, used by `InitDungeon`'s
`validateAndNormalizeAuthoredEdges` at persist time). Both hardcode the same
room-chain arithmetic (room widths, left-to-right, minus the connector-gap column),
and both need their own canvas-mode branch — extending one without the other leaves a
silent gap at whichever validation path wasn't touched.

The concrete, verified consequence: with `rooms: []`, both functions compute
`totalWidth = 0`, which collapses every in-bounds column check to `[0,0)`. A
canvas document's `walls:`/`start:` are not merely *unsupported* today — they are
*rejected as out-of-bounds for every possible coordinate*, because the room-only
math has nothing to range over. This is why `spec.md` states the floor-source swap as
a requirement rather than a "nice to have": without it, canvas mode's own
acceptance criteria (an in-bounds canvas document validates) are unreachable.

The boss-cluster skip list has a parallel discovery. `validateBossCardinality`
returns a `*RoomSpec` that three *later* checks in `Validate`'s sequence dereference
with no nil guard: `validateBossAxis` (`:263`, `min(bossRoom.Width, height)`),
`validateM1Restrictions` (`:175`, `bossRoom.Boss.At`), and `validateBossRef` (`:274`,
`refParts(bossRoom.Boss.Ref)`). Skipping cardinality's error return without also
skipping all three of these means a canvas document triggers a nil-pointer panic, not
a wrong-but-safe validation error — `validateBossRef` in particular sits after the
obstacles loop in `Validate`'s call order, which is probably why it reads as
separate from the boss cluster rather than a third instance of the same hazard.

## Regions: the scopes model, explained

`spec.md` §4.10.2 states eight numbered MUST rules with no rationale attached. The
rationale, settled across five comments on rpg-project#180
(2026-08-04, 19:27–21:50 UTC — consumer position, refinement, representation/
resolution, runtime convergence, platform decision checkpoint), is recorded here.

**Hierarchy is derived from cell-set containment, never authored, and never a
`parent:` field.** Four reasons, from the 19:52 comment: no donut problem (a parent's
cells are its true full extent; deleting a child leaves the parent whole; repainting a
child edits one list); the contradiction class (a child spilling outside its parent)
is unrepresentable, since geometry is the only authority; a `parent:` field would
duplicate what the cell sets already say, and duplicated truth drifts; flat lists keep
diffs small and comments attached to the right list. Because overlap is allowed only
by containment, all regions containing a given cell form a **chain** — any two
regions sharing a cell must be nested, hence comparable. This is what makes "the
parent" and "the innermost region" well-defined, cheap lookups rather than an
ambiguous multi-parent graph.

**Per-property resolution, illustrated.** The vault (innermost) declares `audio:` —
the vault's value applies. The vault is silent on `lighting:` — its parent (the
crypt)'s value applies. Nothing in the chain declares a property — root's default
applies. A region being silent on one property never forces it to inherit *every*
property from its parent; each property is resolved independently, walking the same
containment chain.

**Future enter/exit seam, illustrated.** A future capability may derive transitions
from scope-chain changes: entering a nested vault could transition into the vault
without leaving the crypt, while exiting could return to the crypt scope. Wave 1
neither emits nor requires region transition events and adds no trigger capability.

**The runtime substrate already exists; regions extend it rather than replacing it.**
Authored `region.id` compiles to the toolkit's existing `RegionData.ID`, which already
stamps `ZoneID` on the perception/knowledge layer (`encounter/perception/knowledge.go`'s
`HexObservation`, `encounter/knowledge.go`'s `refreshObservations` via
`Space.RegionAt(h)`) and is carried on the wire today as `HexRecord.zone_id`. Today's
compiler already does a strict 1:1 room→region compile via `compileRoom` →
`DungeonRegionParams`, using the identical archetype vocabulary. #180/v0.3 is authors
painting *more* of that same structure — including nested ones — onto the mechanism
that already drives fog, spawn seeding, and door naming, not inventing a second
runtime concept.

**Why region ids must be stable.** Every future per-scope semantic (audio triggers,
lighting scopes, quest references, fog memory — the extension seam `spec.md` §4.10.5
exists for) hangs off the id. Repainting a region's cells must not change its id, or
every attached future semantic loses its attachment point on the next edit. This is
the reasoning behind `spec.md` §4.10.2 point 8's non-invalidation requirement, which
is also the invariant behind Kirk's own framing (#180, 19:27 comment): "deleting a
region must never break props; that would be an authoring-feel disaster."

## Per-construct notes

**Connectors (`spec.md` §4.3) are a hard topological ceiling, not a permanent one.**
Because a connector only ever joins `rooms[i]` to `rooms[i+1]`, a room-chain document
can only express a **linear** dungeon — no branching, no loops, no room with two
distinct connections. That's a property of the current chain encoding, not a
permanent geometry model: once authored canonical edges and cell-authored semantic
regions are compiled, a room means a stable gameplay region, not one link in an
ordered array. An inner wall affects movement and line of sight without splitting a
room; an author draws a second semantic region only when gameplay identity — not
geometry alone — requires it. Branching topology follows from that same
canonical-edge/semantic-region model, not from generalizing `connectors:` itself.

**Walls (`spec.md` §4.7): authored-door symmetric interaction is a real behavior
change from the legacy connector door, not a retrofit.** The legacy single-position
connector door (`DoorData.Position`) is not touched by this spec; an authored edge's
door gets its own, new, endpoint-derived identity and interaction path.

**Top-level placement (`spec.md` §4.6) does not weaken rooms into disposable
geometry.** Rooms remain stable semantic regions owning reveal, placement, spawning,
scripting, and archetype meaning; they are not *existential* placement containers — a
placement does not stop being real just because no room claims it yet. A canvas-mode
top-level placement does not imply a synthetic room exists.

**Facing (`spec.md` §4.9): the corrected error-order finding.** `TARGET-YAML.md`'s own
probe table records `validateTopLevelPlace`'s error order backwards — as if the
top-level-unsupported error fires before facing is ever checked. `spec.md` corrects
this rather than inheriting it: `validateTopLevelPlace` (`validate.go:379-392`) checks
`facing` before `mount`, and only reaches the top-level-unsupported error once it has
confirmed no entry sets either.

**`wallLines:` (above v0.3, `spec.md` §2) is unfiled specifically because its
footprint rule has no issue to land in yet.** Kirk's rule for a straight wall: every
hex the line genuinely passes through is blocked, unless the line only touches a
vertex or runs exactly along one of the hex's own edges — "any hex that is not 100%
uncovered would not be traversable." `TARGET-YAML.md`'s implementation names the
precise epsilon (`FOOTPRINT_EPSILON`, `BOARD_HEX_SIZE * 1e-3`) that turns that rule
into a real touch-vs-clip test. None of this is in scope for v0.3; it's recorded here
so a future issue filing has the rule already written down instead of re-deriving it
from the builder's implementation.

## Reconciliation notes

Eight source disagreements found while drafting this spec, across two drafting passes
(one at initial submission, three more — items 6, 7, 8 — during gate repair). Flagged
rather than resolved silently.

1. **`extent: {min,max}` vs. `cells:` for region shape.** The [Specimen Pack v0.2 comment](https://github.com/KirkDiggler/rpg-project/issues/175#issuecomment-5185751479)
   on rpg-project#175 (2026-08-04, "canonical target") used rectangle
   `extent: {min: [c,r], max: [c,r]}` for every region example. This directly
   contradicts `TARGET-YAML.md`'s settled shape, the #180 discovery comment's own
   concrete proposal (`cells: [[9,2],[9,3],...]`), and the specimen pack the builder
   actually generates and round-trips through real mutators (`createRegion`/
   `addCellToRegion`/...). **Resolution: `cells:` wins.** Kirk has ruled out `extent:`
   rectangles for regions; `extent:` may return in a future spec level as authoring
   *sugar* that expands to `cells:` at parse time, but never as the storage/acceptance
   grammar. `spec.md` §4.10.1 states this as a bare rule. **Named debt, not fixed in
   this PR**: `ideas/battlemap/dungeon-builder/design.md:5` and `ideas/battlemap/dungeon-builder/plan.md:7`,
   both on `main` today, still open by pointing to that same superseded comment as
   "the target grammar and acceptance authority" — an implementer starting from
   `plan.md` (the merged wave doc, where they will start) follows that line straight
   to the rectangle grammar this item rejects. This spec's supersession, above, names
   both lines explicitly for exactly that reason, but does not edit `design.md`/
   `plan.md` themselves — different doc lineage, tracked here as a follow-up edit
   those two files need on `main` once this spec is ratified. **RATIFIED 2026-08-05
   — debt paid**: rpg-project#195 / PR #196 edited both lines to point at the
   current versioned spec directory instead of a fixed comment.
2. **Flat/disjoint-only vs. nesting-by-containment for regions.** `TARGET-YAML.md`'s
   own "Invariants — validated client-side" section states a bare "**Non-overlapping**
   — no cell may belong to more than one region at once," with no containment
   exception, and its "Open questions" section does not mention nesting at all. This
   reflects the client's *current* implementation (`regionGeometry.ts`,
   `validateRegionCells`), which predates the five-comment "regions are scopes"
   refinement on rpg-project#180 (2026-08-04 19:27–21:50). `ideas/battlemap/dungeon-builder/design.md`/
   `plan.md` already carry the *later*, nesting-capable language ("the parent is the
   smallest strict superset... disjoint regions are siblings; equal or partial overlap
   is invalid"). **Resolution: the nesting-by-containment model wins for this
   contract** (`spec.md` §4.10.2) — it is what `design.md`/`plan.md`/the #180 issue
   body already ratify, and what this spec is a contract *for*, not a description of
   what one existing client build already exercises. The client-side validator will
   need a follow-up round to actually author a nested region through the UI; that gap
   is real and is noted here, not hidden.
3. **"Complete absolute extents" (design.md/plan.md's own wording) is ambiguous**
   between "extent" as an abstract cell-set description and "extent" as the literal
   `extent: {min,max}` rectangle grammar item 1 above rejects. **Resolution**: read
   "extents" in `design.md`/`plan.md` as the abstract sense (a region's complete,
   explicit cell membership) — consistent with item 1's resolution — never as an
   endorsement of rectangle grammar.
4. **Canvas-mode vs. room-chain-mode exclusivity is unstated by every source.**
   Neither `design.md`/`plan.md`, rpg-project#192, nor `TARGET-YAML.md` states what
   happens if a document declares both non-empty `rooms:` and a `canvas:` block.
   `spec.md` §4.5 marks this OPEN with a recommendation (reject the combination) but
   does **not** treat it as ratified — it needs an explicit Kirk call, the same way
   the `rooms:`/`regions:` precedence question (item 5) does. **RATIFIED 2026-08-05
   — reject**, adopting the recommendation verbatim; `spec.md` §4.5 states it as a
   MUST-reject rule.
5. **Whether a document may combine non-empty `rooms:` with declared `regions:` is
   the one open question here — precedence itself is already settled.** `spec.md`
   §4.2 ("Superseded when `regions:` are declared") and §4.10.2 point 7 ("Archetype
   authority follows scopes, exclusively") already state, per the 19:27 comment and
   the platform checkpoint, that region scopes win over room archetypes for
   validation the moment any `regions:` exist — that part is not open. What
   `TARGET-YAML.md`'s own open question actually leaves unresolved is narrower:
   whether the *combination itself* (both constructs non-empty in one document)
   should be flatly rejected outright, or silently tolerated with regions simply
   taking validation precedence. `spec.md` §4.10.3 marks only that narrower question
   OPEN. **RATIFIED 2026-08-05 — reject**, adopting the recommendation verbatim (see
   the Ratification record's item 4 for Kirk's rationale and the named
   future-relaxation path); `spec.md` §4.10.3 states it as a MUST-reject rule.
6. **The authoring-projection scope in `spec.md` §4.5/§4.10 is thinner than what
   `plan.md`/`design.md`/the platform checkpoint ask for, on both waves — found while
   drafting, not resolved.**
   - **Wave 0.** `plan.md:13` asks the proto to define "authoring canvas dimensions,
     **projected canonical structural-floor cells**, and canonical physical edges";
     `design.md:11` says "Authoring projection exposes those explicit canonical cells
     and canonical physical edges." `spec.md` §4.5 requires only dimensions
     (`rooms: []` + `canvas.width`/`canvas.height`) plus authored edges — no
     projected floor-cell list. This spec faithfully transcribed #192's own scope
     item 6, which is itself dimensions-only; #192 and `design.md`/`plan.md`
     disagree with *each other* here, and this spec inherited #192's side without
     flagging it, until now.
   - **Wave 1.** `plan.md:22` asks for "authoring extents plus toolkit-derived parent
     ID and runtime `Zone.parent_id`"; the platform checkpoint says "A minimal
     runtime addition is derived `Zone.parent_id`, while the authoring FloorPlan
     needs explicit region extents." `spec.md` §4.10's acceptance criteria cover
     only the runtime side (`Zone.parent_id`, per-hex innermost `zone_id`,
     fog-authorized) — nothing requires the authoring wire to carry region extents
     or the derived parent id. Today's `FloorPlan`
     (`dnd5e/api/authoring/v1alpha1/service.proto:113-133`) carries `rooms`/
     `connectors`/`height`/`door_row`/`entrance`/`edges` and nothing region-shaped —
     this is real, currently-unrequested proto surface, not an oversight in an
     existing field.

   **Both positions, and a recommendation.** For the fuller projection: an authoring
   board needs to *render* the compiled result, not just validate against it — with
   no floor-cell/extent echo, a client either re-derives geometry itself (the
   re-canonicalization the Boundary Rule forbids) or renders only what it locally
   authored, which silently disagrees with the server the moment authored and
   server-derived content interact. For the leaner dims-only wire this spec
   currently specifies: it's the minimum a client already holds from its own
   outgoing `PutDungeon` call, and every added `FloorPlan` field is permanent proto
   surface and a second place truth can drift from `regions:`/`canvas:` themselves.
   **Recommendation: adopt the wave docs' fuller projection** (projected floor cells
   for Wave 0; extents plus derived parent id for Wave 1) — the rendering need is
   concrete and immediate, the leaner wire's savings are speculative. This is a
   genuine Kirk/platform ratification point; `spec.md` §4.5's and §4.10's acceptance
   criteria are deliberately **not** amended to require it until that ratification
   happens.

   **RATIFIED 2026-08-05 — adopt the fuller projection**, adopting the
   recommendation verbatim. `spec.md` §4.5's acceptance criteria now require
   `FloorPlan` to project canonical structural-floor cells for a canvas document;
   `spec.md` §4.10.4 now requires the authoring wire to carry, per declared region,
   its `cells:` extent and a toolkit-derived parent id. The "not required by this
   list" hedge is removed from both sections.
7. **`RegionDoc.archetype` is required in the builder's real TypeScript type, while
   this spec (following `design.md`/#180) makes it optional.** `dungeonYaml.ts`'s
   `RegionDoc` declares `archetype: string` (not `archetype?: string`), and
   `createRegionNode` (`dungeonYaml.ts:1858-1867`) sets `obj.archetype`
   unconditionally while gating `name` on truthiness — the client cannot author a
   region with an inherited/omitted archetype today. The contract direction in
   `spec.md` §4.10.1/§4.10.3 (optional, inherits per §4.10.2 point 5) is correct per
   #180's body and `design.md:13`, both of which say archetype is optional and
   inherits when omitted; this is the same class of client-behind-the-model gap as
   item 2 above, and — per this appendix's own discipline — it should have been
   listed there and wasn't. Flagged now: a future builder round needs to make
   `archetype` genuinely optional client-side to match this contract.
8. **Canvas-mode top-level `facing:` — was an open ratification point, not resolved
   by this spec as drafted; RATIFIED 2026-08-05 — accepted (see the Ratification
   record's item 6 for the platform team's rationale).** `spec.md` §4.9 states the
   current, corrected `validateTopLevelPlace` error-ordering as fact (facing checked
   before mount, verified against source); at drafting time it did not decide what
   canvas mode *should* accept for `facing:` — that question is what got ratified.
   #178's own scope
   line, quoted verbatim rather than paraphrased, is "Add
   `facing: E|NE|NW|W|SW|SE` to existing room-scoped `place:` entries" — written
   before canvas mode existed as a document shape, so "room-scoped" there names the
   only entry type #178 ever faced, not a considered exclusion of one it didn't. Two
   readings follow, and this spec does not pick between them:
   - **#178-scoped exclusion (conservative).** `facing:` stays rejected on every
     canvas-mode top-level entry, room-scoped or not, because #178 never named
     canvas mode as accepted; extending it would be extrapolation, not a reading of
     settled text. Cost: v0.3 as cut makes `facing:` unusable anywhere in canvas
     mode — the exact from-scratch authoring flow Wave 0 exists to enable — and
     makes the builder's own flagship specimen non-conforming:
     `specimens/canvas.yaml:14` emits `facing: W` on its top-level altar placement
     today.
   - **Canvas-mode extension.** Accept `facing:` on a canvas-mode top-level entry
     under the same conditions `spec.md` §4.9 already applies to a room-scoped one
     (non-monster, `mount: floor`), reading "room-scoped" in #178's scope line as
     naming the only shape that existed when #178 was written, not a deliberate
     exclusion. Cost: extends accepted surface beyond what #178's text literally
     settled, without a #178-equivalent approval covering the extension
     specifically.

   Neither reading would mislead an implementer once chosen — both are unambiguous
   and testable. This spec did not choose; Kirk decided. **The canvas-mode extension
   reading won** — `spec.md` §4.9 now states `facing:` acceptance as scoped to
   non-monster, `mount: floor` placements regardless of which list they live in, and
   both `spec.md` §4.6's example and its own acceptance criteria cover the
   canvas-mode top-level case directly.

## Specimens — the executable half

The executable half of this contract lives in `rpg-dnd5e-web`'s
`src/concepts/dungeon-builder/specimens/` (`kitchen-sink.yaml`, `canvas.yaml`, plus
the `v1-subset` strip and `dropped.json` evidence), currently at **pack v0.3**
(2026-08-03/04). That pack already emits `regions:` as `cells:` lists (not `extent:`
rectangles), matching `spec.md` §4.10.1, and already separates `canvas.yaml` as its
own document shape, matching `spec.md` §4.5.

**Pack v0.4 will be regenerated to exactly this spec cut once this PR is approved** —
that regeneration is a follow-up in `rpg-dnd5e-web`, a different repository, and is
explicitly **not** done as part of this PR. Until then, the current v0.3 pack is
representative but predates one thing this spec adds precision to: the
region-nesting/containment model (`spec.md` §4.10.2 — the pack's own client-side
validator currently only enforces flat/disjoint, item 2 above). Ratification resolved
the other two things the pack used to predate: the canvas/room-chain mode-exclusivity
question (ratified reject, item 4 — moot for this pack anyway, since `canvas.yaml`
never combines both) and canvas-mode top-level `facing:` (ratified accepted, item 8) —
`canvas.yaml:14`'s `facing: W` on a top-level altar placement is conforming outright
now, not a gap.

## Pointers

- `ideas/battlemap/dungeon-builder/design.md`, `ideas/battlemap/dungeon-builder/plan.md` — the platform's
  two-wave delivery structure this spec's level cut stays consistent with.
- rpg-project#180 — Wave 1 tracker; read every comment, not just the issue body, for
  the regions-as-scopes model `spec.md` §4.10 states as bare rules and this file
  explains.
- rpg-project#192 — Wave 0 tracker (canvas-space compilation).
- rpg-project#184–#191 — the future-construct queue, `spec.md` §2.
- `rpg-dnd5e-web` `src/concepts/dungeon-builder/TARGET-YAML.md` and `specimens/` — the
  dialect source of truth and its executable half.
