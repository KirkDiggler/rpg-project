# A guard opens the shut door he walks into

**Scene:** walk site `gate-walk` on the stack: strike the captain and watch the sentry behind the shut gate; in the keep, the throne room door is shut on arrival.  
**Status:** in play. Walked on `gate-walk` 2026-10-29 ("the sentry came through the gate, watcher ran, worked").  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S1 ([#715](https://github.com/KirkDiggler/rpg-project/issues/715))  
**Roadmap:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714)

## Idea

The keep's brief asks that "patrols visit authored locations through the
actual connected dungeon" and that "surviving hostile guards pursue after the
objective is completed"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
The engine had no creature that opens a door: a driven walk routes only
through open doors, so every patrol, march and pursuit stops at the first shut
one, and the keep authored its throne door open for the AI's sake
([knowledge finding](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L21),
[rpg-toolkit#2069](https://github.com/KirkDiggler/rpg-toolkit/issues/2069)).

## Decision

- D5, creatures and locked doors: "never, for S1; carrying a key is Grant's neighbour" ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- Knowledge line: none yet for D5.

## Implementation

- rpg-toolkit · [#2150](https://github.com/KirkDiggler/rpg-toolkit/pull/2150) · a creature's own walk (toward, dash, away, search, pursuit, a patrol leg) routes through a closed, unlocked door at the cost of the step and opens it through the same change a player's open makes: the door beat names the creature, and `opened: { fact }` lands on who saw it. Locked doors, pushes, routs, the public route and threat zones still stop. · [run record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/knowledge/runs/2026-10-29-franks-list-s1-doors.md)
- The keep authors `throne-wall-door: { closed: true }`, and its storm test asserts the garrison opens it.
- Not built: a patrol leg through a locked door ends in front of it with no stayed beat, and a pursuit that needs a lock loses its quarry silently. The unbounded route flood costs castle moves about 1.5x the time ([rpg-toolkit#2163](https://github.com/KirkDiggler/rpg-toolkit/issues/2163)). Threat zones still treat a shut door as closed.

## Verify

- On `gate-walk`, strike the captain → the sentry opens the shut gate and comes to the sound.
- In the keep, arrive → the throne room door is shut.
- In the keep, learn Ashford from the cook (`gossip`, then `ashford`), then `turn` the captain → a guard opens the shut throne door on the march, and the door beat names the guard.
- Shut a door on a pursuing guard → it opens the door and keeps the chase; the story log reads one open line.
- In the studio, lock a door the march needs and Save & Play → the march stops at it, and the record names the door and its lock ("door … is locked, DC …").
