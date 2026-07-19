# Active handoff — 2026-07-19 (evening): COMPACTION POINT — UI pass shipped, wave 2 begun, one prod mystery open

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

The director session compacted at ~900k tokens at Kirk's call. Everything durable
is below; two implementer agents were live at compaction (their worktrees/PRs
survive — resume or re-dispatch, see In-Flight).

## The one open mystery: prod movement (#495)

**rpg-dnd5e-web#495** — the ONLY unresolved blocker for playing in Discord.
Trail (all evidence on the issue):
1. Movement dead in deployed Discord; three client/server theories falsified by
   live repro (local current-main works perfectly end-to-end: 185 hexes revealed
   on connect, click-to-move works).
2. Kirk's console capture: `protocol error: incomplete envelope` → nginx was
   buffering/truncating grpc-web streams. Fixed: **rpg-deployment#51** (merged,
   deployed ~19:00): proxy_buffering off, 1h stream timeouts, conditional
   Connection header.
3. Post-fix: stream error GONE; briefly saw `invalid envelope` on click
   (signature of an nginx error page — rate-limit 503 or referer-gate 404 —
   where proto bytes belong; suspects: `limit_req zone=api burst=20` shared by
   stream+RPCs, and the `if ($http_referer !~ discordsays.com) return 404` gate,
   both in nginx-ssl.conf's Service location). Then: **cannot reproduce; console
   quiet; still mid-room with no movement.**
4. CRITICAL UNKNOWN: Kirk may STILL be resumed into a poisoned OLD encounter —
   the resume feature re-imprisons (no abandon path, **rpg-api#663**), and old
   encounters' tiny revealed sets are PERSISTED server-side; no client/nginx fix
   can grow them retroactively.

**Next steps, in order** (also on #495):
a. Kirk escapes the old encounter: `redis-cli DEL "player:<discord-id>:lobby"`
   on the prod host → reload → create a genuinely FRESH encounter post-all-fixes.
b. Fresh encounter floor WIDE + movement works → close #495, promote #663
   (abandon path) so the trap dies.
c. Fresh encounter STILL 1-tile → prod server-side reveal bug: get
   `redis-cli GET enc:v2:<id> | grep -o '"sight_range":[0-9]*'` from prod, and
   check what api image the prod compose actually pins (deploy may pin a stale
   tag — never verified).
d. If movement dies with wide floor: capture MoveEntity's network entry
   (status + response preview — HTML=nginx rule, which code = which rule).

## Kirk's list at compaction

1. **Merge rpg-project#92** — wave-2 design, forks all RESOLVED (recorded in-doc).
2. #495 next-steps above (his infra/keys).
3. Character creation + sheet eyeball pass (post-#487 styling revival) — low priority.

## In-flight agents at compaction (worktrees/branches survive)

- **toolkit-slice0** — wave-2 slice 0: toolkit#787 (seed room RNG; QuickRoom
  never calls WithRandomSeed → identical rooms) + toolkit#788 (lerpCube cube-
  rounding; LoS asymmetric ≥22 hexes). Worktree
  rpg-toolkit/.claude/worktrees/wave2-slice0, branch feat/wave2-slice0. If dead:
  re-dispatch from the issues; cross-module = #779 two-commit pattern, no replaces.
- **web-485-fixes** (veteran, many merged slices) — CHARACTER MODELS HOOKUP:
  class GLBs shipped class-named in rpg-game-assets (harness/models/synty/
  characters/{class}.glb + -downed variants + manifest + portraits). Map
  classRefId (threaded via #493/#665) → GLB, SYNTY_SCALE, downed variant off the
  unconscious status, fallback = current MediumHumanoid (#479 boundary lineage),
  monsters out of scope. Issue-first; live verify with multi-class party.

## Wave 2 — The Dungeon (design DONE, forks LOCKED 2026-07-19)

Design: rpg-project#92 (ideas/the-dungeon/wave2-design.md) — gate-hardened
(adversarial gate found the missing crux). All six forks RESOLVED as recommended:
1. ONE continuous Space, rooms = wall-partitioned regions (orchestrator in
   tools/spatial is abstract/positionless — rejected with rationale); spawn
   per-region up-front, no respawn.
2. **COMBAT POCKETS** (the wave's one substantial new toolkit build): LoS-scoped
   initiative + non-terminal region-clear TURN_BASED→FREE_ROAM exit + ModeEnded
   reserved for dungeon completion. Without it: one dungeon-long initiative that
   soft-locks on the locked boss (gate-proven; every escape hatch checked closed).
3. Toolkit-owned generation (retire the dormant rpg-api internal/components/dungeon).
4. Doors: DoorData entity = truth (encounter/data.go:170 scaffolding, has lock
   fields), PROJECTED as DOOR-kind walls; ONE additive wire field `Wall.id`
   bridges click→Interact (which works end-to-end today). Passage-edge problem
   dissolves via from/to. DoorOpened's revealed/removed fields: use-or-deprecate.
5. Skill-check unlocks (already wired: Interact routes Locked→AttemptUnlock).
6. Three chambers / two doors / boss (NewGoblinBoss to be built; only NewGoblin
   exists); party split allowed ungated.
Slices: 0 (#787+#788, in flight) → 1 doors block+reveal (integration-gated,
honest label) → 1b combat pockets → 2 two-chamber traversal (carries Wall.id,
the wave's only wire change) → 3 locked boss door + completion. Closing playtest:
mouse-only enter→fight→open→traverse→boss→victory.

## Today's shipped ledger (all deployed; deploy pipeline ALIVE and fail-loud)

- Deploy resurrection: 5 months of silent rot (dead DEPLOYMENT_TOKEN swallowed by
  continue-on-error) found via smoke pass, fixed end-to-end (rpg-deployment#50
  closed; web#478/api#662; token rotated; both chains live-verified).
- Styling resurrection: web#487 — Tailwind v4 migration had killed ALL themed
  utilities app-wide for ~a year (`@tailwind` directives v4 ignores); one-line
  fix; CSS 22KB→53KB.
- The UI pass: #482 lobby polish, #493 EncounterDock (max-viewport map, portal,
  #486 real combat movement economy), #496 dock responsiveness (42vh cap),
  #498 action icons, #500 wall variety (deterministic FNV per-edge), #485
  free-roam movement (killed a fabricated client 30ft budget) + unlit floor.
- api#665: Entity.display_name + CharacterData.class_ref (dock shows real names).
- toolkit#786: arcade recovery (death encounter-scoped; AddPlayer-gated restore;
  delivered to api by PURE BUMP — the toolkit-as-product ideal). #785 closed.
- Asset initiative (Kirk's parallel sessions): private rpg-game-assets repo,
  assets:sync, deploy baking with PAT (leak-safe pattern gate-verified), env
  piece→role manifest, 9 action icons, class-named character models. Coordination
  via web#469 + asset-request label + board Asset Pipeline lane.

## Open issues worth knowing (all boarded on #19)

- web#495 (the mystery above), api#663 (abandon encounter — PROMOTE after #495),
  web#492 (shared snapshot-apply helper — kills the unconsumed-field disease
  class, 4 instances to date), web#471 (death-arc rendering remainder + downed
  visual), web#484 addendum (audit silently-unstyled components), api#658
  (silent move refusals — more reachable now), api#666 (identity-read batching
  seam), toolkit#780 (provenance drift tripwires), toolkit#784 (per-hit HP sync
  remainder + monster side), protos#182 (EncounterMode ENDED value), protos#183
  (sheet-vs-snapshot: RESOLVED as Option B overlay — future web slice: pull-out
  sheet in-encounter), api#660 (roll contract divergence / future Roll mode).

## Process rules locked this week (memory has details)

Playtest-before-merge on BRANCHES (Kirk's correction — merges land verified
code); PRs open READY (drafts suppress Copilot); evidence images commit-pinned
raw URLs, attach-at-write, viewed-statements both directions (gates refuse
unviewable claims); TaskStop-before-replacement-dispatch; never delete foreign
worktrees; Scope-decisions PR sections; CDP-script screenshot recipe (MCP
screenshot writes are sandboxed); no background children in agents (results
route to the session loop); single-line bash for agents; bumps ride
implementation PRs; issue-first; board #19 is the three-initiative coordination
surface (code / assets / playtest-feedback label for friends night).
