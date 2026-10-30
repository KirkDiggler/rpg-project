# A knight who saw the king fall acts on it

**Scene:** walk site `gate-walk` on the stack: drop the captain with the watcher in sight and the sentry behind the gate.  
**Status:** in play. Walked on `gate-walk` 2026-10-29. In the keep, the court stands down the moment the king falls, so a knight's `fell` row shows only after a fresh blow (D12, open).  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S2 ([#716](https://github.com/KirkDiggler/rpg-project/issues/716))

## Idea

The keep's brief says "a named actor's death can transform living … NPCs into
hostile variants while preserving positions and mission history"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
A row reads only what its creature holds, and a member going down landed no
deed and minted no fact, so no row could react to a named death
([knowledge finding](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L19)).

## Decision

- D1, how a death reaches a row: "a deed `fell: { who, within }` landed on witnesses; testimony, never a world flag" ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D6, hostile variants: "behaviour and allegiance change (S2 + `until: down`) plus a reserve wave; no runtime template swap; revisit after S2 walks with Frank" ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D8, two falls by one hero: "go with the first option, file by who fell. maybe the target will be the right answer but I will wait for a use case to show that" ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D12, open: the keep's `until: { down: king }` is world truth and stands the whole court down at once; the lean keeps both and names an `until` that reads the faction mind's testimony, which needs mind succession, as the real entry ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- Knowledge line: none yet for D1, D6, D8.

## Implementation

- rpg-toolkit · [#2149](https://github.com/KirkDiggler/rpg-toolkit/pull/2149) · `mind/behavior` v0.16.0: `fell` joins the deed words; `who` is legal only on `fell`; a fall is filed by who fell as well as actor and verb.
- rpg-toolkit · [#2151](https://github.com/KirkDiggler/rpg-toolkit/pull/2151) · encounter: a fall lands a `fell` deed on everyone whose sight reached the fallen; the dialect seals `who` to a monster placed in the file; the keep's two knights carry `{ when: { fell: { who: king, within: 3, on: ally } }, away: enemy, weight: 3 }`. · [run record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/knowledge/runs/2026-10-29-franks-list-s2-fell.md)
- Not built: `away: actor` on a `fell` row ignores `who`, so with two falls held it can name the wrong dropper; a fall no blow explains (a death save, a hazard) names the fallen as its own actor.

## Verify

- On `gate-walk`, drop the captain → the watcher who saw it runs; the sentry who only heard does not.
- In the studio, Tables → keep-patrol → the first row reads "when an ally fell within 3 (king) · weight 3 · away enemy".
- In the studio, name a prop in `who` (`who: altar`) and Save & Play → refused at the row: "`altar` is not a placement in this dungeon".
- Write `who` on `attacked` → refused; the sentence names `attacked` and says only `fell` takes `who`.
