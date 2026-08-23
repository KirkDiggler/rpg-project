# Dungeon Builder on the Session Stack — implementation ledger

*Adjustments made while building against `design.md` / `plan.md`. The design
and plan stay open on rpg-project PR #255 through implementation; this file
is where reality corrects them. One entry per adjustment: what the design
said, what was built instead, why, and where (PR / commit).*

| Date | Where (PR) | Design said | Built instead | Why |
|---|---|---|---|---|
| 2026-08-23 | rpg-api#806 branch → T3 | `PutDungeon` answers with the atlas; rpg-api projects it from the compiled world (plan A: `Entry.Atlas session.Atlas`) | T3 exports `session.AtlasOf(world *encounter.EncounterData) (*Atlas, error)` — loads with the session package's own construction capabilities, calls `enc.Atlas()`, runs the same `projectAtlas`; rpg-api calls it | The only producer of `session.Atlas` was `Manager.Atlas(session)` via unexported `projectAtlas`, and a dungeon in the registry has no session. Re-deriving the projection in rpg-api would be a second geometry path (the symmetric-bug rule); one projection, one producer. |
| 2026-08-23 | P (rpg-api-protos#238) | plan P kept `tests/declaration-remaining` for a follow-up | deleted in the same PR | Kirk: "we do not need tests in the protos, we have them mechanically compiled" |
| 2026-08-23 | rpg-api#820 | `Registry.Put(ctx, key, yaml, validateOnly)` | `Put(ctx, *PutInput) (*PutOutput, error)` | rpg-api's Input/Output struct rule |
| 2026-08-23 | rpg-api#820 | `Entry{…, Compiled dungeonspec.Compiled, Atlas}` | `Entry` carries `*sessionworld.Dungeon` | lobby needs absolute party seats, which under v1 exist only after the projection; revisit when T2's `Compiled` is absolute by construction |
| 2026-08-23 | rpg-api#820 | `RPG_CONTENT_DIR` required | defaults to `./content` when authoring is off; required when `RPG_AUTHORING_ENABLED=1` | a plain checkout / image must boot the tomb with no env |
| 2026-08-23 | rpg-api#820 | boot refuses a non-compiling file | also refuses filename ≠ `key` and a dir without `reference-tomb.yaml` | the default dungeon must exist; one name per dungeon |

## Rulings made during implementation

*(none yet — see `design.md` §7b for the open threads)*
