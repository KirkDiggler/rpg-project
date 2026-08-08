# Dungeon YAML Spec v0.4 — context and rationale

[`spec.md`](spec.md) is the normative contract: **PROPOSED v0.4, not yet
ratified.** This README records why the proposal has this shape, the evidence it
transcribes, how it reconciles prior authority, and what still needs ratification.
It is not a second source of normative grammar.

## What this revision is

Kirk approved the broad **authored playable scene** direction and authorized this
session to make architecture-consistent decisions without waiting for monster-AI PR
[#202](https://github.com/KirkDiggler/rpg-project/pull/202) to be amended. The result
is one broad documentation cut with four independently implementable tranches:

1. topology: canvas is workspace/bounds; regions may be the actual floor;
2. semantic prop composition learned from the real-model web experiment;
3. Phase-1 monster behavior configuration from the platform handoff; and
4. document/region environment lighting with provider-owned resolution.

This is a proposed contract baseline, not an implementation or delivery claim. The
tranches may ship separately. Strict rejection plus a capability check keeps partial
rollout honest; no `spec: "0.4"` wire marker pretends they arrive as one server flag.

At the time of this revision, v0.3 Wave 0 and Wave 1 both have LIVE VERIFIED evidence:
Wave 0 is recorded on [#192](https://github.com/KirkDiggler/rpg-project/issues/192#issuecomment-5206548237),
and Wave 1 on [#180](https://github.com/KirkDiggler/rpg-project/issues/180#issuecomment-5219362342).
Those delivered contracts remain the compatibility baseline while this proposal is
reviewed.

## The approved topology direction

Kirk's 2026-08-08 ruling remains the headline:

> “I think the floor is the region and we can see what is inside the walls. I think
> the canvas is available space in the builder but only regions carry into the game.”

The proposal turns that into a narrow, explicit topology choice:

```yaml
canvas:
  width: 20
  height: 30
  floor_source: regions
```

`canvas.floor_source` says what it does. `regions` means the deduplicated union of
`regions[].cells`; `bounds` (and omission) means the delivered v0.3 rectangle. Canvas
dimensions remain coordinate-legality and workspace bounds in both cases. This is
safer and clearer than the first PR #203 draft's overloaded document-level `spec:`
field.

### Why omission preserves `bounds`

Changing omission in place would silently reinterpret every persisted v0.3 canvas.
A document with no regions would shrink from a rectangle to empty without a decode
error. Omission therefore keeps the rectangle; a new authored masked floor says
`floor_source: regions` explicitly. A provider too old to know the nested field
strict-rejects it, which is the desired hard stop.

### The three axes are deliberately separate

- **v0.4** is this documentation/review level.
- **`version: 1`** remains the YAML document version.
- **`canvas.floor_source`** selects one canvas topology semantic.

None implies either of the others. There is no YAML `spec:` field and no server-side
`draft` value. This avoids confusing documentation cadence, decode compatibility,
and a single semantic choice.

## Why one canonical floor mask is load-bearing

The first PR #203 draft correctly noticed two existing floor helpers, but specifying
“remember to branch both” would perpetuate the underlying problem. A third path —
pathfinding, fog, targeting, or reload — could still forget the branch later.

The revised contract instead requires the toolkit to compile one sorted canonical
floor mask, persist it with compiled truth, and make every mechanical consumer query
that mask. Source regions still author the union; the compiled mask is the one
runtime truth. Fresh source compilation replaces it atomically. A source/persisted
mismatch fails rather than selecting the convenient interpretation.

That rule covers the seams most likely to drift:

- validate-only and non-validate compile;
- persistence and restart/reload;
- pathing, placement, start/party seating, range/target reachability;
- walls, LoS, reveal, and fog authorization; and
- authoring/runtime projection.

It also aligns with the prior walls-from-truth decision: once the provider publishes
canonical geometry, clients render it rather than recreating a parallel version.

## Envelope, voids, drafts, and edits

Every floor/void side becomes an implicit solid envelope, including an interior void.
The provider projects those edges explicitly; a client does not infer them from the
mask. An envelope door remains incoherent because its other side is void. Connecting
areas means authoring connecting floor cells; gating adjacent floor means an ordinary
interior door.

The revised draft separates authoring validity from runnable validity:

- `validate_only=true` accepts an empty region-union draft and returns an empty plan;
- a write/run/encounter compile rejects empty floor;
- content on non-floor still rejects normally in both cases.

That preserves the first-keystroke validation loop without persisting a dungeon that
cannot run.

Union-changing edits are candidate recompiles, not semantic-scope reassignment. If
removing floor or its envelope would orphan a placement, start, wall endpoint, party
envelope, or prop attachment, the edit rejects atomically. “Fall back to root” still
makes sense only for inheritance when the structural union is unchanged — for
example deleting a nested scope whose cells remain covered by its parent. It never
means turning removed floor into rectangle floor or moving content.

Omitted start on an irregular mask is deterministic: canonical floor order plus the
existing ordered PartyCap envelope, with a named failure if no candidate seats the
party. There is no random/seed-dependent rescue and no root fallback.

## Prop composition: what #728/#729 actually proved

Web issue [#728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728) and merged
PR [#729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729) ran a real-model
bookcase-to-ornate-torch experiment. Its evidence document is
[`prop-composition-728.md`](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/docs/evidence/prop-composition-728.md).
The useful result was ownership, not fixture field names:

| Fact | Learned owner | Replacement |
|---|---|---|
| GLB geometry/pivot, calibrated scale, attachment baseline, model-forward correction, footprint/light behavior | asset catalog/model | re-resolve for new ref |
| chosen canonical cell and wall/span relationship | authored placement | preserve |
| surface/support/orientation intent | authored placement | preserve or explicitly change as part of the semantic replacement |
| bounded anchor-local cosmetic adjustment | authored placement, validated against catalog bounds | preserve if valid; otherwise reject |
| raw matrix/world position, selection ID, temporary fixture slot | client/render state | never persist |

The failed first render was important: preserving the bookcase's raw transform put
the torch at the wall base, and treating the raw bookcase pivot as the cell center
put visible bookcases between hexes. The corrected fixture measured actual geometry
and used fixture-local asset corrections. Kirk visually accepted the corrected
result (“those look dead on center awesome”), but the issue explicitly parked an
Asset Anchor Lab before any correction becomes global.

### Why the proposed `anchor` encoding looks this way

The production encoding in `spec.md` does not copy the fixture's `slotId`, coordinate
names, limits, or numeric attachment offsets. It persists semantic facts:

- `at`: canonical owning cell;
- `surface`: floor or wall alignment surface;
- wall `edge` and `span: center`;
- `support`: floor-supported versus wall-supported;
- semantic orientation; and
- optional adjustment in the resolved anchor's local basis.

This represents both cases the old `mount` proposal blurred:

- wall-aligned floor bookcase = `surface: wall`, `support: floor`;
- wall-mounted torch = `surface: wall`, `support: wall`.

A one-action replacement may explicitly change support while preserving the cell,
edge/span, orientation, and committed adjustment. The asset catalog then re-resolves
all model-specific facts. An incompatible new asset rejects; it never clamps,
strips, or inherits the old matrix.

The v0.3 `facing` field remains a compatibility shorthand for an unanchored floor
prop. The proposed `anchor` replaces the still-unimplemented `mount: wall`/`height`
shape in [#188](https://github.com/KirkDiggler/rpg-project/issues/188) rather than
adding another overlapping transform vocabulary. `anchor.adjustment.vertical`
provides cosmetic local height without claiming raw world Y as source truth.

### Actual unresolved asset work

The architecture can be coherent without inventing catalog constants. What remains
real work is choosing the catalog owner/API, attachment-profile schema, provider
versioning, adjustment-bound policy, and a calibration gate across multiple assets,
wall orientations, and camera views. The fixture's measured bookcase correction,
torch height, and ± nudge bounds are evidence only. They are intentionally absent
from this spec.

## Phase-1 monster behavior: the platform handoff, reconciled

PR [#202](https://github.com/KirkDiggler/rpg-project/pull/202) supplies the approved
monster-AI design/plan context. The direct v0.4 handoff is
[issuecomment-5228001955](https://github.com/KirkDiggler/rpg-project/pull/202#issuecomment-5228001955).
The proposal carries its stable grammar and boundaries without freezing the
slice-3 candidates that the comment warns may change.

Settled here:

- flat optional `targeting`, `profile`, `params` on monster room `place`, canvas
  `place`, and room `boss` with full parity;
- fixed Phase-1 targeting values `closest`, `lowest-health`, `lowest-ac`;
- props reject all behavior fields;
- `defaults` is exact full monster-ref only — no prop/family/pattern matching;
- presence-aware order: constructor → default profile → default explicit params/
  targeting → placement profile → placement explicit params/targeting;
- explicit zero and `targeting: closest` override rather than disappear;
- profiles compile out; runtime persists only resolved targeting/config in
  `DataJSON`; and
- authoring never includes runtime mode, memory, perceptions, counters, target IDs,
  paths, or rationale.

The earlier design illustrates names such as `aggressive`, `timid`, and candidate
knobs. The platform comment explicitly says to freeze the shape, not that list. v0.4
therefore makes the toolkit registry authoritative for profile names and parameter
key/type/range validation. Example names in specimens are placeholders that an
implementation fixture substitutes with registered names.

The five-stage merge order is slightly more explicit than the comment's compressed
four-stage summary. It preserves the comment's intent while resolving the case where
a default profile, explicit default override, and placement profile all coexist.
Presence is checked at every stage.

## Environment: inheritance must resolve before rendering

The first PR #203 draft allowed document/region ambient lighting but left “renderer
baseline” as an implicit local choice. That would make reload and different clients
disagree. The revision puts resolution with the toolkit environment provider:

1. innermost region declaration wins;
2. otherwise walk outward to document/root;
3. if root is absent, use the named provider baseline;
4. persist provider identity/version plus the resolved snapshot.

A reload uses the compiled snapshot, not whatever a newer renderer or provider now
prefers. A deliberate recompile may adopt the new provider baseline atomically.
Authoring projection can show every resolved value; gameplay projection exposes the
resolved value only through fog-authorized cells/zone ancestry. A hidden lighting
override must not reveal a hidden region.

This keeps document lighting useful in room-chain mode, adds region inheritance in
canvas mode, and does not introduce per-source lights.

## Client-local versus platform contract

`wallLines`, draft/coverage masks, raw authoring controls, temporary IDs, raw
matrices/world coordinates, UI snap previews, and uncommitted nudges are client-local
compilation concerns. The platform accepts canonical walls, region cells, and
placements. It does not normatively depend on a particular web implementation or
TypeScript algorithm.

This corrects two problems in the first PR #203 draft:

1. it normatively cited a specific TypeScript wall geometry implementation; and
2. it described strip-before-wire as a compatibility technique.

The revised rule is lossless compile or hard stop. A local line tool may emit
canonical edges; a mask tool may emit canonical region cells; a committed nudge may
become semantic `anchor.adjustment`. If the local state cannot be represented
without loss, nothing is sent. Fractional coverage never becomes server pathing data.

## Scope and delivery matrix

| Concern | Authored by | Validated/resolved by | Persisted canonical truth | Runtime exposure |
|---|---|---|---|---|
| irregular structural floor | region cells + semantic floor source | toolkit topology provider | discriminator + canonical floor mask | fog-authorized cells |
| envelope/interior walls | derived envelope + authored `walls` | toolkit edge model | normalized canonical edges | explicit authorized per-hex edges |
| prop composition | cell + semantic anchor | toolkit + asset catalog | semantic source + resolved catalog facts (no raw matrix) | semantic placement + immutable catalog facts when authorized |
| monster behavior | exact-ref defaults + placement fields | toolkit behavior registry/compiler | source + resolved `DataJSON`; profile compiled out | ordinary monster authorization, no authoring/runtime-state leak |
| lighting | root/region declarations | toolkit environment provider | authored values + provider/version/resolved snapshot | resolved value only for authorized cells/zones |
| local authoring controls | web/editor | client compiler | never as raw controls | never |

Implementation dependencies are narrow:

- topology needs existing regions and canonical edges but no prop/behavior/lighting;
- prop anchors need canonical solid-edge queries and the catalog, not region-floor
  mode specifically;
- behavior needs the monster registry/DataJSON path and works in every topology;
- lighting needs the existing region containment graph for regional inheritance but
  document-level lighting works independently.

## Reconciliation with prior canonical documents

[`../../design.md`](../../design.md) and [`../../plan.md`](../../plan.md) are the
approved/delivered **v0.3 Wave 0/Wave 1** authority. Their statements that canvas
bounds create complete rectangle floor, regions do not change geometry, unpainted
root remains floor, and region edits cannot affect content were correct for that
delivery and are preserved as history.

They are not v0.4 implementation instructions. If this proposal is ratified,
`canvas.floor_source: regions` narrowly supersedes those topology statements for
that explicit branch only:

- omission/`bounds` still follows the delivered documents exactly;
- `regions` makes union changes structural and revalidates dependent content;
- semantic-only edits with unchanged union retain the prior inheritance/fallback
  model.

The old plan also says no lighting/geometry changes in #180. That remains true of
completed #180; Tranches A/D are new future work, not a retroactive broadening of the
closed wave. This scoping resolves the conflict without rewriting delivered evidence
or unrelated project state.

## Alternatives considered

### Keep document-level `spec: "0.4"`

Rejected. It overloads a documentation label as a broad wire semantic, implies all
tranches move together, and is easily confused with `version: 1`. A nested semantic
`canvas.floor_source` says exactly which behavior it controls and leaves other fields
additive/topology-neutral.

### Break omission in place

Rejected because it silently empties or shrinks persisted v0.3 canvases. Explicit
`regions` plus legacy omission produces a loud compatibility boundary.

### Let validate-only and write both accept empty floor

Rejected. Empty is useful during authoring but is not runnable content. Operation
intent already distinguishes those cases without inventing draft YAML.

### Re-derive floor/envelope in every subsystem or client

Rejected. Multiple “equivalent” helpers are the failure mode. Compile one mask and
one canonical edge set; persist/project them.

### Persist a complete prop transform

Rejected by #729's actual-model evidence. It transfers old-asset pivot/attachment
facts during replacement and couples source to renderer axes. Persist semantic
anchor intent; catalog-resolve the transform.

### Keep only `mount` + `height` + overloaded `facing`

Rejected for composition. It cannot cleanly distinguish a floor-supported bookcase
aligned to a wall from a wall-supported torch on the same span, and it overloads
orientation as edge selection. The nested anchor makes those facts explicit.

### Freeze `aggressive`/`timid` and candidate knobs in v0.4

Rejected per the platform handoff. The names were design candidates; the stable
contract is a toolkit-owned registry with loud name/key/type/range validation.

### Let the renderer choose absent ambient

Rejected because clients/reloads can diverge. Resolve and snapshot provider truth.

## Ratification record

The broad direction is approved; the document remains PROPOSED until these concrete
integration points are answered in the same PR:

1. envelope authoring-wire representation for one-sided floor/void edges;
2. asset catalog repository/API, versioning, attachment schema, and calibration gate;
3. per-asset/attachment adjustment-bound policy;
4. behavior registry discovery and compiled-provider provenance;
5. environment baseline provider and exact authoring/runtime projection fields; and
6. independent-tranche capability discovery used by the builder's hard stop.

These points do not block a coherent proposed architecture and therefore were not
escalated as product blockers. Ratification should confirm interfaces/evidence, not
reopen the approved authored-playable-scene direction.

## Acceptance specimens

The normative specimens live in [`spec.md` §13](spec.md#13-cross-layer-acceptance-specimens):

- irregular region-union floor with an interior void, explicit envelope, empty
  validate-only versus rejected runnable write, and dependent-edit rejection;
- wall-aligned floor bookcase → wall-mounted torch replacement, preserving semantic
  span/adjustment while catalog facts refresh; and
- exact-ref monster defaults plus placement profile/explicit overrides, proving
  zero/closest presence and room/canvas/boss parity;
- root/region lighting inheritance with absent-root provider baseline and fog-safe
  projection.

Each implementation tranche needs compile, durable persistence/restart, authoring
projection, real runtime projection, and adversarial rejection evidence for its
relevant specimen. Unit-only decode evidence is insufficient.

## Pointers

- [`../v0.3/spec.md`](../v0.3/spec.md) and
  [`../v0.3/README.md`](../v0.3/README.md) — ratified compatibility baseline.
- [`../../design.md`](../../design.md) and [`../../plan.md`](../../plan.md) — delivered
  v0.3 Wave 0/Wave 1 authority, scoped above rather than silently treated as v0.4.
- [rpg-project PR #203](https://github.com/KirkDiggler/rpg-project/pull/203) — this
  proposal's single review/ratification surface.
- [rpg-project PR #202](https://github.com/KirkDiggler/rpg-project/pull/202) and
  [platform handoff](https://github.com/KirkDiggler/rpg-project/pull/202#issuecomment-5228001955)
  — Phase-1 monster behavior authority used here.
- [rpg-dnd5e-web #728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728),
  [PR #729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729), and
  [evidence](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/docs/evidence/prop-composition-728.md)
  — real-model prop-composition learning.
- [rpg-project #188](https://github.com/KirkDiggler/rpg-project/issues/188),
  [#190](https://github.com/KirkDiggler/rpg-project/issues/190), and
  [#191](https://github.com/KirkDiggler/rpg-project/issues/191) — earlier narrow
  proposals reconciled into this broad cut.
