# All-race hair customization implementation checklist

This is intentionally a short two-PR execution plan. The approved design is
`all-race-hair-design.md`; Dwarf production is the executable reference.

## Provider — rpg-game-assets#117

1. Add RED profile/default-selection/aggregate-manifest tests.
2. Curate neutral Human bodies and front/profile hair sheets; stop for Kirk's
   default/identity verdict.
3. Generalize Dwarf scripts/config into a profile matrix while keeping Concept
   and Dwarf command/import compatibility.
4. Build two clean seven-profile stages and prove byte equality.
5. Run all 1,792 aggregate compatibility checks and exact Dwarf/unrelated byte
   preservation.
6. Render race-specific thumbnails and one aggregate overview; apply one atomic
   publication transaction.
7. Run focused gates, then one full suite, GLM whole-PR review/responses, and
   merge the provider PR.

## Web — rpg-dnd5e-web#897

1. Start only from the exact provider merge; add RED aggregate generator tests.
2. Generate a profile-driven catalog and sync ignored bytes without tracking
   licensed assets.
3. Replace Dwarf-specific resolver/picker names with generic profile inputs;
   preserve all Dwarf tests and evidence.
4. Parameterize creation, default-style/default-none, fallback, and owner/peer
   tests across eight profiles.
5. Run focused gates and launch the real picker for Kirk at the first useful
   aggregate checkpoint.
6. Capture two automated four-race normal sessions plus honest human verdict.
7. Run one full CI, GLM whole-PR review/responses, and merge the web PR.

## Closeout

Record exact provider/web merges, Human/default verdict, final game verdict, and
review outcomes in the design. Merge the design record and close #352/#353.
Gear appearance begins as the next separate Journey.
