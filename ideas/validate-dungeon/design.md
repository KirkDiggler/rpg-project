# The engine grades the file — `ValidateDungeon` for the World Builder

**Status:** SHIPPED 2026-09-19 (rpg-project#481 RULED, corrected in #482 before build; three PRs on pseudo-versions, merged inside-out; the web slice is Kirk's lane in the new World Builder).

| Repo | Release |
|---|---|
| rpg-toolkit | rulebooks/dnd5e/encounter v0.94.1 (#1843: an unknown key in EITHER dialect is a pathed `FieldError` — `"<key>" is not a key this build reads: they are <list>` — by reflection over the spec shapes, every unknown key reported, not just the first) |
| rpg-api | dev 16584f93 (#1018: the registry's authoring gate moved to writes only — `if !in.ValidateOnly && !r.authoring` — so a read-only registry grades a file; wire test probes missing faction, typo trigger, unknown key, and asserts the unknown key comes back pathed) |
| rpg-dnd5e-web | NOT MERGED. #1162 (`feat/engine-grades-the-file`) stands as a draft with the refusal map in its comment; Kirk is authoring in the new World Builder and takes, cherry-picks, or closes it. The contract the web needs: `PutDungeon({key, yaml, validateOnly: true}) → {errors: FieldError[{path, message}]}`; empty errors = plays; a gRPC status only for a request the engine cannot name; the scene is not graded; authoring off = Unimplemented. |

**What the build corrected.** The RULED text claimed the single-room dialect already named unknown keys by path; it did not (its `KnownFields` decode surfaced the same YAML line-and-Go-type error as v2). #1843 fixed both dialects. Deferred, with owners: the hand-read shape's refusal carries no "they are" list (rpg-toolkit#1844); ungating validate-only is unobservable when `AuthoringService` is not registered — every deployed env sets `RPG_AUTHORING_ENABLED=1`, so it is documented, not built.

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
  `put_dungeon.go:39`), and it already has `validate_only = 3`, documented as *"the builder's
  per-edit preview"* (`service.proto:72-77`). The web already calls it on every edit
  (`src/author/authoringRpc.ts:118`). **The first draft of this note missed that and proposed a
  new RPC; corrected 2026-09-19.** What IS missing: `FileRegistry.Put` returns
  `ErrAuthoringDisabled` at its top, before the `ValidateOnly` branch (`registry.go:291` vs the
  branch below it), so a read-only registry cannot grade a file at all.
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

The third is the one defect in the contract: an unknown key surfaces as a YAML decode error
with a line number and a Go type name, not as a `FieldError` with a path. **Corrected 2026-09-19
by the build:** this is true of BOTH dialects. The first draft claimed the single-room dialect
already named unknown keys by path; it does not (`DecodeSingleRoom` hands the decoder's error
through as one pathless defect, and its test pinned the Go text). toolkit#1843 fixes both.

## The shape

### No new RPC: ungate `validate_only`

`PutDungeon{validate_only: true}` is the door. The change is in rpg-api only: a validate-only
request compiles and answers whether or not the registry is writable; `ErrAuthoringDisabled`
applies to the WRITE, not the grade. Reading content mutates nothing (the proto's own words for
`GetDungeon`), and grading a file the caller supplies mutates nothing either. The proto doc for
`validate_only` gains one sentence saying so; no field changes.

### The engine's one fix (toolkit, `rulebooks/dnd5e/encounter`)

Unknown keys in both dialects become `FieldError`s with a path and the sentence form
`validation.placeOn` already uses (`"tempre" is not a key this build reads: they are id, mind,
on, temper`), so the response never carries a Go type name or a line number. Every unknown key
in the file is reported. That is the only engine change, a fix to the existing contract.

**Deferred, with the reason:** the most-authored shape reads its own keys in a custom
unmarshaler, so its refusal offers no "they are" list; hoisting the list so the unmarshaler and
the sentence share it is a follow-up (toolkit issue filed by the ship record). And with
`RPG_AUTHORING_ENABLED` off the server registers no AuthoringService at all, so a read-only
server cannot grade a file, nor serve `GetDungeon`, even after rpg-api#1018; every shipped
environment sets the flag on, so registering the service always and gating only the write is
deferred until an environment needs it.

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

- **R1 (corrected).** No new RPC. `PutDungeon{validate_only}` answers on a read-only registry;
  the gate moves to the write.
- **R2.** The v2 dialect's unknown-key refusal becomes a pathed `FieldError` (engine fix, one
  module) before the RPC ships, so the contract holds for both dialects.
- **R3.** The web deletes its gameplay verdicts (keeps word lists) in the same wave.

## Slices

| # | Repo | What | Proof |
|---|---|---|---|
| 1 | rpg-toolkit encounter | v2 unknown key → `FieldError{path}` | the probe above returns `factions[0].tempre: …` with no Go type name |
| 2 | rpg-api | validate-only answers on a read-only registry; the three probes through `PutDungeon{validate_only}` | orchestrator + handler tests, one with authoring disabled |
| 3 | rpg-api-protos (doc only, optional) | one sentence on `validate_only`: ungated | buf lint |
| 4 | rpg-dnd5e-web | render the engine's grade as the verdict; delete mirrored verdicts, keep word lists | a file with a missing faction shows the engine's sentence at the path; #1145's `temper` case passes with no web rule |

Walk: in the World Builder, misspell a faction, see the engine's sentence at the path; fix it,
see the grade pass; save.

## Done when

- `grep -rn "is not a trigger this build rolls" rpg-dnd5e-web/src` finds nothing: no engine
  sentence is authored twice.
- The three probes return pathed `FieldError`s through the RPC, including the unknown key.
- The World Builder shows two grades from two owners and never refuses a file the engine plays.
