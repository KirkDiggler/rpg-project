# Dungeon Builder on the Session Stack — implementation ledger

*Adjustments made while building against `design.md` / `plan.md`. The design
and plan stay open on rpg-project PR #255 through implementation; this file
is where reality corrects them. One entry per adjustment: what the design
said, what was built instead, why, and where (PR / commit).*

| Date | Where (PR) | Design said | Built instead | Why |
|---|---|---|---|---|
| 2026-08-23 | rpg-api#806 branch → T3 | `PutDungeon` answers with the atlas; rpg-api projects it from the compiled world (plan A: `Entry.Atlas session.Atlas`) | T3 exports `Manager.AtlasOf(ctx, &AtlasOfInput{World})` — a Manager method (not a package func: `LoadEncounter` needs Initiative/Standing/Sight/TurnDriver/Striker, and a free func would invent four of them); shares `loadAuthored` and `projectAtlas` with `Manager.Atlas`; rpg-api wires the session Manager as its AtlasProjector | The only producer of `session.Atlas` was `Manager.Atlas(session)` via unexported `projectAtlas`, and a dungeon in the registry has no session. Re-deriving the projection in rpg-api would be a second geometry path (the symmetric-bug rule); one projection, one producer. |
| 2026-08-23 | P (rpg-api-protos#238) | plan P kept `tests/declaration-remaining` for a follow-up | deleted in the same PR | Kirk: "we do not need tests in the protos, we have them mechanically compiled" |
| 2026-08-23 | rpg-api#820 | `Registry.Put(ctx, key, yaml, validateOnly)` | `Put(ctx, *PutInput) (*PutOutput, error)` | rpg-api's Input/Output struct rule |
| 2026-08-23 | rpg-api#820 | `Entry{…, Compiled dungeonspec.Compiled, Atlas}` | `Entry` carries `*sessionworld.Dungeon` | lobby needs absolute party seats, which under v1 exist only after the projection; revisit when T2's `Compiled` is absolute by construction |
| 2026-08-23 | rpg-api#820 | `RPG_CONTENT_DIR` required | defaults to `./content` when authoring is off; required when `RPG_AUTHORING_ENABLED=1` | a plain checkout / image must boot the tomb with no env |
| 2026-08-23 | rpg-api#820 | boot refuses a non-compiling file | also refuses filename ≠ `key` and a dir without `reference-tomb.yaml` | the default dungeon must exist; one name per dungeon |
| 2026-08-23 | rpg-dnd5e-web#781 | `buildScene3D` takes `layout` and renders both orientations | takes `layout`, 2D canvas draws both, 3D still refuses flat-top by name (web#763) — same words as the game | flat-top 3D is its own slice; the builder must not get ahead of the game |
| 2026-08-23 | rpg-dnd5e-web#781 | Save & Play = `PutDungeon` → `StartEncounter{lobby_id, dungeon_key}` | `PutDungeon` → `CreateLobby` (Home-selected character) → `SetReady` → `StartEncounter{dungeon_key}` → game route | no lobby is open while authoring |
| 2026-08-23 | rpg-dnd5e-web#781 | — | `PropCompositionConcept` deleted; toolkit-contributor sandbox ported to a v2 doc; one `hexOffset.ts` bridge (odd-r pointy / odd-q flat) pinned by a pixel-formula test | rode on the deleted preview renderer; the symmetric-bug rule |
| 2026-08-23 | toolkit T1 (c46efd9) | `Atlas.Regions` added beside the per-region props/boundaries/doorways | `encounter.Atlas` is FLAT: `{Orientation, Cells, Regions, Props, Boundaries, Doorways{Door, From, To}}`; no Grid/Origin/Width/Height; one doorway per door edge keyed by door id | props, walls and doors are field-level facts once rooms are gone; session's projection becomes a straight copy |
| 2026-08-23 | toolkit T1 | `FieldInput{Canvas, Regions, Doors, Walls}` | plus `Props []PropInput`; `MemberInput.Room`, `TriggerReachedPosition.Room` gone; `EndingData.At` replaces Room+Position; `StepOutput.Crossing` and `ErrBadConnection` deleted, the moved beat carries `doors: [ids]` | props had to live somewhere; every position is absolute now |
| 2026-08-23 | toolkit T1 | hex only, orientation authored | `CanvasInput.Orientation` REQUIRED, `Grid()` always hex, square family deleted; `EncounterData.Field` refuses `rooms`/`connections` keys by name | Kirk's ruling; fail-loud load |
| 2026-08-23 | toolkit#1210 (T2) | door ids `<key>/<id>`; golden compares the atlas byte-for-byte | door ids changed from v1's `<key>:<west>-<east>`; golden compares doorways by cells; open connectors became open `DoorInput`s (the v2 tomb's entrance→hall too) | connections are gone, a doorway is a door with no state |
| 2026-08-23 | toolkit#1210 (T2) | `Validate(spec) []FieldError`, every refusal names a YAML path | `ValidationError{[]FieldError}` collecting EVERY defect, `errors.Is(ErrBadSpec)`; unknown-key defects carry `line N` as the path | yaml.v3 cannot give a path for an unknown key; the builder gets a line instead — open question (b) below |
| 2026-08-23 | toolkit#1210 (T1) | new sentinels for intensity range, duplicate wall, cell budget | `ErrNoField` for all three; `maxRoomCells`/`maxRoomSpan` gone, `maxFieldCells` bounds listed cells; Atlas is O(cells) (atlascost_test deleted) | sentinels only where a caller branches on them |
| 2026-08-23 | toolkit#1210 (ADR-0044) | — | non-void test scenes declare a transparent void | a sightline hugging the edge column of a sheared rectangle crosses void cells, so under an opaque void two members on column 0 cannot see each other (hex behaviour since #1127; regions make it easy to notice) — open question (a) below |
| 2026-08-23 | toolkit#1211 (T3) | `AtlasDoorway.Connection` | `AtlasDoorway.Door` (JSON `door`) | encounter keys doorways by door id; connections no longer exist |
| 2026-08-23 | toolkit#1211 (T3) | — | `GridSquare` deleted, `Atlas.Grid` always `hex`; ~20 external + 2 internal session fixtures, the workbench and main's new `two_players` fixture ported to regions via `regionfixtures_test.go`; square-era scenes re-geometried for hex adjacency | hex only (Kirk's ruling); the port was mechanical but wide |

## Rulings made during implementation

*Open for Kirk's walk (from toolkit#1210):*

- **(a) Edge-hugging sightlines through opaque void.** In the tomb itself, two members on the edge column of a sheared rectangle may not see each other because the straight line between them clips void cells. Accept as the hex model's truth, or declare the tomb's void transparent / pad the regions?
- **(b) Unknown-key errors carry `line N`, not a YAML path.** Good enough for the builder (it emits its own YAML and never produces unknown keys), or should the builder pre-check keys itself?

