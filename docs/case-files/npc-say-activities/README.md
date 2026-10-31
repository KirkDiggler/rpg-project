# Counted say activity — recorded verification

Case file for [project#746](https://github.com/KirkDiggler/rpg-project/issues/746).
This records a completed walk; it is not a new runtime contract or a substitute
for running future changes. The operator independently verified two cycles in
[the early view](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1363#issuecomment-6539659476).

## Revisions and environment

| Evidence | API | Web |
|---|---|---|
| Native Studio undo/redo, save/reopen, custom-look launch | 9bf790e | 1ffca2db |
| Final two-client count/interruption and refresh/fresh-launch traces | 3d7d945b0429f5b66249524ba2714f442e01e8e2 | d4e75340f8089085ac86ea74b48d035cc67ab6ad |

Final provider heads: behavior `9d157715`, encounter `9e81f6ec`, session
`2cb2d4d9`. Proto cue contract is published in v0.1.248. API's base catch-up
retains the newer generated SDK `4717a4416c26` and the already-reviewed
CharacterConfigReader/locking changes; it does not downgrade those contracts.
The full Studio flow predates that base catch-up, but its saved My World source
was then launched and exercised by both clients at the final heads.

QA ran on **3047/8137**, `local/say-activity-qa`, using detached source trees.
Operator preview **3046/8136** stayed fixed at the accepted earlier head;
castle3030/8115, old pacing3044 and the Blender lab3042 were not changed.

Mounted-source hashes:
- untracked `say-activity-walk.yaml`:
  `0c50b6480eaaebd466684688cf266d8dc1c1bff58d38d24874b619fa45f04d55`;
- canonical `the-mad-king.yaml` after its two-cycle prisoner example:
  `81e7a4c96ffe0713ca52d9eb67465bb804db23d7c23fa737447992e957063da7`.

The temporary walk source is deliberately not a shipped API reference. It adds
an interruption-friendly 20-cycle question to the earlier two-cycle fixture.
All runtime assets were synced from committed provider pins; no source models,
converted GLBs, textures or skeletal animation tracks are in this evidence directory.
The one image here is the Studio's 2D controls. Model-render screenshots remain
local; the compact traces contain only observable state and metadata.

## What ran

1. **Actual Studio, not a prepared JSON edit:** open the reference; set the
   ordinary response to three sustain cycles; Undo returns two; Redo returns
   three. Set both verdict rows to three, change Style to03 and Skin palette toB,
   save `qa-say-activities` to My World, reopen from the server and verify all
   choices. `studio-reopened.png` shows the saved controls.
2. **Authored launch:** Play from Studio, ask “Tell me what happened.” The actual
   mounted body URL ends in `goblinPrisoner02/03-b.glb`; the real Three action has
   `repetitions:3`, advances through its native cycle boundaries, exits, and
   returns to Relaxed. `studio.json` and `studio-samples.json.gz` record it.
3. **Two independent clients:** Fighter hosts, Barbarian joins; launch the
   Bard-authored My World site. Both receive the selected response and play
   three cycles on the saved look, then return to default. This is not an RPC-
   response-only effect on the speaker's client.
4. **Interrupt the long response:** ask “Take your time thinking.” Both native
   actions have `repetitions:20`. Start the pacing drill. The requested walk
   cancels the cue before its budget finishes, completes Exit, then translates
   only while walking. After arrival and six additional seconds, neither body
   revives Thoughtful. Traces: `fighter.json.gz`, `barbarian.json.gz`;
   checkpoints/errors: `two-client.json`.
5. **Lifecycle:** refresh during a long live cue; catch-up does not replay it.
   A genuinely new live response still plays. Abandon during another active
   cue and launch fresh: original positions, no old move sequence/hold/cue,
   Relaxed sustain after six seconds. `lifecycle.json`, `catchup.json.gz` and
   `fresh-launch.json.gz` record the final-head pass.

No page errors were recorded in these successful runs. The first repeat-cycle
counter in Three0.181.2 remains `-1` until the first wrap; persisted values1 and2
then precede completion at three. The verifier accounts for that native
implementation rather than pretending a polled zero is required. Studio's final
Relaxed sample is in its explicit completion checkpoint: the40ms interval can
end just before that final frame.

Run the consistency check:

```sh
python3 docs/case-files/npc-say-activities/verify.py
```

It checks the recorded native counts, return before the next cue, long-cue
cancellation, walking-only translation and no revival. The UI action checks
come from the executed driver, not from this offline checker.

## Review and regression evidence

One independent backend round and one web round found three Important defects:
- encounter cue pointer escaped copy-in/copy-out, direct and nested;
- queued cue survived visible DOWNED→UP before reveal;
- a failed body retained a cue for a replacement body, including late delivery
  to an already-failed body.

All were reproduced before correction. Existing cloneAnswer now detaches the
cue (`3ca599cc`); separate standing-body cue eligibility preserves walk ownership
(`64c13596`); terminal model failure retires exact pending delivery while retaining
the diagnostic (`1ffca2db`). Review threads have explicit dispositions and are
resolved. No second broad review round was requested.

The canonical castle's prisoner-1 `where-is-the-king` response now authors two
Thoughtful cycles on either verdict, without changing text, facts, DC, faction
or appearance. A real compiler/encounter Speak assertion, after reload, failed
for missing cues and now passes on both verdicts. Narrow golden regeneration
changes exactly those two Activity objects. Full castle/module race gates pass.
The operator's running castle was not used or restarted, and no claim is made
that all unrelated live castle navigation/sneaking steps were repeated.

Final local web gate at d4e75340: **9507 passed / 6 skipped**,731 files passed /1
skipped; format/lint/typecheck/build pass. The extra conditional real-provider
dice observation check has no ignored dice-tray manifest in the builder tree;
no skip was added. Hosted checks pass at the final heads. Behavior, encounter
and session full module race/lint gates and API `make test`/`make ci-check` pass.
API gates temporarily hold the untracked local walk file outside the shipped
content shelf and restore it afterward; no shipped fixture assertion is waived.

## Driver corrections and limits

These were automation/setup errors, not product fixes: Wizard was not a seeded
identity (only Fighter/Barbarian/Bard are seeded); a resumed game needs Back
before choosing the Home character card; controls have full per-line labels;
observing a body must wait for Canvas, not require Speak on a nonacting client's
hotbar. One early immediate-reload attempt timed out; a scoped active-lobby/
roster probe and subsequent full final-head lifecycle pass succeeded. No claim
is made that the separately tracked older abandon/refresh issue web#1349 was
fixed by this slice.

Release re-pins and explicit operator merge authorization remain separate from
this evidence. Only the proto contract has merged when this record is written.
