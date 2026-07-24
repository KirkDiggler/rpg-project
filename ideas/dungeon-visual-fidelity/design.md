# Dungeon Visual Fidelity — why the live game doesn't look like the reference

**Status:** diagnosis complete, implementation not started
**Date:** 2026-07-24
**Goal:** Make the live encounter route (`EncounterView`/`EncounterMap`) render the
crypt the way the playtest harness already does.

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

### 4. Camera framing

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

**The locked door itself.** `crypt-door-corridor-boss` is
`locked: true, lock_dc: 12, lock_ability: "dex"` — correct, by design, from
rpg-toolkit #815/#824. The missing feedback was the stale-proto issue above.
Server-side projection work is already planned in
`ideas/locked-door-terminal-projection/plan.md`.

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
