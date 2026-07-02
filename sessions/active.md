# Active handoff — 2026-07-02: Beat 1 signed off + cleanup wave verified; Beat 2 (mechanical effects) is a fresh session

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**Chapter 2: Combat Verbs**, board **#13**, umbrella **rpg-project #54**. **Beat 1 (TakeAction) is signed off end-to-end** and the **pre-Beat-2 cleanup wave is merged and playtest-verified**. **Beat 2 = mechanical effects** (make the resolved verbs *do* something) and is intentionally **a new session** (Kirk's call). This doc is the handoff into it. Director runs thin — commission + verify (`docs/teams/roles/director/`).

## Solid (verified — keep, don't re-derive)
- **North star** = `ideas/encounter/v1alpha2/design.md` §**★ North-Star Invariants** (13). Every wave validates against it via an adversarial review gate BEFORE implementation.
- **Everything is on `main` across all repos, verified via the consolidated cleanup playtest (2026-07-02):**
  - toolkit `main` (encounter **v0.21.4**, rulebooks/dnd5e **v0.61.4**): #711 granted-strike, #713 DEX-swap, #717 ref→weapon, #718 OA-emits-attackResolved, #719/#710 move-path correlationId, #720 Move economy.
  - api `main` (on v0.21.4 / v0.61.4): #606 SELF/NONE targets, #607 bump, #610 resolver wired to `WeaponForActionRef`.
  - web `main`: #427 menu render, #429 harness surfaces rejected TakeAction.
- **Playtest-verified behaviors (2026-07-02):** general ref-keyed dispatch; attack bonus +5 (DEX swap); granted Monk bonus strike swings + damages; Hide/Dodge/Dash resolve (Dash doubles movement 30→60); Move economy drains + caps (over-budget rejected: `insufficient movement remaining`) + pushes TurnStateChanged; harness renders rejected actions as red error lines; EndTurn/NPC loop clean over 3 rounds. **No regressions.**
- **Conventions locked this session (now in director memory):** worktree-per-implementer off fresh origin/main (canonical checkout = runtime home); ALWAYS set `model` explicitly on dispatches (Sonnet default, Opus for review/deep-debug — never inherit); reference issues as `repo#N` + link.

## Open questions / calls (verify before acting; not findings)
- **Kirk's call — `rpg-api#596`** (combat-mutable-state ONE HOME, promote vs archive; recommend *promote* — the Move-economy work reinforced that mutable state has no single owner).
- **OA/reaction-path bugs found by the cleanup playtest (filed, boarded under #54) — combat-correctness, decide whether Beat 2 folds them in or a separate reaction pass:**
  - `rpg-toolkit#722` — OA **hit** deals `amount=0` / wrong damage type: the reaction attack doesn't thread the attacker's weapon (direct attacks do). OA-hit-does-nothing is a real gameplay bug — worth prioritizing.
  - `rpg-toolkit#723` — `entityDamaged` lacks correlationId on the NPC-attack / OA path (#710 fixed only the player move-path).
- **`rpg-api#611`** — `#712` ref→weapon is unit-verified only; no weapon-holding-monk/dual-wielder devseed fixture exists to observe it end-to-end.
- **Deferred cleanup follow-ups (filed, boarded under #54):** `rpg-api#608` (move_entity dense-path forwarding), `rpg-api#609` (devseed goblin-first-initiative freeze), `rpg-api#603` (door fixture for Interact), `rpg-toolkit#721` (`make pre-commit` broken on main — bc script). Minor: intermittent `rogue_test.go` procedural-gen flakiness.

## Next (for the Beat 2 session)
1. **Kirk decides `rpg-api#596`.**
2. **Validate Beat 2 scope against the North-Star Invariants** (adversarial review gate) before building.
3. **Implement Beat 2 — mechanical effects (toolkit-owned):** `rpg-toolkit#716` (Help + Hide produce a real effect/status, not economy-only) and `rpg-toolkit#699` (Dodge → DodgingCondition.Apply). Worktree-per-implementer, Sonnet dispatches. Consume into api/web as needed; bump cleanly (real tags, no replace).
4. **Sign-off:** end-to-end MCP playtest of the new effects + participatory retro (that's what closes Beat 2 — green CI is not sufficient).
5. Optionally sweep the OA-path bugs (#722/#723) — they gate honest combat but aren't the Beat 2 verb-effects goal.

## Decision log (2026-07-01 → 07-02)
1. **Reconciled the brain repo** after the Tumult/C++ detour: merged origin (9 local + 14 remote, conflict-free), kept genuine local-only notes, dropped redundant/superseded files. Tumult parked (board #14). Brain `main` is **ahead of origin, unpushed — push gated on Kirk.**
2. **Beat 1 signed off** end-to-end via MCP playtest (the done-bar; the merged PRs alone weren't the close).
3. **Pre-Beat-2 cleanup wave** (Kirk: "clean slate for Beat 2"): filed + fixed the gap backlog — toolkit #714/#715/#710/#712 + api #606-era + web #428; all merged; consolidated playtest verified the player-facing paths and surfaced the OA-path bugs above.
4. **Session model discipline:** Kirk runs director on his chosen model; dispatched agents always Sonnet (explicit) — after Fable was exhausted mid-session by inherited dispatches.

## Pointers
- Boards: **#13** (Chapter 2, umbrella #54); **#11** (Chapter 1, closed); **#14** (Tumult, parked).
- Design: `ideas/encounter/v1alpha2/{design,take-action/design,take-action/decisions}.md`; north-star invariants in `design.md`.
- Code: toolkit `encounter/` (verbs, move/OA resolver) + `rulebooks/dnd5e/{character,combat,conditions}`; api `internal/orchestrators/encounter/v2/` + `internal/handlers/dnd5e/v2/encounter/` (resolver); web `PlaytestHarness.tsx`, `ActionMenu.tsx`, `EconomyBar.tsx`.
- Dev stack was left running by the cleanup playtest (api :50051, envoy :8080, redis :6379, vite :3001, `wave-2-monk` seeded) — fine to tear down.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
