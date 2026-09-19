# The engine grades the file — `ValidateDungeon` for the World Builder

**Status:** DRAFT for Kirk's rulings (2026-09-19). Lane: toolkit (platform). Small: one additive RPC, one api handler, one web call.

**Where it came from (Kirk, 2026-09-19):** "I was thinking the web could send us a yaml and if we
supported it we could give it the passing grade … I only want us to validate that the engine
supports it. a missing faction or a typo somewhere we should catch right?"

## The one sentence

The World Builder asks the engine whether a file plays, and shows the engine's own paths and
sentences; the web stops mirroring the gameplay grammar the way the engine stopped mirroring
the editor's bounds (rpg-project#479).

## What exists, measured

- `AuthoringService.PutDungeon` already compiles the file through `dungeonspec.Load` and returns
  `repeated FieldError { path, message }` (`authoring/v1alpha1/service.proto:90-115`,
  `put_dungeon.go:39`). But it PERSISTS on success and is gated behind `RPG_AUTHORING_ENABLED`.
  There is no validate-only door.
- The web mirrors the gameplay grammar in four files: `src/author/answerVocabulary.ts`,
  `src/author/factionVocabulary.ts`, `src/concepts/world-building/answerTableShape.ts`,
  `strictShape.ts`. A mirror drifts; rpg-dnd5e-web#1119 was the web refusing a file the engine
  accepts, and #1145 refused `temper` on a binding the engine takes.
- The engine's refusals are already an author's contract. Probed on 2026-09-19 against
  `rulebooks/dnd5e/encounter` v0.94.0 with the shipped `reference-front-room.yaml`, one mutation
  each:

```
missing faction   place[0].faction: "dnd5e:monsters:goblin" is in faction "gobins", and no faction in this dungeon has that id — declare it under `factions:`
typo trigger key  factions[0].on.intimdate_failed: "intimdate_failed" is not a trigger this build rolls: they are intimidated, intimidate_failed, persuaded, persuade_failed, time
unknown key       line 129: field tempre not found in type dungeonspec.FactionSpec
```

The third is the one defect in the contract: in the v2 dialect an unknown key surfaces as a
YAML decode error with a line number and a Go type name, not as a `FieldError` with a path. The
single-room dialect already walks the shape and names unknown keys by path.

## The shape

### The RPC, additive

```proto
// ValidateDungeon compiles a dungeon file exactly as PutDungeon would and reports
// every defect, persisting nothing. Ungated: it mutates nothing. An empty errors
// list is the engine saying "this file plays"; it says nothing about the room's
// appearance, which the World Builder's own codec judges (rpg-project#479).
rpc ValidateDungeon(ValidateDungeonRequest) returns (ValidateDungeonResponse);
message ValidateDungeonRequest  { string yaml = 1; }
message ValidateDungeonResponse { repeated FieldError errors = 1; string key = 2; string name = 3; }
```

`key` and `name` come back so the builder can show what the engine thinks it was handed. Same
`FieldError` message `PutDungeon` uses; no second shape.

### The api handler

Decode, validate, compile through `dungeonspec.Load`; map `*ValidationError` to the list. No
registry, no Redis, no gate. Transport only.

### The engine's one fix (toolkit, `rulebooks/dnd5e/encounter`)

Unknown keys in the v2 dialect become `FieldError`s with a path, as the single-room dialect
already does, so the response never carries a Go type name or a line number. That is the only
engine change, and it is a fix to the existing contract, not a new grammar.

### The web

Call it on save and on the preview refresh; render the list next to the codec's own scene
defects, each labelled by its owner ("won't play" from the engine, "can't draw" from the
codec). Then delete the four mirror files' VERDICT logic; keep the word lists the palette needs
to offer completions. Those lists can come from an RPC later; not now.

## Ownership

| Noun | Owner |
|---|---|
| Whether a file plays | the engine (dungeonspec), reached through `ValidateDungeon` |
| Whether the scene is well-formed | the web's codec |
| The words a palette offers | the web, as lists, until an RPC brings them |
| The paths and sentences an author sees | the engine, verbatim |

## Rulings needed

- **R1.** Additive `ValidateDungeon` on the AuthoringService, ungated, same `FieldError`, no
  persistence; `key`/`name` echoed.
- **R2.** The v2 dialect's unknown-key refusal becomes a pathed `FieldError` (engine fix, one
  module) before the RPC ships, so the contract holds for both dialects.
- **R3.** The web deletes its gameplay verdicts (keeps word lists) in the same wave.

## Slices

| # | Repo | What | Proof |
|---|---|---|---|
| 1 | rpg-api-protos | the RPC and two messages | buf lint/breaking, generate compiles |
| 2 | rpg-toolkit encounter | v2 unknown key → `FieldError{path}` | the probe above returns `factions[0].tempre: …` with no Go type name |
| 3 | rpg-api | handler; test: the three probes above through the RPC | handler tests |
| 4 | rpg-dnd5e-web | call + render; delete mirrored verdicts | a file with a missing faction shows the engine's sentence at the path; #1145's `temper` case passes with no web rule |

Walk: in the World Builder, misspell a faction, see the engine's sentence at the path; fix it,
see the grade pass; save.

## Done when

- `grep -rn "is not a trigger this build rolls" rpg-dnd5e-web/src` finds nothing: no engine
  sentence is authored twice.
- The three probes return pathed `FieldError`s through the RPC, including the unknown key.
- The World Builder shows two grades from two owners and never refuses a file the engine plays.
