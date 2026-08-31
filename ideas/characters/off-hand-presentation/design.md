# Owner-authoritative off-hand equipment presentation

## Status

Design and written specification approved by Kirk on 2026-08-31; implementation plan committed beside this document.

Journey: [rpg-project#334](https://github.com/KirkDiggler/rpg-project/issues/334), **Show owner-authoritative off-hand equipment**.

Related but separate: [rpg-project#302](https://github.com/KirkDiggler/rpg-project/issues/302), **Complete four-class starting weapon presentation**. Its Glaive, Lance, Scimitar, and Trident slice pauses while this higher-impact presentation seam is built, then resumes unchanged.

North star: **when the acting character has a shield or supported weapon in the server-owned off-hand slot, the game shows that exact equipment in the character's left hand without adding any equipment rules to the asset or web presentation lane.**

The first complete proof is one canonical shield plus one existing promoted weapon in `Hand_L`, shown through the production character renderer and driven only by existing `CharacterData.equipped.off_hand`. Main-hand presentation remains on `Hand_R` and byte/behavior compatible.

## Context and verified current truth

Verified 2026-08-31 against the current provider, API, and web checkouts:

- The game already supports `main_hand` and `off_hand` equipment slots. Equip, unequip, legal-slot validation, shield armor class, two-handed displacement, persistence, and reconnect restoration are existing toolkit/API behavior.
- The owner-private `CharacterData.equipped` map already carries exact refs for both slots. The web equipment UI already sends normal equip/unequip RPCs and replaces its owner cache from complete authoritative responses.
- The production class-GLB route projects only `equipped.main_hand` into `ClassCharacterModel`. `ClassCharacterModel` mounts the exact provider weapon beneath the cloned `Hand_R` bone and plays the existing Idle/Walk clips.
- The current main-hand provider roster has 27 exact `dnd5e:item:*` weapon refs. Provider-owned GLBs carry baked scale, axes, grip, one source-compatible 1024² atlas, normals, and tangents. Web owns no item correction table.
- The current player roster has four Human/Townfolk class aliases plus 28 modular race × class standing models. The rig-family resolver already distinguishes Townfolk and modular main-hand socket profiles without class, race, or item exceptions.
- Every current animated player rig contains `Hand_L`. A correct exported-GLB socket receipt does not yet exist for it.
- The older `MediumHumanoid` path contains `CharacterShield`, legacy OBJ shield configurations, and estimated character-space transforms. That path predates the provider GLB contract, does not attach beneath the production class rig's `Hand_L`, and is not authoritative evidence for this Journey.
- The reusable Weapon Gallery character is a normal Fighter created through existing services. Its inventory already includes the class-provided shield while weapon normalization preserves non-weapon inventory and equipped state. No API fixture expansion is required for the first proof.
- Equipment is owner-private. No public peer equipment contract exists, so this Journey must not infer or expose peer off-hand state.

## Approved decisions

Kirk approved these decisions on 2026-08-31:

1. **Prioritize off-hand presentation now.** It has more immediate gameplay impact than adding the next four main-hand silhouettes.
2. **Asset lane only.** The game already knows what is equipped and already applies every mechanical effect. This work only makes that state visible.
3. **Full foundation, not a shortcut.** Promote a reviewed shield and establish reusable left-hand attachment. Do not wire the guessed legacy OBJ shield.
4. **One hookup per rig family.** Townfolk and modular rigs each receive one reusable `Hand_L` socket. No class, race, shield, or weapon correction table is allowed.
5. **Provider owns item normalization.** Scale, axes, grip origin, atlas, normals, tangents, and shield geometry offset from its grip are baked into provider bytes.
6. **Reuse existing weapons unchanged.** An exact already-promoted off-hand weapon uses its existing GLB. A left-hand socket, not a second mirrored asset, places it.
7. **Authority without rule duplication.** Web trusts the server-owned slot. It does not decide whether a weapon is light, whether dual wielding is legal, whether a shield changes AC, or whether a two-handed weapon clears the other hand.
8. **Owner only.** Only the acting player's private equipment drives this presentation.
9. **Main hand remains stable.** Existing `Hand_R` sockets, 27 weapon mappings, owner routing, fallbacks, and hashes remain unchanged.
10. **Shield is hand-gripped in v1.** Forearm straps, finger posing, and combat-specific contact are later visual polish.

## Considered approaches

### Shared `Hand_L` foundation with a promoted shield — selected

Promote one exact shield GLB, measure one left-hand socket per existing rig family, and project owner-authoritative `off_hand` into a shared bone-attachment renderer. Shield and weapon assets share the socket convention; their provider geometry owns shape-specific placement relative to the grip.

This creates one durable seam for shields and dual wielding while preserving existing game authority.

### Finish the next four main-hand weapons first — deferred

Glaive, Lance, Scimitar, and Trident remain approved work under #302. They add useful silhouettes but do not unlock a new presentation channel. They resume after this Journey.

### Wire the legacy OBJ shield — rejected

The old renderer uses guessed character-space transforms, has no reviewed provider authority, and would not establish reusable weapon off-hand attachment. Shipping it would create a second, weaker truth beside the provider GLB path.

### Pre-baked shielded or dual-wield character variants — rejected

Armed character files multiply race × class × equipment combinations and contradict the unarmed-character runtime attachment contract. Characters remain unarmed on disk.

### Add API or toolkit presentation fields — rejected

Existing `CharacterData.equipped.off_hand` is sufficient. A model URL, socket, or visual variant in gameplay authority would move asset policy into the wrong repository.

## Scope boundary: presentation, not equipment

No code in this Journey may create or modify:

- equipment slots or legal slot combinations;
- equip or unequip semantics;
- shield armor-class calculations;
- main-hand or off-hand damage calculations;
- light, versatile, heavy, or two-handed rules;
- inventory quantities or displacement behavior;
- character persistence or reconnect semantics;
- API/proto/toolkit equipment contracts; or
- reusable fixture storage.

The normal UI and server responses may be used as evidence. Observing AC rise when a shield is equipped proves existing authority remains connected; it is not an implementation requirement for the presentation code.

## Provider contracts

### Canonical shield identity

The first provider output is keyed by exact equipment ref:

```text
dnd5e:item:shield
```

It resolves to one canonical reviewed model. Round, kite, and tower silhouettes are candidate choices, not runtime variants. Kirk selects the production silhouette from visual evidence before promotion.

A weak, semantically misleading, incomplete, or source-incompatible candidate is rejected. A hash-bound authored derivative is allowed only if no source-faithful complete candidate satisfies the shield contract.

### Shield GLB contract

Use a dedicated shield contract rather than inserting a non-weapon into the weapon manifest:

```text
harness/models/synty/shields/manifest.json
harness/models/synty/shields/shield.glb
```

The exact public runtime path is:

```text
/models/synty/shields/shield.glb
```

The shield must satisfy the same strict static glTF subset as promoted weapons:

- glTF 2.0;
- one static identity root in the canonical grip frame;
- true-meter scale;
- finite transforms;
- POSITION, NORMAL, TEXCOORD_0, and TANGENT on every rendered primitive;
- no skin, animation, camera, or light;
- one source-compatible 1024² atlas;
- no more than 4.5 MiB decoded base RGBA8 texture memory; and
- deterministic byte-stable output from sealed inputs.

Grip-space convention:

- root origin is the center of the intended left-hand grip;
- local `+Y` runs from the grip toward the shield's upper edge;
- local `+Z` is the outward-facing shield front;
- any center offset between the grip and shield face is baked into mesh geometry;
- the identity root is valid at the shared socket; and
- no web transform is specific to `shield`.

Detailed licensed source paths, provenance, derivatives, checkpoints, and atlas content remain private in `rpg-game-assets`.

### Cumulative-release invariants

The shield promotion is cumulative and transactional:

- preserve every canonical character GLB hash at the selected clean provider base;
- preserve all 27 canonical weapon GLB hashes;
- preserve existing weapon and character manifest semantics;
- validate and seal staged bytes before apply;
- support rollback and refuse dirty tracked HEAD;
- apply transactionally; and
- rebuild every cumulative release target from a clean worktree and compare byte-for-byte.

The shield contract may reuse generic static-GLB, atlas, tangent, release-evidence, and transactional-apply machinery. It must not weaken weapon validation to make the new kind fit.

## Shared left-hand socket contracts

The provider and consumer record two socket profiles:

```text
townfolk-off-hand-v1
modular-fantasy-hero-off-hand-v1
```

Each profile owns exactly:

- semantic slot `off_hand`;
- bone `Hand_L`;
- positive finite `boneUnitMeters`;
- joint-local position in meters;
- joint-local rotation quaternion `[x, y, z, w]`; and
- uniform scale, normally `1`.

The values are authored and verified in exported glTF/Three.js joint-local coordinates. Blender helper values are not copied unchecked. Receipts distinguish `authoredBlenderBoneLocal` from `runtimeThree`, following the existing main-hand coordinate contract.

The sockets are calibrated with normalized assets at identity:

1. canonical shield;
2. one symmetric/simple existing weapon such as Dagger or Shortsword; and
3. one visibly asymmetric existing weapon to prove the socket does not rely on accidental symmetry.

All assets must fit without changing the socket. If the shield needs a nudge, its provider normalization is wrong. If one weapon needs a nudge, its existing grip normalization or the proposed left socket is wrong. No consumer exception is accepted.

Automated checks inspect all four current Townfolk class aliases and all 28 current modular race × class standing GLBs. Rig-family profiles may differ from each other; entries within a family may not.

## Web presentation contract

### Pure resolution

The web consumes the existing owner-private map and produces presentation only:

```ts
type OffHandPresentation = {
  ref: string;
  assetUrl: string;
  assetKind: 'shield' | 'weapon';
  socket: HandSocket;
};
```

Resolution rules:

1. Absent `off_hand` returns `undefined`.
2. Exact `dnd5e:item:shield` resolves to the promoted shield URL.
3. An exact `dnd5e:item:*` ref already in the promoted weapon presentation catalog reuses that exact weapon URL unchanged.
4. Unknown item refs return `undefined`.
5. Attack-shaped refs such as `dnd5e:weapons:dagger` return `undefined`.
6. No visual similarity fallback is allowed.

The resolver does not filter by weapon properties. If authoritative state contains a promoted exact weapon in `off_hand`, presentation shows it. The server is the only owner of whether that state is legal.

### Shared attachment core

Extract the bone-parenting lifecycle behind the current `MainHandAttachmentSlot` into a focused shared implementation, then keep semantic wrappers for main and off hand.

The core owns only:

- cloning the cached static asset scene without mutating it;
- finding the configured bone in the exact rendered character clone;
- validating socket values;
- applying unit compensation, quaternion, and scale;
- parenting the asset beneath the bone;
- replacement/unmount cleanup;
- independent load/error status; and
- disabling attachment raycasts so entity interaction remains on the stable proxy.

`MainHandAttachmentSlot` retains its current interface and behavior. `OffHandAttachmentSlot` receives `OffHandPresentation | undefined`. A failure in either attachment slot cannot remove the character or the other slot.

### Owner-only data flow

```text
owner CharacterData.equipped
  -> existing SessionEncounterView owner routing
  -> exact off_hand presentation resolver
  -> SessionCanvas
  -> owner HexEntity only
  -> ClassCharacterModel
  -> cloned character Hand_L
```

Peer public roster data provides no equipment, so peer entities receive no off-hand presentation. No client guess fills that absence.

The legacy `CharacterShield`/estimated OBJ branch does not become a fallback for this path. Production player routing must not substitute it after a provider load, ref, socket, or bone failure. If it has no remaining legitimate consumer, removal may occur in the web slice; otherwise it remains isolated and explicitly outside the authoritative class-GLB route.

## Runtime behavior

For one mounted character:

1. Load and skeleton-clone the unarmed character as today.
2. Resolve owner `main_hand` through the unchanged main-hand mapping.
3. Resolve owner `off_hand` through the new exact resolver.
4. Mount main-hand presentation beneath `Hand_R` through the existing semantic wrapper.
5. Mount off-hand presentation beneath `Hand_L` through the new semantic wrapper.
6. Let the character's existing Idle or Walk clip drive both bones and their children.
7. On authoritative state replacement, independently replace or remove the affected slot.
8. On reconnect, render the first complete authoritative owner snapshot without local reconstruction.

There is no client-side two-handed suppression. When the game clears `off_hand`, the complete response removes its presentation. When the game permits both slots, both render.

## Failure behavior

| Condition | Honest result |
| --- | --- |
| `off_hand` absent | Render no left-hand item. |
| Unknown or attack-shaped ref | Render no left-hand item; development status is `unmapped-ref`. |
| Shield/weapon GLB load failure | Keep character and main hand; omit off hand and report `asset-load-failed`. |
| `Hand_L` absent | Keep character and main hand; omit off hand and report `missing-bone`. |
| Invalid socket | Refuse scene mutation; report `invalid-socket`. |
| Ref changes while loading | Keyed stale branch cannot attach after replacement. |
| Candidate needs an item-specific web offset | Reject/re-normalize; do not add an exception. |
| Peer has no public equipment | Render no peer off hand. |

No failure can unmount the entity or Canvas.

## Learn and visual decision workflow

The first implementation issue is a private provider Learn slice:

1. Refresh deterministic source inventory from the current licensed packs.
2. Search broadly for complete shield candidates and classify round/kite/tower silhouettes honestly.
3. Build public-safe source and grip contact sheets without exposing licensed paths or provenance.
4. Show the actual visual choices to Kirk before hardening promotion configuration.
5. Reject misleading candidates; author a hash-bound derivative only if necessary.
6. Calibrate provisional Townfolk and modular `Hand_L` sockets with the selected shield and existing normalized weapons.
7. Verify Idle and Walk contact without altering animation.
8. Record Kirk's exact selection and accepted canonical size.

Kirk owns the visual choice. The agent owns source authority, transform math, derivative determinism, validation, tests, and sequencing.

## Verification design

### Provider verification

- Candidate closure and private provenance authority.
- Strict shield GLB structure and semantic checks.
- Exact atlas dimensions and decoded base-memory gate.
- Deterministic/idempotent normals and tangent handling.
- Identity root and grip-frame checks.
- Exact Townfolk and modular `Hand_L` socket receipts.
- Automated compatibility against 4 Townfolk + 28 modular standing models.
- Idle and Walk Blender evidence for shield, sword-and-shield, and asymmetric off-hand weapon.
- Preservation of every existing character and weapon hash.
- Clean-head cumulative rebuild and byte comparison.
- Sealed evidence rendered from immutable validated stage bytes.

### Web tests

TDD covers:

- absent, shield, known weapon, unknown item, and attack-shaped off-hand refs;
- reuse of exact existing weapon URLs with no second catalog;
- rig-family socket selection;
- independent main/off-hand mounting and cleanup;
- exact `Hand_L` lookup and bone-unit compensation;
- cached-scene immutability;
- stale load/replacement behavior;
- attachment-local failure boundaries;
- owner-only `SessionEncounterView -> SessionCanvas -> HexEntity -> ClassCharacterModel` routing;
- no peer projection;
- unchanged main-hand mapping/socket behavior;
- no item/class/race transform table; and
- legacy guessed shield path not used as authoritative fallback.

### Browser and normal-game evidence

Use ignored assets synced from the exact merged provider commit.

Concept evidence covers:

- shield only;
- main-hand weapon plus shield;
- two supported one-hand weapons;
- unarmed/empty off hand;
- all four Human/Townfolk classes in Idle;
- Dwarf, Elf, Gnome, Half-Elf, Half-Orc, Halfling, and Tiefling Fighter in Idle for each equipped state;
- automated contract coverage of all 28 modular race × class combinations;
- focused Human/Townfolk Fighter and one modular Fighter Walk for each equipped state; and
- exact HTTP 200/provider hashes with zero unexpected browser errors.

Normal game evidence uses existing UI and authority only:

1. capture exact initial owner state;
2. equip Shield into `off_hand` and observe immediate presentation;
3. reconnect and observe restored presentation;
4. unequip Shield and observe removal;
5. equip a server-valid two-weapon pairing such as Shortsword main hand plus Dagger off hand;
6. reconnect and observe both exact assets;
7. remove both through normal UI;
8. reconnect and prove exact initial-state restoration.

Any AC values are recorded as evidence only. The presentation code never calculates or changes them.

## Repository responsibilities and sequence

| Repository | Responsibility |
| --- | --- |
| `rpg-project` | Journey #334, approved design, implementation plan, cross-repository sequencing, and final outcome record. |
| `rpg-game-assets` | Shield Learn, visual decision evidence, optional derivative authority, strict shield promotion, `Hand_L` socket receipts, cumulative hash preservation, and private licensed artifacts. |
| `rpg-dnd5e-web` | Exact off-hand resolver, shared attachment core, owner-only routing, Concepts proof, normal-game evidence, and no tracked licensed GLBs. |
| `rpg-api` | No change. Existing owner data and normal UI calls are evidence inputs only. |
| `rpg-api-protos` | No change. Existing `CharacterData.equipped.off_hand` is sufficient. |
| `rpg-toolkit` | No change. Existing equipment legality and mechanics remain authoritative. |
| `game-dev` | Local source ingestion/Blender/browser tooling only; no durable product policy or licensed public artifacts. |

Merge sequence:

1. provider Learn and Kirk visual decision;
2. provider Build and exact merged shield/socket contract;
3. web Concept + production consumer against that exact provider merge;
4. normal-game authority/restoration evidence;
5. final review, merge, Journey closure;
6. resume #302 with Glaive, Lance, Scimitar, and Trident.

There is no API gallery change unless verification disproves the current fact that the existing normal Fighter/Weapon Gallery already carries a Shield. A missing local test identity is not permission to add gameplay or storage scope; use normal character creation/equipment paths instead.

## Delivery discipline

- Isolated worktrees and PR-only integration.
- TDD for provider and web behavior.
- Clean tracked provider HEAD and sealed input digests.
- Validate before apply, rollback on failure, transactional apply.
- Exact provider commit consumed by web only after provider merge.
- No licensed GLBs committed to public web or project repositories.
- Fresh independent review for every substantive PR.
- Final verdict published directly on each PR with reviewed head, readiness, severity counts, scope, verification, and finding disposition.
- Re-review and republish if a reviewed head changes materially.
- Copilot remains disabled until Kirk explicitly re-enables it.

## Explicit non-goals

- equipment gameplay or API changes;
- peer equipment visibility or propagation;
- public equipment contract design;
- forearm straps or finger posing;
- two-hand support contact;
- shield bash, block, attack, draw, stow, or combat animations;
- runtime recoloring or shield variants;
- armor, quivers, scabbards, or visibly stowed inventory;
- monster/NPC equipment;
- item-specific, class-specific, or race-specific web transforms;
- mirrored duplicate weapon assets;
- pre-baked armed character variants; or
- Glaive, Lance, Scimitar, and Trident promotion inside this Journey.

## Done when

- Kirk has selected one honest canonical shield.
- `dnd5e:item:shield` has one strict, reviewed, deterministic provider GLB.
- One shared `Hand_L` socket exists for Townfolk and one for modular rigs.
- Every current character and weapon GLB remains byte-identical.
- Exact shield and already-promoted off-hand weapon refs render from owner authority.
- Main-hand presentation remains unchanged.
- Unknown, attack-shaped, peer, and failure cases remain honest and isolated.
- Four-class/current-rig Idle and Walk evidence is complete.
- Shield and two-weapon normal-UI reconnect sequences are proven.
- The exact initial owner state is restored.
- Full provider/web verification and published independent reviews are clean.

— assets agent, on behalf of KirkDiggler
