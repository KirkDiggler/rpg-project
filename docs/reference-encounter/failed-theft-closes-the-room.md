# A failed theft closes that room's chances, not the keep's

**Scene:** walk site `theft-walk` on the stack: walk in through the open south door to the cook; in the keep, the cook in the kitchen.  
**Status:** in play. Engine on the stack as `theft-walk`; the studio's `not` editor is on web `dev`. No operator walk is recorded on the slice issue.  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S5 ([#719](https://github.com/KirkDiggler/rpg-project/issues/719))  
**Roadmap:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714)

## Idea

The keep's brief asks to "support room-local social consequences: failed
theft can close that room's theft/dialogue opportunities without creating a
universal alarm"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
The check, the failure row and the fact's local audience were all authorable;
the predicate grammar had no `not`, so no later line could be withheld on a
fact
([knowledge finding](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/encounter/knowledge/decisions.md#L22)).

## Decision

- D3, whose knowledge `requires: { fact }` reads: "keep the speaker's knowledge; land `not`; a cook who forgets is the finding that brings the second reading" ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- D14, open: `requires` takes one predicate, so a caught thief can still bribe; the lean leaves it as a story tonight and brings an `all:` form when a second use case names it ([lane](https://github.com/KirkDiggler/rpg-project/issues/722)).
- Knowledge line: none yet for D3.

## Implementation

- rpg-toolkit · [#2166](https://github.com/KirkDiggler/rpg-toolkit/pull/2166) · `{ not: <one predicate> }` wherever a predicate is, judged where and when the inner one is; a waited-for `not` of a fact, a round or a fall is refused by the liveness law. The keep's cook gains `pilfer` (Sleight of Hand DC 13, failure "Thief!" teaching `caught-in-kitchen`), and `pilfer` and `gossip` carry `requires: { not: { fact: caught-in-kitchen } }`. Site `theft-walk`. · [run record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/rulebooks/dnd5e/knowledge/runs/2026-10-30-franks-list-s5-not.md)
- rpg-dnd5e-web · [#1355](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1355) · the predicate editor and the requires picker offer `not`; the Read back says "Offered unless the speaker knows caught-in-kitchen." · run record: unsourced (rpg-dnd5e-web `dev` has no S5 run record).
- Not built: `ashford` and `bribe` already carry a `requires`, so they cannot also carry the `not` (D14).

## Verify

- On `theft-walk`, walk in to the cook, say `pilfer` and fail it (Sleight of Hand DC 13) → "Thief!", `caught-in-kitchen` in the record, the cook's menu empty for that character; a second character who stayed outside still sees `pilfer` and `gossip`.
- Walk out to the pedlar (four cells west of the start) → his line is offered and answers.
- Leave by the front door → the run ends `caught`, failure; replay, beat or skip the pilfer, leave → `clean-getaway`, success.
- In the keep, fail the cook's `pilfer` → `gossip` and `pilfer` are gone; `bribe` is still offered (D14).
- In the studio, an option's requires → `not` → fact → the Read back says "Offered unless …".
