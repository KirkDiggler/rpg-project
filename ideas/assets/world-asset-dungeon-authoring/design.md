# World assets as dungeon-authorable scenery

**Status:** Approved direction; detailed design awaiting human review  
**Issue:** [rpg-project#400](https://github.com/KirkDiggler/rpg-project/issues/400)  
**Date:** 2026-09-07

## Problem

The world-asset pipeline can now promote reviewed visual assets under four exact
reference namespaces:

- `dnd5e:props:*`
- `dnd5e:items:*`
- `dnd5e:weapons:*`
- `dnd5e:env:*`

After `world-assets:sync`, the generated catalog automatically feeds the World
Builder. The Dungeon Builder still derives its palette from the legacy
`PROP_KEYS` vocabulary, the Toolkit DungeonSpec compiler routes only `props`
and `monsters`, and the dungeon/game renderer resolves scenery through the
legacy `PropModel` path. A promoted asset can therefore exist in the game build
without becoming dungeon-authorable.

That boundary was sensible before the World Builder and generated catalog
existed. It is now unnecessary friction during pre-playtest development.

## Outcome

Every valid entry in the synchronized generated world-asset catalog appears
automatically in both builders. A Dungeon Builder author can place it using its
exact visual reference, edit the existing placement fields, compile it through
the authoring service, preview it, save it, and see the same model in play.

No second hand-maintained palette list or per-asset Web mapping is required.

## Design principles

1. **Promotion grants visual authorability.** If an asset passed provider
   promotion and Web synchronization, it is available to both builders.
2. **Visual category is not gameplay meaning.** `items` and `weapons` identify
   visual intent here. They do not automatically grant inventory, equip a
   character, or invoke a rulebook definition.
3. **Placement owns placement behavior.** Existing authored fields such as
   `blocks_movement` and `blocks_los` remain explicit facts in each DungeonSpec
   placement.
4. **Do not infer semantics.** Filenames, dimensions, tags, and visual category
   do not infer collision, line of sight, lighting, pickup behavior, or rules.
5. **Prefer openness before playtest.** A synchronized asset is not withheld
   without an observed technical or gameplay reason.
6. **Preserve exact refs.** The exact visual ref survives builder, YAML,
   compiler, atlas, preview, and game rendering unchanged.
7. **Fail closed visually.** An unsupported exact ref never substitutes a
   different asset.

## Scope

### Included

- Treat generated `props`, `items`, `weapons`, and `env` refs as scenery when
  used in DungeonSpec `place[]`.
- Feed the generated world-asset catalog into the Dungeon Builder palette.
- Preserve existing legacy props and monster authoring.
- Render generated dungeon placements through `WorldAssetModel` in preview and
  play.
- Preserve current position, facing, offset, identifier, holdable, composition,
  movement-blocking, and line-of-sight fields.
- Retain today's one-anchor-cell placement mechanics.
- Update Toolkit, API dependency integration, and Web with end-to-end proof.

### Not included

- Granting inventory items or rulebook features when scenery is picked up.
- Equipping a placed weapon.
- Mapping a visual ref to a rulebook item, weapon, or spell.
- Changing the meaning of current `holdable` or `holds` fields.
- Inferring or automatically authoring lights.
- Multi-hex occupancy, rotated footprint masks, or per-cell collision.
- New placement defaults or changes to the existing movement/LoS controls.
- Runtime fetching of the private provider catalog.
- Making arbitrary local files or unsynchronized refs authorable.

These are future designs informed by playtest evidence. In particular, a
visually large environment asset may overlap several hexes while affecting only
its authored anchor cell in this slice. That limitation is explicit, not a
claim that the asset occupies one hex physically.

## Reference semantics

DungeonSpec continues to use the existing `place[].ref` string. No parallel
`asset_ref` field or proto migration is introduced.

When a placement ref parses as one of the following types, the compiler treats
it as scenery:

```text
props | items | weapons | env
```

When it parses as `monsters`, existing monster behavior remains unchanged.
Other ref types remain unsupported by the DungeonSpec compiler.

For scenery, all current prop-like validation applies uniformly:

- `blocks_movement` is required;
- `blocks_los` is required;
- authored facing and offset are allowed;
- current identifier, holdable, holds, arrival, faction, and scenario rules
  remain as they are for scenery today;
- targeting and boss semantics remain monster-only.

The Toolkit does not resolve a visual reference to a GLB and does not maintain
a list of promoted assets. This matches its current treatment of arbitrary
well-formed `props` refs: Toolkit validates reference shape and placement
semantics while the Web client owns visual availability.

## Data flow

```text
Ready review entries
  → private Assets provider recipes + normalized GLBs
  → public, license-safe provider catalog
  → Web world-assets:sync
  → generated worldAssetCatalog.ts + ignored runtime GLBs
  → shared generated catalog adapters
       ├─ World Builder palette and WorldAssetModel
       └─ Dungeon Builder palette
            → place[].ref + authored placement fields
            → authoring API
            → Toolkit DungeonSpec scenery compile
            → atlas prop ref unchanged
            → Dungeon preview / game AtlasPropModel
            → WorldAssetModel
```

The private provider remains build-time input only. Production Web code reads
its checked-in generated metadata and locally synchronized ignored GLBs.

## Toolkit changes

The DungeonSpec compiler currently classifies refs as either `props` or
`monsters`. It will classify them as either **scenery** or **monster**:

- `props`, `items`, `weapons`, and `env` route to the existing scenery
  validation and compilation path;
- `monsters` route to the unchanged monster path;
- every other parsed type receives the existing unsupported-type error.

This is a routing expansion, not a new gameplay entity system. Compiled field
props continue carrying their exact ref and existing authored blocking facts.
The encounter field applies those facts to the anchor cell exactly as it does
for props today.

Tests pin all four scenery namespaces through decode, validation, compile, and
compiled output. They also pin that missing blocking declarations still fail,
monster-only fields still fail on scenery, and unsupported types remain
rejected.

## API integration

No protobuf field changes are expected. Authored and compiled placement refs
already travel as strings, and blocking fields already exist.

The API work is therefore a Toolkit dependency update plus authoring-service
acceptance coverage. The API must prove that a DungeonSpec containing each new
scenery namespace validates and returns an atlas with the exact refs and
blocking values preserved.

If repository inspection during implementation finds a hidden API allowlist,
that allowlist is expanded to the same four-type scenery set; the wire shape
still does not change. Discovery of a required proto change stops this design
for reconsideration rather than silently widening scope.

## Web catalog and palette

The generated `worldAssetCatalog.ts` remains generated and is never edited by
hand. A shared adapter exposes its entries to both builders.

The Dungeon Builder palette becomes the deterministic union of:

1. existing legacy prop entries; and
2. every generated world-asset entry not already represented by the same exact
   ref.

Legacy ordering and behavior remain stable. Generated entries retain provider
order, display name, exact ref, visual category, and any available thumbnail.
A generated asset without a thumbnail uses the existing non-broken fallback
presentation.

The palette model must not fabricate a legacy `PropRole` from a generated
visual category. If the current component needs a swatch role, its display model
is widened so legacy role and generated category are separate optional
presentation facts.

Placement keeps the Dungeon Builder's current unknown-scenery fallback for
initial movement/LoS values. The existing Inspector checkboxes remain the
source of authored values. This slice does not decide new defaults.

Because the palette reads the generated catalog, the next successful
`world-assets:sync` adds new entries without another palette edit.

## Web rendering

`AtlasPropModel` becomes the common dispatcher for authored scenery:

1. composition refs continue through `CompositionPlacementModel`;
2. generated exact world refs resolve through the generated catalog and render
   with `WorldAssetModel`;
3. legacy refs continue through `resolvePropVariant` and `PropModel`;
4. unsupported exact refs render empty rather than receiving a legacy
   placeholder or a different model;
5. existing non-exact legacy fallback behavior remains unchanged.

The generated path applies the same provider-authored normalization and runtime
scale already used by the World Builder. Dungeon cell position, authored
facing, and authored offset are layered around that model in the existing atlas
coordinate system. No generated asset is copied into `PropVariant`, because
that would duplicate metadata and risk applying the wrong normalization path.

This dispatcher is used by the Dungeon Builder 3D preview and the game's
`DungeonEnvironment`, so preview and play cannot choose different visual
resolvers.

## Compatibility

- Existing DungeonSpec YAML remains valid and byte-equivalent when re-emitted.
- Existing `props` and `monsters` compile and render unchanged.
- Existing exact Plushie and generated world refs retain their current resolver
  authority; no family alias is invented for new world assets.
- Existing compositions remain valid.
- Old Web clients encountering newly authored refs may render them empty but do
  not receive substituted art. Deployment ordering therefore updates the Web
  consumer before using new-category refs in shared dungeons.
- The authoring API must consume the Toolkit expansion before Web enables those
  categories in the promoted Dungeon Builder.

## Failure behavior

- `world-assets:sync` and `world-assets:check` remain the gate that generated
  metadata matches provider bytes.
- Duplicate exact refs between legacy and generated sources are resolved
  deterministically with existing legacy authority; tests make the collision
  visible.
- A malformed ref or unsupported ref type is rejected by Toolkit in the
  author's source path.
- A catalog-known ref whose local GLB is absent or mismatched fails sync/check;
  runtime loading still never substitutes another model.
- A well-formed scenery ref hand-authored outside the synchronized Web catalog
  may compile, as unknown `props` refs already can, but the Web renders an
  unsupported exact ref empty. The promoted palette never offers such a ref.
- Provider categories remain an explicit allowlist. A future category does not
  become DungeonSpec scenery until this design is deliberately extended.

## Verification

### Toolkit

- Table-driven validation and compile tests for `props`, `items`, `weapons`,
  and `env` refs.
- Exact ref, facing, offset, movement, and LoS preservation.
- Existing props/monster suites unchanged.
- Missing blocking facts and monster-only scenery fields rejected.
- Unsupported types rejected.

### API

- Authoring validate-only request containing all four scenery types.
- Atlas response preserves exact refs and blocking fields.
- No proto generation delta.

### Web

- Generated catalog entries from all four categories appear in the Dungeon
  palette without a hand-written list.
- Legacy palette ordering and entries remain stable.
- Placement, Inspector edits, YAML export/import, and server-error paths retain
  exact refs.
- `AtlasPropModel` routes generated refs to `WorldAssetModel`, legacy refs to
  `PropModel`, and unsupported exact refs to empty output.
- Dungeon preview and session renderer share the route.
- World Builder discovery remains unchanged.
- Production exclusion and licensed-file guards remain green.

### End to end

With synchronized provider assets and compatible Toolkit/API versions:

1. open the Dungeon Builder;
2. find a generated entry without editing a palette source file;
3. place it and edit movement/LoS;
4. confirm YAML carries the exact ref and authored values;
5. validate and save through the real authoring API;
6. confirm the preview and game render the same GLB;
7. reload and confirm the exact ref and placement behavior survive.

The first implementation can prove this with the current promoted props and
unit fixtures for the other categories. The first real promoted item, weapon,
and environment asset should each receive the same browser proof when they
arrive; their absence today does not justify a production restriction.

## Delivery sequence

1. **Toolkit:** expand DungeonSpec scenery routing and prove compatibility.
2. **API:** consume the Toolkit version and prove real authoring compilation.
3. **Web:** feed all generated categories to the Dungeon palette and dispatch
   generated atlas refs through `WorldAssetModel`.
4. **Integrated gate:** synchronize the merged provider, run repository CI,
   then exercise real authoring preview/save/play.

Each repository receives its own issue, worktree, PR, and owner-specific
review. Assets requires no provider-format change for this slice.

## Alternatives rejected

### Convert every visual to a `props` alias

This would make the current compiler accept them quickly, but duplicates
identity, destroys useful category information, and creates mappings that must
be maintained forever.

### Add `asset_ref` beside `ref` now

Separating visual and gameplay identities may become useful when pickup and
equipment bindings arrive. It is unnecessary for scenery-only authoring and
would force a wire migration before evidence requires one.

### Require gameplay mappings before palette discovery

This recreates the bottleneck the generated catalog removed. Visual
availability and authored collision are sufficient for pre-playtest scenery.
Gameplay bindings remain explicit future work.

### Infer collision or footprint from bounds

Bounds are useful evidence, not gameplay intent. Automatic inference would
create authoritative behavior from geometry without an author decision and
would make future multi-hex work harder to reason about.

## Future extensions

The design deliberately leaves clear seams for later work:

- optional per-placement or catalog-suggested rulebook bindings;
- pickup verbs that grant inventory items;
- weapon equipment interactions;
- containers and authored loot;
- explicit lighting metadata;
- provider-authored or builder-authored multi-hex footprint masks;
- orientation-aware occupied-cell computation;
- per-cell movement and LoS behavior.

Those features build on a stable visual-placement foundation instead of being
prerequisites for seeing and testing assets in the world.
