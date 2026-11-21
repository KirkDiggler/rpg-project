# 2026-11-08 — What can still happen while an effect ends?

Written during [Composable conditions S3 #706](https://github.com/KirkDiggler/rpg-project/issues/706), following [concentration as an effect](2026-11-08-when-upkeep-is-an-effect-too.md).

## Question

If several pieces share a lifetime, does marking that lifetime as ending mean everything is already gone? The distinction matters for concentration: a save needs the current contributions, but end reporting must not start another check or advance a sibling's upkeep.

## Evidence

The independent [provider review](https://github.com/KirkDiggler/rpg-toolkit/pull/2327#pullrequestreview-5819824571) found two concrete failures. Query-only contributions disappeared inside End even though subscribed damage still answered. A nested notification also let a sibling machine mutate or detach before the outer report finished. The original end test covered subscribed damage only and did not reveal either difference.

Permanent reproductions failed first. [The fix](https://github.com/KirkDiggler/rpg-toolkit/commit/5d95849cd0484650dc62befdab590d5ab053e58a) retains the pre-terminal observation window and gates machine dispatch before sibling reactions run. [Focused closure](https://github.com/KirkDiggler/rpg-toolkit/pull/2327#pullrequestreview-5819875758) killed five targeted mutants and closed the findings without another whole-diff round. Snapshot still reports actual machine state; the output window is not another saved lifecycle.

The next real consumer exposed the other side of the distinction: concentration's output requests work, unlike a damage multiplier that answers a fold. The provider's [CanReact addition](https://github.com/KirkDiggler/rpg-toolkit/commit/eb5ffb63) reads the existing attachment/state/assembly permission. The [rulebook checkpoint](https://github.com/KirkDiggler/rpg-toolkit/commit/0b2f830f) uses it to refuse a new follow-up during end; removing that guard fails the test. Concentration, duration, end policies, resistance and unresolved roll modifiers now have vocabulary-level proof and full root gates. Linked persistence/transport and native caller migration are not yet implemented.

## Interpretation

Ending has an observation window. A contribution can still answer the event being resolved while permission to react to another event is already closed. Treating those as one boolean made one execution mode differ from another and let callback ordering decide which benefits survived long enough to be read.

The correction belongs in dispatch and observation, not in a special Bless or concentration workaround. The subsequent rulebook consumer uses that same distinction instead of keeping another ending flag.

## Next

The [checked plan](https://github.com/KirkDiggler/rpg-toolkit/pull/2323) moves next to the current linked codec and actual membership transport. Those paths must carry the exact source and lifetime across real requests; vocabulary tests alone do not prove native adoption. Journal #863 remains open.

— cross-team agent, on behalf of KirkDiggler
