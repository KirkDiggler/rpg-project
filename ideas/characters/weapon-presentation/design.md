# Equipped Weapon Presentation — fighter main-hand proof

## Status: Design approved by Kirk 2026-08-26; written review pending

Parent journey: [rpg-project#281](https://github.com/KirkDiggler/rpg-project/issues/281), **See Equipped Weapons in Character Hands**. This Decide slice is [rpg-project#282](https://github.com/KirkDiggler/rpg-project/issues/282).

North star: **a player can tell what their character is wielding by looking at the character in the 3D game, because the model reflects authoritative equipped state rather than a baked default or a client guess.**

The first acceptance case is deliberately narrow: one canonical fighter switches among unarmed, longsword, and shortbow main-hand states in the Concepts Lab, then the validated presentation is promoted so the acting player's live fighter does the same on the game route. Existing idle and walk animation must continue to move the held weapon with `Hand_R`.

## Context — what exists today

Verified 2026-08-26 against `rpg-game-assets` `origin/main`, `rpg-dnd5e-web` `origin/dev`, and the current proto/toolkit/API checkouts:

- Canonical class GLBs are intentionally unarmed. `rpg-game-assets` PR #27 removed baked weapons while retaining standalone weapon files and socket metadata for future equipment-driven presentation.
- `harness/models/synty/characters/manifest.json` currently records class-specific `Hand_R` offsets in a Blender bone-head rest frame. Those values proved the earlier Blender export path, but no Synty web renderer consumes them.
- A direct Three.js GLTFLoader probe of the exact synced files shows `Hand_R` inherits world scale `~0.01` inside the fighter GLB while both standalone weapon roots load at identity scale `1.0`. A bone child therefore needs explicit centimeter-to-meter compensation inside the rig before the outer shared `SYNTY_SCALE` is allowed to scale character and weapon together.
- `scripts/attach_weapon.py` proved that a standalone weapon can be rigidly attached through full vertex weight to one hand bone and survive a Blender GLB export/reimport round trip. That technique solves an exported-GLB interoperability problem; it does not require the runtime consumer to generate another skinned mesh.
- The production Synty character path is `ClassCharacterModel`. It loads a class GLB, uses `SkeletonUtils.clone()` so each instance owns a correct cloned skeleton, and plays baked idle/walk clips. It has no equipment attachment seam.
- The existing `CharacterWeapon` component belongs to the older OBJ `MediumHumanoid` path. It uses character-space offsets and must not be adapted as if it were an animated Synty bone attachment.
- Owner `CharacterData.equipped` already maps `main_hand` and `off_hand` to authoritative item refs. The acting client receives recomputed post-equip state from `EquipItem`/`UnequipItem`.
- Equipment changes are not broadcast to peers today. Immediate multiplayer appearance convergence is a separate session concern, not an asset-contract requirement for this journey.
- `MonsterData` carries only `monster_ref`; monsters expose no equipped or held-weapon state. Any monster weapon selection in the web would therefore be a guess.
- The 17 promoted standalone weapon GLBs each embed a full 2048×2048 or 4096×4096 character atlas. The current fighter weapon costs an estimated 16 MB decoded and each current bow costs 64 MB before mipmaps. Every standalone weapon intentionally fails the 4.5 MB weapon budget.
- Blender 5.0.1 and the loopback-only Blender MCP add-on are verified locally; Kirk also has Auto-Rig Pro available. Blender MCP is suitable for interactive inspection and calibration. Auto-Rig Pro is reserved for later animation authoring/transfer and is not required for this rigid attachment proof.

## Decisions

Kirk approved these decisions in the 2026-08-26 design conversation:

1. **Equipped state only.** Inventory contents and the weapon used by the latest attack do not alter presentation.
2. **Main hand only.** The first proof does not represent off-hand weapons, shields, dual wielding, or a left-hand support pose for a bow.
3. **Players before monsters.** Monsters wait for an authoritative held-equipment signal.
4. **Fighter only.** Broader rig compatibility earns a later slice after one complete vertical proof.
5. **Concept before production.** The Concepts Lab is the outside-in contract bench; it uses the actual shared renderer and real synced assets rather than a throwaway mock renderer.
6. **Blender authors transforms.** The browser validates them but does not provide transform sliders or become an asset editor.
7. **Runtime bone attachment.** The character stays unarmed on disk; the standalone equipped weapon is attached beneath the cloned `Hand_R` bone at runtime.
8. **Normalize weapons, calibrate rigs.** Weapon-specific pivot/orientation correction belongs in each exported weapon. The fighter contributes one reusable main-hand socket transform. A character × weapon offset matrix is prohibited.
9. **Existing idle/walk only.** Sword-combat and bow animation packs are later work. This proof establishes the socket contract those animations will consume.
10. **Acting player first.** The first live promotion updates the acting player's model immediately and restores it on reconnect. Multiplayer propagation is explicitly deferred.

## Considered approaches

### Runtime child of the cloned hand bone — selected

Load an unarmed fighter and a standalone weapon, clone both, find `Hand_R` in the fighter clone, and parent the normalized weapon beneath that bone with one fighter socket transform.

This is the only option that directly matches authoritative runtime swapping without multiplying assets. The weapon naturally follows every existing and future animation that moves `Hand_R`.

### Pre-baked armed character variants — rejected

Generating `fighter-longsword.glb`, `fighter-shortbow.glb`, and equivalent files for every future class, color, pose, and weapon repeats the path deliberately retired by the unarmed promotion. It makes equipment swapping an asset combinatorics problem and cannot be the reusable contract.

### World-space matrix follower — rejected

Rendering the weapon as a scene sibling and copying the hand's world matrix every frame avoids adding it to the bone hierarchy, but introduces continuous synchronization, ordering hazards, and avoidable drift. The hand bone is already the correct scene-graph parent.

## Contract boundaries

The contract separates **what a weapon is** from **where a rig's hand socket is**.

### Normalized weapon definition

A production weapon definition is asset-owned and keyed by the exact ref carried in `CharacterData.equipped`, for example `dnd5e:item:longsword` (`rpg-api` currently projects equipment refs with type `item`). This is intentionally distinct from session `AttackRef.ref`, such as `dnd5e:weapons:longsword`: attack identity does not drive equipped presentation. The equipment ref resolves to one reviewed standalone GLB.

Each promoted weapon must satisfy this grip-space convention:

- glTF/Three.js Y-up coordinates;
- true meter scale before any shared scene scale;
- root origin at the center of the intended grip;
- local `+Y` runs from the grip toward the primary working end (blade tip or upper bow limb);
- local `+Z` records the authored presentation front (the bow's firing direction or the blade's outward-facing broad-face normal);
- an identity root is valid at the canonical grip frame; weapon-specific correction is baked into the exported asset rather than emitted as a web offset;
- no character animation or character armature is embedded in the standalone weapon; and
- exact source, export tool identity, GLB SHA-256, dimensions, material/texture facts, and visual evidence are recorded privately in `rpg-game-assets`.

The first Concepts fixture may resolve `dnd5e:item:longsword` to the already promoted `SM_Wep_Slayer_01`-derived fighter weapon and `dnd5e:item:shortbow` to `bow-01.glb` as explicitly provisional visual candidates. That does not assert either as the final production mapping. The Concept verdict determines whether those candidates are accepted or replaced from the private library before the Asset Build slice is filed.

### Fighter main-hand socket profile

The fighter profile owns exactly:

- semantic socket key `main_hand`;
- bone name `Hand_R`;
- `boneUnitMeters` (exactly `0.01` for the canonical fighter rig);
- joint-local position in meters;
- joint-local rotation as a quaternion `[x, y, z, w]`; and
- uniform true-meter weapon scale (normally `1.0`).

The quaternion is deliberate: it removes Euler order and Blender-extrinsic-versus-Three-intrinsic ambiguity from the runtime contract. Values are measured in the exported glTF joint-local frame, not copied unchecked from the existing Blender bone-head-frame receipt. `boneUnitMeters` is equally deliberate: the runtime divides position and weapon scale by `0.01` when assigning child-local Three.js transforms, compensating only the rig's internal Root scale. The outer `SYNTY_SCALE` then still scales the character and attached weapon together.

One Blender helper, `Socket_MainHand`, is parented under `Hand_R` while calibrating. A deterministic receipt/export step serializes its local transform into the provisional fixture and, after approval, the provider manifest.

Both normalized weapons must fit the same fighter socket. If one needs an item-specific runtime offset, the workflow first treats that as a weapon normalization defect. The Concept may reject a candidate; it must not hide the mismatch by adding a special-case web transform.

### Renderer input

The shared renderer receives presentation data, not rulebook or proto ownership:

```ts
interface MainHandPresentation {
  ref: string;
  weaponUrl: string;
  socket: {
    bone: 'Hand_R';
    boneUnitMeters: 0.01;
    positionMeters: readonly [number, number, number];
    rotationQuaternion: readonly [number, number, number, number];
    scale: number;
  };
}
```

`ClassCharacterModel` (or a focused shared child extracted beside it) consumes `MainHandPresentation | undefined`. It never decides which items are weapons, whether a character may equip them, or which attack is legal. Those are server/rulebook facts already reflected by authoritative equipped state.

## Concepts Lab design

Add a development-only `weapon-attachment` concept registered through `ConceptsView` and deep-linkable as `?concept=weapon-attachment`.

It uses the actual canonical fighter model, `SkeletonUtils.clone()`, the existing animation mixer, shared Synty scale, canonical facing conversion, and tactical camera constants. It does not create a second character renderer.

The bench provides:

- fixture state: Unarmed / Longsword / Shortbow;
- motion state: Idle / Walk;
- view: hand close-up / full-body orbit / tactical play camera;
- all six canonical game facings; and
- a contract inspector showing the fixture's equipped ref, provisional asset source, resolved URL, socket profile, bone lookup, GLB load result, attachment result, and texture-budget warning.

Fixture data mirrors the existing `equipped` map shape. The composition performs one-way projection:

```text
wire-shaped equipped fixture
  -> pure main-hand presentation resolver
  -> MainHandPresentation | undefined
  -> shared animated fighter renderer
  -> cloned fighter Hand_R
```

The Concept's candidate mapping and socket receipt are explicitly provisional. It writes no provider manifest, changes no character record, and cannot silently promote its own values.

## Runtime attachment lifecycle

For one mounted fighter instance:

1. Load the character GLB and create its existing skeleton-safe clone.
2. Resolve the equipped fixture ref through the provisional presentation catalog.
3. If unresolved or empty, render the unarmed fighter and stop.
4. Load and clone the standalone weapon without mutating drei's URL-keyed shared scene.
5. Find the configured bone in the same fighter clone rendered by the component.
6. Validate positive finite `boneUnitMeters`, then apply child-local position `positionMeters / boneUnitMeters`, the socket quaternion, and child-local scale `scale / boneUnitMeters` to the normalized weapon root.
7. Parent the weapon root beneath the bone.
8. On equipped-ref or model change, remove the prior weapon before attaching the replacement.
9. On unmount, detach the per-instance weapon clone and dispose only resources the instance owns; never dispose shared cached geometry or materials.

The weapon is not a raycast target in this slice. Entity interaction remains on `HexEntity`'s stable capsule proxy, avoiding the existing animated-skinned-mesh raycast problem.

## Failure behavior

Failure preserves the character and tells the truth:

| Condition | Result |
| --- | --- |
| `main_hand` absent | Render unarmed; no diagnostic error. |
| Unknown equipped ref | Render unarmed; inspector records `unmapped-ref`; development warning only. |
| Weapon GLB fails to load | Keep the fighter rendered unarmed; attachment-local boundary records `asset-load-failed`. |
| Configured bone absent | Keep the fighter rendered unarmed; record `missing-bone` with the requested bone name. |
| Non-finite/invalid socket transform or non-positive `boneUnitMeters` | Refuse attachment before scene mutation; record `invalid-socket`. |
| Ref changes during load | Stale attachment branch unmounts and cannot attach after the new keyed branch. |
| Candidate needs a special per-item offset | Reject or re-normalize the candidate; do not add a web exception. |

No unknown ref falls back to a visually similar weapon. No attachment failure may take down the fighter, the entity, or the whole Canvas.

## Blender-to-game workflow

### 1. Inspect privately

Use a disposable/private Blender scene. Import the canonical fighter and candidate standalone weapon GLBs from the private provider checkout. Through Blender MCP or direct inspection, verify root hierarchy, meter scale, meshes, materials, bounds, `Hand_R`, and the fighter's existing clips before editing.

All cloud asset integrations remain disabled. Licensed source, converted GLBs, `.blend` checkpoints, and detailed atlas contents stay in `rpg-game-assets` or ignored local paths, never in public `rpg-project` or `rpg-dnd5e-web` history.

### 2. Normalize candidates

Place each candidate's grip at the canonical origin/orientation, apply the correction in the private export, and confirm its exported root is the identity grip frame. This step owns differences between source-pack pivots and scale profiles.

### 3. Calibrate one fighter socket

Parent `Socket_MainHand` under `Hand_R`, tune the helper with one normalized weapon, then replace it with the second normalized weapon at identity. Both must read correctly without changing the helper. Save the reviewed private `.blend` checkpoint and deterministic socket receipt.

### 4. Produce Blender evidence

Render full-body and hand-close views for unarmed, sword, and bow states. Sample idle and multiple walk frames. Verify that the grip remains inside the hand, the weapon does not drift, and the bow's one-hand idle presentation is accepted as a main-hand-only v1 rather than mistaken for a finished two-hand bow pose.

### 5. Verify through Concepts

Sync the provider into the web's ignored `public/models/synty/` tree. Inspect all fixture states in close/orbit/tactical views, idle/walk, and six facings. Capture browser evidence through the real Three.js path. Record any Blender-versus-Three discrepancy as a coordinate-contract defect; do not compensate with an unexplained consumer nudge.

The Blender evidence proves authoring and animation binding. The browser evidence proves actual game-loader behavior. Neither substitutes for the other.

## Validation and tests

### Web Concept slice

- Pure resolver tests: empty, `dnd5e:item:longsword`, `dnd5e:item:shortbow`, unknown ref, rejection of the attack-shaped `dnd5e:weapons:longsword`, and exact full-ref matching.
- Attachment lifecycle tests with a representative mocked Three hierarchy: exact `Hand_R` lookup, `0.01` bone-unit position/scale compensation, one child attachment, replacement cleanup, unarmed cleanup, missing bone, and stale keyed load behavior.
- Shared-cache safety test: the attachment path clones and never reparents/mutates the `useGLTF` cached weapon scene.
- Failure-boundary test: weapon load failure does not remove the fighter.
- Concept structure tests: all three fixture states, idle/walk selector, three views, six facings, and diagnostic inspector.
- Existing `ClassCharacterModel` animation and clone tests remain green.
- Fresh `npm run ci-check` plus a live Concepts walk with actual synced private assets.

### Asset Build slice, filed after the Concept verdict

- Deterministic manifest/receipt generation and byte-stable check mode.
- Full canonical refs; exact promoted paths; exact SHA-256; unique IDs/paths.
- GLB structural checks for true scale, normalized grip root, finite transforms, mesh/material presence, and zero weapon animation/armature payload.
- Fighter bone/socket validation against the exact promoted fighter GLB.
- Blender headless evidence regeneration from the reviewed checkpoint.
- Mesh stats regeneration.
- Production promotion refuses the current 16–64 MB standalone-atlas shape. The selected weapons must satisfy the provider's weapon texture budget or use one documented shared-texture mechanism whose actual browser memory behavior is measured.

### Production Web slice, filed after provider promotion

- Live `equipped.main_hand` projection into the shared resolver.
- Equip longsword -> sword visible; equip shortbow -> bow replaces it; unequip -> unarmed.
- Existing idle/walk behavior remains correct.
- Unknown/unavailable visual mapping remains unarmed without affecting equipment rules.
- Reconnect restores presentation from authoritative equipment state.
- One live game-route walk by Kirk is the closing evidence for the journey.

## Repository responsibilities

| Repository | Responsibility |
| --- | --- |
| `rpg-project` | Journey, approved design, scope/decision record, and eventual plan. No licensed binaries. |
| `rpg-game-assets` | Candidate selection, normalization, fighter socket receipt, private checkpoints, promoted GLBs, asset manifest, hashes, texture budget, and Blender evidence. |
| `rpg-dnd5e-web` | Concepts bench, shared runtime attachment seam, pure presentation resolver, acting-player production wiring, and browser/game-route evidence. No weapon rules or licensed committed binaries. |
| `rpg-api-protos` | No first-wave change. Existing owner `CharacterData.equipped` is sufficient. |
| `rpg-api` | No first-wave change. It already returns authoritative post-equip owner state. |
| `rpg-toolkit` | No presentation work. It continues to own equipment and attack rules. |
| `game-dev` | Local Blender MCP/bootstrap support only; it does not become an asset contract store. |

## Rollout and board shape

Project 19 carries one Assets-owned, Game Screen journey under the current Four-player Level-3 Dungeon initiative.

1. **Decide — `rpg-project#282` (now):** record and review this design.
2. **Concept — `rpg-dnd5e-web` (next):** prove the fighter-only contract against provisional candidates and actual synced assets.
3. **Build — `rpg-game-assets` (after the verdict):** file the exact normalization/promotion work the Concept evidence identifies.
4. **Build — `rpg-dnd5e-web` (after provider promotion):** consume the validated provider contract and wire the acting player's live `equipped.main_hand`.
5. **Verify:** Kirk equips sword/bow/empty on the real game route and reconnects; then journey #281 can close.

Only the journey and immediate Decide slice are created before this written review. The Concept and Build issues are not speculative placeholders: each is filed when its predecessor yields the exact contract it needs.

Development remains outside-in and merging remains inside-out. The Concept may merge as a development-only proof using existing provisional assets. Production consumption merges only after the provider assets and manifest are promoted.

## Explicit non-goals

- immediate equipment appearance updates for other connected clients;
- public equipment disclosure design;
- off-hand weapons, shields, dual wielding, or two-handed occupancy presentation;
- monster or NPC weapons before an authoritative held-equipment contract exists;
- attack, draw, release, hit, or stow animations;
- Auto-Rig Pro retargeting or animation cleanup;
- left-hand bow support or arrow/projectile presentation;
- class compatibility beyond fighter;
- armor, quivers, scabbards, or visibly stowed inventory;
- attack-ref-driven visual switching;
- replacing the server-owned equipment rules; or
- committing licensed Synty binaries to a public repository.

## Later layers, not implied commitments

The accepted fighter socket is the base for later experiments, not proof that those experiments are free. Candidate later slices include sword-combat animation on the same attachment, bow animation with left-hand/arrow requirements, additional rig socket profiles, off-hand/shield presentation, and a server-authored monster held-equipment signal. Each must earn its own design and evidence.

— assets agent, on behalf of KirkDiggler
