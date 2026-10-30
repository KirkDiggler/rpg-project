# The studio speaks every word the engine learned

**Scene:** the Encounter Studio on :3030: Site → Import YAML → `the-mad-king.yaml` from the stack content dir → Replace draft; then each walk site the same way.  
**Status:** in studio. Merged to web `dev`; the operator's studio look on :3030 is listed on the story's checklist and not yet ticked.  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S8 ([#732](https://github.com/KirkDiggler/rpg-project/issues/732))  
**Roadmap:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714)

## Idea

The keep's brief sets the finish line: "the agreed mechanics are authorable
and playable together through the real multiplayer experience"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
After S1–S5 the studio refused the keep and every walk site: "`fell` is not a
deed this build holds", "`toward: \"ally\"` is not a selector this build
resolves", "faction … unsupported field: options", "ending … unsupported
field: if"; the `dash` words of
[rpg-project#671](https://github.com/KirkDiggler/rpg-project/issues/671) were
missing as well ([#732](https://github.com/KirkDiggler/rpg-project/issues/732)).

## Decision

- No ruling: the slice mirrors the engine. Every refusal is the engine's sentence, word for word, and lands at the noun it names ([#732](https://github.com/KirkDiggler/rpg-project/issues/732)).
- Knowledge lines:
  - [web L10](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/knowledge/decisions.md#L10): "the studio mirrors the engine and never judges for it …"
  - [web L12](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/knowledge/decisions.md#L12): "download writes the encoder's bytes, never the source's …"
  - [web L13](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/knowledge/decisions.md#L13): "a one-step `until` keeps the mapping spelling …"
  - [web L14](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/knowledge/decisions.md#L14): "the Endings panel fires an ending either "when a condition holds" …"

## Implementation

- rpg-dnd5e-web · [#1357](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1357) · `fell` with a "Who fell" picker; `ally` on toward, dash and away; `dash` with `enemy: walk | dash | far` and `threat`; a faction's `options` in the Factions panel; endings `left`, `if` and `outcome`; `until` as numbered steps with Then. The web's keep fixture is the toolkit's; the keep and five walk sites import, and Download is a fixed point. · [run record](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/knowledge/runs/2026-10-30-franks-list-s8-words.md)
- Not built: the version-2 builder still refuses `who` and does not read `threat`. Download writes the encoder's bytes, so a hand-written file's comments do not survive; the keep's empty `mapLabels: []` comes back absent.

## Verify

- Import `the-mad-king.yaml` → it opens with no refusal (10 areas, 18 creatures, 7 doors).
- Dispositions → garrison and party → two Until steps, guards-turned then allied, party-recognized then hostile, "the first of these that holds wins".
- Factions → loyalists → `yield` and `refuse`; Endings → `king-slain`, `king-deposed`, `freed`, `king-lives`, all "leaves by front-door", only `king-lives` a failure.
- Download YAML → import it → Download again → identical files.
- Import `gate-walk`, `ally-walk`, `survivors-walk` and `theft-walk` the same way → each imports.
