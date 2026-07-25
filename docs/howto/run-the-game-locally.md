# Run the game locally

The fast loop is **backend in containers, web on the host**. Vite serves the
client with HMR; everything else runs from `rpg-deployment`'s compose files.

> Do **not** use `docker-compose.local-src.yml` for visual work. It
> containerizes the web app too — read-only mount, no `node_modules` — which
> kills Vite HMR and puts a Docker rebuild in the middle of every UI tweak.

## Start everything

```bash
cd rpg-deployment

# redis, mongo, 5e-srd-api, envoy, nginx (prebuilt images)
docker compose -f docker-compose.local-dev.yml up -d

# rpg-api from your local ../rpg-api working tree (~23s)
docker build -t rpg-api:local ../rpg-api
docker compose -f docker-compose.local-dev.yml \
               -f docker-compose.local-api-src.yml up -d rpg-api

# client on the host
cd ../rpg-dnd5e-web && npm run dev        # :3001
```

`docker-compose.local-api-src.yml` is an **overlay** — it only replaces the
`rpg-api` service with one built from local source. Always pass both `-f` flags
together, in that order, or compose will not know about the other services.

| Service | Port | Notes |
| --- | --- | --- |
| vite (host) | 3001 | `vite.config.ts` proxies `/connect` → `localhost:8080` |
| nginx | 80 | |
| envoy | 8080 | gRPC-web bridge |
| 5e-srd-api | 3002 | |
| redis | 6380 | host port; container listens on 6379 |

## Restart the API

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

**Toolkit changes are not in this loop.** `rpg-api/go.mod` pins published
`rpg-toolkit` versions with no `replace` directives, so toolkit work still needs
publish → `go get` before the API can see it.

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
