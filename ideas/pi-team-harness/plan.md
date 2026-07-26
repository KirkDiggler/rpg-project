---
name: Pi Team Harness Implementation Plan
issue: rpg-project#150
status: proposed — live-feedback reconciliation review
historical_design_pr: rpg-project#136 (merged)
---

# Pi Team Harness — Implementation Plan

## 1. Purpose and delivery rule

This plan reconciles the narrow Pi team-harness proof described in
[`design.md`](design.md) with delivered PIH-M1/M2, PIH-3, PIH-4, and live-use
findings. It is not implementation. PR #136 is merged historical design/plan,
not a surface to reopen. This fresh #150 ready PR is the canonical review of
its focused corrections.

```text
#136 merged history -> #150 live-feedback reconciliation -> fresh issue-backed
implementation units -> evidence/discovery -> fresh rpg-project correction PR
when a design change is needed -> Kirk review/merge
```

After a fresh design-to-plan consistency check, the coordinating lead creates
each listed executable unit, adds it to Project 19, and assigns
Team/Feature/Kind before a branch exists. Each implementer owns its own
issue/PR-to-merge flow. A discovery that changes the design or plan is reported
on its implementation issue/PR and updated visibly through a fresh
`rpg-project` documentation issue/PR; it is never carried in local Pi state.
Every runtime PR links #150 and historical #135/#136. It is reviewable, never
self-declared `MERGE-READY`.

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

### 2.3 `game-dev` project-local runtime/package shape

The portable runtime is a `game-dev` project-local extension package, never a
global install or a second policy root:

```text
game-dev/
  .gitignore
  .pi/extensions/pi-team-harness/
    package.json
    package-lock.json
    index.ts
    src/
      charters.ts          # resolves ../rpg-project canonical paths at runtime
      roles.ts
      worker-policy.ts
      github.ts
      checkpoint.ts
      rpc-jsonl.ts
      rpc-worker.ts
      supervisor.ts
      status.ts
      focus.ts
      inbox.ts
      repository-map.ts
      worktree.ts
      team-overlay.ts
      evidence.ts
      capabilities/
        mcp-bridge.ts
        chrome.ts
        # destination only, created by a post-retro Assets issue:
        blender-lease.ts
        blender.ts
    test/
      fixtures/fake-pi-rpc.mjs
      fixtures/fake-mcp-server.mjs
      *.test.ts
  scripts/pi-team-harness.sh
  scripts/verify-pi-team-harness.mjs
  tests/pi-team-harness-clean-machine.sh
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
plan correction through a fresh issue-backed `rpg-project` documentation PR
rather than guessing an API.

The entry script resolves its own `game-dev` root, requires the bootstrapped
`$ROOT/rpg-project` clone and canonical charter paths, changes to `$ROOT`, and
`exec`s Pi while forwarding user arguments. It does not set a trust default,
answer a trust/permission prompt automatically, write global Pi files, or
launch from `rpg-project` to hide the ownership seam. Starting `pi` directly from `game-dev` has the same
project-local discovery result; the script is the portable named entry point.

All TypeScript runs through Pi's extension loader at runtime. Local tests use
`tsx` plus Node's built-in test runner; no new repository-wide test framework is
introduced. The package's exact commands, run from `game-dev`, are:

```bash
npm --prefix .pi/extensions/pi-team-harness ci
npm --prefix .pi/extensions/pi-team-harness run typecheck
npm --prefix .pi/extensions/pi-team-harness test
node scripts/verify-pi-team-harness.mjs
bash tests/pi-team-harness-clean-machine.sh
```

The verifier and clean-machine contract are deterministic and make **no model
call, GitHub mutation, Chrome connection, Blender connection, or MCP launch**.
They validate runtime file/lock shape, installed Pi version, no-store policy,
bootstrap presence of the `rpg-project` clone, canonical charter resolution,
entry-root behavior, and absence of global Pi/config/credential writes. Live
acceptance belongs to later issue evidence, not these verifiers.

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

`src/charters.ts` resolves those paths from the bootstrapped
`game-dev/rpg-project` clone, reads their current content at worker startup, and
constructs the worker context in memory. It never writes a copied role prompt,
context journal, or task record under `game-dev/.pi`. The extension uses
`CONFIG_DIR_NAME` rather than hardcoding a Pi-resource path; the workspace root
and sibling `rpg-project` path are explicit runtime inputs checked by the
clean-machine contract.

The manifest additionally maps each implementation role to exactly one
repository descriptor: GitHub owner/repository, sibling root name, expected
`origin`, base `origin/main`, and its permitted isolated-worktree parent. The
mapping is closed—not inferred from a role name, Team field, issue number, or
`game-dev` runtime root. `rpg-toolkit-member`, for example, maps only to
`KirkDiggler/rpg-toolkit`; web overlay roles both map to
`KirkDiggler/rpg-dnd5e-web`; Cross-team coordinating roles are non-implementing
and cannot select a repository by default.

The first live dispatch enables only the selected lead and one selected standing
member. The manifest may validate all current canonical role IDs so drift is
caught structurally, but it does not build routing, background workers, or
capability access for every role in the first proof.

### 3.2 Worker launch and message contract

For a human-authorized dispatch the lead extension starts a child equivalent
to:

```text
pi --mode rpc --no-session \
  --extension <game-dev>/.pi/extensions/pi-team-harness/index.ts \
  --tools <role-profile-tools>
```

It never bypasses normal project trust, sets a project-trust default, or
auto-answers a permission/auth prompt. Before spawn, the lead checks that its own project
session is already trusted and asks the human to confirm the named issue, role,
worktree, and tool profile. If trust is absent, the UI is unavailable, or the
human declines, dispatch stops with a visible blocker and no child process.

The supervisor uses `spawn(..., { shell: false, cwd: assignedWorktree })` and
sets explicit child-mode environment values containing the role ID, `game-dev`
runtime root, and canonical `game-dev/rpg-project` root. The explicit extension
path keeps the runtime available when an assigned sibling worktree cannot
auto-discover `game-dev/.pi/extensions`. `index.ts` detects child mode: it
installs the role context and policy interception but never creates another
supervisor. The lead mode creates
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
| start dispatch | `prompt` | one scoped worker brief after repository-qualified issue/board validation; attaches its issue to current in-memory focus |
| correct current task | `steer` | delivered after the current assistant tool batch |
| queue next request | `follow_up` | delivered only after the current run settles |
| stop | `abort` | explicit human/lead cancellation only |
| inspect | `get_state` / streamed events | live, non-durable status only |

The supervisor permits exactly **one** active child. Several focus/mailbox rows
are not worker slots and cannot schedule, retry, or restore one. A future
concurrency increase is an explicit post-core design decision.

`/team-focus add|remove|inspect`, `/team-status managed|team|project`, and
`/team-inbox [refresh|inspect]` are presentation commands. They accept only live
GitHub issue/PR/scoped-Project roots, retain focus/unread state in memory, and
never turn a broad Team/Project query into a hidden queue. A `request
checkpoint` action is a human-readable follow-up instruction, not a fabricated
checkpoint. The worker must publish the actual GitHub comment with completed
work, verification scope, blockers, and explicit next action.

### 3.3 GitHub/Project 19 and repository-aware dispatch contract

`src/github.ts` is a narrow injected `gh`-CLI adapter. The dispatch input names
`{ owner, repository, issueNumber }`, role, fresh branch, worktree path, and
brief; a bare issue number is invalid. `src/repository-map.ts` resolves the
role's sole descriptor before GitHub or git commands. Dispatch refuses unless:

1. the requested owner/repository exactly equals the role mapping;
2. `gh issue view --repo owner/repository` returns an open issue whose canonical
   URL exactly matches that owner/repository/number;
3. the Project 19 item exists and its content URL exactly matches that issue URL;
4. Status, Team, Feature, and Kind are present and the role is Team/overlay
   compatible; and
5. the mapped sibling root is a clean git checkout with expected `origin`, a
   resolvable `origin/main`, a fresh branch, and an isolated path contained by
   that descriptor's allowed worktree parent but by no repository/worktree.

`src/worktree.ts` resolves physical paths (including symlinks) before inspection
and performs the worktree creation from the mapped repository root, not from
`game-dev`. It rejects a missing/non-git/dirty root, wrong remote/base,
pre-existing branch/path/worktree, `..`/symlink escape, and any path nested in
or containing another repository/worktree. The child still receives the
explicit canonical `game-dev/.pi/extensions/pi-team-harness/index.ts` path and
`game-dev/rpg-project` charter root; the extension runtime never substitutes
for a target repository.

The adapter has no local cache. The TUI may retain only its current render and
marks it stale after a read error. It does not use Pi `appendEntry`, session
JSONL, a JSON file, SQLite, Redis, or a hidden task queue for task/checkpoint,
focus, unread, or mailbox state. This is the explicit **no second durable task
database** rule; verifier and recovery tests enforce it as a negative contract.

`src/checkpoint.ts` renders the required signed issue-comment template. It
requires completed work, verification, blockers (explicit `none` is allowed),
next action/owner, and branch/PR when available. PIH-4.1 accepts optional
explicit root and managed issue/PR links for GitHub-only re-anchor but never
serializes live focus/unread state. It ends every generated GitHub body with:

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

- **Chrome:** the `game-dev` bridge derives the existing Chrome DevTools server
  command from the bootstrapped `game-dev/rpg-project/opencode.jsonc`, rather
  than copying a second configuration. At this plan's baseline that is the local
  `chrome-devtools-mcp` command against `http://127.0.0.1:9222` with usage
  statistics disabled. It never launches a browser, touches unrelated tabs, or
  uses production credentials. A fallback evidence path may invoke the existing
  `game-dev/tools/browser/screenshot.mjs` for a named local URL; it is a
  screenshot harness, not a claim of DevTools coverage.
- **Blender (deferred destination):** no Blender adapter, lease module, or live
  Blender capability is activated in PIH-1 through PIH-7. Only after the Chrome
  core proof and PIH-7 retro may a separate, linked Assets issue activate it.
  That `game-dev` issue must derive the pinned server command/environment from
  existing `game-dev/opencode.json`: `uvx --from blender-mcp==1.6.4 blender-mcp`,
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

### Delivered history — migration, PIH-3, and PIH-4

- PIH-1/#140 and PIH-2/#142 were delivered first in `rpg-project`; they are
  history, not a reason to restore a runtime there.
- PIH-M1 is delivered as `game-dev#6` / PR #7: portable foundation, entry,
  bootstrap and clean-machine proof with runtime charter resolution.
- PIH-M2 is delivered as `rpg-project#145` / PR #146: the temporary
  `rpg-project` runtime copy is removed. The stopped wrong-repo PIH-3 #144 has
  no PR.
- PIH-3 is delivered as `game-dev#8` / PR #9: deterministic fake-child proof
  of one `--no-session` RPC worker and GitHub-only reconstruction.
- PIH-4 is delivered as `game-dev#12` / PR #13: five commands, live
  GitHub/Project status rendering, compact TUI/headless surface, and no durable
  task state. Its post-merge live read proved fresh rendering only; it exposed
  108 My-team rows and 258 repetitive Needs-human rows. It did **not** prove a
  live child, recovery, dispatch, or multiple workers.

These PRs are preserved history. The remaining ordered corrections begin at
PIH-4.1 and must finish before PIH-5.

### Delivered — PIH-M1 migration/proof in `game-dev`

**Delivered:** `game-dev#6` / PR #7, after #140/#142 and stopped #144. It
links #135/#136 history and proves the portable ownership boundary.

**Files:** temporarily add the delivered runtime foundation under
`game-dev/.pi/extensions/pi-team-harness/` and its package tests; add
`game-dev/scripts/{pi-team-harness.sh,verify-pi-team-harness.mjs}`;
`game-dev/tests/pi-team-harness-clean-machine.sh`; required `.gitignore`,
`bootstrap.sh`, and `scripts/verify-workspace.sh` changes. The copied files are
runtime only. `charters.ts` resolves `game-dev/rpg-project/docs/teams/roles/**`
at runtime; no role policy, Project 19 policy, design, or plan is copied.

**Tests/checks:** run §2.3 commands from `game-dev`; test a temporary
clean-machine workspace fixture in which bootstrap supplies all seven repos,
including `rpg-project`, then verify the entry script discovers the
`game-dev/.pi/extensions` package and resolves canonical charters. Red probes
must fail for missing `rpg-project`, missing charter paths, a runtime launched
from the wrong root, attempted global Pi/config/credential write, or a durable
store/daemon. Bootstrap remains idempotent and does not install Pi or alter
its global configuration.

**Acceptance evidence:** the `game-dev` PR shows the foundation works from the
portable root after bootstrap, not from a developer's pre-existing
`rpg-project` checkout. Temporary runtime duplication with #140/#142 is
explicitly recorded only until PIH-M2 completes.

### Delivered — PIH-M2 removes the temporary `rpg-project` runtime copy

**Delivered:** `rpg-project#145` / PR #146, after PIH-M1. It removed only the
old runtime/package/verifier and proves game-dev still discovers its own runtime
and canonical charter clone with no fallback copy.

**Tests/checks:** run the `game-dev` runtime verifier/clean-machine contract
against the migrated foundation; prove the removed `rpg-project/.pi/extensions/
pi-team-harness` path is no longer a runtime source; verify that `game-dev`
continues to resolve the canonical clone paths without a fallback copy.

**Acceptance evidence:** the cleanup PR is the durable end of temporary
runtime duplication. A fresh `game-dev` bootstrap+entry run succeeds after the
old copy is gone; failure is a blocker, not a reason to retain two runtimes.

### Delivered — PIH-3 one nonblocking RPC worker and honest recovery

**Delivered:** `game-dev#8` / PR #9 after PIH-M2, superseding stopped
`rpg-project#144`. It is deliberately one child and fake-child deterministic
evidence only; it is not multi-worker or live recovery proof.

**Files:** under `game-dev/.pi/extensions/pi-team-harness/`, create
`src/{rpc-jsonl.ts,rpc-worker.ts,supervisor.ts}`, extend `index.ts`, add
`test/fixtures/fake-pi-rpc.mjs` and
`test/{rpc-jsonl.test.ts,rpc-worker.test.ts,supervisor.test.ts,recovery.test.ts}`,
and extend the `game-dev` verifier. The explicit child extension path comes
from `game-dev`; charter resolution remains against `game-dev/rpg-project`.

**Tests/checks:** retain the existing strict-LF/CRLF/Unicode JSONL, response-ID,
`agent_settled`, `steer`/`follow_up`/`abort`, one-child/nonblocking/`shell:false`,
shutdown, and memory-loss GitHub-only replacement checks. Add a root-discovery
case proving a worker worktree does not become a second runtime root. Permission
or auth requests remain visible hard stops; no global trust/config mutation or
second durable store is permitted.

**Acceptance evidence:** test output identifies `pi --mode rpc --no-session`
launched from the portable `game-dev` runtime; live claim remains fake-child only
until PIH-6. The checkpoint names crash limitations and exact human action if a
child cannot start/authenticate.

### Delivered — PIH-4 status/escalation surface

`game-dev#12` / PR #13 delivered the five command/status surface after PIH-M2
and PIH-3. Its fixture and post-merge live GitHub-read evidence remain valid
only for status rendering. The unscoped global output is the explicit live-use
finding that requires the following ordered units; it is not a reason to revise
delivered history or claim live dispatch/recovery.

### PIH-4.1 — Managed-session focus and bounded projections

**Dependency/order:** first correction after delivered PIH-4. It must merge
before PIH-4.2, PIH-4.3, and PIH-5. Create one `game-dev` Build issue/Project
19 item (Team=Cross-team, Feature=Infra, Kind=Build) and ready PR linking
rpg-project#150 and historical #135/#136 plus game-dev#12/PR#13.

**Files:** add `game-dev/.pi/extensions/pi-team-harness/src/focus.ts`; extend
`src/{github.ts,checkpoint.ts,status.ts,team-overlay.ts}`, `index.ts`, and
`scripts/verify-pi-team-harness.mjs`; add `test/focus.test.ts`; extend
`test/{status.test.ts,team-overlay.test.ts,commands.test.ts}` and GitHub/
checkpoint fixtures.

**Tests/checks:** deterministic fixtures prove that `/team-focus add|remove|
inspect` accepts only live GitHub issue/PR/scoped-Project roots (not a bare
Project), keeps reason/source only in memory, attaches confirmed dispatch work,
and makes no GitHub mutation. One initiative root and several roots yield only
roots plus explicit GitHub PR/checkpoint/background descendants; Team/project
queries, arbitrary text links, Pi JSONL, worktrees, and old children are never
fallback inputs. `/team-status` defaults managed; labelled team/project modes
remain opt-in. Managed Needs-human/Review/Recovery rows are active/review/
recovery scoped, group repeated missing facts, cap each section at ten, show
shown/total and inspect path, and preserve width/TUI/RPC guards. Complete
restart begins with no focus and re-anchors only explicit checkpoint/root links
or returns ask-Kirk; a background row never starts/schedules/retries a child.

**Acceptance commands:** from `game-dev`, run `npm --prefix
.pi/extensions/pi-team-harness ci`, `npm --prefix .pi/extensions/pi-team-harness
run typecheck`, `npm --prefix .pi/extensions/pi-team-harness test`, `node
scripts/verify-pi-team-harness.mjs`, `bash tests/pi-team-harness-clean-machine.sh`,
and `git diff --check`.

**Acceptance evidence:** ready-PR fixture captures show one root, several
background roots, grouped/capped sections, opt-in broad view, and re-anchor
versus ask-Kirk. A later real `/team-status managed` capture names roots,
source/timestamp, totals, and omissions; it proves live GitHub rendering only,
not a worker, recovery, scheduler, or concurrency.

### PIH-4.2 — Managed director mailbox and refresh semantics

**Dependency/order:** PIH-4.1 merged. This is second and must merge before
PIH-4.3 and PIH-5. Create one `game-dev` Build issue/Project 19 item and ready
PR linking #150, #135/#136, PIH-4.1, and delivered PIH-3/4.

**Files:** add `src/inbox.ts`; extend `src/{status.ts,team-overlay.ts,github.ts,
supervisor.ts}`, `index.ts`, and the verifier; add `test/inbox.test.ts`; extend
`test/{status.test.ts,team-overlay.test.ts,commands.test.ts,supervisor.test.ts}`
and fixtures.

**Tests/checks:** fixtures require every event to have source role/team/author,
event type, concise summary, repository-qualified link, timestamp, root,
origin (`owned-rpc` or `github-refresh`), and next action/owner. They prove
owned RPC lifecycle/permission/auth/settlement/failure events notify
immediately, while external GitHub comments/checkpoints/reviews appear only on
startup or explicit `/team-inbox refresh`/status refresh; no timer, watcher,
webhook, listener, daemon, or poller is introduced. Tests prove ephemeral
unread/selected state is discarded on restart, which then displays recent
managed GitHub events read/unread-unknown; no appendEntry/session JSONL/local
cursor is read or written. TUI tests cover theme, width, focus/cancel, inbox
inspect/open-link, explicit refresh, and session-only read/unread actions; RPC
tests inspect `extension_ui_request` notification/status/widget fallback and
never call `custom()`.

**Acceptance commands:** run the same five `game-dev` package/verifier/
clean-machine/diff commands listed in PIH-4.1.

**Acceptance evidence:** fixture terminal/RPC captures label immediate owned
RPC notification versus refresh-discovered external activity and include all
mailbox fields. Any real capture is labelled presentation/live-read only; it
makes no claim of GitHub push delivery, durable unread state, worker recovery,
or a second child.

### PIH-4.3 — Repository-aware dispatch isolation

**Dependency/order:** PIH-4.2 merged. This is third and the required gate before
PIH-5 or any real sibling-repository field trial. Create one `game-dev` Build
issue/Project 19 item and ready PR linking #150, #135/#136, PIH-4.1/4.2, and
PIH-3/4.

**Files:** add `src/{repository-map.ts,worktree.ts}`; extend
`src/{roles.ts,github.ts,dispatch.ts,rpc-worker.ts,supervisor.ts}`, `index.ts`,
and verifier; add `test/{repository-map.test.ts,worktree.test.ts}`; extend
`test/{dispatch.test.ts,commands.test.ts,rpc-worker.test.ts,recovery.test.ts}`
and canonical-workspace/GitHub fixtures.

**Tests/checks:** prove role→owner/repository descriptors and explicit issue
identity; exact issue URL and Project item content URL matching; repository-
specific root/origin/`origin/main`/fresh-branch checks; target-root `git
worktree add` cwd; physical containment/isolation and collision refusal. Red
probes prove a `rpg-toolkit-member` dispatch refuses when any of
`KirkDiggler/game-dev`, game-dev issue URL/Project item, game-dev root/origin,
or game-dev-contained worktree substitutes for toolkit. Also prove no explicit
extension-path regression: the child launched in toolkit cwd still receives the
canonical game-dev extension and canonical game-dev/rpg-project charter root.
One-child enforcement remains green.

**Acceptance commands:** run the same five `game-dev` package/verifier/
clean-machine/diff commands listed in PIH-4.1.

**Acceptance evidence:** ready-PR deterministic trace shows successful
repository-qualified fixture dispatch and every game-dev-for-toolkit refusal.
Only after Kirk reviews that PR may one real simple toolkit task be selected;
that later evidence must state the exact target repo/root/origin/branch/path and
still proves one child, not simultaneous workers. No live task runs merely to
make CI green.

### PIH-5 — Role-scoped Chrome evidence bridge and evidence contract

**Dependency:** delivered PIH-M2/PIH-3/PIH-4 plus PIH-4.1, PIH-4.2, and
PIH-4.3 merged.

**Issue/PR:** create `game-dev` issue “Add Pi role-scoped Chrome evidence
bridge”; Team=Cross-team, Feature=Infra, Kind=Build. One ready `game-dev` PR
links #150, historical #135/#136, and PIH-M1/M2.

**Files:** under `game-dev/.pi/extensions/pi-team-harness/`, create
`src/capabilities/{mcp-bridge.ts,chrome.ts}` and `src/evidence.ts`; extend
`src/{roles.ts,worker-policy.ts,supervisor.ts,index.ts}` for explicit Chrome
capability requests; create `test/fixtures/fake-mcp-server.mjs` and
`test/{mcp-bridge.test.ts,chrome.test.ts,evidence.test.ts}`; and extend the
package lock and `game-dev` verifier for the MCP SDK and Chrome-config
derivation.

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

**Dependency:** delivered PIH-M2/PIH-3/PIH-4 and PIH-4.1 through PIH-5
merged; a human selects one small real Project 19 task with the existing local
Chrome evidence path. A sibling-repository task also requires PIH-4.3's
repository-aware dispatch evidence first.

**Issue/PR:** create an evidence-only `game-dev` Verify issue “Prove Pi
team-harness replacement and Chrome evidence”; Team=Cross-team, Feature=Infra,
Kind=Verify. It links #150, historical #135/#136, and the selected real task. It contains no
implementation code PR unless the proof discovers a scoped defect; that defect
gets its own linked Build/Fix issue and PR.

**Files:** no planned runtime source change. Any discovery-driven design/plan
correction uses a fresh issue-backed `rpg-project` documentation branch/PR and
references this `game-dev` Verify issue; it is not silently carried in a worker
session.

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
was not proven. Kirk reviews the proof; no agent merges the historical #136 or any current
reconciliation PR.

### PIH-7 — Side-by-side evaluation, retro, and final reconciliation

**Dependency:** PIH-6 complete and all discovery fixes/corrections merged.

**Issue/PR:** create an evidence-only `rpg-project` Decide/Verify issue “Review
Pi harness against Claude Code and OpenCode”; Team=Cross-team, Feature=Infra,
Kind=Decide. It links #150 and historical #135/#136. No speculative optimization PR is created from
this issue; an observed deficiency becomes a new linked issue after the retro.

**Files:** update `ideas/pi-team-harness/{design.md,plan.md}` only in a fresh
issue-backed `rpg-project` documentation PR where PIH-4.1..6 evidence changed
an assumption. Add no separate status database, local evidence journal, or
runtime implementation code in this reconciliation beat.

**Checks:** compare comparable board-backed task evidence from Pi, Claude Code,
and OpenCode on checkpoint/recovery completeness, human interruption behavior,
boundary/refusal behavior, QA/gate results, evidence scope, worktree/auth/
lease failures, operator visibility, and contextual cost/latency. Do not call
unlike tasks a benchmark. Record each change/no-change decision and its
rationale on the retro issue and historical #135/#136.

**Acceptance evidence:** final reconciliation checklist in §10 is complete,
all linked implementation PRs and evidence URLs are enumerated, and Kirk has a
single readable canonical documentation PR to review. It never reopens or
merges #136; Kirk decides any new documentation PR merge.

### Deferred Blender activation — conditional post-retro issue

**Dependency:** PIH-6 Chrome proof and PIH-7 retro complete. The retro must
identify a real unmet Assets evidence need; otherwise this unit is not created
and Chrome remains the completed narrow proof.

**Issue/PR:** only if the retro justifies it, create a separate `game-dev`
Assets Build issue and Project 19 item for Blender activation. It links #150 and
historical #135/#136 and receives its own `game-dev` branch/ready PR. If a
current documentation correction is still required, it receives its own fresh
`rpg-project` PR; the approved destination architecture remains documented
without pretending it was proved.

**Files/tests/evidence:** create
`game-dev/.pi/extensions/pi-team-harness/src/capabilities/{blender-lease.ts,blender.ts}`
and their package `test/{blender-lease.test.ts,blender.test.ts}` only in this
issue.
Tests derive the pinned command, loopback environment, telemetry setting, and
180-second timeout from an `opencode.json` fixture; prove Assets-only access,
one process-local lease, explicit human grant, normal release/disconnect, and
post-crash human GUI Disconnect before a replacement lease. Live evidence uses
the hardened disposable-scene-first pilot and the issue/PR-visible report
contract in §3.5. It never publishes raw Synty assets/GLBs or treats an active
scene capture as export/release QA.

## 5. Worktree, branch, and review protocol for all code units

Runtime Build/Fix units (the delivered PIH-M1/3/4 history, PIH-4.1 through
PIH-6, and any deferred Blender activation) live in `game-dev`: their owner
creates a `game-dev` issue and Project 19 item, branches from `game-dev` main,
and uses an isolated `game-dev` worktree. PIH-M2 is delivered in `rpg-project`;
PIH-7 and canonical design/plan corrections use fresh `rpg-project` issues/PRs,
not merged #136. Every new unit links #150 and historical #135/#136, stages only
declared files, never uses `git add -A` or `--no-verify`, self-reviews, and
opens one ready PR that closes only its own issue.

No runtime unit edits `rpg-project` role policy or design docs as a convenience;
it reports a discovery and a fresh canonical documentation PR reconciles it.
Conversely, canonical documentation never smuggles `game-dev` runtime code. The
PR body states whether it is workflow setup (deterministic checks and normal
review) or product behavior (normal review plus independent product gate). It
never says `MERGE-READY` before the appropriate reviewer does. Every
implementing role signs GitHub text.

PIH-6 and PIH-7 are evidence-only issues unless they expose a defect. They do
not create a branch just to manufacture a PR; an actual corrective change gets
its own linked issue/PR in the owning repository.

## 6. Cross-unit acceptance matrix

| Design concern | Implementing unit(s) | Required proof |
|---|---|---|
| portable runtime/entry/bootstrap clean machine | PIH-M1 (`game-dev`) | bootstrap clone of `rpg-project`; game-dev discovery/entry; no global mutation |
| temporary duplication removal | PIH-M2 (`rpg-project`) | migrated game-dev proof green before old runtime removal |
| canonical roles, toolkit core vs D&D overlays, UI/Assets seam | #140/#142 history + PIH-M1 | game-dev resolver reads literal rpg-project charter/overlay paths; no copied policy |
| GitHub/Project 19 sole durable truth and checkpoint shape | #142 history + PIH-M1/PIH-4.1/PIH-6 | migrated fixture contract plus explicit-focus checkpoint/replacement evidence |
| nonblocking one-child worker and two-way messages | delivered PIH-3/4 + PIH-4.2/PIH-6 (`game-dev`) | fake RPC event/settlement tests; immediate owned notification; live one-child steer/follow-up proof |
| honest crash/restart boundary | PIH-3, PIH-4.1, PIH-6 (`game-dev`) | no-session launch + kill fixture; GitHub-only focus re-anchor or ask-Kirk |
| managed views/escalations | delivered PIH-4 + PIH-4.1 (`game-dev`) | fixture focus/group/cap/headless proof plus honestly scoped live status read |
| director mailbox/notification boundary | PIH-4.2 (`game-dev`) | fixture immediate owned RPC versus explicit/startup external refresh; no durable unread/push claim |
| repository-aware sibling dispatch | PIH-4.3 (`game-dev`) | role/repo/Project URL/root/origin/containment tests and game-dev-for-toolkit refusal probes |
| role-scoped Chrome safety | PIH-5, PIH-6 (`game-dev`) | policy/bridge tests and one durable Chrome artifact |
| deferred Blender safety | conditional post-retro `game-dev` issue only | Assets-only lease/pilot tests and evidence only if retro justifies activation |
| no daemon or second durable store | PIH-M1 through PIH-7 | game-dev verifier/static checks plus restart/recovery evidence |
| side-by-side evaluation and future scope | PIH-7 (`rpg-project`) | comparable evidence-based retro, not a synthetic benchmark |
| canonical documentation lifecycle | merged #136 history + #150/future corrections | fresh issue-backed corrections; no reopening/replacing #136 |

## 7. Explicit non-goals and stop conditions

The first proof stops rather than expands when it encounters any of the
following:

- a request for a long-lived daemon, listener, shared queue, second task
  database, session-derived recovery mechanism, or automatic multi-worker
  scheduler;
- a request to dispatch a sibling role through `game-dev`, infer a repository
  from Team/role text, bypass exact issue/Project URL/root/origin/containment
  checks, copy/replace canonical charters, move D&D rules into toolkit
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

Before the #150 reconciliation checkpoint/ready PR, verify:

- [ ] #136 is recorded accurately as merged history; #150 is the fresh ready,
  non-draft reconciliation review and is not a covert attempt to reopen #136.
- [ ] The design/plan agree on one lead plus one child, managed focus/mailbox
  visibility only, Chrome before Blender, no daemon/listener/store/auto-approval,
  and Kirk-only merge authority.
- [ ] PIH-M1/M2, PIH-3, and PIH-4 are recorded as delivered with their real
  issues/PRs and evidence limits.
- [ ] PIH-4.1 → PIH-4.2 → PIH-4.3 is explicitly ordered before PIH-5; every
  unit names game-dev files, tests, acceptance commands, live-evidence boundary,
  Project item, and historical/current links.
- [ ] Focus roots/descendants, mailbox fields/actions, immediate owned RPC versus
  refresh-only external activity, and ephemeral unread/re-anchor rules are
  precise and prohibit Pi/session/worktree recovery.
- [ ] Dispatch requires explicit owner/repository/issue identity, exact Project
  item URL match, role mapping, target root/origin/base/branch/worktree checks,
  physical containment, and canonical game-dev extension/charter paths.
- [ ] PIH-4.3 proves game-dev cannot substitute for toolkit and preserves the
  one-child guard; no real sibling-repository trial precedes it.
- [ ] PIH-5 does not imply native Pi MCP, Chrome core proof remains first, and
  Blender stays conditional post-retro with its hardened lease contract.
- [ ] The plan contains no runtime implementation, global Pi mutation, daemon,
  listener, second store, raw asset/GLB publication, or product approval claim.
- [ ] `git diff --check`, documentation consistency review, and staged-file
  review pass before commit.

## 9. Planned review questions for Kirk

1. Is managed focus plus a bounded ephemeral mailbox the right director-session
   operating surface before attempting a real sibling-repository task?
2. Does PIH-4.3's exact target-repository refusal contract make a one-child
   toolkit trial safe enough to authorize after its deterministic PR review?
3. Does the Chrome-only PIH-6 proof remain narrow while retaining Blender's
   conditional hardened destination and deferring concurrency decisions?

## 10. Final reconciliation gate for a future documentation PR

Before Kirk is asked to merge a future Pi-harness reconciliation PR, its signed
checkpoint and PR comment enumerate:

1. delivered history (#140/#142, game-dev#6/PR#7, #145/PR#146, game-dev#8/PR#9,
   game-dev#12/PR#13) and every PIH-4.1+ issue, Project item, branch, PR, and
   merged/not-merged state;
2. clean-machine/bootstrap/entry evidence that runtime remains in `game-dev`,
   resolves `game-dev/rpg-project` charters without policy copy, and never
   depends on the removed rpg-project runtime;
3. managed-focus and mailbox evidence: explicit roots/descendants, counts,
   bounded scope, refresh origin, ephemeral unread/re-anchor result, and what
   the UI did not prove;
4. repository-aware dispatch evidence including every game-dev-for-toolkit
   refusal, target root/origin/branch/worktree checks, and proof that exactly
   one child remains enforced;
5. deterministic verifier/test/typecheck results for each implementation PR;
6. deliberate-kill/replacement evidence and durable Chrome artifact, including
   what neither proved; conditional Blender evidence only if the retro justified
   its later issue;
7. the comparable Claude Code/OpenCode retro, observed failures, and deferred
   follow-up issues; and
8. confirmation that no daemon/listener/scheduler, second durable store, hidden
   focus/recovery dependency, global mutation, policy copy, extra Blender
   client, unsafe asset publication, auto-approval, or concurrency expansion
   entered scope.

Kirk decides whether that fresh documentation PR reconciles with delivered
behavior. If evidence is missing, an assumption is stale, or a dragon remains,
the PR stays open with the next action published visibly. No agent merges it or
reopens #136.
