# Dungeon Visual Fidelity — why the live game doesn't look like the reference

**Status:** ⚠️ **LARGELY SUPERSEDED — read this section first**
**Date:** 2026-07-24
**Goal:** Make the live encounter route (`EncounterView`/`EncounterMap`) render the
crypt the way the playtest harness already does.

## ⚠️ CORRECTION (2026-07-24, same day, later)

**Most of the "root causes" below were already fixed and merged. The real problem
was that every local checkout was behind `origin/main`.** The diagnosis was
performed against stale source and reached confident, wrong conclusions.

Actual staleness found:

| Repo | Behind `origin/main` | Contained |
|---|---|---|
| `rpg-dnd5e-web` | 4 commits | #585 consume server theme on real route, #587 crypt brightness + live dial, #588 brazier/torch light-anchor glow |
| `rpg-api` | 8 commits | #702 project revealed static obstacles, #703 project locked door state, #705 perimeter edge walls, #708 consume toolkit crypt dressing |
| `rpg-toolkit` | 6 commits | — |
| `rpg-project` | behind | the merged `ideas/dungeon-authoring/` design (PR #117) |
| `node_modules` protos | stale vs lockfile | `Space.theme`, `WALL_KIND_DOOR_LOCKED` |

Corrections to the causes below:

- **Cause 1 (EncounterMap drops theme/lighting props) — FIXED.** `EncounterMap.tsx`
  on main passes `spaceTheme`, `ambientIntensity`, `directionalIntensity`, and
  `moodPointLights` (`:333-354`). Merged in web #585/#587/#588.
- **Cause 2 (`obstacle_ref` never written) — FIXED.** `project.go` on main builds
  `obstacleEntity()` with `ObstacleRef: obstacleRefFor(obstacle.Ref)`. Merged in
  api #702.
- **The "contract trap" warning was WRONG.** `obstacleRefFor` already splits
  `"dnd5e:props:pillar"` into `{Module:"dnd5e", Type:"props", Id:"pillar"}` — the
  bare tail, exactly as the client wants, with tests asserting it. There is no
  double-prefix hazard. Disregard that warning.
- **Cause 4 (walls read as "rubble") — FIXED by the same work.**
  `SyntyHexWall.tsx:308` selects the crypt variant weights when
  `spaceTheme === 'crypt'`, so every wall in a themed space uses the plain-heavy
  10:2:1 mix. The weighting analysis below is still accurate as *background*, but
  it is no longer an open defect.
- **The locked-door prompt soft-lock** needs re-verification against api #703
  (project locked door state), which may already address it.

**After syncing all repos and rebuilding, local rendering matches deployed.**

**The durable lesson is the inverse of the original premise.** The team believed
they were "deploying to see miss after miss." In fact *deployed was ahead of
local* — the local workspace was the stale one, across four repos plus
`node_modules`. Before diagnosing any visual gap, **verify every checkout is
current**:

```bash
for r in rpg-toolkit rpg-api rpg-dnd5e-web rpg-project rpg-api-protos; do
  (cd "$r" && git fetch -q origin \
    && echo "$r behind by $(git rev-list --count HEAD..origin/main)")
done
```

What genuinely remains open is documented in "Still open after the correction"
near the end. Everything between here and there is preserved as the original
(largely superseded) analysis.

## The short version

The reference screenshot everyone has been chasing was captured from the
**playtest harness**, not the game. The two routes render the *same*
`<HexGrid>`, but the playtest route feeds it five extra props that the live
route does not. Nothing about the look is unachievable — it was built, and it
shipped into a route the game doesn't use.

On top of that, a **stale installed proto package** was making the local dev
client unable to decode fields the server was already sending. That one is
fixed (see "Already fixed"), and fixing it restored all wall/floor/character
textures on the live route.

## Evidence

Two screenshots, same codebase:

- **Reference** — header reads `Playtest demo as alice / connection: disconnected`.
  That is `PlaytestMap.tsx`, reached via `?encounterId=<id>&playerId=alice&cryptdemo=1`.
  Dark, green-lit floor, warm rim light on walls, banners/urns/rune stone, deep
  black falloff.
- **Actual** — header reads `mode: FREE_ROAM / connected`. That is
  `EncounterMap.tsx`, the real game route. Flat uniform grey, no colored light,
  sparse props, purple capsules where props should be.

## Root causes

### 1. `EncounterMap` drops the theme/lighting props (client)

`PlaytestMap.tsx:279-293` and `EncounterMap.tsx:220-244` build a `<HexGrid>` with
identical arguments **except** PlaytestMap adds five lines:

```tsx
themeWallHexKeys={cryptLayout?.themeWallHexKeys}
themeFloorHexKeys={cryptThemeFloorHexKeys}
ambientIntensity={cryptLayout ? 0.08 : undefined}
directionalIntensity={cryptLayout ? 0.05 : undefined}
moodPointLights={cryptMoodLights}
```

`HexGrid`'s own doc comment (`HexGrid.tsx:136-156`) states that undefined callers
keep "the original 0.6/0.8 defaults". So the live route renders at
`ambientLight 0.6` + `directionalLight 0.8` with zero point lights — a flat,
evenly-lit grey room. That is exactly what the actual screenshot shows.

Confirmed by grep: **no non-playtest code** calls `buildCryptLayout`,
`buildCryptMoodLights`, or `buildCryptDoorLights`.

**Important coupling:** `buildCryptMoodLights()` derives point lights *from prop
positions* (braziers, candles, torches). No decor props → no light sources → flat
lighting. Cause 3 below is therefore partly a lighting fix too.

### 2. `obstacle_ref` / `prop_ref` are never written by the server

The client is fully built for this and has been waiting:

- Proto has the fields: `obstacleRef` (`types_pb.ts:652`), `propRef` (`:716`)
- Client reads them: `EncounterView.tsx:309-310` → `entity.data.value.obstacleRef?.id`
- Resolver exists: `obstaclePropKeys.ts` → `propManifest.ts` (29 keys)
- Assets exist: **all 42 GLBs** referenced by the manifest are on disk under
  `public/models/synty/props/`, textures **embedded** in each GLB
  (`SyntyAtlas` material, `metallicFactor: 0`) — nothing 404s
- All 6 refs the server currently generates resolve cleanly against the manifest

But `git grep` for `obstacle_ref|ObstacleRef|prop_ref|PropRef` across the **entire
rpg-api repo** returns **zero hits**. `ProjectFor`
(`internal/handlers/dnd5e/v2/encounter/project.go:153-187`) builds its entity list
from **players (154-169) and monsters (181-187) only**. Obstacles live in
`data.Space.Obstacles` and are simply never projected.

Result: `propRefId` is always undefined → `resolvePropVariantForEntity` returns
nothing → `HexEntity.tsx:487` falls through to a capsule painted `#805ad5`
(`HexEntity.tsx:97`). **The purple capsules in-game are the obstacles.**

> **Contract trap when fixing this.** `obstaclePropKeys.ts:54` composes the key
> as `` `dnd5e:props:${refId}` `` — it expects a **bare tail**. Redis stores the
> **full** key (`"ref":"dnd5e:props:pillar"`). If rpg-api forwards the stored value
> verbatim, the client builds `dnd5e:props:dnd5e:props:pillar` and the props stay
> purple. **The wire must carry `"pillar"`.**

Same shape of bug for monster identity: the monster record carries
`monster_ref: "dnd5e:monsters:skeleton-captain"` and a base64 `data_json`
containing `"name":"Skeleton Captain"`, but the UI displays the raw generated id
(`entrance-entrance`, `boss-boss` — the `<region>-<region>` pattern).

### 3. The generator emits no decor (toolkit)

A live encounter's `space.obstacles` had **8 entries** in a 27×8 room across 3
regions. Every one is `blocks_movement: true` — pure gameplay collision. Only 6
distinct manifest keys are ever used: `obelisk`, `pillar`, `coffin`, `altar`,
`statue-reaper`, `statue-knight-hooded`.

**23 of the 29 manifest keys are never sent**, including every `decor` key:
`banner`, `books`, `candles`, `brazier`, `torch-ornate`, `vase`, `bone-pile`,
`chain`, `skeleton-remains`, `armor-stand` — plus `cover` props `barrel`, `crate`,
`chest`, `tomb`, `rocks`, `log-spike`, `skeleton-table`.

The room is bare because the generator only places collision geometry.

### 4. Walls read as "rubble", not masonry

`syntyHexWallHelpers.ts:125-147` defines three wall GLBs with weights
`plain:broken:alcove = 3:1:1` — the code's own comment calls this **"the rubble
look"**. The `'crypt'` theme reweights them to **10:2:1**, because "a crypt reads
as intact worked masonry, not a ruin" (`:161-171`).

But `WALL_VARIANTS_BY_THEME` is selected by the `themeWallHexKeys` prop, which
cause 1 above establishes the live route never passes. Line 157 says it outright:
default weights apply to "**every real dungeon wall today**".

So ~40% of live-route walls are broken/alcove pieces, versus ~23% under the crypt
theme (and plain goes from 60% → 77%). **Fixing cause 1 fixes much of the wall
jankiness for free** — no new assets, just the theme reaching the selector.

Residual jank after that is structural, not weighting: walls follow hex edges, so
organic generated room shapes stair-step. That's the "author the room layout"
problem below, not a variant-selection problem.

### 5. Camera framing

The reference is a pulled-back, near-orthographic diorama of the whole room. The
live route sits much closer and lower. Smallest of the four, but it's part of why
the two shots read so differently.

## Already fixed (2026-07-24)

**Stale installed proto package in `rpg-dnd5e-web`.** `package.json` pinned
`v0.1.113` and `package-lock.json` resolved to commit `0e4e2d77d73a` — both
correct, and identical to what rpg-api consumes
(`v0.0.0-20260723025004-0e4e2d77d73a`). But the *installed* `node_modules` tree
was an older build containing **zero** occurrences of `DOOR_LOCKED` or `theme`,
while the generated TS at that exact commit on GitHub contains both.

Consequences while stale — the local client could not decode:
- `Space.theme = "crypt"` (the server has been sending it)
- `WALL_KIND_DOOR_LOCKED` — so the locked boss door never resolved as locked,
  which is why clicking it gave no feedback

Fix (per `rpg-dnd5e-web/CLAUDE.md`, "Proto Updates Require Lock File Regeneration"):

```bash
cd rpg-dnd5e-web && rm -rf node_modules package-lock.json && npm install
```

Then restart vite. `npm install --force` alone is **not** enough — npm's git cache
serves the stale tarball. After the fix, `Space.theme: string` (field 5) and
`DOOR_LOCKED` are present, and the live route renders textured walls, textured
floor, and a textured character — all of which had been flat/untextured.

**This made local behave differently from deployed** (CI does a clean install and
got the correct protos), which is a strong candidate for why iteration kept
producing surprises. Worth a periodic check.

## Not a bug — checked and cleared

**Critical hits.** A `19+5 vs AC 13` that dealt 24 damage is correct:

- The explosion icon is the generic damage marker (`CombatLog.tsx:185`), **not** a
  crit marker. Crit prints the literal word `CRIT` on the attack line (`:161`).
- Crit-doubled damage components get a `‡` suffix (`:180`). The observed
  breakdown had none.
- `7 + 3 + 2 + 0 = 12`, doubled to 24 by **vulnerability**, which `damage.go:388`
  applies as a `2.0` multiplier (hence it displays as `0` — a multiplier, not an
  addend). Skeleton + bludgeoning greatclub is textbook 5e.
- `attack_phases.go:324` derives `attackRoll` from the raw d20 and `:450` compares
  that natural roll to threshold 20. A 19 cannot crit.

(Pathfinder uses a flat vulnerability bonus rather than doubling — easy to conflate.)

**The locked door being locked.** `crypt-door-corridor-boss` is
`locked: true, lock_dc: 12, lock_ability: "dex"` — correct, by design, from
rpg-toolkit #815/#824. Server-side projection work is already planned in
`ideas/locked-door-terminal-projection/plan.md`.

But interacting with it surfaced a separate, serious bug — see below.

## BUG: unresolved door prompts permanently soft-lock the player

Observed live: `Door interaction error: [failed_precondition] resolve the pending
prompt before issuing another action`, with every subsequent action refused.

Confirmed in Redis — the encounter carries an unresolved prompt forever:

```json
"pending_prompts": {"test-player": {"kind":1, "dc":12, "ability":"dex",
                    "triggered_by":"crypt-door-corridor-boss",
                    "triggered_action":"open"}}
```

**Why it's unrecoverable.** A door skill-check prompt is delivered **exactly
once**, as `InteractResponse.input_required` (`rpg-api`
`internal/handlers/dnd5e/v2/encounter/interact.go:60-64`). The toolkit is explicit
that it is *not* broadcast: "Does not publish any broker event — prompts are
persisted state, not transient broadcasts. The orchestrator picks them up by
reading `Data.PendingPrompts`" (`rpg-toolkit/encounter/prompts.go:133-136`).

`ProjectFor` never reads `Data.PendingPrompts` — `grep` for
`PendingPrompt|pending_prompt` across the encounter handler package hits
`interact.go`, `submit_check.go`, and the *reaction* path
(`InputRequiredDeliveredEvent`), but **not `project.go`**.

So if the client drops that one response — modal closed, page refreshed, stream
reconnected, render failure — the prompt survives in persisted state, the client
can never rediscover it, and `AttemptUnlock` rejects everything from then on
(`prompts.go:148-150`). There is no cancel, no timeout, and no re-delivery.

**Fix:** project `Data.PendingPrompts[viewer]` into the connect-time snapshot in
`ProjectFor`, so a reconnecting client re-discovers its own pending prompt. The
toolkit doc already assumes the orchestrator does this ("*and from
Data.PendingPrompts on subsequent loads*", `prompts.go:122`). Worth also
considering a cancel/expiry path so a stuck prompt is never terminal.

Same family as causes 1-3: **the server holds the state and never puts it on the
wire.**

**Unwedging a stuck encounter locally:**

```bash
docker exec rpg-redis-dev redis-cli GET "enc:v2:<id>"   # confirm pending_prompts
# then either answer the check via SubmitCheck, or clear the encounter and restart
```

## Authoring room layout

Open request from Kirk (2026-07-24): *"it would be great if our dungeon data
allowed us to layout the room."*

Relevant existing contract — more of this exists than you'd expect:

- `Space.theme` — dungeon-wide visual family, opaque string, no fixed vocabulary
- `Zone.archetype` — per-zone room function from the toolkit's **fixed** vocabulary
  (`"entrance" | "chamber" | "corridor" | "boss"`, rpg-toolkit#814). A live
  encounter already carries 3 zones tagged this way.
- The proto comment on `Zone.archetype` explicitly reserves the next step:
  *"Future ambient hooks (lighting profile ref, music cue ref, trigger refs) will
  be added with new field numbers as needed."*

So the seam for per-room dressing/lighting is designed and partly built. What's
missing is (a) generators authoring meaningful room *shapes* rather than organic
blobs, and (b) the client consuming `archetype` at all. Worth deciding whether
"lay out the room" means authored templates in the toolkit generator, or a
data-driven room description the client dresses — before cause 2's obstacle
projection hardens the entity shape.

## The local iteration loop

This is the point of the whole exercise — stop deploying to find visual misses.

**Backend in containers, web on the host.** Do **not** use
`docker-compose.local-src.yml` for visual work: it containerizes the web app with
a read-only mount and no `node_modules`, which kills Vite HMR and puts a Docker
rebuild in the middle of every lighting tweak.

```bash
cd rpg-deployment

# backend (prebuilt images: redis, mongo, 5e-srd-api, envoy, nginx)
docker compose -f docker-compose.local-dev.yml up -d

# rpg-api from YOUR working tree (~23s build)
docker build -t rpg-api:local ../rpg-api
docker compose -f docker-compose.local-dev.yml \
               -f docker-compose.local-api-src.yml up -d rpg-api

# web, on the host
cd ../rpg-dnd5e-web && npm run dev     # :3001
```

Ports: nginx `:80`, envoy `:8080`, dnd-api `:3002`, redis `:6380`, vite `:3001`.
`vite.config.ts` already proxies `/connect` → `localhost:8080`.

**Uncommitted, load-bearing** (in `rpg-deployment`, needs committing or it will be lost):
- `docker-compose.local-dev.yml` — `AUTH_DEV_MODE=true` on rpg-api; envoy port 8080 published
- `envoy/envoy.yaml` — `authorization` added to CORS `allow_headers`
- `docker-compose.local-api-src.yml` — **new**, the local-build overlay

**Toolkit changes are NOT in this loop.** `rpg-api/go.mod` pins published
`rpg-toolkit` versions with no replace directives (deliberate, per
`rpg-toolkit/CLAUDE.md`). Toolkit work still requires publish → `go get`.

### Inspecting live server state

The encounter blob is readable straight out of Redis — far faster than adding logging:

```bash
docker exec rpg-redis-dev redis-cli --scan --count 200 | grep enc:
docker exec rpg-redis-dev redis-cli GET "enc:v2:<id>"
```

Top-level keys: `id, sequence, players, doors, monsters, mode, reaction_readiness, space`.
`space` carries `walls, width, height, entrance, regions, theme, obstacles`.

### Screenshot gotcha

`tools/browser/screenshot.mjs` uses `waitUntil: 'networkidle'`, which **always
times out on the live game route** — `StreamEncounter` holds an open server
stream, so the network never goes idle. Use `waitUntil: 'domcontentloaded'` plus a
fixed settle delay (~12-15s for GLB loading) instead.

Playwright's Chromium was missing from `~/.cache/ms-playwright` and had to be
reinstalled (`node node_modules/playwright/cli.js install chromium`).

## Useful dev flags

| Flag | Route | Effect |
|---|---|---|
| `?encounterId=<id>` | any | dev-only gate into `PlaytestHarness` (`App.tsx:30-34`) |
| `?cryptdemo=1` | playtest | builds the crypt layout (`PlaytestMap.tsx:160`) — **this is the reference look** |
| `?playerId=alice\|bob` | any | dev player override, two tabs as two players |
| `?devPropDemoKeys=barrel,pillar` | **live** | injects synthetic obstacle entities with prop refs next to the player — proves the render path without server changes (`EncounterMap.tsx:120-138`) |

## Proposed order of work

1. **Client theme/lighting** (`rpg-dnd5e-web`) — pass the five props in
   `EncounterMap`, driven by `space.theme` (which now decodes). Biggest visual
   delta, no server dependency, and `PlaytestMap.tsx:289-293` is a working
   reference implementation.
2. **Server obstacle projection** (`rpg-api`) — append obstacle entities in
   `ProjectFor` carrying `obstacle_ref` as a **bare tail id**. Turns purple
   capsules into real models. Testable locally via the 23s build loop. Consider
   projecting monster display name in the same pass.
3. **Toolkit decor generation** (`rpg-toolkit`) — place `decor`/`cover` props, not
   just collision. Also supplies the light sources that `buildCryptMoodLights`
   needs, so it feeds back into (1).
4. **Camera framing** — match the reference's pulled-back diorama framing.

## Open questions

- **Is the reference screenshot the spec?** The whole plan assumes yes. If that
  image was a happy accident rather than the intended art direction, pin the real
  target before matching numbers off it.
- **Should `space.theme` drive lighting directly, or should the server send
  explicit light sources?** Today lighting is derived client-side from prop
  positions. That works but couples "how the room is dressed" to "how it's lit."
- **Decor as entities or as a separate wire concept?** Decor doesn't block
  movement or LOS, so projecting it through the same LOS-gated entity list as
  monsters may be wrong — worth deciding before (2) hardens the shape.

## Still open after the correction

Verified against synced `origin/main` on all repos, 2026-07-24:

1. **Decor density.** The generator places obstacles as collision geometry;
   api #708 ("consume toolkit crypt dressing") may have widened this. Re-count
   distinct prop refs in a *freshly generated* encounter before assuming the
   23-of-29-unused figure below still holds — that count was taken against stale
   code.
2. **Camera framing.** Untouched by any of the merged work. The live route still
   sits closer and lower than the reference's pulled-back diorama.
3. **Locked-door prompt recovery.** `ProjectFor` re-surfacing
   `Data.PendingPrompts[viewer]` on reconnect — re-verify against api #703 first.
4. **Authored room layout** — the live thread. See below; this is where
   `ideas/dungeon-authoring/` takes over.

## Relationship to `ideas/dungeon-authoring/`

That design (PR #117, **merged**; implementation plan PR #121, open) is the
answer to "let us lay out the room." Kirk's framing, 2026-07-24: *"when we can
make something we control look right, we can look to make pieces of it
procedural."* — author first, proceduralize what's proven.

It collapses a dungeon definition (today scattered across `CryptDungeonParams` in
the toolkit, a hardcoded monster table in rpg-api, and a key resolver) into one
YAML file, with the schema and compiler in the toolkit and content files in
rpg-api. Its acceptance case is authoring a 4-room crypt without writing Go.

Two notes connecting it to this document:

- It asserts **"protos / rpg-dnd5e-web: no changes"** because the wire already
  carries zones/archetypes, theme, walls, obstacles and monsters. Given api #702
  and web #585 are merged, that now holds — it did not when this doc was first
  written against stale checkouts.
- Its v1 explicitly defers hand-drawn room layouts (`rooms[].layout` is a reserved
  seat). So "the walls stair-step because generated rooms are organic blobs"
  remains true after v1 — v1 fixes *what is in* each room, not its silhouette.

## Anchors

| What | Where |
|---|---|
| Reference implementation of the look | `rpg-dnd5e-web/src/components/playtest/PlaytestMap.tsx:279-293` |
| The live route that's missing it | `rpg-dnd5e-web/src/components/game/EncounterMap.tsx:220-244` |
| Lighting defaults + prop docs | `rpg-dnd5e-web/src/components/hex-grid/HexGrid.tsx:136-156` |
| Purple fallback capsule | `rpg-dnd5e-web/src/components/hex-grid/HexEntity.tsx:97,487` |
| Bare-tail key composition | `rpg-dnd5e-web/src/components/hex-grid/obstaclePropKeys.ts:50-55` |
| Prop manifest (29 keys) | `rpg-dnd5e-web/src/components/hex-grid/propManifest.ts` |
| Entity projection (no obstacles) | `rpg-api/internal/handlers/dnd5e/v2/encounter/project.go:153-200` |
| Vulnerability multiplier | `rpg-toolkit/rulebooks/dnd5e/combat/damage.go:386-412` |
| Crit determination | `rpg-toolkit/rulebooks/dnd5e/combat/attack_phases.go:324,450` |
| Related, already planned | `rpg-project/ideas/locked-door-terminal-projection/plan.md` |
