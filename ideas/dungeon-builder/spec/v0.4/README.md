# Dungeon YAML Spec v0.4 — context and rationale

[`spec.md`](spec.md) is the normative **PROPOSED** delta over ratified v0.3. This
README records evidence, decisions, prior-authority reconciliation, and the two
remaining ratification points. It does not add grammar.

## What changed in this revision

Kirk approved the broad **authored playable scene** direction and authorized this
session to decide coherent defaults. Independent review of the first broad draft
found six places where it still described an open interface as if it were an
independent tranche: capability discovery, draft/run lifecycle, persistence,
envelope projection, asset-catalog/server ownership, and lighting projection. This
revision closes those interfaces rather than leaving them as ratification questions.

The four additive tranches remain:

| Tranche | Source | Complete projection boundary |
|---|---|---|
| A — topology | `canvas.floor_source: regions` + region cells | resolved source, full authoring mask, existing pair edges, presence-aware entrance; snapshot-authorized runtime hex/edges |
| B — prop composition | authored ref/cell + semantic anchor/adjustment | source facts + `source_path` in authoring/runtime; web resolves released asset catalog |
| C — Phase-1 monster behavior | exact-ref defaults + flat placement/boss fields | typed scalar-union source/resolved behavior by source path; resolved `DataJSON` runtime |
| D — lighting | raw root/region ambient overrides | raw + optional effective authoring values; optional override on authorized runtime hexes |

Each tranche now has a stable builder capability key and a concrete
`AuthoringService.PutDungeon(validate_only)` probe. There is no new capabilities RPC.
A missing/incorrect projection makes the key unsupported, and the exact candidate is
always validate-only checked before save. Unlike the older builder experiment, v0.4
never strips an unsupported semantic.

v0.3 Wave 0 and Wave 1 are both LIVE VERIFIED:
[Wave 0](https://github.com/KirkDiggler/rpg-project/issues/192#issuecomment-5206548237)
and
[Wave 1](https://github.com/KirkDiggler/rpg-project/issues/180#issuecomment-5219362342).
This proposal is future work and claims no implementation.

## Authority and three axes

v0.4 is honestly a normative **delta** over
[`../v0.3/spec.md`](../v0.3/spec.md), not a false standalone restatement. Ratified
v0.3 governs every omitted construct.

Three axes stay independent:

1. **v0.4** — documentation/review cut;
2. **`version: 1`** — unchanged YAML decode version;
3. **`canvas.floor_source: bounds | regions`** — canvas topology only, omission
   resolving to legacy `bounds`.

There is no YAML `spec:`/`draft` marker. Behavior and lighting work in room chain,
bounds canvas, and region-floor canvas wherever their placement/scope exists.

## Topology: workspace bounds, region floor

Kirk's direction remains the headline:

> “I think the floor is the region and we can see what is inside the walls. I think
> the canvas is available space in the builder but only regions carry into the game.”

`floor_source: regions` compiles the deduplicated region-cell union. Canvas dimensions
remain workspace and legality bounds. Omission remains the v0.3 rectangle so old
content never silently shrinks.

### One compiled mask, two explicit lifecycles

Every mechanical path uses one compiled mask: placement, pathing, target
reachability, start/seating, walls, LoS, reveal/fog, initialization, and projection.
But authored content and a running encounter have intentionally different reload
rules:

- authored load/write recompiles YAML and atomically replaces registered content;
- encounter start snapshots the exact mask/edges/config it starts with;
- encounter reload validates snapshot consistency only and never compares against
  current authored YAML.

This means editing a dungeon changes future encounters without corrupting one already
in progress. It also removes the prior draft's contradictory source-versus-compiled
persistence authority.

### Envelope wire is closed, not open

The existing flat `FloorPlan.edges {from,to,kind}` pair represents an envelope without
a new message. Exactly one endpoint belongs to `floor_cells` and owns the side; the
other is the adjacent void coordinate, including negative/out-of-bounds coordinates.
Pair order is nonsemantic. Runtime attaches it only to the owning floor
`HexRecord`. A one-cell floor has six pairs; an interior one-cell void has six pairs
owned by its surrounding floor cells.

Clients render provider edges; they never infer envelope geometry from floor shape.
An authored wall still requires two floor endpoints. There is no envelope door into
void.

### Draft validity is not runnable validity

`validate_only` accepts empty, too-small, and disconnected structurally valid masks
and always projects their floor/envelope. `FloorPlan.entrance` presence is meaningful:
absent until an anchor has a complete PartyCap seating envelope within its own floor
component. With omitted start, qualifying anchors use deterministic canonical order.

Non-validate remains strict: floor nonempty, entrance/seats present, all seats in the
anchor component, and **all floor cells in that same component** for this cut. A
two-island draft may preview and even have a viable entrance on island A, but it does
not run while island B exists. That discriminates “can seat the party somewhere” from
“one runnable connected authored floor.”

Union-changing edits compile the complete candidate. Prior derived entrance/seats are
discarded and freshly recomputed; they are not dependencies and may change. A strict
write rejects only when the candidate is not runnable or explicitly authored start,
content, wall, or support becomes invalid — never because prior derived seats moved.
Pure semantic changes with unchanged union can still inherit outward.

### Region equality correction

Within one region, duplicate cells canonicalize. Between regions, equal sets reject.
That includes two empty regions: one empty region is legal/root-owned, but two empties
are equal sets and therefore invalid. This is the deterministic application of the
existing overlap model, not a new empty-region hierarchy exception.

## Capabilities: exact probes, no downgrade

The builder already has a real pattern: one minimally discriminating
`PutDungeon(validate_only)` request per feature because strict whole-document decode
cannot identify multiple unknown fields independently. v0.4 standardizes four stable
client keys:

- `authoring.floor_source.regions`;
- `authoring.prop.semantic_anchor`;
- `authoring.monster.behavior_config`;
- `authoring.environment.region_lighting`.

A key is true only if response success **and returned projection** match the key's
fixture. The server keeps the probe capability-gated until that release also has the
required runtime projection, so success attests a complete tranche rather than decode
alone. Probe failure is displayed raw and disables the surface. Before actual save,
the exact candidate document is validate-only checked; that is how dynamic behavior
profile/param values are tested without freezing a registry list in the capability
protocol.

This is additive and implementable through today's AuthoringService shape. It does
not invent a capability RPC, and it no longer allows the earlier `stripToV1Subset`
behavior to erase unsupported authored intent.

## Prop composition: retain the lesson, not the fixture grammar

Web issue [#728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728), PR
[#729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729), and its
[evidence](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/docs/evidence/prop-composition-728.md)
proved:

- authored placement owns canonical wall/cell/orientation intent and a bounded local
  adjustment UX;
- the asset catalog owns pivot, calibrated scale, attachment baseline, and
  model-forward correction;
- preserving a raw bookcase transform for a torch is wrong; and
- actual GLB bounds, not nominal roots, are the visual acceptance evidence.

The prior broad draft overfit that evidence into a server-owned asset resolver,
`span: center`, replacement semantics, and catalog provenance wire. All are removed.

### Minimal production intent

A canonical prop keeps:

- existing asset `ref` and canonical `at`;
- floor/wall `surface`;
- wall `edge` where applicable;
- floor versus wall `support`;
- semantic orientation; and
- optional finite anchor-local `along`/`normal`/`vertical` adjustment.

The bases are exact: floor normal=forward, floor along=right, wall normal follows
`into-cell`/`into-wall`, wall along is right looking along that normal, and vertical
is world up. The chosen cell+edge already identifies the wall span, so no `span` field
exists.

Wall support must be seed-invariant: structural room perimeter, region envelope, or
explicit authored solid edge. A random pattern wall can never support authored
content. `blocks_movement` and `blocks_los` remain the only mechanical prop facts;
catalog footprint/light is render/fit only.

### Catalog and replacement ownership

The synced `rpg-game-assets` catalog is consumed by the web. Builder and game parity
requires the same released catalog bundle. The server returns only authored ref,
semantic anchor/adjustment, absolute cell, and source path; it neither loads the
catalog nor projects a matrix/correction/profile.

Replacement is ordinary client-local whole-document editing. There is no server
replacement RPC or placement stable ID. A bookcase→torch editor action submits a new
complete document; the server validates that result independently. The specimen
explicitly changes blockers to `false/false` for the torch, proving render footprint
does not become game mechanics. No #729 numeric correction is promoted.

## Monster behavior: two independent chains

The authoritative handoff is monster-AI PR
[#202](https://github.com/KirkDiggler/rpg-project/pull/202) comment
[5228001955](https://github.com/KirkDiggler/rpg-project/pull/202#issuecomment-5228001955).
The stable source shape remains flat optional `targeting`, `profile`, and `params` on
monster room/canvas `place` and room `boss`, with props rejected and exact-ref
monster-only defaults.

The correction is that profiles contain MachineConfig params **only**, never
targeting. Resolution is two independent chains:

- targeting: constructor → exact-ref default explicit targeting → placement/boss
  explicit targeting;
- config per key: constructor → default profile params → default explicit params →
  placement/boss profile params → placement/boss explicit params.

YAML `params` values are JSON scalar bool/string/finite number. Null/list/map reject;
the toolkit registry owns per-key scalar kind, integer requirement, range/set, and
dynamic profile/knob names. The authoring proto uses repeated, unique-key
`BehaviorParam` entries with a bool/string/double oneof — never an untyped map. Numeric
producers require finite values; registry metadata adds integer/range checks.

Omitted params and `params: {}` both project no source entries and have no semantic
presence distinction. Real keys preserve the correct union arm, including zero,
false, and valid empty string. Profiles compile out. `FloorPlan.placements[]` returns
optional typed `source_behavior`, required typed `resolved_behavior`, and
`source_path`; resolved behavior contains targeting + unique sorted params, never
profile. Runtime `DataJSON` contains resolved targeting and MachineConfig only. Boss
has full parity. Runtime mode/memory/perception/counters/path/target state remains
excluded.

## Lighting: exact, render-only, fog-safe

Root and region `ambient` are absolute normalized renderer-intensity overrides in
`[0,1]`. Inheritance selects the nearest **authored** override walking innermost →
outward → root. If no scope authors one, effective ambient is absent and the existing
game renderer/theme baseline remains in force. No numeric “legacy baseline” is
invented; current surfaces legitimately differ (for example general versus crypt).
The toolkit resolves authored inheritance as rules provider; API passes through.

Lighting is render-only and cannot change LoS, visibility, reveal, fog authorization,
targeting, or pathing. Authoring FloorPlan returns raw root/region presence plus
optional effective override. Runtime adds optional resolved ambient only to authorized
visible/remembered `HexRecord`s whose chain has an authored override; hidden data stays
absent, and an authorized hex with no override also omits it. The probe requires both
present inherited `0.35` and absent unrelated values, so an older all-omitted wire
cannot falsely pass.

Authored reload recompiles raw YAML. Encounter snapshots retain each started hex's
optional override presence/value, matching the general lifecycle.

## Boundary matrix

| Concern | Canonical platform input/truth | Client-local |
|---|---|---|
| region floor | semantic discriminator + region cells; provider mask/edge projection | paint/draft mask UI |
| walls | authored `walls` + provider envelope pairs | `wallLines`, footprint/coverage compiler |
| prop | ref/cell/anchor/adjustment/blocker flags | replacement action, temp ID, raw matrix, catalog calibration application |
| behavior | refs + source config; toolkit resolved config | no behavior calculation |
| lighting | raw declarations; toolkit effective ambient | rendering only after authorization |

`wallLines`, coverage, raw draft controls, UI nudges before commit, matrices/world
coordinates, and temporary IDs never reach the platform. The platform must not cite a
particular TypeScript implementation. Local compilation is lossless or it hard-stops.

## Reconciliation with delivered v0.3 design/plan

[`../../design.md`](../../design.md) and [`../../plan.md`](../../plan.md) remain the
delivered v0.3 Wave 0/Wave 1 authority. Their rectangle floor, semantic-only region,
unpainted root, and nonmutating region-edit statements remain correct under omitted/
`floor_source: bounds`. Their “no lighting/geometry changes in #180” boundary remains
true because #180 is complete; this proposal is a later cut.

If ratified, explicit `floor_source: regions` narrowly changes those topology facts:
region union becomes floor, union edits revalidate content, and unpainted cells are
void. This README scopes the conflict rather than rewriting completed delivery
history or unrelated project state.

## Alternatives rejected

- **Document `spec: "0.4"`:** conflates documentation level with one semantic choice.
- **Silent break on omission:** would reinterpret persisted rectangles.
- **Compiled authoring cache as reload authority:** stale against edited YAML; authored
  load now recompiles, while encounters retain snapshots.
- **New envelope message or client derivation:** unnecessary; the existing pair plus
  floor membership identifies the owner exactly.
- **Empty/disconnected hard failure in validate-only:** breaks draft preview; runnable
  strictness belongs to non-validate.
- **Server asset resolver/catalog provenance:** not needed for canonical validation;
  web already consumes the asset bundle.
- **Server replacement semantics/stable placement ID:** unsupported by the whole-doc
  authoring API and not proven by #729.
- **Profiles carrying targeting:** couples two independent decisions and breaks the
  required targeting precedence.
- **One invented numeric fallback for absent lighting:** false across current themes;
  omission now preserves each renderer/theme baseline while authored overrides remain
  absolute and provider-projected.
- **Server `wallLines`/coverage:** duplicates canonical edge/floor truth.

## Ratification points

The broad direction and wire/lifecycle decisions above are made. Exactly two points
remain; this document is **not ratification-ready** until both are resolved:

1. **Asset calibration release gate:** the `rpg-game-assets` owner must define and
   evidence multi-asset/multi-wall calibration coverage, catalog-informed adjustment
   UX bounds, and the released bundle consumed identically by builder and game. No
   server catalog schema or fixture numeric constant is requested.
2. **Initial behavior registry contents:** the monster-behavior owner must ratify the
   first profile names and parameter keys/scalar kinds/integer/ranges. The v0.4
   container, merge, validation, persistence, error-path, and projection behavior is
   already fixed and vocabulary can grow without a YAML shape change.

## Acceptance pointers

Normative cross-layer specimens are in
[`spec.md` §9](spec.md#9-cross-layer-acceptance-specimens):

- irregular floor with an interior void and one-sided pair envelope;
- too-small draft plus a two-island discriminator separating entrance from runnable
  connectedness;
- whole-document wall-bookcase→wall-torch edit with authored blocker truth;
- independent targeting/config chains across room/canvas/boss with source paths; and
- ambient inheritance with remembered/hidden fog assertions and mechanical
  noninterference.

Each delivered tranche needs decode/compile, exact authoring projection, atomic
persistence/restart, real authorized runtime projection, probe discrimination, and
adversarial rejection/noninterference evidence. A decode-only unit test is not enough.

## Pointers

- [`../v0.3/spec.md`](../v0.3/spec.md) and
  [`../v0.3/README.md`](../v0.3/README.md) — ratified baseline.
- [`../../design.md`](../../design.md) and [`../../plan.md`](../../plan.md) — delivered
  v0.3 scope, reconciled above.
- [PR #203](https://github.com/KirkDiggler/rpg-project/pull/203) — single proposal and
  ratification surface.
- [PR #202 platform handoff](https://github.com/KirkDiggler/rpg-project/pull/202#issuecomment-5228001955)
  — behavior contract input.
- [Web #728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728),
  [PR #729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729), and
  [evidence](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/docs/evidence/prop-composition-728.md)
  — real-model composition learning and explicit calibration caveat.
