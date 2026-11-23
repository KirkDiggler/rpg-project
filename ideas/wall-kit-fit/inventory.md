# Wall/door configuration inventory

Case inventory for [Courtyard S6 #1022](https://github.com/KirkDiggler/rpg-project/issues/1022), initially inspected against Assets `6c74fd4`, updated for the first prepared pair at `a5ebe4e` and the expanded castle candidates at `a9ec761`. This is a human coverage list, not a runtime name-matching classifier.

72 wall/door/window/gate/arch-related environment entries are listed. Generic props also receive the same height-profile shape; these labels do not automatically grant repetition, masonry mapping or door behavior.

**Not a completion table.** The current modes preserve existing declarations: most still need part/surface preparation. A fixed standalone leaf is a part, not a complete doorway. Atlas variants and shared masonry are distinct compatible finish families. The complete pass cannot be claimed from just Fit01/Single01.

**Current selection:** the operator considers the simpler Castle Wall Double01 and the single doorway sufficient for castle-wall doors. Both have the shared fitting/material configuration. The large gateway is **parked, not accepted**: correcting its joins did not establish a correct overall assembly. Its captured work stays recorded, not promoted to acceptance. Inner-wall family inspection is next; house-kit panels/doorways below are candidates, not yet configured equivalents of the castle set.

| Asset / exact key suffix | Current height mode | Declared parts | Current finish support | Owning private recipe |
|---|---|---|---|---|
| 45 Wall 01 (`45_wall_01`) | scale | generic body (no part roles) | not enrolled | `dark-fortress-world-assets-45-wall-01.json` |
| Double Door (`wall_door_double_01`) | repeat-above | frame: Door_Frame, leaf: Door_Left, leaf: Door_Right, above: Door_Wall_Above | not enrolled | `dark-fortress-world-assets-20260917-580a24f5-a125-42cb-978b-9ba038cc3f45.json` |
| Castle alternate right door leaf (`castle_door_leaf_alternate_right_01`) | fixed | leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| Castle Single Door Leaf 01 (`castle_door_leaf_single_01`) | fixed | leaf: Door_Leaf | not enrolled | `kingdom-single-opening-v1.json` |
| Castle Single Door Leaf 01 (seated revision) (`castle_door_leaf_single_02`) | fixed | leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| Castle gate — standard Doorway (`castle_gateway_doorway_01`) | fixed; tested growth band too narrow | frame + two original leaves | shared front/back masonry; original attributes retained | `kingdom-door-batch-v1.json` |
| Castle gate — large tower Doorway (`castle_gateway_doorway_large_01`) | repeat-above + cap; minimum8.1317 | frame / above / cap + two original leaves | shared masonry; **parked / assembly not accepted** | `kingdom-door-batch-v1.json` |
| Castle gate — standard Opening (`castle_gateway_opening_01`) | fixed; tested growth band too narrow | frame | shared front/back masonry | `kingdom-door-batch-v1.json` |
| Castle gate — large tower Opening (`castle_gateway_opening_large_01`) | repeat-above + cap; minimum8.1317 | frame / above / cap | shared masonry; **parked with large-gateway family** | `kingdom-door-batch-v1.json` |
| Castle Wall 01 (`castle_wall_01`) | scale | named body/fit anchors; original default retained | shared masonry | `kingdom-standard-walls-v1.json` |
| Castle Wall 02 (`castle_wall_02`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; other surfaces authored | `kingdom-feature-walls-v1.json` |
| Castle Wall 03 (`castle_wall_03`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; other surfaces authored | `kingdom-feature-walls-v1.json` |
| Castle Wall 04 (`castle_wall_04`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; other surfaces authored | `kingdom-feature-walls-v1.json` |
| Castle Wall 05 (`castle_wall_05`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; other surfaces authored | `kingdom-feature-walls-v1.json` |
| Castle Wall Archway 01 (`castle_wall_archway_01`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; no inferred passage | `kingdom-feature-walls-v1.json` |
| Castle Wall Door 01 (`castle_wall_door_01`) | repeat-above + cap | frame / above / cap; body anchors; no leaf | shared masonry; original default images retained | `kingdom-feature-walls-v1.json` |
| Castle Wall Door Big 01 (`castle_wall_door_big_01`) | repeat-above + cap | frame / above / cap; body anchors; no moving leaf | facade masonry | `kingdom-feature-walls-v1.json` |
| Castle Wall Door Big Door 01 (`castle_wall_door_big_door_01`) | scale | generic body (no part roles) | not enrolled | `fantasy-kingdom-selected-20261005.json` |
| Castle Wall Doorway Double 01 (`castle_wall_doorway_double_01`) | repeat-above + cap | frame / above / cap + two original leaves | shared masonry; exact leaf signatures retained | `kingdom-door-batch-v1.json` |
| Castle Wall Doorway Single 01 (`castle_wall_doorway_single_01`) | repeat-above | frame: Door_Frame, leaf: Door_Leaf, above: Door_Wall_Above | fixed + upper masonry | `kingdom-single-opening-v1.json` |
| Castle Wall Doorway Single 01 (seated revision) (`castle_wall_doorway_single_02`) | repeat-above + cap | frame / above / cap + original leaf | shared masonry; exact leaf signature retained | `kingdom-door-batch-v1.json` |
| Castle Wall Drain 01 (`castle_wall_drain_01`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; no inferred passage | `kingdom-feature-walls-v1.json` |
| Castle Wall Drain Tunnel 01 (`castle_wall_drain_tunnel_01`) | scale | generic body (no part roles) | not enrolled | `fantasy-kingdom-selected-20261005.json` |
| Castle Wall Fit 01 (`castle_wall_fit_01`) | scale | generic body (no part roles) | shared masonry | `kingdom-wall-corners-v1.json` |
| Castle Wall Gate Door L 01 (`castle_wall_gate_door_l_01`) | scale | generic body (no part roles) | not enrolled | `fantasy-kingdom-selected-20261005.json` |
| Castle Wall Gate Door R 01 (`castle_wall_gate_door_r_01`) | scale | generic body (no part roles) | not enrolled | `fantasy-kingdom-selected-20261005.json` |
| Castle Wall Opening Double 01 (`castle_wall_opening_double_01`) | repeat-above + cap | frame / above / cap | shared masonry; matches assembly fixed-part signatures | `kingdom-door-batch-v1.json` |
| Castle Wall Opening Single 01 (`castle_wall_opening_single_01`) | repeat-above | frame: Door_Frame, above: Door_Wall_Above | fixed + upper masonry; full-body static-section adapter verified at 3.75/6 | `kingdom-single-opening-v1.json` |
| Castle Wall Opening Single 01 (seated revision) (`castle_wall_opening_single_02`) | repeat-above + cap | frame / above / cap | shared masonry; matches assembly fixed-part signatures | `kingdom-door-batch-v1.json` |
| Castle Wall Panel Stone 02 (`castle_wall_panel_stone_02`) | scale | generic body (no part roles) | not enrolled | `fantasy-kingdom-selected-20261005.json` |
| Castle Wall Window 03 (`castle_wall_window_03`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; no inferred passage | `kingdom-feature-walls-v1.json` |
| Castle Wall Window Fancy 01 (`castle_wall_window_fancy_01`) | repeat-above + cap | frame / above / cap; body anchors | facade masonry; no inferred passage | `kingdom-feature-walls-v1.json` |
| House Base Wall 02 (`house_base_wall_02`) | scale | generic body (no part roles) | not enrolled | `fantasy-kingdom-selected-20261005.json` |
| House Wall 01 (`house_wall_01`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall 02 (`house_wall_02`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-selected-20261005.json` |
| House Wall 03 (`house_wall_03`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-selected-20261005.json` |
| House Wall 04 (`house_wall_04`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-selected-20261005.json` |
| House Wall 05 (`house_wall_05`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall 06 (`house_wall_06`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall 07 (`house_wall_07`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall 08 (`house_wall_08`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall 09 (`house_wall_09`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall Door 01 (`house_wall_door_01`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall Door 02 (`house_wall_door_02`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-selected-20261005.json` |
| House Wall Door 03 (`house_wall_door_03`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall Door 04 (`house_wall_door_04`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House 01 — arched — plaster Doorway (`house_wall_doorway_01`) | fixed | frame: Door_Frame, leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 02 — square — plaster Doorway (`house_wall_doorway_02`) | fixed | frame: Door_Frame, leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 03 — arched — boards Doorway (`house_wall_doorway_03`) | fixed | frame: Door_Frame, leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 04 — square — boards Doorway (`house_wall_doorway_04`) | fixed | frame: Door_Frame, leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 01 — arched — plaster leaf (`house_wall_leaf_01`) | fixed | leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 02 — square — plaster leaf (`house_wall_leaf_02`) | fixed | leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 03 — arched — boards leaf (`house_wall_leaf_03`) | fixed | leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 04 — square — boards leaf (`house_wall_leaf_04`) | fixed | leaf: Door_Leaf | not enrolled | `kingdom-door-batch-v1.json` |
| House 01 — arched — plaster Opening (`house_wall_opening_01`) | fixed | frame: Door_Frame | not enrolled | `kingdom-door-batch-v1.json` |
| House 02 — square — plaster Opening (`house_wall_opening_02`) | fixed | frame: Door_Frame | not enrolled | `kingdom-door-batch-v1.json` |
| House 03 — arched — boards Opening (`house_wall_opening_03`) | fixed | frame: Door_Frame | not enrolled | `kingdom-door-batch-v1.json` |
| House 04 — square — boards Opening (`house_wall_opening_04`) | fixed | frame: Door_Frame | not enrolled | `kingdom-door-batch-v1.json` |
| House Wall Window 01 (`house_wall_window_01`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Window 01 Glazed (`house_wall_window_01_glazed`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-glazed-windows-v1.json` |
| House Wall Window 02 (`house_wall_window_02`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Window 02 Glazed (`house_wall_window_02_glazed`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-glazed-windows-v1.json` |
| House Wall Window 03 (`house_wall_window_03`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Window 03 Glazed (`house_wall_window_03_glazed`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-glazed-windows-v1.json` |
| House Wall Window 04 (`house_wall_window_04`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Wall Window 05 (`house_wall_window_05`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Window 05 Glazed (`house_wall_window_05_glazed`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-glazed-windows-v1.json` |
| House Wall Window 06 (`house_wall_window_06`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Window 06 Glazed (`house_wall_window_06_glazed`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-glazed-windows-v1.json` |
| House Wall Window 07 (`house_wall_window_07`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |
| House Window 07 Glazed (`house_wall_window_07_glazed`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-glazed-windows-v1.json` |
| House Wall Window 08 (`house_wall_window_08`) | scale | generic body (no part roles) | atlas variants | `fantasy-kingdom-house-kit-v1.json` |

## Expanded castle source pass

The ten pictured/raw feature candidates now include the original Castle Wall Door01 ref. Captured Double01 and seated Single02 pairs support 3.75/6 with fixed leaves/caps; the large gateway supports 12/15 with a real minimum8.1317. The standard gateway gains matching masonry at native height4.39638 but remains fixed: the tested5.5..5.8 raw band has backing narrower than its full frame envelope. No repeat capability is invented. Actual chart errors stay below2e-7 in the inspected views.

Original captured leaves remain exact in final runtime signatures. A Blender import/export initially changed only their normal quantization/vertex splitting; original capture payloads were retained rather than accepting the drift. The standard gateway material-only preparation copies all triangle corner attributes exactly and leaves native thumbnail PNGs byte-unchanged. All314 refs remain; current profiles are278 scale,17 fixed,19 repeat-above.

Captured house frames were inspected from their original sources: timber/plaster/boards with color maps but no normal maps. That is a distinct material capability, not a reason to label them masonry. Their source preparation/material work remains open; this pass does not complete the72-entry inventory.

## Adapter/default-height reconciliation

Web486ed93d verifies Original fixed UV preservation and upper/cap chart continuation on Window03/Wall04, retains selected-finish alignment, and fits the actual opening-only Single01 by full body span/centre without adding a gameplay opening. Eight other measured leafless opening records now share that static fit adapter; their existing fixed heights and absent shared-masonry bindings remain explicit limitations, not completed source preparation. Incompatible new finish choices refuse at the owner before history changes. Legacy static-prop absolute heights measure companions through the shared scale planner, without changing existing percentage semantics.

Raw Castle Wall Door01 is now prepared. Remaining unique-feature/house preparations and per-asset material enrollment are still tracked here. An adapter is not an invented part or a complete 72-entry delivery.

## Feature checkpoint

All nine candidates retain existing refs and have fixed lower geometry, a proved repeat band, and one unscaled top cap. Wall04 and Window03 were measured in actual Studio at heights 3.75 and 6: fixed feature/cap sizes unchanged and facade chart errors below 7e-8. Preview :3191 uses one short wall per piece; this is **not** the pending one-logical-wall section placement workflow. Original/default charts, native/play delivery and the remaining inventory are still open.

## First source checkpoint

- The immutable `kingdom-single-v1.blend` contains the original Castle Wall 01 body and Single Doorway frame/leaf. The existing Fit01 exporter already derives its plain wall from that same original wall object; it is not a different-height source model.
- In the original doorway, decorative/fixed frame vertices extend to raw Z ≈2.865 while the wall reaches Z=5. A repeat band must be explicitly authored above the protected feature, not inferred by the runtime or obtained by scaling the whole doorway.
- Other raw castle wall FBXs are present in the existing Kingdom source pack. Their declarations still require preparation; existence alone is not verification or enrollment.
- Castle Wall 01 was transferred from its converted owner to an authored standard-wall owner without changing its ref or original gray maps. Unchanged old-owner entries retain their historical proofs. Fit01 remains a compatible older appearance; no additional picker alias was invented.
- The first actual Studio checkpoint compares heights 3.75 and 6 with matching selected gray finishes and unchanged leaf height. It does not complete the rest of this inventory.
