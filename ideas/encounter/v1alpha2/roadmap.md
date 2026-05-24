---
name: v1alpha2 Encounter — Wave Roadmap
description: Goal-shaped wave sequence from movement (done) through v1alpha1 deletion. Future waves are sketched, not committed; only the current wave's inner issues exist on board #11.
---

# v1alpha2 Encounter — Wave Roadmap

This document is the **between-the-master-plan-and-the-board** layer. It names the wave sequence we believe will get us from where we are (movement working) to where the master plan ends (v1alpha1 deleted), without committing to inner-work shape for waves we haven't reached yet.

**Cross-references:**
- Master plan: `plan.md` (5-phase decomposition; phases written just-in-time)
- Design: `design.md` (the v1alpha2 contract itself)
- Chapter: board #11 — Architecture Honesty

## The principle

Per the master plan: **future waves' inner issues are not filed until the prior wave ships**. Wave 2.5 (movement) taught us things that would have been wrong in a Wave 2.6 issue written before 2.5 shipped (e.g. snapshot replay = rpg-api#497 only became visible during the 2-browser playtest). Each wave's issues are written *after* the prior wave's close-the-loop step extracts what we learned.

This roadmap describes **goal sentences** for future waves so the runway is visible. It does not describe how we'll get there. That's the prior-wave's close-the-loop output.

## Wave sequence

| Wave | Status | Goal sentence (verified by playtest) |
|---|---|---|
| **2.5** | ✅ Done (2026-05-08) | Two players in one encounter, one moves, both receive per-viewer-projected events; web renders without gating |
| **2.6** | ✅ Done (2026-05-08) | Player creates a v2 encounter and the snapshot arrives on connect (no hardcoded fallbacks) |
| **2.7** | ✅ Done (2026-05-09) | Player opens a door, room 2 contents render via v2 events on both browsers |
| **2.8** | ✅ Done (2026-05-09) | Player attacks monster under TURN_BASED, monster takes its turn back, attack outcomes project per viewer |
| **2.9** | ✅ Done (2026-05-09) | Player attempts a locked door; SubmitCheck resolves the skill-check prompt; door opens or stays closed; both browsers see the correct outcome |
| **2.10** | ✅ Done (2026-05-09) | Player attacks monster; HP hits 0; monster dies and is removed from initiative + visible state; once last hostile dies, encounter ends per viewer |
| **2.11a** | ✅ Done (2026-05-09) | CombatResolver injection seam: encounter SDK exposes `CombatResolver` interface; rpg-api wires `StandInCombatResolver` (mimics pre-2.11a math) at all 6 LoadFromData/New sites — prerequisite slice for 2.11b |
| **2.11b** | ✅ Done (2026-05-10) | rpg-api's `StandInCombatResolver` replaced with real `dnd5e.combat.ResolveAttack` chain (Combatant adapter + CombatantLookup); `Encounter.NPCAct` folded onto same `CombatResolver` (in-package `resolveAttack` deleted); weapon-equipping bridge verified end-to-end |
| **2.11c** | ✅ Done (2026-05-14) | Foundation + discrete attack phases: encounter-scoped event bus (conditions persist subscriptions across attacks), gamectx wired through `Dnd5eCombatResolver` (Sneak Attack / Protection / Shield can call `gamectx.RequireRoom` / `RequireCharacters` / `IsReactionReady` without crashing), broadened CharacterRegistry to encounter-scope, reaction-readiness map on the encounter, and `combat.ResolveAttack` split into `ResolveAttackHit` + `ApplyAttackOutcome` discrete operations + `ReactionTriggerEvent` published by condition handlers when predicate matches AND readiness is set. NO chain pause-resume primitive — chain runs end-to-end per phase. Also folds in the Wave 2.11a/b cleanup (per-attack bus, narrow registry) so the seam is correct from the moment reactions land. Verified by Sneak Attack integration test through v2 (UsedThisTurn persists across attacks via JSON round-trip + write-back, resets on TurnEnd) AND the discrete-phase split runs phase 1 + phase 2 inline correctly when no reactions are readied. |
| **2.11d** | ✅ Done (2026-05-19, scope-adjusted) | Orchestration plumbing (PhasedCombatResolver + TakeActionPhased SDK + Dnd5eCombatResolver impl), readiness toggle infrastructure (SetReactionReady RPC + per-character UI), Shield + OA condition predicate verification (toolkit unit tests + rpg-api `NotReady_NoPrompt` integration test), publish-before-save + single-reactor enforcement fixes, `cmd/devseed` real-character seed pipeline + interactive visual playtest harness. End-to-end Shield-fires-and-blocks + player-OA-on-move deferred to Wave 2.11e along with their SDK-extension prerequisites. **Three structural toolkit gaps discovered via verification gate during closeout** (Encounter.Move bypasses MovementChain; LoadFromData drops SpellSlots — fixed in scope as rpg-toolkit#660; CompleteTakeAction expects player-attacker). Two deferred to 2.11e as the cleanest narrative shape. |
| **2.11e** | 🟡 Active | **Complete the reactions**: MovementResolver SDK interface so OA fires on player movement (rpg-toolkit#658 + rpg-api#539); CompleteTakeAction NPC-attacker symmetry so Shield resume works in the only PvE direction Shield can fire (rpg-toolkit#662 + rpg-api#541, includes 2 deferred Shield integration tests); multi-reactor aggregation when 2nd post-hit reaction ships (rpg-api#540); broaden Dnd5eCombatResolver gamectx registry to include ability scores for Unarmored Defense / Martial Arts (rpg-api#533). Verified by MCP playtest: wizard with Shield ready blocks an NPC attack via the prompt+resume flow; player retreats past adjacent NPC who fires OA; multi-character combat reaches the no-hostiles end condition. |
| **2.11f** | Proposed | `AvailableAction` enumeration + `TurnState.available_actions` emission + class-action surfacing in the harness (monk Flurry of Blows, fighter fighting-style indicator). Decoupled from 2.11d/e so reactions can ship without dragging the class-action surface scope along. |
| **2.12** | Sketched | Dungeon flow: multi-room, doors as room transitions ("encounter ended" unlocks next room) |
| **2.13** | Sketched | Party scaling to 4 players in a single encounter |
| **5** (existing) | Sketched | Real LobbyView talks v2 exclusively for migrated verbs; `/playtest` harness gone |
| **Final** | Sketched | `grep "v1alpha1.EncounterService"` returns nothing across rpg-api-protos / rpg-api / rpg-dnd5e-web |

Neighboring waves may merge based on what we learn. Two examples:
- 2.6+2.7 considered for merger; kept separate because OpenDoor's verb shape exercises a distinct (geometry) event path that's worth verifying on its own.
- **2.8+2.9 merged** (2026-05-08): the original 2.8 (TURN_BASED foundation) and 2.9 (Attack) collapsed into a single combat-slice wave. Splitting created an ungrounded 2.8 milestone (mode flips and turns cycle, but nothing happens on a turn) and a 2.9 wave that couldn't actually run because TURN_BASED hadn't shipped. The merged wave verifies the full attack → monster-turn loop end-to-end. Wave 2.10 (Prompts) renumbers to 2.9.

## The close-the-loop pattern

Each wave's **final issue** is `chore: wave-N close-the-loop`. It is dispatched after the wave's playable goal is verified, and it produces the next wave's runway.

**Standard close-the-loop done-when:**

1. **Stale issues closed.** Every wave-N issue on the board reflects reality: shipped → closed with a comment pointing at the merging PR; superseded → closed with a pointer to the replacement.
2. **Followups filed.** Any "filed in PR description but not yet a real issue" item from the wave is filed and placed on board #11 with the right wave (this wave's slice 3 followups, or a future wave's bucket).
3. **Board reflects reality.** Status field on every wave-N item is `Done` or moved to the wave it really belongs to. No item is `Todo` if its work is shipped.
4. **Next wave's plan drafted.** A short plan in `rpg-project/ideas/encounter/v1alpha2/plans/0X-<wave-name>.md` describing inner-work shape for the next wave. This is what the next wave's first dispatch reads.
5. **Roadmap updated.** This file's table is updated: this wave → Done; next wave → Active.
6. **Affected role context dirs updated.** Each role that contributed to this wave gets new entries appended to `docs/teams/roles/<role>/context/{patterns,discoveries,dependencies}.json` capturing what fresh agents in that role would need to know to pick up the next dispatch cold. This is the **forward-loading** step: the role context dirs are what fresh agents read at boot, separately from any specific issue. New patterns shipped this wave belong here so the next dispatch doesn't have to rediscover them.
7. **Platform docs updated.** Each affected repo's platform docs reflect what shipped — landed in the WAVE PR(s), not deferred to a cleanup PR:
   - `rpg-toolkit/docs/status.md` — active work, paused items, per-subsystem confidence
   - `rpg-toolkit/docs/quality.md` — A-D scorecard with rationale per affected component
   - `rpg-toolkit/docs/architecture/components/<module>.md` — new verbs, event types, persistence shapes, HOST CONTRACT-style assumptions
   - Equivalent rpg-api docs once they exist
   Per `feedback_toolkit_docs_close_the_loop`: stale platform docs = future agents reading SDK cold and missing decided architecture. The Pattern question in the verification gate doubles as the doc-cite check.
8. **Retro on the wave's tracker issue.** Comment summarizing what shipped, what was learned, what's deferred. Three bullets each.

The close-the-loop issue is **done in director conversation (Kirk + the main session)**, not dispatched as a fresh agent. The work is judgmental — deciding which findings are load-bearing for future agents, which followups merit their own issue vs a comment, which role-context entries are validated vs anticipated. Those calls benefit from the conversational shape; producers don't need it.

The split that emerged:

- **Producer dispatches** (fresh feature-manager agents per wave): wave package production. Tracker, inner issues, plan doc, roadmap update, anticipated role-context entries, board placement. Mostly mechanical given a clear brief.
- **Close-the-loop in this conversation**: validate or correct anticipated patterns; decide what shipped that role-context should capture as durable; decide whether playtest findings warrant followup issues; draft the next wave's plan from retro insights. Director judgment, not production.

The close-the-loop issue stays on the board as the wave's terminal tracker (so done-when state is visible), but the work behind it lives in conversation, not in an agent dispatch.

### Forward-loading split

The close-the-loop step institutionalizes the principle that load-bearing context goes in two places, by shape:

- **Role-shaped knowledge** (durable across all dispatches in that role) → role context dirs (`docs/teams/roles/<role>/context/*.json`). Examples: file layout conventions, broker-pattern shape, translator typed errors, bufconn test fixture, status-code mapping, dependency pseudo-versions. Read once when the agent boots into the role.
- **Task-shaped knowledge** (load-bearing for one issue) → issue body. Examples: exact file to create, the specific proto request shape to translate, which test suite to extend, which other in-flight issue to coordinate the projectFor helper with. Read cold when the agent picks up the issue.

If a fresh agent has to read 5 link-hopped docs before they can start work, the forward-loading is incomplete. Each wave's close-the-loop fixes whatever gap that wave's execution surfaced.

## Wave 2.6 — Encounter lifecycle ▶ Snapshot

**Goal:** Player creates a v2 encounter and the snapshot arrives on connect (no hardcoded fallbacks).

**Why now:** Slice 1 of wave 2.5 left `SnapshotDelivered.encounter` empty by design and required hardcoded fallback positions in the playtest harness. Wave 2.6 makes a v2 encounter come into being honestly: created via RPC, retrievable via RPC, snapshot replays on connect so the harness's fallbacks become deletable (slice 3 unblock).

**Inner issues** (filed on board #11 with Wave 2.6 + Status Todo):

- 🎮 Tracker: rpg-project — wave goal sentence + done-when
- rpg-api: implement `CreateEncounter` v2
- rpg-api: implement `GetEncounter` v2
- rpg-api#497: replay initial state events at connect (filed; moved from Wave 2.5)
- chore: Wave 2.6 close-the-loop

**Out of scope for 2.6:**
- Persistence beyond in-memory (master plan §"What will not be done")
- Other RPCs (OpenDoor, Attack, EndTurn, etc. — those are 2.7+)
- Web-side migration of LobbyView lifecycle code (Wave 5 territory)

## Future waves (sketched only)

The following descriptions are intent, not commitment. Inner shape is decided at each wave's start.

## Wave 2.7 — OpenDoor

**Goal:** Player opens a door, room 2 contents render via v2 events on both browsers.

**Why now:** Wave 2.5 proved the per-viewer event projection works for movement (entity events). Wave 2.6 made the encounter lifecycle honest (real RPC creation, snapshot replay). Wave 2.7 extends the per-viewer projection to a *new verb shape* — geometry events — using the same `Interact` RPC that future waves (2.10) will further extend for chests/levers/traps. It's the smallest meaningful extension of the contract that proves the Interact path works.

**Toolkit work: none.** `Encounter.OpenDoor()`, `Encounter.AddDoor()`, `events.DoorOpenedEvent`, and `perception.ProjectDoorOpen` all exist (shipped with the encounter SDK). Wave 2.7 is rpg-api handler + rpg-dnd5e-web consumer + harness extension only.

**Inner issues** (filed on board #11 with Wave 2.7 + Status Todo):

- 🎮 Tracker: rpg-project — wave goal sentence + done-when
- rpg-api: implement `Interact` RPC for door interactions
- rpg-dnd5e-web: consume `DoorOpened` event in v2 stream + reducer; harness button to call Interact
- chore: Wave 2.7 close-the-loop

**Out of scope for 2.7:**
- `DoorClosed` / closing doors (the wave goal verifies open; close is cheap to add later but not required)
- Locked doors / `InputRequired{skill_check}` (Wave 2.10 — that's the SubmitCheck wave)
- Other Interact targets — chest, lever, NPC, trap (Wave 2.10)
- Web LobbyView migration (Wave 5 territory)

### Wave 2.8 — Combat slice (Player attacks monster, monster attacks back)

**Goal sentence:** Player attacks monster under TURN_BASED, monster takes its turn back, attack outcomes project per viewer.

**Likely shape:** Originally two waves (2.8 TURN_BASED foundation, 2.9 Attack), merged into a single combat-slice wave on 2026-05-08. The toolkit gets monster + mode + turn state on `encounter.Data`, a new `Encounter.NPCAct(npcID)` verb wrapping `monster.TakeTurn`, new combat events (`AttackResolvedEvent`, `DamageDealtEvent`, `ConditionAppliedEvent`, `ModeChangedEvent`, `TurnStartedEvent`, `TurnEndedEvent`). rpg-api implements `TakeAction(attack)` + `EndTurn` handlers with mode-gating and an NPC dispatch loop after EndTurn cycles to an NPC. Web adds combat reducers (HP, status, mode, active-actor), `useTakeActionV2` + `useEndTurnV2` hooks, and harness combat controls. Toolkit work is REQUIRED — monster AI lives in toolkit, not rpg-api. See `plans/06-wave-2.8-combat-slice.md`.

### Wave 2.9 — Interact and SubmitCheck (renumbered from 2.10)

**Goal sentence:** Player triggers a trap or interacts with a mimic; SubmitCheck resolves the prompt.

**Likely shape:** Trap-trigger via `Interact`, prompt model on the encounter, `SubmitCheck` RPC validates against pending prompt, prompt resolution event back to the stream. Lower-frequency verbs but completes the contract.

### Wave 5 — Web cutover (existing on board)

**Goal sentence:** Real LobbyView talks v2 exclusively for the verbs that have shipped; `/playtest` harness deleted.

**Likely shape:** Replace `LobbyView`'s v1 hooks with v2 equivalents, delete the harness (slice 3 of wave 2.5 contributes the movement-specific deletion), reduce v1 surface to only-not-yet-migrated verbs. Web no longer needs the harness as a verification path.

### Wave Final — v1alpha1 deletion

**Goal sentence:** `grep "v1alpha1.EncounterService"` returns nothing across rpg-api-protos / rpg-api / rpg-dnd5e-web.

**Likely shape:** Final cleanup — delete `dnd5e/api/v1alpha1/encounter.proto`, delete the v1alpha1 service registration in rpg-api, delete remaining v1alpha1 imports in web. This wave can only run when *every* verb has migrated through 2.7–2.10 and Wave 5 has cut the web over.

## What changes between waves

After each wave's close-the-loop:
- This file's table updates
- Next wave's plan exists in `plans/`
- Next wave's inner issues exist on board #11
- A short retro lives on the wave's tracker issue
