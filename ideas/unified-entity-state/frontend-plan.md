# Unified Entity State — Frontend Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace fragmented entity state in the web client with a single store backed by the new `EntityState` and `EncounterStateData` proto messages, fixing bugs #357, #358, #347.

**Architecture:** One `useEncounterState` hook holds all entity state. Snapshot events replace the whole store. Delta events merge by entity ID. Components read from one source of truth.

**Tech Stack:** TypeScript, React, Vitest, @kirkdiggler/rpg-api-protos (generated branch)

**Spec:** `rpg-project/ideas/unified-entity-state/design.md`
**API PR:** rpg-api#454 (wires entity state to all events)

**Proto names:**
- `EncounterStateData` (not EncounterState — enum conflict)
- `EntityState` with `oneof details { CharacterDetails, MonsterDetails, ObstacleDetails }`
- Snapshot events carry `encounter_state_data` field
- Delta events carry `attacker_state`/`target_state` or `updated_entity`/`updated_entities` fields

---

## Task 0: Update Proto Package

**Files:**
- Modify: `package.json`

- [ ] **Step 1: Install latest proto package**

```bash
cd /home/kirk/personal/rpg-dnd5e-web
npm install @kirkdiggler/rpg-api-protos@latest
```

- [ ] **Step 2: Verify new types exist**

Check that these types are available in the generated TS:
- `EntityState` with `entityId`, `currentHitPoints`, `activeConditions`, `details` oneof
- `EncounterStateData` with `entities` map, `rooms` map, `combat`
- `CharacterDetails` with `name`, `race`, `characterClass`, `appearance`
- `MonsterDetails` with `name`, `monsterType`

```bash
grep -r "EntityState\|EncounterStateData\|CharacterDetails\|MonsterDetails" node_modules/@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/ | head -20
```

- [ ] **Step 3: Commit**

```bash
git add package.json package-lock.json && git commit -m "chore: update rpg-api-protos for unified entity state"
```

---

## Task 1: Create Entity Helper Utilities

**Files:**
- Create: `src/utils/entityHelpers.ts`
- Create: `src/utils/entityHelpers.test.ts`

These are pure functions with no React dependencies — easy to test in isolation.

- [ ] **Step 1: Write tests**

```typescript
import { describe, it, expect } from 'vitest';
// Import generated proto types
import { EntityState, ConditionId } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/encounter_pb';

describe('entityHelpers', () => {
  describe('hasCondition', () => {
    it('returns true when condition present', () => {});
    it('returns false when condition absent', () => {});
    it('returns false for empty conditions list', () => {});
  });

  describe('isDead', () => {
    it('returns true when DEAD condition present', () => {});
    it('returns false when alive', () => {});
  });

  describe('isUnconscious', () => {
    it('returns true when UNCONSCIOUS condition present', () => {});
  });

  describe('getHealthCategory', () => {
    it('returns "dead" when entity has DEAD condition', () => {});
    it('returns "uninjured" at full HP', () => {});
    it('returns "injured" at 50-99% HP', () => {});
    it('returns "bloodied" at 25-49% HP', () => {});
    it('returns "near death" below 25% HP', () => {});
  });

  describe('getEntityName', () => {
    it('returns character name from characterDetails', () => {});
    it('returns monster name from monsterDetails', () => {});
    it('returns entityId as fallback', () => {});
  });
});
```

- [ ] **Step 2: Run tests to verify they fail**

```bash
npm run test:run -- --reporter verbose src/utils/entityHelpers.test.ts
```

- [ ] **Step 3: Implement helpers**

```typescript
import { EntityState } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/encounter_pb';
import { ConditionId } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha1/enums_pb';

export function hasCondition(entity: EntityState, conditionId: ConditionId): boolean {
  return entity.activeConditions.some(c => c.id === conditionId);
}

export function isDead(entity: EntityState): boolean {
  // Monsters die at 0 HP, characters go unconscious
  // Check for explicit DEAD condition OR 0 HP on monsters
  return hasCondition(entity, ConditionId.DEAD) ||
    (entity.details.case === 'monsterDetails' && entity.currentHitPoints <= 0);
}

export function isUnconscious(entity: EntityState): boolean {
  return hasCondition(entity, ConditionId.UNCONSCIOUS);
}

export function getHealthCategory(entity: EntityState): { label: string; color: string } {
  if (isDead(entity)) return { label: 'dead', color: '#666' };
  if (entity.maxHitPoints === 0) return { label: 'unknown', color: '#888' };
  const ratio = entity.currentHitPoints / entity.maxHitPoints;
  if (ratio >= 1) return { label: 'uninjured', color: '#4CAF50' };
  if (ratio >= 0.5) return { label: 'injured', color: '#FFC107' };
  if (ratio >= 0.25) return { label: 'bloodied', color: '#FF9800' };
  return { label: 'near death', color: '#F44336' };
}

export function getEntityName(entity: EntityState): string {
  if (entity.details.case === 'characterDetails') return entity.details.value.name;
  if (entity.details.case === 'monsterDetails') return entity.details.value.name;
  return entity.entityId;
}
```

Note: Check the exact proto TS type shapes. The `details` field may use `case`/`value` pattern from protobuf-es, or it may be a different accessor pattern. Read the generated TS to confirm.

- [ ] **Step 4: Run tests**

```bash
npm run test:run -- --reporter verbose src/utils/entityHelpers.test.ts
```

- [ ] **Step 5: Commit**

```bash
git add src/utils/entityHelpers*.ts && git commit -m "feat: add entity helper utilities (hasCondition, isDead, getHealthCategory)"
```

---

## Task 2: Create useEncounterState Hook

**Files:**
- Create: `src/hooks/useEncounterState.ts`
- Create: `src/hooks/useEncounterState.test.ts`

- [ ] **Step 1: Write tests**

```typescript
describe('useEncounterState', () => {
  it('starts with empty state', () => {});
  it('applySnapshot replaces entire state from EncounterStateData proto', () => {});
  it('applyEntityUpdates merges entities by ID without losing others', () => {});
  it('applyEntityUpdates adds new entities', () => {});
  it('applyCombatState updates combat without touching entities', () => {});
  it('reset returns to empty state', () => {});
});
```

- [ ] **Step 2: Run tests to verify they fail**

```bash
npm run test:run -- --reporter verbose src/hooks/useEncounterState.test.ts
```

- [ ] **Step 3: Implement the hook**

Key implementation detail: Proto `map` fields generate `Record<string, T>` in TypeScript (plain objects), not `Map`. The hook should convert to `Map` for React-friendly usage:

```typescript
import { useState, useCallback } from 'react';
import { EntityState, EncounterStateData, CombatState, RoomLayout, DoorInfo } from '...';
import { DungeonState } from '...';

export interface LocalEncounterState {
  encounterId: string;
  dungeonId: string;
  entities: Map<string, EntityState>;
  rooms: Map<string, RoomLayout>;
  currentRoomId: string;
  revealedRoomIds: string[];
  combat: CombatState | null;
  doors: Map<string, DoorInfo>;
  dungeonState: DungeonState;
}

function createEmptyState(): LocalEncounterState { ... }

function protoMapToMap<T>(record: Record<string, T> | undefined): Map<string, T> {
  return new Map(Object.entries(record ?? {}));
}

export function useEncounterState() {
  const [state, setState] = useState<LocalEncounterState>(createEmptyState());

  const applySnapshot = useCallback((proto: EncounterStateData) => {
    setState({
      encounterId: proto.encounterId,
      dungeonId: proto.dungeonId,
      entities: protoMapToMap(proto.entities),
      rooms: protoMapToMap(proto.rooms),
      currentRoomId: proto.currentRoomId,
      revealedRoomIds: [...proto.revealedRoomIds],
      combat: proto.combat ?? null,
      doors: protoMapToMap(proto.doors),
      dungeonState: proto.dungeonState,
    });
  }, []);

  const applyEntityUpdates = useCallback((updates: EntityState[]) => {
    setState(prev => {
      const newEntities = new Map(prev.entities);
      for (const entity of updates) {
        if (entity) newEntities.set(entity.entityId, entity);
      }
      return { ...prev, entities: newEntities };
    });
  }, []);

  const applyCombatState = useCallback((combat: CombatState) => {
    setState(prev => ({ ...prev, combat }));
  }, []);

  const reset = useCallback(() => setState(createEmptyState()), []);

  return { state, applySnapshot, applyEntityUpdates, applyCombatState, reset };
}
```

- [ ] **Step 4: Run tests**

```bash
npm run test:run -- --reporter verbose src/hooks/useEncounterState.test.ts
```

- [ ] **Step 5: Commit**

```bash
git add src/hooks/useEncounterState*.ts && git commit -m "feat: add useEncounterState hook with snapshot/delta operations"
```

---

## Task 3: Rewire LobbyView Event Handlers

**Files:**
- Modify: `src/components/LobbyView.tsx`

This is the biggest task. LobbyView currently has 15+ event handlers each manually syncing 2-4 state containers.

**Strategy:** Add `useEncounterState` alongside existing state. Update event handlers to populate the new store from the new proto fields. Keep old state as fallback until components are updated. This allows incremental migration.

- [ ] **Step 1: Add useEncounterState to LobbyView**

Add alongside existing state (don't remove old state yet):
```typescript
const { state: encounterState, applySnapshot, applyEntityUpdates, applyCombatState, reset: resetEncounterState } = useEncounterState();
```

- [ ] **Step 2: Update snapshot event handlers**

For each snapshot handler, add the new path:

```typescript
const handleCombatStarted = (event: CombatStartedEvent) => {
  // New path: use EncounterStateData if present
  if (event.encounterStateData) {
    applySnapshot(event.encounterStateData);
  }

  // Legacy path: keep existing state sync for components not yet migrated
  // ... existing code stays for now ...
};
```

Apply same pattern to: handleRoomRevealed, handleStateSync, handleCombatResumed, handleDungeonVictory, handleDungeonFailure, handleCombatEnded.

- [ ] **Step 3: Update delta event handlers**

```typescript
const handleAttackResolved = (event: AttackResolvedEvent) => {
  // New path
  const updates = [event.attackerState, event.targetState].filter(Boolean);
  if (updates.length > 0) applyEntityUpdates(updates);

  // Legacy path stays for combat log, etc.
  // ... existing code ...
};

const handleMovementCompleted = (event: MovementCompletedEvent) => {
  if (event.updatedEntity) applyEntityUpdates([event.updatedEntity]);
  if (event.combatState) applyCombatState(event.combatState);
  // Legacy path stays ...
};
```

Apply same pattern to: handleTurnEnded, handleMonsterTurnCompleted, handleFeatureActivated, handleActionExecuted, handleCombatAbilityActivated, handleRestCompleted, death save handlers.

**Rule:** For any delta event that includes `combatState`, always call both `applyEntityUpdates` AND `applyCombatState`.

- [ ] **Step 4: Pass encounterState to child components (alongside old props)**

For now, pass `encounterState` as a new prop to BattleMapPanel, HoverInfoPanel, etc. — alongside the old props. Components will migrate one at a time.

- [ ] **Step 5: Commit**

```bash
git add src/components/LobbyView.tsx && git commit -m "feat: wire useEncounterState into LobbyView event handlers"
```

---

## Task 4: Update BattleMapPanel to Read from EntityState

**Files:**
- Modify: `src/components/encounter/BattleMapPanel.tsx`

- [ ] **Step 1: Add entities prop from encounter state**

Accept `encounterEntities?: Map<string, EntityState>` as a new prop alongside existing props.

- [ ] **Step 2: Prefer new entities when available**

```typescript
const renderableEntities = useMemo(() => {
  // Prefer unified entity state when available
  if (encounterEntities && encounterEntities.size > 0) {
    return Array.from(encounterEntities.values())
      .filter(entity => !isDead(entity))
      .filter(entity => entity.roomId === currentRoomId || !entity.roomId)
      .map(entity => ({
        entityId: entity.entityId,
        name: getEntityName(entity),
        position: entity.position,
        type: entity.entityType === EntityType.CHARACTER ? 'player' : 'monster',
      }));
  }

  // Fallback to legacy dungeonMap.entities
  return Array.from(dungeonMap.entities.values()).map(entity => ({ ... }));
}, [encounterEntities, dungeonMap.entities, currentRoomId]);
```

This fixes **#358** — dead monsters are filtered out because `isDead()` checks the same entity the renderer uses.

- [ ] **Step 3: Commit**

```bash
git add src/components/encounter/BattleMapPanel.tsx && git commit -m "feat: BattleMapPanel reads from unified entity state (fixes #358)"
```

---

## Task 5: Update HoverInfoPanel to Read from EntityState

**Files:**
- Modify: `src/components/combat-v2/panels/HoverInfoPanel.tsx`

- [ ] **Step 1: Accept entities from encounter state**

Accept `encounterEntities?: Map<string, EntityState>` as prop.

- [ ] **Step 2: Read HP from EntityState when available**

```typescript
// When unified entity state is available, use it
if (encounterEntities && hoveredEntityId) {
  const entity = encounterEntities.get(hoveredEntityId);
  if (entity) {
    const health = getHealthCategory(entity);
    // Render with health.label and health.color
  }
}
```

This fixes **#357** — hover panel reads HP from the same EntityState that the renderer uses.

- [ ] **Step 3: Commit**

```bash
git add src/components/combat-v2/panels/HoverInfoPanel.tsx && git commit -m "fix: hover panel reads HP from unified entity state (fixes #357)"
```

---

## Task 6: Update Remaining Components

**Files:**
- Modify: `src/components/hex-grid/HexEntity.tsx` — read character/monster rendering data from EntityState details
- Modify: `src/components/combat-v2/panels/CombatPanel.tsx` — current entity details from encounter state
- Modify: Any initiative tracker that reads HP

- [ ] **Step 1: Update HexEntity**

Character rendering reads from `entity.details.characterDetails` (appearance, equipment, class). Monster rendering reads from `entity.details.monsterDetails` (monsterType for texture).

- [ ] **Step 2: Update CombatPanel**

Current turn entity from `encounterState.combat.currentTurn.entityId` → look up in `encounterState.entities`.

- [ ] **Step 3: Fix all TypeScript errors**

```bash
npx tsc --noEmit
```

- [ ] **Step 4: Commit**

```bash
git add src/components/ && git commit -m "refactor: remaining components read from unified entity state"
```

---

## Task 7: Delete Dead Code

**Files:**
- Delete: `src/utils/characterMerge.ts`
- Delete: `src/utils/characterMerge.test.ts`
- Modify: `src/components/LobbyView.tsx` — remove old state (monsters, fullCharactersMap)
- Modify: `src/hooks/useDungeonMap.ts` — remove entity tracking if fully replaced

- [ ] **Step 1: Remove old state from LobbyView**

Delete these useState calls:
```typescript
// REMOVE:
const [monsters, setMonsters] = useState<MonsterCombatState[]>([]);
const [fullCharactersMap, setFullCharactersMap] = useState<Map<string, Character>>(new Map());
```

Remove all legacy event handler code that syncs these.
Remove `mergeCharacterUpdate` imports and usage.

- [ ] **Step 2: Delete characterMerge.ts**

```bash
rm src/utils/characterMerge.ts src/utils/characterMerge.test.ts
```

- [ ] **Step 3: Verify no remaining imports**

```bash
grep -r "characterMerge\|mergeCharacterUpdate\|fullCharactersMap\|setMonsters" src/
```

Should return nothing.

- [ ] **Step 4: Clean up useDungeonMap**

If entity tracking has been fully moved to `useEncounterState`, remove the `entities` field from `DungeonMapState`. Keep floor tile and wall accumulation — those are still needed for hex grid rendering.

- [ ] **Step 5: Commit**

```bash
git add -A && git commit -m "chore: delete mergeCharacterUpdate and legacy entity state code"
```

---

## Task 8: Tests and CI

- [ ] **Step 1: Run all tests**

```bash
npm run test:run
```

- [ ] **Step 2: Fix any failing tests**

Update test fixtures that reference old state shapes.

- [ ] **Step 3: Run ci-check**

```bash
npm run ci-check
```

- [ ] **Step 4: Commit fixes**

```bash
git add -A && git commit -m "test: update tests for unified entity state"
```

---

## Task 9: Push and Create PR

- [ ] **Step 1: Push and create PR**

```bash
git push -u origin feat/unified-entity-state-web
gh pr create --title "feat: unified entity state - web client" --body "Fixes #357 #358 #347"
```
