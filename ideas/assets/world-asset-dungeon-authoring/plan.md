# World Assets as Dungeon-Authorable Scenery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make every synchronized `props`, `items`, `weapons`, and `env` world asset automatically discoverable and placeable as DungeonSpec scenery while preserving monsters, future NPC seams, compositions, exact refs, and current one-anchor placement behavior.

**Architecture:** Toolkit expands its existing prop-like scenery route to four visual namespaces without changing the wire shape. API consumes the published Toolkit module and proves exact refs survive real authoring compilation. Web adds one discriminated builder-catalog adapter, Dungeon palette search/facets, and an `AtlasPropModel` dispatch to `WorldAssetModel`; existing legacy prop, monster, NPC seam, and composition paths remain separate behind the shared catalog surface.

**Tech Stack:** Go, `rpg-toolkit` DungeonSpec, `rpg-api` authoring/session projection, TypeScript, React, Three.js/R3F, Vitest, Testing Library, Playwright/Chromium.

**Spec:** `ideas/assets/world-asset-dungeon-authoring/design.md`

## Global Constraints

- Work in one issue worktree per repository; never implement in canonical roots.
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
- Unsupported exact refs render empty; never substitute a family/default or unrelated asset.
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

- [ ] **Step 1: Create the failing namespace acceptance tests**

Create `world_asset_scenery_test.go` with a compact valid dungeon and table-driven exact refs:

```go
package dungeonspec_test

import (
    "fmt"
    "testing"

    "github.com/stretchr/testify/require"

    "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec"
    "github.com/KirkDiggler/rpg-toolkit/tools/spatial"
)

func worldAssetSceneryYAML(ref string) string {
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
  - { ref: %q, at: [1,0], blocks_movement: false, blocks_los: true, facing: ne, offset: [0.2,-0.1,0.3] }
`, ref)
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
            compiled, err := dungeonspec.Load([]byte(worldAssetSceneryYAML(ref)))
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

func TestNonWorldAssetNamespaceStillCannotBePlaced(t *testing.T) {
    defects := dungeonspec.Validate(mustDecodeWorldAsset(t,
        worldAssetSceneryYAML("dnd5e:spells:fireball")))
    require.NotEmpty(t, defects)
    require.Contains(t, defects[0].Message, `type "spells"`)
}
```

Add this uniquely named local helper in the same file:

```go
func mustDecodeWorldAsset(t *testing.T, raw string) *dungeonspec.Spec {
    t.Helper()
    spec, err := dungeonspec.Decode([]byte(raw))
    require.NoError(t, err)
    return spec
}
```

- [ ] **Step 2: Run the focused test and verify the current restriction**

Run:

```bash
cd rpg-toolkit/rulebooks/dnd5e/encounter
go test ./dungeonspec -run 'TestWorldAssetNamespaces|TestNonWorldAssetNamespace' -count=1
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
go test ./dungeonspec -run 'TestWorldAssetNamespaces|TestNonWorldAssetNamespace|TestCompileSuite|TestScenerySuite' -count=1
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

### Task 3: Build one discriminated Web builder catalog

**Files:**
- Create: `rpg-dnd5e-web/src/author/builderCatalog.ts`
- Create: `rpg-dnd5e-web/src/author/builderCatalog.test.ts`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/monsterModels.ts`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/monsterModels.test.ts`
- Modify: `rpg-dnd5e-web/src/author/paletteData.ts`
- Modify: `rpg-dnd5e-web/src/author/paletteData.test.ts`
- Read only: `rpg-dnd5e-web/src/generated/worldAssetCatalog.ts`
- Read only: `rpg-dnd5e-web/src/concepts/world-building/catalog.ts`

**Interfaces:**
- Consumes: `GENERATED_WORLD_ASSETS`, legacy `PROP_KEYS`, authoritative monster model keys, current composition refs.
- Produces:
  - `BuilderCatalogEntry` discriminated by `authoringKind`, `source`, and `visualCategory`;
  - `STATIC_BUILDER_CATALOG`;
  - `compositionBuilderEntry(...)`;
  - `filterBuilderCatalog(entries, query, category)`;
  - stable `collection` and `searchText` presentation metadata.
- Tasks 4 and 5 consume these exact exports.

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

Expected: fail because the export does not exist.

Then add beside `MONSTER_REF_MODELS`:

```ts
export const AUTHORABLE_MONSTER_REF_IDS: readonly string[] = Object.freeze(
  Object.keys(MONSTER_REF_MODELS)
);
```

Do not export mutable candidate arrays and do not add unbound NPC appearances.

- [ ] **Step 2: Define the catalog types and category vocabulary with failing tests**

Create `builderCatalog.test.ts` first. Test a supplied fixture set rather than mutating generated code:

```ts
const worldAsset = (
  ref: string,
  displayName: string,
  category: GeneratedWorldAsset['category'],
  tags: string[]
): GeneratedWorldAsset => ({
  ref,
  displayName,
  category,
  tags,
  url: `/models/${category}/${ref.split(':').at(-1)}.glb`,
  glbSha256: 'a'.repeat(64),
  sizeBytes: 1,
  boundsMeters: [1, 1, 1],
  supportsDecoration: false,
});

const legacyEntry = (ref: string): BuilderCatalogEntry => ({
  ref,
  label: 'Legacy Altar',
  short: 'LA',
  authoringKind: 'scenery',
  source: 'legacy',
  visualCategory: 'props',
  collection: 'legacy',
  tags: [],
  blocksMovement: true,
  blocksLoS: false,
  searchText: `legacy altar ${ref}`.toLowerCase(),
});

const generated = [
  worldAsset('dnd5e:props:dark-fortress:altar_01', 'Altar 01', 'props', ['ritual']),
  worldAsset('dnd5e:items:dark-fortress:health_potion_01', 'Health Potion', 'items', ['healing']),
  worldAsset('dnd5e:weapons:dark-fortress:sword_01', 'Sword 01', 'weapons', []),
  worldAsset('dnd5e:env:dark-fortress:gate_01', 'Gate 01', 'env', ['fortress']),
];

it('adapts every generated category as scenery without inventing rules', () => {
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
  expect(entries.every((entry) => entry.gameplayRef === undefined)).toBe(true);
});
```

Run:

```bash
npm test -- --run src/author/builderCatalog.test.ts
```

Expected: fail because the module does not exist.

- [ ] **Step 3: Implement the focused catalog model**

Create `builderCatalog.ts` with these public types:

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

export interface BuilderCatalogEntry {
  ref: string;
  label: string;
  short: string;
  authoringKind: 'scenery' | 'actor';
  source: 'legacy' | 'generated' | 'composition' | 'monster' | 'npc';
  visualCategory: BuilderVisualCategory;
  collection: string;
  tags: readonly string[];
  thumbnail?: string;
  blocksMovement?: boolean;
  blocksLoS?: boolean;
  actorKind?: 'monster' | 'npc';
  gameplayRef?: string;
  searchText: string;
}
```

Expose pure functions with exact signatures:

```ts
export function generatedWorldAssetEntries(
  assets: readonly GeneratedWorldAsset[]
): BuilderCatalogEntry[];

export function buildStaticBuilderCatalog(input: {
  legacy: readonly BuilderCatalogEntry[];
  generated: readonly GeneratedWorldAsset[];
  monsters: readonly BuilderCatalogEntry[];
}): BuilderCatalogEntry[];

export function compositionBuilderEntry(input: {
  ref: string;
  label: string;
  thumbnail?: string;
}): BuilderCatalogEntry;

export function filterBuilderCatalog(
  entries: readonly BuilderCatalogEntry[],
  query: string,
  filter: BuilderCatalogFilter
): BuilderCatalogEntry[];

export const STATIC_BUILDER_CATALOG: readonly BuilderCatalogEntry[];
```

Rules:

```ts
const collectionOf = (ref: string, source: BuilderCatalogEntry['source']) => {
  const parts = ref.split(':');
  return source === 'generated' && parts.length >= 4 ? parts[2] : 'legacy';
};

const searchTextOf = (entry: Pick<
  BuilderCatalogEntry,
  'label' | 'ref' | 'tags' | 'collection'
>) => [entry.label, entry.ref, entry.collection, ...entry.tags]
  .join(' ')
  .toLocaleLowerCase();
```

`filterBuilderCatalog` trims and lowercases the query, filters by exact visual category unless `all`, preserves input order, and matches `searchText.includes(query)`.

Build `STATIC_BUILDER_CATALOG` by adapting legacy `PALETTE_PROPS`, all `Object.values(GENERATED_WORLD_ASSETS)` not colliding with a legacy exact ref, and model-backed monsters. Keep current monster labels, bossability, and thumbnail behavior in `paletteData`; do not infer NPC bindings. NPC entries are an empty adapter in this slice, but `npcs` remains in the type/filter vocabulary.

- [ ] **Step 4: Pin collision, actor, composition, and search behavior**

Add tests:

```ts
it('keeps legacy authority when generated and legacy refs collide', () => {
  const entries = buildStaticBuilderCatalog({
    legacy: [legacyEntry('dnd5e:props:altar')],
    generated: [worldAsset('dnd5e:props:altar', 'Generated Altar', 'props', [])],
    monsters: [],
  });
  expect(entries.filter((entry) => entry.ref === 'dnd5e:props:altar')).toEqual([
    expect.objectContaining({ source: 'legacy' }),
  ]);
});

it('searches label, exact ref, tags, and collection while preserving order', () => {
  expect(filterBuilderCatalog(entries, 'healing', 'all').map((e) => e.ref))
    .toEqual(['dnd5e:items:dark-fortress:health_potion_01']);
  expect(filterBuilderCatalog(entries, 'DARK-FORTRESS', 'weapons').map((e) => e.ref))
    .toEqual(['dnd5e:weapons:dark-fortress:sword_01']);
});

it('models a composition as single-anchor scenery', () => {
  expect(compositionBuilderEntry({ ref: 'composition:props:altar-set', label: 'Altar set' }))
    .toMatchObject({
      authoringKind: 'scenery',
      source: 'composition',
      visualCategory: 'compositions',
      collection: 'legacy',
    });
});

it('keeps monsters as actors and exposes no invented NPC', () => {
  expect(
    entries
      .filter((entry) => entry.visualCategory === 'monsters')
      .every((entry) => entry.authoringKind === 'actor' && entry.actorKind === 'monster')
  ).toBe(true);
  expect(entries.filter((entry) => entry.visualCategory === 'npcs')).toEqual([]);
});
```

- [ ] **Step 5: Run catalog and existing palette tests**

```bash
npm test -- --run \
  src/author/builderCatalog.test.ts \
  src/author/paletteData.test.ts \
  src/components/hex-grid/monsterModels.test.ts \
  src/concepts/world-building/WorldBuildingConcept.test.tsx
npm run typecheck
```

Expected: all pass; the World Builder still contains all generated categories.

- [ ] **Step 6: Commit and review the catalog boundary**

```bash
git add \
  src/author/builderCatalog.ts \
  src/author/builderCatalog.test.ts \
  src/author/paletteData.ts \
  src/author/paletteData.test.ts \
  src/components/hex-grid/monsterModels.ts \
  src/components/hex-grid/monsterModels.test.ts
git commit -m "feat: share a discriminated builder catalog"
```

Review must explicitly confirm: compositions are scenery; monsters remain actors; NPC is a seam without invented behavior; all four generated categories are included; legacy exact refs win collisions; no generated file changed.

---

### Task 4: Add Dungeon palette search, categories, collections, counts, and empty states

**Files:**
- Create: `rpg-dnd5e-web/src/author/Palette.test.tsx`
- Modify: `rpg-dnd5e-web/src/author/Palette.tsx`
- Modify: `rpg-dnd5e-web/src/author/DungeonBuilder.css`
- Modify: `rpg-dnd5e-web/src/author/DungeonBuilder.test.tsx`
- Modify: `rpg-dnd5e-web/src/author/CompositionThumbnailTiles.tsx`
- Modify: `rpg-dnd5e-web/src/author/CompositionThumbnailTiles.test.tsx`
- Read only: `rpg-dnd5e-web/src/author/types.ts` (retain `PaletteItem.kind: 'prop' | 'monster'`)

**Interfaces:**
- Consumes: Task 3 `STATIC_BUILDER_CATALOG`, `compositionBuilderEntry`, `filterBuilderCatalog`, and `BuilderCatalogFilter`.
- Produces: one searchable/filterable Dungeon Builder asset palette; existing `onArm(PaletteItem)` and `place` tool behavior remain compatible.
- Task 6 consumes: accessible labels/test IDs for browser proof.

- [ ] **Step 1: Write failing visible findability tests**

Create `Palette.test.tsx` with the current required props and a controlled composition source. Mock only the dynamic composition hook, not the static catalog. Add tests with these assertions:

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
  await user.click(screen.getByRole('button', { name: /^Items \(/ }));
  expect(screen.getByText(/0 assets shown/i)).toBeVisible();
  expect(screen.getByText(/No Items match the current search and filter/i)).toBeVisible();
});
```

Add a composition fixture and prove searching/filtering Compositions retains the one armed composition ref.

- [ ] **Step 2: Run the Palette tests and verify failure**

```bash
npm test -- --run src/author/Palette.test.tsx
```

Expected: fail because search, category controls, generated entries, counts, and empty state do not exist.

- [ ] **Step 3: Implement palette state and accessible controls**

In `Palette.tsx`, add local presentation-only state:

```ts
const [assetQuery, setAssetQuery] = useState('');
const [assetFilter, setAssetFilter] = useState<BuilderCatalogFilter>('all');
```

Import `useState` and Task 3 helpers. Adapt `compositionList.compositions` through `compositionBuilderEntry`, then filter the combined entries:

```ts
const compositionEntries = compositionList.compositions.map((composition) => {
  const metadata = compositionMetadata(composition);
  return compositionBuilderEntry({
    ref: compositionRef(composition.id),
    label: metadata.status === 'ready' ? metadata.name : composition.id,
  });
});
const entries = filterBuilderCatalog(
  [...STATIC_BUILDER_CATALOG, ...compositionEntries],
  assetQuery,
  assetFilter
);
```

Keep thumbnail production exclusively in `CompositionThumbnailTiles`. Build a `Set` of the filtered composition refs and pass that component only composition records whose `compositionRef(id)` is in the set. Malformed composition IDs remain visible through that component's existing error entry rather than being armed.

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

Render all fixed filters with counts computed from the unfiltered combined catalog. Labels must be accessible as `Items (0)`, not conveyed only by color. Keep all filters visible at zero.

- [ ] **Step 4: Preserve kind-specific arming and existing renderers**

For generated and legacy scenery, retain the placement tool contract:

```ts
onArm({ kind: 'prop', ref: entry.ref });
onTool('place');
```

For monsters:

```ts
onArm({ kind: 'monster', ref: entry.ref });
onTool('place');
```

For compositions, keep `CompositionThumbnailTiles` and its `{ kind: 'prop' }` arming contract. Filter its input list by the refs returned from `filterBuilderCatalog`; do not flatten composition thumbnails into generic swatches and do not expand compositions in the palette.

NPC count remains zero and cannot arm an entry because Task 3 exposes no NPC entries.

- [ ] **Step 5: Group visible scenery by collection without changing order**

Create collection sections from the already-filtered sequence with insertion-order maps:

```ts
const byCollection = new Map<string, BuilderCatalogEntry[]>();
for (const entry of visibleScenery) {
  const group = byCollection.get(entry.collection) ?? [];
  group.push(entry);
  byCollection.set(entry.collection, group);
}
```

Render disclosed labels such as `dark-fortress` and `legacy`. Do not alphabetically reshuffle entries inside a collection. Category filters and search must not mutate the armed item or DungeonDoc.

Add focused `.dg-asset-search`, `.dg-asset-filters`, `.dg-asset-count`, and `.dg-asset-empty` styles in `DungeonBuilder.css`; preserve the existing narrow left rail and keyboard focus visibility.

- [ ] **Step 6: Pin placement defaults and document stability**

In `DungeonBuilder.test.tsx`, place a generated exact ref from the palette and assert emitted YAML includes the same ref and the current fallback values:

```ts
expect(sourceText().textContent).toContain(
  'ref: "dnd5e:props:dark-fortress:alchemy_tools_01"'
);
expect(sourceText().textContent).toContain('blocks_movement: true');
expect(sourceText().textContent).toContain('blocks_los: false');
```

Then toggle both Inspector checkboxes and assert YAML follows the author. This test freezes “existing fallback, author-controlled afterward” without declaring the fallback a new semantic rule.

- [ ] **Step 7: Run UI, composition, and authoring regressions**

```bash
npm test -- --run \
  src/author/builderCatalog.test.ts \
  src/author/Palette.test.tsx \
  src/author/Palette.compositions.test.tsx \
  src/author/CompositionThumbnailTiles.test.tsx \
  src/author/DungeonBuilder.test.tsx \
  src/author/DungeonBuilder.compositions.test.tsx \
  src/author/paletteData.test.ts
npm run typecheck
npm run lint -- --quiet
```

Expected: all pass; existing region/tools sections and composition behavior remain.

- [ ] **Step 8: Commit and review findability**

```bash
git add \
  src/author/Palette.tsx \
  src/author/Palette.test.tsx \
  src/author/DungeonBuilder.css \
  src/author/DungeonBuilder.test.tsx \
  src/author/CompositionThumbnailTiles.tsx \
  src/author/CompositionThumbnailTiles.test.tsx
git commit -m "feat: make dungeon assets searchable and filterable"
```

Review must exercise zero-count filters, combined search/filter, stable collection order, composition filtering, monster preservation, and no document mutation from presentation controls. `src/author/types.ts` remains unchanged.

---

### Task 5: Render generated DungeonSpec scenery in preview and play

**Files:**
- Modify: `rpg-dnd5e-web/src/components/session/AtlasPropModel.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/AtlasPropModel.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/DungeonEnvironment.test.tsx`
- Read only: `rpg-dnd5e-web/src/components/hex-grid/WorldAssetModel.tsx`
- Read only: `rpg-dnd5e-web/src/generated/worldAssetCatalog.ts`

**Interfaces:**
- Consumes: exact generated refs from `GENERATED_WORLD_ASSETS`; `WorldAssetModel`; existing `SceneProp3D` world position/facing/offset conversion.
- Produces: `AtlasPropModel` dispatch order `composition → generated world asset → legacy prop → unsupported exact empty → legacy placeholder` shared by builder preview and gameplay.
- Task 6 consumes: real generated asset rendering in the Dungeon Builder and session route.

- [ ] **Step 1: Add failing generated-dispatch tests**

In `AtlasPropModel.test.tsx`, mock `WorldAssetModel` separately from the existing composition mock:

```tsx
vi.mock('../hex-grid/WorldAssetModel', () => ({
  WorldAssetModel: ({ assetRef }: { assetRef: string }) => (
    <group name="world-asset-model-probe" userData={{ assetRef }} />
  ),
}));

it('routes a generated exact world ref through WorldAssetModel', async () => {
  const renderer = await renderAtlasProp({
    ref: 'dnd5e:props:dark-fortress:alchemy_tools_01',
    position: { x: 1, y: -1, z: 0 },
    facing: 'ne',
    offset: { x: 0.2, y: -0.3, z: 0.4 },
  });
  const probe = renderer.scene.findByProps({ name: 'world-asset-model-probe' });
  expect(probe.props.userData.assetRef).toBe(
    'dnd5e:props:dark-fortress:alchemy_tools_01'
  );
  expect(meshes(renderer)).toHaveLength(0);
});
```

Use table cases to repeat the routing assertion for fixture refs in `items`, `weapons`, and `env` by mocking `GENERATED_WORLD_ASSETS` with all four categories. Add separate assertions that legacy and composition refs retain their old dispatch, and an unsupported exact ref renders no model probe.

- [ ] **Step 2: Run the focused test and verify failure**

```bash
npm test -- --run src/components/session/AtlasPropModel.test.tsx
```

Expected: generated world refs currently miss `resolvePropVariant` and render empty rather than invoking `WorldAssetModel`.

- [ ] **Step 3: Implement generated dispatch without changing generated code**

In `AtlasPropModel.tsx`, import:

```ts
import { WorldAssetModel } from '../hex-grid/WorldAssetModel';
import { GENERATED_WORLD_ASSETS } from '@/generated/worldAssetCatalog';
```

After the existing composition branch and before the legacy placeholder/variant branch, add:

```tsx
const worldAsset = GENERATED_WORLD_ASSETS[prop.ref];
if (worldAsset) {
  return (
    <Suspense fallback={null}>
      <ErrorBoundary fallback={null}>
        <WorldAssetModel
          assetRef={prop.ref}
          position={[world.x, world.y, world.z]}
          rotationY={facingToYaw(prop.facing)}
        />
      </ErrorBoundary>
    </Suspense>
  );
}
```

`propWorldPosition` remains the sole position/offset conversion. `WorldAssetModel` remains the sole provider normalization/runtime-scale renderer. Do not copy generated entries into `PropVariant` and do not call diagnostic-producing exact lookup on every legacy ref.

Keep the existing final branch:

```ts
if (!variant) return isExactPropRef(prop.ref) ? null : placeholder;
```

This retains fail-closed unsupported exact refs.

- [ ] **Step 4: Prove DungeonEnvironment shares the dispatch**

Extend `DungeonEnvironment.test.tsx` with an atlas prop using the promoted Alchemy Tools exact ref. Mock `AtlasPropModel` and assert the ref, authored facing, and offset are passed unchanged. This pins that builder preview and session rendering both reach the same dispatcher rather than adding a builder-only renderer.

- [ ] **Step 5: Run renderer and scene regressions**

```bash
npm test -- --run \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx \
  src/components/hex-grid/WorldAssetModel.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/author/preview3d/DungeonPreview3D.render.test.tsx \
  src/components/session/SessionCanvas.test.tsx
npm run typecheck
```

Expected: all pass; compositions, lights, monsters, legacy props, and unsupported exact refs retain behavior.

- [ ] **Step 6: Commit and review renderer authority**

```bash
git add \
  src/components/session/AtlasPropModel.tsx \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx
git commit -m "feat: render generated scenery in authored dungeons"
```

Review must confirm exact dispatch precedence, no double scale/grounding, offsets applied once, legacy and composition paths unchanged, and no substitute for unsupported exact refs.

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
  src/author/builderCatalog.test.ts \
  src/author/Palette.test.tsx \
  src/author/paletteData.test.ts \
  src/author/DungeonBuilder.test.tsx \
  src/author/DungeonBuilder.compositions.test.tsx \
  src/author/CompositionThumbnailTiles.test.tsx \
  src/author/dungeonYaml.test.ts \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx \
  src/components/hex-grid/WorldAssetModel.test.tsx \
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
12. record console errors, page errors, failed requests, HTTP errors, and WebGL errors as empty arrays.

Use role/name locators instead of CSS internals. Save screenshots under `/tmp/web-${WEB_ISSUE_NUMBER}-world-asset-dungeon-authoring/`, record their SHA-256 values in the evidence README, and commit only the textual README/JSON receipt. Do not commit licensed model pixels to the public Web repository.

- [ ] **Step 6: Document the contract and current limitations**

Update `src/author/CONTRACT.md` with:

```text
Generated props/items/weapons/env are DungeonSpec scenery. Their exact visual
ref is authored; blocking remains per placement. Visual category grants no
inventory/equipment meaning. Every placement affects one anchor cell in this
slice. Compositions are assembled technical scenery props represented by one
anchor ref. Monsters remain actors; NPC placement remains a future contract.
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
- existing monsters/default dungeon and compositions are preserved;
- NPC is an explicit adapter seam with no invented placement semantics;
- generated refs compile through merged Toolkit/API and render through `WorldAssetModel`;
- exact refs, authored blocking values, offsets, and one-anchor behavior survive;
- no licensed bytes are tracked;
- unsupported exact refs remain empty;
- Critical/Important findings are zero before merge readiness.

Push only the reviewed head and open the Web PR against `dev`. Record the reviewed head and check commands in the PR body/comment, then move implementation issues to In Review. The human director retains merge authority.

---

## Completion checklist

- [ ] Toolkit PR merged to `main`; normal CI-published Encounter module version recorded.
- [ ] API consumes the published Encounter version and merges real authoring/session acceptance to `dev`.
- [ ] Web static catalog includes legacy scenery, all generated scenery categories, model-backed monsters, composition adapter, and empty NPC seam.
- [ ] Dungeon palette has fixed category filters, counts, search, collection grouping, and empty state.
- [ ] World Builder still discovers every generated category and keeps current search behavior.
- [ ] Dungeon preview and play use `WorldAssetModel` for generated exact refs.
- [ ] Existing legacy props, default-dungeon monsters, and compositions retain behavior.
- [ ] Provider check, focused tests, full CI, and real browser authoring pass.
- [ ] No generated file is hand-edited and no licensed GLB is tracked.
- [ ] Current large assets remain explicitly single-anchor until a separate multi-hex design.
- [ ] Final independent review reports zero Critical/Important findings.
