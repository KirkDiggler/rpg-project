---
name: walk
description: Use when a slice must be walked on a real local stack before it merges — brings up a builder's own walk environment, runs the standing QA castle script, posts per-step evidence on the slice issue, measures before deferring a cost, and tears the environment down when the slice ships.
---

# Walk a slice

A walk proves the changed journey through the real stack. It runs the
standing QA script, reports what each step showed, and leaves the operator's
own stack alone.

## The standing QA script

The script is the QA castle: the-mad-king encounter (rpg-project#639).

1. Start outside the castle, neutral, with guards in front of the gate door.
2. Round a corner out of their sight and start sneaking; the watch cones show.
3. Start sneaking in plain sight: nothing happens.
4. Fight the gate guards: the next room is empty and nobody comes.
5. Open the gate door while it is unguarded: guards come from the far room.

A slice is walked once, at its final head, after the operator's early view.
A re-pin whose delta is only `go.mod` and `go.sum` does not walk again.

Run the steps the slice touches, plus any step whose result the slice could
change. A slice that changes what a step should show updates the encounter
and this step list in the same PR. The operator loads, edits and saves the
encounter in the studio.

## Ports and stacks

- **Web :3030 and api :8115 belong to the operator.** A builder never brings a
  stack up on them, restarts one, or takes them over, and never runs
  `dev-env.sh down-all`, which would stop the operator's stack.
- **A builder's walk uses its own ports** through a manifest at
  `envs/local/<name>.env`, for example `RPG_WEB_HOST_PORT=3031` and
  `RPG_API_HOST_PORT=8116`, and is started from the `game-dev` root with
  `scripts/dev-env.sh up local/<name>`.
- **The api runs from a detached worktree at the PR head:**
  `git -C <repo> worktree add <repo>/.worktrees/<name>-walk --detach <sha>`,
  with `RPG_API_PATH` pointing at it. Provider pins (pseudo-versions of
  unmerged toolkit commits) are edited in that walk tree only and never
  committed.
- **The web runs from the PR branch** (`RPG_DND5E_WEB_REF=<branch>`) or from
  its own detached walk tree. A walk never serves a builder's working tree:
  a served tree reloads the builder's uncommitted edits.
- A studio walk the operator is asked to view keeps one fixed port, because
  the studio's draft lives in the browser per origin.

## Evidence

The walk's evidence, the ticked checklist, stays on the slice issue. Its
operator-facing twin is the PR body's Try-it script: the same steps, written
for the operator to drive on their own stack.

- Post on the slice issue, as a `- [ ]` checklist ticked per step: what the
  step showed, a screenshot (`node tools/browser/screenshot.mjs <url> <out>`)
  and the wire values that prove it (ids, act ids, bodies, response fields).
- A walk on an ungated local stack does not prove access policy; the
  slice's test does.
- A slice that adds a gRPC service or RPC runs one gated-mode smoke check.
  Bring the walk api up with `RPG_WORLD_ACCESS_ENFORCEMENT=true` and the
  local dev identities that gated mode needs (`envs/local/world-access.compose.yml`
  is the template), then call the new RPC once as a seated player. A
  PermissionDenied "RPC has no configured access policy" is the finding.
- **Never fake evidence.** A step that cannot be produced on this stack is
  stated plainly with the reason; it is not inferred from a unit test.
- **Two clients** whenever the slice touches delivery, toasts, presentation
  sync or dice: what each client saw, per step.
- Scratch files and drivers live in the job's temporary directory under a
  name carrying the repo, the PR and the walker, never a generic name.

## Measure before deferring

A cost is not "fine at today's sizes" until it is measured on the walk:
client-side timing around the call, plus a temporary timer around the SDK
call in the walk tree's api. The api's own timing log does not measure this
(rpg-api#1103). Post the numbers with the deferral.

## Teardown

When the lead reports the slice shipped:
`scripts/dev-env.sh clean local/<name>` from the `game-dev` root, then remove
this walk's worktrees by exact path (`git worktree remove <path>`). Never a
glob, and never another slice's tree.
