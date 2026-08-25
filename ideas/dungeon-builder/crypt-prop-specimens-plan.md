# Builder-Ready Crypt Prop Specimens Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship one complete skeleton cage, one complete skeleton-at-table scene, and one correctly scaled rug that render identically in the dungeon builder preview and the playable dungeon.

**Architecture:** `rpg-game-assets` hand-assembles and validates three single-GLB specimens while preserving private Blender sources and component provenance. `rpg-dnd5e-web` maps the existing semantic refs to those exact artifacts and routes builder and game rendering through shared prop and lighting leaves; a checked-in showcase fixture supplies the visual gate. Development starts at the web evidence surface, but merge order is provider first and exact-pinned consumer second.

**Tech Stack:** Blender 5.x, Python 3 stdlib/unittest, GLB/glTF, React 19, TypeScript 5.8, React Three Fiber, Three.js, Vitest, Playwright, GitHub Project 19.

**Spec:** `ideas/dungeon-builder/crypt-prop-specimens.md`

## Global Constraints

- The stable refs remain `dnd5e:props:skeleton-cage`, `dnd5e:props:skeleton-table`, and `dnd5e:props:rug`.
- Finished specimens are one GLB per placeable prop; runtime composition and independently placeable components are out of scope.
- Raw converted source GLBs remain unchanged under private `library/`; finished assembly artifacts are distinct files under private `harness/models/synty/props/`.
- Never commit raw or converted Synty assets, Blender assembly sources, or provider manifests to public `rpg-dnd5e-web`.
- The web consumes the approved assets without specimen-specific scale, rotation, offset, or companion repair.
- Remove `PropVariant.renderScale`; every prop renders at `SYNTY_SCALE` plus authored facing/offset only.
- Existing gameplay semantics remain cage = obstacle/blocks movement+LoS, table = cover/blocks movement only, rug = decor/blocks neither. Stop for a new decision if visual evidence disproves one of those roles.
- Builder and game use the same `AtlasPropModel`, `PropModel`, resolver, artifact URL, authored facing/offset, and fixed existing scene lights.
- No authored-lighting behavior, proto, toolkit, API, or deployment change belongs in this wave.
- Candidate review through an explicit local asset source authorizes provider promotion; final review against the exact merged provider revision authorizes web merge.
- New implementation branches require owning-repository slice issues on Project 19 before branch creation.
- Provider branch base/PR target is `origin/main`; web branch base/PR target is `origin/dev`. Fetch first and cut from the remote ref.
- Use Team `Assets`, Area `The Dungeon`, Initiative `Four-player Level-3 Dungeon`; provider Kind `Build`, web Kind `Fix`; both are sub-issues of journey #169.
- GitHub issue/PR posts use the authenticated login and the Assets signature: `— assets agent, on behalf of KirkDiggler`.
- Kirk alone performs visual approval and merges.

## File Map

### `rpg-game-assets` provider

- Create `scripts/export_crypt_prop_specimens.py` — Blender-only exporter for three named assembly collections; records component transforms before baking and exports grounded static GLBs.
- Create `scripts/validate_crypt_prop_specimens.py` — pure-Python fail-loud validator and deterministic verification-report generator.
- Create `scripts/test_validate_crypt_prop_specimens.py` — stdlib unit tests for record shape, exact specimen identities, hashes, bounds, nodes, materials, floor contact, and evidence.
- Create `library/assemblies/crypt-prop-specimens-v1.blend` — private hand-authored source scene.
- Create `library/assemblies/crypt-prop-specimens-v1.json` — generated private component/provenance/transform record.
- Create `harness/models/synty/props/Crypt_Skeleton_Cage_01.glb` — final cage assembly.
- Create `harness/models/synty/props/Crypt_Skeleton_Table_01.glb` — final table assembly.
- Create `harness/models/synty/props/Crypt_Rug_01.glb` — final approved rug.
- Create `evidence/crypt-prop-specimens-v1/README.md` and PNG evidence — neutral/rejected-candidate review.
- Create `evidence/crypt-prop-specimens-v1/verification.json` — deterministic hashes, bounds, nodes, and mesh statistics.
- Modify `library/prop-role-map.json` — replace the three broken key families with one approved artifact each and truthful notes.
- Modify generated `harness/models/synty/props/manifest.json`, `harness/models/synty/mesh-stats.json`, and `harness/catalogs/synty-complete-inventory.json`.
- Delete the five superseded promoted files from `harness/models/synty/props/`: `SM_Prop_Skeleton_Cage_01.glb`, `SM_Prop_Skeleton_Table_01.glb`, and `SM_Prop_Rug_01/02/03.glb`. Their private `library/polygon-dungeon/` sources remain.

### `rpg-dnd5e-web` consumer

- Create `src/components/session/AtlasPropModel.tsx` — the sole atlas-prop resolver/placeholder/`PropModel` leaf used by builder and game.
- Create `src/components/session/AtlasPropModel.test.tsx` — known, unknown, loading/error, transform, and URL behavior.
- Create `src/components/session/DungeonSceneLights.tsx` and `.test.tsx` — one fixed existing light rig shared by builder and game.
- Create `src/author/fixtures/cryptPropShowcase.ts` and `.test.ts` — the reusable three-specimen authored document.
- Create `docs/evidence/275-crypt-prop-specimens/README.md` plus before/candidate/final screenshots — exact provider revision and artifact hashes.
- Modify `src/author/DungeonBuilderSandbox.tsx` — select the showcase with `?authorFixture=crypt-props` while retaining the reference tomb default.
- Modify `src/author/preview3d/DungeonPreview3D.tsx` and `.test.ts` — use shared prop/light leaves and pin preview/game scene identity for the showcase.
- Modify `src/components/session/SessionCanvas.tsx` and `.test.tsx` — use shared prop/light leaves without changing fallback behavior.
- Modify `src/components/hex-grid/propManifest.ts`, `.test.ts`, `PropModel.tsx`, and `PropModel.test.tsx` — exact approved paths, one variant per key, no `renderScale` type or multiplication.
- Replace `src/author/thumbs/skeleton-cage.png`, `skeleton-table.png`, and `rug.png` with truthful exact-artifact thumbnails.

---

## Execution Setup: Earn the Two Repository Slices

Run this once from `/home/kirk/game-dev` before Task 1. It creates both backing issues, boards them, and attaches them directly beneath journey #169. Do not create either branch until the final read-back prints the required fields.

- [ ] **Step 1: Derive the operator and create the exact issue bodies**

```bash
set -euo pipefail
LOGIN=$(gh api user --jq .login)
test "$LOGIN" = "KirkDiggler"

cat > /tmp/crypt-props-assets-issue.md <<'EOF'
## Parent journey

KirkDiggler/rpg-project#169 — Composable Dungeon Builder

## Design

KirkDiggler/rpg-project#275 / PR #276

## Outcome

Hand-assemble and promote one complete skeleton cage, one complete skeleton-at-table scene, and one correctly scaled rug as private, provenance-tracked, builder-ready GLBs.

## Done when

- Private Blender source and component-transform provenance are committed.
- Three distinct final GLBs pass manifest, inventory, bounds, floor-contact, material, texture, and mesh-stat checks.
- Neutral and rejected-candidate evidence is reviewable.
- Kirk approves the candidates in the web showcase using an explicit non-mutating provider source.
- The provider PR merges before the web consumer pins it.

## Non-goals

Runtime prop composition, public asset commits, authored lighting, gameplay interactions, or changes outside rpg-game-assets.

— assets agent, on behalf of KirkDiggler
EOF

cat > /tmp/crypt-props-web-issue.md <<'EOF'
## Parent journey

KirkDiggler/rpg-project#169 — Composable Dungeon Builder

## Design

KirkDiggler/rpg-project#275 / PR #276

## Outcome

Consume the three approved crypt prop specimens without client-side repair and prove the same artifacts/transforms in the builder preview and playable dungeon.

## Done when

- The three refs resolve to exactly one approved provider artifact each.
- `renderScale` is removed and every prop uses the shared scale contract.
- Builder and game use shared atlas-prop and scene-light leaves.
- A reusable showcase fixture round-trips ref, facing, and offset.
- Thumbnails show the exact approved artifacts.
- Kirk approves builder and playable-dungeon evidence against the exact merged provider revision.

## Non-goals

Committing licensed GLBs, runtime component composition, authored lighting behavior, or backend contract changes.

— assets agent, on behalf of KirkDiggler
EOF
```

- [ ] **Step 2: Create both issues and add exact Project 19 fields**

```bash
ASSET_ISSUE_URL=$(gh issue create --repo KirkDiggler/rpg-game-assets \
  --title 'asset: assemble builder-ready crypt prop specimens' \
  --body-file /tmp/crypt-props-assets-issue.md --assignee "$LOGIN")
WEB_ISSUE_URL=$(gh issue create --repo KirkDiggler/rpg-dnd5e-web \
  --title 'fix: consume approved crypt prop specimens in builder and game' \
  --body-file /tmp/crypt-props-web-issue.md --assignee "$LOGIN")

FIELDS=$(mktemp)
gh project field-list 19 --owner KirkDiggler --format json > "$FIELDS"
PROJECT_ID=$(gh project view 19 --owner KirkDiggler --format json --jq .id)

board_slice() {
  issue_url=$1
  kind=$2
  item_id=$(gh project item-add 19 --owner KirkDiggler --url "$issue_url" --format json --jq .id)
  for pair in "Status|In Progress" "Team|Assets" "Area|The Dungeon" \
              "Initiative|Four-player Level-3 Dungeon" "Kind|$kind"; do
    field_name=${pair%%|*}
    option_name=${pair#*|}
    field_id=$(jq -er --arg n "$field_name" '.fields[] | select(.name == $n) | .id' "$FIELDS")
    option_id=$(jq -er --arg n "$field_name" --arg o "$option_name" \
      '.fields[] | select(.name == $n) | .options[] | select(.name == $o) | .id' "$FIELDS")
    gh project item-edit --id "$item_id" --project-id "$PROJECT_ID" \
      --field-id "$field_id" --single-select-option-id "$option_id" >/dev/null
  done
  issue_number=${issue_url##*/}
  repo=${issue_url#https://github.com/}; repo=${repo%/issues/*}
  child_db_id=$(gh api "repos/$repo/issues/$issue_number" --jq .id)
  gh api --method POST repos/KirkDiggler/rpg-project/issues/169/sub_issues \
    -F sub_issue_id="$child_db_id" >/dev/null
}

board_slice "$ASSET_ISSUE_URL" Build
board_slice "$WEB_ISSUE_URL" Fix
printf 'ASSET_ISSUE_URL=%s\nWEB_ISSUE_URL=%s\n' "$ASSET_ISSUE_URL" "$WEB_ISSUE_URL"
```

- [ ] **Step 3: Read back both board items before branching**

```bash
gh project item-list 19 --owner KirkDiggler --format json --limit 1000 | \
  jq --arg a "$ASSET_ISSUE_URL" --arg w "$WEB_ISSUE_URL" '
    [.items[] | select(.content.url == $a or .content.url == $w) |
      {url:.content.url,status,team,area,initiative,kind,assignees}]'
```

Expected: two items; both `In Progress`, `Assets`, `The Dungeon`, `Four-player Level-3 Dungeon`; kinds `Build` and `Fix`; assignee `KirkDiggler`.

### Context reload for every task shell

Every task executor runs this before its task commands; no task relies on shell
variables surviving an earlier step:

```bash
ASSET_ISSUE_URL=$(gh issue list --repo KirkDiggler/rpg-game-assets --state all \
  --limit 100 --json title,url --jq \
  '.[] | select(.title == "asset: assemble builder-ready crypt prop specimens") | .url')
WEB_ISSUE_URL=$(gh issue list --repo KirkDiggler/rpg-dnd5e-web --state all \
  --limit 100 --json title,url --jq \
  '.[] | select(.title == "fix: consume approved crypt prop specimens in builder and game") | .url')
test -n "$ASSET_ISSUE_URL" && test -n "$WEB_ISSUE_URL"
ASSET_ISSUE_NUMBER=${ASSET_ISSUE_URL##*/}
WEB_ISSUE_NUMBER=${WEB_ISSUE_URL##*/}
ASSET_WT="/home/kirk/.pi/worktrees/rpg-game-assets/${ASSET_ISSUE_NUMBER}-crypt-props"
WEB_WT="/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE_NUMBER}-crypt-props"
```

## Task 1: Build the Web Showcase and One Shared Render Path

**Files:**
- Create: `rpg-dnd5e-web/src/author/fixtures/cryptPropShowcase.ts`
- Create: `rpg-dnd5e-web/src/author/fixtures/cryptPropShowcase.test.ts`
- Create: `rpg-dnd5e-web/src/components/session/AtlasPropModel.tsx`
- Create: `rpg-dnd5e-web/src/components/session/AtlasPropModel.test.tsx`
- Create: `rpg-dnd5e-web/src/components/session/DungeonSceneLights.tsx`
- Create: `rpg-dnd5e-web/src/components/session/DungeonSceneLights.test.tsx`
- Modify: `rpg-dnd5e-web/src/author/DungeonBuilderSandbox.tsx:1-24`
- Modify: `rpg-dnd5e-web/src/author/preview3d/DungeonPreview3D.tsx:15-93,164-191`
- Modify: `rpg-dnd5e-web/src/author/preview3d/DungeonPreview3D.test.ts:1-96`
- Modify: `rpg-dnd5e-web/src/components/session/SessionCanvas.tsx:66-162,446-470`
- Modify: `rpg-dnd5e-web/src/components/session/SessionCanvas.test.tsx:157-312`

**Interfaces:**
- Consumes: existing `DungeonDoc`, `fromOffset`, `fixtureAtlasOf`, `SceneProp3D`, `propWorldPosition`, `facingToYaw`, `resolvePropVariant`, and `PropModel`.
- Produces: `cryptPropShowcaseDoc(): DungeonDoc`, `sandboxDocForSearch(search: string): DungeonDoc`, `AtlasPropModel({prop, hexSize, orientation})`, `DungeonSceneLights`, and `DUNGEON_SCENE_LIGHTING`.

- [ ] **Step 1: Create the web branch from the required integration base**

```bash
WEB_ISSUE_NUMBER=${WEB_ISSUE_URL##*/}
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add \
  "/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE_NUMBER}-crypt-props" \
  -b "fix/${WEB_ISSUE_NUMBER}-crypt-prop-specimens" origin/dev
cd "/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE_NUMBER}-crypt-props"
npm install
gh issue comment "$WEB_ISSUE_NUMBER" --repo KirkDiggler/rpg-dnd5e-web --body \
"WORK SESSION STARTED

Branch: fix/${WEB_ISSUE_NUMBER}-crypt-prop-specimens from origin/dev.
First proof: checked-in crypt showcase through shared builder/game render leaves.

— assets agent, on behalf of KirkDiggler"
npm run test:run -- src/author/preview3d/DungeonPreview3D.test.ts \
  src/components/session/SessionCanvas.test.tsx
```

Expected: existing preview/session tests pass before extraction.

- [ ] **Step 2: Write the failing showcase fixture test**

```ts
import { emitDungeon, parseDungeon } from '../dungeonYaml';
import { fixtureAtlasOf } from './fixtureAtlas';
import { cryptPropShowcaseDoc } from './cryptPropShowcase';
import { describe, expect, it } from 'vitest';

describe('cryptPropShowcaseDoc', () => {
  it('contains exactly the three first-wave refs with authored facing and offset', () => {
    const doc = cryptPropShowcaseDoc();
    expect(doc.place.map((p) => p.ref)).toEqual([
      'dnd5e:props:skeleton-cage',
      'dnd5e:props:skeleton-table',
      'dnd5e:props:rug',
    ]);
    expect(doc.place.map((p) => [p.facing, p.offset])).toEqual([
      ['se', [0, 0]],
      ['e', [0, 0]],
      ['e', [0, 0]],
    ]);
  });

  it('round-trips byte-for-byte and projects the same three atlas refs', () => {
    const doc = cryptPropShowcaseDoc();
    const yaml = emitDungeon(doc);
    expect(emitDungeon(parseDungeon(yaml))).toBe(yaml);
    expect(fixtureAtlasOf(doc).props.map((p) => p.ref)).toEqual(
      doc.place.map((p) => p.ref)
    );
  });
});
```

- [ ] **Step 3: Run the fixture test and verify red**

Run: `npm run test:run -- src/author/fixtures/cryptPropShowcase.test.ts`

Expected: FAIL because `./cryptPropShowcase` does not exist.

- [ ] **Step 4: Implement the exact showcase document**

```ts
import type { DungeonDoc } from '../dungeonYaml';
import { fromOffset } from '../hexOffset';

export function cryptPropShowcaseDoc(): DungeonDoc {
  const p = (col: number, row: number) => fromOffset('pointy', [col, row]);
  const cells = Array.from({ length: 7 }, (_, row) =>
    Array.from({ length: 12 }, (_, col) => p(col, row))
  ).flat();
  return {
    version: 2,
    key: 'crypt-prop-showcase',
    name: 'Crypt Prop Showcase',
    orientation: 'pointy',
    void: 'opaque',
    regions: [
      {
        id: 'gallery',
        name: 'Gallery',
        archetype: 'crypt',
        lighting: { intensity: 0.4 },
        cells,
      },
    ],
    start: p(1, 3),
    walls: [],
    doors: [],
    place: [
      {
        ref: 'dnd5e:props:skeleton-cage',
        at: p(3, 2),
        blocksMovement: true,
        blocksLos: true,
        facing: 'se',
        offset: [0, 0],
      },
      {
        ref: 'dnd5e:props:skeleton-table',
        at: p(6, 4),
        blocksMovement: true,
        blocksLos: false,
        facing: 'e',
        offset: [0, 0],
      },
      {
        ref: 'dnd5e:props:rug',
        at: p(9, 3),
        blocksMovement: false,
        blocksLos: false,
        facing: 'e',
        offset: [0, 0],
      },
    ],
  };
}
```

- [ ] **Step 5: Add query-selected sandbox loading**

In `DungeonBuilderSandbox.tsx`, export this pure selector and use it to build `initialYaml`:

```ts
export function sandboxDocForSearch(search: string): DungeonDoc {
  return new URLSearchParams(search).get('authorFixture') === 'crypt-props'
    ? cryptPropShowcaseDoc()
    : referenceTombDoc();
}

export function DungeonBuilderSandbox() {
  const initialYaml = useMemo(
    () => emitDungeon(sandboxDocForSearch(window.location.search)),
    []
  );
  return (
    <DungeonBuilder
      initialYaml={initialYaml}
      fixtureCompile={fixtureAtlasOf}
      persistDraft={false}
      allowYamlFileIO
    />
  );
}
```

Extend `cryptPropShowcase.test.ts` to assert empty search returns `reference-tomb` and `?authorFixture=crypt-props` returns `crypt-prop-showcase`.

- [ ] **Step 6: Run the fixture tests and verify green**

Run: `npm run test:run -- src/author/fixtures/cryptPropShowcase.test.ts`

Expected: PASS, 4 tests.

- [ ] **Step 7: Write failing tests for the shared prop and light leaves**

`AtlasPropModel.test.tsx` must define its own R3F helpers so it tests the new
leaf directly rather than reaching through `SessionCanvas`:

```tsx
vi.mock('@react-three/drei', () => ({
  useGLTF: (url: string) => {
    const scene = new THREE.Group();
    const mesh = new THREE.Mesh(
      new THREE.BoxGeometry(),
      new THREE.MeshStandardMaterial()
    );
    mesh.name = url;
    scene.add(mesh);
    return { scene };
  },
}));

async function renderAtlasProp(prop: SceneProp3D) {
  return ReactThreeTestRenderer.create(
    <AtlasPropModel prop={prop} hexSize={1} orientation="pointy" />
  );
}

function meshes(renderer: Awaited<ReturnType<typeof renderAtlasProp>>) {
  return renderer.scene
    .findAllByType('Mesh')
    .map((node) => (node as unknown as { instance: THREE.Mesh }).instance);
}

it('resolves a known ref and applies authored position/facing through PropModel', async () => {
  const renderer = await renderAtlasProp({
    ref: 'dnd5e:props:pillar',
    position: { x: 1, y: -1, z: 0 },
    facing: 'ne',
    offset: { x: 0.2, y: -0.3 },
  });
  expect(meshes(renderer).map((mesh) => mesh.name)).toContain(
    '/models/synty/props/SM_Env_Pillar_Round_01.glb'
  );
  const outer = renderer.scene
    .findAllByType('Group')
    .map((node) => (node as unknown as { instance: THREE.Group }).instance)
    .find((group) => Math.abs(group.position.x) > 0.1);
  expect(outer?.rotation.y).toBeCloseTo(facingToYaw('pointy', 'ne'));
});

it('renders the neutral placeholder for an unknown ref', async () => {
  const renderer = await renderAtlasProp({
    ref: 'homebrew:props:unknown',
    position: { x: 0, y: 0, z: 0 },
    facing: '',
    offset: { x: 0, y: 0 },
  });
  expect(
    meshes(renderer).filter((mesh) => mesh.geometry.type === 'CylinderGeometry')
  ).toHaveLength(1);
});
```

Import `ReactThreeTestRenderer`, `THREE`, `SceneProp3D`, `facingToYaw`,
`AtlasPropModel`, and Vitest's `describe/expect/it/vi` explicitly in that file.

`DungeonSceneLights.test.tsx` must assert the exported values are exactly:

```ts
expect(DUNGEON_SCENE_LIGHTING).toEqual({
  ambientIntensity: 0.6,
  directionalIntensity: 0.8,
  directionalPosition: [10, 20, 10],
});
```

- [ ] **Step 8: Run both new tests and verify red**

Run: `npm run test:run -- src/components/session/AtlasPropModel.test.tsx src/components/session/DungeonSceneLights.test.tsx`

Expected: FAIL because both modules are absent.

- [ ] **Step 9: Extract the shared leaves without changing behavior**

Create `DungeonSceneLights.tsx`:

```tsx
export const DUNGEON_SCENE_LIGHTING = {
  ambientIntensity: 0.6,
  directionalIntensity: 0.8,
  directionalPosition: [10, 20, 10] as [number, number, number],
} as const;

export function DungeonSceneLights() {
  return (
    <>
      <ambientLight intensity={DUNGEON_SCENE_LIGHTING.ambientIntensity} />
      <directionalLight
        intensity={DUNGEON_SCENE_LIGHTING.directionalIntensity}
        position={DUNGEON_SCENE_LIGHTING.directionalPosition}
      />
    </>
  );
}
```

Create `AtlasPropModel.tsx` by moving the existing `SessionCanvas.tsx` implementation into one exported component with this interface:

```tsx
export interface AtlasPropModelProps {
  prop: SceneProp3D;
  hexSize: number;
  orientation: 'pointy';
}

export function AtlasPropModel({
  prop,
  hexSize,
  orientation,
}: AtlasPropModelProps) {
  const world = propWorldPosition(prop, hexSize);
  const placeholder = (
    <mesh position={[world.x, hexSize * 0.5, world.z]}>
      <cylinderGeometry args={[hexSize * 0.3, hexSize * 0.3, hexSize, 6]} />
      <meshStandardMaterial color="#a16207" />
    </mesh>
  );
  const variant = resolvePropVariant(prop.ref);
  if (!variant) return placeholder;
  return (
    <Suspense fallback={placeholder}>
      <ErrorBoundary fallback={placeholder}>
        <PropModel
          variant={variant}
          position={[world.x, 0, world.z]}
          rotationY={facingToYaw(orientation, prop.facing)}
        />
      </ErrorBoundary>
    </Suspense>
  );
}
```

Replace the preview map leaf with
`<AtlasPropModel prop={prop} hexSize={HEX_SIZE} orientation="pointy" />`, the
session map leaf with
`<AtlasPropModel prop={prop} hexSize={hexSize} orientation="pointy" />`, and
both duplicated light pairs with `<DungeonSceneLights />`.

- [ ] **Step 10: Run shared-leaf and existing path tests**

Run:

```bash
npm run test:run -- \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonSceneLights.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/author/fixtures/cryptPropShowcase.test.ts
```

Expected: all pass; existing unknown/loading/error placeholder tests remain green.

- [ ] **Step 11: Add showcase preview/game identity coverage**

Extend `DungeonPreview3D.test.ts` with:

```ts
it('builds the crypt specimen showcase identically for preview and game', () => {
  const atlas = fixtureAtlasOf(cryptPropShowcaseDoc());
  const preview = previewScene(atlas);
  expect(preview.ok).toBe(true);
  if (!preview.ok) return;
  const gate = resolveSceneLayout(atlas);
  expect(gate.ok).toBe(true);
  if (!gate.ok) return;
  const game = buildScene3D(atlas, HEX_SIZE, gate.layout);
  expect(preview.scene.props).toEqual(game.props);
  expect(preview.scene.props.map((p) => p.ref)).toEqual([
    'dnd5e:props:skeleton-cage',
    'dnd5e:props:skeleton-table',
    'dnd5e:props:rug',
  ]);
});
```

- [ ] **Step 12: Verify and commit the web evidence surface**

Run:

```bash
npm run test:run -- \
  src/author/fixtures/cryptPropShowcase.test.ts \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonSceneLights.test.tsx \
  src/components/session/SessionCanvas.test.tsx
npm run format:check

git add src/author src/components/session
git commit -m "test(author): add shared crypt prop showcase (${WEB_ISSUE_NUMBER})"
```

## Task 2: Build the Provider Export and Validation Contract

**Files:**
- Create: `rpg-game-assets/scripts/export_crypt_prop_specimens.py`
- Create: `rpg-game-assets/scripts/validate_crypt_prop_specimens.py`
- Create: `rpg-game-assets/scripts/test_validate_crypt_prop_specimens.py`

**Interfaces:**
- Consumes: Blender collections `EXPORT_Crypt_Skeleton_Cage_01`, `EXPORT_Crypt_Skeleton_Table_01`, `EXPORT_Crypt_Rug_01`; each source root has a canonical `source_path` custom property.
- Produces: three exact harness GLBs, `library/assemblies/crypt-prop-specimens-v1.json`, and deterministic `evidence/crypt-prop-specimens-v1/verification.json`.
- `validate_record(record_path: Path, repo_root: Path) -> dict[str, Any]` raises `SpecimenValidationError` on any contract violation.
- `write_report(record_path: Path, output_path: Path, repo_root: Path, check: bool) -> dict[str, Any]` writes or byte-checks deterministic JSON.

- [ ] **Step 1: Create the provider branch from `origin/main`**

```bash
ASSET_ISSUE_NUMBER=${ASSET_ISSUE_URL##*/}
git -C /home/kirk/game-dev/rpg-game-assets fetch origin
git -C /home/kirk/game-dev/rpg-game-assets worktree add \
  "/home/kirk/.pi/worktrees/rpg-game-assets/${ASSET_ISSUE_NUMBER}-crypt-props" \
  -b "asset/${ASSET_ISSUE_NUMBER}-crypt-prop-specimens" origin/main
cd "/home/kirk/.pi/worktrees/rpg-game-assets/${ASSET_ISSUE_NUMBER}-crypt-props"
gh issue comment "$ASSET_ISSUE_NUMBER" --repo KirkDiggler/rpg-game-assets --body \
"WORK SESSION STARTED

Branch: asset/${ASSET_ISSUE_NUMBER}-crypt-prop-specimens from origin/main.
First proof: fail-loud assembly export/provenance validator before binary promotion.

— assets agent, on behalf of KirkDiggler"
python3 -m unittest discover -s scripts -p 'test_*.py'
```

Expected: existing provider test baseline passes; Blender-dependent integration cases may report their existing documented skips.

- [ ] **Step 2: Write the failing validator contract tests**

The test fixture writes tiny indexed GLBs with one PNG-backed material and
identity nodes. In the test class, define
`fixture_repo(self, *, valid: bool, exported_translation: list[float] | None =
None) -> tuple[Path, Path]`. It owns a `TemporaryDirectory` via
`self.addCleanup`, writes the exact three source sets from `EXPECTED`, writes
three final artifact GLBs whose node names match the record, writes the 12
required neutral PNG paths, writes the source `.blend` sentinel, and returns
`(repo_root, record_path)`. Its GLB helper must emit indexed TRIANGLES,
POSITION min/max, one material, and one embedded 1×1 PNG so the fixture passes
the same parser as real artifacts. Add these exact test cases:

```py
class CryptPropSpecimenValidationTests(unittest.TestCase):
    def test_valid_three_specimen_record_produces_stable_report(self):
        root, record = self.fixture_repo(valid=True)
        first = validate.validate_record(record, root)
        second = validate.validate_record(record, root)
        self.assertEqual(first, second)
        self.assertEqual(
            ["skeleton-cage", "skeleton-table", "rug"],
            [item["id"] for item in first["specimens"]],
        )

    def test_ref_artifact_and_role_contracts_are_exact(self):
        root, record = self.fixture_repo(valid=True)
        document = json.loads(record.read_text())
        document["specimens"][0]["ref"] = "dnd5e:props:cage"
        record.write_text(json.dumps(document))
        with self.assertRaisesRegex(validate.SpecimenValidationError, "skeleton-cage: ref"):
            validate.validate_record(record, root)

    def test_missing_component_source_or_named_node_fails(self):
        root, record = self.fixture_repo(valid=True)
        (root / "library/polygon-dungeon/SM_Prop_Toture_Cage_01.glb").unlink()
        with self.assertRaisesRegex(validate.SpecimenValidationError, "source component missing"):
            validate.validate_record(record, root)

    def test_non_identity_export_node_or_floor_gap_fails(self):
        root, record = self.fixture_repo(valid=True, exported_translation=[0, 0.25, 0])
        with self.assertRaisesRegex(validate.SpecimenValidationError, "identity export transform"):
            validate.validate_record(record, root)

    def test_missing_material_texture_or_neutral_evidence_fails(self):
        root, record = self.fixture_repo(valid=True)
        evidence = root / "evidence/crypt-prop-specimens-v1/Crypt_Rug_01_top.png"
        evidence.unlink()
        with self.assertRaisesRegex(validate.SpecimenValidationError, "neutral evidence missing"):
            validate.validate_record(record, root)

    def test_check_mode_refuses_stale_report_bytes(self):
        root, record = self.fixture_repo(valid=True)
        output = root / "evidence/crypt-prop-specimens-v1/verification.json"
        output.write_text("{}\n")
        with self.assertRaisesRegex(validate.SpecimenValidationError, "stale verification report"):
            validate.write_report(record, output, root, check=True)
```

- [ ] **Step 3: Run the validator tests and verify red**

Run: `python3 scripts/test_validate_crypt_prop_specimens.py -v`

Expected: FAIL because `validate_crypt_prop_specimens` does not exist.

- [ ] **Step 4: Implement the pure validator and exact contracts**

Define these immutable contracts in `validate_crypt_prop_specimens.py`:

```py
EXPECTED_ORDER = ("skeleton-cage", "skeleton-table", "rug")
EXPECTED = {
    "skeleton-cage": {
        "ref": "dnd5e:props:skeleton-cage",
        "artifact": "harness/models/synty/props/Crypt_Skeleton_Cage_01.glb",
        "role": "obstacle",
        "blocksMovement": True,
        "blocksLineOfSight": True,
        "requiredSources": {
            "library/polygon-dungeon/SM_Prop_Skeleton_Cage_01.glb",
            "library/polygon-dungeon/SM_Prop_Toture_Cage_01.glb",
        },
    },
    "skeleton-table": {
        "ref": "dnd5e:props:skeleton-table",
        "artifact": "harness/models/synty/props/Crypt_Skeleton_Table_01.glb",
        "role": "cover",
        "blocksMovement": True,
        "blocksLineOfSight": False,
        "requiredPoseSource":
            "library/polygon-dungeon/SM_Prop_Skeleton_Table_01.glb",
        "allowedSupportSources": {
            "library/polygon-dungeon/SM_Prop_Table_01.glb",
            "library/polygon-dungeon/SM_Prop_StoneTable_01.glb",
            "library/polygon-dungeon/SM_Prop_Table_Round_01.glb",
            "library/polygon-dungeon/SM_Prop_Table_Round_02.glb",
            "library/polygon-dungeon/SM_Prop_Table_Round_Broken_01.glb",
        },
    },
    "rug": {
        "ref": "dnd5e:props:rug",
        "artifact": "harness/models/synty/props/Crypt_Rug_01.glb",
        "role": "decor",
        "blocksMovement": False,
        "blocksLineOfSight": False,
        "allowedSources": {
            "library/polygon-dungeon/SM_Prop_Rug_01.glb",
            "library/polygon-dungeon/SM_Prop_Rug_02.glb",
            "library/polygon-dungeon/SM_Prop_Rug_03.glb",
        },
    },
}
TARGET_REFS = frozenset(item["ref"] for item in EXPECTED.values())
```

Validation must require:

- record keys exactly `schemaVersion`, `specimenSet`, `sourceBlend`, `specimens`;
- version `1`, set `crypt-prop-specimens-v1`, source blend `library/assemblies/crypt-prop-specimens-v1.blend`;
- exactly `EXPECTED_ORDER`, preserving cage → table → rug contract order;
- each specimen record has exactly `id`, `ref`, `artifact`, `role`,
  `blocksMovement`, `blocksLineOfSight`, `forwardAxis`, `floorContactY`, and
  `components`; each component has exactly `source`, `node`, `translation`,
  `rotationQuaternion`, and `scale`;
- exact ref/artifact/role/blocking values above;
- cage exact source set; table pose source plus exactly one allowed support source; rug exactly one allowed source;
- every component has canonical source path, preserved node name, finite
  three-number translation, normalized four-number rotation quaternion in glTF
  `[x,y,z,w]` order, and finite three-number non-zero scale;
- final GLB exists, hashes successfully, has only identity exported node transforms, contains each preserved named component node, has meshes/materials/embedded PNG images, and no animations;
- accessor bounds have finite values and combined minimum Y is within `1e-4` of zero;
- these 12 exact neutral files:
  `Crypt_Skeleton_Cage_01_{front,side,threequarter,top}.png`,
  `Crypt_Skeleton_Table_01_{front,side,threequarter,top}.png`, and
  `Crypt_Rug_01_{front,side,threequarter,top}.png`;
- report top-level keys exactly `schemaVersion`, `specimenSet`,
  `sourceRecordSha256`, and `specimens`; each specimen entry has exactly `id`,
  `ref`, `role`, `artifact`, `bounds`, `floorContactY`, `forwardAxis`, `nodes`,
  `components`, and `meshStats`; `artifact` has `path`, `sha256`, and
  `sizeBytes`; `bounds` has `min` and `max`; specimens retain
  `EXPECTED_ORDER`;
- report includes component transforms, forward axis `+Z`, floor contact `0`, and `build_mesh_stats.stats_for_glb` output; and
- JSON serialization uses `indent=2`, `sort_keys=True`, and one trailing newline.

CLI:

```bash
python3 scripts/validate_crypt_prop_specimens.py \
  --record library/assemblies/crypt-prop-specimens-v1.json \
  --output evidence/crypt-prop-specimens-v1/verification.json
python3 scripts/validate_crypt_prop_specimens.py \
  --record library/assemblies/crypt-prop-specimens-v1.json \
  --output evidence/crypt-prop-specimens-v1/verification.json --check
```

- [ ] **Step 5: Run validator tests and verify green**

Run: `python3 scripts/test_validate_crypt_prop_specimens.py -v`

Expected: PASS, 6 tests.

- [ ] **Step 6: Implement the Blender exporter contract**

`export_crypt_prop_specimens.py` must be importable only inside Blender and define:

```py
SPECIMENS = (
    {
        "id": "skeleton-cage",
        "collection": "EXPORT_Crypt_Skeleton_Cage_01",
        "artifact": "harness/models/synty/props/Crypt_Skeleton_Cage_01.glb",
        "ref": "dnd5e:props:skeleton-cage",
        "role": "obstacle",
        "blocksMovement": True,
        "blocksLineOfSight": True,
    },
    {
        "id": "skeleton-table",
        "collection": "EXPORT_Crypt_Skeleton_Table_01",
        "artifact": "harness/models/synty/props/Crypt_Skeleton_Table_01.glb",
        "ref": "dnd5e:props:skeleton-table",
        "role": "cover",
        "blocksMovement": True,
        "blocksLineOfSight": False,
    },
    {
        "id": "rug",
        "collection": "EXPORT_Crypt_Rug_01",
        "artifact": "harness/models/synty/props/Crypt_Rug_01.glb",
        "ref": "dnd5e:props:rug",
        "role": "decor",
        "blocksMovement": False,
        "blocksLineOfSight": False,
    },
)
```

For each collection the exporter must:

1. require at least one mesh and a `source_path` custom property on every top-level source object;
2. capture source path, preserved object name, world translation, glTF-order
   `[x,y,z,w]` quaternion, and scale before baking;
3. duplicate the collection objects into a temporary export collection;
4. shift the duplicate set so combined world minimum Z is exactly zero in Blender's Z-up scene;
5. apply location, rotation, and scale to the duplicates while preserving names;
6. export selected duplicates with `export_format="GLB"`, `export_yup=True`, `export_apply=False`, `export_animations=False`, and embedded images;
7. delete the temporary collection; and
8. write the private record in `EXPECTED_ORDER` to `library/assemblies/crypt-prop-specimens-v1.json`.

Run it with:

```bash
blender library/assemblies/crypt-prop-specimens-v1.blend --background \
  --python scripts/export_crypt_prop_specimens.py
```

- [ ] **Step 7: Syntax-check, run unit tests, and commit tooling**

```bash
python3 -m py_compile scripts/validate_crypt_prop_specimens.py
python3 scripts/test_validate_crypt_prop_specimens.py -v
git add scripts/export_crypt_prop_specimens.py \
  scripts/validate_crypt_prop_specimens.py \
  scripts/test_validate_crypt_prop_specimens.py
git commit -m "test(assets): define crypt specimen export contract (${ASSET_ISSUE_NUMBER})"
```

## Task 3: Hand-Assemble and Export the Three Candidates

**Files:**
- Create: `rpg-game-assets/library/assemblies/crypt-prop-specimens-v1.blend`
- Create: `rpg-game-assets/library/assemblies/crypt-prop-specimens-v1.json`
- Create: three final GLBs listed in the File Map
- Create: `rpg-game-assets/evidence/crypt-prop-specimens-v1/README.md`
- Create: accepted and rejected-candidate PNGs under the same evidence directory
- Create: `rpg-game-assets/evidence/crypt-prop-specimens-v1/verification.json`

**Interfaces:**
- Consumes: Task 2 exporter/validator and exact source GLBs from `library/polygon-dungeon/`.
- Produces: three candidate artifacts whose names and hashes Task 4 promotes and Task 5 consumes.

- [ ] **Step 1: Create the private Blender scene and exact export collections**

Open Blender 5.x with a blank scene. Create:

- `EXPORT_Crypt_Skeleton_Cage_01`
- `EXPORT_Crypt_Skeleton_Table_01`
- `EXPORT_Crypt_Rug_01`
- scratch collections `REVIEW_Rug_01`, `REVIEW_Rug_02`, `REVIEW_Rug_03`

Import the source GLBs from the provider worktree. Rename top-level source objects so the final nodes remain legible:

- `Cage__SM_Prop_Toture_Cage_01`
- `Skeleton__SM_Prop_Skeleton_Cage_01`
- `Table__SM_Prop_Table_01`
- `Skeleton__SM_Prop_Skeleton_Table_01`
- `Rug__SM_Prop_Rug_01`, `_02`, or `_03` for the selected rug

Set each top-level object's custom `source_path` to the exact corresponding
repo-relative path: the two cage paths, skeleton-table pose path plus selected
table-support path, and selected rug path listed in Task 2's `EXPECTED`
contract.

- [ ] **Step 2: Assemble the cage specimen by eye**

Keep the cage support grounded. Translate/rotate the skeleton until it is visibly contained from front, three-quarter, and top views. Do not scale either component to compensate for a bad camera. If a deliberately suspended arrangement is used, include a visible support/mount in this same final GLB; otherwise keep the assembly floor-standing.

- [ ] **Step 3: Assemble the table specimen by eye**

Use `SM_Prop_Table_01` first. Position `SM_Prop_Skeleton_Table_01` so the torso/arms are visibly supported and the feet/limbs do not create destructive intersections. If another same-pack table is materially better, render both candidate combinations and record the rejected one in the evidence README before switching the export collection.

- [ ] **Step 4: Compare all three rugs at measured scale**

First render the unmodified source candidates with one repeatable command:

```bash
mkdir -p evidence/crypt-prop-specimens-v1/rug-candidates
blender --background --python scripts/render_neutral_views.py -- \
  --out-dir evidence/crypt-prop-specimens-v1/rug-candidates \
  library/polygon-dungeon/SM_Prop_Rug_01.glb \
  library/polygon-dungeon/SM_Prop_Rug_02.glb \
  library/polygon-dungeon/SM_Prop_Rug_03.glb
```

Place the same three source GLBs unscaled in the scratch review collections and
render a common tactical three-quarter view from the `.blend`. Select exactly
one variant that lies flat and fits the 12×7 showcase without a client
multiplier. Move only that source object into `EXPORT_Crypt_Rug_01`; retain all
12 neutral source renders and the common-camera comparison.

- [ ] **Step 5: Normalize default facing, then save and export**

Rotate each completed export collection as a whole so its approved default
visual forward is local `+Z` before transforms are recorded; for the rug, `+Z`
is its long-axis direction. Save as
`library/assemblies/crypt-prop-specimens-v1.blend`, then run:

```bash
blender library/assemblies/crypt-prop-specimens-v1.blend --background \
  --python scripts/export_crypt_prop_specimens.py
```

Expected: three GLBs and one contract-ordered `library/assemblies/crypt-prop-specimens-v1.json` record.

- [ ] **Step 6: Render neutral evidence for every accepted artifact**

```bash
mkdir -p evidence/crypt-prop-specimens-v1
blender --background --python scripts/render_neutral_views.py -- \
  --out-dir evidence/crypt-prop-specimens-v1 \
  harness/models/synty/props/Crypt_Skeleton_Cage_01.glb \
  harness/models/synty/props/Crypt_Skeleton_Table_01.glb \
  harness/models/synty/props/Crypt_Rug_01.glb
```

Expected: 12 accepted PNGs with `_front`, `_side`, `_threequarter`, and `_top` suffixes.

- [ ] **Step 7: Write the evidence README from observed facts**

Use these exact headings and populate them from the generated record/render output, not memory:

```md
# Crypt prop specimens v1

## Source and license boundary
## Skeleton cage — components, measured bounds, pivot, and visual read
## Skeleton at table — components, measured bounds, pivot, and visual read
## Rug candidate comparison — 01 vs 02 vs 03 and selected source
## Rejected candidates
## Neutral evidence index
## Validation commands and output
## Candidate builder/game checkpoint
```

The README must state that builder/game images live in public web evidence but licensed Blender/GLB sources remain private here.

- [ ] **Step 8: Generate and byte-check the deterministic provider report**

```bash
python3 scripts/validate_crypt_prop_specimens.py \
  --record library/assemblies/crypt-prop-specimens-v1.json \
  --output evidence/crypt-prop-specimens-v1/verification.json
python3 scripts/validate_crypt_prop_specimens.py \
  --record library/assemblies/crypt-prop-specimens-v1.json \
  --output evidence/crypt-prop-specimens-v1/verification.json --check
```

Expected: both exit 0; report lists exactly three specimens and three SHA-256 values.

- [ ] **Step 9: Inspect the candidates before catalog promotion**

```bash
python3 - <<'PY'
import json
p = json.load(open('evidence/crypt-prop-specimens-v1/verification.json'))
for item in p['specimens']:
    print(item['id'], item['artifact']['sha256'], item['bounds'], item['meshStats'])
PY
```

Expected: finite grounded bounds, no animations, at least one material/texture per specimen. Record any existing budget warning in the README with an explicit keep/rework decision.

- [ ] **Step 10: Commit the private candidate checkpoint**

```bash
git add library/assemblies harness/models/synty/props/Crypt_*.glb \
  evidence/crypt-prop-specimens-v1
git commit -m "asset: assemble crypt prop specimens (${ASSET_ISSUE_NUMBER})"
```

## Task 4: Promote the Approved Provider Contract and Open Its PR

**Files:**
- Modify/delete the provider manifest, stats, inventory, role-map, and superseded files listed in the File Map.
- Test: `scripts/test_validate_crypt_prop_specimens.py`

**Interfaces:**
- Consumes: Task 3 candidate GLBs and verification record.
- Produces: a complete staged Synty provider tree where each target ref has exactly one approved artifact.

- [ ] **Step 1: Add the failing real-role-map promotion test**

Add:

```py
class RealRoleMapPromotionTests(unittest.TestCase):
    def test_three_builder_keys_resolve_only_to_finished_specimens(self):
        role_map = json.loads((REPO_ROOT / "library/prop-role-map.json").read_text())
        by_key = {}
        for role in ("obstacle", "cover", "decor"):
            for piece in role_map[role]["pieces"]:
                if piece.get("key") in validate.TARGET_REFS:
                    by_key.setdefault(piece["key"], []).append(piece["name"])
        self.assertEqual({
            "dnd5e:props:skeleton-cage": ["Crypt_Skeleton_Cage_01"],
            "dnd5e:props:skeleton-table": ["Crypt_Skeleton_Table_01"],
            "dnd5e:props:rug": ["Crypt_Rug_01"],
        }, by_key)
```

- [ ] **Step 2: Run the role-map test and verify red**

Run: `python3 scripts/test_validate_crypt_prop_specimens.py -v`

Expected: FAIL showing old Synty names and three rug variants.

- [ ] **Step 3: Replace the three role-map families with truthful entries**

Use one entry per key:

```json
{ "name": "Crypt_Skeleton_Cage_01", "key": "dnd5e:props:skeleton-cage", "pack": "assembly", "themes": ["crypt"], "notes": "rpg-project#275. Hand-assembled complete cage plus contained skeleton; private source/provenance: library/assemblies/crypt-prop-specimens-v1.json. Builder-ready only after neutral and builder/game review." }
{ "name": "Crypt_Skeleton_Table_01", "key": "dnd5e:props:skeleton-table", "pack": "assembly", "themes": ["crypt"], "blocksLoSOverride": false, "notes": "rpg-project#275. Hand-assembled table plus visibly supported skeleton; private source/provenance: library/assemblies/crypt-prop-specimens-v1.json." }
{ "name": "Crypt_Rug_01", "key": "dnd5e:props:rug", "pack": "assembly", "themes": ["crypt", "library", "dungeon"], "blocksLoSOverride": false, "notes": "rpg-project#275. One measured, visually approved rug; intended size is baked into the artifact and requires no consumer renderScale." }
```

Keep the cage under `obstacle`, table under `cover`, rug under `decor`. Remove only the old entries carrying these three keys.

- [ ] **Step 4: Remove superseded harness copies and regenerate provider outputs**

```bash
rm harness/models/synty/props/SM_Prop_Skeleton_Cage_01.glb \
   harness/models/synty/props/SM_Prop_Skeleton_Table_01.glb \
   harness/models/synty/props/SM_Prop_Rug_01.glb \
   harness/models/synty/props/SM_Prop_Rug_02.glb \
   harness/models/synty/props/SM_Prop_Rug_03.glb
python3 scripts/build_prop_manifest.py
python3 scripts/build_mesh_stats.py 2> /tmp/crypt-prop-mesh-warnings.txt
python3 scripts/build_synty_complete_inventory.py
```

Expected: generated manifest has one piece for each target key; complete inventory contains all three `Crypt_*.glb` files and none of the five removed harness files.

- [ ] **Step 5: Run provider gates**

```bash
python3 scripts/test_validate_crypt_prop_specimens.py -v
python3 scripts/validate_crypt_prop_specimens.py \
  --record library/assemblies/crypt-prop-specimens-v1.json \
  --output evidence/crypt-prop-specimens-v1/verification.json --check
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/verify_web_asset_stage.py --verify-only \
  --report /tmp/crypt-prop-provider-report.json
python3 -m unittest discover -s scripts -p 'test_*.py'
```

Expected: all non-skipped tests pass; stage report's inventory includes the three exact artifact hashes.

- [ ] **Step 6: Review warnings and generated diffs**

```bash
cat /tmp/crypt-prop-mesh-warnings.txt
git diff --check
git status --short
git diff -- library/prop-role-map.json harness/models/synty/props/manifest.json
```

Expected: no unexplained warning, no role change outside the three keys, and no licensed file outside the private repository.

- [ ] **Step 7: Commit provider promotion**

```bash
git add library/prop-role-map.json \
  harness/models/synty/props \
  harness/models/synty/mesh-stats.json \
  harness/catalogs/synty-complete-inventory.json \
  evidence/crypt-prop-specimens-v1/verification.json
git commit -m "asset: promote builder-ready crypt props (${ASSET_ISSUE_NUMBER})"
```

- [ ] **Step 8: Push and open the provider PR as candidate-gated**

```bash
git push -u origin "asset/${ASSET_ISSUE_NUMBER}-crypt-prop-specimens"
gh pr create --repo KirkDiggler/rpg-game-assets --base main \
  --head "asset/${ASSET_ISSUE_NUMBER}-crypt-prop-specimens" \
  --title 'Assemble builder-ready crypt prop specimens' \
  --body "$(cat <<EOF
## Summary
- hand-assemble complete skeleton cage and skeleton-at-table specimens
- select and export one measured rug with no consumer scale repair
- preserve private Blender/provenance records and neutral evidence
- replace the three builder keys with one approved artifact each

## Verification
- crypt specimen validator + deterministic report
- prop manifest, mesh stats, complete inventory
- exact provider stage verification
- full Python unittest discovery

## Visual gate
Candidate only until Kirk approves the builder and playable views from the linked web branch.

Closes #${ASSET_ISSUE_NUMBER}
Design: KirkDiggler/rpg-project#275

— assets agent, on behalf of KirkDiggler
EOF
)"
```

Do not merge yet.

## Task 5: Consume Candidates in the Web and Run the Visual Checkpoint

**Files:**
- Modify: `src/components/hex-grid/propManifest.ts:45-85,435-462,697-734`
- Modify: `src/components/hex-grid/propManifest.test.ts:46-84`
- Modify: `src/components/hex-grid/PropModel.tsx:42-65,145-160`
- Modify: `src/components/hex-grid/PropModel.test.tsx:133-162`
- Replace: three thumbnail PNGs
- Create: `docs/evidence/275-crypt-prop-specimens/README.md`, `before-builder.png`, `candidate-builder.png`

**Interfaces:**
- Consumes: Task 4 provider worktree through `RPG_GAME_ASSETS_PATH`; Task 1 shared showcase/render leaves.
- Produces: one exact `PropVariant` per target ref, no `renderScale` API, candidate visual evidence, and Kirk's provider promotion verdict.

- [ ] **Step 1: Write the failing exact-artifact manifest test**

Add:

```ts
it('maps the crypt specimen keys one-to-one to approved assembly artifacts', () => {
  const expected = {
    'dnd5e:props:skeleton-cage': {
      name: 'Crypt_Skeleton_Cage_01',
      file: 'props/Crypt_Skeleton_Cage_01.glb',
      role: 'obstacle',
      blocksLoS: true,
    },
    'dnd5e:props:skeleton-table': {
      name: 'Crypt_Skeleton_Table_01',
      file: 'props/Crypt_Skeleton_Table_01.glb',
      role: 'cover',
      blocksLoS: false,
    },
    'dnd5e:props:rug': {
      name: 'Crypt_Rug_01',
      file: 'props/Crypt_Rug_01.glb',
      role: 'decor',
      blocksLoS: false,
    },
  } as const;
  for (const [key, contract] of Object.entries(expected)) {
    expect(PROP_KEYS[key]).toHaveLength(1);
    expect(PROP_KEYS[key]?.[0]).toMatchObject(contract);
    expect(Number.isInteger(PROP_KEYS[key]?.[0]?.footprintHexes)).toBe(true);
    expect(PROP_KEYS[key]?.[0]?.footprintHexes).toBeGreaterThan(0);
    expect(PROP_KEYS[key]?.[0]).not.toHaveProperty('renderScale');
  }
});
```

- [ ] **Step 2: Run the manifest test and verify red**

Run: `npm run test:run -- src/components/hex-grid/propManifest.test.ts`

Expected: FAIL on old source filenames, three rug variants, and `renderScale`.

- [ ] **Step 3: Capture the broken baseline before changing the web mirror**

Sync the still-current provider `main`, start this fixture-enabled web branch,
and capture the known-bad state:

```bash
RPG_GAME_ASSETS_PATH=/home/kirk/game-dev/rpg-game-assets npm run assets:sync
mkdir -p docs/evidence/275-crypt-prop-specimens
VITE_API_HOST=http://localhost:8081 npm run dev -- --port 3012 \
  > /tmp/crypt-props-vite.log 2>&1 &
echo $! > /tmp/crypt-props-vite.pid
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  'http://127.0.0.1:3012/?concept=dungeon-builder&authorFixture=crypt-props' \
  docs/evidence/275-crypt-prop-specimens/before-builder.png 6000 1600 900
```

Expected: the image shows the cage skeleton without its cage, the unsupported
table skeleton, and the current bad rug.

- [ ] **Step 4: Update the web mirror from the provider generated manifest**

For the three target keys only, copy exact `name`, `file`, `role`, `footprintHexes`, and `blocksLoS` from the provider's `harness/models/synty/props/manifest.json`. Do not copy private source paths or evidence metadata.

Remove from `PropVariant`:

```ts
renderScale?: number;
```

Remove the rug-specific comment block and remove `renderScale: 2` from every variant. Reduce `dnd5e:props:rug` to the single generated `Crypt_Rug_01` entry.

- [ ] **Step 5: Remove the renderer repair lever**

Change `PropModel.tsx` to:

```tsx
<group
  position={position}
  rotation={[0, rotationY, 0]}
  scale={SYNTY_SCALE}
>
```

Delete the `PropModel renderScale` test block. Retain the existing test proving an ordinary prop renders at `SYNTY_SCALE`; rename it to `renders every prop at the shared SYNTY_SCALE`.

- [ ] **Step 6: Run manifest and renderer tests**

Run:

```bash
npm run test:run -- \
  src/components/hex-grid/propManifest.test.ts \
  src/components/hex-grid/PropModel.test.tsx \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/author/fixtures/cryptPropShowcase.test.ts
```

Expected: all pass; the three exact paths are asserted and no `renderScale` symbol remains under `src/`.

- [ ] **Step 7: Sync the unmerged provider candidate explicitly**

From the web worktree:

```bash
ASSET_WT="/home/kirk/.pi/worktrees/rpg-game-assets/${ASSET_ISSUE_NUMBER}-crypt-props"
RPG_GAME_ASSETS_PATH="$ASSET_WT" npm run assets:sync
for file in Crypt_Skeleton_Cage_01 Crypt_Skeleton_Table_01 Crypt_Rug_01; do
  test -f "public/models/synty/props/${file}.glb"
done
```

Expected: script prints `Using explicit rpg-game-assets source`; no clone/pull mutates the provider branch. Hard-reload the browser to evict cached GLBs.

- [ ] **Step 8: Bake truthful thumbnails from the exact candidate files**

From the web worktree while Vite remains on port 3012:

```bash
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  'http://127.0.0.1:3012/?thumbGlb=/models/synty/props/Crypt_Skeleton_Cage_01.glb' \
  src/author/thumbs/skeleton-cage.png 4000 128 128
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  'http://127.0.0.1:3012/?thumbGlb=/models/synty/props/Crypt_Skeleton_Table_01.glb' \
  src/author/thumbs/skeleton-table.png 4000 128 128
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  'http://127.0.0.1:3012/?thumbGlb=/models/synty/props/Crypt_Rug_01.glb' \
  src/author/thumbs/rug.png 4000 128 128
```

- [ ] **Step 9: Capture candidate builder evidence**

```bash
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  'http://127.0.0.1:3012/?concept=dungeon-builder&authorFixture=crypt-props' \
  docs/evidence/275-crypt-prop-specimens/candidate-builder.png 6000 1600 900
```

- [ ] **Step 10: Start the isolated lab stack and load the candidate dungeon**

```bash
cd /home/kirk/game-dev/rpg-deployment
docker compose -f docker-compose.local-dev.yml \
  -f docker-compose.local-lab.yml up -d rpg-api-lab envoy-lab
```

The Vite process from Step 3 already targets `http://localhost:8081`. In the
Concepts builder, Download the showcase YAML. Open the real authoring route
against the same Vite server, Load that YAML, Save, and Save & Play with a
selected character. Hard-reload after every asset sync.

- [ ] **Step 11: Stop for Kirk's candidate verdict**

Kirk reviews:

1. cage contains the skeleton and has intentional support;
2. body is visibly supported by the table;
3. rug reads flat and room-scaled;
4. all three match between builder preview and playable tactical camera; and
5. current fixed scene lights are unchanged between views.

If any item is rejected, return to Task 3 and re-export from Blender. Do not apply a web transform.

- [ ] **Step 12: Commit candidate consumer mapping and evidence**

After approval:

```bash
npm run format
npm run test:run -- \
  src/components/hex-grid/propManifest.test.ts \
  src/components/hex-grid/PropModel.test.tsx \
  src/components/session/AtlasPropModel.test.tsx \
  src/components/session/DungeonSceneLights.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/author/fixtures/cryptPropShowcase.test.ts \
  src/author/preview3d/DungeonPreview3D.test.ts

git add src/components/hex-grid src/components/session src/author \
  docs/evidence/275-crypt-prop-specimens
git commit -m "fix(assets): consume complete crypt prop specimens (${WEB_ISSUE_NUMBER})"
```

- [ ] **Step 13: Kirk merges the provider PR**

Before merge, verify provider CI/review and candidate evidence are linked in the provider PR. Kirk merges; record:

```bash
ASSET_PR_NUMBER=$(gh pr list --repo KirkDiggler/rpg-game-assets \
  --head "asset/${ASSET_ISSUE_NUMBER}-crypt-prop-specimens" --json number --jq '.[0].number')
ASSET_MERGE_SHA=$(gh pr view "$ASSET_PR_NUMBER" --repo KirkDiggler/rpg-game-assets \
  --json mergeCommit --jq .mergeCommit.oid)
printf 'ASSET_PR_NUMBER=%s\nASSET_MERGE_SHA=%s\n' "$ASSET_PR_NUMBER" "$ASSET_MERGE_SHA"
```

## Task 6: Pin the Merged Provider and Prove the Real Path

**Files:**
- Modify: `rpg-dnd5e-web/docs/evidence/275-crypt-prop-specimens/README.md`
- Add: `final-builder.png`, `final-game.png`, and any focused close-ups in the same directory
- Modify if final evidence finds drift: only files already owned by Tasks 1/5; return to the owning task for behavioral changes.

**Interfaces:**
- Consumes: exact merged provider PR from Task 5 and approved web mapping.
- Produces: byte-identity proof, full web gate, final visual approval, and merge-ready web PR.

- [ ] **Step 1: Resolve and sync from an isolated exact-merge checkout**

```bash
ASSET_PR_NUMBER=$(gh pr list --repo KirkDiggler/rpg-game-assets \
  --head "asset/${ASSET_ISSUE_NUMBER}-crypt-prop-specimens" --state merged \
  --json number --jq '.[0].number')
ASSET_MERGE_SHA=$(gh pr view "$ASSET_PR_NUMBER" --repo KirkDiggler/rpg-game-assets \
  --json mergeCommit --jq .mergeCommit.oid)
MERGED_ASSET_WT="/home/kirk/.pi/worktrees/rpg-game-assets/${ASSET_ISSUE_NUMBER}-crypt-props-merged"
git -C /home/kirk/game-dev/rpg-game-assets fetch origin
git -C /home/kirk/game-dev/rpg-game-assets worktree add --detach \
  "$MERGED_ASSET_WT" "$ASSET_MERGE_SHA"
cd "/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE_NUMBER}-crypt-props"
RPG_GAME_ASSETS_PATH="$MERGED_ASSET_WT" npm run assets:sync
```

Expected: explicit source message; detached provider worktree HEAD equals the
recorded merge SHA; no shared primary checkout is mutated.

- [ ] **Step 2: Prove byte identity for all three consumer files**

```bash
MERGED_ASSET_WT="/home/kirk/.pi/worktrees/rpg-game-assets/${ASSET_ISSUE_NUMBER}-crypt-props-merged"
for file in Crypt_Skeleton_Cage_01 Crypt_Skeleton_Table_01 Crypt_Rug_01; do
  provider="${MERGED_ASSET_WT}/harness/models/synty/props/${file}.glb"
  consumer="public/models/synty/props/${file}.glb"
  test "$(sha256sum "$provider" | cut -d' ' -f1)" = \
       "$(sha256sum "$consumer" | cut -d' ' -f1)"
  sha256sum "$consumer"
done
```

Copy the printed hashes and `ASSET_MERGE_SHA` into the evidence README.

- [ ] **Step 3: Complete the evidence README**

Use these exact headings:

```md
# Builder-ready crypt prop specimens — web evidence

## Provider authority — PR, merge SHA, inventory digest, artifact hashes
## Before — floating cage skeleton, unsupported table skeleton, broken rug
## Candidate checkpoint — explicit local provider source
## Builder preview — final exact merged provider
## Playable dungeon — final exact merged provider
## Preview/game identity — shared AtlasPropModel, PropModel, and DungeonSceneLights
## Save/reopen — ref, facing, and offset preserved
## Commands and test output
## Kirk verdict
```

- [ ] **Step 4: Run the full web gate**

```bash
npm run ci-check
```

Expected: formatting, lint, typecheck, build, and the full Vitest suite all pass.

- [ ] **Step 5: Re-run the final builder capture**

```bash
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  'http://127.0.0.1:3012/?concept=dungeon-builder&authorFixture=crypt-props' \
  docs/evidence/275-crypt-prop-specimens/final-builder.png 6000 1600 900
```

Expected: no browser console errors; exact merged assets visible.

- [ ] **Step 6: Re-run the real Save & Play path**

Against the lab API, Load the downloaded showcase YAML into the real authoring route, Save, reopen it from the dungeon picker, verify the three placements retain ref/facing/offset, then Save & Play. Capture `final-game.png` from the tactical camera using `domcontentloaded` rather than `networkidle` because the session stream remains open.

- [ ] **Step 7: Stop for Kirk's final verdict**

Kirk compares `final-builder.png` and `final-game.png` and approves all three exact merged artifacts. A rejection returns to Task 3; no web repair is allowed.

- [ ] **Step 8: Commit final evidence and rerun the gate**

```bash
git add docs/evidence/275-crypt-prop-specimens
git commit -m "docs(assets): record crypt prop real-path evidence (${WEB_ISSUE_NUMBER})"
npm run ci-check
git status --short --branch
```

Expected: full gate passes and worktree is clean/ahead only.

- [ ] **Step 9: Push and open the web PR against `dev`**

```bash
ASSET_PR_NUMBER=$(gh pr list --repo KirkDiggler/rpg-game-assets \
  --head "asset/${ASSET_ISSUE_NUMBER}-crypt-prop-specimens" --state merged \
  --json number --jq '.[0].number')
ASSET_MERGE_SHA=$(gh pr view "$ASSET_PR_NUMBER" --repo KirkDiggler/rpg-game-assets \
  --json mergeCommit --jq .mergeCommit.oid)
git push -u origin "fix/${WEB_ISSUE_NUMBER}-crypt-prop-specimens"
gh pr create --repo KirkDiggler/rpg-dnd5e-web --base dev \
  --head "fix/${WEB_ISSUE_NUMBER}-crypt-prop-specimens" \
  --title 'Consume approved crypt prop specimens' \
  --body "$(cat <<EOF
## Summary
- map cage, table, and rug refs to one approved provider artifact each
- remove the rug renderScale repair lever
- share atlas-prop and fixed scene-light leaves between builder and game
- add the reusable crypt prop showcase and exact-pinned visual evidence

## Provider
- rpg-game-assets PR #${ASSET_PR_NUMBER}
- merge: ${ASSET_MERGE_SHA}

## Verification
- npm run ci-check
- builder preview + real Save & Play evidence
- Kirk visual approval

Closes #${WEB_ISSUE_NUMBER}
Design: KirkDiggler/rpg-project#275

— assets agent, on behalf of KirkDiggler
EOF
)"
```

- [ ] **Step 10: Complete review and merge in dependency order**

Request one Copilot review. For every inline finding, verify validity, fix valid findings with focused tests, reply with action+rationale, rerun `npm run ci-check`, and push. Kirk merges the web PR only after the provider merge and final visual verdict.

## Task 7: Reconcile the Journey Record and Retire Stale Trackers

**Files:**
- Modify: `rpg-project/ideas/dungeon-builder/crypt-prop-specimens.md` — append landed ledger with exact PRs, merge SHAs, asset hashes, evidence path, and Kirk verdict.
- Modify: `rpg-project/ideas/dungeon-builder/crypt-prop-specimens-plan.md` — check completed boxes only after command/evidence verification.

**Interfaces:**
- Consumes: merged provider/web PRs and final evidence.
- Produces: reconciled Project 19 state; #169 remains open for lighting and remaining builder work.

- [ ] **Step 1: Verify both implementation issues are closed by merged PRs**

```bash
gh issue view "$ASSET_ISSUE_NUMBER" --repo KirkDiggler/rpg-game-assets \
  --json state,closedByPullRequestsReferences,url
gh issue view "$WEB_ISSUE_NUMBER" --repo KirkDiggler/rpg-dnd5e-web \
  --json state,closedByPullRequestsReferences,url
```

Expected: both CLOSED and each names its own merged PR.

- [ ] **Step 2: Append the landed ledger to the design**

Resolve the web merge date and use it in the heading:

```bash
WEB_PR_NUMBER=$(gh pr list --repo KirkDiggler/rpg-dnd5e-web \
  --head "fix/${WEB_ISSUE_NUMBER}-crypt-prop-specimens" --state merged \
  --json number --jq '.[0].number')
LANDED_DATE=$(gh pr view "$WEB_PR_NUMBER" --repo KirkDiggler/rpg-dnd5e-web \
  --json mergedAt --jq '.mergedAt[0:10]')
printf '## Landed — %s\n' "$LANDED_DATE"
```

Record exact values under that printed heading:

- provider issue/PR and merge SHA;
- three artifact SHA-256 values and provider inventory tree digest;
- web issue/PR and merge SHA;
- `docs/evidence/275-crypt-prop-specimens/README.md`;
- final `npm run ci-check` counts; and
- Kirk's exact visual verdict.

- [ ] **Step 3: Close the stale trackers with canonical links**

```bash
gh issue close 132 --repo KirkDiggler/rpg-project --reason 'not planned' --comment \
"Superseded by journey #169 and the approved specimen flow in #275 / PR #276. The first complete cage, table, and rug landed through the linked provider/web slices and real-path evidence. The Synty-bar outcome is preserved; the stale implementation arc is retired.

— cross-team agent, on behalf of KirkDiggler"

gh issue close 35 --repo KirkDiggler/rpg-game-assets --reason 'not planned' --comment \
"Superseded by rpg-project#275 and the merged builder-ready specimen provider slice. This mixed batch is not implemented as written; any remaining desired asset earns a fresh visually gated slice when selected.

— assets agent, on behalf of KirkDiggler"

gh issue close 642 --repo KirkDiggler/rpg-dnd5e-web --reason completed --comment \
"Resolved by the merged exact-provider rug specimen and builder/playable evidence from rpg-project#275. The web-only renderScale repair is gone.

— assets agent, on behalf of KirkDiggler"
```

- [ ] **Step 4: Set the two merged implementation slices Done**

```bash
FIELDS=$(mktemp)
gh project field-list 19 --owner KirkDiggler --format json > "$FIELDS" || {
  echo 'Project 19 unavailable'; exit 1;
}
PROJECT_ID=$(gh project view 19 --owner KirkDiggler --format json --jq .id)
STATUS_FIELD=$(jq -er '.fields[] | select(.name == "Status") | .id' "$FIELDS")
DONE_OPTION=$(jq -er '.fields[] | select(.name == "Status") | .options[] | select(.name == "Done") | .id' "$FIELDS")

set_done() {
  issue_url=$1
  item_id=$(gh project item-list 19 --owner KirkDiggler --format json --limit 1000 |
    jq -er --arg u "$issue_url" '.items[] | select(.content.url == $u) | .id')
  gh project item-edit --id "$item_id" --project-id "$PROJECT_ID" \
    --field-id "$STATUS_FIELD" --single-select-option-id "$DONE_OPTION" >/dev/null
}
set_done "$ASSET_ISSUE_URL"
set_done "$WEB_ISSUE_URL"
```

Keep Decide slice #275 `In Review` until PR #276 merges. Keep journey #169 `In
Progress`; authored lighting remains its next visual-fidelity lane.

- [ ] **Step 5: Verify and commit the final design ledger**

```bash
cd /home/kirk/.pi/worktrees/rpg-project/275-crypt-prop-specimens
git diff --check
rg -n 'Landed|rpg-game-assets|rpg-dnd5e-web|sha256|Kirk' \
  ideas/dungeon-builder/crypt-prop-specimens.md
git add ideas/dungeon-builder/crypt-prop-specimens.md \
  ideas/dungeon-builder/crypt-prop-specimens-plan.md
git commit -m 'docs(dungeon-builder): record crypt prop specimen delivery (#275)'
git push
```

- [ ] **Step 6: Kirk merges rpg-project PR #276**

Before merge, confirm PR #276 contains the approved design, this plan, and
landed ledger; the implementation slice fields agree with merged repository
evidence; #169 remains open.

- [ ] **Step 7: Close the Decide slice and verify the journey remains active**

```bash
gh issue close 275 --repo KirkDiggler/rpg-project --reason completed --comment \
"Design, implementation plan, and landed provider/web evidence are recorded in merged PR #276. Journey #169 remains active for authored lighting and its remaining builder outcomes.

— cross-team agent, on behalf of KirkDiggler"
FIELDS=$(mktemp)
gh project field-list 19 --owner KirkDiggler --format json > "$FIELDS" || {
  echo 'Project 19 unavailable'; exit 1;
}
PROJECT_ID=$(gh project view 19 --owner KirkDiggler --format json --jq .id)
STATUS_FIELD=$(jq -er '.fields[] | select(.name == "Status") | .id' "$FIELDS")
DONE_OPTION=$(jq -er '.fields[] | select(.name == "Status") | .options[] | select(.name == "Done") | .id' "$FIELDS")
ITEM_275=$(gh project item-list 19 --owner KirkDiggler --format json --limit 1000 |
  jq -er '.items[] | select(.content.repository == "KirkDiggler/rpg-project" and .content.number == 275) | .id')
gh project item-edit --id "$ITEM_275" --project-id "$PROJECT_ID" \
  --field-id "$STATUS_FIELD" --single-select-option-id "$DONE_OPTION" >/dev/null
gh project item-list 19 --owner KirkDiggler --format json --limit 1000 | jq '
  [.items[] | select(
    (.content.repository == "KirkDiggler/rpg-project" and .content.number == 169) or
    (.content.repository == "KirkDiggler/rpg-project" and .content.number == 275)
  ) | {url:.content.url,status,team,area,initiative,kind}]'
```

Expected: #275 is `Done`; #169 remains `In Progress`.
