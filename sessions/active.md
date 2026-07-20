# Active handoff — 2026-07-20 (~02:00): THREE-SESSION SPLIT — platform / game-UX / assets

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

The director session ends near its token ceiling after a massive night. Kirk split the
work into THREE parallel main sessions; this handoff seeds all of them. The board
(#19, https://github.com/users/KirkDiggler/projects/13 is chapter history — the live
queue is board #19) remains the single coordination surface. Issue-first, one issue
per PR, every item boarded.

## The three sessions (Kirk's org decision, 2026-07-20)

1. **PLATFORM (this handoff's primary heir)** — toolkit, api, protos, wire/events,
   dungeon slices. Everything behind the renderable contract.
2. **GAME UX** — game-screen information architecture, panel design, input feel.
   Kickoff charter: **web#525** (Kirk's live critique: OA chip reads as a toggle,
   End Turn doesn't read as a button, stacked-row taxonomy buries the verb flow).
   Also reads rpg-project#78. Constraint: web renders + sends intent, never gates
   on rules; server-driven action menu stays authoritative; Discord-shaped
   viewports are the design target. Design direction reviewed with Kirk BEFORE
   implementation (/concepts route).
3. **ASSETS (already running, Kirk's parallel sessions)** — the look, end-to-end:
   rpg-game-assets AND the web's 3D rendering components. Charter: **web#523**
   (props: keys-not-enums contract), plus transferred items: #509 (parked
   gate-passed animation PR — re-verify vs new packs before merge), #512 (downed
   GLB re-export — was in progress), #515 (local-player perma-tint). Wiring the 4
   classes + idle + downed states is theirs.

## Now (platform)

- **api#675 (the night's capstone) was IN ITS FINAL LOOP at handoff**: abandon
  path (Closes api#663) + toolkit bumps that activate combat pockets + rage-at-
  seating in the live game. State: live playtest evidence on the PR (pocket
  bounce + live abandon push — director-viewed), Copilot left 3 valid threads
  (findVisiblePosition placement validation; two tests that hang instead of fail
  on missing events), agent abandon-663 was fixing them. REMAINING: fixes pushed
  → CI green → Opus review gate (NOT yet run — dispatch it; every gate tonight
  caught something) → Kirk merges → deploy → **the merge playtest must confirm
  the pocket bounce live in Discord** (fight goblin 1 → FREE_ROAM breather →
  goblin 2 fresh pocket → ModeEnded).
- **web#520** (dock 49%→10.7% at Discord viewports): all real checks green;
  Deploy Preview check failed ONLY due to the GitHub Actions incident
  (2026-07-20 ~00:07 UTC; agent-verified platform-side). Rerun when Actions
  recovers → Kirk merges. web#521 tracks the deprecated Node-20 pin it exposed.
- **toolkit#803** (cosmetic: End error-string leak, closes #802) — CI was
  running; land whenever green.

## Next (platform queue, in order)

1. Finish #675 (above). It closes the #495→#510→#663 arc entirely.
2. **Wave-2 Slice 2** — the two-chamber dungeon, THE next major move. All
   prerequisites merged tonight (doors #791, pockets #796, varied rooms #793,
   End #798). Spec: design doc §Slice 2 (ideas/the-dungeon/wave2-design.md).
   Toolkit: generator emits 2 chambers + door + entrance + region tags. Protos:
   additive Wall.id (THE wave's only wire change). API: project door walls,
   entrance-anchored spawn (kills the wall-adjacent-spawn debt, roomCenterHex
   dies), per-chamber goblin seeding. Web: door click → Interact. Slice-2 sweeps
   owed: toolkit#802's residual (if #803 unmerged), the reload-blocking
   integration test (gate note on #791), the co-located door+wall dedup note.
3. Quick wins (any free agent): web Leave-button wiring (completes #663 for real
   users — one small web PR, AbandonEncounter RPC exists post-#675), **web#516**
   (stale economy bar in FREE_ROAM — USER-VISIBLE the moment #675 deploys),
   web#471 (death-save UI — Kirk played that scene blind tonight; borderline
   UX-session, coordinate), api#674 (decouple flaky tests + no-retry dispatch
   from image publishing — tonight's 6-hour stale-prod incident), api#667
   (ally opportunity attacks — TWICE independently reproduced tonight; toolkit
   hostility check, likely small), toolkit#799/#800 (spell-slot orphan / dead
   map cleanup — pre-caster-play debt).

## Solid (verified tonight — do not re-derive)

- **#495 CLOSED.** Root cause was nginx TWICE: buffering (nginx-ssl.conf, #51)
  then the same fix missing from nginx-http.conf — THE file prod actually runs
  (SSL_MODE=http behind Cloudflare Flexible; deploy.yml writes it every deploy).
  rpg-deployment#54 fixed it; Kirk live-confirmed streams through full combat.
  Neighbors tracked: rpg-deployment#53 (limit_req 503 HTML → "invalid envelope"
  on click bursts), web#507 (silent stream reconnect, no last_seen_sequence).
- **web#510 CLOSED**: players were invisible on the REAL game screen since #502
  — plain Object3D.clone breaks SkinnedMesh skeleton binding; SkeletonUtils.clone
  fix shipped in #517. ALL prior "live verification" had run the /playtest
  harness route only. **Process rule (in memory + below): player-visible
  evidence must come from the real game route.**
- **api#670 CLOSED (#671)**: StartEncounter seats now route through toolkit
  RestoreForNewEncounter + persist. **toolkit#795 (#801)**: restore now also
  refreshes ALL Data.Resources pools (rage/ki/hit dice) at EVERY new seating,
  ungated by HP (arcade semantics; hit dice deliberately full, not RAW-half).
  Delivery = #675's dnd5e bump through the #671 call site.
- **Toolkit main** (all merged tonight): #789 seeded rooms + symmetric LoS,
  #791 doors block/reveal (DoorData single-source, projected walls), #793
  RandomPattern retry (30%→96% non-empty at 20×20), #796 combat pockets
  (LoS-scoped initiative, non-terminal TURN_BASED→FREE_ROAM pocket exit,
  ModeEnded = whole-clear only), #798 End(reason) admin verb (works from
  FREE_ROAM; reason "abandoned"), #801 arcade resource restore.
- **api main**: #669+#673 (toolkit delivery bumps + honest spawn test — the
  all-six-directions assertion was an empty-room-era fixture assumption),
  #671 (restore at seating). **protos#184**: AbandonEncounter(lobby_id) on
  LobbyService (additive; director line-reviewed — Copilot doesn't cover protos).
- **web main**: #502 class GLBs (+#517 skeleton fix), #514 armed-action state
  machine (there WAS no click-action-then-target flow before), #487/#493/#496/
  #498/#500 (the UI pass), evidence discipline throughout.
- **Deploy truth**: merging ≠ shipping. rpg-api's docker.yml runs tests in the
  publish job — a flake stranded prod 6+ hours tonight while Kirk manually
  redeployed stale images (api#674 tracks decoupling). **Post-merge duty: check
  the main build+push+deploy-trigger completed after EVERY merge** (in memory).
- **devcombat.Inject** (in #675): now guarantees LoS placement, forced-SetMode
  escape hatch DELETED (post-#796 it manufactured impossible states). Known:
  room.GetEntitiesInRange is blind to players (only spatial-grid entities) —
  trap for Slice 2 spawn work.

## Open questions

- Kirk's ghost sighting mid-evening (model off-map in the void, untextured) —
  the position half was never separately root-caused (the skeleton fix made the
  model render at the right hex; the off-map T-poser may have been the same bug
  manifesting differently). If it recurs post-#517: fresh investigation, don't
  assume.
- Why the harness route tolerated the broken clone but the real route didn't —
  flagged unresolved in #517's PR body. Matters only if it bites again.
- The #656-guard flake under -race in docker-stage runs specifically (passed PR
  CI, failed main twice) — post-#673 rewrite it SHOULD be stable; if the publish
  job flakes again on it, that's a real hole, not weather (see api#674).

## Decision log (tonight, dated 2026-07-19/20, all visible on board/PRs)

- Three-session split (Kirk): platform / game-UX (#525) / assets (#523).
- Asset team owns the look end-to-end incl. web 3D components; props use string
  reference keys + asset-owned manifest, NOT per-prop enums (director rec, Kirk
  reviewing on #523); enum stays for structural kinds only.
- Cross-module toolkit PRs = MERGE COMMITS, never squash (squash orphans pinned
  pseudo-version SHAs; #789's bb98112 rescued via archive/pr-789-head tag +
  bot tags). Single-module PRs may squash. → toolkit CLAUDE.md at next retro.
- Playtest evidence: real-route frames for anything player-visible; harness
  dev-panel acceptable for wire/behavior claims (event-log proofs).
- The bump PR fixes what the bump breaks (precedent: #673 test rewrite, #675
  devcombat fix). Main never lands a known-red test.
- AbandonEncounter proto shape approved by director under standing authority
  (additive, matches #663's written design) — logged same-turn to Kirk.
- #509 parked at Kirk's call ("animations ahead of itself") then transferred to
  asset lane with re-verify condition.
- UX split into its own main session (Kirk, on the panel critique — #525).

## Process rules locked/updated tonight (memory has full versions)

Real-route evidence for player-visible claims; post-merge main-build
verification after every merge to auto-deploying repos; agent Copilot monitors
SYSTEMATICALLY race the review — director gh-checks on every implementer idle
(GraphQL reviewThreads works when the REST comments endpoint is down); TaskStop
before replacement dispatch; single-line bash for agents (heredocs hang Kirk's
prompts); background-child research results route to the SESSION loop, not the
spawning agent — director relays.

## Pointers

- Wave-2 design (forks LOCKED): ideas/the-dungeon/wave2-design.md
- Board: https://github.com/users/KirkDiggler/projects/19 — legs: Party
  Assembles / Class Kits / The Dungeon / Game Screen / Capstone / Shelf /
  Asset Pipeline. IDs for item-edit are in the director memory
  (feedback_* + this file's git history).
- UX kickoff: web#525 + rpg-project#78. Asset kickoff: web#523.
- Deploy: rpg-deployment (nginx-http.conf is the LIVE config; Cloudflare
  Flexible in front; deploy.yml SSM pulls :latest — verify GHCR moved before
  believing a deploy changed anything).
- Open PRs at handoff: api#675 (capstone, in final loop), web#520 (dock,
  awaiting Actions recovery), toolkit#803 (cosmetic), web#509 (parked → asset
  lane). Dependabot debt: web#314/#254/#253/#252/#251/#205, stale web#38.
