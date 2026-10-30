# A faction can turn more than one way; the captain follows the party

**Scene:** walk site `ally-walk` on the stack: the veteran outside the gatehouse; in the keep, the gate captain after `turn`.  
**Status:** in play. Walked on `ally-walk` 2026-10-30 ("looks good to me"). The companion is in no party roster or party-side turn order yet; that is S9 ([rpg-project#733](https://github.com/KirkDiggler/rpg-project/issues/733)).  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S3 ([#717](https://github.com/KirkDiggler/rpg-project/issues/717))

## Idea

The keep's brief says "the same actors can become allied, hostile, retreat,
reinforce another encounter or protect someone elsewhere" and "companions
follow and fight under AI control"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
`allied` was a static stance: an `until` on an allied pair was refused, each
pair got exactly one flip, behaviour never read allied and no selector named
an ally
([knowledge finding](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L20)).

## Decision

- D2, the ally word: "selector `ally`; allegiance through the faction's disposition, no personal `follows:` binding" ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D11, one pair, two flips: "turning hostile on recognition and waiting for the captain's turn to change our disposition sound like good ideas": `until` is a list of steps, each a predicate and the stance it turns to, and the first that holds wins ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D13, open: a recruited ally the hero strikes stays allied; the lean is that harm across an allied pair turns it neutral ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- Knowledge line: none yet for D2, D11.

## Implementation

- rpg-toolkit · [#2156](https://github.com/KirkDiggler/rpg-toolkit/pull/2156) · `mind/behavior` v0.17.0: the `ally` selector on `toward`, `dash` and `away`, refused on `attack`; a follow row is a candidate only while an ally is held.
- rpg-toolkit · [#2157](https://github.com/KirkDiggler/rpg-toolkit/pull/2157) · encounter: `until` as one step or a list with `then`; `ally` resolved from the creature's own sight and memory, its own faction excluded. The keep's garrison and party pair carries `[{ fact: guards-turned, then: allied }, { fact: party-recognized, then: hostile }]`, and the captain's march row is `toward: ally`. Site `ally-walk`. · [run record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/knowledge/runs/2026-10-29-franks-list-s3-companions.md)
- Not built: the roster, the party-side turn order and the recap's "who came with you" (S9). A list of more than one step names facts only. A follower one step behind a hero at a doorway joins the fight only once it sees a foe itself.

## Verify

- On `ally-walk`, say `join` to the veteran (Persuasion DC 10) → "Aye. Lead on." and a stance beat: party and veterans allied.
- Walk away a few cells → his pick is `toward ally`; he steps up beside you.
- Enter through the open south door → the fight forms with him on your side; he hits a thug, never you.
- Replay without `join` and walk away → he holds.
- In the keep, learn Ashford from the cook, then `turn` the captain → the garrison is allied and the captain follows you; fail `stand-aside` after and he stays allied.
