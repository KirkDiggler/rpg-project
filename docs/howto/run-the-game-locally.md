# Run the game locally

The fast loop is **backend in containers, web on the host**. Vite serves the
client with HMR; everything else runs from `rpg-deployment`'s compose files.

> Do **not** use `docker-compose.local-src.yml` for visual work. It
> containerizes the web app too — read-only mount, no `node_modules` — which
> kills Vite HMR and puts a Docker rebuild in the middle of every UI tweak.

## Start everything (default: pull the `dev` image)

The default flow **pulls** `rpg-api`'s image — there is no build step. That's
what makes "what am I playing?" answerable: the answer is "whatever's on the
`dev` branch," not an archaeology exercise through local Docker images and
half-remembered rebuilds.

```bash
cd rpg-deployment

# redis, mongo, 5e-srd-api, envoy, nginx, rpg-api — all prebuilt images
docker compose -f docker-compose.local-dev.yml up -d

# client on the host
cd ../rpg-dnd5e-web && npm run dev        # :3001
```

`rpg-api`'s image tag defaults to `dev`, published by rpg-api's `docker.yml`
workflow on every push to `origin/dev`. Override `RPG_API_IMAGE_TAG` to pin
something else — `latest` (what production runs) or a specific `sha`:

```bash
RPG_API_IMAGE_TAG=latest docker compose -f docker-compose.local-dev.yml up -d
```

`dev` is a mutable tag — Compose does not notice a new remote push on its
own. If you started the stack a while ago and want whatever's newly on
`dev`, pull before recreating:

```bash
docker compose -f docker-compose.local-dev.yml pull rpg-api
docker compose -f docker-compose.local-dev.yml up -d rpg-api
```

| Service | Port | Notes |
| --- | --- | --- |
| vite (host) | 3001 | `vite.config.ts` proxies `/connect` → `localhost:8080` |
| nginx | 80 | |
| envoy | 8080 | gRPC-web bridge |
| 5e-srd-api | 3002 | |
| redis | 6380 | host port; container listens on 6379 |

## Iterating on rpg-api from local source (opt-in)

Building `rpg-api` from your local working tree is **not** the default —
reach for it only when you're actively changing rpg-api source and want the
fast edit-and-look loop. `docker-compose.local-api-src.yml` is an **overlay**
that swaps just the `rpg-api` service for one built from `../rpg-api`;
everything else stays on the prebuilt images from `local-dev.yml`.

```bash
cd rpg-deployment

docker build -t rpg-api:local ../rpg-api          # ~23s
docker compose -f docker-compose.local-dev.yml \
               -f docker-compose.local-api-src.yml up -d rpg-api
```

Always pass both `-f` flags together, in that order, or compose will not
know about the other services.

## Restart the API

**Running the default pulled image:**

```bash
cd rpg-deployment
docker compose -f docker-compose.local-dev.yml pull rpg-api
docker compose -f docker-compose.local-dev.yml up -d rpg-api
```

**Iterating on local source (`local-api-src.yml` overlay active):**

```bash
cd rpg-deployment

# restart in place — no code change
docker compose -f docker-compose.local-dev.yml \
               -f docker-compose.local-api-src.yml restart rpg-api

# pick up Go changes — rebuild the image, then recreate the container
docker build -t rpg-api:local ../rpg-api
docker compose -f docker-compose.local-dev.yml \
               -f docker-compose.local-api-src.yml up -d rpg-api
```

`restart` reuses the existing image, so it will **not** pick up code changes.
Rebuild first, then `up -d` — that recreates the container against the new
image. Leave redis/mongo/5e-srd-api/envoy/nginx alone; they run prebuilt images
and rarely need touching.

## Toolkit local override

**Toolkit changes are not in the loops above.** `rpg-api/go.mod` normally pins
published `rpg-toolkit` versions with no `replace` directives, so toolkit work
needs publish → tag → `go get` before the API can see it — impractical if
you're mid-way through several edit-and-look cycles on an unpublished change.

For that, rpg-api has a local override loop: `scripts/toolkit-local-override.sh
{on|off|status}` plus `Dockerfile.local-toolkit` (rpg-api#741). Read the
script's own header comment (`rpg-api/scripts/toolkit-local-override.sh`) and
`rpg-api/docs/how-to/local-toolkit-override.md` for the mechanics — it
explains itself well, so this is just the two rules that matter beyond that:

- **One module at a time.** The script hardcodes a single `MODULE`
  (`github.com/KirkDiggler/rpg-toolkit/encounter`) and validates that the
  source `go.mod` actually declares it. Sibling toolkit modules stay at
  whatever published versions they're already pinned to — this is
  deliberately not a "sync the whole toolkit" tool. Same principle as
  CLAUDE.md's [How a wave is shaped](../../CLAUDE.md#how-a-wave-is-shaped):
  one branch per wave, one version of a module at a time, just at a smaller
  scale.
- **Local loop only — must never reach a merged branch or CI.** The
  `replace` it adds points at a directory (`local-toolkit/`) that exists only
  on your machine. The exit path is always: publish the toolkit change → tag
  the version → `scripts/toolkit-local-override.sh off` → bump rpg-api's pin
  to the real tag. Check `git diff go.mod` before committing or opening a
  PR — a `replace ... => ./local-toolkit/encounter` line means the override
  is still on.

## Parallel lab api

The primary `rpg-api` container above is a **shared, one-at-a-time**
resource: its `RPG_DUNGEON_KEY`/`RPG_CONTENT_DIR` get toggled by whoever is
currently authoring or verifying a dungeon, and two teams working the local
stack at once will collide (one team's restart or env swap silently changes
what another team's client is looking at).

Use a parallel lab instance instead of touching the primary whenever you're
running an experiment, authoring/iterating on dungeon content, or verifying
something that isn't "the thing everyone else is currently testing." **Rule:
the primary stays on `reference-tomb` for everyone; labs and experiments run
on the parallel instances.** There are two: `rpg-api-lab` on :8081 and
`rpg-api-lab2` on :8082, so two concurrent experiments don't collide with
each other either.

```bash
cd rpg-deployment

docker build -t rpg-api:local ../rpg-api   # skip if already built

# brings up rpg-api-lab + envoy-lab alongside the existing stack — never
# touches the primary rpg-api/envoy services
docker compose -f docker-compose.local-dev.yml \
               -f docker-compose.local-lab.yml up -d rpg-api-lab envoy-lab
```

Point a worktree's dev server at the lab instance instead of the primary:

```bash
VITE_API_HOST=http://localhost:8081 npm run dev
```

`vite.config.ts`'s `/connect` proxy already reads `VITE_API_HOST` (falling
back to `localhost:8080`), so this needs no code change — just the env var
on the `npm run dev` you start from that worktree.

| Knob | Default | Purpose |
| --- | --- | --- |
| `RPG_LAB_DUNGEON_KEY` | `wall-lab` | lab1's default dungeon key |
| `RPG_LAB2_DUNGEON_KEY` | `look-lab` | lab2's default dungeon key |
| `RPG_CONTENT_HOST_DIR` | `../dungeon-content` | host dir mounted read-only at `/content` on both lab instances |

Both are shell env vars read at `docker compose up` time (e.g.
`RPG_LAB_DUNGEON_KEY=my-experiment docker compose -f ... up -d rpg-api-lab`),
not values baked into the compose file — set them per shell, don't edit the
file. The lab instance shares the primary's redis/mongo/5e-srd-api network;
only `rpg-api-lab`/`envoy-lab` are separate. See
`rpg-deployment/docker-compose.local-lab.yml` for the full service
definitions.

## Update assets (models, textures)

**Assets have nothing to do with the API.** They are static files that Vite
serves straight off disk from `rpg-dnd5e-web/public/models/synty/`. Replacing
them needs no API restart, no container rebuild, and no Vite restart — just a
hard reload in the browser.

```bash
cd rpg-dnd5e-web
npm run assets:sync        # pulls rpg-game-assets, rsyncs harness/models/synty/ -> public/models/synty/
```

Then **hard-reload** (Ctrl+Shift+R). GLBs cache aggressively; a normal reload
will keep serving the old model and look like the sync did nothing.

Two things to know about `assets:sync`:

- It runs `git -C ../rpg-game-assets pull`, so it gives you whatever is on
  that repo's **default branch**. Assets sitting on an unmerged branch will not
  appear — you will silently get the old ones back.
- It is `rsync -a --delete`, so `public/models/synty/` becomes an exact mirror.
  That also makes it the reliable undo after any manual experiment.

To preview assets from a branch or a local Blender export before merging, copy
them in directly and use `assets:sync` to restore:

```bash
cp <source>/harness/models/synty/characters/*.glb \
   rpg-dnd5e-web/public/models/synty/characters/
# ... look at it, then:
npm run assets:sync        # back to the default branch's set
```

`public/models/synty/` is gitignored — Synty's license permits shipping
converted assets in game builds but forbids redistributing them through a public
repo, which is why it is populated by script rather than committed.

**Merging an asset PR does not deploy it.** `rpg-dnd5e-web`'s `docker.yml`
triggers only on pushes/PRs to *that* repo (or `workflow_dispatch`), and bakes
assets by shallow-cloning `rpg-game-assets`'s default branch at image build
time. Shipping an asset change is: merge in `rpg-game-assets` → trigger a
`rpg-dnd5e-web` image build → deploy.

## Watching stream events

`debug-stream.md` (the old rpg-dnd5e-web how-to for this) is gone — it documented a
manual console.log-and-squint workflow tied to code that's since been refactored.
The real answer now: **`npm run dev` already logs every streamed message, with no
setup.**

`src/api/streamLogging.ts` (rpg-dnd5e-web#649) wraps `StreamEncounter`'s response in
client.ts's shared logging interceptor. Unlike a plain unary request/response log,
it logs each **message as it arrives**, not just the fact that a stream opened — the
gap it closed was that the old interceptor could only log the stream's iterator
object once, at open, and never a single event inside it.

It's gated on `import.meta.env.MODE === 'development'` in `client.ts`, same as the
rest of the request/response logging — automatic in `npm run dev`, nothing to turn
on. Open the browser console and look for:

```
🟣 Stream opened: dnd5e.api.v1alpha2.encounter.EncounterService.StreamEncounter
🟣 Stream: ...StreamEncounter #1 +12ms hexKnowledgeChanged 12 hexes (9 visible, 3 remembered), 2 entities
🟣 Stream: ...StreamEncounter #2 +340ms entityMoved
⚪ Stream ended: ...StreamEncounter (2 messages, 1204ms)
```

Each line's label is the event's oneof case (`entityMoved`, `roomRevealed`, ...);
`hexKnowledgeChanged` — the highest-traffic event — gets a richer summary (hex
count, VISIBLE vs REMEMBERED split, entity count) instead of a bare label. A `🔴`
line means the stream itself errored, not a single bad event.

An in-game panel for the raw event stream is filed (rpg-dnd5e-web#647) but not
built — the console wrapper is the current tool, not a stopgap being replaced
imminently.

## Gotchas that have cost real time

**Stale installed protos.** `rpg-dnd5e-web`'s *installed*
`node_modules/@kirkdiggler/rpg-api-protos` can be stale even when `package.json`
and `package-lock.json` are both correct. When stale, the client silently fails
to decode newer proto fields — hit with `Space.theme` and
`WALL_KIND_DOOR_LOCKED` — so **local behaves differently from deployed**, since
CI does a clean install and gets the right protos. Symptoms are visual or
interaction gaps with no errors. `npm install --force` does not fix it; npm's
git cache serves the stale tarball. Only this works:

```bash
cd rpg-dnd5e-web && rm -rf node_modules package-lock.json && npm install
```

**Screenshotting the live game route.** Use `waitUntil: 'domcontentloaded'`,
never `networkidle` — `StreamEncounter` holds an open stream, so `networkidle`
always times out. `tools/browser/screenshot.mjs` still defaults to
`networkidle` and will hang on game routes.

**Reading live server state.** Go straight to Redis instead of adding logging:

```bash
docker exec rpg-redis-dev redis-cli GET "enc:v2:<encounter-id>"
```
