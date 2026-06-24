# Session handoff — 2026-06-24: tumult + Slice 2 prep for rpgkit-ue integration

**For the next session:** read this, then `CLAUDE.md`, then
`ideas/tumult/{design,plan-slice-2}.md`. The active thread is:
**tumult (the game library) is live; rpgkit-ue adopts it (Slice 2)**.

The prior thread (Chapter 1: Architecture Honesty / rpg-api encounter
clean-slate) is archived at `sessions/active-2026-05-30-rpg-api-encounter.md`.
That work is valid but paused — the active energy moved to a cleaner path:
extracting the game from the host entirely into `tumult`, then re-hosting
UE as a pure edge.

## Where things stand (end of 2026-06-24 Linux session)

### The repos

| Repo | Role | State |
|------|------|-------|
| **rpgkit** | Dep (C++ Bus/Chain/Effect) | v0.3.0 tagged, observation API landed. PR #56 (tutorial workflow doc) open. |
| **tumult** | The game (on rpgkit) | **Slice 1 MERGED + tagged v0.1.0.** 27 tests green on Linux. PR #2 merged. Issue #1 closed. |
| **rpgkit-ue** | UE edge | Main unchanged. PR #4 (Extract encounter runtime) OPEN → closing as superseded by Slice 2. UE 5.7 → 5.8 upgrade pending. |
| **rpg-project** | Brain | PR #63 (tumult design + Slice 1 plan) merged. PR #64 (Slice 2 design + plan) open. |
| **rpgkit-demo-game** | Terminal edge | Unchanged (Slice 7 target). |

### What was built today

- **tumult v0.1.0** — `Character`, `Encounter` (owns bus + characters +
  request-topic subscriptions), `VulnerableEffect`/`ToughSkinEffect`/
  `BleedEffect` (extend `rpg::core::Effect`, source
  "vulnerable"/"tough-skin"/"rend"), `BreakdownStep` + `formatStep`.
- **The simplification proof:** `Integration.HeroStrikeBreakdownNamesEveryModifierFromReceipts`
  — `formatStep` reads modifier `source` from `Chain::Step::source` (rpgkit
  v0.3.0 receipt), not a hardcoded string. The chain runs base 8 →
  vulnerable 12 → tough-skin 11; the formatted output contains
  `(vulnerable)` and `(tough-skin)` from the receipt.
- Copilot review on PR #2 caught 6 real findings; all fixed and merged
  (applyEffect stale-pointer, base step id, endTurn/bus guards, bleed bus
  capture, remove status propagation).

### The four Slice 2 design decisions (locked 2026-06-24)

Read `ideas/tumult/design.md` §"Slice 2 design decisions" for the full
text. Summary:

1. **tumult vendored as UE module** (`ThirdParty/tumult/` +
   `tumult.Build.cs` + sync script pinning v0.1.0).
2. **PR #4 (rpgkit-ue) superseded** — closes without merging; Slice 2
   wraps `tumult::Encounter` from the start.
3. **URPGKitBus deleted** via 3-step no-shortcut path (GameMode delegates
   verbs to Encounter; PublishEvent → GameMode BlueprintCallable;
   URPGKitBus cold-deleted). Windows gate = Kirk's BP-asset scan.
4. **UE effects wrap tumult effects** — thin UObjects owning tumult
   subclasses; receipts surface to BP hooks.

## Next session (Windows / UE 5.8)

The Slice 2 implementation plan is at `ideas/tumult/plan-slice-2.md`
(10 tasks, each marked `[LINUX]` or `[WINDOWS]`). The Linux session can
land:

- Task 0: UE 5.8 upgrade + PR #4 closure
- Tasks 1–7: Vendor tumult, migrate GameMode verbs, migrate effects,
  PublishEvent forwarder (write-only — cannot verify UE build on Linux)
- Task 9: Docs + PR

**Kirk's Windows session finishes:**
- Task 8: BP-asset scan for URPGKitBus references → delete URPGKitBus →
  UE 5.8 build → playtest
- Verify Tasks 2–7 compile + the demo loop plays end-to-end
- Confirm HUD damage breakdown shows `(vulnerable)` etc. from the chain
  receipt (the Slice 1 thesis, now on the live UE HUD)

## Open PRs (as of this writing)

- `rpg-project #63` — tumult design + Slice 1 plan — **MERGED**
- `rpg-project #64` — tumult Slice 2 design + plan — **OPEN** (review
  pending; merge on Kirk's OK)
- `rpgkit #56` — tutorial session workflow doc — **OPEN** (Copilot review
  pending)
- `rpgkit-ue #4` — Extract encounter runtime — **OPEN** → closing as
  superseded (Task 0, Step 2 of the Slice 2 plan)

## Key context for a fresh session

- **CLAUDE.md** mentions boards #11 (Architecture Honesty) and #12 (4
  Brothers) — those are the prior chapter's boards. The CURRENT board is
  `rpgkit-ue` project #15 ("RPGKit Unreal Workshop"). tumult issues live
  on `KirkDiggler/tumult` (issue #1 closed; no board yet — PRs and issues
  are tracked directly).
- The **boundary rule** (client → API → toolkit) from the prior chapter
  now maps to: **tumult knows the game; UE + terminal are edges; rpgkit
  is a dep**. If you see game logic in rpgkit-ue's GameMode that belongs
  in tumult, that's a finding (file against tumult), not a local fix.
- TDD on tumult (Linux) = GoogleTest. Verification on rpgkit-ue (Windows)
  = UE build + playtest. The plan names the split per task.
- Subagent model: `opencode.jsonc` pins `general` to
  `opencode-go/deepseek-v4-pro` (free tier, strong code model). All
  Slice 1 tasks ran on it and passed review. Revert to Sonnet if a task
  comes back BLOCKED with reasoning depth issues.

## Pointers

- Design: `ideas/tumult/design.md` (PR #63 merged; PR #64 extends with
  Slice 2 decisions)
- Slice 1 plan: `ideas/tumult/plan.md` (executed — Slice 1 merged)
- Slice 2 plan: `ideas/tumult/plan-slice-2.md` (PR #64 open)
- Prior chapter archive: `sessions/active-2026-05-30-rpg-api-encounter.md`
- tumult repo: https://github.com/KirkDiggler/tumult (v0.1.0 tagged)
- tumult release: https://github.com/KirkDiggler/tumult/releases/tag/v0.1.0