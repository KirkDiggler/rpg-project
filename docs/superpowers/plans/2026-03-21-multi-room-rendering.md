# Multi-Room Dungeon Rendering Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render multi-room dungeons as one contiguous map with camera follow and pan navigation.

**Architecture:** Thread `dungeonMap` (from existing `useDungeonMap` hook) through BattleMapPanel -> HexGrid -> InstancedHexTiles, replacing grid-based rendering with tile-map rendering. Add camera lerp on turn start and fix WASD stuck-key bug.

**Tech Stack:** React, Three.js, React Three Fiber, TypeScript, Vitest

**Spec:** `docs/superpowers/specs/2026-03-21-multi-room-rendering-design.md`

**Worktree:** `/home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest`

**Issues:** rpg-dnd5e-web #312, #313, #310

---

## File Map

| File | Action | Responsibility |
|------|--------|----------------|
| `src/utils/hexUtils.ts` | Modify | Add `walkable` param to `findHexPath` |
| `src/utils/hexUtils.test.ts` | Create | Tests for `findHexPath` walkability |
| `src/components/hex-grid/InstancedHexTiles.tsx` | Modify | Accept `floorTiles` map instead of `gridWidth/gridHeight` |
| `src/components/hex-grid/useHexInteraction.ts` | Modify | Accept `floorTiles` instead of `gridWidth/gridHeight` |
| `src/components/hex-grid/useHexInteraction.test.ts` | Modify | Update existing tests for floorTiles-based validation |
| `src/components/hex-grid/HexGrid.tsx` | Modify | Thread floorTiles through, update `isBlocked` |
| `src/components/encounter/BattleMapPanel.tsx` | Modify | Accept `dungeonMap`, extract accumulated state |
| `src/components/LobbyView.tsx` | Modify | Pass `dungeonMap` to BattleMapPanel |
| `src/components/hex-grid/useCameraControls.ts` | Modify | Add `focusTarget` lerp + blur fix |

**Note on pathfinding:** There are two pathfinders — `findHexPath` (greedy, in `hexUtils.ts`, used by LobbyView) and `findPath` (A*, in `hexMath.ts`, used by `useHexInteraction`). Only `findHexPath` needs a `walkable` param (Task 1) because `findPath` already receives `isBlocked` from HexGrid, and after Task 4 `isBlocked` checks `floorTiles.has()`, which gates off-map tiles.

---

### Task 1: Add walkability check to `findHexPath`

The greedy pathfinder in `hexUtils.ts` currently only checks `occupiedPositions`. It needs a `walkable` set so paths can't go off-map. This is a standalone utility change with no component dependencies.

**Files:**
- Modify: `src/utils/hexUtils.ts:76-120` (`findHexPath` function)
- Create: `src/utils/hexUtils.test.ts`

- [ ] **Step 1: Write failing tests for walkability**

```typescript
// src/utils/hexUtils.test.ts
import { describe, expect, it } from 'vitest';
import { cubeKey, findHexPath } from './hexUtils';

describe('findHexPath', () => {
  // Helper to create a walkable set from a list of coords
  function makeWalkable(coords: Array<{ x: number; y: number; z: number }>): Set<string> {
    return new Set(coords.map((c) => cubeKey(c)));
  }

  it('finds path between adjacent walkable hexes', () => {
    const walkable = makeWalkable([
      { x: 0, y: 0, z: 0 },
      { x: 1, y: -1, z: 0 },
    ]);
    const path = findHexPath(
      { x: 0, y: 0, z: 0 },
      { x: 1, y: -1, z: 0 },
      new Set(),
      walkable
    );
    expect(path).toHaveLength(1);
    expect(path[0]).toEqual({ x: 1, y: -1, z: 0 });
  });

  it('rejects path through non-walkable hex', () => {
    // Only start and end are walkable, but they are not adjacent
    const walkable = makeWalkable([
      { x: 0, y: 0, z: 0 },
      { x: 2, y: -2, z: 0 },
    ]);
    const path = findHexPath(
      { x: 0, y: 0, z: 0 },
      { x: 2, y: -2, z: 0 },
      new Set(),
      walkable
    );
    expect(path).toHaveLength(0);
  });

  it('works without walkable param (backward compat)', () => {
    const path = findHexPath(
      { x: 0, y: 0, z: 0 },
      { x: 1, y: -1, z: 0 },
      new Set()
    );
    expect(path).toHaveLength(1);
  });

  it('finds path across room boundary when both rooms are walkable', () => {
    // Room 1: (0,0,0) to (2,-2,0), Room 2: (3,-3,0) to (5,-5,0)
    // with connecting hex (2,-2,0) -> (3,-3,0)
    const walkable = makeWalkable([
      { x: 0, y: 0, z: 0 },
      { x: 1, y: -1, z: 0 },
      { x: 2, y: -2, z: 0 },
      { x: 3, y: -3, z: 0 },
      { x: 4, y: -4, z: 0 },
    ]);
    const path = findHexPath(
      { x: 0, y: 0, z: 0 },
      { x: 4, y: -4, z: 0 },
      new Set(),
      walkable
    );
    expect(path.length).toBeGreaterThan(0);
    expect(path[path.length - 1]).toEqual({ x: 4, y: -4, z: 0 });
  });
});
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx vitest run src/utils/hexUtils.test.ts`
Expected: FAIL — `findHexPath` doesn't accept 4th parameter

- [ ] **Step 3: Add walkable parameter to findHexPath**

In `src/utils/hexUtils.ts`, update the `findHexPath` function signature and neighbor filtering:

```typescript
export function findHexPath(
  from: CubeCoord,
  to: CubeCoord,
  occupiedPositions: Set<string>,
  walkable?: Set<string>
): CubeCoord[] {
  // If already adjacent or same, return direct path
  const dist = hexDistance(from.x, from.y, from.z, to.x, to.y, to.z);
  if (dist <= 1) return [to];

  // Simple straight-line pathing (greedy approach)
  const path: CubeCoord[] = [];
  let current: CubeCoord = { ...from };

  while (hexDistance(current.x, current.y, current.z, to.x, to.y, to.z) > 0) {
    const neighbors = getHexNeighbors(current);

    let best: CubeCoord | null = null;
    let bestDist = Infinity;

    for (const neighbor of neighbors) {
      const key = cubeKey(neighbor);
      if (occupiedPositions.has(key)) continue;
      if (walkable && !walkable.has(key)) continue; // Skip non-walkable
      // ... rest of existing logic unchanged
```

Note: `getHexNeighbors` is a private function in hexUtils.ts. The only change is the one `if (walkable && !walkable.has(key)) continue;` line after the occupied check, plus adding the parameter.

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx vitest run src/utils/hexUtils.test.ts`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/utils/hexUtils.ts src/utils/hexUtils.test.ts
git commit -m "feat(#312): add walkable set param to findHexPath for multi-room pathing"
```

---

### Task 2: Refactor InstancedHexTiles to accept floorTiles map

Replace `gridWidth/gridHeight` grid iteration with `floorTiles: Map<string, AbsoluteFloorTile>` iteration.

**Files:**
- Modify: `src/components/hex-grid/InstancedHexTiles.tsx`

**Key context:**
- `AbsoluteFloorTile` is exported from `src/hooks/useDungeonMap.ts` with `{ x, y, z, roomId }`
- `cubeToWorld` is already imported from `./hexMath`
- Instance count changes from `gridWidth * gridHeight` to `floorTiles.size`
- The `indexToCube` helper function is no longer needed
- `cubeToKey` and `cubesEqual` helper functions stay (used for coloring)

- [ ] **Step 1: Update props interface**

Replace the props interface in `src/components/hex-grid/InstancedHexTiles.tsx`:

```typescript
import type { AbsoluteFloorTile } from '@/hooks/useDungeonMap';

interface InstancedHexTilesProps {
  floorTiles: Map<string, AbsoluteFloorTile>;
  hexSize: number;
  hoveredHex: CubeCoord | null;
  selectedHex: CubeCoord | null;
  doorPositions?: CubeCoord[];
  wallPositions?: CubeCoord[];
}
```

- [ ] **Step 2: Update component body — instance setup**

Replace the instance matrix `useEffect` (lines 126-154). Instead of iterating `gridWidth * gridHeight`, iterate the floorTiles map:

```typescript
export function InstancedHexTiles({
  floorTiles,
  hexSize,
  hoveredHex,
  selectedHex,
  doorPositions = [],
  wallPositions = [],
}: InstancedHexTilesProps) {
  const { invalidate } = useThree();
  const meshRef = useRef<THREE.InstancedMesh>(null);
  const instanceCount = floorTiles.size;

  // ... geometry and material useMemo stay the same ...

  // Initialize instance matrices from floorTiles
  useEffect(() => {
    if (!meshRef.current) return;

    const mesh = meshRef.current;
    const matrix = new THREE.Matrix4();
    const rotation = new THREE.Euler(-Math.PI / 2, 0, 0);
    const quaternion = new THREE.Quaternion().setFromEuler(rotation);
    const scale = new THREE.Vector3(1, 1, 1);

    let instanceIndex = 0;
    for (const [, tile] of floorTiles) {
      const worldPos = cubeToWorld({ x: tile.x, y: tile.y, z: tile.z }, hexSize);
      matrix.compose(
        new THREE.Vector3(worldPos.x, 0, worldPos.z),
        quaternion,
        scale
      );
      mesh.setMatrixAt(instanceIndex, matrix);
      instanceIndex++;
    }

    mesh.instanceMatrix.needsUpdate = true;
    invalidate();
  }, [floorTiles, hexSize, geometry, material, invalidate]);
```

- [ ] **Step 3: Update color effect**

Replace the color `useEffect` (lines 157-209). Iterate floorTiles instead of grid indices:

```typescript
  useEffect(() => {
    if (!meshRef.current) return;

    const mesh = meshRef.current;
    const doorSet = positionsToSet(doorPositions);
    const wallSet = positionsToSet(wallPositions);
    const colors = new Float32Array(instanceCount * 3);

    let instanceIndex = 0;
    for (const [key] of floorTiles) {
      // Parse the key back to cube coord for comparison
      const [tx, ty, tz] = key.split(',').map(Number);
      const cube: CubeCoord = { x: tx, y: ty, z: tz };

      let color: THREE.Color;
      if (cubesEqual(selectedHex, cube)) {
        color = COLORS.selected;
      } else if (cubesEqual(hoveredHex, cube)) {
        color = COLORS.hovered;
      } else if (doorSet.has(key)) {
        color = COLORS.door;
      } else if (wallSet.has(key)) {
        color = COLORS.wall;
      } else {
        color = COLORS.default;
      }

      colors[instanceIndex * 3] = color.r;
      colors[instanceIndex * 3 + 1] = color.g;
      colors[instanceIndex * 3 + 2] = color.b;
      instanceIndex++;
    }

    mesh.instanceColor = new THREE.InstancedBufferAttribute(colors, 3);
    mesh.instanceColor.needsUpdate = true;
    invalidate();
  }, [floorTiles, hoveredHex, selectedHex, doorPositions, wallPositions, instanceCount, invalidate]);
```

- [ ] **Step 4: Remove unused `indexToCube` function**

Delete the `indexToCube` function (lines 63-66) — no longer needed.

- [ ] **Step 5: Verify TypeScript compiles**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx tsc --noEmit 2>&1 | head -30`
Expected: Errors in HexGrid.tsx (still passing `gridWidth/gridHeight`) — this is expected, we'll fix it in Task 4.

- [ ] **Step 6: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/hex-grid/InstancedHexTiles.tsx
git commit -m "feat(#312): refactor InstancedHexTiles to use floorTiles map

Replaces gridWidth/gridHeight grid iteration with absolute tile positions
from the accumulated dungeon map."
```

---

### Task 3: Refactor useHexInteraction to accept floorTiles

Replace `gridWidth/gridHeight` bounds checking with floorTiles map membership.

**Files:**
- Modify: `src/components/hex-grid/useHexInteraction.ts`
- Modify: `src/components/hex-grid/useHexInteraction.test.ts`

**Key context:**
- `isValidHex` currently checks `coord.x >= 0 && coord.x < gridWidth && coord.z >= 0 && coord.z < gridHeight`
- Change to `floorTiles.has(coordKey)` where coordKey = `"x,y,z"`
- The hook is consumed by `HexGrid.tsx` Scene function (line 219)

- [ ] **Step 1: Update existing tests**

In `src/components/hex-grid/useHexInteraction.test.ts`, find all references to `gridWidth` and `gridHeight` in test setup and replace with a `floorTiles` map. Create a helper:

```typescript
import type { AbsoluteFloorTile } from '@/hooks/useDungeonMap';

/** Helper to create a floorTiles map for a rectangular grid */
function makeFloorTiles(width: number, height: number): Map<string, AbsoluteFloorTile> {
  const tiles = new Map<string, AbsoluteFloorTile>();
  for (let z = 0; z < height; z++) {
    for (let x = 0; x < width; x++) {
      const y = -x - z;
      tiles.set(`${x},${y},${z}`, { x, y, z, roomId: 'room-1' });
    }
  }
  return tiles;
}
```

Then replace all `gridWidth: N, gridHeight: M` with `floorTiles: makeFloorTiles(N, M)` in the test props.

- [ ] **Step 2: Run tests to verify they fail**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx vitest run src/components/hex-grid/useHexInteraction.test.ts`
Expected: FAIL — `gridWidth` is no longer in props type

- [ ] **Step 3: Update the hook props and isValidHex**

In `src/components/hex-grid/useHexInteraction.ts`:

Update imports:
```typescript
import type { AbsoluteFloorTile } from '@/hooks/useDungeonMap';
```

Update props interface:
```typescript
export interface UseHexInteractionProps {
  hexSize: number;
  floorTiles: Map<string, AbsoluteFloorTile>;
  onHexClick?: (coord: CubeCoord) => void;
  onHexHover?: (coord: CubeCoord | null) => void;
  entityPosition?: CubeCoord | null;
  movementRemaining?: number;
  isBlocked?: (coord: CubeCoord) => boolean;
  entities?: Map<string, Entity>;
}
```

Update function signature and `isValidHex`:
```typescript
export function useHexInteraction({
  hexSize,
  floorTiles,
  onHexClick,
  onHexHover,
  entityPosition,
  movementRemaining = 0,
  isBlocked,
  entities,
}: UseHexInteractionProps): UseHexInteractionReturn {
  // ...

  const isValidHex = useCallback(
    (coord: CubeCoord): boolean => {
      return floorTiles.has(`${coord.x},${coord.y},${coord.z}`);
    },
    [floorTiles]
  );
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx vitest run src/components/hex-grid/useHexInteraction.test.ts`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/hex-grid/useHexInteraction.ts src/components/hex-grid/useHexInteraction.test.ts
git commit -m "feat(#312): refactor useHexInteraction to use floorTiles map

Replaces gridWidth/gridHeight bounds checking with floorTiles.has() for
multi-room hex validation."
```

---

### Task 4: Update HexGrid to thread floorTiles through

Replace `gridWidth/gridHeight` props with `floorTiles` and accumulated entities/walls/doors. Update `isBlocked`, `gridCenter`, and all child component props.

**Files:**
- Modify: `src/components/hex-grid/HexGrid.tsx`

**Key context:**
- `HexGridProps` currently has `gridWidth`, `gridHeight`, `entities` array, `walls`, `doors`
- Change to `floorTiles: Map<string, AbsoluteFloorTile>`, keep entities/walls/doors as-is
- `gridCenter` changes from `gridWidth/2, gridHeight/2` to bounding box midpoint of all tiles
- `isBlocked` changes from grid bounds check to `floorTiles.has()`
- `useHexInteraction` call changes from `gridWidth, gridHeight` to `floorTiles`
- `InstancedHexTiles` call changes from `gridWidth, gridHeight` to `floorTiles`

- [ ] **Step 1: Update HexGridProps interface**

```typescript
import type { AbsoluteFloorTile } from '@/hooks/useDungeonMap';

export interface HexGridProps {
  floorTiles: Map<string, AbsoluteFloorTile>;
  entities: Array<{
    entityId: string;
    name: string;
    position: { x: number; y: number; z: number };
    type: 'player' | 'monster' | 'obstacle';
  }>;
  // ... rest of existing props stay the same, minus gridWidth/gridHeight ...
}
```

- [ ] **Step 2: Update Scene function — gridCenter calculation**

Replace the `gridCenter` useMemo (lines 129-140):

```typescript
  const gridCenter = useMemo(() => {
    if (floorTiles.size === 0) {
      return new THREE.Vector3(0, 0, 0);
    }
    let minX = Infinity, maxX = -Infinity;
    let minZ = Infinity, maxZ = -Infinity;
    for (const [, tile] of floorTiles) {
      const worldPos = cubeToWorld({ x: tile.x, y: tile.y, z: tile.z }, HEX_SIZE);
      minX = Math.min(minX, worldPos.x);
      maxX = Math.max(maxX, worldPos.x);
      minZ = Math.min(minZ, worldPos.z);
      maxZ = Math.max(maxZ, worldPos.z);
    }
    return new THREE.Vector3((minX + maxX) / 2, 0, (minZ + maxZ) / 2);
  }, [floorTiles]);
```

- [ ] **Step 3: Update isBlocked callback**

Replace the `isBlocked` useCallback (lines 187-208):

```typescript
  const isBlocked = useCallback(
    (coord: CubeCoord) => {
      const key = `${coord.x},${coord.y},${coord.z}`;
      // Not walkable if outside the map
      if (!floorTiles.has(key)) {
        return true;
      }
      // Check for other entities
      return entities.some(
        (entity) =>
          entity.position.x === coord.x &&
          entity.position.y === coord.y &&
          entity.position.z === coord.z &&
          entity.entityId !== currentEntityId
      );
    },
    [entities, currentEntityId, floorTiles]
  );
```

- [ ] **Step 4: Update useHexInteraction call**

Replace `gridWidth, gridHeight` with `floorTiles` in the `useHexInteraction` call (line 219-242):

```typescript
  } = useHexInteraction({
    hexSize: HEX_SIZE,
    floorTiles,
    onHexClick: (coord) => {
      // ... existing click handler unchanged ...
    },
    onHexHover,
    entityPosition: currentEntityPosition,
    movementRemaining,
    isBlocked,
    entities: entitiesMap,
  });
```

- [ ] **Step 5: Update InstancedHexTiles call**

Replace `gridWidth, gridHeight` with `floorTiles` in the JSX (lines 326-334):

```typescript
      <InstancedHexTiles
        floorTiles={floorTiles}
        hexSize={HEX_SIZE}
        hoveredHex={hoveredHex}
        selectedHex={selectedHex}
        doorPositions={doorPositions}
        wallPositions={wallPositions}
      />
```

- [ ] **Step 6: Update Scene function signature**

Remove `gridWidth` and `gridHeight` from the Scene destructuring (line 86-107), add `floorTiles`.

- [ ] **Step 7: Verify TypeScript compiles**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx tsc --noEmit 2>&1 | head -30`
Expected: Errors in BattleMapPanel.tsx (still passing `gridWidth/gridHeight`) — fixed in Task 5.

- [ ] **Step 8: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/hex-grid/HexGrid.tsx
git commit -m "feat(#312): update HexGrid to use floorTiles map

Replaces gridWidth/gridHeight with floorTiles-based rendering, bounds
checking, and grid center calculation."
```

---

### Task 5: Update BattleMapPanel to accept dungeonMap

Wire `DungeonMapState` into the component, extract accumulated state, pass to HexGrid.

**Files:**
- Modify: `src/components/encounter/BattleMapPanel.tsx`

**Key context:**
- Currently receives `room: Room` and passes `room.width`, `room.height`, `room.entities`, `room.walls`
- Will receive `dungeonMap: DungeonMapState` instead
- Extract `floorTiles`, accumulated entities, walls, doors from dungeonMap
- Still needs `room` for current room context (combat state references current room)
- `doors` prop currently comes from LobbyView state — will switch to dungeonMap.doors

- [ ] **Step 1: Update BattleMapPanelProps**

```typescript
import type { DungeonMapState, AbsoluteFloorTile } from '@/hooks/useDungeonMap';
import { EntityType } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/enums_pb';

interface BattleMapPanelProps {
  dungeonMap: DungeonMapState;
  selectedEntity: string | null;
  availableCharacters: Character[];
  allPartyCharacters: Character[];
  encounterId?: string | null;
  combatState?: CombatState | null;
  monsters?: MonsterCombatState[];
  onEntityClick: (entityId: string) => void;
  onCellClick: (coord: CubeCoord) => void;
  onMoveComplete?: (path: CubeCoord[]) => void;
  onAttackComplete?: (targetId: string) => void;
  onHoverChange?: (
    entity: { id: string; type: string; name: string } | null
  ) => void;
  onDoorClick?: (connectionId: string) => void;
  isDoorLoading?: boolean;
}
```

- [ ] **Step 2: Update component body**

Extract accumulated state from dungeonMap and pass to HexGrid:

```typescript
export function BattleMapPanel({
  dungeonMap,
  selectedEntity,
  availableCharacters,
  allPartyCharacters,
  encounterId,
  combatState,
  monsters,
  onEntityClick,
  onCellClick,
  onMoveComplete,
  onAttackComplete,
  onHoverChange,
  onDoorClick,
  isDoorLoading,
}: BattleMapPanelProps) {
  // Build entities array from accumulated dungeonMap entities
  const entities = useMemo(() => {
    return Array.from(dungeonMap.entities.values()).map((entity) => {
      let displayType: 'player' | 'monster' | 'obstacle';
      if (entity.entityType === EntityType.CHARACTER) {
        displayType = 'player';
      } else if (entity.entityType === EntityType.MONSTER) {
        displayType = 'monster';
      } else {
        displayType = 'obstacle';
      }
      return {
        entityId: entity.entityId,
        name:
          allPartyCharacters.find((c) => c.id === entity.entityId)?.name ||
          entity.entityId,
        position: {
          x: entity.position?.x || 0,
          y: entity.position?.y || 0,
          z: entity.position?.z || 0,
        },
        type: displayType,
      };
    });
  }, [dungeonMap.entities, allPartyCharacters]);

  // Convert doors map to array for HexGrid
  const doorsArray = useMemo(
    () => Array.from(dungeonMap.doors.values()),
    [dungeonMap.doors]
  );

  // Collect all walls from dungeonMap
  const walls = dungeonMap.walls;

  return (
    <div
      className="rounded-lg"
      style={{
        backgroundColor: 'var(--card-bg)',
        border: '1px solid var(--border-primary)',
        height: '100%',
      }}
    >
      <HexGrid
        floorTiles={dungeonMap.floorTiles}
        entities={entities}
        selectedEntityId={selectedEntity || undefined}
        onHexClick={onCellClick}
        onEntityClick={onEntityClick}
        encounterId={encounterId}
        combatState={combatState}
        characters={allPartyCharacters}
        monsters={monsters}
        currentEntityId={combatState?.currentTurn?.entityId}
        movementRemaining={
          combatState?.currentTurn
            ? (combatState.currentTurn.movementMax || 30) -
              (combatState.currentTurn.movementUsed || 0)
            : 0
        }
        isPlayerTurn={
          combatState?.currentTurn?.entityId
            ? availableCharacters.some(
                (c) => c.id === combatState.currentTurn?.entityId
              )
            : false
        }
        onMoveComplete={onMoveComplete}
        onAttackComplete={onAttackComplete}
        onHoverChange={onHoverChange}
        doors={doorsArray}
        onDoorClick={onDoorClick}
        isDoorLoading={isDoorLoading}
        walls={walls}
      />
    </div>
  );
}
```

**Important:** Add `useMemo` to the React import at the top of the file: `import { useMemo } from 'react';`

- [ ] **Step 3: Verify TypeScript compiles**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx tsc --noEmit 2>&1 | head -30`
Expected: Errors in LobbyView.tsx (still passing old props) — fixed in Task 6.

- [ ] **Step 4: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/encounter/BattleMapPanel.tsx
git commit -m "feat(#312): update BattleMapPanel to accept dungeonMap

Extracts accumulated entities, doors, walls, and floorTiles from
DungeonMapState for multi-room rendering."
```

---

### Task 6: Wire dungeonMap through LobbyView

Pass `dungeonMap` to BattleMapPanel and update `findHexPath` calls to use walkable set.

**Files:**
- Modify: `src/components/LobbyView.tsx`

**Key context:**
- Line 143-149: `useDungeonMap()` currently destructures only `currentRoom`, `addRoom`, `updateEntities`, `reset`
- Need to also destructure `dungeonMap`
- Line 1992-2010: `BattleMapPanel` call needs to change from `room={room}` to `dungeonMap={dungeonMap}`
- Line 1137: `findHexPath` call needs `walkable` set from `dungeonMap.floorTiles`
- Remove `doors` prop from BattleMapPanel (now comes from dungeonMap)

- [ ] **Step 1: Destructure dungeonMap from useDungeonMap**

Update line 143-149:

```typescript
  const {
    dungeonMap,
    currentRoom: room,
    addRoom: addRoomToMap,
    updateEntities: updateMapEntities,
    reset: resetDungeonMap,
  } = useDungeonMap();
```

- [ ] **Step 2: Create walkable keys set for findHexPath**

**Important:** Add `useMemo` to the React import if not already present (check LobbyView.tsx imports).

Add a memoized walkable set derived from dungeonMap.floorTiles, near the top of the component (after `useDungeonMap`):

```typescript
  // Walkable tile keys for cross-room pathfinding
  const walkableTileKeys = useMemo(
    () => new Set(dungeonMap.floorTiles.keys()),
    [dungeonMap.floorTiles]
  );
```

- [ ] **Step 3: Update findHexPath call**

At line 1137, add the walkable param:

```typescript
      const newSegment = findHexPath(lastPos, clickedCube, occupiedPositions, walkableTileKeys);
```

- [ ] **Step 4: Update BattleMapPanel call**

Replace the BattleMapPanel JSX (lines 1992-2010):

```typescript
              <BattleMapPanel
                dungeonMap={dungeonMap}
                selectedEntity={selectedEntity}
                availableCharacters={availableCharacters}
                allPartyCharacters={Array.from(fullCharactersMap.values())}
                onEntityClick={handleEntityClick}
                onCellClick={handleCellClick}
                encounterId={encounterId}
                combatState={combatState}
                monsters={monsters}
                onMoveComplete={handleMoveComplete}
                onAttackComplete={handleAttackComplete}
                onHoverChange={setHoveredEntity}
                onDoorClick={handleDoorClick}
                isDoorLoading={doorLoading}
              />
```

Note: `doors` and `room` props are removed — they come from `dungeonMap` now.

- [ ] **Step 5: Run TypeScript check**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx tsc --noEmit`
Expected: PASS (all components now aligned)

- [ ] **Step 6: Run full test suite**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx vitest run`
Expected: PASS

- [ ] **Step 7: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/LobbyView.tsx
git commit -m "feat(#312): wire dungeonMap through LobbyView to BattleMapPanel

Passes accumulated dungeon state for multi-room rendering. Adds walkable
set to findHexPath for cross-room pathfinding."
```

---

### Task 7: Fix WASD stuck-key bug

Add `blur` event listener to clear pressed keys when window loses focus.

**Files:**
- Modify: `src/components/hex-grid/useCameraControls.ts`

**Key context:**
- Lines 81-103: keyboard event listeners for WASD/QE
- `keys.current` tracks pressed state but never resets on blur
- Fix: add `blur` listener on `window` in the same `useEffect`

- [ ] **Step 1: Add blur handler**

In `src/components/hex-grid/useCameraControls.ts`, inside the keyboard `useEffect` (lines 81-103), add:

```typescript
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      const key = e.key.toLowerCase();
      if (key in keys.current) {
        keys.current[key as keyof typeof keys.current] = true;
      }
    };

    const handleKeyUp = (e: KeyboardEvent) => {
      const key = e.key.toLowerCase();
      if (key in keys.current) {
        keys.current[key as keyof typeof keys.current] = false;
      }
    };

    const handleBlur = () => {
      keys.current.w = false;
      keys.current.a = false;
      keys.current.s = false;
      keys.current.d = false;
      keys.current.q = false;
      keys.current.e = false;
    };

    window.addEventListener('keydown', handleKeyDown);
    window.addEventListener('keyup', handleKeyUp);
    window.addEventListener('blur', handleBlur);

    return () => {
      window.removeEventListener('keydown', handleKeyDown);
      window.removeEventListener('keyup', handleKeyUp);
      window.removeEventListener('blur', handleBlur);
    };
  }, []);
```

- [ ] **Step 2: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/hex-grid/useCameraControls.ts
git commit -m "fix(#313): clear WASD keys on window blur to prevent stuck movement"
```

---

### Task 8: Add camera auto-center on turn start

Add `focusTarget` prop to `useCameraControls` that lerps camera to a position when it changes.

**Files:**
- Modify: `src/components/hex-grid/useCameraControls.ts`
- Modify: `src/components/hex-grid/HexGrid.tsx`

**Key context:**
- `useCameraControls` uses `useFrame` for WASD movement
- Camera position is computed from `target` + spherical coords (azimuth, polarAngle, distance)
- To center on a character, we move `target` to the character's world position
- The lerp interpolates `target` toward `focusTarget` over ~0.5s
- If user presses WASD during lerp, cancel the lerp

- [ ] **Step 1: Add focusTarget to CameraControlsOptions**

```typescript
interface CameraControlsOptions {
  target: THREE.Vector3;
  polarAngle?: number;
  panSpeed?: number;
  rotateSpeed?: number;
  minZoom?: number;
  maxZoom?: number;
  /** When set, camera lerps target to this position. Cleared on manual pan. */
  focusTarget?: THREE.Vector3 | null;
}
```

- [ ] **Step 2: Add lerp logic to useFrame**

Add refs to track the lerp and update the `useFrame` callback. Uses exponential smoothing (`target.lerp(dest, factor)` each frame with a fixed factor) for smooth deceleration:

```typescript
  // Track lerp target for auto-center
  const lerpTarget = useRef<THREE.Vector3 | null>(null);

  // Update lerp target when focusTarget changes
  useEffect(() => {
    if (focusTarget) {
      lerpTarget.current = focusTarget.clone();
    }
  }, [focusTarget]);

  useFrame((_, delta) => {
    const { w, a, s, d, q, e } = keys.current;

    // If user is panning, cancel any active lerp
    if ((w || a || s || d) && lerpTarget.current) {
      lerpTarget.current = null;
    }

    // Handle lerp to focus target (exponential smoothing)
    if (lerpTarget.current) {
      // factor = 1 - e^(-speed * delta), gives frame-rate-independent smoothing
      const factor = 1 - Math.pow(0.001, delta);
      target.lerp(lerpTarget.current, factor);
      updateCamera();
      invalidate();

      // Snap when close enough
      if (target.distanceTo(lerpTarget.current) < 0.01) {
        target.copy(lerpTarget.current);
        lerpTarget.current = null;
        updateCamera();
      }
      // Still process rotation during lerp
      if (q) { azimuth.current += rotateSpeed; updateCamera(); invalidate(); }
      if (e) { azimuth.current -= rotateSpeed; updateCamera(); invalidate(); }
      return;
    }

    // Normal WASD handling (existing code)
    if (!w && !a && !s && !d && !q && !e) return;

    // ... existing pan/rotate code unchanged ...
  });
```

- [ ] **Step 3: Pass focusTarget from HexGrid Scene**

In `src/components/hex-grid/HexGrid.tsx`, compute a `focusTarget` that only changes when the turn changes (not on every position update during a turn). Use `currentEntityId` as the trigger, and compute position at that moment:

```typescript
  // Compute camera focus target on turn change only (not on movement during turn)
  const focusTarget = useMemo(() => {
    if (!currentEntityId) return null;
    const entity = entities.find((e) => e.entityId === currentEntityId);
    if (!entity) return null;
    const worldPos = cubeToWorld(entity.position, HEX_SIZE);
    return new THREE.Vector3(worldPos.x, 0, worldPos.z);
    // eslint-disable-next-line react-hooks/exhaustive-deps -- intentionally only trigger on turn change
  }, [currentEntityId]);

  useCameraControls({
    target: gridCenter,
    polarAngle: Math.PI / 3.5,
    panSpeed: 0.3,
    rotateSpeed: 0.02,
    minZoom: 30,
    maxZoom: 150,
    focusTarget,
  });
```

- [ ] **Step 4: Run TypeScript check**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npx tsc --noEmit`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/hex-grid/useCameraControls.ts src/components/hex-grid/HexGrid.tsx
git commit -m "feat(#313): add camera auto-center on turn start

Camera lerps to active character position when turn changes.
Manual WASD panning cancels the lerp."
```

---

### Task 9: Guard against re-opening doors + final CI check

Ensure already-opened doors don't trigger OpenDoor RPC, then run full CI.

**Files:**
- Modify: `src/components/hex-grid/HexGrid.tsx` (door click handler)

**Key context:**
- HexGrid.tsx line 359-363: door click handler checks `isPlayerTurn`, `isProcessing`, `isDoorLoading` but NOT `door.isOpen`
- Adding `|| door.isOpen` to the guard prevents re-triggering

- [ ] **Step 1: Add isOpen guard to door click**

In `src/components/hex-grid/HexGrid.tsx`, update the HexDoor onClick (around line 359):

The `door` variable is already in scope from the `.map()` iterator, so use `door.isOpen` directly:

```typescript
            onClick={(connectionId) => {
              if (!isPlayerTurn || isProcessing || isDoorLoading || door.isOpen) return;
              onDoorClick?.(connectionId);
            }}
```

- [ ] **Step 2: Run full CI check**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npm run ci-check`
Expected: PASS

- [ ] **Step 3: Commit**

```bash
cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest
git add src/components/hex-grid/HexGrid.tsx
git commit -m "fix(#310): prevent re-opening already opened doors"
```

---

### Task 10: Manual smoke test checklist

This task has no code — it's a verification checklist for the developer to run through manually once the API is serving Room.origin data.

- [ ] **Step 1: Start dev server and connect to API**

Run: `cd /home/kirk/personal/rpg-dnd5e-web/.worktrees/playtest && npm run dev`

- [ ] **Step 2: Verify single room renders correctly**

Start a combat encounter. First room should render at its origin position. Tiles, entities, walls, and doors should all appear correctly.

- [ ] **Step 3: Verify door opens and second room appears**

Click a closed door. The new room should appear adjacent to the first room with correct positioning. No room replacement — both rooms visible.

- [ ] **Step 4: Verify camera auto-centers on turn start**

Wait for turn change. Camera should smoothly move to the active character's position.

- [ ] **Step 5: Verify WASD panning works without sticking**

Press WASD to pan. Alt-tab away and back. Keys should not be stuck.

- [ ] **Step 6: Verify pathfinding across rooms**

Move a character near the door boundary. Hover on tiles in the adjacent room — path preview should cross the boundary.

- [ ] **Step 7: Verify door doesn't re-open**

Click an already-opened door. Nothing should happen (no loading spinner, no API call).
