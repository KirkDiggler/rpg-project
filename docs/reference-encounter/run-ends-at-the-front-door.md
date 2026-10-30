# The run ends when the last one walks out the front door

**Scene:** walk site `gate-walk` on the stack: the party start is the front door; in the keep, the front door is the party start too.  
**Status:** in play. Walked on `gate-walk` 2026-10-29. No screen shows success or failure yet; that is S9 ([rpg-project#733](https://github.com/KirkDiggler/rpg-project/issues/733)).  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S2 ([#716](https://github.com/KirkDiggler/rpg-project/issues/716))

## Idea

The keep's brief asks to "distinguish primary-objective victory from final
extraction. Exploration and independent optional encounters can remain after
the boss dies"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
An ending could wait only on a round, a down, a fact or a stance; nothing
fired on leaving, the run had no success or failure, and the keep's
`king-slain` closed the run the moment the king fell.

## Decision

- D7, the keep's ending: "walk out the front door to end. king taken care of? no? failure, yes? success." ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D9, an ending that fires on leaving: "last one out ends it, go with your lean": `endings: [{ id, when: { left: <exit id> }, if: <one predicate>, outcome: success | failure }]`; the first ending in declaration order whose conditions hold fires ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D10, `king-deposed` and `freed`: both move to the front door as leave endings with their fact as the `if`; revisit if the walk says a deposition should stop the run ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- Knowledge line: none yet for D7, D9, D10.

## Implementation

- rpg-toolkit · [#2155](https://github.com/KirkDiggler/rpg-toolkit/pull/2155) · `when: { left: <exit> }` holds when the last standing player leaves by that exit and is refused off an ending; `if:` reads one `down`, `fact` or `stance` at that moment; `outcome` rides `Outcome.Result` and the `ended` beat. The keep declares `front-door` at the party start and ends four ways there: `king-slain`, `king-deposed`, `freed` (success) and `king-lives` (failure). · [run record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/knowledge/runs/2026-10-29-franks-list-s2-fell.md)
- Not built: session, rpg-api, the protos and the web carry the ending key and not the outcome (S9). A last player who leaves by the lobby with monsters still inside leaves the run open and ownerless.

## Verify

- On `gate-walk`, drop the captain and leave by the front door → the run ends `captain-slain`, success.
- On `gate-walk`, leave without touching him → `captain-lives`, failure.
- In the keep, bring the king down and walk out the front door → `king-slain`, success; with the king alive, `king-lives`, failure.
- With a party of two, drop the king and leave one at a time → the first one out ends nothing; the last one out ends the run.
- In the studio, Endings → four front-door endings, only `king-lives` a failure; write `when: { left: back-door }` → refused, no such exit is declared.
