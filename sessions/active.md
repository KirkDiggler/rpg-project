# Active handoff — 2026-07-20 (~22:30): Slice 2 DELIVERED — partial sign-off, walls-render blocker

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.
> PLATFORM lane (toolkit / api / protos / deploy). The game-UX (web#525) and asset
> (web#523) lanes run their own parallel sessions; their state is theirs —
> coordinate via board #19, respect the renderable seam. Kirk is moving active work
> to fresh Opus sessions to save capacity; this handoff seeds a clean pickup.
> Board #19 is the queue; issue-first, one issue per PR, Opus gate posted on the PR
> before any merge-ready call. Merges go through the director + Kirk.

## Now (platform)

**Wave: The Dungeon — Slice 2 (two-chamber dungeon), rpg-project#96 — DELIVERED +
DEPLOYED to prod, PARTIAL sign-off.** The two chambers, the door contract, per-
chamber goblin seeding, entrance spawn, movement markers, and the FREE_ROAM
movement fix are all live. The one thing gating a FULL sign-off: **walls (incl. the
door) don't render** on the default Synty dungeon — web#562, an asset/render-lane
concern, not platform-data.

## Solid (verified this session / deployed — do not re-derive)

- **Slice 2, all four legs merged + deployed, each Opus-gated PASS:** toolkit —
  two-chamber generator (#806) + connectivity-doc correction (#807); protos —
  additive `Wall.id` (#186); api — door projection as `Wall{id,from,to,DOOR_CLOSED}`
  + entrance-anchored spawn (roomCenterHex deleted) + per-chamber out-of-sight
  seeding (#677); web — door click→`useInteract` + open/closed pose + `DoorOpened`
  reveal + door-blocked walkability (#549), movement markers/path raised above the
  Synty floor (#550, closes the "markers under the map" class). Copilot's real
  catches this wave all fixed pre-merge: RegionAt nil-panic, doorPassageNeighbor
  nil-panic, **chamberEntryAnchor wrong-region** (seeded chamber-2 goblins on the
  wrong side; masked only because a closed door blocks LoS — white-box test now
  guards it).
- **FREE_ROAM move-budget bug FIXED + deployed (toolkit#808 → #810 → api#679,
  prod 22:09Z).** Root cause: the move gate keys off `char.InCombat()` (economy !=
  nil), NOT the mode; the pocket exit `checkPocketCleared` flipped `SetMode(FreeRoam)`
  but never tore down the combat economy, so players stayed `InCombat()` with 0
  movement → next move rejected in free-roam. The TERMINAL exit already did this
  teardown (`ExitCombat`, #767); the non-terminal pocket exit (#794) never got it.
  Fix: `ExitCombat` per held player at the pocket exit + `Mode==TurnBased` hardening
  on the move gate. **ExitCombat-ONLY, no EndCombat sweep → Rage/combat-scoped
  conditions PERSIST across the breather** (Kirk's call).
- **LIVE sign-off confirmed by Kirk in prod (real game route):** free movement in
  the breather works, path-following correct, pocket bounce fires (kill goblin →
  FREE_ROAM → move freely again), markers render above the floor. These are the
  wave's core behaviors — player-verified.

## Open questions / gated (verify before acting; NOT findings)

- **Walls (incl. the door) not rendering — web#562.** Space reads as flat tile, but
  movement is correctly blocked, so wall DATA + walkability are correct server-side;
  it's purely a RENDER gap. Almost certainly why "can't find the door" earlier (the
  door IS a DOOR-kind wall). Asset/render lane; Kirk deprioritized it for platform.
  Check: is default SyntyHexWall failing to render wall meshes, and did #549's
  SyntyHexWall changes regress it vs a pre-existing Synty gap? Blocks the door→
  chamber-2→fresh-pocket sign-off leg.
- Door-orientation cosmetic (doorPassageNeighbor picks the first region-tagged
  neighbor as the passage edge) — flagged on web#526; only matters once walls render.

## Next (platform, in order)

1. Close Slice 2: once web#562 makes walls/door visible, finish the door→chamber-2
   reveal→fresh-pocket sign-off leg against #96's bar; then the **retro** sweeps the
   #96 ledger and closes the wave.
2. **toolkit#809 (Decide)** — make the pocket-exit condition-sweep configurable +
   settle the RAW-correct default (5e rage-end is genuinely ambiguous for a lull).
   DO NOT build before the pocket behavior is playtested at least once.
3. Retro-owed follow-ups (on the #96 ledger + the night wave): stale EncounterView
   `onDoorOpened` comment (ride next web PR); live-`GeometryRevealed`-carries-no-
   Walls gap (api translate.go — retro watch item, web works around it safely);
   **api#678** (29 pre-existing lint findings + no CI lint gate); **api#674**
   (deploy coupling — tests run in the publish job, stranded prod 6h once); **web#516**
   (stale economy bar in FREE_ROAM — user-visible); **toolkit#805** (cross-module =
   merge-commit rule → toolkit CLAUDE.md doc PR); toolkit#799/#800.
4. **Slice 3** (locked boss door + boss chamber + completion) per wave2-design
   §Slice 3 — after Slice 2 fully closes.

## Decision log (this session, 2026-07-20, visible on board/PRs)

- Slice 2 delivered leg-by-leg, each Opus-gated; **gate scaled to the change** for
  the pure dep-bump delivery (api#679) — the delivered fix was already gated at #810,
  no api code change, integration suite green → a fresh adversarial gate would be
  disproportionate (reasoning posted on the PR).
- FREE_ROAM fix = ExitCombat-only, Rage persists across the pocket breather;
  configurable sweep deferred to toolkit#809 (Kirk).
- **Merge-janitor** standing role adopted at the retro; first worktree sweep ran
  (48 merged+clean worktrees removed, locked seats / asset staging preserved).
- Platform lane = one of THREE teams on board #19 (platform / asset web#523 / UI
  web#525); the board is multi-team — no lane assumes it's the only writer.

## Process notes (locked/learned this session — full versions in memory)

- **Relocation prompt:** dispatch builders to isolate via `EnterWorktree` **name-
  form**; a model-supplied `git worktree add` path triggers a permission-root
  relocation prompt that **dontAsk does NOT clear**. Read-only agents (gates,
  investigators) don't isolate → no prompt. (`feedback_worktree_relocation_prompt`)
- Agents idle without delivering their report systematically; a one-turn re-ask
  recovers every time — end every dispatch with "SendMessage your report BEFORE
  going idle." Check gh state before pinging.
- **MCP chrome tools don't reach dispatched subagents** → the MCP playtest is Kirk's
  hands + director verification of captured evidence, not a delegatable task.
- **Post-merge deploy verification** after every merge to an auto-deploying repo:
  watch the main build → "Deploy RPG Platform" run to terminal success (two Docker
  builds failed earlier tonight; merging ≠ shipping).
- `dontAsk` is set in `~/.claude/settings.json` (user-global); a background session
  launched before it applied runs default mode until relaunched — flip via
  Shift+Tab / `/permissions` in-session, not from a config edit.

## Pointers

- Wave-2 design (forks LOCKED): `ideas/the-dungeon/wave2-design.md` — §Slice 2 done,
  §Slice 3 next.
- Slice 2 wave umbrella: **rpg-project#96** (In Progress; its comment ledger has the
  full live-walk results + all follow-up watch items). Wave-level dungeon umbrella:
  rpg-api#648.
- Board: https://github.com/users/KirkDiggler/projects/19 — legs: Party Assembles /
  Class Kits / The Dungeon / Game Screen / Capstone / Shelf / Asset Pipeline.
- Deploy: rpg-deployment auto-deploys every main merge; nginx-http.conf is the LIVE
  config (Cloudflare Flexible in front); verify the "Deploy RPG Platform" run
  succeeded before believing prod changed.
- Director role: `docs/teams/roles/director/{prompt,field-notes}.md` (read first).
