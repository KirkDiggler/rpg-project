# Removing v2 — the implementation plan

**Status:** PLAN 2026-09-24, for review with
[`one-room-one-shape.md`](./one-room-one-shape.md). That doc makes the ruling;
this one says what the ruling costs and in what order it is paid.

**Not started.** Nothing here is implemented, and step 0 is "the ruling lands".

## Why a plan rather than a PR

Two things in the measurement change the shape of the work, and neither was
obvious from the ruling alone:

1. **The files are not cleanly split by dialect.** `grammar.go`, `predicate.go`,
   `factions.go` and `concealments.go` have **zero** references to single-room,
   but single-room **calls into them** — `grammar.approaches`,
   `grammar.dispositions`, `grammar.endings`, `grammar.factions`,
   `grammar.scenarios`, `grammar.predicate`, plus `predicateOf`, `factionsOf`,
   `ConcealmentSpec`, `concealmentsShape`. **They are shared, not v2's.**
   Deleting "the v2 files" by name would delete the v4 path's own validators.
2. **A clean deletion is possible anyway**, because the test split *is* clean —
   verified: **16 v2-only test files, 8 single-room-only, zero mixed.**

So the plan's first job is telling the two kinds of file apart, and its second
is deleting only one kind.

## The measured inventory

### Production files (11 non-`single_room_*` files in `dungeonspec/`)

| File | Verdict | Why |
|---|---|---|
| `spec.go` | **SPLIT** | Holds `PlaceSpec` (v2, dies) *and* every shared authored type (`AnswerSpec`, `WhenSpec`, `CheckSpec`, `IntelSpec`, `FactionSpec`, `DispositionSpec`, `ScenarioSpec`, `ExitSpec`, `EndingSpec`, `ConcealmentSpec`, `PredicateSpec`, `SelectorSpec`, `TemperSpec`) |
| `compile.go` | **SPLIT** | `Load`'s version dispatch stays; `Compile(*Spec)` and the v2-only helpers (`ordersOf` fed by `place[]`, `monstersOf`) go |
| `validate.go` | **SPLIT** | Holds the ten crossing refusals (v2-only) *and* shared validators the single-room path calls |
| `decode.go` | **SPLIT** | v2's decoding; shares helpers with `single_room_decode.go` |
| `grammar.go` | **KEEP** | Shared — single-room calls seven of its members |
| `predicate.go` | **KEEP** | Shared — `predicateOf` serves both |
| `factions.go` | **KEEP** | Shared — `factionsOf` serves both |
| `concealments.go` | **KEEP** | Shared — `concealmentsShape`, `ConcealmentSpec` |
| `geometry.go` | **KEEP** | Shared — v2 region/geometry helpers still reached from the v4 path |
| `walls.go` | **KEEP** | Shared — walls are authored in both |
| `unknown_key.go` | **SPLIT** | The unknown-key law is shared; v2's key *lists* are not |

**The honest headline: this is not "delete eleven files."** It is "delete the
v2 halves of four files and the type `PlaceSpec`, plus the key lists that admit
it." The four KEEP files are the v4 path's own machinery and must not move.

### Test files — a clean split

**v2-only (16, all die):** `compile_test.go`, `dialect_test.go`,
`facing_offset_test.go`, `factions_test.go`, `heirloom_test.go`,
`intimidate_test.go`, `lines_test.go`, `refparts_test.go`, `scale_test.go`,
`scenery_test.go`, `startfacing_test.go`, `table_test.go`, `tombsource_test.go`,
`tomb_test.go`, `unknown_key_test.go`, `world_asset_scenery_test.go`

**Single-room-only (8, all stay):** `prop_bindings_test.go`,
`single_room_compile_test.go`, `single_room_decode_test.go`,
`single_room_doors_test.go`, `single_room_lowering_test.go`,
`single_room_placement_test.go`, `single_room_site_test.go`,
`v4_gameplay_test.go`

**Unclassified — 8 files**, which reference neither marker directly and so were
caught by neither list: `arrivals_test.go`, `capabilities_test.go`,
`contentgolden_test.go`, `geometry_internal_test.go`, `golden_test.go`,
`v2_concealments_test.go`, `v4_concealments_test.go`, `v4_run_test.go`.

Named in advance because **two of them invert the obvious reading**:

- `contentgolden_test.go` reads the *shipped v2 content*, so it dies with v2.
- `golden_test.go` pins `testdata/reference-tomb.atlas.json` — **the v2 tomb's
  compiled atlas** — and calls itself "THE FORCING CASE" in its own header. It is
  the single strongest piece of evidence for Step 1: the v4 tomb port should be
  held to **the same 224-cell atlas** the v2 tomb produces, which turns "port the
  tomb" from a judgement call into a diff. `golden_test.go` is therefore
  load-bearing for the port, not cleanup — it is the last thing deleted, not the
  first.

### Rooms — pruned to exactly one, and it is specified

Kirk, 2026-09-24, correcting this plan's first draft:

> "The testator at dungeons are fine. It is the eight that's in the RPG API. We
> only need the reference tomb that has the merchant in the front room, the two
> monsters in the second and the captain and the third that is sufficient for
> now."

**Measured against that:**

| Tree | v2 | v3/v4 | Disposition |
|---|---|---|---|
| rpg-toolkit `dungeonspec/testdata/` | 5 | 8 | **13 → 1** (the tomb) |
| rpg-api `content/` (shipped) | 5 | 0 | **→ the v4 tomb** |
| rpg-api `internal/dungeons/testdata/` | 0 | 3 | **→ 0** |

**The correction that matters:** rpg-api's `internal/dungeons/testdata/` holds
**zero v2 files** — all three are v3/v4 (`placed-table-room` v4,
`workshop-one-seat` v3, `workshop-room` v3). So they are *not* swept away by
removing v2; they are **deleted by the pruning rule** ("we only need the
reference tomb"), which is a separate decision from the dialect removal and
needs to be stated as one. An earlier draft of this plan conflated the two.

### The one tomb, specified

The surviving room is **the reference tomb as v4**, with three parts:

1. **The merchant in the front room.**
2. **Two monsters in the second.**
3. **The captain in the third.**

That is the whole authored corpus for now, and it is enough: it exercises a
non-combatant, a group, and a leader — a faction, an intel-bearing NPC, and a
boss-shaped creature — which is what the slices in flight need to be walked
against.

**The per-file disposition of the other 20 is step 3's work**; this plan
deliberately does not enumerate it, for `one-room-one-shape.md`'s reason — an
inventory buries the ruling.

### Consumers

**rpg-api's production path is v4-only.** `sessionworld.Compile` calls
`tkdungeonspec.Load`, which routes `version >= 3` to `DecodeSingleRoom` /
`CompileSingleRoom`. The API's imports from the package are **six symbols** —
`Compiled`, `Load`, `MonsterPlacement`, `ErrBadSpec`, `FieldError`,
`ValidationError` — and **all six stay.** `MonsterPlacement` is shared despite
being declared in `compile.go` beside the v2 types: the single-room compile
produces it (`single_room_compile.go`) and the API consumes it. It is a
`SPLIT`-file survivor, not a v2 type — the file it lives in is not the dialect
it belongs to.

The API's *tests* reference the five v2 content files (13 in `convert_test.go`,
9 each in `seed_test.go`/`registry_test.go`, 6 in a lobby stack test, and
several `sessionworld` tests). Those are the churn: each needs either a v4
replacement or deletion.

## The plan, in order

### Step 0 — the ruling lands

`one-room-one-shape.md` is merged. Nothing below starts before it. No branch, no
code.

### Step 1 — v4 reference tomb

**One toolkit PR.** Port `reference-tomb.yaml` to the v4 dialect and check it into
`dungeonspec/testdata/`. Prove it compiles through `Load` and covers what the v2
tomb covered: regions, walls, doors, props with facing/offset, monsters with
factions, concealments, intel, exits, endings, scenarios.

**Why first:** everything else deletes against it. Deleting v2 tests before a v4
tomb carries the same coverage would remove the engine's regression net and call
it cleanup.

**Done when:** the v4 tomb compiles, and every behaviour the 16 v2 test files
assert has either a v4 test or a written reason it no longer needs one.

### Step 2 — the ten crossing refusals and `PlaceSpec`

**One toolkit PR.** Remove `PlaceSpec`, the ten `validate.go` crossings, the v2
key lists, and `Compile(*Spec)` / the v2 half of `Load`. `Load` keeps its
dispatch and refuses `version: 2` **by name with where it went** —
`laterWords`' courtesy, not a bare unknown-version error.

**Rides with:** the `intimidate`/`persuade` removal
(`one-room-one-shape.md` Ruling 1), since both touch the same binding and the
same goldens. They are one edit to one shape.

**Done when:** the module builds with `PlaceSpec` gone, `version: 2` is refused
by a sentence naming v4, and no KEEP file was touched.

### Step 3 — prune the rooms and the tests

**One toolkit PR.** Delete the 16 v2-only test files and their fixtures; resolve
the unclassified seven; prune `testdata/` to the v4 tomb.

**Done when:** `dungeonspec/testdata/` holds the tomb and nothing that exists
only to give a deleted test a file.

### Step 4 — rpg-api content and tests

**One rpg-api PR.** Delete the five v2 content files; repoint or delete the
tests that read them; make the seeded content the v4 tomb so a fresh box seeds
something that compiles.

**Depends on Step 1's tomb existing as v4** and on a toolkit tag carrying Steps
1–3. This is the one place a pin genuinely gates work, so it is sequenced last.

### Step 5 — confirm the builder

The builder already emits v4, so this should need nothing. **Verify** rather than
assume: the World Builder's own tests and its publish path should be green
against a stack whose content is v4-only.

## What could go wrong, named in advance

1. **Deleting a shared file because its name looks v2.** The measurement above
   is the guard; `grammar.go` and its three neighbours are the trap. A reviewer
   should check the KEEP column rather than trusting the file list.
2. **Losing coverage silently.** Step 1 exists for this. A deleted test whose
   behaviour nothing else asserts is a regression net removed, not cleanup.
3. **`version: 2` becoming an unknown-version error.** It must speak — an author
   with an old file is exactly who needs the sentence.
4. **The unclassified seven.** They were not caught by either marker; each needs
   a human call rather than a grep.

## What this plan does not do

- Does not migrate any v2 document. Kirk: *"we do not need to migrate anything."*
- Does not decide the room-by-room pruning in advance (Step 3's work).
- Does not touch `place[].facing` on monsters (rpg-toolkit#1892) or the named
  table's key names — those land after this, on the shape that stays.

— rpg-toolkit agent, on behalf of KirkDiggler