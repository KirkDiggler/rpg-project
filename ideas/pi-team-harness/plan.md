---
name: Pi Team Harness Implementation Plan
issue: rpg-project#135
status: proposed — Plan Review
canonical_pr: rpg-project#136
---

# Pi Team Harness — Implementation Plan

## 1. Purpose and delivery rule

This plan implements the narrow Pi team-harness proof described in
[`design.md`](design.md). It is not implementation itself. It translates the
approved design into issue-backed, reviewable units while keeping #136 open as
the canonical design/plan/reconciliation surface.

Kirk's lifecycle decision is binding:

```text
#136: approved design -> Plan Review -> linked implementation evidence and
      canonical design/plan corrections -> final reconciliation -> Kirk merge

separate implementation issue + branch + ready PR per executable unit
      -> links #135 and #136; does not close #135
```

No implementation issue is created by this plan PR. After a fresh
design-to-plan consistency check confirms this plan is faithful to Kirk's
approved design, the coordinating lead creates each listed issue, adds it to
Project 19, and assigns its Team/Feature/Kind before a branch exists. Each
implementer owns its own issue/PR-to-merge flow. A discovery that changes the design or plan is
reported on its implementation issue/PR and updated visibly on the open #136
branch before the affected work can be considered reconciled. #136 merges only
when §10's final-reconciliation gate passes; it is never self-declared
`MERGE-READY`.

## 2. Fixed initial technical choices

### 2.1 Pi baseline verified for this plan

This plan is written against the installed **Pi 0.82.1**:

- project extensions auto-discover from `.pi/extensions/` after project trust;
- an extension may register commands/tools, listen for lifecycle events, set a
  status/widget, and use `ctx.ui.custom()` only in TUI mode;
- `session_start` is the first legal point for session-scoped resources and
  `session_shutdown` is the best-effort cleanup hook;
- `pi --mode rpc` uses strict LF-delimited JSONL and provides `prompt`,
  `steer`, `follow_up`, `abort`, lifecycle events, and `agent_settled`; and
- Pi has no built-in MCP or multi-agent control plane. A project extension must
  supply those integrations explicitly; no nonexistent native Pi MCP API is
  assumed.

The implementation pins the extension's Pi type/dev dependency to `0.82.1` and
the structural verifier rejects a different installed Pi major/minor baseline
until this plan is intentionally revised. The extension does not modify global
Pi settings, credentials, model catalogs, trust records, or package state.

### 2.2 Selected worker mechanism: replaceable RPC child process

The first proof uses **one `pi --mode rpc --no-session` child process per
worker**, not a same-process SDK session.

Why this is the initial choice:

1. RPC gives the supervisor a real process boundary and a documented JSONL
   protocol for `prompt`, `steer`, `follow_up`, `abort`, event streaming, and
   `agent_settled`.
2. `--no-session` makes local Pi session history explicitly non-durable; a
   crash cannot accidentally become a recovery dependency.
3. It matches the desired kill/replacement proof and can run with the worker's
   isolated worktree as `cwd`.
4. It avoids having to rebuild/rebind `AgentSessionRuntime` subscriptions when
   sessions are replaced. The SDK remains a later, evidence-driven alternative,
   not a parallel initial implementation.

The supervisor is an in-memory object owned by the lead extension. It holds
only a child process handle, JSONL decoder buffer, live status, and the current
worktree path. It starts no service, socket listener, file watcher, timer-based
queue, daemon, or durable database. Normal `session_shutdown` sends best-effort
child termination/release. A crash can skip cleanup; the recovery contract is
GitHub, not orphan cleanup or Pi session restoration.

### 2.3 Project-local package shape

The implementation is a project-local extension package, not a global install:

```text
rpg-project/
  .gitignore
  .pi/extensions/pi-team-harness/
    package.json
    package-lock.json
    index.ts
    src/
      charters.ts
      roles.ts
      worker-policy.ts
      github.ts
      checkpoint.ts
      rpc-jsonl.ts
      rpc-worker.ts
      supervisor.ts
      status.ts
      team-overlay.ts
      evidence.ts
      capabilities/
        mcp-bridge.ts
        chrome.ts
        # destination architecture only; created only by the post-retro
        # Blender activation issue, never by the core proof:
        blender-lease.ts
        blender.ts
    test/
      fixtures/fake-pi-rpc.mjs
      fixtures/fake-mcp-server.mjs
      *.test.ts
  scripts/verify-pi-team-harness.mjs
```

`package.json` declares `@earendil-works/pi-coding-agent`,
`@earendil-works/pi-tui`, and `typebox` as Pi-required peer dependencies (the
Pi package convention uses `"*"` ranges). For local type/tests it uses matching
`0.82.1` Pi dev dependencies plus `typebox@1.1.38`, `tsx@4.23.1`, and
`typescript@5.9.3`; the lockfile fixes their full transitive graph.
`node_modules/` is ignored and never committed. The only runtime dependency is
`@modelcontextprotocol/sdk@1.29.0` for the optional, short-lived capability
bridge; it is not a Pi feature. It requires Node >=18. The implementer
rechecks its release notes/API during the bridge issue and records any necessary
plan correction on #136 rather than guessing an API.

All TypeScript runs through Pi's extension loader at runtime. Local tests use
`tsx` plus Node's built-in test runner; no new repository-wide test framework is
introduced. The package's exact commands are:

```bash
npm --prefix .pi/extensions/pi-team-harness ci
npm --prefix .pi/extensions/pi-team-harness run typecheck
npm --prefix .pi/extensions/pi-team-harness test
node scripts/verify-pi-team-harness.mjs
```

The verifier is deterministic and makes **no model call, GitHub mutation,
Chrome connection, Blender connection, or MCP launch**. It validates file
shape, lock/package constraints, canonical charter paths, the no-store policy,
and the installed Pi version. Live acceptance belongs to later issue evidence,
not this verifier.

## 3. Shared implementation contracts

### 3.1 Role manifest and charter loading

`src/roles.ts` is a small manifest, not a copied policy store. For each existing
lead/member/fixer/supporting role it contains only:

- role ID and applicable Team;
- literal canonical charter path(s), including the web responsibility overlay
  where applicable;
- the smallest allowed built-in Pi tool set; and
- capability flags (`github`, `chromeEvidence`, `blenderLease`) defaulting to
  false unless the design permits them.

`src/charters.ts` resolves those paths from the `rpg-project` root supplied by
the lead, reads their current content at worker startup, and constructs the
worker context in memory. It never writes a copied role prompt, context journal,
or task record under `.pi`. The extension uses `CONFIG_DIR_NAME` rather than
hardcoding a path when it must locate Pi project resources.

The first live dispatch enables only the selected lead and one selected standing
member. The manifest may validate all current canonical role IDs so drift is
caught structurally, but it does not build routing, background workers, or
capability access for every role in the first proof.

### 3.2 Worker launch and message contract

For a human-authorized dispatch the lead extension starts a child equivalent
to:

```text
pi --mode rpc --no-session \
  --extension <rpg-project>/.pi/extensions/pi-team-harness/index.ts \
  --tools <role-profile-tools>
```

It never bypasses normal project trust, sets a project-trust default, or
auto-answers a permission/auth prompt. Before spawn, the lead checks that its own project
session is already trusted and asks the human to confirm the named issue, role,
worktree, and tool profile. If trust is absent, the UI is unavailable, or the
human declines, dispatch stops with a visible blocker and no child process.

The supervisor uses `spawn(..., { shell: false, cwd: assignedWorktree })` and
sets explicit child-mode environment values containing the role ID and canonical
project root. `index.ts` detects child mode: it installs the role context and
policy interception but never creates another supervisor. The lead mode creates
one in-memory supervisor only after `session_start`. A child-side permission or
auth request is surfaced to the human; it is never approved by the supervisor.

The JSONL decoder must split on LF only, strip one trailing CR from input, and
retain all other Unicode characters. It must **not** use Node `readline`, which
is not protocol-compliant for Pi RPC framing. It correlates command responses by
request ID, preserves event order, treats `agent_end` as non-final, and marks a
worker settled only on `agent_settled`.

The lead exposes exactly these worker actions:

| Action | RPC command | Meaning |
|---|---|---|
| start dispatch | `prompt` | one scoped worker brief after issue/board validation |
| correct current task | `steer` | delivered after the current assistant tool batch |
| queue next request | `follow_up` | delivered only after the current run settles |
| stop | `abort` | explicit human/lead cancellation only |
| inspect | `get_state` / streamed events | live, non-durable status only |

A `request checkpoint` action is a human-readable follow-up instruction; it is
not a fabricated checkpoint. The worker must publish the actual GitHub comment
with completed work, verification scope, blockers, and explicit next action.

### 3.3 GitHub/Project 19 dispatch contract

`src/github.ts` is a narrow `gh`-CLI adapter behind an injectable interface. It
reads the backing issue and Project 19 item before launch and refuses dispatch
unless all of these are true:

1. the issue exists and is open;
2. its Project 19 item exists;
3. Status, Team, Feature, and Kind are present;
4. the role is compatible with the Team/ownership boundary; and
5. the caller supplied an issue ID, fresh branch name, and isolated worktree.

The adapter has no local cache. The TUI may retain the last read response only
for its current render and marks it stale after a read error. It does not use
Pi `appendEntry`, session JSONL, a JSON file, SQLite, Redis, or a hidden task
queue for task/checkpoint state. This is the explicit **no second durable task
database** rule; the verifier and recovery tests enforce it as a negative
contract.

`src/checkpoint.ts` renders the required signed issue-comment template. It
requires completed work, verification, blockers (explicit `none` is allowed),
next action/owner, and branch/PR when available. It ends every generated GitHub
body with:

```text
— Pi team-harness design worker, on behalf of KirkDiggler
```

Implementation roles use their own role label in the same required signature
shape. The template deliberately cannot emit `MERGE-READY`.

### 3.4 Tool permissions are defense in depth

`src/worker-policy.ts` applies the manifest's least-privilege built-in tool
set and an extension `tool_call` interceptor in child mode. It blocks known
out-of-profile tool/path/capability attempts and demands an explicit
capability token for the Chrome custom tools. The eventual Blender adapter adds
its own lease check only after the post-retro activation issue. This does not
claim to sandbox arbitrary shell behavior; canonical charters, issue scope,
isolated worktrees, and review remain authoritative.

The initial profiles are deliberately small:

- Explore: read/search only, no child dispatch, write, GitHub mutation, Chrome,
  or Blender.
- Janitor: only its documented curation/GitHub hygiene surface, no product
  implementation or evidence control.
- Lead: coordination/GitHub reads and comments; no edit/write/implementation or
  merge command.
- Selected standing member: only the assigned worktree's normal repository
  tools, plus GitHub checkpoint capability.
- Chrome evidence: designated UI/UX or Assets web worker only, on an
  issue-backed evidence action; this is the only capability activated in the
  core proof.
- Blender: destination-only until the post-retro activation issue; then only an
  explicitly assigned Assets worker after a human grant and while the one
  in-memory exclusive lease is active.

The policy test suite proves the block/allow decisions for these profiles. It
also proves that a charter refusal or a missing Project item cannot be converted
into a dispatch by a TUI action.

### 3.5 Evidence/capability bridge contract

Pi 0.82.1 has no native MCP client. `src/capabilities/mcp-bridge.ts` is therefore
a short-lived stdio MCP client built with the pinned MCP SDK, started only for a
role-scoped evidence command and stopped at completion/release. It is a child of
the extension process, not a daemon and not a general tool marketplace.

The bridge has a safe-default empty capability allowlist. Before a live use, the
backing issue names the target, expected evidence, and the exact approved MCP
capability names after the bridge has listed them. The bridge rejects all other
calls. **The core proof activates Chrome only.** Unit tests use
`fake-mcp-server.mjs`; no test needs a real browser or Blender GUI.

- **Chrome:** the bridge derives the existing Chrome DevTools server command
  from `rpg-project/opencode.jsonc` rather than copying a second configuration.
  At this plan's baseline that is the local `chrome-devtools-mcp` command against
  `http://127.0.0.1:9222` with usage statistics disabled. It never launches a
  browser, touches unrelated tabs, or uses production credentials. A fallback
  evidence path may invoke the existing `game-dev/tools/browser/screenshot.mjs`
  for a named local URL; it is a screenshot harness, not a claim of DevTools
  coverage.
- **Blender (deferred destination):** no Blender adapter, lease module, or live
  Blender capability is activated in PIH-1 through PIH-7. Only after the Chrome
  core proof and PIH-7 retro may a separate, linked Assets issue activate it.
  That issue must derive the pinned server command/environment from existing
  `game-dev/opencode.json`: `uvx --from blender-mcp==1.6.4 blender-mcp`,
  loopback host/port 9876, telemetry disabled, and 180-second timeout. It must
  not write another global/project Blender configuration. Its
  `blender-lease.ts` is process-local and permits only one named Assets worker:
  it starts unleased, requires an explicit human grant, releases/disconnects on
  normal completion or `session_shutdown`, and after a crash requires a human
  GUI Disconnect before replacement. This remains a procedural safety control,
  not a false claim of protocol-level locking.

`src/evidence.ts` creates a concise report containing role, issue/PR URL,
command/version/config fingerprints, target, actions, artifact URL, what route
was actually exercised, and residual risk. A report without an issue/PR-visible
artifact URL is rejected as non-durable. Raw Synty assets, converted GLBs,
credentials, and private scene contents are never attached to `rpg-project`.
If approved GitHub-accessible artifact storage is unavailable, the worker posts
that exact blocker and does not claim the evidence gate passed.

## 4. Reviewable implementation units

The identifiers below are planning labels, **not issue numbers**. Create the
actual issue and Project 19 item only after a fresh consistency check confirms
this plan is faithful to the approved design. Each code unit is workflow setup:
deterministic checks + self-review + ready PR + normal review; it does not
receive an independent product gate. A later proof that changes real product
behavior follows the existing product-gate policy.

### PIH-1 — Extension skeleton and deterministic verifier

**Dependency:** fresh design-to-plan consistency check complete.

**Issue/PR:** create `rpg-project` issue “Build Pi team-harness extension
skeleton and verifier”; Team=Cross-team, Feature=Infra, Kind=Build. Its one
ready PR links #135/#136 and changes only this unit's files.

**Files:**

- create `.gitignore` entries for
  `.pi/extensions/pi-team-harness/node_modules/` and package test output;
- create `.pi/extensions/pi-team-harness/{package.json,package-lock.json,index.ts}`;
- create minimal `src/{roles.ts,charters.ts,worker-policy.ts}` exports;
- create package `test/{roles.test.ts,charters.test.ts,worker-policy.test.ts}`;
- create `scripts/verify-pi-team-harness.mjs`.

**Tests/checks:** run the four §2.3 commands. The test suite verifies the
package's pinned Pi 0.82.1 contract, no copied charter prose, all literal
canonical charter/overlay paths resolve, and absent node modules/configuration
produce a deterministic verifier failure. The verifier checks the extension has
no listener/database/queue configuration and does not call Pi, `gh`, Chrome,
Blender, or an MCP server.

**Acceptance evidence:** PR contains the red verifier output before the required
files exist and the green deterministic output after; the issue checkpoint
links the ready PR, exact commands, and the scoped diff.

### PIH-2 — Charter adapters and GitHub-derived dispatch/checkpoint contract

**Dependency:** PIH-1 merged.

**Issue/PR:** create `rpg-project` issue “Add Pi role adapters and GitHub-only
dispatch contract”; Team=Cross-team, Feature=Infra, Kind=Build. One ready PR
links #135/#136.

**Files:**

- extend `src/roles.ts` with current lead/member/fixer/Explore/Janitor/gate
  manifest entries and web overlays;
- extend `src/charters.ts` to resolve charter content only at dispatch time;
- create `src/{github.ts,checkpoint.ts,dispatch.ts}`;
- create `test/{dispatch.test.ts,checkpoint.test.ts,github.test.ts}` and fake
  `gh` fixture data under `test/fixtures/`;
- extend `scripts/verify-pi-team-harness.mjs` for manifest/path/no-store checks.

**Tests/checks:** injected fake GitHub responses prove dispatch refuses missing
issue, missing Project 19 item, incomplete Team/Feature/Kind, mismatched role,
and dirty/colliding worktree. Tests prove a valid input produces no persisted
local task record, canonical charter paths are read rather than copied, and a
checkpoint contains all required fields/signature but never `MERGE-READY`.

**Acceptance evidence:** ready PR shows only adapter/contract/verifier files;
issue checkpoint includes the test/typecheck/verifier output and demonstrates a
sample valid contract entirely from fixture GitHub data. No real issue comment
or Project field is mutated by unit tests.

### PIH-3 — One nonblocking RPC worker and honest recovery semantics

**Dependency:** PIH-1 and PIH-2 merged.

**Issue/PR:** create `rpg-project` issue “Prototype Pi RPC worker supervision
and recovery”; Team=Cross-team, Feature=Infra, Kind=Build. One ready PR links
#135/#136.

**Files:**

- create `src/{rpc-jsonl.ts,rpc-worker.ts,supervisor.ts}`;
- extend `index.ts` with lead/child mode and `session_start`/
  `session_shutdown` ownership;
- create `test/fixtures/fake-pi-rpc.mjs` plus
  `test/{rpc-jsonl.test.ts,rpc-worker.test.ts,supervisor.test.ts,recovery.test.ts}`;
- extend the verifier to reject a session/task persistence path or daemon
  configuration.

**Tests/checks:**

- JSONL fixture includes CRLF input and U+2028/U+2029 content to prove the
  decoder splits only on LF and does not use `readline`.
- Fake RPC stream proves `agent_end` is not completion and `agent_settled` is.
- Tests prove `steer`, `follow_up`, `abort`, response IDs, and child stdout/stderr
  failure reporting map to the documented RPC commands.
- Supervisor starts at most one child for the prototype, returns control without
  awaiting its settlement, uses `shell: false`, and sends normal-shutdown cleanup
  best effort.
- Kill simulation deletes all in-memory supervisor state, starts a new supervisor,
  and proves the replacement input is only the supplied issue/Project/branch/PR/
  checkpoint fixture — not session JSONL, PID, worktree path, or prior messages.

**Acceptance evidence:** test transcript identifies selected mechanism as
`pi --mode rpc --no-session`; live claim remains limited to a fake child until
PIH-6. The issue checkpoint names crash limitations and the exact human next
action if a real child cannot start/authenticate.

### PIH-4 — TUI team status and human escalation surface

**Dependency:** PIH-2 and PIH-3 merged.

**Issue/PR:** create `rpg-project` issue “Add Pi team status and escalation
surface”; Team=Cross-team, Feature=Infra, Kind=Build. One ready PR links
#135/#136.

**Files:**

- create `src/{status.ts,team-overlay.ts}`;
- extend `index.ts` with `/team-status`, `/team-dispatch`, `/team-message`,
  `/team-abort`, and `/team-escalate` commands;
- create `test/{status.test.ts,team-overlay.test.ts,commands.test.ts}`;
- extend verifier checks for TUI-mode guards and command fallback declarations.

**Tests/checks:** fixture data renders Team-now, Needs-a-human, Review, Recovery,
and cross-team seam rows with source URLs/timestamps and an explicit stale state
when GitHub reads fail. TUI tests constrain every render line to supplied width
and exercise focus/cancel behavior. RPC/headless tests prove `ctx.mode !==
"tui"` does not call `ctx.ui.custom()` and commands still emit structured
status/extension UI notifications.

**Acceptance evidence:** a terminal capture from a fake supervisor and fake
GitHub dataset shows a running card plus a human escalation. It is labelled as
fixture UI evidence, not a real Project 19 or worker recovery claim.

### PIH-5 — Role-scoped Chrome evidence bridge and evidence contract

**Dependency:** PIH-2, PIH-3, and PIH-4 merged.

**Issue/PR:** create `rpg-project` issue “Add Pi role-scoped Chrome evidence
bridge”; Team=Cross-team, Feature=Infra, Kind=Build. One ready PR links
#135/#136.

**Files:**

- create `src/capabilities/{mcp-bridge.ts,chrome.ts}`;
- create `src/evidence.ts`;
- extend `src/{roles.ts,worker-policy.ts,supervisor.ts,index.ts}` for explicit
  Chrome capability requests only;
- create `test/fixtures/fake-mcp-server.mjs` and
  `test/{mcp-bridge.test.ts,chrome.test.ts,evidence.test.ts}`;
- extend package lock and verifier for the MCP SDK and Chrome-config derivation.

**Tests/checks:** a fake MCP stdio server proves short-lived launch/list/call/
shutdown framing, safe-default empty allowlist, role rejection, and no live
server in deterministic checks. Chrome tests derive command data from an
`opencode.jsonc` fixture and reject a non-loopback/usage-statistics-enabled
configuration. Evidence tests reject missing issue/PR URL, artifact URL, command
fingerprint, scope, or residual risk. No Blender module, lease test, server
configuration read, or active-scene connection is part of this unit.

**Acceptance evidence:** the PR demonstrates only fake-server Chrome checks.
PIH-6 is the first and only live evidence route in the core proof. The
capability PR never connects to a browser merely to make CI green.

### PIH-6 — Deliberate-kill/replacement proof and durable Chrome evidence

**Dependency:** PIH-1 through PIH-5 merged; a human selects one small real
Project 19 task with the existing local Chrome evidence path.

**Issue/PR:** create an evidence-only `rpg-project` Verify issue “Prove Pi
team-harness replacement and Chrome evidence”; Team=Cross-team, Feature=Infra,
Kind=Verify. It links #135/#136 and the selected real task. It contains no
implementation code PR unless the proof discovers a scoped defect; that defect
gets its own linked Build/Fix issue and PR.

**Files:** no planned source change. Any discovery-driven design/plan correction
is committed to the open #136 branch and references this Verify issue; it is not
silently carried in a worker session.

**Procedure and checks:**

1. Confirm the real issue, Project 19 fields, role, fresh worktree/branch, and
   evidence acceptance before dispatch.
2. Post `WORK SESSION STARTED`; launch exactly one selected RPC child and show a
   nonblocking lead interaction.
3. Exchange one scoped `steer` or `follow_up`; publish a complete GitHub
   checkpoint.
4. Deliberately terminate the child after that checkpoint. Discard supervisor
   memory. A fresh worker reads only issue/Project/branch/PR/checkpoint state
   and performs the recorded next action.
5. Capture **Chrome only**: a designated UI/UX or Assets web evidence worker
   uses the named local target and approved capability allowlist, attaches an
   artifact URL with console/network scope, and states honestly whether the real
   player path was exercised. Blender is not an alternate core-proof route.
6. Attach/link the Chrome evidence report from the selected task's issue/PR. If
   an auth, permission, browser, artifact-storage, or recovery prerequisite
   fails, stop and publish the exact blocker; do not retry invisibly or call the
   proof passed.

**Acceptance evidence:** linked issue comments show the before-kill checkpoint,
replacement worker's GitHub-only reconstruction, one durable Chrome artifact
URL, command/version/config fingerprint, and a scoped statement of what was and
was not proven. Kirk reviews the proof; no agent merges #136.

### PIH-7 — Side-by-side evaluation, retro, and final reconciliation

**Dependency:** PIH-6 complete and all discovery fixes/corrections merged.

**Issue/PR:** create an evidence-only `rpg-project` Decide/Verify issue “Review
Pi harness against Claude Code and OpenCode”; Team=Cross-team, Feature=Infra,
Kind=Decide. It links #135/#136. No speculative optimization PR is created from
this issue; an observed deficiency becomes a new linked issue after the retro.

**Files:** update `ideas/pi-team-harness/{design.md,plan.md}` on the existing
#136 branch only where PIH-1..6 evidence changed an assumption. Add no separate
status database, local evidence journal, or implementation code in this
reconciliation beat.

**Checks:** compare comparable board-backed task evidence from Pi, Claude Code,
and OpenCode on checkpoint/recovery completeness, human interruption behavior,
boundary/refusal behavior, QA/gate results, evidence scope, worktree/auth/
lease failures, operator visibility, and contextual cost/latency. Do not call
unlike tasks a benchmark. Record each change/no-change decision and its
rationale on the retro issue and #136.

**Acceptance evidence:** final reconciliation checklist in §10 is complete,
all linked implementation PRs and evidence URLs are enumerated, and Kirk has a
single readable canonical PR to review. This is the only unit that may request
Kirk's final #136 merge decision; it never performs the merge.

### Deferred Blender activation — conditional post-retro issue

**Dependency:** PIH-6 Chrome proof and PIH-7 retro complete. The retro must
identify a real unmet Assets evidence need; otherwise this unit is not created
and Chrome remains the completed narrow proof.

**Issue/PR:** only if the retro justifies it, create a separate `rpg-project`
Assets Build issue and Project 19 item for Blender activation. It links #135/#136
and receives its own branch/ready PR. If created before #136's final
reconciliation, #136 remains open until it is reconciled; if deferred, the
approved destination architecture remains documented without pretending it was
proved.

**Files/tests/evidence:** create `src/capabilities/{blender-lease.ts,blender.ts}`
and their `test/{blender-lease.test.ts,blender.test.ts}` only in this issue.
Tests derive the pinned command, loopback environment, telemetry setting, and
180-second timeout from an `opencode.json` fixture; prove Assets-only access,
one process-local lease, explicit human grant, normal release/disconnect, and
post-crash human GUI Disconnect before a replacement lease. Live evidence uses
the hardened disposable-scene-first pilot and the issue/PR-visible report
contract in §3.5. It never publishes raw Synty assets/GLBs or treats an active
scene capture as export/release QA.

## 5. Worktree, branch, and review protocol for all code units

For each PIH Build/Fix unit, the assigned standing owner:

1. creates/uses an issue and Project 19 item first;
2. fetches `origin/main` without touching another dirty checkout;
3. creates an issue-named branch and isolated worktree under
   `/home/kirk/game-dev/.pi-worktrees/`;
4. changes only the unit's declared files, stages them explicitly, and never
   uses `git add -A` or `--no-verify`;
5. runs the unit's test/typecheck/verifier commands and self-review; and
6. pushes one ready PR with `Closes #<its-own-issue>` and a link to #135/#136.

The PR body states whether it is workflow setup (self-review/deterministic
checks/Kirk review) or product behavior (normal review plus the independent
product gate). It never says `MERGE-READY` before the appropriate reviewer does.
The implementing role posts all GitHub comments with its required signature.

PIH-6 and PIH-7 are evidence-only issues unless they expose a defect. They do
not create a branch just to manufacture a PR; any actual corrective code/docs
change receives a real linked issue/PR under the normal rule.

## 6. Cross-unit acceptance matrix

| Design concern | Implementing unit(s) | Required proof |
|---|---|---|
| canonical roles, toolkit core vs D&D overlays, UI/Assets seam | PIH-1, PIH-2 | literal charter/overlay resolver tests; no copied policy |
| GitHub/Project 19 sole durable truth and checkpoint shape | PIH-2, PIH-6 | fake contract refusal tests; live GitHub checkpoint/replacement evidence |
| nonblocking persistent worker and two-way messages | PIH-3, PIH-4, PIH-6 | fake RPC event/settlement tests; live one-child steer/follow-up proof |
| honest crash/restart boundary | PIH-3, PIH-6 | no-session launch + kill fixture; live GitHub-only replacement |
| human-oriented views/escalations | PIH-4 | fixture TUI/headless fallback tests and a labelled terminal capture |
| role-scoped Chrome safety | PIH-2, PIH-5, PIH-6 | policy/bridge tests and one durable Chrome artifact |
| deferred Blender safety | conditional post-retro issue only | Assets-only lease/pilot tests and evidence only if the retro justifies activation |
| worktree isolation | PIH-2, PIH-3, PIH-6 | collision/dirty worktree refusal tests; live branch/worktree checkpoint |
| no daemon or second durable store | PIH-1 through PIH-7 | verifier/static checks plus restart/recovery evidence |
| side-by-side evaluation and future scope | PIH-7 | comparable evidence-based retro, not a synthetic benchmark |
| long-lived canonical PR lifecycle | this PR, PIH-1 through PIH-7 | linked PRs plus §10 reconciliation before #136 merge |

## 7. Explicit non-goals and stop conditions

The first proof stops rather than expands when it encounters any of the
following:

- a request for a long-lived daemon, listener, shared queue, second task
  database, session-derived recovery mechanism, or automatic multi-worker
  scheduler;
- a request to copy/replace canonical role charters, move D&D rules into toolkit
  core/API/proto/web, or grant a lead/member merge authority;
- an MCP capability not named in an issue-approved allowlist;
- a second Blender client, a missing human save/grant/disconnect step, a
  non-loopback Blender endpoint, telemetry/cloud integration, or a request to
  modify an active pipeline scene after a pilot failure;
- a Chrome target outside the named local/dev target, credential handling, or
  a fixture being claimed as a player-path result; or
- a permission/auth prompt that cannot be resolved visibly.

The worker publishes the exact blocker and next human action. It does not patch
upstream MCP software, loosen safeguards, use an unrelated worktree, or persist
state locally to make the failure disappear.

## 8. Plan-review self-checklist

Before publishing the focused plan-consistency checkpoint on #136, verify:

- [ ] #136's body says `Review phase: Plan Review`, stays ready/non-draft, and
  says it remains open through implementation.
- [ ] `design.md` states the same long-lived canonical lifecycle without the old
  separate-plan-PR contradiction.
- [ ] The plan selects RPC child processes with `--no-session`, identifies the
  SDK only as deferred, and cites actual Pi 0.82.1 RPC/TUI/extension mechanics.
- [ ] Every PIH unit has a post-approval issue/Project 19 requirement,
  dependency, exact files, deterministic/live checks, and acceptance evidence.
- [ ] PIH-5 does not imply a native Pi MCP API, derives only the existing
  Chrome configuration, and leaves Blender adapter activation to the conditional
  post-retro issue with its hardened pilot/lease contract intact.
- [ ] PIH-6 proves one child, one replacement, and one durable **Chrome**
  artifact — not a speculative multi-worker deployment or Blender pilot.
- [ ] The plan contains no implementation issue creation, source implementation,
  global Pi change, daemon, second store, raw asset/GLB publication, or merge
  instruction.
- [ ] `git diff --check`, package/doc consistency review, and staged-file review
  pass before commit.

## 9. Planned review questions for Kirk

1. Does the selected initial mechanism — a single `--no-session` RPC child —
   give the right recovery honesty, with SDK sessions deferred until evidence
   shows a need?
2. Does the Chrome-only PIH-6 proof stay narrow enough while preserving the
   hardened Blender destination for a retro-justified Assets issue?
3. Is the final reconciliation gate sufficiently strict before #136 merges, or
   is an additional human decision point needed before PIH-7 begins?

## 10. Final reconciliation and #136 merge gate

Before Kirk is asked to merge #136, the Pi team-harness design worker/lead
publishes a final signed checkpoint and a #136 comment that enumerates:

1. every PIH issue, Project 19 item, branch, PR, and merged/not-merged state;
2. every discovery that changed `design.md` or `plan.md`, with a link to its
   implementation evidence and canonical #136 commit;
3. deterministic verifier/test/typecheck results for each implementation PR;
4. the deliberate-kill/replacement evidence and the durable Chrome artifact,
   including what it did not prove; and any conditional Blender activation
   evidence only if the PIH-7 retro actually justified that later issue;
5. the side-by-side Claude Code/OpenCode retro, observed failures, and any
   deferred follow-up issues;
6. confirmation that no daemon, second durable task store, hidden recovery
   dependency, global Pi/config/credential mutation, extra Blender client, or
   unsafe asset publication entered the delivered scope; and
7. the four-question done-gate answers plus remaining risks.

Kirk then decides whether the canonical artifacts reconcile with delivered
behavior and whether to merge #136. If any linked implementation work is open,
evidence is missing, an assumption is stale, or a dragon remains unresolved,
#136 stays open and the next action is published visibly. No agent merges it.
