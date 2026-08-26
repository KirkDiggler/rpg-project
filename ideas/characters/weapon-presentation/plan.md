# Fighter Main-Hand Weapon Concept Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the fighter-only Concepts Lab proof in which wire-shaped `main_hand` state switches the real animated Synty fighter among unarmed, provisional longsword, and provisional shortbow presentations through one `Hand_R` socket.

**Architecture:** Add one pure Three.js attachment unit, wrap it in an attachment-local React loader boundary, and expose it as an optional presentation input on the existing `ClassCharacterModel`. The Concepts Lab supplies provisional wire-shaped fixtures and uses the real shared renderer; Blender authors/verifies the socket transform, while the browser only validates and records a non-production verdict.

**Tech Stack:** React 19, TypeScript 5.8, React Three Fiber 9, drei 10, Three.js 0.181, Vitest 4, React Three Test Renderer 9, Testing Library, Blender 5.0.1 with loopback Blender MCP.

**Spec:** `ideas/characters/weapon-presentation/design.md`

**Tracking:** `rpg-project#281` → `rpg-dnd5e-web#821`; design review surface `rpg-project#283`.

## Global Constraints

- Implement only `rpg-dnd5e-web#821`; branch from fresh `origin/dev`, never local `dev`.
- Licensed GLBs remain in private `rpg-game-assets` and ignored `public/models/synty/`; never commit them to `rpg-dnd5e-web` or `rpg-project`.
- Fixture refs are exact equipped-item refs: `dnd5e:item:longsword` and `dnd5e:item:shortbow`. The attack-shaped `dnd5e:weapons:longsword` must remain unmapped.
- Character GLBs stay unarmed on disk. Runtime presentation attaches a standalone weapon beneath the cloned fighter `Hand_R` bone.
- One fighter socket is shared by sword and bow. Do not add per-weapon runtime offsets or a character × weapon transform table.
- The fighter profile carries `boneUnitMeters: 0.01`; child-local position is `positionMeters / boneUnitMeters`, and child-local scale is `scale / boneUnitMeters` so the outer `SYNTY_SCALE` still affects fighter and weapon together.
- Blender is the transform-authoring surface. The Concepts Lab must not expose transform sliders or write provider data.
- Scope is fighter, main hand, acting-player-shaped fixture data, existing idle/walk, and development-only Concepts. No live game wiring, off-hand/shields, monsters, multiplayer propagation, additional classes, or combat/bow animation authoring.
- Unknown ref, missing asset, invalid socket, and missing bone preserve the fighter and report an honest attachment-local status. Never substitute a visually similar weapon.
- `ClassCharacterModel` must continue using `SkeletonUtils.clone()` and must never mutate/reparent drei's cached character or weapon scenes.
- Entity raycasting remains on `HexEntity`'s stable proxy; attached weapons are not new interaction targets.
- Run focused tests after every task and fresh `npm run ci-check` before publication. Never use `--no-verify`.
- Every GitHub body/comment uses the Assets signature: `— assets agent, on behalf of KirkDiggler`.

---

## File Structure

### Shared production renderer seam

- Create `rpg-dnd5e-web/src/components/hex-grid/mainHandPresentation.ts`
  - Owns presentation/socket/status types, socket validation, unit compensation, imperative attach/detach.
- Create `rpg-dnd5e-web/src/components/hex-grid/mainHandPresentation.test.ts`
  - Pins `0.01` compensation, quaternion application, cleanup, invalid socket, and missing bone.
- Create `rpg-dnd5e-web/src/components/hex-grid/MainHandAttachment.tsx`
  - Loads/clones a weapon, invokes the pure attachment unit, isolates Suspense/load failure, reports status.
- Create `rpg-dnd5e-web/src/components/hex-grid/MainHandAttachment.test.tsx`
  - Pins shared-cache safety, attach/unmount/replacement, load failure, and unarmed status.
- Modify `rpg-dnd5e-web/src/components/hex-grid/ClassCharacterModel.tsx`
  - Adds optional `mainHandPresentation` and `onMainHandStatus`; mounts the attachment slot beside the existing primitive.
- Create `rpg-dnd5e-web/src/components/hex-grid/ClassCharacterModel.test.tsx`
  - Proves the real shared component keeps rendering the fighter when its optional weapon fails.

### Concepts proof

- Create `rpg-dnd5e-web/src/concepts/weapon-attachment/weaponAttachmentExperiment.ts`
  - Owns provisional candidate metadata, exact equipped fixtures, pure resolver, observation coverage, non-production verdict shape.
- Create `rpg-dnd5e-web/src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts`
  - Pins exact refs, candidate paths, shared socket identity, attack-ref rejection, and non-Cartesian coverage gate.
- Create `rpg-dnd5e-web/src/concepts/weapon-attachment/WeaponAttachmentPreview.tsx`
  - Owns Canvas, cameras, lights, real `ClassCharacterModel`, motion/facing projection, and post-commit observations.
- Create `rpg-dnd5e-web/src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx`
  - Proves the real shared component receives exact concept state and that only successful unarmed/attached renders count.
- Create `rpg-dnd5e-web/src/concepts/weapon-attachment/WeaponAttachmentConcept.tsx`
  - Owns fixture/motion/view/facing controls, contract inspector, coverage checklist, and gated verdict output.
- Create `rpg-dnd5e-web/src/concepts/weapon-attachment/WeaponAttachmentConcept.test.tsx`
  - Pins all controls, inspector truth, coverage gate, and output warning.
- Create `rpg-dnd5e-web/src/concepts/weapon-attachment/CONTRACT.md`
  - Records the final observed consumer contract and provider gaps after Kirk's walk.
- Modify `rpg-dnd5e-web/src/concepts/ConceptsView.tsx`
  - Registers `weapon-attachment` and the `?concept=weapon-attachment` deep link.
- Modify `rpg-dnd5e-web/docs/architecture/components/concepts-route.md`
  - Adds the Concept and its evidence status.
- Modify `rpg-dnd5e-web/docs/how-to/concepts-route.md`
  - Adds the deep link/current-concepts row.

### Visual evidence

- Create `rpg-dnd5e-web/docs/evidence/821-weapon-attachment/README.md`
  - Records exact provider revision/assets, Blender/Three observations, screenshots, texture warnings, and Kirk's verdict.
- Create reviewed screenshots under `rpg-dnd5e-web/docs/evidence/821-weapon-attachment/` only after viewing them.

---

### Task 1: Pure Main-Hand Attachment Contract

**Files:**
- Create: `src/components/hex-grid/mainHandPresentation.ts`
- Create: `src/components/hex-grid/mainHandPresentation.test.ts`

**Interfaces:**
- Consumes: `THREE.Object3D`, `THREE.Bone`, normalized true-meter weapon roots.
- Produces:
  - `MainHandSocket`
  - `MainHandPresentation`
  - `MainHandAttachmentCode`
  - `MainHandAttachmentStatus`
  - `validateMainHandSocket(socket): boolean`
  - `attachMainHandObject(characterRoot, weaponRoot, presentation): MainHandAttachmentResult`

- [ ] **Step 1: Create the isolated web worktree and establish a clean baseline**

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web
git fetch --prune origin
git worktree add /home/kirk/.pi/worktrees/rpg-dnd5e-web/821-weapon-attachment \
  -b concept/821-fighter-weapon-attachment origin/dev
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/821-weapon-attachment
npm install
RPG_GAME_ASSETS_PATH=/home/kirk/game-dev/rpg-game-assets npm run assets:sync
npm run test:run -- src/components/hex-grid/classCharacterModels.test.ts src/components/hex-grid/HexEntity.test.tsx
```

Expected: focused baseline tests pass; the ignored `public/models/synty/characters/{fighter.glb,weapons/fighter-weapon.glb,weapons/bow-01.glb}` files exist.

Move Project 19 item `PVTI_lAHOAASbwc4Bcj4vzg4BWnM` from Todo to In Progress:

```bash
gh project item-edit \
  --project-id PVT_kwHOAASbwc4Bcj4v \
  --id PVTI_lAHOAASbwc4Bcj4vzg4BWnM \
  --field-id PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM \
  --single-select-option-id a434eab1
```

- [ ] **Step 2: Write the failing pure attachment tests**

Create `src/components/hex-grid/mainHandPresentation.test.ts`:

```ts
import * as THREE from 'three';
import { describe, expect, it } from 'vitest';
import {
  attachMainHandObject,
  type MainHandPresentation,
} from './mainHandPresentation';

const presentation = (): MainHandPresentation => ({
  ref: 'dnd5e:item:longsword',
  weaponUrl: '/models/synty/characters/weapons/fighter-weapon.glb',
  socket: {
    bone: 'Hand_R',
    boneUnitMeters: 0.01,
    positionMeters: [-0.0554, 0.1299, 0.0237],
    rotationQuaternion: [
      -0.7071067811865475,
      0,
      0,
      0.7071067811865476,
    ],
    scale: 1,
  },
});

function characterWithHand(): { root: THREE.Group; hand: THREE.Bone } {
  const root = new THREE.Group();
  const hand = new THREE.Bone();
  hand.name = 'Hand_R';
  root.add(hand);
  return { root, hand };
}

describe('attachMainHandObject', () => {
  it('compensates the fighter 0.01 bone units and attaches exactly once', () => {
    const { root, hand } = characterWithHand();
    const weapon = new THREE.Group();

    const result = attachMainHandObject(root, weapon, presentation());

    expect(result.status.code).toBe('attached');
    expect(hand.children).toEqual([weapon]);
    expect(weapon.position.toArray()).toEqual([-5.54, 12.99, 2.37]);
    expect(weapon.scale.toArray()).toEqual([100, 100, 100]);
    expect(weapon.quaternion.toArray()).toEqual(
      expect.arrayContaining([
        expect.closeTo(-0.7071067811865475, 12),
        0,
        0,
        expect.closeTo(0.7071067811865476, 12),
      ])
    );

    result.detach();
    expect(hand.children).toEqual([]);
    result.detach();
    expect(hand.children).toEqual([]);
  });

  it('refuses a missing hand without mutating the weapon', () => {
    const root = new THREE.Group();
    const weapon = new THREE.Group();

    const result = attachMainHandObject(root, weapon, presentation());

    expect(result.status).toMatchObject({
      code: 'missing-bone',
      bone: 'Hand_R',
      ref: 'dnd5e:item:longsword',
    });
    expect(weapon.parent).toBeNull();
  });

  it.each([
    { field: 'boneUnitMeters', value: 0 },
    { field: 'boneUnitMeters', value: Number.NaN },
    { field: 'scale', value: 0 },
    { field: 'scale', value: Number.POSITIVE_INFINITY },
  ])('refuses invalid $field=$value before scene mutation', ({ field, value }) => {
    const { root, hand } = characterWithHand();
    const weapon = new THREE.Group();
    const candidate = presentation();
    candidate.socket = { ...candidate.socket, [field]: value };

    const result = attachMainHandObject(root, weapon, candidate);

    expect(result.status.code).toBe('invalid-socket');
    expect(hand.children).toEqual([]);
    expect(weapon.parent).toBeNull();
  });

  it('refuses non-finite position and a zero-length quaternion', () => {
    const { root, hand } = characterWithHand();
    const weapon = new THREE.Group();
    const candidate = presentation();
    candidate.socket = {
      ...candidate.socket,
      positionMeters: [Number.NaN, 0, 0],
      rotationQuaternion: [0, 0, 0, 0],
    };

    const result = attachMainHandObject(root, weapon, candidate);

    expect(result.status.code).toBe('invalid-socket');
    expect(hand.children).toEqual([]);
  });
});
```

- [ ] **Step 3: Run the test and verify RED**

```bash
npm run test:run -- src/components/hex-grid/mainHandPresentation.test.ts
```

Expected: FAIL because `./mainHandPresentation` does not exist.

- [ ] **Step 4: Implement the minimal pure contract**

Create `src/components/hex-grid/mainHandPresentation.ts`:

```ts
import * as THREE from 'three';

export type Vec3Tuple = readonly [number, number, number];
export type QuaternionTuple = readonly [number, number, number, number];

export interface MainHandSocket {
  bone: string;
  boneUnitMeters: number;
  positionMeters: Vec3Tuple;
  rotationQuaternion: QuaternionTuple;
  scale: number;
}

export interface MainHandPresentation {
  ref: string;
  weaponUrl: string;
  socket: MainHandSocket;
}

export type MainHandAttachmentCode =
  | 'unarmed'
  | 'loading'
  | 'attached'
  | 'unmapped-ref'
  | 'asset-load-failed'
  | 'missing-bone'
  | 'invalid-socket';

export interface MainHandAttachmentStatus {
  code: MainHandAttachmentCode;
  ref?: string;
  weaponUrl?: string;
  bone?: string;
  message?: string;
}

export interface MainHandAttachmentResult {
  status: MainHandAttachmentStatus;
  detach: () => void;
}

const allFinite = (values: readonly number[]): boolean =>
  values.every(Number.isFinite);

export function validateMainHandSocket(socket: MainHandSocket): boolean {
  const quaternionLengthSquared = socket.rotationQuaternion.reduce(
    (sum, value) => sum + value * value,
    0
  );
  return (
    socket.bone.trim().length > 0 &&
    Number.isFinite(socket.boneUnitMeters) &&
    socket.boneUnitMeters > 0 &&
    allFinite(socket.positionMeters) &&
    allFinite(socket.rotationQuaternion) &&
    quaternionLengthSquared > Number.EPSILON &&
    Number.isFinite(socket.scale) &&
    socket.scale > 0
  );
}

const statusFor = (
  code: MainHandAttachmentCode,
  presentation: MainHandPresentation,
  message?: string
): MainHandAttachmentStatus => ({
  code,
  ref: presentation.ref,
  weaponUrl: presentation.weaponUrl,
  bone: presentation.socket.bone,
  message,
});

export function attachMainHandObject(
  characterRoot: THREE.Object3D,
  weaponRoot: THREE.Object3D,
  presentation: MainHandPresentation
): MainHandAttachmentResult {
  const { socket } = presentation;
  const noDetach = () => {};
  if (!validateMainHandSocket(socket)) {
    return {
      status: statusFor('invalid-socket', presentation),
      detach: noDetach,
    };
  }

  const bone = characterRoot.getObjectByName(socket.bone);
  if (!(bone instanceof THREE.Bone)) {
    return {
      status: statusFor('missing-bone', presentation),
      detach: noDetach,
    };
  }

  const unitsPerMeter = 1 / socket.boneUnitMeters;
  weaponRoot.position
    .fromArray([...socket.positionMeters])
    .multiplyScalar(unitsPerMeter);
  weaponRoot.quaternion
    .fromArray([...socket.rotationQuaternion])
    .normalize();
  weaponRoot.scale.setScalar(socket.scale * unitsPerMeter);
  bone.add(weaponRoot);

  return {
    status: statusFor('attached', presentation),
    detach: () => {
      if (weaponRoot.parent === bone) bone.remove(weaponRoot);
    },
  };
}
```

- [ ] **Step 5: Run focused tests and verify GREEN**

```bash
npm run test:run -- src/components/hex-grid/mainHandPresentation.test.ts
npm run typecheck
```

Expected: attachment tests pass; typecheck passes.

- [ ] **Step 6: Commit the pure unit**

```bash
git add src/components/hex-grid/mainHandPresentation.ts \
  src/components/hex-grid/mainHandPresentation.test.ts
git commit -m "feat(models): define main-hand bone attachment contract (#821)"
```

---

### Task 2: Attachment-Local Loader and Real Fighter Integration

**Files:**
- Create: `src/components/hex-grid/MainHandAttachment.tsx`
- Create: `src/components/hex-grid/MainHandAttachment.test.tsx`
- Modify: `src/components/hex-grid/ClassCharacterModel.tsx`
- Create: `src/components/hex-grid/ClassCharacterModel.test.tsx`

**Interfaces:**
- Consumes from Task 1: `MainHandPresentation`, `MainHandAttachmentStatus`, `attachMainHandObject()`.
- Produces:
  - `MainHandAttachmentSlot({ characterRoot, presentation, onStatus })`
  - optional `ClassCharacterModelProps.mainHandPresentation`
  - optional `ClassCharacterModelProps.onMainHandStatus`

- [ ] **Step 1: Write failing loader/cleanup/cache-safety tests**

Create `src/components/hex-grid/MainHandAttachment.test.tsx` with a URL-keyed `useGLTF` mock. The mock must retain the cached source scenes so the test can prove they remain unparented:

```tsx
import ReactThreeTestRenderer from '@react-three/test-renderer';
import * as THREE from 'three';
import { afterEach, describe, expect, it, vi } from 'vitest';
import type {
  MainHandAttachmentStatus,
  MainHandPresentation,
} from './mainHandPresentation';

const gltf = vi.hoisted(() => ({
  scenes: new Map<string, THREE.Group>(),
  failed: new Set<string>(),
}));

vi.mock('@react-three/drei', () => ({
  useGLTF: (url: string) => {
    if (gltf.failed.has(url)) throw new Error(`failed ${url}`);
    let scene = gltf.scenes.get(url);
    if (!scene) {
      scene = new THREE.Group();
      scene.name = `cached:${url}`;
      const mesh = new THREE.Mesh(
        new THREE.BoxGeometry(),
        new THREE.MeshStandardMaterial()
      );
      mesh.name = `cached-mesh:${url}`;
      scene.add(mesh);
      gltf.scenes.set(url, scene);
    }
    return { scene };
  },
}));

import { MainHandAttachmentSlot } from './MainHandAttachment';

const sword: MainHandPresentation = {
  ref: 'dnd5e:item:longsword',
  weaponUrl: '/models/synty/characters/weapons/fighter-weapon.glb',
  socket: {
    bone: 'Hand_R',
    boneUnitMeters: 0.01,
    positionMeters: [-0.0554, 0.1299, 0.0237],
    rotationQuaternion: [-0.7071067811865475, 0, 0, 0.7071067811865476],
    scale: 1,
  },
};
const bow: MainHandPresentation = {
  ...sword,
  ref: 'dnd5e:item:shortbow',
  weaponUrl: '/models/synty/characters/weapons/bow-01.glb',
};

function character(): { root: THREE.Group; hand: THREE.Bone } {
  const root = new THREE.Group();
  const hand = new THREE.Bone();
  hand.name = 'Hand_R';
  root.add(hand);
  return { root, hand };
}

afterEach(() => {
  gltf.scenes.clear();
  gltf.failed.clear();
});

describe('MainHandAttachmentSlot', () => {
  it('attaches a clone and never reparents the cached GLTF scene', async () => {
    const { root, hand } = character();
    const statuses: MainHandAttachmentStatus[] = [];
    const renderer = await ReactThreeTestRenderer.create(
      <MainHandAttachmentSlot
        characterRoot={root}
        presentation={sword}
        onStatus={(status) => statuses.push(status)}
      />
    );

    const cached = gltf.scenes.get(sword.weaponUrl)!;
    expect(cached.parent).toBeNull();
    expect(hand.children).toHaveLength(1);
    expect(hand.children[0]).not.toBe(cached);
    const cachedMesh = cached.getObjectByName(
      `cached-mesh:${sword.weaponUrl}`
    ) as THREE.Mesh;
    const attachedMesh = hand.children[0]!.getObjectByName(
      `cached-mesh:${sword.weaponUrl}`
    ) as THREE.Mesh;
    expect(attachedMesh).toBeDefined();
    expect(attachedMesh.raycast).not.toBe(cachedMesh.raycast);
    expect(statuses.at(-1)?.code).toBe('attached');

    await renderer.unmount();
    expect(hand.children).toHaveLength(0);
  });

  it('replaces the old clone when the keyed slot changes weapon', async () => {
    const { root, hand } = character();
    const renderer = await ReactThreeTestRenderer.create(
      <MainHandAttachmentSlot
        key={sword.ref}
        characterRoot={root}
        presentation={sword}
      />
    );
    const swordClone = hand.children[0];

    await renderer.update(
      <MainHandAttachmentSlot
        key={bow.ref}
        characterRoot={root}
        presentation={bow}
      />
    );

    expect(hand.children).toHaveLength(1);
    expect(hand.children[0]).not.toBe(swordClone);
    expect(gltf.scenes.get(sword.weaponUrl)!.parent).toBeNull();
    expect(gltf.scenes.get(bow.weaponUrl)!.parent).toBeNull();
  });

  it('reports unarmed without loading a weapon', async () => {
    const { root, hand } = character();
    const statuses: MainHandAttachmentStatus[] = [];
    await ReactThreeTestRenderer.create(
      <MainHandAttachmentSlot
        characterRoot={root}
        onStatus={(status) => statuses.push(status)}
      />
    );

    expect(gltf.scenes.size).toBe(0);
    expect(hand.children).toHaveLength(0);
    expect(statuses.at(-1)?.code).toBe('unarmed');
  });

  it('contains a weapon load failure and reports the exact URL', async () => {
    const { root, hand } = character();
    const statuses: MainHandAttachmentStatus[] = [];
    gltf.failed.add(sword.weaponUrl);

    await ReactThreeTestRenderer.create(
      <MainHandAttachmentSlot
        key={sword.ref}
        characterRoot={root}
        presentation={sword}
        onStatus={(status) => statuses.push(status)}
      />
    );

    expect(hand.children).toHaveLength(0);
    expect(statuses.at(-1)).toMatchObject({
      code: 'asset-load-failed',
      ref: sword.ref,
      weaponUrl: sword.weaponUrl,
    });
  });
});
```

- [ ] **Step 2: Write the failing real fighter resilience test**

Create `src/components/hex-grid/ClassCharacterModel.test.tsx`. Mock fighter `useGLTF` with a visible mesh plus `Hand_R`, fail only the weapon URL, and assert the fighter mesh remains in the rendered graph while status is `asset-load-failed`:

```tsx
import ReactThreeTestRenderer from '@react-three/test-renderer';
import * as THREE from 'three';
import { describe, expect, it, vi } from 'vitest';
import type { MainHandAttachmentStatus } from './mainHandPresentation';

const failedWeapon = '/models/synty/characters/weapons/fighter-weapon.glb';

vi.mock('@react-three/drei', () => ({
  useGLTF: (url: string) => {
    if (url === failedWeapon) throw new Error(`failed ${url}`);
    const scene = new THREE.Group();
    const root = new THREE.Group();
    root.name = 'Root';
    const hand = new THREE.Bone();
    hand.name = 'Hand_R';
    const body = new THREE.Mesh(
      new THREE.BoxGeometry(),
      new THREE.MeshStandardMaterial()
    );
    body.name = 'fighter-body';
    root.add(hand, body);
    scene.add(root);
    return { scene, animations: [] };
  },
  useAnimations: () => ({ actions: {}, names: [] }),
}));

import { ClassCharacterModel } from './ClassCharacterModel';

it('keeps the fighter rendered when only its weapon load fails', async () => {
  const statuses: MainHandAttachmentStatus[] = [];
  const renderer = await ReactThreeTestRenderer.create(
    <ClassCharacterModel
      url="/models/synty/characters/fighter.glb"
      mainHandPresentation={{
        ref: 'dnd5e:item:longsword',
        weaponUrl: failedWeapon,
        socket: {
          bone: 'Hand_R',
          boneUnitMeters: 0.01,
          positionMeters: [-0.0554, 0.1299, 0.0237],
          rotationQuaternion: [
            -0.7071067811865475,
            0,
            0,
            0.7071067811865476,
          ],
          scale: 1,
        },
      }}
      onMainHandStatus={(status) => statuses.push(status)}
    />
  );

  expect(
    renderer.scene.findAll(
      (node) =>
        (node.instance as { name?: string } | undefined)?.name ===
        'fighter-body'
    )
  ).toHaveLength(1);
  expect(statuses.at(-1)?.code).toBe('asset-load-failed');
});
```

- [ ] **Step 3: Run both tests and verify RED**

```bash
npm run test:run -- \
  src/components/hex-grid/MainHandAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

Expected: FAIL because `MainHandAttachment` and the new `ClassCharacterModel` props do not exist.

- [ ] **Step 4: Implement the attachment-local React boundary**

Create `src/components/hex-grid/MainHandAttachment.tsx`:

```tsx
import { useGLTF } from '@react-three/drei';
import {
  Component,
  Suspense,
  useEffect,
  useMemo,
  type ReactNode,
} from 'react';
import * as THREE from 'three';
import {
  attachMainHandObject,
  type MainHandAttachmentStatus,
  type MainHandPresentation,
} from './mainHandPresentation';

interface StatusProps {
  status: MainHandAttachmentStatus;
  onStatus?: (status: MainHandAttachmentStatus) => void;
}

function StatusReporter({ status, onStatus }: StatusProps) {
  useEffect(() => onStatus?.(status), [
    onStatus,
    status.code,
    status.ref,
    status.weaponUrl,
    status.bone,
    status.message,
  ]);
  return null;
}

interface LoadedAttachmentProps {
  characterRoot: THREE.Object3D;
  presentation: MainHandPresentation;
  onStatus?: (status: MainHandAttachmentStatus) => void;
}

function LoadedAttachment({
  characterRoot,
  presentation,
  onStatus,
}: LoadedAttachmentProps) {
  const { scene } = useGLTF(presentation.weaponUrl);
  const weapon = useMemo(() => {
    const clone = scene.clone(true);
    clone.traverse((child) => {
      if (child instanceof THREE.Mesh) child.raycast = () => {};
    });
    return clone;
  }, [scene]);

  useEffect(() => {
    const result = attachMainHandObject(
      characterRoot,
      weapon,
      presentation
    );
    onStatus?.(result.status);
    return result.detach;
  }, [characterRoot, onStatus, presentation, weapon]);

  return null;
}

interface ErrorBoundaryProps {
  presentation: MainHandPresentation;
  onStatus?: (status: MainHandAttachmentStatus) => void;
  children: ReactNode;
}

class AttachmentErrorBoundary extends Component<
  ErrorBoundaryProps,
  { failed: boolean }
> {
  state = { failed: false };

  static getDerivedStateFromError() {
    return { failed: true };
  }

  componentDidCatch(error: unknown) {
    const { presentation, onStatus } = this.props;
    onStatus?.({
      code: 'asset-load-failed',
      ref: presentation.ref,
      weaponUrl: presentation.weaponUrl,
      bone: presentation.socket.bone,
      message: error instanceof Error ? error.message : String(error),
    });
  }

  render() {
    return this.state.failed ? null : this.props.children;
  }
}

export interface MainHandAttachmentSlotProps {
  characterRoot: THREE.Object3D;
  presentation?: MainHandPresentation;
  onStatus?: (status: MainHandAttachmentStatus) => void;
}

export function MainHandAttachmentSlot({
  characterRoot,
  presentation,
  onStatus,
}: MainHandAttachmentSlotProps) {
  if (!presentation) {
    return <StatusReporter status={{ code: 'unarmed' }} onStatus={onStatus} />;
  }

  const loading: MainHandAttachmentStatus = {
    code: 'loading',
    ref: presentation.ref,
    weaponUrl: presentation.weaponUrl,
    bone: presentation.socket.bone,
  };
  return (
    <AttachmentErrorBoundary
      presentation={presentation}
      onStatus={onStatus}
    >
      <Suspense
        fallback={<StatusReporter status={loading} onStatus={onStatus} />}
      >
        <LoadedAttachment
          characterRoot={characterRoot}
          presentation={presentation}
          onStatus={onStatus}
        />
      </Suspense>
    </AttachmentErrorBoundary>
  );
}
```

- [ ] **Step 5: Integrate the optional slot into `ClassCharacterModel`**

Modify `src/components/hex-grid/ClassCharacterModel.tsx`:

1. Import `MainHandAttachmentSlot` and the two shared types.
2. Extend `ClassCharacterModelProps`:

```ts
mainHandPresentation?: MainHandPresentation;
onMainHandStatus?: (status: MainHandAttachmentStatus) => void;
```

3. Destructure both props.
4. Replace the single returned `<primitive>` with this fragment:

```tsx
return (
  <>
    <primitive
      object={cloned}
      scale={SYNTY_SCALE}
      rotation={[0, facingRotation, 0]}
    />
    <MainHandAttachmentSlot
      key={
        mainHandPresentation
          ? `${mainHandPresentation.ref}|${mainHandPresentation.weaponUrl}`
          : 'unarmed'
      }
      characterRoot={cloned}
      presentation={mainHandPresentation}
      onStatus={onMainHandStatus}
    />
  </>
);
```

Do not move cloning, tinting, animation, warning, or `useFrame` logic.

- [ ] **Step 6: Run focused integration tests and existing regressions**

```bash
npm run test:run -- \
  src/components/hex-grid/mainHandPresentation.test.ts \
  src/components/hex-grid/MainHandAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/components/hex-grid/HexEntity.test.tsx \
  src/components/session/SessionCanvas.test.tsx
npm run typecheck
```

Expected: all listed tests and typecheck pass. Existing `HexEntity` callers compile unchanged because both new props are optional.

- [ ] **Step 7: Commit the shared renderer seam**

```bash
git add src/components/hex-grid/MainHandAttachment.tsx \
  src/components/hex-grid/MainHandAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
git commit -m "feat(models): attach optional weapon to cloned hand bone (#821)"
```

---

### Task 3: Provisional Fighter Fixtures and Verdict Coverage

**Files:**
- Create: `src/concepts/weapon-attachment/weaponAttachmentExperiment.ts`
- Create: `src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts`

**Interfaces:**
- Consumes: `EquippedMap`, `RefLike`, `refKey()`, `MainHandPresentation`, `MainHandAttachmentCode`.
- Produces:
  - `WEAPON_ATTACHMENT_FIXTURES`
  - `PROVISIONAL_FIGHTER_SOCKET`
  - `resolveProvisionalMainHand(equipped): MainHandResolution`
  - `WeaponRenderObservation`
  - `coverageFor(observations): WeaponConceptCoverage`
  - `canRecordWeaponVerdict(observations): boolean`
  - `weaponConceptVerdict(observations): WeaponConceptVerdict`

- [ ] **Step 1: Verify the provisional assets and socket starting point in Blender**

This checkpoint was completed collaboratively in Blender 5.0.1 on 2026-08-26 before Task 3 dispatch. The disposable scene is saved at `/tmp/rpg-821-fighter-weapon-concept.blend`; Blender MCP protocol 4 recorded Kirk's manual move/rotate operations.

Human visual rulings:

- `SM_Wep_Slayer_01` follows the socket but is rejected as the production longsword candidate because it is the oversized prior large-model weapon.
- `SM_Prop_Bow_01` is accepted as the provisional shortbow candidate for the Concept.
- The accepted provisional fighter socket is:
  - position meters `[-0.11356719583272934, 0.04377313703298569, -0.0070696864277124405]`;
  - rotation quaternion `[-0.5601389408111572, -0.8049638271331787, 0.16070428490638733, 0.11158794164657593]`;
  - weapon scale `1.0`;
  - bone unit meters `0.01`.
- Fingers do not curl around the grip in the existing idle pose; finger posing belongs to the later weapon-animation/hand-pose layer and does not block rigid attachment.

Use these values verbatim in the provisional fixture. Do not repeat calibration, choose a replacement provider asset, modify provider files, or add per-weapon offsets in this Concept slice.

- [ ] **Step 2: Write failing resolver and coverage tests**

Create `src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts`:

```ts
import { describe, expect, it } from 'vitest';
import type { EquippedMap } from '@/components/game/equipment/equipmentTypes';
import {
  PROVISIONAL_FIGHTER_SOCKET,
  WEAPON_ATTACHMENT_FIXTURES,
  canRecordWeaponVerdict,
  coverageFor,
  resolveProvisionalMainHand,
  type WeaponRenderObservation,
} from './weaponAttachmentExperiment';

const ref = (type: string, id: string) => ({
  module: 'dnd5e',
  type,
  id,
});
const equipped = (mainHand?: ReturnType<typeof ref>): EquippedMap =>
  mainHand ? { main_hand: mainHand } : {};

it('maps only exact equipped item refs to provisional assets', () => {
  expect(
    resolveProvisionalMainHand(equipped(ref('item', 'longsword')))
  ).toMatchObject({
    code: 'mapped',
    ref: 'dnd5e:item:longsword',
    presentation: {
      weaponUrl: '/models/synty/characters/weapons/fighter-weapon.glb',
    },
  });
  expect(
    resolveProvisionalMainHand(equipped(ref('item', 'shortbow')))
  ).toMatchObject({
    code: 'mapped',
    ref: 'dnd5e:item:shortbow',
    presentation: {
      weaponUrl: '/models/synty/characters/weapons/bow-01.glb',
    },
  });
  expect(
    resolveProvisionalMainHand(equipped(ref('weapons', 'longsword')))
  ).toMatchObject({
    code: 'unmapped-ref',
    ref: 'dnd5e:weapons:longsword',
  });
});

it('treats absent main_hand as intentionally unarmed', () => {
  expect(resolveProvisionalMainHand({})).toEqual({ code: 'unarmed' });
});

it('uses one frozen fighter socket object for both candidates', () => {
  const sword = WEAPON_ATTACHMENT_FIXTURES.longsword.equipped.main_hand!;
  const bow = WEAPON_ATTACHMENT_FIXTURES.shortbow.equipped.main_hand!;
  const swordResult = resolveProvisionalMainHand({ main_hand: sword });
  const bowResult = resolveProvisionalMainHand({ main_hand: bow });
  expect(swordResult.presentation?.socket).toBe(PROVISIONAL_FIGHTER_SOCKET);
  expect(bowResult.presentation?.socket).toBe(PROVISIONAL_FIGHTER_SOCKET);
  expect(PROVISIONAL_FIGHTER_SOCKET).toEqual({
    bone: 'Hand_R',
    boneUnitMeters: 0.01,
    positionMeters: [
      -0.11356719583272934,
      0.04377313703298569,
      -0.0070696864277124405,
    ],
    rotationQuaternion: [
      -0.5601389408111572,
      -0.8049638271331787,
      0.16070428490638733,
      0.11158794164657593,
    ],
    scale: 1,
  });
  expect(swordResult.candidate).toMatchObject({
    source: 'SM_Wep_Slayer_01 · rejected oversized longsword candidate',
    decodedTextureMb: 16,
    budgetMb: 4.5,
  });
  expect(bowResult.candidate).toMatchObject({
    source: 'SM_Prop_Bow_01 · accepted provisional shortbow candidate',
    decodedTextureMb: 64,
    budgetMb: 4.5,
  });
});

const observation = (
  changes: Partial<WeaponRenderObservation>
): WeaponRenderObservation => ({
  equipmentState: 'unarmed',
  motion: 'idle',
  view: 'close',
  facing: 0,
  attachmentCode: 'unarmed',
  ...changes,
});

it('requires each dimension, not the 108-case Cartesian product', () => {
  const observations: WeaponRenderObservation[] = [
    observation({ equipmentState: 'unarmed', attachmentCode: 'unarmed' }),
    observation({
      equipmentState: 'longsword',
      motion: 'walk',
      view: 'orbit',
      facing: 1,
      attachmentCode: 'attached',
    }),
    observation({
      equipmentState: 'shortbow',
      view: 'play',
      facing: 2,
      attachmentCode: 'attached',
    }),
    observation({ facing: 3 }),
    observation({ facing: 4 }),
    observation({ facing: 5 }),
  ];

  expect(coverageFor(observations)).toEqual({
    equipmentStates: ['unarmed', 'longsword', 'shortbow'],
    motions: ['idle', 'walk'],
    views: ['close', 'orbit', 'play'],
    facings: [0, 1, 2, 3, 4, 5],
  });
  expect(canRecordWeaponVerdict(observations)).toBe(true);
});

it('does not credit a mapped weapon until its status is attached', () => {
  const observations = [
    observation({ equipmentState: 'unarmed' }),
    observation({
      equipmentState: 'longsword',
      attachmentCode: 'loading',
    }),
    observation({
      equipmentState: 'shortbow',
      attachmentCode: 'asset-load-failed',
    }),
  ];
  expect(coverageFor(observations).equipmentStates).toEqual(['unarmed']);
  expect(canRecordWeaponVerdict(observations)).toBe(false);
});
```

- [ ] **Step 3: Run the test and verify RED**

```bash
npm run test:run -- src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts
```

Expected: FAIL because the experiment module does not exist.

- [ ] **Step 4: Implement exact provisional fixtures, resolver, and coverage**

Create `src/concepts/weapon-attachment/weaponAttachmentExperiment.ts` with:

```ts
import {
  refKey,
  type EquippedMap,
  type RefLike,
} from '@/components/game/equipment/equipmentTypes';
import type {
  MainHandAttachmentCode,
  MainHandPresentation,
  MainHandSocket,
} from '@/components/hex-grid/mainHandPresentation';

export type WeaponEquipmentState = 'unarmed' | 'longsword' | 'shortbow';
export type WeaponMotion = 'idle' | 'walk';
export type WeaponView = 'close' | 'orbit' | 'play';
export type WeaponFacing = 0 | 1 | 2 | 3 | 4 | 5;

const itemRef = (id: string): RefLike => ({
  module: 'dnd5e',
  type: 'item',
  id,
});

export const PROVISIONAL_FIGHTER_SOCKET: MainHandSocket = Object.freeze({
  bone: 'Hand_R',
  boneUnitMeters: 0.01,
  positionMeters: [
    -0.11356719583272934,
    0.04377313703298569,
    -0.0070696864277124405,
  ],
  rotationQuaternion: [
    -0.5601389408111572,
    -0.8049638271331787,
    0.16070428490638733,
    0.11158794164657593,
  ],
  scale: 1,
});

interface Candidate {
  ref: string;
  source: string;
  weaponUrl: string;
  decodedTextureMb: number;
  budgetMb: 4.5;
}

const CANDIDATES: Record<string, Candidate> = {
  'dnd5e:item:longsword': {
    ref: 'dnd5e:item:longsword',
    source: 'SM_Wep_Slayer_01 · rejected oversized longsword candidate',
    weaponUrl: '/models/synty/characters/weapons/fighter-weapon.glb',
    decodedTextureMb: 16,
    budgetMb: 4.5,
  },
  'dnd5e:item:shortbow': {
    ref: 'dnd5e:item:shortbow',
    source: 'SM_Prop_Bow_01 · accepted provisional shortbow candidate',
    weaponUrl: '/models/synty/characters/weapons/bow-01.glb',
    decodedTextureMb: 64,
    budgetMb: 4.5,
  },
};

export const WEAPON_ATTACHMENT_FIXTURES: Record<
  WeaponEquipmentState,
  { label: string; equipped: EquippedMap }
> = {
  unarmed: { label: 'Unarmed', equipped: {} },
  longsword: {
    label: 'Longsword',
    equipped: { main_hand: itemRef('longsword') },
  },
  shortbow: {
    label: 'Shortbow',
    equipped: { main_hand: itemRef('shortbow') },
  },
};

export type MainHandResolution =
  | { code: 'unarmed' }
  | { code: 'unmapped-ref'; ref: string }
  | {
      code: 'mapped';
      ref: string;
      candidate: Candidate;
      presentation: MainHandPresentation;
    };

export function resolveProvisionalMainHand(
  equipped: EquippedMap
): MainHandResolution {
  const ref = equipped.main_hand;
  if (!ref) return { code: 'unarmed' };
  const key = refKey(ref);
  const candidate = CANDIDATES[key];
  if (!candidate) return { code: 'unmapped-ref', ref: key };
  return {
    code: 'mapped',
    ref: key,
    candidate,
    presentation: {
      ref: key,
      weaponUrl: candidate.weaponUrl,
      socket: PROVISIONAL_FIGHTER_SOCKET,
    },
  };
}

export interface WeaponRenderObservation {
  equipmentState: WeaponEquipmentState;
  motion: WeaponMotion;
  view: WeaponView;
  facing: WeaponFacing;
  attachmentCode: MainHandAttachmentCode;
}

export interface WeaponConceptCoverage {
  equipmentStates: WeaponEquipmentState[];
  motions: WeaponMotion[];
  views: WeaponView[];
  facings: WeaponFacing[];
}

const EQUIPMENT_ORDER: WeaponEquipmentState[] = [
  'unarmed',
  'longsword',
  'shortbow',
];
const MOTION_ORDER: WeaponMotion[] = ['idle', 'walk'];
const VIEW_ORDER: WeaponView[] = ['close', 'orbit', 'play'];
const FACING_ORDER: WeaponFacing[] = [0, 1, 2, 3, 4, 5];

const validEquipmentObservation = (row: WeaponRenderObservation): boolean =>
  row.equipmentState === 'unarmed'
    ? row.attachmentCode === 'unarmed'
    : row.attachmentCode === 'attached';

export function coverageFor(
  observations: readonly WeaponRenderObservation[]
): WeaponConceptCoverage {
  const valid = observations.filter(validEquipmentObservation);
  return {
    equipmentStates: EQUIPMENT_ORDER.filter((value) =>
      valid.some((row) => row.equipmentState === value)
    ),
    motions: MOTION_ORDER.filter((value) =>
      valid.some((row) => row.motion === value)
    ),
    views: VIEW_ORDER.filter((value) =>
      valid.some((row) => row.view === value)
    ),
    facings: FACING_ORDER.filter((value) =>
      valid.some((row) => row.facing === value)
    ),
  };
}

export function canRecordWeaponVerdict(
  observations: readonly WeaponRenderObservation[]
): boolean {
  const coverage = coverageFor(observations);
  return (
    coverage.equipmentStates.length === EQUIPMENT_ORDER.length &&
    coverage.motions.length === MOTION_ORDER.length &&
    coverage.views.length === VIEW_ORDER.length &&
    coverage.facings.length === FACING_ORDER.length
  );
}

export interface WeaponConceptVerdict {
  warning: 'NON-PRODUCTION CONCEPT EVIDENCE';
  fighterModel: '/models/synty/characters/fighter.glb';
  socket: MainHandSocket;
  candidates: Candidate[];
  coverage: WeaponConceptCoverage;
}

export function weaponConceptVerdict(
  observations: readonly WeaponRenderObservation[]
): WeaponConceptVerdict {
  if (!canRecordWeaponVerdict(observations)) {
    throw new Error('weapon concept coverage is incomplete');
  }
  return {
    warning: 'NON-PRODUCTION CONCEPT EVIDENCE',
    fighterModel: '/models/synty/characters/fighter.glb',
    socket: PROVISIONAL_FIGHTER_SOCKET,
    candidates: Object.values(CANDIDATES),
    coverage: coverageFor(observations),
  };
}
```

- [ ] **Step 5: Run resolver/coverage tests and typecheck**

```bash
npm run test:run -- src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts
npm run typecheck
```

Expected: all tests pass; no alternate attack ref or per-candidate socket appears.

- [ ] **Step 6: Commit the provisional experiment contract**

```bash
git add src/concepts/weapon-attachment/weaponAttachmentExperiment.ts \
  src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts
git commit -m "concept(models): define fighter weapon fixtures and verdict gate (#821)"
```

---

### Task 4: Real Three.js Concepts Preview

**Files:**
- Create: `src/concepts/weapon-attachment/WeaponAttachmentPreview.tsx`
- Create: `src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx`

**Interfaces:**
- Consumes: `ClassCharacterModel`, `MainHandPresentation`, Task 3 state types, `facingToRotationY`, shared tactical camera constants.
- Produces:
  - `WeaponAttachmentPreviewProps`
  - `WeaponAttachmentScene`
  - post-commit `onRenderObserved(observation)` only for stable `unarmed`/`attached` states.

- [ ] **Step 1: Write the failing preview test**

Mock `ClassCharacterModel` as a status-emitting R3F group so the test exercises the preview's real camera/state projection without loading assets. Create `src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx`:

```tsx
import ReactThreeTestRenderer from '@react-three/test-renderer';
import { useEffect } from 'react';
import { describe, expect, it, vi } from 'vitest';
import type { MainHandAttachmentStatus } from '@/components/hex-grid/mainHandPresentation';
import { resolveProvisionalMainHand } from './weaponAttachmentExperiment';

const modelStatus = vi.hoisted(() => ({
  code: 'attached' as MainHandAttachmentStatus['code'],
}));

vi.mock('@/components/hex-grid/ClassCharacterModel', () => ({
  ClassCharacterModel: (props: {
    isMoving: boolean;
    facingRotation: number;
    mainHandPresentation?: { ref: string };
    onMainHandStatus?: (status: MainHandAttachmentStatus) => void;
  }) => {
    useEffect(() => {
      props.onMainHandStatus?.({
        code: props.mainHandPresentation ? modelStatus.code : 'unarmed',
        ref: props.mainHandPresentation?.ref,
      });
    }, [props]);
    return (
      <group
        name="mock-real-class-character-model"
        userData={{
          isMoving: props.isMoving,
          facingRotation: props.facingRotation,
          ref: props.mainHandPresentation?.ref ?? 'unarmed',
        }}
      />
    );
  },
}));

import { WeaponAttachmentScene } from './WeaponAttachmentPreview';

it('projects walk, facing, mapped weapon, and close camera into the shared renderer', async () => {
  const mapped = resolveProvisionalMainHand({
    main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
  });
  if (mapped.code !== 'mapped') throw new Error('fixture must map');
  const onRenderObserved = vi.fn();
  const renderer = await ReactThreeTestRenderer.create(
    <WeaponAttachmentScene
      equipmentState="longsword"
      motion="walk"
      view="close"
      facing={3}
      presentation={mapped.presentation}
      onAttachmentStatus={() => {}}
      onRenderObserved={onRenderObserved}
    />
  );

  const model = renderer.scene.findAll(
    (node) => node.props.name === 'mock-real-class-character-model'
  )[0]!;
  expect(model.props.userData).toMatchObject({
    isMoving: true,
    ref: 'dnd5e:item:longsword',
  });
  expect(model.props.userData.facingRotation).toBeCloseTo(-Math.PI, 5);
  expect(
    renderer.scene.findAll(
      (node) => node.props.name === 'weapon-attachment-close-camera'
    )
  ).toHaveLength(1);
  expect(onRenderObserved).toHaveBeenCalledWith({
    equipmentState: 'longsword',
    motion: 'walk',
    view: 'close',
    facing: 3,
    attachmentCode: 'attached',
  });
});

it('does not acknowledge failure, then acknowledges recovery to the same tuple', async () => {
  const mapped = resolveProvisionalMainHand({
    main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
  });
  if (mapped.code !== 'mapped') throw new Error('fixture must map');
  const onRenderObserved = vi.fn();
  modelStatus.code = 'asset-load-failed';
  const props = {
    equipmentState: 'longsword' as const,
    motion: 'idle' as const,
    view: 'orbit' as const,
    facing: 0 as const,
    presentation: mapped.presentation,
    onAttachmentStatus: () => {},
    onRenderObserved,
  };
  const renderer = await ReactThreeTestRenderer.create(
    <WeaponAttachmentScene {...props} />
  );
  expect(onRenderObserved).not.toHaveBeenCalled();

  modelStatus.code = 'attached';
  await renderer.update(<WeaponAttachmentScene {...props} />);
  expect(onRenderObserved).toHaveBeenCalledWith({
    equipmentState: 'longsword',
    motion: 'idle',
    view: 'orbit',
    facing: 0,
    attachmentCode: 'attached',
  });
});
```

- [ ] **Step 2: Run the test and verify RED**

```bash
npm run test:run -- src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx
```

Expected: FAIL because `WeaponAttachmentPreview` does not exist.

- [ ] **Step 3: Implement the preview with exact camera branches**

Create `src/concepts/weapon-attachment/WeaponAttachmentPreview.tsx`:

- `WeaponAttachmentScene` renders the real `ClassCharacterModel` with fighter URL `/models/synty/characters/fighter.glb`, `isMoving={motion === 'walk'}`, `facingRotation={facingToRotationY(facing)}`, `mainHandPresentation`, and the status callback.
- Use `PerspectiveCamera` + `OrbitControls` for `orbit` at `[3.2, 2.4, 4.2]`, target `[0, 0.95, 0]`.
- Use a close `PerspectiveCamera` named `weapon-attachment-close-camera` at `[-1.45, 1.45, 1.15]`, looking at the known fighter right-hand region `[-0.8, 1.30, 0]`.
- Use `sphericalCameraPosition`, `POLAR_ANGLE`, `INITIAL_AZIMUTH`, `INITIAL_DISTANCE`, `ORTHO_ZOOM`, `ORTHO_NEAR`, and `ORTHO_FAR` from `@/author/preview3d/playCameraRig` for the named tactical orthographic camera.
- Render ambient light `0.9`, directional light at `[5, 8, 4]` intensity `1.1`, and a grid helper.
- Keep `Canvas frameloop="demand" dpr={[1, 1.5]}`; `ClassCharacterModel` already invalidates while a clip plays.
- Keep the character under `Suspense fallback={null}`.
- Track the latest attachment status in scene state. Emit `onRenderObserved` from an effect only when:
  - selected state is unarmed and status is `unarmed`; or
  - selected state is longsword/shortbow and status is `attached`.
- Do not expose a forced-status prop. Tests control emitted status inside the mocked `ClassCharacterModel`, leaving the preview API production-shaped.
- When status is loading, failed, missing, or otherwise invalid for the selected equipment state, clear the observation dedupe key before returning. Recovery to the same stable tuple must emit a fresh observation.
- Add focused assertions for a valid unarmed observation and the named orbit/tactical camera branches.

The state effect must emit exactly:

```ts
{
  equipmentState,
  motion,
  view,
  facing,
  attachmentCode: stableStatus.code,
}
```

- [ ] **Step 4: Run the preview test and renderer regressions**

```bash
npm run test:run -- \
  src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/author/AssetAnchorLabPreview.test.tsx
npm run typecheck
```

Expected: all tests and typecheck pass.

- [ ] **Step 5: Commit the real preview**

```bash
git add src/concepts/weapon-attachment/WeaponAttachmentPreview.tsx \
  src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx
git commit -m "concept(models): render fighter weapon proof in Three.js (#821)"
```

---

### Task 5: Concepts Lab Controls, Inspector, and Route

**Files:**
- Create: `src/concepts/weapon-attachment/WeaponAttachmentConcept.tsx`
- Create: `src/concepts/weapon-attachment/WeaponAttachmentConcept.test.tsx`
- Modify: `src/concepts/ConceptsView.tsx`
- Modify: `docs/architecture/components/concepts-route.md`
- Modify: `docs/how-to/concepts-route.md`

**Interfaces:**
- Consumes: Task 3 fixtures/resolver/coverage/verdict and Task 4 preview.
- Produces: `WeaponAttachmentConcept`, deep link `?concept=weapon-attachment`, gated non-production verdict JSON.

- [ ] **Step 1: Write the failing DOM interaction test**

Mock `WeaponAttachmentPreview` to emit a valid observation for the selected state after each control change:

```tsx
vi.mock('./WeaponAttachmentPreview', () => ({
  WeaponAttachmentPreview: (props: {
    equipmentState: 'unarmed' | 'longsword' | 'shortbow';
    motion: 'idle' | 'walk';
    view: 'close' | 'orbit' | 'play';
    facing: 0 | 1 | 2 | 3 | 4 | 5;
    presentation?: { ref: string };
    onAttachmentStatus: (status: { code: 'unarmed' | 'attached' }) => void;
    onRenderObserved: (observation: {
      equipmentState: 'unarmed' | 'longsword' | 'shortbow';
      motion: 'idle' | 'walk';
      view: 'close' | 'orbit' | 'play';
      facing: 0 | 1 | 2 | 3 | 4 | 5;
      attachmentCode: 'unarmed' | 'attached';
    }) => void;
  }) => {
    const {
      equipmentState,
      motion,
      view,
      facing,
      presentation,
      onAttachmentStatus,
      onRenderObserved,
    } = props;
    const code = presentation ? 'attached' : 'unarmed';
    useEffect(() => {
      onAttachmentStatus({ code });
      onRenderObserved({
        equipmentState,
        motion,
        view,
        facing,
        attachmentCode: code,
      });
    }, [
      code,
      equipmentState,
      facing,
      motion,
      onAttachmentStatus,
      onRenderObserved,
      view,
    ]);
    return (
      <div data-testid="mock-weapon-preview">
        {props.equipmentState}|{props.motion}|{props.view}|{props.facing}
      </div>
    );
  },
}));
```

The test must assert:

```tsx
render(<WeaponAttachmentConcept />);
expect(screen.getByRole('heading', { name: /Equipped Weapon Lab/i })).toBeTruthy();
expect(screen.getByTestId('equipped-ref').textContent).toContain('unarmed');

fireEvent.click(screen.getByRole('button', { name: 'Longsword' }));
expect(screen.getByTestId('equipped-ref').textContent).toContain(
  'dnd5e:item:longsword'
);
expect(screen.getByTestId('candidate-source').textContent).toContain(
  'SM_Wep_Slayer_01'
);
expect(screen.getByTestId('texture-warning').textContent).toContain(
  '16 MB > 4.5 MB'
);

fireEvent.click(screen.getByRole('button', { name: 'Shortbow' }));
expect(screen.getByTestId('equipped-ref').textContent).toContain(
  'dnd5e:item:shortbow'
);
expect(screen.getByTestId('texture-warning').textContent).toContain(
  '64 MB > 4.5 MB'
);

fireEvent.click(screen.getByRole('button', { name: 'Walk' }));
fireEvent.click(screen.getByRole('button', { name: 'Full orbit' }));
fireEvent.click(screen.getByRole('button', { name: 'Tactical play' }));
for (const label of ['E', 'NE', 'NW', 'W', 'SW', 'SE']) {
  fireEvent.click(screen.getByRole('button', { name: `Facing ${label}` }));
}
fireEvent.click(screen.getByRole('button', { name: 'Unarmed' }));

const record = screen.getByRole('button', {
  name: 'Record non-production verdict',
}) as HTMLButtonElement;
expect(record.disabled).toBe(false);
fireEvent.click(record);
expect(screen.getByTestId('provisional-verdict').textContent).toContain(
  'NON-PRODUCTION CONCEPT EVIDENCE'
);
```

Also assert there are no controls whose accessible names contain `position`, `rotation`, `scale`, `nudge`, or `transform`.

- [ ] **Step 2: Run the Concept test and verify RED**

```bash
npm run test:run -- src/concepts/weapon-attachment/WeaponAttachmentConcept.test.tsx
```

Expected: FAIL because the Concept component does not exist.

- [ ] **Step 3: Implement the Concept composition**

Create `src/concepts/weapon-attachment/WeaponAttachmentConcept.tsx` with these exact controls and labels:

- Equipment: `Unarmed`, `Longsword`, `Shortbow`.
- Motion: `Idle`, `Walk`.
- View: `Hand close-up`, `Full orbit`, `Tactical play`.
- Facing: `E`, `NE`, `NW`, `W`, `SW`, `SE` using indices 0–5.
- Verdict button: `Record non-production verdict`.

Keep a deduplicated `WeaponRenderObservation[]`; use the key
`equipmentState|motion|view|facing|attachmentCode` so repeated renders do not inflate coverage. Resolve the selected fixture inside `useMemo(..., [equipmentState])` with `resolveProvisionalMainHand`; this keeps the presentation object stable when attachment status updates trigger a parent render. Pass only `resolution.code === 'mapped' ? resolution.presentation : undefined` to the preview. Pass `useCallback`-stable status and observation handlers so the loaded attachment effect does not detach/re-attach and the mocked/real observation effect does not re-fire solely because the inspector updated.

Support reproducible evidence deep links with allowlisted initial-only query parameters:

```ts
function readEnumParam<const Values extends readonly string[]>(
  name: string,
  values: Values,
  fallback: Values[number]
): Values[number] {
  if (typeof window === 'undefined') return fallback;
  const value = new URLSearchParams(window.location.search).get(name);
  return value !== null && values.some((candidate) => candidate === value)
    ? (value as Values[number])
    : fallback;
}

function readFacingParam(name: string, fallback: WeaponFacing): WeaponFacing {
  if (typeof window === 'undefined') return fallback;
  const value = new URLSearchParams(window.location.search).get(name);
  return value !== null && /^[0-5]$/.test(value)
    ? (Number(value) as WeaponFacing)
    : fallback;
}

const initialEquipment = readEnumParam(
  'weapon',
  ['unarmed', 'longsword', 'shortbow'] as const,
  'unarmed'
);
const initialMotion = readEnumParam(
  'motion',
  ['idle', 'walk'] as const,
  'idle'
);
const initialView = readEnumParam(
  'view',
  ['close', 'orbit', 'play'] as const,
  'play'
);
const initialFacing = readFacingParam('facing', 0);
```

`readEnumParam` returns the fallback unless the URL value is exactly in the supplied readonly list. `readFacingParam` accepts only integer strings `0` through `5`. Parameters initialize local state once; control clicks remain local fixture state and never write the URL or production state.

The inspector must expose these `data-testid` values:

- `equipped-ref`: exact mapped/unmapped ref or `unarmed`;
- `candidate-source`: exact source or `none`;
- `candidate-url`: exact URL or `none`;
- `socket-profile`: `Hand_R · bone units 0.01m · pos [-0.113567, 0.043773, -0.007070] · quat [-0.560139, -0.804964, 0.160704, 0.111588] · scale 1`;
- `attachment-status`: latest callback code;
- `texture-warning`: `16 MB > 4.5 MB production budget` or `64 MB > 4.5 MB production budget`, and `none` for unarmed;
- `coverage-status`: counts `equipment 0/3 · motion 0/2 · views 0/3 · facings 0/6`, updated from `coverageFor()`;
- `provisional-verdict`: rendered only after the enabled button calls `weaponConceptVerdict()`.

The page header/subtitle must state `Equipped Weapon Lab · Concept` and `actual shared ClassCharacterModel · provisional private assets · no writer`.

- [ ] **Step 4: Register the deep link**

Modify `src/concepts/ConceptsView.tsx`:

- import `WeaponAttachmentConcept`;
- add `'weapon-attachment'` to `ConceptPage`;
- add `{ id: 'weapon-attachment', label: 'Weapon Attachment' }` to `CONCEPT_PAGES`;
- render `<WeaponAttachmentConcept />` for that active page.

Add focused route assertions to `WeaponAttachmentConcept.test.tsx`:

1. Set `window.history.replaceState({}, '', '/?concept=weapon-attachment')`, render `<ConceptsView onBack={() => {}} />`, and assert the heading is present.
2. Set `/?concept=weapon-attachment&weapon=shortbow&motion=walk&view=close&facing=4`, render the Concept, and assert the inspector/preview start at shortbow, walk, close, facing 4.
3. Set invalid values `weapon=axe&motion=run&view=side&facing=9` and assert the documented fallbacks unarmed, idle, play, facing 0.

- [ ] **Step 5: Update Concepts documentation**

Add to `docs/architecture/components/concepts-route.md` and `docs/how-to/concepts-route.md`:

- issue `rpg-dnd5e-web#821`;
- deep link `?concept=weapon-attachment`;
- real shared `ClassCharacterModel` + provisional equipped fixtures;
- exact scope: fighter/main hand/unarmed-longsword-shortbow/idle-walk;
- provider gaps: final semantic asset selection, normalized production exports, socket receipt, and texture budget;
- no production writer or live equipment wiring.

Do not call the Concept accepted until Task 6 records Kirk's viewed verdict.

- [ ] **Step 6: Run all Concept tests and documentation checks**

```bash
npm run test:run -- \
  src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts \
  src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx \
  src/concepts/weapon-attachment/WeaponAttachmentConcept.test.tsx
npm run format:check -- \
  src/concepts/weapon-attachment \
  src/concepts/ConceptsView.tsx \
  docs/architecture/components/concepts-route.md \
  docs/how-to/concepts-route.md
npm run typecheck
```

Expected: focused tests, formatting, and typecheck pass.

- [ ] **Step 7: Commit the registered Concept**

```bash
git add src/concepts/weapon-attachment \
  src/concepts/ConceptsView.tsx \
  docs/architecture/components/concepts-route.md \
  docs/how-to/concepts-route.md
git commit -m "concept(models): add fighter equipped-weapon lab (#821)"
```

---

### Task 6: Actual-Asset Walk, Evidence, and Concept Verdict

**Files:**
- Create: `src/concepts/weapon-attachment/CONTRACT.md`
- Create: `docs/evidence/821-weapon-attachment/README.md`
- Create after viewing: screenshots under `docs/evidence/821-weapon-attachment/`
- Modify from the pre-walk evidence gate: `src/concepts/weapon-attachment/WeaponAttachmentPreview.tsx` and `.test.tsx`.
- Modify only if later observed evidence requires it: provisional values in `weaponAttachmentExperiment.ts` and their exact tests.

**Interfaces:**
- Consumes: actual synced private GLBs, live Concepts deep link, gated verdict JSON.
- Produces: viewed visual verdict and exact provider findings; no provider manifest or production wiring.

- [ ] **Step 0: Correct the preview framing found by the first actual-browser probe**

The 2026-08-26 pre-walk screenshot showed the browser using the Canvas default height (~150px), making every model too small and putting the scaled fighter's hand below the pre-scale close-camera target. This is a Concept defect, not an asset/socket verdict.

TDD requirements:

- wrap `Canvas` in a `div` with `data-testid="weapon-attachment-preview"`, width/height `100%`, and `minHeight: 520`;
- change orbit camera position to `[2.4, 1.8, 3.1]` and target to `[0, 0.7, 0]`;
- change close camera position to `[-1.2, 1.22, 0.85]` and target to the measured post-`SYNTY_SCALE` right-hand region `[-0.6, 1.02, -0.025]`;
- retain the tactical camera's real shared gameplay constants unchanged;
- compute close/orbit/tactical camera orientation declaratively and pass each camera an explicit quaternion tuple `[x, y, z, w]` (not a `THREE.Quaternion` object); R3F applies the tuple through `fromArray`, while the object prop remained identity in the actual browser;
- retain the one post-mount `invalidate()` after drei's `makeDefault` activation so the custom camera's first frame is requested under demand mode;
- add focused test assertions for the explicit preview height, corrected close/orbit positions, and non-identity declarative camera quaternions;
- run preview, concept, and shared character focused tests plus typecheck;
- commit as `fix(concepts): make weapon attachment views judgeable (#821)`.

Do not record screenshots or a visual verdict until Kirk sees the corrected live browser path.

- [ ] **Step 1: Start a clean Concepts server with exact private assets**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/821-weapon-attachment
RPG_GAME_ASSETS_PATH=/home/kirk/game-dev/rpg-game-assets npm run assets:sync
test -f public/models/synty/characters/fighter.glb
test -f public/models/synty/characters/weapons/fighter-weapon.glb
test -f public/models/synty/characters/weapons/bow-01.glb
ss -ltn | grep -q ':3011 ' && { echo 'port 3011 occupied'; exit 1; } || true
npm run dev -- --host 127.0.0.1 --port 3011 \
  > /tmp/rpg-web-821-vite.log 2>&1 &
printf '%s\n' "$!" > /tmp/rpg-web-821-vite.pid
```

Wait until `curl --fail http://127.0.0.1:3011/?concept=weapon-attachment` succeeds. If Vite does not start, inspect `/tmp/rpg-web-821-vite.log`; do not silently change ports without recording the actual URL in evidence.

- [ ] **Step 2: Walk every required dimension with Kirk**

Open:

```text
http://127.0.0.1:3011/?concept=weapon-attachment
```

Observe and record:

1. unarmed fighter in tactical view;
2. longsword hand close-up in Idle;
3. longsword full orbit in Walk;
4. shortbow hand close-up in Idle;
5. shortbow tactical view in Walk;
6. all six facing buttons;
7. inspector's exact ref/source/URL/socket/load/status;
8. both texture budget warnings;
9. enabled non-production verdict only after equipment/motion/view/facing coverage is complete.

Kirk must explicitly rule each provisional candidate `accepted for provider normalization` or `rejected as the final semantic model`. A rejection does not fail the attachment Concept if the asset attached and followed the hand; it becomes the exact Asset Build selection requirement.

- [ ] **Step 3: Capture and inspect evidence**

Capture exact deep links with the workspace screenshot harness:

```bash
root=/home/kirk/.pi/worktrees/rpg-dnd5e-web/821-weapon-attachment
out="$root/docs/evidence/821-weapon-attachment"
mkdir -p "$out"
shot=/home/kirk/game-dev/tools/browser/screenshot.mjs
base='http://127.0.0.1:3011/?concept=weapon-attachment'
node "$shot" "$base&weapon=unarmed&motion=idle&view=play&facing=0" \
  "$out/unarmed-tactical.png"
node "$shot" "$base&weapon=longsword&motion=idle&view=close&facing=0" \
  "$out/longsword-hand-idle.png"
node "$shot" "$base&weapon=longsword&motion=walk&view=orbit&facing=0" \
  "$out/longsword-orbit-walk.png"
node "$shot" "$base&weapon=shortbow&motion=idle&view=close&facing=0" \
  "$out/shortbow-hand-idle.png"
node "$shot" "$base&weapon=shortbow&motion=walk&view=play&facing=0" \
  "$out/shortbow-tactical-walk.png"
for facing in 0 1 2 3 4 5; do
  node "$shot" \
    "$base&weapon=longsword&motion=idle&view=orbit&facing=$facing" \
    "$out/facing-$facing.png"
done
```

Open every resulting PNG with the file/image reader before committing. Delete and recapture blank, loading, fallback-only, stale, or visually clipped images. The gated coverage verdict is exercised manually in Step 2 and pinned by tests; isolated evidence deep links intentionally do not manufacture prior observation history.

- [ ] **Step 4: Record `CONTRACT.md` from observed facts**

Create `src/concepts/weapon-attachment/CONTRACT.md` with these sections and exact observed values:

```markdown
# Fighter Equipped Main-Hand Concept Contract

## Verdict

Kirk's dated verdict for attachment behavior, longsword candidate, and shortbow candidate.

## Proven consumer contract

- exact equipped refs consumed;
- exact shared fighter socket used;
- Blender/Three unit compensation observed;
- idle/walk/facing behavior observed;
- honest unarmed/error behavior proven by tests.

## Provider findings

- accepted/rejected source candidate per canonical ref;
- normalized grip/export work required;
- 16 MB / 64 MB texture-budget failures;
- exact next Asset Build boundary.

## Not requested from Platform

Existing owner `CharacterData.equipped` is sufficient for the acting-player follow-up. No proto/API/toolkit issue is created by this Concept.
```

Replace the descriptive bullets with the actual dated observations; do not leave placeholder markers, speculative provider values, or unviewed claims.

- [ ] **Step 5: Record evidence README and viewed statements**

Create `docs/evidence/821-weapon-attachment/README.md` containing:

- web commit SHA;
- exact `rpg-game-assets` commit SHA and paths;
- Blender 5.0.1 and add-on/server versions used;
- URL and Vite port;
- screenshot filename → what Kirk viewed → verdict;
- exact attachment status for each fixture;
- whether one socket worked for both candidates;
- texture warning values;
- explicit list of deferred work.

- [ ] **Step 6: Re-run the complete verification gate**

```bash
npm run test:run -- \
  src/components/hex-grid/mainHandPresentation.test.ts \
  src/components/hex-grid/MainHandAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/concepts/weapon-attachment/weaponAttachmentExperiment.test.ts \
  src/concepts/weapon-attachment/WeaponAttachmentPreview.test.tsx \
  src/concepts/weapon-attachment/WeaponAttachmentConcept.test.tsx
npm run ci-check
git diff --check
git status --short
```

Expected: focused tests and full CI pass; only intended source/docs/evidence changes remain; no GLB, `.blend`, private manifest, or ignored asset is staged.

- [ ] **Step 7: Commit the observed verdict**

```bash
git add src/concepts/weapon-attachment/CONTRACT.md \
  docs/evidence/821-weapon-attachment
git commit -m "docs(models): record fighter weapon concept verdict (#821)"
```

- [ ] **Step 8: Publish the Concept PR and move Project 19 to In Review**

```bash
git push -u origin concept/821-fighter-weapon-attachment
gh pr create \
  --repo KirkDiggler/rpg-dnd5e-web \
  --base dev \
  --head concept/821-fighter-weapon-attachment \
  --title "concept(models): fighter equipped main-hand attachment" \
  --body-file - <<'EOF'
## Summary

- proves runtime `Hand_R` attachment on the real shared Synty fighter renderer
- exercises unarmed, provisional longsword, and provisional shortbow equipped fixtures
- preserves idle/walk and validates close/orbit/tactical views plus all six facings
- records exact provider findings without writing production asset state

## Evidence

See `docs/evidence/821-weapon-attachment/README.md` and the Concepts deep link `?concept=weapon-attachment`.

## Scope

Concept only: no live equipment wiring, provider promotion, multiplayer propagation, off hand, monsters, extra classes, or combat animation.

Closes #821
Parent journey: KirkDiggler/rpg-project#281
Design: KirkDiggler/rpg-project#283

— assets agent, on behalf of KirkDiggler
EOF

gh project item-edit \
  --project-id PVT_kwHOAASbwc4Bcj4v \
  --id PVTI_lAHOAASbwc4Bcj4vzg4BWnM \
  --field-id PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM \
  --single-select-option-id 9dac2cae
```

- [ ] **Step 9: Complete review, then stop before Asset Build**

Check GitHub checks and inline review comments:

```bash
gh pr checks --repo KirkDiggler/rpg-dnd5e-web --watch
pr_number=$(gh pr view concept/821-fighter-weapon-attachment \
  --repo KirkDiggler/rpg-dnd5e-web \
  --json number --jq .number)
gh api "repos/KirkDiggler/rpg-dnd5e-web/pulls/$pr_number/comments"
```

Address valid findings with focused tests and commits, rerun `npm run ci-check`, and verify remote head equals the reviewed local head.

Stop after the Concept PR is review-ready. Do not create or implement the `rpg-game-assets` Build slice until Kirk accepts the Concept verdict and its exact provider findings.
