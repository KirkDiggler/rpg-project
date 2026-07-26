---
name: Pi Team Harness
issue: rpg-project#135
status: design approved; Plan Review — canonical PR remains open through implementation
---

# Pi Team Harness — Design

## 1. Purpose, decision, and boundaries

This is the Pi sibling evaluation to the approved OpenCode Team Workflow
(`ideas/opencode-team-workflow/design.md`). It evaluates a **Pi-based responsive
team cockpit** alongside Claude Code and OpenCode; it neither replaces either
runtime nor revises the OpenCode decision to exclude an initial plugin, daemon,
and second durable store.

The harness makes a human-facing lead able to stay in a Pi conversation while
specialist workers make bounded progress asynchronously. It ports the existing
provider-neutral team policy, rather than copying a particular provider's agent
mechanics:

- canonical role policy remains `docs/teams/roles/**`;
- GitHub issues, PRs, and Project 19 are the durable task record;
- a human (Kirk) makes product decisions and merges;
- a worker is replaceable from GitHub alone; and
- a role's architectural boundary is more important than a prompt or tool
  permission that happens to permit it.

The initial deliverable is a **project-local Pi extension/package in
`game-dev`**, the portable travel entry point: clone `game-dev`, run
`./bootstrap.sh`, and resume without a missing workspace beat. `game-dev` owns
the cockpit runtime/package, bootstrap integration, entry/launcher behavior as
needed, and the clean-machine verifier. It is a runtime adapter and cockpit,
not a second policy store: it may hold process-local child handles and render a
live view, but it must not create a task database, daemon, queue service, or
local checkpoint ledger. Pi session JSONL and extension entries are private,
best-effort runtime history; they are never a recovery prerequisite or task
authority.

`rpg-project` remains the canonical cross-repo policy and design root: role
charters/overlays, Project 19 protocol, and this design/plan live there.
`game-dev/bootstrap.sh` already guarantees a sibling `rpg-project` clone. A Pi
process launched from `game-dev` discovers the local runtime under
`game-dev/.pi/extensions/`; that runtime resolves charter paths from the cloned
`game-dev/rpg-project` at execution time and never copies policy into
`game-dev`. A missing clone or canonical path is a visible hard-stop, not a
fallback prompt or a second policy store. Neither repository mutates global Pi
configuration, credentials, or trust defaults.

### Non-goals

This design does not:

- remove Claude Code or OpenCode, assume a cutover, or benchmark providers in
  the abstract;
- add an OpenCode plugin, daemon, or durable store;
- make Pi sessions, a local package cache, a worktree, or a TUI card durable
  task state;
- grant merge or product-decision authority to a lead, member, reviewer, or
  extension;
- replace repository CI, real-path tests, existing asset QA, human playtests,
  or deployment verification; or
- authorize a generic worker to operate Chrome DevTools or Blender.

The first proof is intentionally one lead, one member, one board-backed issue,
one replacement exercise, and one durable Chrome **or** Blender evidence
artifact. More workers, a shared server, cross-provider routing, and automation
beyond an observed failure are deferred to the retrospective.

## 2. Governing truth and task contract

### 2.1 One durable truth

Project 19 and GitHub are the sole durable execution surface:

| Durable fact | Authoritative location |
|---|---|
| goal, acceptance, ownership, discussion, blockers, checkpoints | backing GitHub issue |
| code/design review and committed artifact | branch and ready GitHub PR |
| queue state and cross-team visibility | Project 19 item and fields |
| human decision and merge | GitHub record made by Kirk |

Every executable task has a repository issue and Project 19 item before
assignment. The existing law remains unchanged: one issue per PR, no branch
without an issue, no issue without a Project 19 item, and fresh work starts
from current `origin/main`. A draft board card is not dispatchable.

A required checkpoint is a GitHub issue comment containing all four items:

1. completed work;
2. verification performed and its scope;
3. blocker(s), including the exact failed command or permission/auth prompt; and
4. the explicit next action and owner.

The comment also names the branch/PR when they exist. All agent-authored GitHub
text ends with `— <role>, on behalf of KirkDiggler`. Failing to publish a
checkpoint is a failed handoff, not a condition to hide in a local session.

`WORK SESSION STARTED` is the first checkpoint. A ready PR moves the board item
to **In Review**. A gate finding remains In Review and is returned to the
original implementing member; it is not silently reset to Todo. Kirk alone
changes the work to Done after the applicable review, merge, and (where
relevant) delivery verification.

### 2.2 Recovery is a first-class acceptance condition

A replacement worker starts by reading, in order:

1. the backing issue and its checkpoint comments;
2. the Project 19 item and fields;
3. the branch and ready PR (if present); and
4. the canonical charter, approved design/plan, and repository instructions.

That is sufficient to answer what is being done, why, who owns it, where the
work lives, what was verified, what failed, and what happens next. It must not
need a parent Pi session, supervisor memory, local TUI card, mailbox, worktree
path, or a terminated worker's context.

The replacement creates a fresh isolated worktree if the old one is unavailable
or dirty. It does not recover by overwriting another worker's worktree or by
assuming a private Pi session is correct. If GitHub publication itself is
blocked, the worker stops and reports the exact blocker through the available
GitHub surface; if no publication route is available, it reports the exact
failure to the human rather than continuing invisibly.

The same rule applies to cockpit focus. A replacement may re-anchor a managed
view only from an explicit GitHub root or a checkpoint's explicit GitHub focus
links; it never infers a focus set from Pi session JSONL, a previous TUI view,
a worktree, a Team-wide query, or an old child handle. If no such anchor is
visible, the cockpit asks Kirk to select one rather than guessing.

### 2.3 Design/plan artifact lifecycle

This artifact follows the existing design lifecycle: the worktree is only an
implementation detail. A design is committed on its issue branch, pushed, and
opened as a **ready, non-draft** `rpg-project` PR whose body explicitly says
`Review phase: Design Review`. It is reviewable, not `MERGE-READY`; that word is
reserved for the applicable independent product gate, not self-declaration.

**Lifecycle decision for this idea.** Kirk approved the design and chose a
long-lived canonical PR because implementation may expose dragons. PR #136
therefore remains open on `design/135-pi-team-harness`: after design approval,
`plan.md` is added to the **same branch and PR**, whose active body phase changes
to `Review phase: Plan Review`. Kirk reviews the plan before implementation.

Each executable implementation unit then has its own linked repository issue,
Project 19 item, branch, and ready PR; those PRs reference `rpg-project#135`
and #136 but do not close #135. An implementation discovery that changes this
design or plan is reconciled visibly by updating the canonical artifact on the
open #136 branch and linking the implementation PR/issue evidence. #136 merges
only after all linked implementation work is complete and a final reconciliation
confirms that the design, plan, issue, board, and evidence tell the same story.
This decision supersedes the ordinary separate-snapshot plan flow for this idea
only; it does not weaken ready review, human approval, or one-issue-per-
implementation-PR discipline.

## 3. Role and knowledge-boundary model

### 3.1 Policy first; Pi adapters second

Pi resources are thin adapters. An adapter names a model/tool profile, loads or
points to the canonical charter, and may provide a narrow cockpit command. It
does not paraphrase, fork, or silently amend the charter. Changing an identity,
lane, refusal rule, or done-gate changes the canonical provider-neutral
charter first and is reviewed as policy.

The standing-owner standard in `docs/teams/roles/README.md` remains binding:
owners carry a repository boundary across sessions, own issue-to-merge work and
living docs, push back on lane violations, and answer the four questions (goal,
pattern, real-path test, pushback) before claiming completion. The role that
performs work remains the final defense against architectural drift.

Pi separates **role policy** from **runtime profile**. A profile may later select
an available provider/model, thinking level, read/write tool allowlist, and
whether a role has an evidence adapter. It cannot confer authority that the
charter withholds. In particular, a permissive shell or MCP tool is not
permission to cross a repository, rules, review, or safety boundary.

### 3.2 Teams and leads

The human-facing teams are UI/UX, Platform, and Assets. Each has a corresponding
lead adapter that follows the director discipline within that team's scope:
triage with the human, ensure issue/board facts, dispatch the right role, read
returned evidence, surface decisions, and keep the visible story honest.

A lead is not an implementer. It does not investigate code inline, make product
decisions, commit, push, merge, or take over a member's repository lane. It can
use Project 19 and GitHub for coordination, but Kirk remains the final decision
maker and merger. The full cross-team director remains the cross-repo overlay;
a team lead cannot silently absorb cross-team authority.

| Team | Standing members / knowledge boundary |
|---|---|
| Platform | `rpg-toolkit-member`, `rpg-api-member`, `rpg-api-protos-member`, `rpg-deployment-member` |
| UI/UX | `rpg-dnd5e-web-member` with the UI/UX responsibility overlay |
| Assets | `rpg-game-assets-member` and `rpg-dnd5e-web-member` with the Assets responsibility overlay |
| Cross-team | human/director coordination; no invented generic implementation owner |

The web is one repository with two material responsibilities. Its shared
canonical charter remains the single render-and-call boundary; small
provider-neutral responsibility overlays distinguish UI/UX (screens, HUD,
accessibility, responsive presentation) from Assets (model loading,
environments, animation, 3D performance, visual seams). A task that materially
touches both is split into linked issues unless that would make one atomic change
artificial. Concurrent web work always uses separate worktrees.

### 3.3 Toolkit core and D&D rulebook overlays

The Pi harness must not turn a role profile into an accidental game-rule
boundary. `rpg-toolkit-member` owns the portable engine: core, mechanics,
tools, SDK shape, event/broker boundaries, and the rule that hosts receive
intent-level verbs rather than D&D calculations. `rpg-api` remains a thin
orchestrator by key; `rpg-api-protos` owns wire shape, never behavior; the web
renders server state and sends intent.

A **D&D 5e rulebook overlay** is loaded only for work that actually belongs in
`rulebooks/dnd5e`: class, condition, resource, equipment, or other D&D rule
semantics. It supplies domain vocabulary and test/evidence expectations, but it
cannot alter the core ownership rule:

```text
core / mechanics / SDK: reusable capability and clean interfaces
rulebooks/dnd5e: D&D meaning, loaders, and rule-specific behavior
api: persistence/orchestration by key, no D&D decisions
web: render pushed result and send reference/intention, no rules
```

A member refuses a brief that moves rulebook knowledge into an agnostic toolkit
interface, makes the API decide a rule, puts a calculation or game-state gate in
the web, or encodes behavior in a proto. The harness makes the relevant charter
and overlay visible at dispatch time; it does not attempt to enforce game design
with tool permissions.

### 3.4 Members, fixers, orientation, and curation

- **Standing members** persist across sessions for their repository. They advise
  and implement in one accountable lane, maintain `status.md`/`quality.md`, run
  local gates, self-review, and carry their issue through human merge.
- **Fixers** (`toolkit-fixer`, `api-fixer`, `web-fixer`) receive one bounded
  task and disperse. They do not acquire repository ownership, standing context,
  board authority, or a permanent worker allocation.
- **Explore** is read-only orientation. It returns evidence, uncertainty, and
  owning lanes; it cannot edit, branch, dispatch, decide, or claim completion.
- **Janitor** curates state pointers and Project hygiene but writes neither code
  nor architectural decisions. It follows the same issue/board/ready-PR
  discipline for its own markdown/context changes.

Persistent Pi processes are a convenience for standing members, not a new
status tier. A member remains a standing owner because of the charter and
accountability, not because a child process happened to stay alive.

### 3.5 Self-review, experience QA, and independent gate are different

These checks intentionally have different owners and powers:

| Check | Owner and timing | May change the PR? | What it proves / does not prove |
|---|---|---:|---|
| Self-review | implementing standing member or fixer, before handoff | Yes, own scoped work | checks charter, diff, repo pattern, tests, docs, and four-question done-gate; it is not independent review |
| Experience QA | designated domain member with the relevant human/evidence surface, before readiness as applicable | only if also the assigned implementer; otherwise routes findings | observes the specified player/asset experience and attaches scoped screenshots, recordings, console/network facts, or pipeline output; it does not make product decisions or substitute for CI/release QA |
| Independent gate | fresh-context `independent-gate` on a product-behavior PR | No | adversarial review of issue, diff, claims, tests, and evidence; can run read-only/reversible experiments; it never fixes, commits, pushes, merges, or changes Project fields |
| Human review/merge | Kirk | Yes, as human | product decision and merge authority; no agent self-declares it |

Workflow setup — including this documentation, role adapters, cockpit extension,
bootstrap/verifier work, and evidence-only Verify tasks — receives deterministic
checks and self-review, then Kirk's ready-PR review. It does not dispatch the
independent product gate. Product behavior (rules, contracts, player-facing
behavior, deployment/runtime behavior, or shipped asset output) receives the
normal review plus one independent gate under the existing gate charter.

A gate finding goes back to the original implementer. The same independent
reviewer performs a focused recheck after remediation; a full new audit is only
needed after material scope rewrite. `MERGE-READY`, when applicable, is a gate
verdict — never an implementer or design-worker claim.

## 4. Nonblocking worker lifecycle and two-way messaging

### 4.1 Dispatch state machine

```text
issue + Project 19 item verified
  -> lead posts WORK SESSION STARTED
  -> worker gets charter/overlay + issue/board/branch brief
  -> isolated worktree and replaceable Pi child started
  -> live progress/events visible in cockpit (ephemeral)
  -> worker publishes GitHub checkpoint (durable)
  -> ready PR + Project 19 In Review
  -> applicable QA/review/gate
  -> Kirk decides and merges
```

The lead may converse with the human throughout. Dispatch is nonblocking: after
starting a worker, the lead's Pi TUI returns to the human rather than waiting
for the worker to finish. The cockpit displays a compact live card such as
`running`, `waiting for human`, `blocked`, `checkpoint due`, or `exited`; it
also links the issue, PR, branch, and evidence. The card is a live display, not
a task tracker. Refreshing it reads GitHub and currently attached child events;
it does not reconstruct from a local database.

### 4.2 Process-local Pi architecture

A narrow project-local extension/package is sufficient for the first proof:

```text
human <-> lead Pi TUI (launched from game-dev)
             |
             | game-dev project-local extension: commands, runtime profiles,
             | live cards, process-local supervisor
             | resolves canonical charters from game-dev/rpg-project
             v
       child Pi process per worker (isolated cwd/worktree)
             |
             | stdin/stdout strict JSONL RPC
             v
      Pi RPC mode / SDK session + charter and scoped tools
             |
             +--> GitHub issue / PR / Project 19 (durable truth)
```

The extension may use Pi's extension API for commands, status/widget rendering,
and TUI overlays. A TUI overlay is presentation only; in non-TUI/RPC mode it
must degrade to commands and normal extension UI requests. The first cockpit
should use existing Pi components and compact status/widget views rather than a
new terminal application.

The supervisor is created only after a trusted project session begins, starts
only the worker requested by a valid dispatch, and closes its children on
normal session shutdown. It keeps child PID, RPC correlation IDs, last streamed
status, and current worktree path **in memory only**. It has no listener,
service registration, startup daemon, database, or surviving queue. Do not
start children in the extension factory, which Pi may execute without starting
a session.

The implementation may use either:

1. **replaceable RPC child processes** — `pi --mode rpc` over strict LF JSONL,
   preferred for isolation and a language-neutral supervisor; or
2. **a same-process SDK session** — only where direct typed control materially
   reduces prototype complexity.

The choice is deliberately replaceable. RPC is the default because its explicit
`prompt`, `steer`, `follow_up`, `abort`, state, and event protocol gives the
cockpit a clean process boundary. An SDK version must preserve the same
semantics and must not turn its session manager into durable dispatch state.

### 4.3 Two-way communication

**Worker to lead.** The supervisor consumes RPC lifecycle and tool events,
streaming output, queue state, and final settlement. It reduces those to a
live card; it does not claim durable completion from them. A worker publishes a
GitHub checkpoint at the defined handoff points. The lead may inspect the
checkpoint, rather than trusting a green-looking live card.

**Lead/human to worker.** The cockpit supplies explicit `steer`, `follow-up`,
`abort`, and `request checkpoint` actions. `steer` is for a correction that
should land after the current tool batch; `follow-up` is for work after the
agent settles; `abort` stops active work. Human input and decision requests are
visible in the TUI and, when they change task direction or block progress, are
also published to the issue. A lead cannot use messaging to smuggle unscoped
implementation work or override a charter refusal.

A permission/auth prompt is a hard stop. The worker reports its exact blocker
rather than silently waiting for an invisible dialog. The cockpit may show the
prompt state and direct the human to it, but it does not auto-approve actions.

### 4.4 Honest crash and restart boundary

A normal session shutdown can request child termination and release a local
Blender lease. A terminal crash, power loss, `SIGKILL`, or supervisor bug may
skip that cleanup and may leave a child or local session JSONL behind. The
design makes no false claim that an extension can guarantee cleanup across that
boundary.

After any interruption, the live card and process handle are discarded. A
replacement begins from GitHub (§2.2), verifies whether a branch/PR/checkpoint
exists, and creates a new worktree/process as needed. An orphan discovery may
be shown as a local safety warning, but it is not task truth; a human can stop
it before replacement. For Blender, the human-visible Connect/Disconnect state
and single-client procedure remain the safety backstop.

The deliberate-kill prototype test demonstrates this boundary: kill a worker
only after it has published a checkpoint, then have a fresh worker complete the
next action using GitHub/Project/branch facts alone. Any information that was
needed but absent becomes a design failure to fix in the checkpoint contract,
not an excuse to recover from a hidden session.

## 5. Project 19 as the human cockpit protocol

Project 19 remains the shared human/agent coordination protocol. Its current
Status, Team, Feature, and Kind fields remain the common language; the harness
reads and updates them through GitHub, not a mirrored local store.

### 5.1 Managed-session focus and human-oriented views

The default is **Managed this session**, not every row assigned to the
director's Team or every Project 19 item. A lead explicitly adds one or more
GitHub-backed focus roots: a backing issue, linked PR, or a named
initiative/section represented by its scoped GitHub/Project 19 URL. A bare
Project URL is not a focus root; broad Project inspection is explicit instead.
A successful human-confirmed dispatch attaches its target issue to this
in-memory set. The cockpit
also projects only the root's explicit GitHub-linked PR/checkpoint descendants
and explicitly linked background issues/PRs; it does not discover a queue from
Team membership, issue text guesses, or local session history.

A focus root records only its URL, why it is present (`Kirk`, `dispatch`, or
`GitHub link`), and the source URL that proves the relationship. That state is
presentation/control memory for the current lead session, never a local task
record. When a checkpoint intentionally hands off or coordinates multiple
background items, it names its root and managed GitHub issue/PR links so a
replacement has an explicit durable re-anchor. No checkpoint link means no
inferred descendant.

The commands and equivalent TUI controls are:

- `/team-focus add <GitHub issue|PR|scoped Project URL>` — validates the live
  GitHub root (and rejects a bare Project-wide URL) before adding it for this
  session;
- `/team-focus remove <root>` — removes only that ephemeral root, never a
  GitHub item;
- `/team-focus inspect` — shows each root, attachment reason, explicit
  descendants, source URLs, and projection counts; and
- `/team-status [managed|team|project]` — defaults to `managed`; `team` and
  `project` are visibly labelled, opt-in broad inspection modes and never
  silently replace session focus.

The managed projection supports one initiative/section or several concurrently
tracked issue/PRs, including background work, without claiming that each row is
a child process. PIH-3 still supervises at most one worker in the prototype;
focus neither starts workers nor schedules, routes, retries, or restores them.

The cockpit presents bounded query views, not new workflow state:

- **Managed now:** focused roots/descendants grouped by Todo / In Progress / In
  Review, with issue, assigned role, latest visible checkpoint, blocker, PR,
  and attachment reason.
- **Needs a human:** only managed, operational active/review/recovery
  escalations (decision, auth/permission, scope/charter, failed/ambiguous check,
  evidence, recovery, or merge). Repeated missing required facts are grouped by
  kind and state with a count and representative links; they are not one noisy
  row per unrelated board item.
- **Review queue:** only managed In Review PR/item rows, review phase, gate/QA
  status, unresolved findings, and next named reviewer.
- **Recovery queue:** only managed items with a GitHub-reported crash, stale or
  missing checkpoint, or an explicit replacement handoff. A lost local child
  after restart is not sufficient evidence; recovery reconstructs from the
  root/issue/Project/branch/PR/checkpoint facts or asks Kirk to re-anchor.
- **Cross-team seams:** focused explicitly linked issues with different Team
  fields, especially toolkit/API/proto/web sequence and the UI/UX–Assets seam.

Each section has a fixed ten-row limit, grouped aggregate counts, and a visible
“showing N of M”/inspect path; truncation never hides the total or pretends
omitted rows are resolved. Views always show source URL and refresh
timestamp. If GitHub cannot be read, the cockpit says stale/unavailable and
discards rows rather than retaining a local snapshot as truth.

### 5.2 Escalations

The following become visible human/lead escalations on the issue and cockpit:

- a missing issue or Project 19 item before requested dispatch;
- a charter refusal or cross-team ownership conflict;
- a product/design decision required from Kirk;
- an authentication or permission prompt;
- failed/ambiguous deterministic check, especially a suspicious green after a
  confusing failure;
- evidence that only exercises a fixture or bypass rather than the player path;
- a stale worker checkpoint, worker crash, orphaned local process, or worktree
  collision; and
- request for Chrome evidence or the exclusive Blender lease.

The lead reports facts and a recommended next action. It does not silently
resolve a human decision, reassign another team's work, or invent a local
status to make the board look clean.

## 6. Worktree and repository isolation

Every implementation, design, QA mutation experiment, and independent product
gate that needs a checkout uses an isolated worktree. The worker begins from
fresh `origin/main` on an issue-named branch and works only in its assigned
path, for example:

```text
/home/kirk/game-dev/.pi-worktrees/<repo>-<issue>-<role>
```

The harness records that path only as live process metadata; the durable branch
name and PR URL appear in GitHub checkpoints. It never edits an existing dirty
main checkout, another worker's worktree, raw licensed asset staging, or an
unrelated branch. A collision means select/create another isolated path and
report it; never overwrite.

Only the worker owning the issue creates commits and pushes the scoped diff. It
stages the intended file(s), never uses `git add -A` or `--no-verify`, runs its
repository gates, self-reviews, and opens the ready PR. Leads coordinate; gates
review without fixes; Kirk merges. An independent gate uses a worktree distinct
from the implementer's and restores any reversible mutation experiment before
reporting.

## 7. Evidence and role-scoped tools

### 7.1 Evidence is a claim with scope

Evidence is attached or linked from the issue/PR and says exactly what it
observed, its environment, command/route, and residual uncertainty. A screenshot
or a passing fixture does not round up to real-path behavior. Existing repository
checks remain authoritative for their lanes: CI and real-path tests for product
behavior, deployment verification after applicable merges, and asset-pipeline
QA/headless renders/in-game screenshots for shipped assets.

The evidence owner is chosen by the claim:

| Claim | Evidence owner |
|---|---|
| toolkit rule / API / proto behavior | owning Platform member; independent gate reviews product PRs |
| player-facing interaction and presentation | UI/UX web responsibility with human-aware experience QA |
| 3D asset/render seam | Assets web responsibility and game-assets member |
| deployed behavior | deployment member after human merge |
| board/state hygiene | Janitor or lead, without claiming product verification |

### 7.2 Chrome DevTools / browser evidence

Chrome tools are **not** a universal worker capability. The harness scopes them
to the designated experience-QA role or assigned web/Assets member on a
board-backed evidence task. A lead requests and reviews the evidence; Explore,
Janitor, generic fixers, and independent gates do not inherit Chrome control
merely because they are Pi children.

Chrome use is limited to the named local/dev target and the test steps in the
issue. The operator captures the relevant screenshot/recording plus concise
console/network/state observations, labels whether the real player path was
exercised, and links the artifact in the issue/PR. It must not exfiltrate
credentials, operate unrelated browser tabs, mutate production data, or claim
that an MCP/browser fixture is a player-path pass when it bypassed the game.
A human remains in the loop for the playtest/evidence judgment where the role
cannot access the required surface.

The initial Pi package does not need to invent a browser protocol. It may expose
a deliberately scoped adapter to the already-approved local Chrome DevTools
surface, or invoke the existing browser evidence harness. Tool availability is
selected per child profile and remains disabled by default.

### 7.3 Blender MCP: Assets-only, one exclusive lease

Blender MCP is full local control of the active GUI scene, not a sandbox. It is
therefore available only to an explicitly assigned Assets worker under the
`rpg-game-assets-member` / Assets-web responsibility boundary, after a
board-backed issue requests it. It is never inherited by a lead, generic web
worker, fixer, gate, or second concurrent Pi child.

The cockpit models a **process-local exclusive Blender lease**:

```text
unleased -> human grants named worker -> active (exactly one client) ->
verify/disconnect -> released
```

The lease is a safety control and live display, not durable task state or a
claim of protocol-level locking. On normal shutdown the supervisor asks the
client to disconnect and clears the local lease. On a crash, the human checks
the active Blender GUI, disconnects any surviving client, and records the
incident/checkpoint before granting a replacement. The existing Blender UI's
Connect/Disconnect state and the one-active-client operating procedure remain
the definitive concurrency backstop.

The assigned worker follows the hardened pilot exactly:

- use the pinned loopback-only server/add-on, telemetry and cloud integrations
  disabled, with Blender at `localhost:9876`;
- save before significant work; inspect before editing; validate first on a
  disposable scene; make only explicitly requested, small reversible edits;
- retain the existing headless render, mesh/animation, pipeline QA, and in-game
  screenshot gates as release truth;
- do not commit raw Synty source or converted/promoted GLBs to a public repo;
  and
- disconnect immediately when access is no longer needed.

A connection, timeout, screenshot, or unexpected-scene failure means preserve
the scene, disconnect, do not patch upstream or alter pipeline assets, and
record the exact add-on revision, MCP package/Blender versions, command, and
error. A Blender capture is evidence of the active scene only; it is not export
or release verification.

## 8. Pi extension, SDK, RPC, and TUI shape

Pi intentionally supplies primitives rather than a built-in multi-agent
control plane. This design uses those primitives without pretending otherwise.

| Layer | Initial responsibility | Explicit limit |
|---|---|---|
| `game-dev` project-local extension/package | runtime profiles; dispatch validation; process-local supervisor; GitHub-backed commands; live TUI cards; resolves `rpg-project` charters by path | no durable queue/store/daemon, policy fork, or global Pi mutation |
| Pi extension API | commands, custom tools, `session_start`/`session_shutdown`, status/widgets, TUI overlay, tool-call safety interception | extensions run with full local privileges; permissions supplement but do not replace charter rules |
| Pi RPC child | isolated per-worker process, JSONL events, prompt/steer/follow-up/abort, child model/session | local session history is non-authoritative and disappears from recovery assumptions |
| Pi SDK alternative | typed same-process session only if prototype warrants it | must preserve child-equivalent isolation and not persist dispatch state |
| Pi TUI | human lead conversation, compact live cards, command palette, approval/escalation views | not a browser UI, durable board, or background service |

The `game-dev` project-local extension loads only after Pi's existing
project-trust flow when Pi is launched from `game-dev`; its optional entry script
therefore changes cwd to that root and forwards arguments rather than launching
Pi from a sibling repository. Child workers use the already-selected runtime
source explicitly when their worktree cwd differs, while their charter resolver
still reads the canonical `game-dev/rpg-project` paths. Its source is reviewed
in `game-dev` like any other full-privilege code. The implementation avoids
starting timers, sockets, watchers, or child workers in the extension factory;
it starts resources after `session_start` or an explicit command and uses
idempotent `session_shutdown` cleanup.

RPC integration uses strict LF-delimited JSONL and correlation IDs. The
supervisor distinguishes `agent_end` from `agent_settled` so a retry,
compaction, or queued follow-up is not displayed as completed. Live output is
truncated for the lead view; full durable conclusions belong in the GitHub
checkpoint, not a huge terminal transcript. Inbound commands preserve Pi's
semantics: `steer` after the current tool batch, `follow_up` after settlement,
and `abort` only when a human/lead explicitly requests it.

TUI scope is deliberately modest: use a status/footer indicator plus compact
widget for active workers and managed-focus counts; open an overlay for dispatch
detail, focus add/remove/inspect, message actions, and scoped escalations.
Custom components must work within the actual terminal width, use Pi's supplied
theme/keybindings, and be optional. The same design must be usable through
normal commands when the runtime is RPC/headless, where custom TUI components
are unavailable.

Tool profiles begin least-privilege: Explore is read-only; Janitor is limited to
its curation surface; a member receives only its repository/worktree tools;
Chrome is an evidence-profile opt-in; Blender is an Assets-only lease opt-in.
The harness may block obvious out-of-scope paths or destructive commands through
extension interception, but tool controls are defense in depth. The canonical
charter and visible GitHub audit trail remain the authoritative safeguards.

## 9. Narrow prototype and retro

### 9.1 First proof

Select one real, sufficiently small Project 19 issue with one standing member
and one team lead. Before dispatch, the issue contains its observable goal,
acceptance, team/feature/kind fields, owner, evidence need, and next action.
The proof must demonstrate all of the following:

1. The lead starts a single role-scoped Pi worker from an isolated worktree
   without blocking its human Pi conversation.
2. The worker receives the canonical charter (and relevant responsibility or
   D&D overlay), posts `WORK SESSION STARTED`, performs its scoped work, and
   publishes a complete checkpoint.
3. Lead-to-worker `steer` or `follow-up` and worker-to-lead live progress work
   while the process is alive; the material outcome is then published to
   GitHub.
4. After a checkpoint, the worker is deliberately killed. A fresh worker
   reconstructs solely from the issue, Project 19, branch, PR, and checkpoint,
   then takes the explicit next action.
5. The task produces one durable evidence artifact: a scoped Chrome
   player/experience artifact **or** a Blender pilot artifact. Chrome is the
   preferred first choice for a UI task; Blender is allowed only for a genuine
   Assets task and includes the exclusive-lease procedure.
6. The resulting ready PR identifies its review phase, moves the item to In
   Review, receives the checks appropriate to its scope, and is left for Kirk's
   decision. No agent merges or calls its own work `MERGE-READY`.

The proof should choose a workflow/setup or small real feature task whose
acceptance can be observed without widening to multi-worker routing, a daemon,
or an entire cross-repo wave. A managed-focus view may show several
GitHub-linked background issue/PR rows, but that is not multi-worker live proof
or a scheduler claim. If the task is product behavior, the existing
independent-gate policy applies; if it is workflow setup, deterministic checks,
self-review, and Kirk's review apply.

### 9.2 Success criteria

The prototype succeeds only if:

- GitHub alone supports the kill-and-replace recovery without lost task facts;
- the board, issue, PR, and checkpoint tell a consistent story;
- the human can continue conversing with the lead during worker execution;
- the worker stays in its charter and responsibility overlay, and any pushback
  is visible;
- the selected evidence is attached/linked, scoped honestly, and does not
  replace the established release gate;
- Chrome access is limited to the evidence owner, or Blender has exactly one
  active client, a saved/validated scene procedure, and a released lease;
- no dirty main checkout or another worker's worktree is touched;
- the supervisor can disappear without becoming a recovery dependency; and
- Kirk retains product and merge authority.

### 9.3 Retrospective and side-by-side evaluation

The retrospective, not design-time preference, decides whether to continue.
Run the Pi proof alongside comparable Claude Code and OpenCode work rather than
assuming any runtime wins. Compare a similarly scoped, board-backed task and
record evidence on the issue/retro surface for:

- time and friction from issue to visible `WORK SESSION STARTED`, checkpoint,
  ready PR, and reviewed outcome;
- quality and completeness of GitHub-only recovery after deliberate replacement;
- human interruption responsiveness and clarity of two-way messaging;
- boundary adherence, self-review quality, QA/gate findings, and false-green
  detection;
- worktree collisions, permission/auth friction, orphan process/lease behavior,
  and any hidden-state failure;
- evidence quality and whether it exercised the real claimed path;
- operator cognitive load: can Kirk see who is doing what, what is blocked, and
  what requires a decision without reading terminal logs; and
- observed model/runtime cost and latency in context, not as a synthetic
  leaderboard.

The retro records what failed, what was awkward, what was genuinely better, and
whether the smallest next change is justified. Only observed failure modes may
justify a future persistent service, richer coordinator, additional automation,
or broader role/tool profile. There is no automatic expansion or cutover.

## 10. Review questions for Kirk

1. Which existing, real Project 19 issue is the smallest useful first proof,
   and should its evidence artifact be Chrome (default for UI/UX) or Blender
   (only if the work is genuinely Assets-scoped)?
2. Is replaceable RPC child-process supervision the preferred initial mechanism,
   with same-process SDK sessions deferred unless the prototype exposes a
   concrete need?
3. Which comparison task(s) should represent the Claude Code and OpenCode sides
   so the retro judges workflow evidence rather than unlike scopes?

These are sequencing/evaluation choices, not invitations to weaken the
charters, GitHub-only recovery contract, exclusive Blender procedure, or
human merge authority.
