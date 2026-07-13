# Active handoff — 2026-07-13: final-weekend close; combat-entry family + rage family fixed; frontier is The Dungeon leg design

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**The final-weekend engagement (2026-07-06 → 07-13) closes with the stack whole.** After
the game-screen rebuild completed (see the #85-era handoff), the bug-fixing stretch
landed four more units, all playtest-verified pre-merge: the **NPC-first stall fix**
(rpg-api#638 — server-side single-flight kick drives NPC turns at combat entry;
`--inject-combat-npc-first` gives a deterministic repro knob), the **rage leak sweep**
(rpg-toolkit#753 + bump #639 — encounter-scoped conditions end at encounter end via a
new `CombatEndEvent`, opt-in per condition mirroring RestTopic), and the **rage-sustain
RAW fix** (rpg-toolkit#756 + bump #640 — sustain flag reads `PostAttackRollChain`, so a
missed attack keeps rage; verified live including rage resistance halving a crit).
Current dnd5e module: **v0.65.2**; encounter: **v0.24.5**. The dev stack demo is fully
reliable: lobby → GameView → inject a goblin → narrated fight → victory, any initiative
order.

## Solid (verified — keep, don't re-derive)
- **Nine playtest-found bugs this engagement, seven fixed+merged**, two open with full
  evidence: rpg-api#637 (inject-seeded first actor's economy empty after restart — 3
  deterministic repros; the #638 cascade path sidesteps it for normal flow) and
  rpg-toolkit#754 (snapshots carry no active conditions — hydrated-in statuses are
  invisible to late/reconnecting viewers; additive contract fix sketched in the issue).
  Plus rpg-dnd5e-web#444 (no resume into a running encounter after refresh) and
  rpg-api#641 (flaky Rogue test: v1 StartCombat generation is seed-dependent — fix with
  a deterministic seed or let it die with v1).
- **Combat entry is still dev tooling** (`devseed --inject-combat [--inject-combat-npc-first]
  --encounter-id=<id>`, out-of-process Redis write; clients pick it up via reconnect /
  api restart). The REAL trigger is The Dungeon leg's design work.
- **No room/space concept exists on the encounter stack** (visibility = SightRange
  radius; ADR-0034 defers the spatial fold-in; tools/spawn room placement is a dead
  stub; doors carry the only wall plumbing). The walled-room design is toolkit-led.
- **HP persists across encounters** (no rest = no heal) — appears intentional; rest
  verbs exist in the rulebook (LongRest/ShortRest) but aren't reachable from the game.
- MCP-input limitation: canvas targeting is driven via the component's onEntityClick
  in playtests (r3f raycaster unreachable by synthetic events); human mouse click on a
  goblin still owed once as confirmation.
- rpg-api lint debt rpg-api#631 unchanged (73 issues, lint-scope to touched packages).

## Process (all locked; memoried on the director side)
1. Playtest-before-merge on any PR with observable behavior; evidence on the PR.
2. Copilot respected — it caught real bugs in five separate PRs this engagement —
   AND verify it actually reviewed (oversized PRs get skipped silently → gate those).
3. Every review gate carries a doc-drift axis.
4. No multi-line bash/heredocs in agent work; Write/Edit for files. Allowlist in
   project `.claude/settings.json` (read-only set + git rm/mv).
5. **Bumps ride the implementation PR that needs them** (Kirk, 2026-07-13); standalone
   chore bumps only for toolkit-only fixes with nothing api-side queued — say so in
   the PR body. Rules fixes land toolkit-side and reach the server as pure bumps —
   that's the boundary working, not a smell.

## Open questions (verify before acting; not findings)
- rpg-api#637 root cause: the seeded-at-inject TurnState/economy doesn't survive
  persist→load→project. The #638 drive path re-seeds correctly; compare the two.
- Whether the four Class-Kit L1 verify stories (board drafts) should run before or
  with The Dungeon leg design — Kirk's call at next pickup.

## Next (driven by board #19)
1. **The Dungeon leg design** — the big one: real combat entry (replaces injection),
   walled rooms (ADR-0034 + environments.QuickRoom bridge + spawn-stub budget +
   doors precedent), monster seeding, multi-room trailblazer. Design bites before
   implementation; old-vs-new.md gap rows 1b–3 are the map.
2. **State-fidelity pair**: rpg-api#637 + rpg-toolkit#754 (related seams; #754's fix
   is additive across all four repos).
3. **Game Screen polish**: HUD (gap row 4), in-map movement (the Move button is a
   stub), resume (web#444).
4. Shelf: rpg-api#631 lint debt, rpg-api#641 flaky test, `AbilityScoresSectionV2`
   naming straggler.

## Decision log (this stretch)
1. NPC turns driven at combat entry via subscribe-time kick, single-flight (#638).
2. Condition lifetime = opt-in per condition via CombatEndTopic; no taxonomy (#753).
3. Rage sustains on attack ATTEMPT (PostAttackRollChain), not hit (#756).
4. Bumps ride implementation PRs (2026-07-13, after #640's flake-blocked round-trip).
5. Flaky v1-path test: deterministic seed or death-with-v1, not a rewrite (#641).

## Pointers
- Board **#19** — https://github.com/users/KirkDiggler/projects/19 — reads true as of
  2026-07-13; every open item above is on it with leg + status.
- `ideas/game-screen-rebuild/` — design.md / lobby-surface.md / old-vs-new.md (the
  remaining-work map).
- Wave records: rpg-project#81 (Party Assembles retro), rpg-api#634 close-out (first
  fight), web#448 body (deletion survivors), PR evidence comments on every merged fix.
- Dev stack: api `AUTH_DEV_MODE=true go run ./cmd/server server` (50051), web
  `npm run dev` (3001), chrome `scripts/rpg-chrome.sh` (9222), cast
  `go run ./cmd/devseed` (+`--fixture=wave-2-beat2`), goblin `--inject-combat`
  (+`--inject-combat-npc-first` for the worst-case order).
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
