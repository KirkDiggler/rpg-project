# Run — 2026-11-26 — Courtyard S7 material-channel design

Slice [rpg-project#1192](https://github.com/KirkDiggler/rpg-project/issues/1192) of [Courtyard#867](https://github.com/KirkDiggler/rpg-project/issues/867).

## Shipped

- Design, walkthrough and checked implementation plan published; product implementation and release are not complete — [Project#1189](https://github.com/KirkDiggler/rpg-project/pull/1189).

## Decided

- KirkDiggler selected existing authored-map controls before replacement maps/per-side looks and approved keeping source-zero emission fixed, 2026-11-26 — [agreement](https://github.com/KirkDiggler/rpg-project/pull/1189#issuecomment-7185560714).
- Shared arithmetic preserves the existing NPC material owner rather than introducing a second clone/replacement writer — [plan](https://github.com/KirkDiggler/rpg-project/blob/design/material-channels/ideas/material-channels/plan.md#plan-check-dispositions-and-explicit-exclusions).

## Found

- Published world metadata has26 normal-bearing GLBs and one emissive-bearing GLB whose emission factors are zero; source-image presence is not editable-channel eligibility — [design case file](https://github.com/KirkDiggler/rpg-project/pull/1189).
- An NPC appearance resolves standing and downed GLBs, so a channel target needs verified representation-specific selectors under one logical identity — [contract](https://github.com/KirkDiggler/rpg-project/blob/design/material-channels/ideas/material-channels/design.md#law).

## Mistaken

- A first draft could have treated one NPC appearance ref as one GLB; the author check caught the omitted representation contract before implementation and the design now requires both bindings — [design case file](https://github.com/KirkDiggler/rpg-project/pull/1189).
