# Level-1 Specialist Weapon Wave — Glaive, Scimitar, and Trident

## Status

Approved in conversation by Kirk on 2026-08-31. Written under [rpg-project#344](https://github.com/KirkDiggler/rpg-project/issues/344) for parent Journey [rpg-project#302](https://github.com/KirkDiggler/rpg-project/issues/302).

This wave adds honest, owner-authoritative presentation for Glaive, Scimitar, and Trident. Lance is not part of this wave: it is deferred until mounted combat exists because its intended presentation and use depend on a mount-aware contract.

The reusable delivery process is defined by [weapon-wave-playbook.md](./weapon-wave-playbook.md). This document records only the wave-specific decisions and acceptance proof.

## Outcome

A player can distinguish Glaive, Scimitar, and Trident by looking at the acting character in the normal game. Glaive and Trident render only in the right hand. Scimitar renders in either hand and can be shown simultaneously in both hands when existing authoritative equipment state places Scimitars in both slots.

No asset or web code determines whether an equip is legal. The existing server remains authoritative for equipment, AC, damage, inventory, and slot behavior.

## Scope

Exact equipment refs:

| Ref | `Hand_R` | `Hand_L` | Notes |
| --- | --- | --- | --- |
| `dnd5e:item:glaive` | Required | Unsupported | One rigid main-hand presentation; second-hand contact remains deferred. |
| `dnd5e:item:scimitar` | Required | Required | Light weapon; left presentation is reviewed and provider-authoritative. |
| `dnd5e:item:trident` | Required | Unsupported | One rigid main-hand presentation; versatile support contact remains deferred. |
| `dnd5e:item:lance` | Deferred | Deferred | No source selection, derivative, promotion, fixture, mapping, or evidence until mounted combat exists. |

This is the level-1 presentation pass. Shields remain supported as shields. Weapon presentation beneath `Hand_L` is limited to exact reviewed Light-weapon refs. This is a visual support allowlist, not a second implementation of rulebook legality. Level progression, Fighting Styles, subclasses, feats, and mounted rules remain outside the asset/web lane.

Journey #302's closure criterion is amended accordingly: Lance becomes a documented mount-blocked exception transferred to a future mounted-combat journey and does not block completion of the level-1 starting-weapon presentation pass. A linked implementation issue is created only when mounted combat provides its actual contract; no speculative Lance placeholder is required now.

## Existing authority

The wave builds on these merged contracts:

- `rpg-game-assets@71dff14f57afe41ff320dee15081123ec1daddc2` provides the reviewed owner off-hand catalog and the exact Townfolk/modular `Hand_L` sockets.
- `rpg-game-assets@a67f916880718a84b502b66b2b63683b03990f59` is the 27-ref cumulative main-hand weapon release incorporated by current asset `main`.
- `rpg-api@9254f85d66793dff276386cb240840b634c70f12` is the reusable 27-item Weapon Gallery fixture release.
- `rpg-dnd5e-web@8c43298791c6f33d41aca520745b9c637ad803a9` provides owner-only `Hand_R`/`Hand_L` projection, independent attachment lifecycle, same-ref cleanup, and the current exact catalogs.

Current character GLBs remain unarmed. Both slots attach standalone equipment dynamically beneath cloned rig bones. Existing rig-family sockets are reused exactly; this wave does not calibrate new web sockets.

## Decisions

1. **One Learn for three semantic targets.** Licensed source inventory and candidate review cover Glaive, Scimitar, and Trident together. Lance candidates are not reviewed opportunistically.
2. **Provider owns both Scimitar presentations.** The canonical normalized Scimitar is first tested unchanged on both sockets. If the exact same bytes pass, the Learn records identity reuse. If they do not, the provider deterministically bakes a left-hand variant. The web never mirrors or rotates it.
3. **Existing sockets remain immutable.** All three main-hand assets consume the current rig-family `Hand_R` profiles. Left Scimitar consumes the current rig-family `Hand_L` profiles. An item needing a web correction is rejected or re-normalized.
4. **Single-hand rigid pose is honest for this layer.** Glaive and Trident follow `Hand_R` through existing Idle and Walk clips. Glaive second-hand contact and Trident versatile support contact wait for a future animation/pose layer.
5. **Exact refs only.** Attack-shaped refs, unknown refs, and unsupported slot/ref combinations render nothing.
6. **Owner-private state only.** The acting player's authoritative `CharacterData.equipped` drives presentation. Peer equipment remains deferred.
7. **Cumulative immutable releases.** Existing main-hand and off-hand canonical GLBs must remain byte-identical. The new provider release adds outputs; it does not silently rebuild accepted history.
8. **No licensed web artifacts.** Source packs, candidate derivatives, Blender checkpoints, and promoted GLBs remain private. The public web repository tracks only code, tests, documentation, and safe evidence.

## Learn contract

The Learn slice begins from current `rpg-game-assets` `origin/main` in an isolated worktree. It inventories licensed matches broadly enough to reject misleading naming and classifies every query result exactly once.

For each of the three targets, Learn records:

- all honest source candidates and explicit rejection reasons;
- source archive/object identity and private provenance;
- dimensions, hierarchy, material/atlas facts, and semantic silhouette;
- deterministic normalization or derivative authority;
- canonical `Hand_R` placement under the existing socket profiles; and
- Idle and Walk evidence on both Townfolk and modular rig families.

Scimitar additionally records both `Hand_L` placements and proves either:

- exact byte identity with the canonical main-hand GLB; or
- the deterministic transform/derivative recipe that produces a left-hand GLB from sealed private input.

Kirk owns the candidate and placement verdict in Blender. The assets agent owns candidate closure, coordinate math, deterministic authority, validation, evidence generation, and tests. A weak or misleading candidate is rejected rather than substituted.

Learn merges no production mapping unless the repository's established Learn contract requires a private accepted roster update. It must preserve every current runtime output hash and leave a complete provider-ready receipt.

## Provider Build contract

One cumulative Build consumes the exact merged Learn commit.

Expected catalog growth:

- main-hand weapon manifest: 27 refs to 30 refs;
- off-hand manifest: five reviewed entries to six, adding Scimitar;
- generated output count depends on Learn's Scimitar identity result: canonical reuse when honest, otherwise one provider-baked left variant.

Every generated GLB must retain the strict provider contract:

- glTF 2.0;
- static identity root in true-meter grip space;
- required `POSITION`, `NORMAL`, `TEXCOORD_0`, and `TANGENT` semantics;
- one 1024×1024 source-compatible atlas;
- base RGBA8 decoded texture memory no greater than 4.5 MiB;
- no skin, animation, camera, or light payload; and
- finite deterministic geometry, normals, tangents, transforms, bounds, and hashes.

Release mechanics remain sealed and transactional: clean tracked HEAD, sealed inputs, stage, validate before apply, rollback on failure, atomic apply, complete release-tree evidence, and clean-head byte-for-byte rebuild. Existing canonical GLB hashes are a hard gate.

## Weapon Gallery contract

The repository-backed Weapon Gallery expands from 27 to 30 main-hand refs without raw storage mutation.

The fixture must:

- preserve its stable character identity;
- normalize idempotently through normal repository/service paths;
- preserve unrelated inventory and equipment;
- include Glaive, Scimitar, and Trident exactly once in the catalog contract;
- materialize exactly one canonical Scimitar inventory stack with `Quantity: 2`, not duplicate Scimitar rows; and
- prove through seed-idempotence and normal UI calls that both authoritative slots can contain `dnd5e:item:scimitar`.

This fixture change does not grant proficiency, alter slot legality, calculate damage, or synthesize equipment state.

## Web consumer contract

The web syncs ignored runtime assets from the exact merged provider commit and verifies manifest/output hashes before coding.

Production mappings add:

- Glaive, Scimitar, and Trident to the exact main-hand resolver; and
- Scimitar only to the exact off-hand weapon resolver.

No existing ref, URL, socket, or GLB hash may drift. The generic attachment core, semantic slot-prefixed React keys, and independent error boundaries remain unchanged unless a failing regression demonstrates a necessary fix.

A production-backed Concept uses actual provider bytes and the shared renderer. Required visual states are:

1. Glaive in `Hand_R`;
2. Trident in `Hand_R`;
3. Scimitar in `Hand_R`;
4. Scimitar in both `Hand_R` and `Hand_L`.

The Concept exposes class/race, Idle/Walk, view, and six-facing controls but no transform controls. It must cover:

- all four current classes on the Townfolk family for all four states;
- Fighter on every currently resolved modular race for all four states;
- all four states in Walk on at least one Townfolk and one modular model; and
- same-ref dual-Scimitar mount, replacement, simultaneous cleanup, and status-rerender stability.

The evidence receipt records the exact matrix resolved at implementation time rather than assuming a stale race count.

## Normal-game authority proof

Use only the visible game UI and normal service/repository-backed fixture state.

1. Capture exact initial owner equipment, AC, HP, speed, and damage text.
2. Equip a representative new long main-hand silhouette (Glaive or Trident), verify immediate presentation, close the browser context, and verify reconnect restoration.
3. Unequip it and verify immediate and reconnect removal.
4. Equip two independently authoritative Scimitars into main and off hand through normal UI calls.
5. Verify both visible, exact server equipment state, and server-produced damage text.
6. Close the browser context and verify both slots restore.
7. Unequip both through normal UI calls and verify removal.
8. Restore the exact initial owner state and confirm it in a fresh context.

AC and damage are observations only; the asset/web lane never predicts or calculates them. Context-close stream aborts are acceptable only when separately identified as expected. Any other console, page, request, RPC, authority, or asset-hash error blocks publication.

## Validation gates

### Learn and provider

- complete candidate classification and private provenance;
- Kirk's recorded Blender verdict;
- both rig families in Idle and Walk;
- deterministic Scimitar left-hand authority;
- all previous canonical hashes exact;
- strict GLB, atlas, manifest, inventory, release-tree, rollback, and clean rebuild checks;
- no licensed source path or bytes in public repositories.

### Gallery

- 27-to-30 exact catalog transition;
- stable identity and idempotent rerun;
- non-destructive inventory/equipment behavior;
- normal service/repository paths only;
- one Scimitar row with `Quantity: 2`, preserved across idempotent reruns, and both authoritative slots proven as `dnd5e:item:scimitar`.

### Web

- exact resolver positives and negative refs;
- unchanged existing main/off mappings and socket literals;
- owner-only routing and no peer projection;
- independent slot failures and same-ref cleanup;
- required Concept matrix with exact HTTP 200 hashes;
- complete authority/reconnect/restoration sequence;
- exact final state parity;
- full CI and test suite;
- tracked licensed GLB count remains zero; and
- published current-head independent review verdict with severity counts and readiness.

## Failure behavior

- An unacceptable source candidate is rejected; the target remains unpromoted.
- A candidate needing item-specific web transforms returns to provider normalization.
- An unsupported off-hand ref renders empty without affecting the character or main hand.
- A failed slot load leaves the character and other slot intact.
- A stale or unsealed input blocks promotion.
- A failed validation leaves the canonical provider tree byte-identical to its pre-build state.
- If prose and Git differ, exact commits, manifests, hashes, and command output win.

## Explicit non-goals

- Lance source selection or presentation before mounted combat;
- mount models, mounted sockets, mounted animations, or mounted combat rules;
- two-hand support contact for Glaive or Trident;
- attack, draw, hit, projectile, stow, finger, or combat animation authoring;
- non-Light off-hand weapon presentation in this level-1 pass;
- peer-private equipment projection;
- monster or NPC equipment;
- inventory, slot, proficiency, Fighting Style, subclass, feat, AC, damage, or persistence changes; or
- per-item/class/race transform tables in web.

## Completion

The wave is complete only after Learn, provider, Gallery, and web PRs merge in dependency order; normal-game state is restored; final verdicts are published on current heads; linked issues are Done; and temporary worktrees, branches, previews, detached checkouts, and private review windows are cleaned up.

— assets agent, on behalf of KirkDiggler
