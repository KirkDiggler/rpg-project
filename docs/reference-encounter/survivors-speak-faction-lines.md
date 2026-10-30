# A beaten side's survivors speak the faction's lines

**Scene:** walk site `survivors-walk` on the stack: walk north into the gatehouse yard and drop the band's leader; in the keep, bring the king down.  
**Status:** in play. On the stack as `survivors-walk`; no operator walk is recorded on the slice issue.  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S4 ([#718](https://github.com/KirkDiggler/rpg-project/issues/718))  
**Roadmap:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714)

## Idea

The keep's brief says "when one side is defeated, pause for a survivor
conversation offering cooperation or continued fighting. Surviving
subordinates can replace dead leaders as speakers"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
`until: { down: <leader> }` already dissolves the fight, but options lived on
one placement, so a dead leader's lines died with him
([knowledge finding](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/knowledge/decisions.md#L38)).

## Decision

- D4, more than one flip per pair: "no list yet; a refusing survivor who swings turns the pair hostile by provocation, which already exists" ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)). D11 brings the list for a different use case; see [the faction card](faction-turns-captain-follows.md).
- D12, open: world truth stands the court down at once; testimony lets each knight react on his own. The lean keeps both tonight and names mind succession, "surviving subordinates replace dead leaders as speakers", as the use case for an `until` that reads the faction mind's testimony ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- Knowledge lines:
  - [encounter L25](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L25): "no `until` list: a refusing survivor who swings …" (D4; the Friction line [L83](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L83) holds it against D11)
  - [encounter L23](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L23): "a faction may carry `options` …"
  - [encounter L26](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L26): "a down creature answers nothing …"
  - [encounter L27](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L27): "the castle's loyalists carry `yield` and `refuse` …"
  - [encounter L89](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L89): "S2's `fell` rows are testimony …" (D12, Open)

## Implementation

- rpg-toolkit · [#2159](https://github.com/KirkDiggler/rpg-toolkit/pull/2159) · `options` on a faction, laid wholesale under a member's own, judged by the one option grammar; speaking to a down member is refused with a sentence. The keep's loyalists carry `yield` (Persuasion DC 12) and `refuse` (Intimidation DC 15), and loyalists and party turn neutral `until: { down: king }`. Site `survivors-walk`. · [run record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/knowledge/runs/2026-10-29-franks-list-s4-survivors.md)
- Not built: `requires` cannot read a `down`, so the keep's `yield` is ungated. The loyalists' mind is the king, so after his fall `loyalists-yielded` folds no loyalist pair. A down member still counts as a witness. Session has no arm for the down refusal; no path reaches it today.

## Verify

- On `survivors-walk`, drop the leader (one blow) → the log says the band stood down; the fight dissolves.
- Speak to a follower: "Yield and live." (DC 10) → "We yield."; nobody swings as you move around them.
- Replay: drop the leader, miss "Throw down your arms.", strike that follower → "We fight on.", then hostile again and the fight re-forms after your blow.
- Speak to the downed leader → no lines offered; a forced line is refused with a sentence.
- In the keep, bring the king down → the court stands down; a surviving knight offers `yield` and `refuse`.
