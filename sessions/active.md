# Active session — Wave 2.11d closeout

**Started**: 2026-05-18
**Director mode**: Kirk delegated architectural calls during multi-session day; reviewing all decisions at session-end.

## Director decisions log

Each entry: decision, reasoning, what it affects, alternatives I rejected. Kirk reviews end-of-session.

### B1–B7 (earlier in session, summarized)

Earlier wave-2.11d architectural calls, made interactively with Kirk and confirmed on Monday catchup:

| # | Decision | One-line rationale |
|---|---|---|
| B1 | Encounter SDK gets two-phase verbs (TakeActionPhased + CompleteTakeAction); legacy TakeAction stays as wrapper. | rpg-api doesn't bypass SDK; SDK owns orchestration + persistence. |
| B2 | New `combat.PostAttackRollChain` topic in toolkit; Shield subscribes there. | Shield's RULE logic stays in toolkit. |
| B3 | Inline 10-line buffered-subscriber pattern, don't generalize. | Premature abstraction avoided. |
| B4 | NPC OAs go through trigger-event-scan path (not re-entrant chain calls). | Unifies player + NPC reaction flows; one ledger update site. |
| B5 | rpg-api applies dnd5e conditions on character/monster load; encounter SDK rulebook-agnostic. | Load-bearing boundary call. |
| B6 | `combat.AttackContext` pure data; `ApplyAttackOutcomeInput` takes `EventBus + Roller` (symmetric with phase 1). | Makes AttackContext JSON-roundtrippable for cross-RPC persistence; cleaner than exporting unexported fields. |
| B7 | SDK `InputRequiredDeliveredEvent` is metadata-only; translator reads PendingReactionPrompts off Data at wire boundary. | SDK stays rulebook-agnostic; mirrors pat-encounter-pending-prompt. |

### Process changes shipped this session

- `feedback_verify_before_done` memory + 4-question gate (Goal / Pattern / Test / Pushback) in api-fixer/toolkit-fixer/web-fixer role prompts.
- `feedback_copilot_thread_replies` enforced in role prompts (reply on every Copilot thread, every PR).
- `feedback_local_override_until_unit_done` memory (use local replaces during unit-of-work, batch toolkit PRs at end).
- `feedback_toolkit_docs_close_the_loop` memory (platform docs land IN the wave PR, not deferred) + roadmap close-the-loop step 7 + api-fixer/toolkit-fixer role prompts.
- New session-log discipline: this file. Director decisions captured for end-of-session review.

### B8 (Wave 2.11d closeout, multi-reactor + OA verification, 2026-05-18)

**Decision**: Send agent to verify OA wiring first → if broken, fix in closeout PR → Option 1 (single-reactor enforcement) for #538C → then integration tests + web + MCP.

**Reasoning**: During verification of agent's `#538C` framing, I found the agent's "two flankers both OA" scenario is structurally wrong — OAs aren't modifiers on the leaver's move chain, they're new attacks. The real `#538C` scope is multi-Shield-style modifier stacking on ONE incoming attack (Shield + Counterspell + future post-hit reactions). For Wave 2.11d's actual shipped surface (Shield is the only post-hit modifier; OA is fan-out), Option 1 (single-reactor enforcement at persist time) is correct.

Bigger architectural question surfaced during the same verification: `Encounter.Move` (encounter.go:401) does NOT install a buffered subscriber on `ReactionTriggerTopic`, and directly mutates player position without running a `MovementChain`. The OA condition subscribes to MovementChain. If MovementChain isn't published from Move, OA's predicate never fires on player movement — meaning player-driven OA scenarios (player retreats past enemies → enemies OA) may not actually work end-to-end. This is wave-goal-relevant: 2.11d's goal includes "OA fires when leaving threatened hex" and this would mean partial verification at best.

**Rejected alternatives**:
- Option 2 (aggregate-then-complete multi-reactor) — adds ~150 lines + new persistent state shape + deadline mechanism for a scenario that doesn't apply to Wave 2.11d's shipped surface. Becomes a Wave 2.11e item when second post-hit reaction ships (Counterspell, etc.).
- Trust agent's framing, skip verification — risks shipping 2.11d with the player-OA path broken and discovering it only in MCP playtest (where the failure mode is "Sarah expected an OA prompt and didn't get one — was that the rules or a bug?").

**What this affects**: The next dispatch's scope. If OA wiring is broken (movement chain not published), the closeout PR grows to include a fix. If working, the closeout is smaller.

**Status**: Dispatching agent to verify.

### B8 outcome (2026-05-18)

Confirmed via toolkit probe test: `Encounter.Move` (encounter.go:401) bypasses `MovementChain` entirely — directly mutates position + publishes MoveEvent/HexRevealedEvent/EntityAppeared/EntityDisappeared, does NOT run any chain that OA's `onMovementChain` subscribes to. The only `MovementChain` publisher in the toolkit is `combat.MoveEntity` (rulebook-side), which rpg-api never calls. Player-OA-on-move is broken end-to-end.

Probe test asserted `triggerCount == 0` after `enc.Move("bob", path)` with alice (fighter with OA Apply()'d + readiness ON) adjacent to bob's start hex. Test passed → zero ReactionTriggerEvents emitted. Confirms wave-goal gap.

### B9 (Wave 2.11d closeout scope adjustment, 2026-05-18)

**Decision**: Adjust Wave 2.11d's scope to "Shield end-to-end + NPC OA + opt-in readiness toggle infrastructure"; defer player-OA-on-movement to Wave 2.11e. Closeout PR ships #538 fixes (Option 1 multi-reactor + save-before-publish) + 3 Shield integration tests; the 2 player-OA tests file as Wave 2.11e items alongside the MovementResolver SDK fix.

**Why this and not Fix-shape B (fix in closeout)**: The fix is structurally a new SDK design (`MovementResolver` interface mirroring `PhasedCombatResolver`) — not "we forgot a line." Bundling it into closeout means another multi-repo wave cycle (toolkit PR + autotag + rpg-api bump + integration + web + playtest). That's Wave 2.11d Round 2, not a closeout. Better to ship 2.11d-as-now-scoped and run Wave 2.11e as a coherent "complete the reactions" wave.

**Why this and not Fix-shape C (host workaround)**: rpg-api calling `combat.MoveEntity` directly + patching SDK state via a new `SetPlayerPosition` hook leaks rulebook awareness into the orchestrator and would create new SDK surface anyway. Same architectural cost as B, worse layering.

**Wave 2.11d adjusted goal sentence**: "Wizard can cast Shield to block an incoming attack (post-roll modifier); NPC OAs fire when players move past them (NPCAct dispatches the OA inline); per-character reaction-readiness toggle infrastructure works end-to-end. Player retreats triggering enemy OA deferred to Wave 2.11e along with multi-reactor aggregation."

**Wave 2.11e scope** (to file as issues):
- rpg-toolkit: `MovementResolver` interface (parallel to `PhasedCombatResolver`); `Encounter.Move` installs trigger buffer + delegates per-step movement to the resolver; resolver implementation in rpg-api wraps `combat.MoveEntity`.
- rpg-api: implement `MovementResolver`, wire into encounter rehydration alongside `PhasedCombatResolver`; integration tests for player-OA-on-move (the 2 deferred from #536).
- Multi-reactor aggregation (#538C Option 2): when Counterspell or another post-hit modifier ships, refactor single-reactor enforcement into aggregate-then-complete with deadline mechanism.

**Rejected alternatives**:
- Fix-shape A: `Encounter.Move` publishes its own MovementChain. Behavioral change to existing SDK verb; rulebook-aware in the SDK. Boundary smell.
- Fix-shape B in closeout: discussed above.
- Fix-shape C: discussed above.
- Hide the gap: not viable per Kirk's stated director role.

**What this affects**:
- Closeout PR scope (3 Shield tests, not 5)
- Wave 2.11d retro: honest about the scope shift, not "shipped goal" without qualifier
- Roadmap update: 2.11d goal sentence revised; 2.11e becomes "complete the reactions" wave
- Wave tracker #47: comment noting scope adjustment + linking new 2.11e issues

**Status**: Dispatching agent to file Wave 2.11e issues + proceed with adjusted-scope closeout.

### B10 (Wave 2.11d closeout — character.LoadFromData SpellSlots gap, 2026-05-18)

**Decision**: Fix the toolkit bug in a fresh small toolkit PR; bump rpg-api closeout to the new tag; ship 3 Shield integration tests in the closeout PR. Do NOT defer Shield to 2.11e.

**The bug** (discovered while writing Shield integration test): `character.LoadFromData` in rpg-toolkit/rulebooks/dnd5e@v0.58.0 (character/data.go:119-200) copies every Data field EXCEPT `SpellSlots`. ToData() at character.go:954 writes SpellSlots correctly. So the field round-trips JSON in / JSON out, but the runtime `c.spellSlots` never gets populated after the initial draft. Production impact: every spellcaster rehydrated via the standard `Get → LoadFromData` lifecycle loses spell slots → `hasFirstLevelSpellSlot` returns false → Shield never Apply()'s → Shield never fires. Wave 2.11d's player-Shield path is broken end-to-end in shipped code.

**Why fix-not-defer (vs B9's "defer")**: B9 deferred player-OA because it required a NEW SDK design (MovementResolver interface). B10's fix is mechanically small (one-line `c.spellSlots = maps.Clone(d.SpellSlots)` in the constructor + a round-trip test). Different cost profile entirely. Not a design call; a bug fix.

**Why not Option 2 (drop the hasFirstLevelSpellSlot gate)** (agent's recommendation): pushes the "which characters get the Shield-toggle UI" question to web. UI would need to know which characters can cast Shield — that's client-side game logic, exactly the `feedback_no_logic_in_web` boundary violation. The "readiness flag is the real gate" argument doesn't address that the toggle would be VISIBLE for every character if Shield is Apply()'d to all. Confusing for players, layering smell for us.

**Why not Option 1a (defer Shield entirely to 2.11e)**: ships Wave 2.11d with ZERO reaction-goal verification (player-OA already deferred per B9, Shield deferred per this option). Wave goal was "opt-in player reactions work end-to-end" — shipping only the plumbing with no working reaction breaks the wave-goal-shaped discipline. Better to take one more small toolkit round than ship a wave that doesn't verify any reaction.

**Why not Option 3 (test fixture bypass)**: agent rejected; I agree. Tests must exercise production paths.

**Plus side audit ask**: while in the LoadFromData fix, agent should check whether OTHER Data fields are silently dropped by the constructor (PreparedSpells, SpellsKnown, anything else draft-compiled with runtime state). If so, fix all in same PR — same bug class.

**What this affects**:
- Adds one small toolkit PR (~10 lines + test) to the closeout sequence
- Tag bump on rulebooks/dnd5e (probably v0.58.1 or v0.59.0)
- rpg-api closeout gets a dep bump commit before merge
- 3 Shield integration tests land in closeout (the originally-scoped Shield path coverage)
- Wave 2.11d retro can honestly say "Shield works end-to-end as designed"

**Status**: Dispatching agent for the toolkit fix + closeout sequence.

### B11 (small, 2026-05-18)

File `BackgroundID` dropped-field bug as a separate rpg-toolkit issue (deferred from #660 because it needs a new struct field + serializer wiring, different bug class from SpellSlots/ClassResources line-add).

### B12 (Wave 2.11d closeout — CompleteTakeAction shape gap, 2026-05-18)

**Decision**: Defer to Wave 2.11e. Ship Wave 2.11d closeout with what works (#538 fixes + 1 of 3 Shield tests + the deferred-to-2.11e issue trail).

**The gap**: `Encounter.CompleteTakeAction` (encounter@v0.9.0/combat_phased.go:218-241) is hardcoded for attacker=player, target=monster. Doc comment even says so. But the ONLY scenario Shield can fire in (in PvE Wave 2.11d scope) is NPC attacks player → player Shield prompt → SubmitCheck resume. The resume verb doesn't accept NPC attackers, so SubmitCheck{take_reaction} fails with "attacker not in encounter."

Shield's prompt path now works (after B10's v0.58.1). Shield's resume path is structurally broken in shipped SDK for the only direction Shield can fire.

**The verification math**: This is the THIRD structural gap surfaced in this single closeout session.
- B8: `Encounter.Move` bypasses MovementChain → player-OA-on-move broken → deferred to 2.11e
- B10: `LoadFromData` drops SpellSlots → Shield Apply()'d→never-fired → fixed in v0.58.1
- B12: `CompleteTakeAction` only accepts player attackers → Shield resume broken → DEFERRING

Each gap discovered by deliberate verification, not by guessing. Each was a pre-existing assumption that didn't matter until Wave 2.11d's reaction surface exposed it.

**Why defer this and not fix (vs B10 which was fixed in scope)**:
- B10 was a 5-line bug in EXISTING shipped behavior (LoadFromData was supposed to load everything; it didn't). Bug fix scope.
- B12 needs a new branch through the SDK's resume verb — design work (polymorphic verb OR new symmetric verb), not a bug fix. Different shape.
- B9 and B12 are structurally identical: "SDK doesn't have what we need for this direction." They belong together in Wave 2.11e ("complete the reactions") in one coherent design pass.
- We've spent today on three full-cycle multi-repo rounds. The original Wave 2.11d scope was misestimated; each round shrinks what's left. At some point ship what works, file the rest.

**Adjusted Wave 2.11d verified goal sentence**:
"Wave 2.11d ships: orchestration plumbing (PhasedCombatResolver + TakeActionPhased SDK + Dnd5eCombatResolver implementation), readiness toggle infrastructure (SetReactionReady RPC + per-character UI), Shield + OA condition predicate verification (toolkit unit tests + rpg-api `NotReady_NoPrompt` integration test), and the publish-vs-save + single-reactor enforcement fixes. Wave 2.11e completes the verb surface — MovementResolver (#658, #539) + CompleteTakeAction NPC-attacker symmetry (#NEW) + multi-reactor aggregation (#540) — which together make Shield-fires-and-blocks + player-OA-on-move actually verifiable end-to-end."

That's an honest report — not pretty but accurate. The verification discipline you asked for is exactly what surfaced these gaps; better to surface and file than to ship a broken claim.

**Rejected**:
- Option A (extend CompleteTakeAction polymorphically) in closeout: another toolkit round-trip, fourth in a session. Diminishing returns.
- Option B (new CompleteNPCAct verb) in closeout: same cost as A, slightly cleaner.
- Option C (auto-resolve NPC→player Shield without prompting): kills the player-choice point that's the whole feature.
- "Just fix it" — pattern says we'd find B13 mid-fix. Closeouts need to actually close.

**To file** (when agent resumes):
- rpg-toolkit: `feat(encounter): CompleteTakeAction (or new CompleteNPCAct) supports NPC-attacker resume path (Wave 2.11e)`
- rpg-api: `feat(encounter/v2): wire NPC-attacker Shield resume + 2 deferred Shield integration tests (Wave 2.11e)`

Both Wave 2.11e. Both go on board with field unset (Kirk adds 2.11e option).

**What this affects**:
- Closeout PR scope shrinks again: 1 Shield test (`NotReady_NoPrompt`) + #538 fixes only
- Wave tracker #47 comment updates with revised goal sentence
- Wave 2.11e tracker (when filed) becomes "complete the reactions": MovementResolver + CompleteTakeAction symmetry + multi-reactor — 3 SDK extensions that together unlock end-to-end verification

**Status**: Dispatching agent for the deferred-scope closeout sequence.

### B13 (Web PR #408 — hold open, don't merge, 2026-05-18)

**Decision**: Hold rpg-dnd5e-web PR #408 OPEN (not merged, not closed). Branch stays alive as the runway for Wave 2.11e web work; next dispatch extends it.

**Context**: Verified that all #408 components land in the playtest harness + shared hooks, NOT production game routes:
- `src/components/playtest/PlaytestHarness.tsx` (+247) — dev-only harness
- `src/api/useSetReactionReady.ts` (new), `useSubmitCheckV2.ts` (extension), `src/hooks/useEncounterState.ts` (new) — reusable infrastructure

No production-route UI changes. Players never see the skeleton-forever state I'd worried about; only Kirk + MCP-driven harness runs do.

**Pattern Kirk articulated**: "we have our playtest route that we use to verify the backend. when we get to hooking up the front end we already have it vetted." The playtest harness is the runway; vetted infrastructure lives on long-lived branches until the SDK story is ready to wire against.

**Correction I owe**: my initial recommendation was to close #408 if pausing. That was wrong — closing loses the branch's status as the canonical "vetted web infrastructure for 2.11d-into-2.11e" pointer. The branch is the work product; keep it open, rebase against main when 2.11e starts.

**What this affects**:
- Wave 2.11d closeout merges only rpg-api #542
- Wave 2.11d ships: rpg-api orchestration + #538 fixes + 1 Shield test + toolkit SpellSlots fix (v0.58.1)
- Wave 2.11e dispatch's first action on web: rebase #408 branch against main, extend with #409/#410/#411 + wire against the new SDK verbs as #662/#541 land
- No new issue needed — #408 IS the tracker for this work

### B14 (cmd/devseed inline rebuild, 2026-05-18)

**Decision**: Wrote `cmd/devseed/main.go` inline (~80 lines) rather than dispatching or skipping MCP, because the seed pipeline was missing entirely (referenced in plan-04 but never committed to rpg-api). Used tkenc.New + AddPlayer + AddMonster + ToData. Confirmed working: seeded `enc:v2:dev-encounter` (6356 bytes), harness reconnects, 3 entities + 127 hexes visible, SetReactionReady RPC verified 200 OK + Redis persistence.

Side effect: live-reproduced #410 (panel shows "unready" while redis says `OA: true` because of default-to-false fallback). Screenshot at `/tmp/wave-2-11d-mcp-readiness-panel.png`.

`cmd/devseed/main.go` is uncommitted, lives in main checkout. Ship via Stream 2 below.

### B15 (Trying parallel team-member dispatch, 2026-05-18)

**Decision**: Switch from sequential single-implementer-agent to parallel team-member dispatch. Two streams to start:

- **Stream 1 — rpg-dnd5e-web-member**: ship #409 + #410 + #411 on the existing `feat/wave-2.11d-reaction-ui-407` branch. Goal: extend PR #408 with the three follow-ups, kill the default-to-false readiness bug (live MCP repro evidence + screenshot from B14), wire stream→reducer for InputRequiredDelivered, add harness test coverage. Makes #408 mergeable end-to-end.

- **Stream 2 — rpg-api/character-content team-member**: ship `cmd/devseed` as a small PR with real characters wired (alice-rogue with SneakAttack + finesse weapon, bob-barbarian with Rage + greataxe, wendy-wizard with Shield prepared + level-1 spell slot). Characters persisted under `character:*` redis keys; encounter player entries reference them. Plus: seed should support initial_mode = TURN_BASED for combat-mode playtest verification.

**Why now**: B-call cluster today (B8/B10/B12) all surfaced through sequential verification. The bottleneck wasn't bad signals — it was that all verification streams converged on one director critical-path. Two parallel streams test whether the cabinet pattern (per `feedback_teams_as_cabinet` role docs) actually delivers velocity without losing visibility.

**Risks acknowledged**:
- Coordination drift (briefs include explicit contract surfaces + integration points)
- Scope creep mid-stream (4-question verification gate + STOP-and-report discipline in role prompts)
- Me as coordination bottleneck (if coordination cost exceeds parallel-velocity win, signal is too-fine-grained streams, recombine)

**Held back from this experiment**:
- Toolkit team-member on Wave 2.11e SDK extensions (#658 MovementResolver + #662 CompleteTakeAction symmetry). Started small with 2 streams not 3. Verification discipline is highest-stakes on SDK design; want to see how the parallel model behaves before adding the trickiest stream.

**What this affects**:
- Two team-members dispatched in parallel via Agent tool
- Each surfaces decisions via SendMessage to me; I make B-calls
- Kirk gets PR merge-gates + active.md log updates only
- If model works: spin up toolkit team-member next session

### B16 (web team-member pushback caught a brief error, 2026-05-19)

**Self-correction**: dispatched web team-member to `git rebase origin/main` for PR #412's conflict resolution. Workspace CLAUDE.md rule: "ALWAYS use merge instead of rebase for feature branches — Rebasing can silently lose code changes — Merge preserves complete history. Only rebase if branch was created TODAY and no other work has been merged." The #412 branch WAS from today but #408 had merged since the branch point, so the exception didn't apply.

Team-member followed the brief but flagged the rule conflict explicitly in their report pushback section. Result is good (clean integration, all tests pass) but cost the linear commit history #408+#412 would have had via merge.

**Lesson for future**: when a branch needs to incorporate other-merged-since-branch-point main, default to `git merge origin/main` not rebase. Even when the conflict resolution looks identical, merge preserves the explicit dual-parent history that makes "what came from which PR" greppable post-hoc. Rebase rewrites that history into a single linear commit.

**Process check that worked**: team-member's pushback discipline caught the brief-vs-rule conflict and surfaced it explicitly. Verification gate working in BOTH directions — team-member checks the director, director can self-correct. The 4-question gate's Pushback question doesn't just mean "verify Copilot's claims" — it also means "verify the director's instructions against standing rules."

### B17 (Docs restructure + toolkit-as-product framing, 2026-05-19)

**Kirk's signal**: "if we want these docs to stay current with the code they need to be in the same repo so a PR can be checked that the docs are updated with any changes we are bringing in." Plus the strategic framing: "rpg-api (Game Server) should be a data orchestrator and lean on the toolkit for everything. our goal there is the game server should be simple to implement. anyone using the toolkit for their game would spend their time in the UI making the game they want and not worry about the game engine or how it is delivered."

**Decision**: Rewrite `rpg-project/docs/architecture/system.md` with toolkit-as-product as the lead framing. Move per-repo content out — keep only cross-repo content (boundary rule, 4-repo topology, what crosses boundaries, vocabulary, doc-freshness ownership table). Per-repo overview.md + component maps are owned by each repo and enforced by PR review. Per-wave sequence diagrams stay in rpg-project (inherently cross-repo).

**Why this matters strategically**: rpg-toolkit IS the product. SDK design calls (especially Wave 2.11e's MovementResolver + CompleteTakeAction symmetry) should be evaluated through "how would another game host use this?" not just "what does rpg-api need today?" When rpg-api grows complex, that's a signal the toolkit is missing something. The event-bus + conditions pattern is the canonical heavy-lifting shape — orchestrator just `Apply()`s; rule logic lives in the subscribers.

**Saved as memories**:
- `project_toolkit_as_product` — captures Kirk's strategic framing in his own words; informs every future architectural call
- `feedback_toolkit_docs_close_the_loop` strengthened with explicit "where each doc type lives" table — per-repo content goes in the repo (PR review enforces), cross-repo content stays in rpg-project (close-the-loop chore enforces)

**Per-repo overview.md additions deferred to Wave 2.11e**: each team-member (toolkit, api, web) adds `<repo>/docs/architecture/overview.md` as part of their first Wave 2.11e PR per the new close-the-loop discipline. system.md links to those overview.md docs as the authoritative per-repo source. Don't dispatch 3 team-members for docs-only PRs now; bake the work into the next-wave dispatches where the toolkit overview informs the SDK extension design.

**Status**: 
- ✅ system.md rewritten (cross-repo content only + toolkit-as-product lead framing)
- ✅ `project_toolkit_as_product` memory saved + indexed
- ✅ `feedback_toolkit_docs_close_the_loop` strengthened with where-each-doc-type-lives table
- ⏭️ Per-repo overview.md additions queued for Wave 2.11e dispatches

### Pattern note for end-of-session retro

Three structural toolkit gaps in one session, each caught by verification before shipping a false "wave goal met" claim:
1. Encounter.Move bypasses MovementChain (B8)
2. LoadFromData drops SpellSlots (B10, fixed)
3. CompleteTakeAction expects player-attacker (B12)

All three are "Wave 2.11d's reaction surface is the first feature to exercise these paths; the assumptions held until now." Each was discoverable only via end-to-end integration testing OR deliberate verification probes, not by lower-layer tests.

**Retro framing for Kirk**: process-success at the verification level (we caught them); process-failure at the planning level (Wave 2.11d's scope didn't account for what the SDK needed to add). The fix going forward is what we already changed: the four-question verification gate + close-the-loop platform-docs step + "STOP and report" discipline. These caught the gaps we'd have shipped silently otherwise.

### Decisions pending review at session end

- B12 outcome (closeout ships with narrow scope, 2.11e issues filed, web + MCP playtest the narrow paths)
- Whether the `mechanics/conditions` pre-existing CI red on rpg-toolkit warrants a fix-it-now or close-the-loop item
- Whether the smell #657 (HOST CONTRACT serializer hook) needs Wave 2.11e placement vs deferred to "future polish"
- Whether to consolidate Wave 2.11e's now-4 issues (MovementResolver, CompleteTakeAction symmetry, multi-reactor aggregation, the 5 deferred integration tests across two repos) into a single tracker with sub-issues, or keep them flat on the board
- The bigger conversation: was original Wave 2.11d scope too ambitious because we didn't audit SDK shape vs wave-goal needs upfront? Worth a process change for future waves — "pre-flight SDK audit" step before brief writeup.

### B18 (Wave 2.11e scope correction — Shield-shaped → OA-shaped, 2026-05-23)

**Decision**: Cancel rpg-api#541 (Shield wire-up). Refocus Wave 2.11e on OA-shaped goal (toolkit#658 MovementResolver + rpg-api#539 wiring + player-OA integration tests). Shield infrastructure stays at SDK level only (#664 shipped) until a future spells project picks it up.

**Trigger**: Implementer's check-in on #541 surfaced "spell-slot consumption assertion" as Q1 — which prompted Kirk to step back and note that the actual playable-goal target is rogue/fighter/barbarian/monk playable. No spells until a future project. Shield was an architectural-verification target in 2.11d (proving SDK could handle slot-cost reactions with pause-and-resume), not a player feature we intended to ship.

**Why the catch was clean**: implementer hadn't started writing code yet — was waiting on the 3-question director sign-off. Zero rework cost.

**Concrete actions taken**:
- rpg-api#541 closed with "deferred to future spells project" rationale; board → Done (cancelled)
- rpg-project#50 (Wave 2.11e umbrella) updated with revised goal sentence + sub-issue status; board → In Progress
- toolkit#658 (MovementResolver SDK design) brief posted; goal-shaped sequencing now drives the wave
- toolkit#662 moved to Done on board (PR #664 merged earlier this session)

**Process lesson for next pre-wave audit**:
The pre-wave SDK audit step needs to also ask **"is this scope feature-shipping or architecture-verification?"** — they have very different completion definitions. Architecture verification ends when the SDK proves the shape; feature-shipping ends when the player sees it. Conflating the two scopes a wave with work toward the wrong goal.

**Bigger framing pivot Kirk surfaced same session**:
Rolling-wave planning. Stop trying to plan all of 2.11e + 2.12 + ... upfront. Plan next wave only; adjust based on lessons from current wave. Director duo (Kirk + me) becomes the planning function; implementers + team-members + Copilot handle execution. Cabinet pattern crystallizes.

Also open: new product-shaped project board ("Path to Playable" or similar) with stepping stones toward 4-class playable. Currently using #11 "Chapter 1: Architecture Honesty" which is initiative-shaped. Pending Kirk's call on whether to spin up new board now or wait until current wave closes.

**Status**: Wave 2.11e pivot in flight. Toolkit#658 brief posted; implementer should pick it up on next loop and surface design questions.

### B19 (Wave 2.11e under-spec catch — symmetric NPC-movement OA gap, 2026-05-24)

**Decision**: Ship PR #667 (MovementResolver SDK, player-movement direction) after one fix-in-PR (delete dead `MovementStepResult.Triggers` field). File new Wave 2.11e issue toolkit#668 for the mirror direction (NPC-initiated movement → player OA against fleeing monster). Refine #50 goal sentence to enumerate both movement directions.

**The catch**: Team-member review on #667 surfaced that `Encounter.applyNPCMovement` bypasses MovementResolver entirely. The PR fixes player-moves → enemy-OA but does NOT fix the symmetric enemy-moves → player-OA. Same B8 shape, mirror direction. Goal-shaped for all 4 classes (fighter/barbarian/monk/rogue all OA fleeing enemies in actual play).

**Root cause**: My pre-wave goal sentence on #50 said "player retreats triggering enemy OA" — singular movement direction. Under-specified. The implementer correctly interpreted "(b) NPC-OA-only scope" as the reactor-type partition (not the player-pause branch from Q1), but had no signal to extend the resolver to NPC movement.

**Process learning**: Pre-wave goal sentences need to enumerate **movement direction** (and any other dimension that asymmetric code paths could miss), not just reactor type. Going forward, the wave-goal-sentence draft should ask: "are there mirror directions of this behavior that share the same B-call gap?"

**Not adding a hard rule** — multi-layer review (team-member + cross-cutting + Copilot) caught it before merge. Same model caught the Shield scope overshoot (#541 cancel). The discipline is working without baking new prompts. The catch happened on the SECOND wave-goal scoping mistake of 2.11e; we should expect this pattern to keep catching them.

**Concrete actions taken**:
- Posted ship-after-one-fix director comment on #667
- Filed toolkit#668 (Wave 2.11e, status Todo, board updated)
- Filed toolkit#669 (low-pri error-mid-path divergence, no wave, board updated)
- Updated #50 with refined two-direction goal sentence + new sub-issue list
- Implementer should pick up the dead-field fix on next loop; then #667 merges; then #668/#666 sequence into rpg-api#539's wiring

**Bigger framing reinforcement**: this is the third catch this wave (Shield scope, MovementResolver Q1=b decision, NPC-movement gap). All three were caught by multi-layer review or director re-scoping mid-session. The rolling-wave + cabinet model is genuinely paying off — these would have shipped silently otherwise.
