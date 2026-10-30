# The reference encounter

The keep, `the-mad-king`, is the one reference encounter: every mechanic the
engine learns for an authored dungeon is shown there, or on a small walk site
cut from it until the keep has named scenes. Each card below is one mechanic it
demonstrates, with four sections: the **Idea** (what the brief asked for and
what the engine lacked), the **Decision** (the rulings, by their words), the
**Implementation** (the PRs and run records) and **Verify** (do this, expect
that). Each card's **Scene** line says where in the keep, or on which walk
site, to start. The story is
[rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714), the
brief is [rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)
and the decisions are on the lane
[rpg-project#722](https://github.com/KirkDiggler/rpg-project/issues/722).

**Status** is one of: `in play` (merged and on the castle stack), `in studio`
(merged to web `dev`), `engine only`, `planned`. The castle stack serves the
engine's side through
[rpg-api#1152](https://github.com/KirkDiggler/rpg-api/pull/1152), which is
still a draft and not yet on api `dev`.

| Mechanic | Scene | Repos | Status | Card |
|---|---|---|---|---|
| A guard opens the shut door he walks into | `gate-walk`; the keep's throne door | rpg-toolkit | in play | [card](guard-opens-shut-door.md) |
| A knight who saw the king fall acts on it | `gate-walk`; the keep's knights | rpg-toolkit | in play | [card](knight-acts-on-the-fall.md) |
| The run ends when the last one walks out the front door | `gate-walk`; the keep's front door | rpg-toolkit | in play | [card](run-ends-at-the-front-door.md) |
| A faction can turn more than one way; the captain follows the party | `ally-walk`; the keep's gate captain | rpg-toolkit | in play | [card](faction-turns-captain-follows.md) |
| A beaten side's survivors speak the faction's lines | `survivors-walk`; the keep's court | rpg-toolkit | in play | [card](survivors-speak-faction-lines.md) |
| A failed theft closes that room's chances, not the keep's | `theft-walk`; the keep's cook | rpg-toolkit, rpg-dnd5e-web | in play | [card](failed-theft-closes-the-room.md) |
| The studio draws a patrol route and gives an NPC an extra action | the studio, the keep imported | rpg-dnd5e-web | in studio | [card](studio-patrol-route-and-extra-action.md) |
| The studio speaks every word the engine learned | the studio, the keep and every walk site imported | rpg-dnd5e-web | in studio | [card](studio-speaks-every-word.md) |
| The outcome and the companion reach the screen | `ally-walk`, `gate-walk` | rpg-api-protos, rpg-api, rpg-dnd5e-web | planned | [rpg-project#733](https://github.com/KirkDiggler/rpg-project/issues/733) (no card yet) |

A new mechanic gets a row here and a card beside this file when its slice is
shaped; its status moves as it merges, reaches the stack and is walked.
