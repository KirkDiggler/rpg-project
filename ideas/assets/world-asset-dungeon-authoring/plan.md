# World Assets as Dungeon-Authorable Scenery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make every synchronized `props`, `items`, `weapons`, and `env` world asset automatically discoverable and placeable as DungeonSpec scenery while preserving monsters, future NPC seams, compositions, exact refs, and current one-anchor placement behavior.

**Architecture:** Toolkit expands its existing prop-like scenery route to four visual namespaces without changing the wire shape. API consumes the published Toolkit Encounter module and proves exact refs survive real authoring compilation. Web adds one neutral discriminated builder-catalog authority and scenery resolver consumed by the World Builder adapter, Dungeon palette, and `AtlasPropModel`; the resolver checks its legacy-first exact index before narrowly preserving existing mapped non-exact `props` family aliases. Source-specific legacy/generated metadata, monsters, an explicit non-placeable NPC-input seam, and dynamic compositions remain discriminated behind that shared surface.

**Tech Stack:** Go, `rpg-toolkit` DungeonSpec, `rpg-api` authoring/session projection, TypeScript, React, Three.js/R3F, Vitest, Testing Library, Playwright/Chromium.

**Spec:** `ideas/assets/world-asset-dungeon-authoring/design.md`

## Global Constraints

- Work in one issue worktree per repository; never implement in canonical roots. Each lane records its absolute worktree path, runs commands with that worktree as the explicit working directory, and hands other lanes only immutable commit/version receipts. Concurrent lanes never change branches, dependencies, or files in another lane's worktree.
- Toolkit branches from and merges to `main`; Toolkit CI publishes module tags after merge. Never create a tag manually.
- API and Web branch from and merge to `dev`.
- Use one writer per worktree and a fresh reviewer gate for every task-sized commit.
- Generated `src/generated/worldAssetCatalog.ts` is never hand-edited.
- Licensed GLBs remain ignored and never enter the public Web repository.
- Generated world categories are exactly `props`, `items`, `weapons`, and `env` in this slice.
- All four visual namespaces use existing scenery validation and one anchor cell.
- Existing `blocks_movement` and `blocks_los` fields remain required and author-controlled; do not infer them from category, filename, tags, or bounds.
- Preserve existing props, monsters, NPC adapter seam, compositions, positions, facing, offsets, support links, point lights, and exact-ref behavior.
- Compositions are technical scenery props assembled from other assets and represented by one anchor placement.
- A visual item or weapon does not grant inventory or equipment behavior.
- NPC and monster appearances do not acquire rules, AI, faction, or placement behavior from visual promotion.
- Parser-valid exact `props`, `items`, `weapons`, and `env` refs missing from the shared catalog render empty; never substitute a family/default or unrelated asset. After an exact-index miss, only a non-exact parsed `props` ref may use the existing `resolvePropVariant` family mapping. A mapped alias renders that legacy variant; an unknown one-part prop uses the neutral placeholder. `items`, `weapons`, and `env` never enter legacy family resolution.
- `src/utils/refs.ts` remains the only ref parser/colon-split site. Production and test code use `parseRef`/`idParts`; never expand `src/utils/refs.guard.test.ts`'s allowlist for this work.
- Shared scenery deduplication and rendering use the same resolver, with existing legacy authority on exact-ref collisions. The exact index is always consulted before the mapped-family branch.
- Multi-hex occupancy and inferred footprints remain out of scope.
- No protobuf shape change is expected. Discovery of a required proto change stops execution and returns to the director.
- Preserve the paused Assets weapon worktree `/home/kirk/.pi/worktrees/rpg-game-assets/115-final-weapon-remainder` unchanged.
- Every execution lane receives these exact environment values in its brief before commands run: `TOOLKIT_ENCOUNTER_VERSION`, `TOOLKIT_COMMIT`, `API_COMMIT`, `ASSETS_PROVIDER_ROOT`, `ASSETS_PROVIDER_COMMIT`, and `WEB_ISSUE_NUMBER`. The lane validates that commits are 40 lowercase hex characters, the Encounter version is a published semantic version, `ASSETS_PROVIDER_ROOT` is an absolute clean Git worktree at `ASSETS_PROVIDER_COMMIT`, and `WEB_ISSUE_NUMBER` is a positive integer.

## Repository delivery graph

```text
Task 1 Toolkit PR → merge/main → CI-published encounter module version
                         ↓
Task 2 API dependency/acceptance PR → merge/dev
                         ↓
Task 3 Web catalog model
   → Task 4 Dungeon palette findability
   → Task 5 dungeon preview/game renderer
                         ↓
Task 6 synchronized end-to-end gate → Web PR/dev
```

Before executing each repository lane, the coordinator creates a repository-owned implementation issue, adds it to Project 19, and creates the required issue worktree. The implementation issues link `rpg-project#400` and this plan. Assets needs no implementation issue unless a new provider batch is deliberately added to the acceptance proof.

---

### Task 1: Route all world-asset visual namespaces through Toolkit scenery

**Files:**
- Create: `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/world_asset_scenery_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/validate.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/compile.go`

**Interfaces:**
- Consumes: `core.ParseString(ref)` and the current DungeonSpec `PlaceSpec` fields.
- Produces: `isSceneryRefType(kind string) bool`; `refKind(ref)` accepting `props|items|weapons|env|monsters`; compiled `encounter.PropInput` entries retaining exact scenery refs.
- Task 2 consumes: the CI-published `rulebooks/dnd5e/encounter` module version from this merged PR. The parent `rulebooks/dnd5e` module is unchanged because `encounter/` is its own nested Go module.

- [ ] **Step 1: Create the failing namespace and scenery-validation tests**

Create `world_asset_scenery_test.go` with a compact dungeon helper whose final
placement fields are supplied by each case:

```go
package dungeonspec_test

import (
    "fmt"
    "testing"

    "github.com/stretchr/testify/require"

    "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec"
    "github.com/KirkDiggler/rpg-toolkit/tools/spatial"
)

func worldAssetSceneryYAML(ref, placementFields string) string {
    return fmt.Sprintf(`version: 2
key: world-assets
orientation: pointy
void: opaque
regions:
  - id: room
    archetype: crypt
    lighting: { intensity: 1 }
    cells:
      - [[0,0],[1,0]]
start: [0,0]
place:
  - { ref: %q, at: [1,0], %s }
`, ref, placementFields)
}

func TestWorldAssetNamespacesCompileAsScenery(t *testing.T) {
    refs := []string{
        "dnd5e:props:dark-fortress:altar_01",
        "dnd5e:items:dark-fortress:health_potion_01",
        "dnd5e:weapons:dark-fortress:sword_01",
        "dnd5e:env:dark-fortress:gate_01",
    }
    for _, ref := range refs {
        t.Run(ref, func(t *testing.T) {
            compiled, err := dungeonspec.Load([]byte(worldAssetSceneryYAML(
                ref,
                "blocks_movement: false, blocks_los: true, facing: ne, offset: [0.2,-0.1,0.3]",
            )))
            require.NoError(t, err)
            require.Len(t, compiled.Field.Props, 1)
            prop := compiled.Field.Props[0]
            require.Equal(t, ref, prop.Ref)
            require.Equal(t, spatial.Position{X: 1, Y: 0}, prop.At)
            require.False(t, *prop.BlocksMovement)
            require.True(t, *prop.BlocksLineOfSight)
            require.Equal(t, "ne", prop.Facing)
            require.Equal(t, [3]float64{0.2, -0.1, 0.3}, prop.Offset)
        })
    }
}

func TestNonPropsSceneryUsesPropValidation(t *testing.T) {
    tests := []struct {
        name, ref, fields, path, message string
    }{
        {
            name: "item missing blocks_movement",
            ref: "dnd5e:items:dark-fortress:health_potion_01",
            fields: "blocks_los: true",
            path: "place[0].blocks_movement",
            message: "there is no default",
        },
        {
            name: "weapon missing blocks_los",
            ref: "dnd5e:weapons:dark-fortress:sword_01",
            fields: "blocks_movement: false",
            path: "place[0].blocks_los",
            message: "there is no default",
        },
        {
            name: "environment with targeting",
            ref: "dnd5e:env:dark-fortress:gate_01",
            fields: "blocks_movement: true, blocks_los: true, targeting: lowest-health",
            path: "place[0].targeting",
            message: "not a monster",
        },
        {
            name: "item marked boss",
            ref: "dnd5e:items:dark-fortress:health_potion_01",
            fields: "blocks_movement: false, blocks_los: false, boss: true",
            path: "place[0].boss",
            message: "not a monster",
        },
    }

    for _, tc := range tests {
        t.Run(tc.name, func(t *testing.T) {
            defects := dungeonspec.Validate(mustDecodeWorldAsset(
                t,
                worldAssetSceneryYAML(tc.ref, tc.fields),
            ))
            require.Len(t, defects, 1)
            require.Equal(t, tc.path, defects[0].Path)
            require.Contains(t, defects[0].Message, tc.message)
        })
    }
}

func TestNonWorldAssetNamespaceStillCannotBePlaced(t *testing.T) {
    defects := dungeonspec.Validate(mustDecodeWorldAsset(
        t,
        worldAssetSceneryYAML(
            "dnd5e:spells:fireball",
            "blocks_movement: false, blocks_los: false",
        ),
    ))
    require.NotEmpty(t, defects)
    require.Equal(t, "place[0].ref", defects[0].Path)
    require.Contains(t, defects[0].Message, `type "spells"`)
}

func mustDecodeWorldAsset(t *testing.T, raw string) *dungeonspec.Spec {
    t.Helper()
    spec, err := dungeonspec.Decode([]byte(raw))
    require.NoError(t, err)
    return spec
}
```

The four negative cases deliberately use only `items`, `weapons`, and `env`.
They prove that the expanded routing reaches the complete existing scenery
validation body, including exact error paths, rather than merely compiling as a
prop-shaped output.

- [ ] **Step 2: Run the focused test and verify the current restriction**

Run:

```bash
cd rpg-toolkit/rulebooks/dnd5e/encounter
go test ./dungeonspec -run 'TestWorldAssetNamespaces|TestNonPropsScenery|TestNonWorldAssetNamespace' -count=1
```

Expected: `props` passes; `items`, `weapons`, and `env` fail with unsupported ref-type validation or produce no compiled prop.

- [ ] **Step 3: Add explicit scenery-type classification**

In `validate.go`, retain `typeMonsters` and define the complete supported visual set beside it:

```go
const (
    typeProps    = "props"
    typeItems    = "items"
    typeWeapons  = "weapons"
    typeEnv      = "env"
    typeMonsters = "monsters"
)

func isSceneryRefType(kind string) bool {
    switch kind {
    case typeProps, typeItems, typeWeapons, typeEnv:
        return true
    default:
        return false
    }
}
```

Update `refKind` so the parser remains authoritative for grammar while the compiler owns the allowlist:

```go
switch parsed.Type {
case typeProps, typeItems, typeWeapons, typeEnv, typeMonsters:
    return parsed.Type, nil
default:
    return "", fmt.Errorf(
        "ref %q names type %q, which this compiler cannot place",
        ref, parsed.Type,
    )
}
```

Change the `place[]` validation switch from one `case typeProps` to:

```go
case typeProps, typeItems, typeWeapons, typeEnv:
    // Keep the complete existing prop/scenery validation body unchanged.
```

Do not change monster branches or default their blocking fields.

- [ ] **Step 4: Compile every scenery type through the existing prop projection**

In `compile.go`, change only the filter in `propsOf`:

```go
for _, p := range spec.Place {
    kind, _ := refKind(p.Ref)
    if !isSceneryRefType(kind) {
        continue
    }
    // Keep the existing PropInput copy and authored offset logic unchanged.
}
```

Search every `refKind` call:

```bash
rg -n 'refKind\(' rulebooks/dnd5e/encounter/dungeonspec
```

For each call, preserve monster checks. Any check that specifically means “compiled scenery” must use `isSceneryRefType`; any check that means “not a monster” may remain a monster comparison. Do not classify `npcs` as scenery.

- [ ] **Step 5: Run focused and compatibility tests**

Run:

```bash
cd rpg-toolkit/rulebooks/dnd5e/encounter
go test ./dungeonspec -run 'TestWorldAssetNamespaces|TestNonPropsScenery|TestNonWorldAssetNamespace|TestCompileSuite|TestScenerySuite' -count=1
go test ./dungeonspec -count=1
go test . -count=1
```

Expected: all pass; existing monster and prop tests remain unchanged.

- [ ] **Step 6: Run the full affected Toolkit module gate**

Run:

```bash
cd rpg-toolkit/rulebooks/dnd5e/encounter
go test ./... -count=1
```

Expected: all packages pass. Run repository-required formatting/lint commands from `rpg-toolkit/AGENTS.md`, then verify:

```bash
git diff --check
git status --short
```

- [ ] **Step 7: Commit, review, publish, and capture the Encounter module version**

```bash
git add \
  rulebooks/dnd5e/encounter/dungeonspec/validate.go \
  rulebooks/dnd5e/encounter/dungeonspec/compile.go \
  rulebooks/dnd5e/encounter/dungeonspec/world_asset_scenery_test.go
git commit -m "feat: treat world asset refs as dungeon scenery"
```

Obtain an independent review. After the Toolkit PR merges to `main`, wait for normal CI module publication. Record the exact published `rulebooks/dnd5e/encounter` version in the Task 1 receipt; never tag manually.

---

### Task 2: Prove world-asset scenery through API authoring and session projection

**Files:**
- Create: `rpg-api/internal/dungeons/dungeonstest/worldassets.go`
- Create: `rpg-api/internal/handlers/dnd5e/authoring/v1alpha1/world_asset_scenery_wire_test.go`
- Create: `rpg-api/internal/integration/session/world_asset_scenery_acceptance_test.go`
- Modify: `rpg-api/go.mod`
- Modify: `rpg-api/go.sum`

**Interfaces:**
- Consumes: Task 1's published Toolkit Encounter module version; current `PutDungeon` and `GetAtlas` protobuf messages.
- Produces: a real API acceptance proving all four exact visual namespaces survive authoring and started-session projection without proto changes.
- Tasks 3–6 consume: API `dev` containing this compiler behavior.

- [ ] **Step 1: Add one shared four-category fixture**

Create `internal/dungeons/dungeonstest/worldassets.go`:

```go
package dungeonstest

const WorldAssetSceneryKey = "world-asset-scenery"

var WorldAssetSceneryRefs = []string{
    "dnd5e:props:dark-fortress:altar_01",
    "dnd5e:items:dark-fortress:health_potion_01",
    "dnd5e:weapons:dark-fortress:sword_01",
    "dnd5e:env:dark-fortress:gate_01",
}

const WorldAssetSceneryYAML = `version: 2
key: world-asset-scenery
name: World Asset Scenery
orientation: pointy
void: opaque
regions:
  - id: room
    archetype: crypt
    lighting: { intensity: 1 }
    cells:
      - [[0,0],[1,0],[2,0]]
      - [[0,1],[1,1],[2,1]]
start: [0,0]
place:
  - { ref: "dnd5e:props:dark-fortress:altar_01", at: [1,0], blocks_movement: true, blocks_los: false }
  - { ref: "dnd5e:items:dark-fortress:health_potion_01", at: [2,0], blocks_movement: false, blocks_los: false }
  - { ref: "dnd5e:weapons:dark-fortress:sword_01", at: [1,1], blocks_movement: false, blocks_los: false, facing: se }
  - { ref: "dnd5e:env:dark-fortress:gate_01", at: [2,1], blocks_movement: true, blocks_los: true, offset: [0.2,-0.1] }
`
```

- [ ] **Step 2: Add failing real-authoring wire coverage**

Create `world_asset_scenery_wire_test.go` using the same real registry/orchestrator/handler setup as `scenery_wire_test.go`. The test body must be:

```go
func (s *WorldAssetSceneryWireSuite) TestPutDungeonPreservesEveryWorldAssetRef() {
    resp, err := s.handler.PutDungeon(s.ctx, &authoringpb.PutDungeonRequest{
        Key: dungeonstest.WorldAssetSceneryKey,
        Yaml: dungeonstest.WorldAssetSceneryYAML,
        ValidateOnly: true,
    })
    s.Require().NoError(err)
    s.Require().Empty(resp.GetErrors(), "the four scenery namespaces must compile")
    s.Require().NotNil(resp.GetAtlas())

    refs := make([]string, 0, len(resp.GetAtlas().GetProps()))
    for _, prop := range resp.GetAtlas().GetProps() {
        refs = append(refs, prop.GetRef())
    }
    s.ElementsMatch(dungeonstest.WorldAssetSceneryRefs, refs)
}
```

Define `WorldAssetSceneryWireSuite`, `TestWorldAssetSceneryWireSuite`, and `SetupTest` by following the concrete `SceneryWireSuite` construction: authenticated context, `dungeonstest.Scratch`, real authoring orchestrator, and real handler. Do not mock the registry.

- [ ] **Step 3: Run against the old dependency and verify failure**

```bash
cd rpg-api
go test ./internal/handlers/dnd5e/authoring/v1alpha1 \
  -run 'TestWorldAssetSceneryWireSuite' -count=1
```

Expected: response contains validation errors for the three newly accepted namespaces.

- [ ] **Step 4: Consume the CI-published Toolkit Encounter version**

Use the exact version recorded by Task 1:

```bash
go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@"$TOOLKIT_ENCOUNTER_VERSION"
go mod tidy
make release-pin-check
```

Expected: `go.mod` contains published semantic versions, no pseudo-version, `replace`, or `go.work` dependency remains.

- [ ] **Step 5: Add started-session exact-ref acceptance**

Create `world_asset_scenery_acceptance_test.go` alongside `exact_ref_prop_acceptance_test.go`. Name the test `TestAcceptance_WorldAssetSceneryRefsReachStartedSession`. Reuse `newAcceptanceHarness`, create the fighter, compile `WorldAssetSceneryYAML`, start/join the session, call `GetAtlas`, and assert:

```go
refs := make([]string, 0, len(atlas.GetProps()))
for _, prop := range atlas.GetProps() {
    refs = append(refs, prop.GetRef())
}
require.ElementsMatch(t, dungeonstest.WorldAssetSceneryRefs, refs)
```

Also index the returned props by ref and assert the fixture's movement/LoS values remain exact for the potion and gate. This proves the fields survive beyond `PutDungeon`, not merely that parsing succeeds.

- [ ] **Step 6: Run API gates and prove there is no proto delta**

```bash
go test ./internal/handlers/dnd5e/authoring/v1alpha1 \
  -run 'TestWorldAssetSceneryWireSuite' -count=1
go test ./internal/integration/session \
  -run 'TestAcceptance_WorldAssetScenery' -count=1
make ci-check
git diff --check
git diff --name-only | rg '(^|/)dnd5e/api/|\.proto$' && exit 1 || true
```

Expected: tests and `make ci-check` pass; no proto file changed.

- [ ] **Step 7: Commit and review the API handoff**

```bash
git add go.mod go.sum \
  internal/dungeons/dungeonstest/worldassets.go \
  internal/handlers/dnd5e/authoring/v1alpha1/world_asset_scenery_wire_test.go \
  internal/integration/session/world_asset_scenery_acceptance_test.go
git commit -m "feat: compile world asset refs as dungeon scenery"
```

Obtain independent review, open the API PR against `dev`, and merge before enabling non-prop categories in the promoted Web Dungeon Builder.

---

### Task 3: Build the neutral shared Web catalog and scenery resolver

**Files:**
- Create: `rpg-dnd5e-web/src/catalog/builderCatalog.ts`
- Create: `rpg-dnd5e-web/src/catalog/builderCatalog.test.ts`
- Create: `rpg-dnd5e-web/src/concepts/world-building/catalog.test.ts`
- Modify: `rpg-dnd5e-web/src/concepts/world-building/catalog.ts`
- Modify: `rpg-dnd5e-web/src/utils/refs.ts`
- Modify: `rpg-dnd5e-web/src/utils/refs.test.ts`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/monsterModels.ts`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/monsterModels.test.ts`
- Modify: `rpg-dnd5e-web/src/author/paletteData.ts`
- Modify: `rpg-dnd5e-web/src/author/paletteData.test.ts`
- Read only: `rpg-dnd5e-web/src/generated/worldAssetCatalog.ts`

**Interfaces:**
- Consumes: `GENERATED_WORLD_ASSETS`, legacy `PROP_KEYS`/
  `resolvePropVariant`, authoritative monster model keys, and an explicit array
  of NPC appearance inputs.
- Produces:
  - from `src/catalog/builderCatalog.ts`, `BuilderCatalogEntry`, a real
    discriminated union;
  - `STATIC_BUILDER_CATALOG` and `STATIC_BUILDER_SCENERY_BY_REF`;
  - `resolveBuilderScenery(ref)` as the only non-composition scenery source
    resolver used by render dispatch, with exact-index-first resolution and a
    non-exact `props`-only mapped-family branch;
  - from `src/utils/refs.ts`, parser-based `isExactSceneryRef(ref)` for the four
    allowed scenery types;
  - `compositionBuilderEntry(...)` for dynamic current-world records;
  - `filterBuilderCatalog(entries, query, category)`;
  - `NpcAppearanceCatalogInput` and `npcAppearanceEntries(inputs)` producing
    explicitly non-placeable actors.
- Task 4 consumes the union, static catalog, composition adapter, and filter.
- Task 5 consumes `ResolvedBuilderScenery` and `resolveBuilderScenery`.
- The World Builder retains `WORLD_BUILDING_CATALOG` and
  `WORLD_BUILDING_CATALOG_BY_REF` as compatibility exports, but they become an
  adapter over the shared catalog rather than a second source union.

- [ ] **Step 1: Export authoritative authorable monster keys**

Add a failing test to `monsterModels.test.ts`:

```ts
import { AUTHORABLE_MONSTER_REF_IDS } from './monsterModels';

it('exports exactly the model-backed monster ref ids in deterministic order', () => {
  expect(AUTHORABLE_MONSTER_REF_IDS).toEqual([
    'skeleton',
    'skeleton-captain',
    'zombie',
  ]);
  for (const refId of AUTHORABLE_MONSTER_REF_IDS) {
    expect(resolveMonsterModelUrl(refId, undefined, false, 'catalog-proof')).toBeDefined();
  }
});
```

Run:

```bash
npm test -- --run src/components/hex-grid/monsterModels.test.ts
```

Expected: fail because the export does not exist. Then add beside
`MONSTER_REF_MODELS`:

```ts
export const AUTHORABLE_MONSTER_REF_IDS: readonly string[] = Object.freeze(
  Object.keys(MONSTER_REF_MODELS)
);
```

Do not export mutable candidate arrays and do not add unbound NPC appearances.

- [ ] **Step 2: Define exactness, the discriminated union, and adapter inputs with failing tests**

First add `isExactSceneryRef` to the existing imports and add coverage in
`src/utils/refs.test.ts`:

```ts
describe('isExactSceneryRef', () => {
  it.each([
    ['props', 'dnd5e:props:dark-fortress:missing'],
    ['items', 'dnd5e:items:dark-fortress:missing'],
    ['weapons', 'dnd5e:weapons:dark-fortress:missing'],
    ['env', 'dnd5e:env:dark-fortress:missing'],
  ])('recognizes exact %s refs', (_kind, ref) => {
    expect(isExactSceneryRef(ref)).toBe(true);
  });

  it('rejects a legacy family, another type, and malformed input', () => {
    expect(isExactSceneryRef('dnd5e:props:plushie')).toBe(false);
    expect(isExactSceneryRef('dnd5e:monsters:crypt:skeleton')).toBe(false);
    expect(isExactSceneryRef('dnd5e:items:')).toBe(false);
  });
});
```

Then create `src/catalog/builderCatalog.test.ts`. Import `parseRef` in fixture
helpers; no test helper may split a ref. The world-asset fixture uses:

```ts
const worldAsset = (
  ref: string,
  displayName: string,
  category: GeneratedWorldAsset['category'],
  tags: string[]
): GeneratedWorldAsset => {
  const parsed = parseRef(ref);
  if (!parsed) throw new Error(`invalid test ref: ${ref}`);
  return {
    ref,
    displayName,
    category,
    tags,
    url: `/models/${category}/${parsed.idParts.at(-1)}.glb`,
    glbSha256: 'a'.repeat(64),
    sizeBytes: 1,
    boundsMeters: [1, 1, 1],
    supportsDecoration: false,
  };
};
```

Test these public union members and literal discriminants:

```ts
export type BuilderVisualCategory =
  | 'props'
  | 'items'
  | 'weapons'
  | 'env'
  | 'monsters'
  | 'npcs'
  | 'compositions';

export type BuilderCatalogFilter = 'all' | BuilderVisualCategory;

export type BuilderCatalogEntry =
  | LegacyBuilderSceneryEntry
  | GeneratedBuilderSceneryEntry
  | CompositionBuilderSceneryEntry
  | MonsterBuilderActorEntry
  | NpcBuilderActorEntry;

export type StaticBuilderCatalogEntry = Exclude<
  BuilderCatalogEntry,
  CompositionBuilderSceneryEntry
>;

export type ResolvedBuilderScenery =
  | LegacyBuilderSceneryEntry
  | GeneratedBuilderSceneryEntry;
```

Every member shares `ref`, `label`, `short`, `authoringKind`, `source`,
`placeable`, `visualCategory`, `collection`, `tags`, optional `thumbnail`, and
`searchText`. Pin the source-specific payloads:

- legacy scenery: `authoringKind: 'scenery'`, `source: 'legacy'`,
  `placeable: true`, `visualCategory: 'props'`, plus `variant`, `role`,
  `blocksMovement`, and `blocksLoS`;
- generated scenery: `source: 'generated'`, `placeable: true`, and the complete
  `asset: GeneratedWorldAsset` without a `gameplayRef`;
- composition scenery: `source: 'composition'`, `placeable: true`, and no
  expanded children;
- monster actor: `source: 'monster'`, `actorKind: 'monster'`,
  `placeable: true`, plus `gameplayRef`, `modelRefId`, `sub`, and current
  `bossable` metadata;
- NPC actor: `source: 'npc'`, `actorKind: 'npc'`, `placeable: false`,
  `appearanceRef`, and `unavailableReason: 'npc-placement-deferred'`; it has no
  `gameplayRef`.

Run:

```bash
npm test -- --run \
  src/utils/refs.test.ts \
  src/catalog/builderCatalog.test.ts
```

Expected: fail because the central predicate and neutral catalog module do not
exist.

- [ ] **Step 3: Implement exactness, adapters, collision authority, filtering, and the resolver**

In `src/utils/refs.ts`, retain `isExactPropRef` for existing callers and add:

```ts
const SCENERY_REF_TYPES = new Set(['props', 'items', 'weapons', 'env']);

export function isExactSceneryRef(ref: string): boolean {
  const parsed = parseRef(ref);
  return (
    parsed !== null &&
    SCENERY_REF_TYPES.has(parsed.type) &&
    parsed.idParts.length >= 2
  );
}
```

This central predicate owns structural exactness; do not add a guard allowlist
entry. Then, in `builderCatalog.ts`, expose these exact construction seams:

```ts
export interface NpcAppearanceCatalogInput {
  appearanceRef: string;
  label: string;
  tags: readonly string[];
  thumbnail?: string;
  collection?: string;
}

export function generatedWorldAssetEntries(
  assets: readonly GeneratedWorldAsset[]
): GeneratedBuilderSceneryEntry[];

export function monsterBuilderEntries(
  refIds: readonly string[]
): MonsterBuilderActorEntry[];

export function npcAppearanceEntries(
  inputs: readonly NpcAppearanceCatalogInput[]
): NpcBuilderActorEntry[];

export function buildStaticBuilderCatalog(input: {
  legacy: readonly LegacyBuilderSceneryEntry[];
  generated: readonly GeneratedWorldAsset[];
  monsters: readonly MonsterBuilderActorEntry[];
  npcs: readonly NpcAppearanceCatalogInput[];
}): StaticBuilderCatalogEntry[];

export function createBuilderSceneryIndex(
  entries: readonly StaticBuilderCatalogEntry[]
): ReadonlyMap<string, ResolvedBuilderScenery>;

export function compositionBuilderEntry(input: {
  ref: string;
  label: string;
  thumbnail?: string;
}): CompositionBuilderSceneryEntry;

export function filterBuilderCatalog(
  entries: readonly BuilderCatalogEntry[],
  query: string,
  filter: BuilderCatalogFilter
): BuilderCatalogEntry[];
```

The production constants and resolver are:

```ts
export const NPC_APPEARANCE_CATALOG_INPUTS: readonly NpcAppearanceCatalogInput[] =
  Object.freeze([]);
export const STATIC_BUILDER_CATALOG: readonly StaticBuilderCatalogEntry[];
export const STATIC_BUILDER_SCENERY_BY_REF: ReadonlyMap<
  string,
  ResolvedBuilderScenery
>;
export function resolveBuilderScenery(
  ref: string
): ResolvedBuilderScenery | undefined;
```

Import `parseRef` and `isExactSceneryRef` from `@/utils/refs`, and retain the
existing `resolvePropVariant` import from the legacy manifest. Use one internal
`legacyBuilderSceneryEntry(ref, variant)` constructor for both indexed legacy
entries and mapped family results. The resolver implementation is exactly
ordered as follows:

```ts
export function resolveBuilderScenery(
  ref: string
): ResolvedBuilderScenery | undefined {
  const indexed = STATIC_BUILDER_SCENERY_BY_REF.get(ref);
  if (indexed) return indexed;

  const parsed = parseRef(ref);
  if (!parsed || parsed.type !== 'props' || isExactSceneryRef(ref)) {
    return undefined;
  }

  const variant = resolvePropVariant(ref);
  return variant ? legacyBuilderSceneryEntry(ref, variant) : undefined;
}
```

Implementation rules:

1. Adapt legacy entries from every `PROP_KEYS` key whose
   `resolvePropVariant(ref)` succeeds by calling
   `legacyBuilderSceneryEntry(ref, variant)`. Retain the resolved variant, role,
   and current Dungeon placement defaults. Preserve manifest order in this
   shared input; consumer-specific ordering remains an adapter concern. Family
   aliases remain absent from the static palette because they are resolved only
   on demand.
2. Adapt every `Object.values(GENERATED_WORLD_ASSETS)` entry. Extract a generated
   collection only with `parseRef(ref)?.idParts`: `idParts[0]` when there are at
   least two ID parts, otherwise the disclosed `legacy` bucket. Never use a
   colon split and never expand `src/utils/refs.guard.test.ts`'s allowlist.
3. Derive monster inclusion from `AUTHORABLE_MONSTER_REF_IDS`. Move the existing
   label/sub/boss presentation constants out of `paletteData.ts` into the
   neutral adapter; metadata may decorate an authoritative key but must not add
   a key absent from the model mapping.
4. `monsterBuilderEntries` admits only supplied authoritative model keys and
   retains current presentation metadata. `npcAppearanceEntries` maps
   `appearanceRef` to the entry `ref` and adapts every supplied appearance as a
   non-placeable NPC actor. The production NPC input array is intentionally
   empty; this is an executable input seam, not an `npcs` string in a type union
   only.
5. `buildStaticBuilderCatalog` appends legacy scenery, non-colliding generated
   scenery, monsters, and adapted NPCs in that order. Remove generated entries
   whose exact ref already appears in legacy before building the map.
6. `createBuilderSceneryIndex` accepts only legacy/generated scenery. On any
   accidental duplicate it retains the first entry, so the catalog's
   legacy-first collision rule is also the resolver rule. The production
   resolver checks this index before parsing for a family alias.
7. Only after an index miss, parse the ref. Return unresolved for malformed
   refs, any `items`/`weapons`/`env` type, and every exact scenery ref. For a
   non-exact parsed `props` ref only, call the existing `resolvePropVariant` and
   adapt a successful mapping as legacy scenery under the original authored
   family ref. Do not add aliases to the catalog or index and do not infer a
   family for generated world assets.
8. `filterBuilderCatalog` trims and lowercases the query, applies exact visual
   category unless `all`, matches precomputed `searchText`, excludes nothing on
   source alone, and preserves input order.

- [ ] **Step 4: Pin generated, collision, family-alias, actor, composition, and search behavior**

Add table-driven/pure tests that prove:

```ts
it('adapts all generated categories as scenery with parser-derived collections', () => {
  const entries = generatedWorldAssetEntries(generated);
  expect(entries.map(({ authoringKind, visualCategory, collection }) => ({
    authoringKind,
    visualCategory,
    collection,
  }))).toEqual([
    { authoringKind: 'scenery', visualCategory: 'props', collection: 'dark-fortress' },
    { authoringKind: 'scenery', visualCategory: 'items', collection: 'dark-fortress' },
    { authoringKind: 'scenery', visualCategory: 'weapons', collection: 'dark-fortress' },
    { authoringKind: 'scenery', visualCategory: 'env', collection: 'dark-fortress' },
  ]);
  expect(entries.every((entry) => !('gameplayRef' in entry))).toBe(true);
});

it('uses one legacy-first rule for both catalog and scenery resolver', () => {
  const catalog = buildStaticBuilderCatalog({
    legacy: [legacyEntry('dnd5e:props:dark-fortress:altar_01')],
    generated: [worldAsset(
      'dnd5e:props:dark-fortress:altar_01',
      'Generated Altar',
      'props',
      []
    )],
    monsters: [],
    npcs: [],
  });
  const index = createBuilderSceneryIndex(catalog);
  expect(catalog.filter((entry) => entry.ref.endsWith('altar_01'))).toEqual([
    expect.objectContaining({ source: 'legacy' }),
  ]);
  expect(index.get('dnd5e:props:dark-fortress:altar_01')).toMatchObject({
    source: 'legacy',
  });
});

it('preserves the existing mapped Plushie family alias after an index miss', () => {
  const entry = resolveBuilderScenery('dnd5e:props:plushie');
  expect(entry).toMatchObject({
    ref: 'dnd5e:props:plushie',
    source: 'legacy',
    variant: {
      displayName: 'Skele Dog Plushie',
      file: 'props/plushie--skeleton-dog.glb',
    },
  });
});

it('leaves an unknown one-part prop unresolved', () => {
  expect(resolveBuilderScenery('dnd5e:props:unknown-family')).toBeUndefined();
});

it.each([
  'dnd5e:items:plushie',
  'dnd5e:weapons:plushie',
  'dnd5e:env:plushie',
])('never applies legacy prop-family fallback to %s', (ref) => {
  expect(resolveBuilderScenery(ref)).toBeUndefined();
});

it('adapts a synthetic NPC appearance without making it placeable', () => {
  const [entry] = npcAppearanceEntries([{
    appearanceRef: 'dnd5e:npcs:townsfolk:apothecary_01',
    label: 'Apothecary',
    tags: ['townsfolk'],
  }]);
  expect(entry).toMatchObject({
    authoringKind: 'actor',
    source: 'npc',
    actorKind: 'npc',
    placeable: false,
    unavailableReason: 'npc-placement-deferred',
  });
  expect('gameplayRef' in entry).toBe(false);
});
```

Also test `compositionBuilderEntry` as one placeable scenery ref, monster entries
as placeable actors, no production NPC entries, and search over label, exact ref,
tags, and collection without reordering.

- [ ] **Step 5: Turn both existing catalog surfaces into adapters**

In `src/author/paletteData.ts`, retain thumbnail lookup, role colors, public
compatibility types, and `paletteNameForRef`, but derive `PALETTE_PROPS` and
`PALETTE_MONSTERS` from `STATIC_BUILDER_CATALOG`. Remove its independent
monster membership list. Pin unchanged legacy defaults, labels, bossability,
and model-backed monster refs in `paletteData.test.ts`.

In `src/concepts/world-building/catalog.ts`, export this pure adapter in addition
to its existing constants:

```ts
export function adaptWorldBuildingCatalog(
  entries: readonly StaticBuilderCatalogEntry[]
): WorldBuildingCatalogEntry[];
```

It filters to legacy/generated scenery and preserves the World Builder's current
contract exactly:

- legacy output retains `source`, `role`, `variant`, legacy label, thumbnail,
  and the existing `SUPPORT_REFS` decision;
- generated output retains `source`, `category`, the complete `asset`, provider
  `supportsDecoration`, display name, and provider order;
- current first-case/alphabetical legacy ordering remains, followed by generated
  order;
- actors are not silently admitted to the World Builder.

Build `WORLD_BUILDING_CATALOG` only by calling this adapter with
`STATIC_BUILDER_CATALOG`, then build `WORLD_BUILDING_CATALOG_BY_REF` from the
result. Delete the local legacy/generated union construction and local
collision set. In `catalog.test.ts`, inject a shared-catalog fixture and assert
legacy `role`/`variant`/support metadata and generated `asset` metadata survive,
actors are excluded, and a legacy exact-ref collision remains the only adapted
entry. Existing `WorldBuildingConcept` search/serialization imports stay
unchanged.

- [ ] **Step 6: Run catalog, parser-guard, and existing builder tests**

```bash
npm test -- --run \
  src/catalog/builderCatalog.test.ts \
  src/concepts/world-building/catalog.test.ts \
  src/author/paletteData.test.ts \
  src/components/hex-grid/monsterModels.test.ts \
  src/utils/refs.test.ts \
  src/concepts/world-building/WorldBuildingConcept.test.tsx \
  src/concepts/world-building/WorldBuildingViewport.test.tsx \
  src/utils/refs.guard.test.ts
npm run typecheck
```

Expected: all pass; the World Builder still contains all generated categories,
its metadata and ordering remain stable, the mapped Plushie family alias resolves
to its existing skeleton-dog variant, an unknown one-part prop remains
unresolved, non-props never use family fallback, and there is no colon-split
guard exception.

- [ ] **Step 7: Commit and review the shared authority**

```bash
git add \
  src/catalog/builderCatalog.ts \
  src/catalog/builderCatalog.test.ts \
  src/concepts/world-building/catalog.ts \
  src/concepts/world-building/catalog.test.ts \
  src/utils/refs.ts \
  src/utils/refs.test.ts \
  src/author/paletteData.ts \
  src/author/paletteData.test.ts \
  src/components/hex-grid/monsterModels.ts \
  src/components/hex-grid/monsterModels.test.ts
git commit -m "feat: share a discriminated builder catalog"
```

Review must explicitly confirm: this is one neutral catalog authority rather
than parallel World/Dungeon unions; the scenery resolver consults the
legacy-first exact index before preserving only mapped non-exact `props` family
aliases; the Plushie alias resolves to its existing skeleton-dog variant; an
unknown one-part prop stays unresolved and non-props never enter family
fallback; World Builder source metadata is preserved by its adapter; all four
generated categories are
included; monsters remain placeable actors; the synthetic NPC input becomes a
non-placeable actor; no ref split or guard exception was added; and no generated
file changed.

---

### Task 4: Add Dungeon palette search, categories, collections, counts, and empty states

**Files:**
- Create: `rpg-dnd5e-web/src/author/Palette.test.tsx`
- Modify: `rpg-dnd5e-web/src/author/Palette.tsx`
- Modify: `rpg-dnd5e-web/src/author/Palette.compositions.test.tsx`
- Modify: `rpg-dnd5e-web/src/author/DungeonBuilder.css`
- Modify: `rpg-dnd5e-web/src/author/DungeonBuilder.test.tsx`
- Modify: `rpg-dnd5e-web/src/compositions/compositionRef.ts`
- Modify: `rpg-dnd5e-web/src/compositions/compositionRef.test.ts`
- Modify: `rpg-dnd5e-web/src/author/CompositionThumbnailTiles.tsx`
- Modify: `rpg-dnd5e-web/src/author/CompositionThumbnailTiles.test.tsx`
- Read only: `rpg-dnd5e-web/src/author/types.ts` (retain
  `PaletteItem.kind: 'prop' | 'monster'`)

**Interfaces:**
- Consumes: Task 3 `STATIC_BUILDER_CATALOG`, `compositionBuilderEntry`,
  `filterBuilderCatalog`, `BuilderCatalogFilter`, and discriminated placeability.
- Produces: one searchable/filterable Dungeon Builder asset palette; existing
  `onArm(PaletteItem)` and `place` tool behavior remain compatible.
- Adds `tryCompositionRef(id): string | null` as the non-throwing authoring-list
  boundary. The strict `compositionRef(id)` remains available to callers that
  already hold a valid ID.
- Task 6 consumes: accessible labels/test IDs for browser proof.

- [ ] **Step 1: Write failing visible findability tests**

Create `Palette.test.tsx` with the current required props and a controlled
composition source. Mock only the dynamic composition hook or thumbnail
renderer, not the static catalog. Add tests with these assertions:

```tsx
it('offers stable all-category filters and counts', () => {
  renderPalette();
  for (const label of [
    'All', 'Props', 'Items', 'Weapons', 'Environment',
    'Monsters', 'NPCs', 'Compositions',
  ]) {
    expect(screen.getByRole('button', { name: new RegExp(`^${label} \\(`) }))
      .toBeVisible();
  }
  expect(screen.getByRole('button', { name: /^NPCs \(0\)$/ })).toBeVisible();
});

it('searches generated display names and exact refs', async () => {
  const user = userEvent.setup();
  renderPalette();
  await user.type(screen.getByRole('searchbox', { name: /find assets/i }), 'alchemy tools');
  expect(screen.getByRole('button', { name: /Alchemy Tools 01/i })).toBeVisible();
  expect(screen.queryByRole('button', { name: /Cage 03/i })).not.toBeInTheDocument();
});

it('composes category filters with search and explains an empty result', async () => {
  const user = userEvent.setup();
  renderPalette();
  await user.type(screen.getByRole('searchbox', { name: /find assets/i }), 'no such asset');
  await user.click(screen.getByRole('button', { name: /^Items \(/ }));
  expect(screen.getByText(/0 assets shown/i)).toBeVisible();
  expect(screen.getByText(/No Items match the current search and filter/i)).toBeVisible();
});
```

Add a valid composition fixture and prove searching/filtering Compositions
retains one armed composition ref.

- [ ] **Step 2: Add the failing malformed-composition filtering regression**

Extend `Palette.compositions.test.tsx` with the existing three-record shape: one
valid record, `table:with space`, and one syntactically valid ID with malformed
metadata. With the Compositions filter active and a query that excludes the
valid record, assert all of the following together:

- the filter label remains `Compositions (1)` and the visible-result text says
  `0 assets shown`; neither malformed record contributes to a count;
- both original malformed records still reach
  `CompositionThumbnailTiles` and render its existing
  `Unsupported composition …` visible error labels;
- no placement button exists for either malformed record and neither `onArm`
  nor `onTool` fires;
- selecting Items hides those error rows, then selecting Compositions restores
  them; presentation filtering never drops them permanently.

Run:

```bash
npm test -- --run \
  src/author/Palette.test.tsx \
  src/author/Palette.compositions.test.tsx
```

Expected: fail because search/category controls and safe pre-filter partitioning
do not exist.

- [ ] **Step 3: Add a non-throwing composition-ref boundary**

In `compositionRef.ts`, add:

```ts
export function tryCompositionRef(compositionId: string): string | null {
  try {
    return compositionRef(compositionId);
  } catch {
    return null;
  }
}
```

Pin valid and invalid IDs in `compositionRef.test.ts`. Change
`CompositionThumbnailTiles.tsx` to call `tryCompositionRef`; a `null` result
uses its existing unsupported-ID error entry. Retain `compositionMetadata`'s
existing result-based error route. Update the focused tile test to prove both
invalid-ID and malformed-metadata rows remain visible. No list/preprocessing
code introduced by this task directly calls throwing `compositionRef`.

- [ ] **Step 4: Safely partition dynamic compositions before filtering**

In `Palette.tsx`, add a pure local/exported helper with this result shape:

```ts
interface PaletteCompositionPartition {
  ready: ReadonlyArray<{
    composition: Composition;
    entry: CompositionBuilderSceneryEntry;
  }>;
  malformed: ReadonlySet<Composition>;
}

export function partitionPaletteCompositions(
  compositions: readonly Composition[]
): PaletteCompositionPartition;
```

For each record, call `tryCompositionRef(composition.id)` and
`compositionMetadata(composition)` before constructing a catalog entry. Add a
record to `ready` only when the ref is non-null and metadata status is `ready`;
otherwise put the original `Composition` object into `malformed`. The helper
must not throw, count, or synthesize a ref for a malformed record.

Use `partition.ready.map(({ entry }) => entry)` in the combined catalog. After
`filterBuilderCatalog`, make a set of visible valid composition refs. Derive the
records passed to `CompositionThumbnailTiles` by filtering the original
`compositionList.compositions` array in place-order:

- a ready record is included only when its entry ref is visible;
- a malformed record is included unchanged whenever the active filter is
  `all` or `compositions`, regardless of the search query, so the visible error
  route cannot be filtered away;
- every composition record is excluded for the other category filters.

Malformed rows are out-of-band errors: they are not entries in the combined
catalog, category counts, result counts, search matches, or arming logic.
Thumbnail production remains exclusively in `CompositionThumbnailTiles`.

- [ ] **Step 5: Implement palette state and accessible controls**

In `Palette.tsx`, add presentation-only state:

```ts
const [assetQuery, setAssetQuery] = useState('');
const [assetFilter, setAssetFilter] = useState<BuilderCatalogFilter>('all');
```

Render one search input:

```tsx
<input
  type="search"
  aria-label="Find assets"
  value={assetQuery}
  onChange={(event) => setAssetQuery(event.target.value)}
  placeholder="name, ref, tag, or collection"
/>
```

Render all fixed filters with counts computed from the unsearched combined
catalog of static entries plus valid compositions. Labels are accessible as
`Items (0)`, not conveyed only by color, and remain visible at zero. The visible
result count comes only from `filterBuilderCatalog`.

Respect union discrimination when arming:

```ts
if (entry.authoringKind === 'scenery' && entry.placeable) {
  onArm({ kind: 'prop', ref: entry.ref });
  onTool('place');
}
if (
  entry.authoringKind === 'actor' &&
  entry.actorKind === 'monster' &&
  entry.placeable
) {
  onArm({ kind: 'monster', ref: entry.gameplayRef });
  onTool('place');
}
```

Compositions keep their `{ kind: 'prop' }` arming inside
`CompositionThumbnailTiles`. A future synthetic NPC entry is disabled/non-armed
by its `placeable: false` discriminant; the production NPC count remains zero.

- [ ] **Step 6: Group visible entries without changing their order**

Create collection sections from the already-filtered, non-composition scenery
sequence with insertion-order maps:

```ts
const byCollection = new Map<string, BuilderCatalogEntry[]>();
for (const entry of visibleScenery) {
  const group = byCollection.get(entry.collection) ?? [];
  group.push(entry);
  byCollection.set(entry.collection, group);
}
```

Render labels such as `dark-fortress` and `legacy`. Do not alphabetically
reshuffle entries inside a collection. Static scenery buttons use
`entry.thumbnail ?? thumbForRef(entry.ref)` so existing legacy thumbnails and
the non-broken text fallback remain. Render monster actors in their existing
section and dynamic compositions through their existing tile component.
Category filters and search must not mutate the armed item or DungeonDoc.

Add focused `.dg-asset-search`, `.dg-asset-filters`, `.dg-asset-count`, and
`.dg-asset-empty` styles in `DungeonBuilder.css`; preserve the existing narrow
left rail and keyboard focus visibility.

- [ ] **Step 7: Pin placement defaults and document stability**

In `DungeonBuilder.test.tsx`, place a generated exact ref from the palette and
assert emitted YAML includes the same ref and the current unknown-scenery
fallback values:

```ts
expect(sourceText().textContent).toContain(
  'ref: "dnd5e:props:dark-fortress:alchemy_tools_01"'
);
expect(sourceText().textContent).toContain('blocks_movement: true');
expect(sourceText().textContent).toContain('blocks_los: false');
```

Then toggle both Inspector checkboxes and assert YAML follows the author. This
test freezes “existing fallback, author-controlled afterward” without declaring
the fallback a new semantic rule.

- [ ] **Step 8: Run UI, composition, parser-guard, and authoring regressions**

```bash
npm test -- --run \
  src/catalog/builderCatalog.test.ts \
  src/author/Palette.test.tsx \
  src/author/Palette.compositions.test.tsx \
  src/author/CompositionThumbnailTiles.test.tsx \
  src/author/DungeonBuilder.test.tsx \
  src/author/DungeonBuilder.compositions.test.tsx \
  src/author/paletteData.test.ts \
  src/compositions/compositionRef.test.ts \
  src/utils/refs.guard.test.ts
npm run typecheck
npm run lint -- --quiet
```

Expected: all pass; malformed composition records remain visibly safe under
filtering, and existing region/tools and composition behavior remain.

- [ ] **Step 9: Commit and review findability**

```bash
git add \
  src/author/Palette.tsx \
  src/author/Palette.test.tsx \
  src/author/Palette.compositions.test.tsx \
  src/author/DungeonBuilder.css \
  src/author/DungeonBuilder.test.tsx \
  src/compositions/compositionRef.ts \
  src/compositions/compositionRef.test.ts \
  src/author/CompositionThumbnailTiles.tsx \
  src/author/CompositionThumbnailTiles.test.tsx
git commit -m "feat: make dungeon assets searchable and filterable"
```

Review must exercise zero-count filters, combined search/filter, stable
collection order, composition filtering, the malformed-record visible error
route, monster preservation, non-placeable NPC discrimination, and no document
mutation from presentation controls. `src/author/types.ts` remains unchanged.

---

### Task 5: Render shared-catalog DungeonSpec scenery in preview and play

**Files:**
- Modify: `rpg-dnd5e-web/src/components/session/AtlasPropModel.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/AtlasPropModel.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/DungeonEnvironment.test.tsx`
- Read only: `rpg-dnd5e-web/src/catalog/builderCatalog.ts`
- Read only: `rpg-dnd5e-web/src/utils/refs.ts`
- Read only: `rpg-dnd5e-web/src/utils/refs.test.ts`
- Read only: `rpg-dnd5e-web/src/components/hex-grid/WorldAssetModel.tsx`
- Read only: `rpg-dnd5e-web/src/generated/worldAssetCatalog.ts`

**Interfaces:**
- Consumes: Task 3 `resolveBuilderScenery(ref)`,
  `ResolvedBuilderScenery`, and parser-based `isExactSceneryRef(ref)`;
  `WorldAssetModel`; existing `SceneProp3D` world position/facing/offset
  conversion.
- Produces: `AtlasPropModel` dispatch order `composition → shared resolved
  scenery source (exact index, then mapped non-exact props alias) → missing
  exact scenery empty → neutral placeholder`, shared by builder preview and
  gameplay.
- Task 6 consumes: real generated asset rendering in the Dungeon Builder and
  session route.

- [ ] **Step 1: Confirm the central exact-scenery contract from Task 3**

Run the parser tests before renderer work:

```bash
npm test -- --run src/utils/refs.test.ts
```

Expected: pass. `isExactSceneryRef` recognizes exact `props`, `items`,
`weapons`, and `env` refs, while `dnd5e:props:plushie` remains non-exact and
eligible for the shared resolver's mapped-family branch.

- [ ] **Step 2: Add failing shared-resolver dispatch and four missing-ref tests**

In `AtlasPropModel.test.tsx`, mock `WorldAssetModel` separately from the existing
composition and `PropModel` probes. Mock Task 3's resolver at its module boundary
with typed legacy/generated fixtures; do not mock or assert a direct generated
catalog lookup in `AtlasPropModel`.

Use table cases for valid fixture refs in `props`, `items`, `weapons`, and `env`.
For each generated resolved entry, assert the exact ref reaches
`WorldAssetModel` and no legacy model/placeholder mesh renders.

Add four missing-ref cases, one per exact scenery namespace:

```tsx
it.each([
  ['props', 'dnd5e:props:missing-collection:missing-model'],
  ['items', 'dnd5e:items:missing-collection:missing-model'],
  ['weapons', 'dnd5e:weapons:missing-collection:missing-model'],
  ['env', 'dnd5e:env:missing-collection:missing-model'],
])('renders empty for a missing exact %s ref', async (_kind, ref) => {
  mockResolveBuilderScenery.mockReturnValueOnce(undefined);
  const renderer = await renderAtlasProp({ ref });
  expect(renderer.scene.findAllByProps({ name: 'world-asset-model-probe' }))
    .toHaveLength(0);
  expect(renderer.scene.findAllByProps({ name: 'legacy-prop-model-probe' }))
    .toHaveLength(0);
  expect(meshes(renderer)).toHaveLength(0);
});
```

Add these separate authority regressions:

1. composition refs still bypass the shared static resolver;
2. a normal resolved legacy ref reaches `PropModel`;
3. the Task 3 resolver test proves `dnd5e:props:plushie` resolves to a legacy
   entry carrying `props/plushie--skeleton-dog.glb`; the Atlas test returns that
   resolved entry, renders the existing skeleton-dog Plushie `variant` through
   `PropModel`, and renders neither `WorldAssetModel` nor the placeholder;
4. `dnd5e:props:unknown-family` remains unresolved and renders exactly the
   neutral placeholder, with neither model probe;
5. for a synthetic exact ref represented by both legacy and generated test
   inputs, Task 3's index resolves the legacy entry and `AtlasPropModel` renders
   the legacy `variant`, never `WorldAssetModel`.

The collision test may use `createBuilderSceneryIndex` to establish the
legacy-selected fixture and have the resolver mock return that exact entry. It
must assert renderer choice, not only index contents. Together with Task 3's
resolver tests, these cases explicitly distinguish a mapped non-exact `props`
alias, an unknown one-part prop, and an exact collision.

- [ ] **Step 3: Dispatch only from the shared resolved scenery entry**

In `AtlasPropModel.tsx`, delete imports of `GENERATED_WORLD_ASSETS`,
`resolveWorldAsset`, and `resolvePropVariant` if present. Import only:

```ts
import { resolveBuilderScenery } from '@/catalog/builderCatalog';
import { isExactSceneryRef } from '@/utils/refs';
import { WorldAssetModel } from '../hex-grid/WorldAssetModel';
```

Keep composition detection first. Then resolve exactly once and switch on its
discriminant:

```tsx
const scenery = resolveBuilderScenery(prop.ref);
if (scenery?.source === 'generated') {
  return (
    <Suspense fallback={null}>
      <ErrorBoundary fallback={null}>
        <WorldAssetModel
          assetRef={scenery.ref}
          position={[world.x, world.y, world.z]}
          rotationY={facingToYaw(prop.facing)}
        />
      </ErrorBoundary>
    </Suspense>
  );
}
if (scenery?.source === 'legacy') {
  return (
    <Suspense fallback={placeholder}>
      <ErrorBoundary fallback={placeholder}>
        <PropModel
          variant={scenery.variant}
          position={[world.x, world.y, world.z]}
          rotationY={facingToYaw(prop.facing)}
        />
      </ErrorBoundary>
    </Suspense>
  );
}
if (isExactSceneryRef(prop.ref)) return null;
return placeholder;
```

Construct `placeholder` before this switch, but only return it for legacy load
failures or unresolved non-exact refs. A mapped legacy family never reaches this
final branch because `resolveBuilderScenery` returns its legacy entry first.
`propWorldPosition` remains the
sole position/offset conversion. `WorldAssetModel` remains the sole provider
normalization/runtime-scale renderer. Do not copy generated entries into
`PropVariant` and do not perform any parallel generated/legacy source lookup in
this component. The shared resolver's legacy-first map is therefore renderer
authority on exact collisions.

- [ ] **Step 4: Prove DungeonEnvironment shares the dispatch**

Extend `DungeonEnvironment.test.tsx` with an atlas prop using the promoted
Alchemy Tools exact ref. Mock `AtlasPropModel` and assert the ref, authored
facing, and offset are passed unchanged. This pins that builder preview and
session rendering both reach the same dispatcher rather than adding a
builder-only renderer.

- [ ] **Step 5: Run renderer, parser-guard, and scene regressions**

```bash
npm test -- --run \
  src/utils/refs.test.ts \
  src/utils/refs.guard.test.ts \
  src/catalog/builderCatalog.test.ts \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx \
  src/components/hex-grid/WorldAssetModel.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/author/preview3d/DungeonPreview3D.render.test.tsx \
  src/components/session/SessionCanvas.test.tsx
npm run typecheck
```

Expected: all pass; compositions, lights, monsters, legacy props, the mapped
Plushie family alias, the unknown-family placeholder, four-category missing exact
refs, and exact collision authority retain their specified behavior.

- [ ] **Step 6: Commit and review renderer authority**

```bash
git add \
  src/components/session/AtlasPropModel.tsx \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx
git commit -m "feat: render generated scenery in authored dungeons"
```

Review must confirm: composition first; one shared scenery resolution; the
mapped `dnd5e:props:plushie` alias renders its existing skeleton-dog variant; an
unknown one-part prop renders the neutral placeholder; exact refs and
`items`/`weapons`/`env` never use prop-family fallback; legacy renderer authority
holds on an exact generated collision; all four missing exact namespaces render
empty; no double scale/grounding; offsets apply once; and no ref split or
generated-file edit was introduced.

---

### Task 6: Synchronize, verify, document, and publish the Web integration

**Files:**
- Modify if sync output changes: `rpg-dnd5e-web/src/generated/worldAssetCatalog.ts`
- Modify if sync output changes: `rpg-dnd5e-web/public/models/synty/world-assets/**` (ignored; never commit)
- Modify: `rpg-dnd5e-web/src/author/CONTRACT.md`
- Modify: `rpg-dnd5e-web/src/concepts/world-building/CONTRACT.md`
- Create: `rpg-dnd5e-web/docs/evidence/${WEB_ISSUE_NUMBER}-world-asset-dungeon-authoring/README.md`
- Create: `rpg-dnd5e-web/docs/evidence/${WEB_ISSUE_NUMBER}-world-asset-dungeon-authoring/receipt.json`
- Create: local ignored browser script under `game-dev/tools/browser/_job_world_asset_dungeon_authoring.mjs`

**Interfaces:**
- Consumes: merged API `dev`; a clean merged Assets provider checkout; Tasks 3–5 Web commits.
- Produces: current-head CI, provider, browser, compatibility, and review evidence for the Web PR.

- [ ] **Step 1: Rebase/merge the current Web `origin/dev` before final gates**

```bash
git fetch origin --prune
git merge --no-ff origin/dev
```

Resolve only additive conflicts. Preserve concurrent Dungeon/World Builder changes. If a conflict changes catalog authority, placement semantics, or render ownership, stop and return to the director.

- [ ] **Step 2: Synchronize against a clean merged provider**

```bash
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_ROOT" \
  npm run world-assets:sync
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_ROOT" \
  npm run world-assets:check
```

Expected: exact provider commit and recipe hashes are printed; all generated entries and ignored GLB bytes match. Verify no licensed file is tracked:

```bash
git status --short
test -z "$(git ls-files public/models/synty/world-assets)"
```

If the repository uses a different tracked-metadata location after concurrent changes, follow the merged `world-assets:sync` command's output; never hand-edit generated metadata.

- [ ] **Step 3: Run the full focused cross-feature suite**

```bash
npm test -- --run \
  src/catalog/builderCatalog.test.ts \
  src/author/Palette.test.tsx \
  src/author/Palette.compositions.test.tsx \
  src/author/paletteData.test.ts \
  src/author/DungeonBuilder.test.tsx \
  src/author/DungeonBuilder.compositions.test.tsx \
  src/author/CompositionThumbnailTiles.test.tsx \
  src/author/dungeonYaml.test.ts \
  src/compositions/compositionRef.test.ts \
  src/utils/refs.test.ts \
  src/utils/refs.guard.test.ts \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx \
  src/components/hex-grid/WorldAssetModel.test.tsx \
  src/concepts/world-building/catalog.test.ts \
  src/concepts/world-building/WorldBuildingConcept.test.tsx \
  src/concepts/world-building/WorldBuildingViewport.test.tsx \
  src/concepts/world-building/serialization.test.ts \
  src/App.test.tsx \
  src/ApplicationRoot.test.tsx
```

Expected: all pass with no unexpected skip.

- [ ] **Step 4: Run repository CI and production/license guards**

```bash
npm run ci-check
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_ROOT" \
  npm run world-assets:check
git diff --check
git status --short
```

Expected: format, lint, typecheck, build, production exclusion, licensed-file guard, and full tests pass.

- [ ] **Step 5: Exercise real Dungeon Builder authoring in Chromium**

Start the API version containing Task 2 and the exact Web worktree. The ignored browser script must:

1. open the promoted Dungeon Builder route;
2. assert filter buttons for all eight categories are visible, including zero counts;
3. search `Alchemy Tools 01` and place the exact promoted ref;
4. change both blocking checkboxes and verify source YAML updates;
5. save through real `PutDungeon`, wait for a compiled preview, and assert no field errors;
6. assert the exact world-asset GLB returns HTTP 200;
7. assert the 3D preview renders the generated model;
8. reload the dungeon and verify exact ref and authored blocking values survive;
9. start/play the dungeon through a seeded local character, then verify the session atlas retains the exact ref; inability to seed/start is a blocked acceptance gate, not a skipped assertion;
10. clear search, select Monsters, and prove an existing default-dungeon monster remains available;
11. select Compositions and prove one current-world composition remains one armed/placed ref;
12. with a malformed current-world composition fixture, prove its visible error row remains while it is absent from counts and cannot be armed;
13. record console errors, page errors, failed requests, HTTP errors, and WebGL errors as empty arrays.

Use role/name locators instead of CSS internals. Save screenshots under `/tmp/web-${WEB_ISSUE_NUMBER}-world-asset-dungeon-authoring/`, record their SHA-256 values in the evidence README, and commit only the textual README/JSON receipt. Do not commit licensed model pixels to the public Web repository.

- [ ] **Step 6: Document the contract and current limitations**

Update `src/author/CONTRACT.md` with:

```text
Generated props/items/weapons/env are DungeonSpec scenery. Their exact visual
ref is authored; blocking remains per placement. One neutral catalog and
legacy-first scenery resolver serve both builders and AtlasPropModel. It checks
the exact index first, then preserves mapped non-exact props aliases such as
dnd5e:props:plushie through the existing skeleton-dog Plushie variant. Missing
exact scenery refs render empty; an unknown one-part prop uses the neutral
placeholder; items/weapons/env never use prop-family fallback. Visual category
grants no inventory/equipment meaning. Every placement affects one
anchor cell in this slice. Compositions are assembled technical scenery props
represented by one anchor ref; malformed records stay visible but cannot arm.
Monsters remain actors; NPC appearance inputs remain explicitly non-placeable.
```

Update the World Builder contract to state that generated all-category discovery remains unchanged and its shared facets are non-authoritative presentation metadata.

The evidence README names exact Toolkit/API/Web/provider revisions, commands, browser route, counts, and limitations. Generate `receipt.json` from validated environment and command output rather than typing revision placeholders:

```bash
export WEB_REVIEWED_HEAD="$(git rev-parse HEAD)"
export PROVIDER_ASSET_COUNT="$(npx tsx -e \"import { GENERATED_WORLD_ASSETS } from './src/generated/worldAssetCatalog.ts'; console.log(Object.keys(GENERATED_WORLD_ASSETS).length)\")"
export WEB_EVIDENCE_DIR="docs/evidence/${WEB_ISSUE_NUMBER}-world-asset-dungeon-authoring"
mkdir -p "$WEB_EVIDENCE_DIR"
jq -n \
  --arg toolkitCommit "$TOOLKIT_COMMIT" \
  --arg encounterVersion "$TOOLKIT_ENCOUNTER_VERSION" \
  --arg apiCommit "$API_COMMIT" \
  --arg webHead "$WEB_REVIEWED_HEAD" \
  --arg providerCommit "$ASSETS_PROVIDER_COMMIT" \
  --argjson assetCount "$PROVIDER_ASSET_COUNT" \
  '{
    toolkit: {commit: $toolkitCommit, encounterVersion: $encounterVersion},
    api: {commit: $apiCommit},
    web: {reviewedHead: $webHead},
    provider: {commit: $providerCommit, assetCount: $assetCount},
    checks: {focused: "pass", ci: "pass", provider: "pass", browser: "pass"},
    limitations: ["single-anchor-cell", "no-gameplay-binding", "npc-placement-deferred"]
  }' > "$WEB_EVIDENCE_DIR/receipt.json"
```

Validate the generated receipt's commits, versions, positive asset count, and four pass statuses before committing.

- [ ] **Step 7: Commit final docs/generated metadata and run current-head checks**

```bash
git add \
  src/author/CONTRACT.md \
  src/concepts/world-building/CONTRACT.md \
  src/generated/worldAssetCatalog.ts \
  "$WEB_EVIDENCE_DIR/README.md" \
  "$WEB_EVIDENCE_DIR/receipt.json"
git commit -m "docs: record world asset dungeon authoring proof"
```

If sync produced no tracked generated-catalog diff, `git add` leaves that path unchanged. Then rerun:

```bash
npm run ci-check
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_ROOT" \
  npm run world-assets:check
git show --check --oneline HEAD
git status --short --branch
```

- [ ] **Step 8: Obtain final review and open the Web PR**

The final independent reviewer reads the approved design, this plan, all task review receipts, `origin/dev..HEAD`, and browser evidence. Required verdicts:

- all four generated categories automatically enter the Dungeon catalog;
- search/filter/collection behavior is usable and non-authoritative;
- existing monsters/default dungeon and compositions are preserved, including the filtered malformed-composition error route;
- a synthetic NPC input is explicitly non-placeable with no invented placement semantics;
- the World Builder, Dungeon palette, and `AtlasPropModel` consume the neutral shared catalog/resolver while World Builder metadata remains intact;
- generated refs compile through merged Toolkit/API and render through `WorldAssetModel`;
- exact refs, authored blocking values, offsets, and one-anchor behavior survive;
- no licensed bytes are tracked;
- all four missing exact scenery namespaces remain empty; `dnd5e:props:plushie` resolves and renders the existing skeleton-dog variant; an unknown one-part prop retains the neutral placeholder; `items`/`weapons`/`env` never use prop-family fallback; and exact-ref collisions use legacy renderer authority;
- no ref colon split or parser-guard allowlist expansion was introduced;
- Critical/Important findings are zero before merge readiness.

Push only the reviewed head and open the Web PR against `dev`. Record the reviewed head and check commands in the PR body/comment, then move implementation issues to In Review. The human director retains merge authority.

---

## Completion checklist

- [ ] Toolkit PR merged to `main`; normal CI-published Encounter module version recorded.
- [ ] API consumes the published Encounter version and merges real authoring/session acceptance to `dev`.
- [ ] One neutral Web catalog/resolver feeds the World Builder adapter, Dungeon palette, and Atlas renderer; it checks the legacy-first exact index before the narrowly gated mapped non-exact `props` family branch.
- [ ] Web static catalog includes legacy scenery, all generated scenery categories, model-backed monsters, dynamic composition adapter, and an explicit synthetic-tested non-placeable NPC input seam.
- [ ] Dungeon palette has fixed category filters, counts, search, collection grouping, and empty state.
- [ ] World Builder still discovers every generated category and keeps current search behavior.
- [ ] Dungeon preview and play use `WorldAssetModel` for generated exact refs.
- [ ] Existing legacy props, default-dungeon monsters, and compositions retain behavior; malformed composition records remain visible but uncounted and unarmable under filtering.
- [ ] Missing exact refs for props/items/weapons/env render empty; `dnd5e:props:plushie` resolves/renders the existing skeleton-dog variant; an unknown one-part prop renders the neutral placeholder; non-prop namespaces never use family fallback; and all collection parsing uses `parseRef`/`idParts` without a guard exception.
- [ ] Provider check, focused tests, full CI, and real browser authoring pass.
- [ ] No generated file is hand-edited and no licensed GLB is tracked.
- [ ] Current large assets remain explicitly single-anchor until a separate multi-hex design.
- [ ] Final independent review reports zero Critical/Important findings.
