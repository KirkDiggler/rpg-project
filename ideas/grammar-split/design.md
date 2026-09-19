# One gameplay grammar, two geometry dialects — the `siteSpec` adapter goes

**Status:** PROPOSED 2026-09-19. Second wave of rpg-project#479 (§4, R6). Lane: toolkit (platform), one module: `rulebooks/dnd5e/encounter`. No proto, api, or web change.

**Where it came from (Kirk, 2026-09-19):** "we are meant to be composable but it feels like we
are making another application again … before we build on this let's get it cleaned up." And
today: "Let's do it."

## The one sentence

The factions, dispositions, answer tables, tempers and actions an author writes are judged and
compiled by one set of functions, whichever geometry the room is drawn in; the single-room
dialect stops impersonating a v2 document to reach them.

## What is true today, measured

Verified against rpg-toolkit `origin/main` e94237d5 (encounter v0.94.1) on 2026-09-19. Paths are
under `rulebooks/dnd5e/encounter/dungeonspec/`.

**The types are already one grammar.** `SingleRoomSpec.Factions` and `.Dispositions`
(`single_room.go:29,33`) are `[]FactionSpec` and `[]DispositionSpec`, the same types v2 uses
(`spec.go:308,351`). A binding's `on`, `temper`, `actions` (`single_room.go:136,141,148`) are the
same `map[string][]AnswerSpec`, `string`, `[]string` as `PlaceSpec.On/.Temper/.Actions`
(`spec.go:1571,1582,1454`). `AnswerSpec`, `WhenSpec`, `SelectorSpec`, `TemperSpec`, `FleeSpec`,
`HoldSpec` have no second copy. The one geometry-shaped field inside the grammar is
`SelectorSpec.At *[2]int` (`spec.go:1148`), an offset cell.

**The functions are not.** Every gameplay validator is a method on `validation`, whose receiver
is built from a v2 `*Spec`. The single-room path reaches them by building a fake one:

| Step | Where |
|---|---|
| `siteSpec()` builds `Spec{Key, Factions, Dispositions}` and one synthetic `PlaceSpec` per monster with `At` left at `[0,0]` | `single_room_site.go:151-165` |
| `siteValues()` runs `factions`, `factionOrders`, `placeFaction`, `placeOn`, `placeTemper`, `placeActions`, `minds`, `dispositions` on it, then re-emits the errors through the dialect's sink | `single_room_site.go:174-216` |
| a `cellRefusal` string on the receiver makes `entrySelector` refuse `at:` and return early, because the fake spec has no floor | `validate.go:207-217, 1546-1551`; set at `single_room_site.go:181` |
| `CompileSingleRoom` copies `monstersOf`'s three lines (`Actions`, layered `Table`, `Temper`) and its two index builds instead of calling them | `single_room_compile.go:49-54, 96-102` vs `compile.go:990-995, 1013-1017`; the comment at `:28-31` admits it |
| a binding that names no live monster is a separate adapter error | `errLiveRoomMonster`, `single_room_site.go:57, 210` |

**What the adapter costs.** In v2 the gameplay validators run inside `if v.geometryUsable()`
(`validate.go:102-124`); through the adapter they run unconditionally, with a comment
explaining why that happens to be safe. The faction-level `at:` refusal is reachable in the
single-room dialect (`factionOrders` → `placeOn("factions[i]")` → `entrySelector`,
`validate.go:1627-1630`) and pinned by no test; `TestSingleRoomSiteRefusals`
(`single_room_site_test.go:128`) covers only the binding level. Both were named by #1834's
review and deferred here.

**The seam that must not move.** Nothing outside the package uses `Decode`, `Validate`,
`Compile`, `DecodeSingleRoom`, `CompileSingleRoom`, `Spec` or `SingleRoomSpec`. The whole
external surface is `Load`, `CompileTable`, `Compiled`, `MonsterPlacement`, `ErrBadSpec`,
`ValidationError`, `FieldError` (session `creaturetable.go:65`; rpg-api
`sessionworld.go:323-655`, `registry.go:399-436`).

**The unknown-key contract already assumes one grammar.** `authoredKeysOfType`
(`unknown_key.go:217-226`) walks both roots with one `seen` map, "a faction is a faction in
either dialect." Its reflection has no `Anonymous`/`,inline` case (`:248, :268-277`), so
embedding a shared struct into the two roots would corrupt the offered key list. This design
does not embed anything, so `unknown_key.go` is untouched.

## The principle it breaks

**Ownership before mechanism.** The v2 dialect owns offset cells and the floor. The single-room
dialect owns axial cells and the walkable set. The grammar owns neither, but today it can only
run with a v2 floor under it, so the second dialect fakes one. A fake owner is the mechanism
standing in for the ownership question: *who resolves a cell a selector names?* The dialect.

## The shape

### 1. The grammar is a value with two inputs from the dialect

A `grammar` (unexported) holds the error sink and receives from the dialect exactly what the
dialect owns:

- **members**: the live monster ids and the declared faction ids (the index the adapter's
  `errLiveRoomMonster` and `placeFaction` both need);
- **cells**: an interface with one method, resolve a `SelectorSpec.At` at a path to either a
  floor cell or a refusal. v2's implementation is today's floor lookup (`validate.go:1558`).
  The single-room implementation returns the shipped sentence *"at is not a place a single room
  can name yet: …"* (`single_room_site.go:49-51`). `cellRefusal` is deleted; the string branch
  becomes the interface.

Every gameplay validator (`factions`, `factionOrders`, `placeFaction`, `placeOn`, `placeTemper`,
`placeActions`, `minds`, `dispositions`, and what they call) moves onto the grammar, taking a
path prefix. `validation` keeps geometry and calls the grammar for the rest, inside
`geometryUsable()` exactly as today. `DecodeSingleRoom`'s validation calls the same grammar with
its own prefixes (`factions[i]`, `room.room.monsterBindings.<id>`) and its own two inputs.
`siteSpec`, `siteValues`, `errLiveRoomMonster`, `sortedBindings` are deleted.

### 2. Orders compile once

`ordersOf(id, ref, faction, on, temper, actions, factionIndex) MonsterPlacement` is the shared
three lines plus the two index builds. `monstersOf` calls it per `PlaceSpec`; `CompileSingleRoom`
calls it per binding. The transcription at `single_room_compile.go:49-54, 96-102` is deleted.
This closes #1834's second deferred thread.

### 3. The faction-level `at:` refusal is pinned

One test in the single-room dialect asserts `factions[0].on.<key>[0].toward.at` refuses with the
shipped sentence. This closes #1834's first deferred thread. The v2 counterpart
(`TestTheV2DialectStillResolvesCellSelectors`) stays.

### 4. v2 is not touched as an authoring surface

No key, sentence, or path changes in either dialect. `PlaceSpec` stays a union of prop and
monster; the single-room split of actor and binding stays. `SelectorSpec.At` stays `*[2]int`:
when the sites layer answers the frame question (rpg-project#479 §5), the cell type inside the
grammar becomes the dialect's, and that is that wave's change, not this one.

### 5. The proof is the pictures

`TestEveryContentFileCompilesToItsCommittedPicture` covers five v2 fixtures and both single-room
fixtures. Every golden is byte-identical before and after. `TestTheTwoDialectsCompileTheSameOrders`
(`single_room_site_test.go:81`) stays and changes meaning: it no longer proves the adapter
translates faithfully, it proves both dialects call one grammar.

## Ownership after

| Noun | Owner | Where |
|---|---|---|
| Faction, disposition, answer, temper, action legality | the grammar | `grammar.go` (new), functions with a path prefix |
| Which monster ids and faction ids exist | the dialect, handed to the grammar as members | each decoder |
| What a selector's cell means | the dialect, as the `cells` interface | `validate.go` (v2 floor), `single_room_decode.go` (refusal) |
| A creature's compiled orders | the grammar | `ordersOf` |
| Geometry: regions, walls, doors, start, footprints, walkable set | each dialect | unchanged |

## Rulings needed

- **R1.** The grammar takes its two inputs (members, cells) from the dialect; nothing in the
  grammar reads a `*Spec` or a `*SingleRoomSpec`. A validator that needs a third dialect input
  is a design change, not a parameter.
- **R2.** `cellRefusal` is replaced by the `cells` interface. The single-room sentence does not
  change.
- **R3.** `siteSpec` and everything only it needed are deleted, not kept as a fallback.
- **R4.** External surface unchanged: `Load`, `CompileTable`, `Compiled`, `MonsterPlacement`,
  `ErrBadSpec`, `ValidationError`, `FieldError`. This is a refactor release of `encounter`
  (patch bump), consumers do not repin.
- **R5.** No struct embedding across the two roots; `unknown_key.go` is untouched and its two
  key-list tests are the guard.

## Slices

One PR, one module (`rulebooks/dnd5e/encounter`), no pseudo-version consumers, no walk needed
beyond the goldens and the existing session/rpg-api compile tests against the new tag: the
external seam does not move. Review round runs as usual.

## Done when

- `grep -rn 'siteSpec\|cellRefusal\|errLiveRoomMonster\|siteValues' dungeonspec/` is empty.
- `CompileSingleRoom` contains no `Table:`/`Temper:`/`Actions:` literal; both compilers call `ordersOf`.
- A single-room test pins the faction-level `at:` refusal by path and sentence.
- All seven goldens byte-identical; `unknown_key_test.go` unchanged and green.
- `go test ./rulebooks/dnd5e/...` green; rpg-api `internal/sessionworld` and `internal/dungeons` green on the new tag.

## Next

Doors (rpg-project#468) is unblocked: Kirk confirms the door roles exist in the new World
Builder's assets (2026-09-19). Its re-base on "scene from the document, door state from the
atlas, joined by item id" can be designed while this builds; it lands on the split grammar, not
on the adapter. Then the sites layer, where `at:` gets its frame.
