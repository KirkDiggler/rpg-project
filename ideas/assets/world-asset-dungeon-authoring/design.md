# World assets as dungeon-authorable scenery

**Status:** Approved; implementation unstarted
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
Builder. The Dungeon Builder still derives its scenery palette from the legacy
`PROP_KEYS` vocabulary, while its monster choices are separately hand-listed
over the current monster-model mapping. The Toolkit DungeonSpec compiler routes
only `props` and `monsters`, and the dungeon/game renderer resolves scenery
through the legacy `PropModel` path. A promoted asset can therefore exist in the
game build without becoming dungeon-authorable.

Monster and NPC appearances are arriving through their own provider paths as
well. Existing default dungeons already contain hard-coded monster refs, and
the first promoted NPC appearance families expose why the catalog must not be
designed as “props plus four visual categories forever.” Scenery and actors
have different runtime semantics, but they should enter one builder-facing
catalog through explicit adapters rather than accumulating independent UI
lists. Compositions are technical scenery props: assembled from other assets,
but authored and placed through one anchor hex.

That boundary was sensible before the World Builder and generated catalogs
existed. It is now unnecessary friction during pre-playtest development.

## Outcome

Every valid entry in the synchronized generated world-asset catalog appears
automatically in both builders. A Dungeon Builder author can find it by search,
visual category, or collection; place it using its exact visual reference; edit
the existing placement fields; compile it through the authoring service;
preview it; save it; and see the same model in play.

No second hand-maintained palette list or per-asset Web mapping is required.

## Design principles

1. **Promotion grants visual discoverability.** If an asset passed provider
   promotion and Web synchronization, it enters the appropriate builder catalog
   adapter without another hand-maintained UI list.
2. **The builder catalog is discriminated.** Scenery and actors share discovery
   and presentation infrastructure without being forced through one runtime
   behavior. Scenery sources include legacy assets, generated world assets, and
   compositions; actor kinds include monsters and NPCs.
3. **Visual category is not gameplay meaning.** `items` and `weapons` identify
   visual intent here. They do not automatically grant inventory, equip a
   character, or invoke a rulebook definition.
4. **Placement owns placement behavior.** Existing authored fields such as
   `blocks_movement` and `blocks_los` remain explicit facts in each DungeonSpec
   placement.
5. **Do not infer semantics.** Filenames, dimensions, tags, and visual category
   do not infer collision, line of sight, lighting, pickup behavior, or rules.
6. **Prefer openness before playtest.** A synchronized asset is not withheld
   without an observed technical or gameplay reason.
7. **Preserve exact refs.** The exact visual ref survives builder, YAML,
   compiler, atlas, preview, and game rendering unchanged.
8. **Fail closed visually.** An unsupported exact ref never substitutes a
   different asset.

## Scope

### Included

- Treat generated `props`, `items`, `weapons`, and `env` refs as scenery when
  used in DungeonSpec `place[]`.
- Feed the generated world-asset catalog into the Dungeon Builder palette.
- Add Dungeon Builder search, visual-category filters, collection grouping,
  result counts, and empty-state feedback so automatic discovery remains
  usable as the catalog grows.
- Preserve existing legacy props and monster authoring.
- Replace builder-local category lists with a discriminated shared catalog seam
  whose scenery side includes legacy assets, generated world assets, and
  single-anchor compositions, while its actor side preserves today's monsters
  and accepts future generated monster/NPC catalog adapters without another
  palette redesign.
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
- Choosing which promoted monster/NPC appearance maps to which rulebook actor,
  combat pool, AI policy, faction, or world-NPC definition.
- Making an unbound monster/NPC appearance behave as scenery or as a combatant.
- Changing the meaning of current `holdable` or `holds` fields.
- Inferring or automatically authoring lights.
- Multi-hex occupancy, rotated footprint masks, or per-cell collision.
- New placement defaults or changes to the existing movement/LoS controls.
- Runtime fetching of the private provider catalog.
- A broader World Builder palette redesign; its existing search remains, while
  this slice's shared facets make later organization additive.
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
Existing monster refs are gameplay identities, not merely model names.

The incoming `npcs` namespace is reserved for the actor side of the shared
catalog. This slice does not route NPCs through scenery or invent DungeonSpec NPC
semantics. An NPC catalog adapter may expose synchronized appearance metadata to
the common catalog surface, but authoring enablement waits for an explicit NPC
placement contract. This is an honest type distinction rather than a UI
hard-code: adding that contract will not require replacing the palette again.

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
  → neutral shared builderCatalog.ts
       ├─ one discriminated static catalog + exact-ref scenery index
       │    ├─ legacy scenery adapter (legacy renderer/default metadata)
       │    ├─ generated scenery adapter (props/items/weapons/env metadata)
       │    ├─ existing monster adapter (gameplay ref + appearance candidates)
       │    └─ NPC appearance-input adapter (explicitly non-placeable)
       ├─ dynamic composition adapter (technical scenery, one anchor hex)
       ├─ World Builder catalog adapter (preserves role/variant/support metadata)
       ├─ Dungeon Builder palette adapter
       └─ resolveBuilderScenery(ref)
            → AtlasPropModel source dispatch
            → legacy PropModel or generated WorldAssetModel
  → Dungeon place[].ref + authored placement fields
       → authoring API
       → Toolkit DungeonSpec scenery compile
       → atlas prop ref unchanged
       → Dungeon preview / game AtlasPropModel
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
hand. The World Builder already consumes generated entries without filtering by
`props`, `items`, `weapons`, or `env`; that all-category discovery remains an
explicit tested invariant. An asset appears only after Ready promotion, provider
merge, and `world-assets:sync`—a candidate in the local review queue is not yet
a published builder asset.

The neutral Web module `src/catalog/builderCatalog.ts` is the sole shared
builder-catalog authority. It owns one discriminated union, one static catalog,
and one exact-ref scenery index. Both builder adapters consume that catalog,
and `AtlasPropModel` consumes its `resolveBuilderScenery(ref)` resolver rather
than independently looking in the generated catalog or legacy manifest.

The union has two top-level authoring kinds:

- **scenery**, with source-specific variants for legacy assets, generated world
  assets, and compositions; and
- **actors**, with source-specific variants for monsters and NPCs.

Source payloads preserve the facts their consumers need instead of flattening
them into optional fields. A legacy scenery entry retains its `PropVariant`,
legacy role, and current placement defaults. A generated scenery entry retains
the complete `GeneratedWorldAsset`. A monster actor retains its gameplay ref,
model-backed ref ID, label, and current boss/presentation metadata. The NPC
adapter accepts appearance-catalog inputs and emits actor entries marked
`placeable: false` with an explicit deferred-placement reason; the production
input is empty in this slice, and a synthetic input test proves the seam without
inventing a gameplay ref.

A composition is therefore not a peer of scenery. It is a dynamic technical
scenery entry assembled from other catalog assets, represented by one
composition ref, and placed through one anchor hex. Its existing
expansion/render path remains specialized behind the scenery entry.

Each entry declares its authoring kind, source, placeability, and visual
category. Shared palette code can search, label, thumbnail, and select supported
kinds, while placement dispatch stays discriminated. A scenery entry never
acquires monster behavior because both appear in one palette, and the type does
not permit a non-placeable NPC entry to be armed.

The shared presentation model also exposes non-authoritative discovery facets:

- authoring kind: scenery or actor;
- visual category: props, items, weapons, environment, monsters, NPCs, or
  compositions;
- collection: the stable collection segment when present, such as
  `dark-fortress`, otherwise a disclosed legacy/default bucket;
- normalized search text built from display name, exact ref, explicit tags, and
  collection.

These facets organize the palette only. Parsing `items` from a ref can place an
entry under the Items filter; it cannot make the scenery grant an item.

The static catalog is assembled by explicit legacy, generated, monster, and
NPC-input adapters. Its scenery partition is the deterministic union of:

1. existing legacy prop entries; and
2. every generated world-asset entry not already represented by the same exact
   ref.

Deduplication happens before the scenery index is built, with legacy authority
on an exact-ref collision. `resolveBuilderScenery` reads that same index, so the
catalog shown to an author and the renderer's source choice cannot disagree.
The World Builder's local `catalog.ts` becomes a compatibility adapter over the
shared scenery entries: it preserves its existing ordering, legacy `role` and
`variant`, generated `asset`, thumbnail, and `supportsDecoration` fields, but it
no longer constructs a second legacy/generated union or collision policy.

The existing monster adapter preserves the currently authorable rulebook refs
and their model candidates, including monsters already used by default
dungeons. It derives entries from the authoritative monster-model keys while
retaining current labels and boss/presentation metadata. The NPC adapter has an
explicit `NpcAppearanceCatalogInput[]` seam. Its production input is empty, and
all adapted NPC entries are non-placeable until a future placement contract
binds an appearance to gameplay. Adding inputs later is an adapter/data change,
not another palette architecture change.

Legacy ordering and behavior remain stable in each builder adapter. Generated
entries retain provider order, display name, exact ref, visual category, tags,
provider metadata, and any available thumbnail. A generated asset without a
thumbnail uses the existing non-broken fallback presentation.

The catalog does not fabricate a legacy `PropRole` from a generated visual
category. The discriminated source payload gives legacy-only metadata only to
legacy entries and provider metadata only to generated entries.

Placement keeps the Dungeon Builder's current unknown-scenery fallback for
initial movement/LoS values. The existing Inspector checkboxes remain the
source of authored values. This slice does not decide new defaults.

Because the palette reads the generated catalog, the next successful
`world-assets:sync` adds new entries without another palette edit.

### Dungeon Builder findability

The Dungeon Builder palette adds one case-insensitive search field and filters
for All, Props, Items, Weapons, Environment, Monsters, NPCs, and Compositions.
Search matches display name, exact ref, explicit tags, and collection. Filters
and search compose, preserve deterministic catalog order, report the visible
result count, and show an explicit empty state rather than a blank panel.

All category filters remain visible with counts, including zero, so incoming
catalog types do not silently change the navigation model. Selecting a zero-count
filter shows the explicit empty state. Selecting or clearing a filter never
mutates the DungeonSpec. Existing placement interactions remain unchanged.

Generated collections group by the stable ref collection segment—for example,
`dnd5e:props:dark-fortress:altar_01` belongs to `dark-fortress`. The adapter
obtains that segment only through `parseRef(...).idParts`; no production or test
helper splits refs itself. Legacy refs without that segment remain in a labeled
legacy/default group. Collection parsing is presentation metadata only and does
not replace provider category or tags, and the ref-parser guard allowlist is not
expanded.

Before search/filtering, the Dungeon palette safely partitions current-world
composition records. A non-throwing composition-ref helper and the existing
safe metadata decoder admit only valid records to catalog counts, filtering, and
arming. Malformed IDs or metadata remain the original records, are never counted
or armed, and are passed unchanged to `CompositionThumbnailTiles` whenever All
or Compositions is active so its existing visible error route remains intact.
No throwing `compositionRef` call occurs in palette preprocessing.

The World Builder keeps its current search behavior in this slice. Its adapter
retains the metadata and ordering that search already consumes, while sourcing
entries from the shared catalog, so a later issue can add matching groups or
filters without changing generated provider data or reference semantics.

## Web rendering

`AtlasPropModel` becomes the common dispatcher for authored scenery:

1. composition refs continue through `CompositionPlacementModel`;
2. every other ref is passed to the shared `resolveBuilderScenery(ref)`;
3. a resolved legacy entry renders its retained `PropVariant` with `PropModel`;
4. a resolved generated entry renders its retained exact ref with
   `WorldAssetModel`;
5. a missing parser-valid exact scenery ref renders empty; and
6. existing non-exact legacy fallback behavior remains unchanged.

The exact-scenery decision is centralized in `src/utils/refs.ts`. It uses
`parseRef`, accepts only `props`, `items`, `weapons`, or `env`, and requires at
least two `idParts`. This makes missing exact refs in all four scenery namespaces
fail closed while preserving the family-placeholder behavior of one-part legacy
refs.

The generated path applies the same provider-authored normalization and runtime
scale already used by the World Builder. Dungeon cell position, authored
facing, and authored offset are layered around that model in the existing atlas
coordinate system. No generated asset is copied into `PropVariant`, because
that would duplicate metadata and risk applying the wrong normalization path.
Because dispatch switches on the shared resolved entry, a legacy/generated
exact-ref collision necessarily takes the legacy renderer as well as the legacy
catalog entry.

This dispatcher is used by the Dungeon Builder 3D preview and the game's
`DungeonEnvironment`, so preview and play cannot choose different visual
resolvers.

## Compatibility

- Existing DungeonSpec YAML remains valid and byte-equivalent when re-emitted.
- Existing `props` and `monsters` compile and render unchanged, including
  monster refs already present in default dungeons.
- The shared catalog refactor does not turn monsters or future NPCs into scenery;
  their adapters retain kind-specific placement dispatch.
- Existing compositions remain single-anchor scenery entries and retain their
  existing expansion, support-link, light, serialization, and rendering paths.
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
- Duplicate exact refs between legacy and generated sources are resolved once,
  in the shared catalog, with existing legacy authority. Catalog-adapter and
  `AtlasPropModel` dispatch tests make the same collision visible.
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

- Generated catalog entries from all four scenery categories appear in the
  Dungeon palette without a hand-written list.
- Dungeon search matches display name, exact ref, tags, and collection;
  category filters compose with search, counts are accurate, deterministic
  ordering is retained, and empty results are explained.
- Collection grouping uses `parseRef().idParts` without expanding the
  ref-parser guard allowlist and without becoming gameplay authority.
- Legacy palette ordering and entries remain stable.
- Existing monster choices are preserved and derived through the monster
  adapter; default-dungeon monster refs still parse, display, preview, and play.
- The common catalog adapts a synthetic NPC appearance input into an explicitly
  non-placeable actor without widening scenery behavior or requiring another
  palette component.
- Compositions appear as scenery-sourced technical props, remain one placement
  at the authoring boundary, and preserve their existing expansion behavior;
  malformed IDs/metadata stay visible but are never counted or armed after
  filtering.
- Placement, Inspector edits, YAML export/import, and server-error paths retain
  exact refs.
- `AtlasPropModel` dispatches the shared resolved entry, routes generated refs
  to `WorldAssetModel`, routes legacy refs (including exact-ref collisions) to
  `PropModel`, and renders four-category missing exact refs as empty output while
  retaining legacy family fallback.
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
3. **Web:** feed all generated categories to the Dungeon palette, add the
   shared discovery facets and Dungeon findability controls, and dispatch
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

- generated monster appearance catalogs bound explicitly to rulebook monster
  identities and appearance pools;
- generated NPC catalogs bound explicitly to world-NPC definitions, movement,
  interaction, and faction policy;
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
