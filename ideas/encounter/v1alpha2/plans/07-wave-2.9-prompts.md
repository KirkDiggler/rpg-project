# Wave 2.9 — Prompts (Locked door / SubmitCheck resolves the prompt)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Each inner issue is its own dispatch; this plan is the wave-shape that ties them together.

**Wave goal:** Player attempts a locked door; server returns `InputRequired{skill_check}` on the Interact response; player submits a roll via `SubmitCheck`; server resolves the prompt; door opens or stays closed; both browsers see the correct outcome.

**Verified by playtest:** alice clicks the open-door button on a *locked* door in `/playtest`; server returns `InteractResponse{input_required: {skill_check: {dc, ability: "DEX", tool: "thieves-tools"}}}`; alice's harness renders a prompt modal with the DC + a "submit roll" input; alice submits a d20 roll via `SubmitCheck`; server validates the pending prompt, computes total, returns `SubmitCheckResponse{success: bool, total: int}`; if success, server emits `DoorOpened` + `GeometryRevealed` per viewer (same projection path Wave 2.7 validated); if failure, the door stays closed and no geometry is revealed; both browsers reflect the outcome consistently.

**Depends on:**
- Wave 2.5 (per-viewer projection, broker, translator, v2 service registration) — shipped.
- Wave 2.6 (CreateEncounter, GetEncounter, snapshot replay) — shipped under rpg-project#19.
- Wave 2.7 (Interact RPC for unlocked doors, `DoorOpened` event consumer in web, `useInteractV2` hook, harness verb-button-with-target pattern) — shipped. Wave 2.9 extends Interact's response shape (`InputRequired`) and adds the parallel `SubmitCheck` resolution path.
- Wave 2.8 (TURN_BASED, TakeAction/EndTurn, combat events) — shipped. Not strictly blocking, but its `pat-v2-combat-event-translator` and per-viewer projection patterns inform how prompts integrate (caller-private response payload vs broadcast events).

## Why this wave exists

Wave 2.7 implemented `Interact` for the unlocked-door case and explicitly deferred locked doors (`Out of scope for 2.7`: "Locked doors / `InputRequired{skill_check}` (Wave 2.10 — that's the SubmitCheck wave)"). Wave 2.8 implemented combat verbs and explicitly deferred `SubmitCheck / InputRequired` (`Out of scope for 2.8`: "SubmitCheck / InputRequired. Wave 2.10 (renumbered Wave 2.9 after this merge)").

The contract has carried `InputRequired` and `SubmitCheck` since Phase 1 (`types.proto:284-317`, `service.proto:104-115`), but no RPC has populated them and no client has consumed them. Wave 2.9 closes that gap: it makes the caller-private follow-up payload real on the wire, exercises the server's pending-prompt-per-player tracking, and proves the web-side prompt-modal reducer works end-to-end.

This is a *narrow* wave by design — one verb shape (skill check), one trigger surface (locked door via Interact), one resolution RPC (SubmitCheck). DialogueChoice and TargetSelect ride the same `InputRequired` oneof but ship in later waves once the skill-check path validates the pending-prompt machinery.

## Why "after combat, before X"

The roadmap places Prompts after the combat slice because:

- **After 2.8 (combat).** Combat shipped first because it exercises the broadest contract surface (mode flips, turn cycling, NPC dispatch, multiple new event types). Once combat is grounded, prompts are a focused addition rather than a parallel uncertainty. Also: the playtest goal "monster attacks back" is the more ambitious behavior verification; prompts is a smaller delta from there.
- **Before web cutover (Wave 5).** The `/playtest` harness is the verification path for prompts; LobbyView's prompt UI can be designed against the validated harness pattern rather than co-evolving. Migration is structurally simpler when every v2 verb has shipped at least one playable surface.
- **Before v1alpha1 deletion (Wave Final).** `SubmitCheck` is the last RPC in the v1alpha2 service surface (`service.proto:115`). Once it ships and Interact's prompt path is exercised, every v1alpha2 RPC has a real implementation; v1alpha1 has no exclusive contract surface left.

## Architectural calls already made (do not relitigate)

- **Pending-prompt-per-player is server-side state, not on the wire.** Per `design.md` §4 ("InputRequired correlation"): "A player can have at most one pending prompt at a time... `SubmitCheck` implicitly resolves that prompt; if none is pending, the server returns `FailedPrecondition`. No correlation id on the wire — the oneof in `InputRequired` tells the client what UI to render; the server's pending-state tracking handles demux." Don't add a `prompt_id` field to the proto.
- **InputRequired rides on `InteractResponse` and `TakeActionResponse`.** Already in the contract (`service.proto:67`, `service.proto:80`). Wave 2.9 populates those fields; it does NOT add a third RPC that emits prompts.
- **Locked-door path is via Interact, not a new verb.** `Interact(lockedDoorId)` returns `InputRequired{skill_check}`; there is no `AttemptUnlock` RPC. The Interact handler dispatches per target state.
- **Prompt resolution does NOT emit the prompt itself as a stream event.** Prompts are caller-private (only the caller sees the DC, the ability requested, the tool needed). They flow on the response, not the stream. The *outcome* (door opens, trap disarms) flows as normal world events on the stream and projects per viewer.
- **`SubmitCheck` is the resolution verb for ALL prompt kinds.** SkillCheckPrompt, DialogueChoice, and TargetSelect all resolve via `SubmitCheck` (or its near-equivalents). Wave 2.9 ships SkillCheckPrompt only; the RPC shape is general enough that future waves wire the other two without proto changes (or with minimal additive ones — see `Out of scope` for what gets deferred).
- **Locked-door state lives in the toolkit's `DoorData`.** If the toolkit's `encounter.Data.Doors` doesn't yet model "locked" + DC + ability + tool, that's a toolkit gap to fill — not a rpg-api concern. The handler reads door state from the toolkit; it does not invent locked-door semantics.

## Inner-work shape

### rpg-api-protos — confirm InputRequired/SkillCheckPrompt fields ship on TakeActionResponse + InteractResponse

The proto contract already has the shape. This wave's proto issue is a **verification-and-tagging** task, not a redesign:

1. **Verify** `types.proto` (lines ~284-317) defines `InputRequired`, `SkillCheckPrompt`, `DialogueChoice`, `TargetSelect`, `DialogueOption`, exactly as design §4 requires. Confirm field numbers are stable from v0.1.94 onward (no churn since Phase 1).
2. **Verify** `InteractResponse.input_required` (`service.proto:67`) and `TakeActionResponse.input_required` (`service.proto:80`) are `optional` and use the correct field number.
3. **Verify** `SubmitCheckRequest` (`service.proto:104`) carries `encounter_id`, `entity_id`, `roll`. Verify `SubmitCheckResponse` carries `success` + `total`.
4. **If gaps surface during web/api implementation** (e.g., the web needs an `attempted_action_kind` to render different prompt headers — "unlock door" vs "disarm trap"), file additive proto changes as separate issues; do NOT bundle them here. The wave's proto issue is the verification baseline.
5. **Tag a release** if any additive change ships (likely none for Wave 2.9 — the contract was forward-loaded). If no proto changes are needed, the issue closes with "verified, no proto changes for Wave 2.9; pinned versions are sufficient" and a comment on the rpg-api issue confirming the version pin.

The proto-side risk is low. The intentional design choice in Phase 1 was to ship the prompt machinery in the contract from day 1 so that wave-by-wave additions don't need proto bumps. Wave 2.9 is the first validation of that bet.

### rpg-api — SubmitCheck RPC + prompt-state tracking + InputRequired follow-up payload on Interact/TakeAction responses

This is the largest of the three slices. Sub-tasks:

1. **Pending-prompt-per-player state on encounter.**
   - Add `pendingPrompts map[core.PlayerID]*PendingPrompt` to the encounter's repository data shape (could live on `encounter.Data` if the toolkit owns it, or in rpg-api's repository wrapper if the toolkit doesn't model prompts). **Decision deferred to dispatch:** does the toolkit own pending-prompt state, or does rpg-api? Argument for toolkit: prompts are encounter-scoped game state; the toolkit is the single source of truth. Argument for rpg-api: prompts are an API/UX concern (the toolkit doesn't care about wire-shape demux). Pick at execution time; document the choice in PR description and update Wave 2.9 close-the-loop role-context entries accordingly.
   - `PendingPrompt` carries: `kind` (skill_check / dialogue / target_select discriminant), `dc int`, `ability string`, `tool *core.Ref`, `triggeredBy core.EntityID` (the door/trap/NPC that issued the prompt), `triggeredAction string` (e.g. "open" — what to do if the check passes).
   - Cleared on successful `SubmitCheck` resolution. Cleared on encounter teardown.

2. **Locked-door path in Interact handler.**
   - Extend `internal/handlers/dnd5e/v2/encounter/interact.go` (file shipped in Wave 2.7). When `target_entity_id` resolves to a `DoorData` with `Locked: true`:
     - Validate caller authority (already done in 2.7).
     - Construct `PendingPrompt{kind: skill_check, dc: door.LockDC, ability: "DEX", tool: ref("dnd5e:item:thieves-tools"), triggeredBy: doorID, triggeredAction: "open"}`.
     - Set encounter's `pendingPrompts[playerID]` (overwriting any prior — server enforces "at most one"; if a prior prompt exists, design choice is to overwrite silently per `design.md §4` "the player can't fire two RPCs concurrently" — but we should error with `FailedPrecondition` if a prompt is already pending and the caller tries a different verb).
     - Save encounter data.
     - Return `InteractResponse{input_required: {skill_check: {dc, ability, tool}}}`. Do NOT emit any stream event for the prompt itself (caller-private).
   - When `target_entity_id` resolves to an unlocked door, behavior is unchanged from Wave 2.7.

3. **`SubmitCheck` RPC handler.**
   - New file `internal/handlers/dnd5e/v2/encounter/submit_check.go`:

   ```go
   func (h *Handler) SubmitCheck(ctx context.Context, req *encounterv2pb.SubmitCheckRequest) (*encounterv2pb.SubmitCheckResponse, error)
   ```

   Required behavior:
   - Auth: `auth.GetPlayerID(ctx)` → empty returns `Unauthenticated`.
   - Validation: `EncounterId`/`EntityId` empty → `InvalidArgument`. `Roll` outside [1, 20] → `InvalidArgument`.
   - Load encounter. Missing → `NotFound`.
   - Look up `pendingPrompts[playerID]`. If absent → `FailedPrecondition` ("no pending prompt for caller") per design §4.
   - Compute `total = roll + ability_modifier(playerID, prompt.ability) + tool_proficiency_bonus(playerID, prompt.tool)`. The toolkit owns the modifier math (e.g., `dnd5e/character.AbilityModifier`); rpg-api orchestrates the call.
   - `success := total >= prompt.dc`.
   - **If success:** dispatch the prompt's `triggeredAction` against `triggeredBy`. For Wave 2.9, only "open" on a door is wired: call `enc.OpenDoor(playerID, prompt.triggeredBy)`. Toolkit emits `DoorOpenedEvent` + `HexRevealedEvent` per viewer (same path as Wave 2.7). Save.
   - **If failure:** clear the prompt; emit no events (the door stays locked silently — narration is web-side from the response). Future waves may add a `CheckFailed` event for trap-trigger-on-fail consequences.
   - Clear `pendingPrompts[playerID]` regardless of outcome.
   - Save encounter data.
   - Return `SubmitCheckResponse{success, total}`.

4. **Translator additions.** None new for Wave 2.9 — the events emitted on success are already-mapped (`DoorOpenedEvent`, `HexRevealedEvent` from Wave 2.7). Verify the existing translator handles them correctly when emitted from the SubmitCheck path (should be path-independent — events publish through the broker the same way).

5. **Pre-existing-prompt collision handling.** If a player tries to call `Interact` (or any prompt-issuing RPC) while a prompt is already pending for them, return `FailedPrecondition` ("resolve pending prompt first"). This matches the `design.md §4` "at most one pending prompt" invariant. The server enforces it; the web doesn't gate (per `feedback_no_logic_in_web`).

6. **Tests.**
   - Unit tests for SubmitCheck handler (no pending prompt → `FailedPrecondition`; success path triggers door open; failure path clears prompt without emitting events; modifier math correct via toolkit).
   - Unit test for Interact-on-locked-door (prompt set on encounter; response carries `InputRequired{skill_check}`; no stream events emitted for the prompt itself).
   - Unit test for prompt-collision (Interact-with-pending-prompt → `FailedPrecondition`).
   - Integration test extending `EncounterV2IntegrationSuite`: two-player + one-locked-door fixture; alice attempts open-door; alice receives `InputRequired{skill_check}` on InteractResponse; bob's stream sees nothing (prompts are caller-private); alice submits a passing roll via `SubmitCheck`; both alice and bob receive `DoorOpened` + `GeometryRevealed` per their PerceptionView; failure path tested separately (alice submits failing roll; bob still sees nothing; alice's SubmitCheckResponse reports failure).

### rpg-dnd5e-web — useSubmitCheckV2 hook + prompt-modal reducer + harness UI for prompt entry

1. **New RPC hook: `src/api/useSubmitCheckV2.ts`.**
   - Wraps `encounterClientV2.submitCheck(...)`. Mirrors `useMoveEntityV2.ts` / `useInteractV2.ts` shape per `v2-rpc-hook-pattern`.
   - Returns `{submitCheck, loading, error, lastResponse}` where `lastResponse: SubmitCheckResponse | null` (so the harness can render the success/total transiently after a roll).

2. **Prompt-modal state in `useEncounterState`.**
   - New reducer: `setPendingPrompt(prompt: InputRequired | null)`.
   - When `useInteractV2.interact()` (or `useTakeActionV2.takeAction()` once future waves wire prompts there) returns a response with `input_required` set, the caller invokes `setPendingPrompt(response.input_required)`. The hook does NOT auto-set the state; callers do (keeps the hooks pure).
   - When `useSubmitCheckV2.submitCheck()` resolves successfully (regardless of roll outcome), the caller invokes `setPendingPrompt(null)`.
   - Encounter state exposes `pendingPrompt: InputRequired | null` for components to render.

3. **Prompt modal component in `PlaytestHarness.tsx`.**
   - When `pendingPrompt !== null` and the kind is `skill_check`: render a small inline section (or a dialog) with: prompt header (e.g., "Skill check: DEX (DC {dc}), tool: thieves-tools"), a number input for the roll (1-20), a "submit" button calling `useSubmitCheckV2`.
   - Other prompt kinds (`dialogue`, `target_select`) render a "not yet supported" placeholder for Wave 2.9. They become real in later waves.
   - On submit success, render the result transiently (e.g., "rolled 14, total 17, success!") for ~2 seconds; clear the prompt state via `setPendingPrompt(null)`.
   - The harness must seed a locked door. Mirror Wave 2.7's seeding decision: extend `CreateEncounter`'s starter scenario to include a locked door (`locked-door-1`, `lockDC: 12`, `lockAbility: DEX`, `lockTool: thieves-tools`). Document the choice in PR description.

4. **Existing harness "Open door" section.**
   - When the player clicks the open-door button against a locked door, the `useInteractV2` response will now carry `input_required` instead of being empty. The harness must call `setPendingPrompt(response.input_required)` to surface the modal. This is a tiny additive change to the existing button's onClick handler.
   - Against an unlocked door, response.input_required is undefined; behavior unchanged from Wave 2.7.

5. **Tests.**
   - Reducer test: `setPendingPrompt` happy + clear path.
   - Hook test: `useSubmitCheckV2` happy + error paths.
   - Harness test: pending-prompt modal renders when state is set; submit button dispatches `useSubmitCheckV2` with the right payload; response clears the modal.
   - Dispatch tests unchanged (no new stream event types; events emitted on success are already-handled `DoorOpened` + `GeometryRevealed`).

### rpg-toolkit — out of scope (mostly)

The toolkit's `OpenDoor` and door-state model already exist (Wave 2.7). Wave 2.9 needs the toolkit to:

- **Model "locked" state on `DoorData`.** Add fields: `Locked bool`, `LockDC int`, `LockAbility string`, `LockTool *core.Ref`. If these don't yet exist in `encounter.DoorData`, add them in a small toolkit issue. Likely small; the door type in `data.go` just gains optional fields.
- **Optionally:** model pending-prompt state on `encounter.Data` (decision deferred per rpg-api section above; if rpg-api owns prompts, the toolkit doesn't need this).

If the toolkit modeling is genuinely small (locked-door fields only, no prompt state), file it as part of the rpg-api issue scope (the rpg-api implementer dispatches a small toolkit PR alongside). If pending-prompt state lives in the toolkit, file as a separate toolkit-member issue.

---

## Inner-issue list (to be filed when wave 2.9 kicks off — NOT filed in this PR)

Per Kirk's preference (file at wave kickoff, not at plan-draft time), these are listed for reference. The Wave 2.8 close-the-loop conversation files them when ready to dispatch.

- 🎮 **Tracker** (rpg-project): wave goal sentence + sign-off + contents checklist
- **rpg-api-protos**: confirm InputRequired/SkillCheckPrompt fields ship on TakeActionResponse + InteractResponse (verification + version pin; additive proto changes only if web/api surfaces a gap)
- **rpg-api**: SubmitCheck RPC + prompt-state tracking + InputRequired follow-up payload on Interact/TakeAction responses (handler + prompt-state model + integration test); may include a small toolkit PR for `DoorData` locked-state fields
- **rpg-dnd5e-web**: useSubmitCheckV2 hook + prompt-modal reducer + harness UI for prompt entry
- **chore: Wave 2.9 close-the-loop** (rpg-project): file followups, update board, draft Wave 5 plan (or Wave 2.10 if more contract gaps surface), update role context with prompt patterns

---

## Out of scope for Wave 2.9

- **DialogueChoice and TargetSelect resolution.** The `InputRequired` oneof carries all three kinds, but Wave 2.9 only wires SkillCheckPrompt. Dialogue and target-select ride the same machinery in later waves once the skill-check path validates the pending-prompt model.
- **Trap-trigger-on-failure.** If alice fails the lock-pick roll, the door stays locked and nothing else happens. Future waves may add trap consequences (e.g., failing to disarm a trap triggers it). Not in 2.9.
- **Multiple actions on the same prompt.** A prompt resolves on one `SubmitCheck`. No "try again" without re-issuing Interact. No advantage/disadvantage in Wave 2.9 (`SubmitCheckRequest` has only `roll` — advantage is a future additive proto field per `service.proto:108` "Future: optional advantage/disadvantage").
- **Prompt timeouts.** Pending prompts persist until resolved or the encounter ends. No auto-cancel after N seconds. Future polish.
- **Other Interact targets** (chest, lever, NPC, trap with detection). Same `InputRequired` shape extends naturally. Out of scope for 2.9 — locked door is the verification target.
- **Prompts during NPC turns.** NPC turns auto-resolve via `monster.TakeTurn`; no NPC ever gets prompted. The pending-prompt-per-player invariant is for player-controlled actors only.
- **Action history on the server.** A pending-prompt log (for replay or debug) isn't shipped. Server tracks current prompt only.
- **LobbyView prompt rendering.** Wave 5. Harness is the sole UI surface for prompts in Wave 2.9.

---

## Forward-loaded patterns (anticipated — refine in close-the-loop)

These are the new patterns Wave 2.9's execution is expected to surface. Marked `status: anticipated` until the wave's close-the-loop step verifies them against shipped reality.

**rpg-api-member additions:**

- `pat-v2-pending-prompt-state` *(anticipated)* — Pending-prompt-per-player is server-side state (not on the wire). Lives on encounter data (toolkit-owned or rpg-api-owned, decision per dispatch). Cleared on `SubmitCheck` resolution or encounter teardown. Server enforces "at most one prompt per player" via `FailedPrecondition` when a verb that would issue a prompt is called while one is already pending.
- `pat-v2-prompt-issuing-rpc-shape` *(anticipated)* — RPCs that may issue prompts (`Interact`, `TakeAction`) populate `InputRequired` on their response when the target's state requires a check. Prompts are caller-private — they ride the response, NOT the stream. The `InputRequired` oneof carries the prompt kind discriminant; the server's pending-prompt state handles demux on resolution (no correlation id on the wire).
- `pat-v2-submit-check-resolution-shape` *(anticipated)* — `SubmitCheck` is the universal resolution RPC for all prompt kinds (Wave 2.9 ships skill_check; future waves wire dialogue + target_select). Handler validates pending-prompt presence (`FailedPrecondition` if absent), computes total via toolkit-provided modifier math, dispatches the prompt's `triggeredAction` on success (e.g., `enc.OpenDoor` for a locked door), clears prompt regardless of outcome.

**rpg-dnd5e-web-member additions:**

- `pat-v2-pending-prompt-reducer` *(anticipated)* — `useEncounterState` exposes `pendingPrompt: InputRequired | null` set by callers when a verb's response carries `input_required`, cleared by callers when `SubmitCheck` resolves. The hooks themselves stay pure (don't auto-mutate state); callers wire the dispatch. UI conditionally renders the prompt modal based on pendingPrompt state.
- `pat-v2-prompt-modal-shape` *(anticipated)* — Prompt UI is one component that switches on `pendingPrompt.kind`. Each kind has its own input shape: skill_check needs a roll input + DC display; dialogue (future) needs option list; target_select (future) needs candidate-entity picker. All resolve through the same `useSubmitCheckV2` hook (or the hook learns variants).

**rpg-toolkit-member additions** (only if toolkit ends up owning prompt state):

- `pat-encounter-pending-prompt-state` *(anticipated, conditional)* — If the toolkit owns prompt state, `encounter.Data.PendingPrompts map[core.PlayerID]*PromptData` is the canonical source. Cleared on resolution via `Encounter.ResolvePrompt(playerID, success bool)`. If rpg-api owns the state instead, this pattern doesn't apply.

After execution, the close-the-loop dispatch promotes verified `anticipated` entries to status-less (validated) and adds any new patterns that surfaced.

---

## Execution sequence

1. **Wave 2.8 close-the-loop** must complete first. That step writes verified Wave 2.8 patterns into role context — Wave 2.9's dispatches read those.
2. **rpg-api-protos issue dispatches first** (verification only — likely closes within an hour with "no proto changes for Wave 2.9; pinned versions are sufficient"). If a gap surfaces, the additive proto change ships and tags before rpg-api work begins.
3. **rpg-api issue dispatches second.** Implements `SubmitCheck` handler, prompt-state model on encounter, locked-door path in Interact, integration test exercising the full lock → prompt → roll → outcome loop. May include a small toolkit PR for `DoorData` locked-state fields if needed.
4. **rpg-dnd5e-web issue dispatches third.** Adds `useSubmitCheckV2`, prompt-modal reducer, harness UI for prompt entry. Builds against the merged rpg-api version.
5. **End-to-end playtest** (cold start, both browsers, 2-tab pattern from `v2-playtest-harness`):
   - alice and bob connect to a freshly-created encounter with a locked door.
   - alice clicks open-door on the locked door; harness renders the prompt modal with DC + ability + tool; bob's stream shows nothing (caller-private).
   - alice rolls (in the harness, types a number) and submits; SubmitCheckResponse returns `success: true, total: N`.
   - Both alice and bob receive `DoorOpened` + `GeometryRevealed` per their PerceptionView.
   - Repeat with a failing roll: door stays locked, no events emitted, prompt clears.
6. **chore: Wave 2.9 close-the-loop** dispatches: files followups, updates board, drafts next wave plan (likely Wave 5 web cutover, or a Wave 2.10 if more contract gaps surface), captures verified patterns into role context, retros on tracker.

---

## Reference

- Wave 2.9 tracker: rpg-project#<filled in by close-the-loop dispatch>
- rpg-api-protos issue: <filled in by close-the-loop dispatch>
- rpg-api issue: <filled in by close-the-loop dispatch>
- rpg-dnd5e-web issue: <filled in by close-the-loop dispatch>
- Master plan: `rpg-project/ideas/encounter/v1alpha2/plan.md`
- Roadmap: `rpg-project/ideas/encounter/v1alpha2/roadmap.md`
- Design: `rpg-project/ideas/encounter/v1alpha2/design.md` §4 (Interact / TakeAction RPC shapes; InputRequired correlation note)
- Wave 2.7 plan (template): `rpg-project/ideas/encounter/v1alpha2/plans/05-wave-2.7-open-door.md`
- Wave 2.8 plan (template): `rpg-project/ideas/encounter/v1alpha2/plans/06-wave-2.8-combat-slice.md`
- Proto contract:
  - `dnd5e/api/v1alpha2/encounter/types.proto:284-317` — `InputRequired`, `SkillCheckPrompt`, `DialogueChoice`, `DialogueOption`, `TargetSelect`
  - `dnd5e/api/v1alpha2/encounter/service.proto:67` — `InteractResponse.input_required`
  - `dnd5e/api/v1alpha2/encounter/service.proto:80` — `TakeActionResponse.input_required`
  - `dnd5e/api/v1alpha2/encounter/service.proto:104-115` — `SubmitCheckRequest` / `SubmitCheckResponse` / RPC definition
- Toolkit references (verify pinned versions at dispatch):
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@<v0.3.0+>/data.go` — `DoorData` (does it have `Locked`?)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@<v0.3.0+>/encounter.go` — `OpenDoor` (Wave 2.7's verb; reused for SubmitCheck success path)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@<v0.55.x>/character/` — ability modifier math
- Board: https://github.com/users/KirkDiggler/projects/11

## Sign-off criterion

Wave 2.9 is done when, in a cold-start two-browser playtest:

- alice attempts a locked door
- the harness shows alice the skill-check prompt (DC, ability, tool); bob sees nothing
- alice submits a passing roll; the door opens; both alice and bob see the door + revealed geometry update via per-viewer events
- a separate attempt with a failing roll leaves the door closed; both browsers stay consistent (nothing wrongly revealed)
- the pending-prompt slot clears after each resolution; alice can attempt the door again

When the close-the-loop runs, the playtest video / screenshots become the verification artifact and the wave ships.
