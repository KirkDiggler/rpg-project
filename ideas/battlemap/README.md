# ideas/battlemap — the surface the game is played on

The grouping for designs about the gridded surface: how it is measured, what
stands on it, how it is built, and how it is authored. The word is the one a
5e table uses for the combat map. It is a domain beside `spells`, `characters`
and `living-world`, not a code package.

| design | what it owns |
|---|---|
| [terrain](terrain/design.md) | what stands on the map and what that does to a cell: footprints, coverage, the cell-fact fold, the distance field. The primitive the group is named around. |
| [directed-movement](directed-movement/design.md) | an effect moves a creature: a directive (away / toward / line, budget, pays, provokes) resolution describes and encounter routes and walks. First proof Thunderwave, then Dissonant Whispers. |
| [coordinate-types](coordinate-types/design.md) | room-local vs dungeon-absolute positions, and the one bridge between them |
| [dungeon-walls](dungeon-walls/design.md) | straight modular walls at the room envelope |
| [multi-room-dungeons](multi-room-dungeons/design.md) | rooms with absolute positioning, connected |
| [dungeon-authoring](dungeon-authoring/design.md) | the YAML dungeon definition |
| [dungeon-builder](dungeon-builder/design.md) | the authoring tool and its walls, facings, lighting, and specimens |

Filed together on 2026-09-11 because "group it when the second arrives" had
never once happened in this tree. Sight and viewer-scoped knowledge
(`fog-of-war`, `perceive`, `monster-intel`) are a different primitive and were
deliberately not moved here.
