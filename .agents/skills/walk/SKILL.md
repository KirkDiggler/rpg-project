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
   The gate holds one body and one eye (rpg-project#836, R4 A): `gate-1` is
   posted on the cell beneath the gate in front of the shut door and never
   leaves it by time; his post holds the gate's whole opening, both cells
   of it (R5, step 5); `gate-2` walks the loop.
   Sneaking answers who sees you, never whose body is in the way, so the
   sneak is two moves. First draw `gate-1` off: a voice he hears this round
   draws him toward it (a companion's word to someone outside, answered
   aloud, does it; in the fixture the captain beside the crossing draws
   nobody, though no test names it). Off his post's two cells he holds
   nothing. Then cross
   on `gate-2`'s blind beat, while she stands with her back to the gate.
   Before the post, both gate guards walked a loop and the crossing was a
   window two world rounds of four.
3. Start sneaking in plain sight: it is refused, and the refusal names who sees you (R11, 2026-11-02; before that the declaration was accepted and the watchers simply kept seeing you).
4. Fight the gate guards: a swing from free roam starts it, and the fight forms on that blow with every guard in view in it. The world pays one round. The next room is empty and nobody comes.
5. On a castle where he still holds the post, the gate is held, not only
   his cell (R5, rpg-project#852). Open the gate door from the cell beside
   him (in reach there: the castle-test fixture
   `TestReserves_MarchOnFightRounds` says so; the walk proves it on the
   stack), then walk for the courtyard. Hover the courtyard from either
   cell of the opening: no path goes through the gate. Click it where no
   way goes round: the walk stops at the opening and the stop reason names
   him, "Guard bars the way", or "Guard stands in the way" when the step is
   onto his cell. Never click his cell, which stays "approach to speak".
   The gate still looks like one door: no extra leaves, no cut in the wall
   at its crossings. A walk past `gate-2`, who is not posted, crosses as
   before. Open the gate door while it is unguarded: guards come from the
   far room. In S1 his body held only his own cell, and the party stepped
   through the opening's other cell beside him (rpg-project#841); Kirk moved
   the hold to the passage on lane #835, R5: "it is A and i think we will
   want a use case to develop until we make any decisions. ideally it is
   all configurable in the studio. this content is great since it really
   tests and reveals the seams and our decisions are either empowering or
   restrictive." The opening passes only once he holds nothing: downed in
   step 4's fight, drawn off, cowed or turned; with a second holder on the
   same door, downing one does not open it. Drawing him off is a voice, not
   a die, so it is walked: with the gate held, speak to someone within his
   earshot (the pedlar did it on Kirk's walk; today any voice he hears this
   round draws him), and he walks off his post; hover the courtyard and the
   path through the gate appears; walk through. The next round he walks
   back and holds again. What draws a post is still the open content call
   (R5), so the lure may change. The other rows are not walked: standing
   him aside, turning the garrison and the troupe let through each rest on
   a won check, and their tests
   stand in (the dice paragraph below): `TestMadKingThePostHoldsTheGatesPassage`
   (rpg-toolkit#2263), `TestMadKingTheGuardHoldsTheGate`,
   `TestMadKingScriptTheTroupeNeedsTheFeast`, and in the session
   `TestCastleThePostHoldsTheGatesOpening` (rpg-toolkit#2265).
6. Speak to the captain from the gatehouse: a plain greeting passes no round of the world. `stand-aside` is a check and pays one round. When it succeeds, its voice turns the gate post alert, which is the sound outside a fight. A free swing makes no such sound: it pulls the post into the fight, and a post inside a fight does not step (rpg-project#785 S2 rows).

A slice is walked once, at its final head, after the operator's early view.
A re-pin whose delta is only `go.mod` and `go.sum` does not walk again.
A row that needs a die to fall a certain way (a failed save, a lost
check) is not walked: the test that pins the row stands in, the checklist
names that test next to the row, and the operator walks the rows he can
reach in one try. A bug seen in play is filed as a small issue, not walked
for (Kirk, 2026-11-03, rpg-project#825 S1: "Rolling the dice to get a save
throw failure is not optimal. If we have coverage we can call it good.").

Run the steps the slice touches, plus any step whose result the slice could
change. A slice that changes what a step should show updates the encounter
and this step list in the same PR. The operator loads, edits and saves the
encounter in the studio.

A slice with a visible change walks on its own walk site first (build-slice
skill, "The walk site"), then on the castle. A walk that opens with "navigate
to" is the tell that the site is missing.

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
- **Content is served from the env's deployment clone, not from the api
  image.** `dev-env.sh up|restart` rebuilds the api from its ref and leaves
  `.runtime/local/<env>/sources/rpg-deployment/content/` as it was, so a
  content PR is invisible on the stack until its file is copied there. Before
  reporting any stack as carrying a content change, copy the PR head's file
  into that directory, restart, and show the hash of the file inside the
  running api container equal to the PR head's (`docker exec <api> sha256sum
  …` against `git show <sha>:content/<file> | sha256sum`). A walk report
  states both hashes.
- **An env ref names a branch or tag.** `RPG_API_REF`/`RPG_DND5E_WEB_REF`
  resolve against origin by name, and changing them does not move an existing
  env's source clone: check the clone out detached at the wanted commit
  (`git -C .runtime/local/<env>/sources/<repo> checkout --detach origin/<branch>`)
  before `restart`, and show `git log -1` of the clone in the walk report.

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
