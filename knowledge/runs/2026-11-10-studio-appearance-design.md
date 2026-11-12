# Run — 2026-11-10 — Manifest-driven Studio appearance design

Work [Web #1402](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402), linked from story [Project #867](https://github.com/KirkDiggler/rpg-project/issues/867). This is an inspected design proposal, not a built panel or playable masonry release.

## Shipped

- No product code shipped; the design, walkthrough and execution brief describe the agreed manifest/adapter boundary and expose the remaining delivery decision — [design](../../ideas/studio-appearance/design.md).

## Decided

- KirkDiggler approved common appearance controls → mixed wall/door sections → richer material editing; tree YAML is not a prerequisite for reusable capabilities or later courtyard assets — [priority record](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402#issuecomment-6755702081).
- KirkDiggler clarified that Assets import metadata supplies valid entries for the shared lists; reuse existing catalog/binding authorities and preserve owner history/persistence rather than add hard-coded lists or a second registry — [ownership record](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402#issuecomment-6755812411).
- The proposed implementation boundary is one shared editor view over typed complete-look, authored-UV and chart-mapped adapters. Copying each picker repeats the inconsistency; one universal material property bag loses valid mapping and owner semantics — [walkthrough](../../ideas/studio-appearance/README.md#trade-offs).

## Found

- Web `9809f6ce` has provider-owned NPC `look { familyRef, style, skinPalette }` tuples resolving to exact complete standing/downed appearances; they are not per-instance material-channel overrides — [NPC choices](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/9809f6ce8b94d38e8e131af5c6cfd1cc825d64be/src/npc-appearances/npcAppearanceChoices.ts).
- The existing atlas control validates binding/model identity but lists all atlas materials. Current material families and exact bindings are reusable inputs; an extensible valid-choice contract must not equate all future atlas materials with compatibility — [atlas controls](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/9809f6ce8b94d38e8e131af5c6cfd1cc825d64be/src/concepts/encounter-studio/StudioTexturePackControls.tsx).
- Five masonry presets carry paired color/normal maps; twelve Kingdom atlas presets carry explicit null normal/emissive channels. The masonry renderer sets its normal map/scale and clears emissive; this is not existing arbitrary channel-editing support — [material types](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/9809f6ce8b94d38e8e131af5c6cfd1cc825d64be/src/rendering/worldMaterialTypes.ts).
- `StudioWallFinishes` saves `scene.wallFinishes` and labels it Studio-only. `projectWallFinishes` also consumes region charts/pattern anchors; `StructuralWallSurfacePieces` accepts finish spans but defaults them to empty. The player path does not supply those spans; its captured material adapter handles atlas override identities instead — [wall projection](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/9809f6ce8b94d38e8e131af5c6cfd1cc825d64be/src/concepts/world-building/wallFinishProjection.ts).
- Existing Studio R19 explicitly defers wall/region finishes until the look catalog. Applying the catalog to masonry playback still needs a concrete captured mapping contract; it is not achieved by replacing a dropdown — [standing design](../../ideas/encounter-studio/design.md#law).
- NPC appearance pickers commit immediately while atlas/masonry fields stage behind Apply; the shared appearance transaction must reconcile that difference deliberately rather than preserve contradictory Cancel behavior — [Arrange owner](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/9809f6ce8b94d38e8e131af5c6cfd1cc825d64be/src/concepts/encounter-studio/StudioArrangePanel.tsx).
- Focused existing tests passed: `npcAppearanceChoices`, `StudioTexturePackControls`, `StudioArrangePanel`, `wallFinishProjection`, and session `materialOverrides`: 83 tests across five files, 17.27 seconds. Receipt `/tmp/rpg-1402-design-baseline-tests.log`; no full CI, newly implemented behavior or fresh browser walk is claimed — [execution brief](../../ideas/studio-appearance/brief.md).

## Mistaken

- The earlier answer correctly identified real normal-bearing wall packs but did not distinguish Studio rendering from captured play support. The correction is explicit: normals exist and work in the masonry Studio path; their choices are not yet projected into played walls. The design now checks provider, editor/native, capture and renderer as separate boundaries before claiming support — [R9 and its consequence](../../ideas/studio-appearance/design.md#open).
