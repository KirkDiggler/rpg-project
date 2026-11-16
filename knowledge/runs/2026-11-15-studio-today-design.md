# Run — 2026-11-15 — Studio today design: places, ways and the engine's read-back

Design for story [rpg-project#966](https://github.com/KirkDiggler/rpg-project/issues/966). Base: `rpg-project` `main` at [`71991a1`](https://github.com/KirkDiggler/rpg-project/commit/71991a1).

## Shipped

- The studio design gains rulings R25 to R41 and its walkthrough a section walking the Guard Captain and the front gate — [rpg-project#976](https://github.com/KirkDiggler/rpg-project/pull/976).
- A wave-one implementation plan with task contracts for four slices — [rpg-project#976](https://github.com/KirkDiggler/rpg-project/pull/976).
- Thirteen slices cut as sub-issues — [rpg-project#966](https://github.com/KirkDiggler/rpg-project/issues/966).

## Decided

- The engine's read-back is the single source for the inspector, because the studio's copy never learned machines and already disagreed with the engine; ruled by the operator 2026-11-15 — [rpg-project#966](https://github.com/KirkDiggler/rpg-project/issues/966).
- The Encounter side is people and places; an earlier cut grouped it by five author questions and was replaced when the operator said it "is people and places really" — [rpg-project#966 comment](https://github.com/KirkDiggler/rpg-project/issues/966#issuecomment-6886648030).
- A join is said as a read-back sentence, not sent as geometry, because the authoring service's notes warn against a second projection beside the atlas; ruled by the operator 2026-11-15 — [shape comment](https://github.com/KirkDiggler/rpg-project/issues/966#issuecomment-6888063363).
- A join can be a door, an opening with no door or open floor, not only a door; corrected by the operator from the keep blockout — [rpg-project#966](https://github.com/KirkDiggler/rpg-project/issues/966).
- The design keeps a world's shelf of pieces open (R35) and builds none of it; ruled by the operator 2026-11-15 — [rpg-project#977](https://github.com/KirkDiggler/rpg-project/issues/977).
- Design diagrams follow one thing through; a per-repository box diagram was replaced after the operator called it forced — [rpg-project#976](https://github.com/KirkDiggler/rpg-project/pull/976).
- The read-back rides the validate-only check and never a save, after CI showed a read-back on every save pushed four castle tests past the 10 s hard line (R36) — [rpg-project#967 comment](https://github.com/KirkDiggler/rpg-project/issues/967#issuecomment-6892012041).
- A join is decided by movement and a window is a sight seam, after review of the joins found a wall can answer sight and feet separately (R38) — [rpg-toolkit#2441 review](https://github.com/KirkDiggler/rpg-toolkit/pull/2441#pullrequestreview-5909893153).
- The engine says the ways, with bars, after the first cut of the places view would have had the studio work them out and a review of the first engine cut showed the rules overclaim off the castle (R39) — [rpg-toolkit#2443 review](https://github.com/KirkDiggler/rpg-toolkit/pull/2443#pullrequestreview-5913211546).
- The fold is the engine's one-thing fold and the studio never folds; the operator asked for the full sentence behind each entry (R37) — [rpg-project#978](https://github.com/KirkDiggler/rpg-project/issues/978).

## Found

- The castle's ten behaviour definitions have no studio entry point, and the inspector says "no table and no rows" for a creature that runs a machine — [rpg-project#944](https://github.com/KirkDiggler/rpg-project/issues/944).
- The castle holds 34 checks: Persuasion 18, Deception 5, Performance 3, Intimidation 2, thieves' tools 2, and one each of Investigation, Sleight of Hand, Strength and Religion — [rpg-project#966 comment](https://github.com/KirkDiggler/rpg-project/issues/966#issuecomment-6886648030).
- The engine's read-back says the castle in 642 statements in about 170 ms and leaves nothing unsaid; no API call carries it — [shape comment](https://github.com/KirkDiggler/rpg-project/issues/966#issuecomment-6888063363).
- Region-to-region joins exist nowhere in the engine; door crossings and the floor's partition do — [shape comment](https://github.com/KirkDiggler/rpg-project/issues/966#issuecomment-6888063363).
- The live studio is `src/concepts/encounter-studio`; `src/author` is the unrouted older builder — [rpg-project#966](https://github.com/KirkDiggler/rpg-project/issues/966).
- The captain's stand-aside is a full way through the gate, and so is turning the garrison; the hand list made on 2026-11-15 missed both — [rpg-project#981 comment](https://github.com/KirkDiggler/rpg-project/issues/981#issuecomment-6902670769).

## Mistaken

- The first scouting brief pointed at `src/author` as the studio; the scout found the routed code and reported the brief wrong — [rpg-project#966](https://github.com/KirkDiggler/rpg-project/issues/966).
- The first shape said "a door joins A and B"; the operator corrected it to regions meeting by a door, an opening or open floor before any slice was built — [rpg-project#966](https://github.com/KirkDiggler/rpg-project/issues/966).
- The wave-one plan said the api carries the read-back on a save as well as a check; the design already said the check, and CI showed the cost — [rpg-project#967 comment](https://github.com/KirkDiggler/rpg-project/issues/967#issuecomment-6892012041).
- The places-view slice was first written with the studio deriving a join's ways from its connections, against the design's own law that the engine derives them; a slice for the engine to say them was put in front — [rpg-project#981](https://github.com/KirkDiggler/rpg-project/issues/981).
