---
name: OpenCode Team Workflow
description: Port the human-centered UI/UX, Platform, and Assets team workflow to OpenCode as a provider-neutral control plane, side-by-side with Claude Code, without duplicating Project 19 as task state
status: approved (2026-07-20) — implementation planning follows in a separate session
---

# OpenCode Team Workflow — Design

Approved design for `rpg-project#101`. This document is the complete,
already-approved spec — implementation planning (`plan.md`) is a separate,
later session; nothing here is a placeholder or an open question.

## 1. Purpose and core architecture

OpenCode becomes a **provider-neutral control plane** while Claude Code
remains operational **side-by-side** during rollout — this is not a
migration-and-cutover, it's a second control plane proven against the same
work before any decision to consolidate.

The port is of **workflow semantics**, not Claude Code's literal team/mailbox
internals: the director/standing-member/fixer/janitor/gate discipline, the
Project-19-is-truth discipline, the four-question done-gate. Those transfer.
Claude Code's specific mailbox/dispatch plumbing does not need to.

**Human-centered team pods.** Three pods — **UI/UX**, **Platform**, **Assets**
— each with a human in its collaboration loop. Kirk occupies multiple pods
today; the structure is designed so future humans can each own a pod aligned
with their strengths without redesigning the workflow.

**Two roots, unchanged relationship:**
- `rpg-project` is the **canonical cross-repo coordination root** — designs,
  plans, role charters live here. Humans start coordination sessions here.
- `game-dev` remains **portable workstation bootstrap and asset tooling.** It
  clones/verifies `rpg-project` (added to its idempotent clone/verify set) but
  does **not** duplicate team policy — no parallel `game-dev/docs/team-workflow`.
  `game-dev` is **not** a newly invented standing team pod — it stays
  bootstrap/tooling. Work filed against `game-dev` itself (the bootstrap
  script, the browser harness, the ingest script) is **Cross-team** or
  **Assets** on the Project 19 board depending on the issue's actual scope,
  never a fourth pod.

**Project 19** (`KirkDiggler`, "The Dungeon Run") is durable execution/task
state. OpenCode sessions are replaceable workers: a worker can be killed and a
different worker can pick up the same task from GitHub state alone, with no
hidden local session dependency.

## 2. Project 19 contract

**Existing fields (unchanged):**

| Field | Values |
|---|---|
| Status | Todo / In Progress / In Review / Done |
| Team | UI/UX / Platform / Assets / Cross-team |
| Feature | Party Assembles / Class Kits / The Dungeon / Game Screen / Capstone / Shelf / Infra |
| Kind | Build / Fix / Verify / Learn / Decide |

**Issue-backing rule.** Every executable task is backed by a repository issue
— Project draft items have no durable comment stream, so a draft item cannot
be dispatched against. Drafts may remain as milestones/placeholders; create or
link an owning issue **before** dispatch, every time.

**Repo law, preserved unchanged:**
- One issue per PR.
- No branch without an issue.
- No issue without a Project 19 entry.
- Fresh issue work starts from a fresh `main` branch.

**Lifecycle:**

```
triage (fields + acceptance + owner)
  -> In Progress
  -> WORK SESSION STARTED (visible marker on the issue)
  -> checkpoints (see §3)
  -> ready PR + In Review
  -> independent gate (Sol, see §4)
  -> findings routed back to implementer  OR  MERGE-READY
  -> human merge (Kirk)
  -> deployment verification where applicable (watch the deploy run to terminal
     success — merge != shipped)
  -> Done
```

**Signature.** Any agent GitHub activity (issue/PR comments) rides Kirk's
account; every such comment ends with a role signature: `— <role>, on behalf
of KirkDiggler`.

### Design/plan artifact lifecycle — never local-only

A **worktree path is an implementation detail, not a handoff or discovery
mechanism.** Once many worktrees exist across leads/members/gates, "read my
local file" stops being a usable review surface — nobody else can find, let
alone diff-review, a path that only exists in one worker's isolated
filesystem. Design and plan artifacts (`design.md`, `plan.md`) therefore
**never remain local-only as a review gate**, even though they're plain
markdown files in `rpg-project`, not code:

```
design committed locally on its issue branch
  -> push the issue branch, open a READY PR (never a draft)
  -> PR body states the active review phase explicitly
     ("Review phase: Design Review")
  -> PR URL published as an issue/Project checkpoint comment
     (the durable pointer a fresh session finds, not a local path)
  -> review happens on GitHub's file/inline-comment surface
  -> written design approval (Kirk) and human merge: durable design snapshot
  -> implementation plan committed on its own issue branch
  -> push and open a READY PR with "Review phase: Plan Review"
  -> publish that PR URL as the next issue/Project checkpoint
  -> written plan approval (Kirk) and human merge: durable plan snapshot
  -> implementation starts only after the approved plan snapshot merges
```

- **Ready, not draft.** This preserves the platform's existing no-draft
  convention: a PR is opened ready-for-review as soon as there's a reviewable
  diff. Agent-to-coordinator checkpoint comments on the issue are the *early*
  planning handoff (informal, before a PR exists or between PR revisions);
  the ready docs PR is the actual **human review surface** once content
  exists to review.
- **Reviewable is not MERGE-READY.** A ready PR is open for comment the
  moment it's pushed, but it is never called MERGE-READY by the agent that
  opened it — that call is reserved for whichever review/gate the phase
  requires (here: Kirk's written approval), exactly as an implementation PR
  is never self-declared MERGE-READY ahead of its gate (§2 lifecycle above).
- **Design and plan may merge as separate reviewable snapshot PRs.** Each
  artifact gets its own ready GitHub review surface and written approval when
  it is created after the preceding snapshot has merged. A follow-up plan PR
  is not a weaker review surface; it is the durable plan snapshot in git
  history. Combining both artifacts in one open documentation PR remains
  allowed only when both are ready for their respective review phases before
  the design snapshot merges.
- **`rpg-project#101` is the design/plan tracker, not an umbrella issue.**
  Once the plan is written and approved, *it* creates and links the separate
  per-repo implementation issues that actually carry PRs — `#101` is never
  reused as the thing every implementation PR closes. This preserves
  one-issue-per-PR: each implementation PR closes its own linked
  implementation issue, never `#101` directly.
- **Post-merge drift is corrected in place, not silently.** If an
  implementation PR discovers that a merged assumption in `design.md`/
  `plan.md` was wrong or incomplete, that implementation PR updates the
  canonical doc when it includes the relevant `rpg-project` documentation
  change. Otherwise it opens a **linked docs follow-up** in `rpg-project`.
  Either way, the original merged snapshot stays visible in git history
  (nothing is force-rewritten); the live doc content is what stays honest and
  current.

## 3. Continuity and persistence

Two layers of continuity, not one:

- **Live continuity (cheap, best-effort):** resume an OpenCode `task_id` when
  the same session/process is still available. This is a convenience, never a
  requirement.
- **Durable continuity (the real contract):** issue, PR, branch, and a
  structured checkpoint comment — nothing else. A worker replacement must be
  able to reconstruct **solely** from these; it must never require inheriting
  hidden local session state from the terminated worker.

**Checkpoint contract.** Every checkpoint records: completed work, verification
performed, blockers, and the explicit next action. A checkpoint that fails to
publish to GitHub **is a failed checkpoint** — not a soft-fail to retry
silently later. An auth prompt or a human-decision-needed point becomes an
explicit, visible blocker in the checkpoint, never a silent stall.

**Authority.** Project 19 + GitHub is the authoritative task state, full stop.

**What each existing artifact keeps doing — nothing new is invented to duplicate it:**
- `sessions/active.md` stays a **thin, current, cross-team cold-start
  narrative** — it is not, and does not become, a second task tracker.
- Existing role `context/*.json` and `field-notes.md` retain **durable
  operating lessons** (the instinct-and-failure-pattern catalog) — unaffected
  by this design.
- An idea's `memories.json` is available for architectural
  decisions/blockers/test-criteria when a session finds it useful — it is
  **not mandatory duplicated task state**; the issue/PR/board triad remains
  authoritative even when a `memories.json` entry exists alongside it.

**No second database, daemon, or orchestration store.** This is a hard
boundary, not a starting default that might change: GitHub + the board is the
entire durable-state surface for this design.

**Same-beat propagation.** A decision propagates to the issue, the PR, and the
board **in the same work beat** it's made — never staggered — so that a cold
session starting from any one of the three never inherits a stale story from
the other two. This generalizes the existing director discipline ("propagate
decisions everywhere visible") to OpenCode workers.

## 4. Human-centered roles and reconciliation with the existing roster

**Primary human-facing OpenCode roles:** `ui-lead`, `platform-lead`,
`assets-lead` — one per pod. Each adapts the existing **director charter**
(`docs/teams/roles/director/prompt.md`) within a single team lane: collaborate
with the pod's human, triage, dispatch, verify, coordinate. Exactly like the
director, a lead does **no** implementation, commit, push, merge, or hands-on
bug investigation — that discipline is the entire reason the director role is
reliable, and it carries over unchanged. The human alone makes final product
decisions and merges.

**Standing expert ownership is retained, not replaced.** The design
deliberately does **not** collapse the roster into one generic implementer —
expert ownership of each repo's boundary is load-bearing (see
`docs/teams/roles/README.md` §"the expert-ownership standard"). This design
also closes two roster holes the existing Platform-only roster left open: it
adds a canonical standing member for `rpg-deployment` and for
`rpg-game-assets`, neither of which had an owner before.

| Pod | Standing members |
|---|---|
| Platform | `rpg-toolkit-member`, `rpg-api-member`, `rpg-api-protos-member` (existing charters, unchanged) + **`rpg-deployment-member`** (new, see below) |
| UI/UX | **`ui-web-member`** (new OpenCode adapter — see "The web-ownership seam," below) |
| Assets | **`rpg-game-assets-member`** (new, see below) + **`assets-web-member`** (new OpenCode adapter — see "The web-ownership seam," below) |

### The web-ownership seam — resolved

`rpg-dnd5e-web` is one repo whose work is split across two pods (UI/UX:
screens, HUD, UX flows, accessibility, responsive Discord viewports,
fixture-to-live presentation; Assets: model loading, props/environment
rendering, animation playback, 3D performance/evidence, appearance seams).
This is decided, not deferred to planning:

- `docs/teams/roles/rpg-dnd5e-web-member/prompt.md` **stays the canonical
  shared web/repo boundary charter** — the render-and-call boundary rule, the
  hard rules, the four-question gate all live there, once, for the whole repo.
- Implementation adds **two small vendor-neutral lane overlays** under that
  role (or an equivalently clear adjacent canonical location, e.g.
  `docs/teams/roles/rpg-dnd5e-web-member/overlays/{ui-ux,assets}.md`):
  - **UI/UX overlay** — owns screens, HUD, UX flows, accessibility, responsive
    Discord viewports, fixture-to-live presentation.
  - **Assets overlay** — owns model loading, props/environment rendering,
    animation playback, 3D performance/evidence, appearance seams.
- OpenCode's `ui-web-member` and `assets-web-member` are **thin adapters**:
  each loads the shared charter **plus** its applicable overlay. Neither
  adapter duplicates shared web law (the render-and-call boundary, the hard
  rules) — that stays single-sourced in the shared charter.
- Work that touches both lanes is **split into linked issues**, or explicitly
  coordinated as one task with both adapters named in the issue if a split
  would be artificial. Either `ui-web-member` or `assets-web-member` working
  concurrently in the same repo uses **isolated worktrees** — the existing
  worktree-collision rule (§7) applies to this seam like any other.
- This replaces any notion that implementation planning picks the shape later
  — the shape is the shared-charter-plus-overlay pattern above, decided here.

### New standing members closing roster holes

- **`rpg-deployment-member`** (Platform) — owns deployment/config/release
  correctness for `rpg-deployment`: the auto-deploy pipeline, `nginx-http.conf`
  and other live config, release sequencing across the platform repos, and
  **post-merge deployment verification** (watching the "Deploy RPG Platform"
  run to terminal success — merge != shipped, per §7). Respects service
  boundaries: it deploys and verifies delivery — it does not own toolkit/API/
  proto business logic, and it pushes back the same way every other standing
  member does if a brief asks it to fix application code instead of
  deployment config.
- **`rpg-game-assets-member`** (Assets) — owns the private `rpg-game-assets`
  library and its harness contract: Synty license boundaries (never commit
  raw source assets or converted GLBs to a public repo), conversion/promotion
  quality (FBX → GLB pipeline output), the `harness/models/synty/` contract
  tree, manifests, performance budgets for shipped assets, and the handoff
  point into `rpg-dnd5e-web` (where `assets-web-member` picks up the client
  seam). It does not own web-side rendering code — that boundary is the
  contract-tree handoff, matching the existing `npm run assets:sync` flow.

**Standing members persist issue PR-to-merge**, own their living docs
(`status.md`/`quality.md`), enforce repo boundaries, and refuse invalid briefs
— unchanged from the existing charters.

**Fixers become task-scoped Terra workers.** The existing
`toolkit-fixer`/`api-fixer`/`web-fixer` charters map to OpenCode Terra workers:
dispatched for one scoped task, disperse when done, carry no standing
ownership. Unchanged in spirit; the OpenCode adapter binds the model (§5).

**Shared read-only exploration role retained** — the Explore/orientation
pattern the director already uses to reconcile a stale handoff against reality
without pulling the whole world into its own context. Maps to a Luna worker
(§5).

**Janitor maps to Luna.** The existing `janitor` charter (curates handoff, role
context, structured memory, board hygiene; writes no code, makes no design
decisions) is unchanged and binds to a Luna model (§5).

**Deprecated `project-manager` stays retired** — Project 19 already owns
tracking; nothing in this design revives it.

**`platform-simplifier` is deferred, not deleted.** It activates only after a
real need is observed **and** its knowledge prerequisites (the per-repo
`.claude/knowledge/` corpus it reads — see `ideas/team-memory-system/design.md`)
exist. This design does not activate it.

**Independent gate — fresh-context Sol reviewer.** Works in an **independent
worktree** (never the implementer's), adversarially reviews the diff, the
claims, the tests, and the evidence — may conduct **reversible** mutation
experiments to probe a claim (e.g., temporarily breaking a code path to
confirm a test actually catches it, then reverting). The gate **never fixes**
what it finds and **never commits or pushes**; findings route back to the
**original implementer**. A **fresh** regate follows remediation — the same
gate re-running on the same context is not an independent regate.

**The four-question done-gate applies to every completion**, human-facing lead
included, unchanged from the existing standard:
1. **Goal** — does observable behavior match the task's goal sentence?
2. **Pattern** — did the work follow the repo's existing patterns?
3. **Test** — proven on the real path, not a stub/fixture bypass?
4. **Pushback** — did anything in the brief conflict with the lane or standing
   rules? Say so.

**Domain lanes (what each pod actually watches for):**
- **UI/UX** — presentation, interaction, accessibility, responsive Discord
  viewport, visual evidence (screenshots/recordings as the artifact of record).
- **Platform** — toolkit/API/protos/deployment correctness, wire-state
  correctness, cross-repo sequencing (proto before API before web, per the
  existing boundary rule).
- **Assets** — asset pipeline, license boundaries, manifests, 3D client seams,
  animation, performance, multi-angle evidence.

## 5. Initial GPT model profile

Model bindings are a **separate axis from role policy** (§4) — this profile
can be swapped for Claude/Gemini/open-source models later, after the GPT
proof, without touching any role's charter or duties.

| Role class | Model | Variant |
|---|---|---|
| Team leads (`ui-lead`, `platform-lead`, `assets-lead`) | `openai/gpt-5.6-sol-fast` | `xhigh` |
| Independent gate | `openai/gpt-5.6-sol` | `max` |
| Standing members / fixers (Terra workers) | `openai/gpt-5.6-terra` | `high` |
| Explore / research | `openai/gpt-5.6-luna` | `medium` |
| Janitor, title/summary/compaction chores | `openai/gpt-5.6-luna` | `low` (may be raised when judgment requires it; `low` is the initial binding, not a ceiling) |

**Sol / Terra / Luna are capability/cost categories**, not separate
intelligence tiers layered on top of variant. `-fast` targets the **same** API
model with priority service (`serviceTier: priority`) — it buys latency, not
different reasoning quality. Reasoning depth is controlled independently by
the **variant** column.

## 6. Configuration and source of truth

**Project-scoped OpenCode setup lives in `rpg-project`:**
- `opencode.jsonc` — project config.
- `AGENTS.md -> CLAUDE.md` — a **Git symlink** (accepted Linux/WSL-only
  constraint; this design does not attempt a Windows-native equivalent).
- `.opencode/agents/` — the thin role adapters (model + permission bindings).
- `.opencode/skills/` — project-scoped skills.

**Charters stay canonical; adapters point, they don't copy.** Every
`docs/teams/roles/**/prompt.md` charter remains the single source of truth for
a role's identity and duties. An `.opencode/agents/` file is a thin binding
(model, variant, permissions, a pointer to the charter path) — never a copied
or paraphrased charter. A charter edit is a one-place edit that both Claude
Code and OpenCode pick up.

**No duplicate policy store.** Vendor-neutral board/gate/team policy stays in
the existing `rpg-project` docs/role system. This design explicitly does
**not** create a `game-dev/docs/team-workflow` (or any other) second source of
truth for that policy.

**Root `CLAUDE.md` becomes tool-neutral where practical**, refactored to stay
current for Project 19 without assuming Claude Code as the only reader.
Tool/provider-specific detail (OpenCode config, agent bindings) lives in
configuration/adapter files, not in the shared narrative doc.

**`opencode.jsonc` boundaries:**
- Does **not** modify `~/.config/opencode/opencode.jsonc`, provider
  credentials, or global work settings.
- **Disables inherited work-only MCPs** at project scope while **retaining**
  relevant browser tools (the MCP playtest/evidence-capture surface stays
  available).
- Credentials remain local/uncommitted — no credential material in this repo.

**Sibling-repo access.** OpenCode launched from `rpg-project` needs **explicit
allowed/reference access** to the sibling implementation repos living under
`game-dev` (`rpg-toolkit`, `rpg-api`, `rpg-dnd5e-web`, `rpg-api-protos`,
`rpg-deployment`, `rpg-game-assets`) — configured, not assumed.

**`game-dev/bootstrap.sh` — full seven-repo workspace.** Today's `REPOS`
array clones/verifies only three repos: `rpg-dnd5e-web`, `rpg-game-assets`,
`rpg-deployment`. That set is insufficient for this design — Platform's
`rpg-toolkit-member`/`rpg-api-member`/`rpg-api-protos-member`/
`rpg-deployment-member` and the `#187` pilot chain (§9) all need repos
`bootstrap.sh` doesn't clone today. Implementation must:
- **Add** `rpg-project`, `rpg-toolkit`, `rpg-api`, and `rpg-api-protos` to the
  idempotent clone/verify set, **alongside** the existing three
  (`rpg-dnd5e-web`, `rpg-game-assets`, `rpg-deployment`) — not replacing them.
- The result is the **full seven-repo workspace** (`rpg-project` + the six
  implementation repos) that all three pods and the equipment pilot chain
  need present on a machine bootstrap runs on.
- May verify OpenCode availability (binary present, reachable).
- **Never** overwrites global OpenCode config — bootstrap only clones/verifies
  and checks tooling presence, it does not write into
  `~/.config/opencode/opencode.jsonc`.

## 7. Error handling and operational gates

- **No issue means no executable dispatch.** A lead cannot claim a task has
  "started" until issue creation/linking and board publication have actually
  succeeded — not attempted, succeeded.
- **Worker permission prompt or auth blocker → stop and publish the blocker.**
  An invisibly-blocked agent is unacceptable; silence is the failure mode, not
  the block itself.
- **Worktree collision → isolate, never overwrite.** A worker that finds its
  target worktree in use creates or uses its own isolated worktree; it never
  overwrites another worker's in-progress state.
- **A gate finding keeps the item In Review** and routes back to the original
  implementer — a finding never silently reopens as a fresh Todo item, losing
  the review thread.
- **A green arriving right after a confusing failure is a flag, not a relief**
  — verify ground truth and test discrimination before believing it (the
  existing director field-notes catalog of false greens applies unchanged to
  OpenCode workers). Use repository-pinned tools and real build/test gates,
  not cascading language-server noise, as the arbiter of "broken."
- **A test/playtest claim is scoped only to the real path it exercised.**
  Fixtures cannot prove production integration; a claim that bypasses the game
  path must say so explicitly rather than rounding up to a stronger claim.
- **Human playtest and evidence judgment remain human-in-loop** wherever a
  subagent cannot itself reach the relevant browser/MCP surface — this is not
  a delegatable step in those cases.
- **Merge does not mean shipped.** For auto-deploying repos, the workflow
  requires watching the main build/deploy run to terminal success before a
  task is marked Done.
- **The retro is the evaluator.** Model/workflow choices made in this design
  are hypotheses; retrospective observation in action — not design-time
  argument — is how they get judged and adjusted (this generalizes the
  existing director field-notes principle "decide with the north star, judge
  in action" to the model-profile and role-split choices in §4–§5).
- **Role permissions reinforce boundaries where practical**, but must still
  allow a lead's board actions (triage, field edits, comments) and a
  reviewer's reversible mutation experiments — permission scoping must not
  accidentally block the gate's actual job.

## 8. Side-by-side rollout and verification

**What gets validated before this is trusted:** config load, symlink
resolution, role discovery, exact model/variant applied per role, unchanged
global config (no accidental overwrite of
`~/.config/opencode/opencode.jsonc`), role permissions (leads can act, gates
can act, neither can merge), sibling-repo access, and board operation
end-to-end.

**First live proof — the deliberate-kill test.** A Terra implementation worker
is **deliberately terminated** after it has published a checkpoint. A
**replacement worker** resumes the task reconstructing **solely** from GitHub
(issue thread + checkpoint comment) and the branch — no inherited local
session state — and carries the task through to opening a PR.

**Gate proof.** A fresh Sol gate verifies the resulting PR. Findings (if any)
cycle: original or replacement implementer addresses → a **fresh** regate
re-verifies. Kirk merges.

**Success criteria for the proof (all must hold):**
- No context loss across the kill/replace boundary.
- No lead performed implementation.
- No reviewer self-remediated a finding it raised.
- Complete Project/issue/PR history — the story is fully reconstructable from
  GitHub alone.
- Repository gates (CI, lint, tests) passing.
- Team-appropriate evidence attached (screenshots for UI/UX, wire-state
  evidence for Platform, multi-angle evidence for Assets, as relevant to the
  pilot task).
- Deployment verified where applicable.
- Human merge authority exercised — Kirk merges, not any agent.

**After the proof:** compare against the equivalent Claude Code workflow.
Automate only observed failures from the comparison — do not pre-build
automation for failure modes that didn't actually occur. Explicitly **out of
scope for this rollout**: an initial plugin, a daemon, a second store,
immediate Claude Code removal, or a multi-provider benchmark. Any of those is
a future decision, made after evidence, not assumed here.

## 9. Approved equipment pilot chain

The equipment feature is both the first live OpenCode proof and a real
in-flight feature — the proof rides real work, not a synthetic exercise.

- **Upstream UI/UX input:** `rpg-dnd5e-web` PR #557 and issue #531 (the
  fixture-first equipment concept that produced the wire-contract request).
- **First live OpenCode proof task/PR:** the additive proto contract under
  `rpg-api-protos#187` (equipment/inventory fields on `CharacterData`,
  equip/unequip intent RPCs, slot taxonomy, display-ready item fields — see
  the issue body for the full enumeration).
- **Before proto implementation begins**, three decisions already open on
  `rpg-api-protos#187` must be resolved:
  1. **Slot-selection ownership** — who decides which slot an equip intent
     targets when ambiguous.
  2. **Authoritative item-slot compatibility representation** — the shape that
     states which slots an item may occupy.
  3. **Whether inventory includes equipped items** — one representation,
     chosen and documented, not left implicit.
  An explicit two-hander-blocked marker on the wire is **minor/non-blocking**
  unless the UI turns out to need it — do not gate the proto PR on it
  preemptively.
- **The broader equipment task stays alive after the proto PR merges** — this
  pilot proves the workflow on one link of the chain, not the whole feature:
  - Rules/occupancy design coordinated with `rpg-project#94` plus a
    corresponding toolkit issue.
  - A new, linked `rpg-api` issue for encounter hydration and equip intents.
  - The UI fixture-to-live swap under `rpg-dnd5e-web#531`.
- **Each repository implementation gets its own linked issue/PR on Project
  19.** This chain is explicitly **not** one oversized cross-repo task — the
  proto, toolkit, API, and web legs are separate issues, separate PRs, each
  independently gated.

## 10. Rollout issue

This design and configuration effort is itself tracked by `rpg-project#101`,
Project 19 Team **Cross-team**, Feature **Infra**, Kind **Build**.

## 11. Approved gate policy (2026-07-21)

This approved addendum supersedes conflicting earlier lifecycle or gate
statements in this design, including §2's lifecycle, §4's independent-gate
description, §8's gate proof, and §9's "independently gated" wording.

An independent adversarial Sol gate is required only for **product-behavior
PRs**: game rules, proto/API contracts, player-facing web behavior,
deployment/runtime behavior, and asset-pipeline output consumed by the game.
Each product code PR receives normal review plus one independent Sol gate. Do
not stack a separate task review and gate over the same change.

If that gate reports findings, the original implementer remediates them and
the same independent reviewer performs a focused recheck of the reported
findings and changed surfaces. Do not restart an unrestricted full audit
unless the remediation materially rewrites scope.

Workflow setup does not require an independent gate: documentation, project
configuration, canonical role charters, adapters, bootstrap scripts, verifier
scripts, and evidence-only Verify tasks instead require deterministic checks,
self-review, a ready PR where applicable, and Kirk's human merge. The
previously gate-required canonical role-policy/charter system, project OpenCode
runtime, seven-repository bootstrap, clean-slate Verify issue, and the
decision-gated equipment planning and evidence-only recovery work are all
workflow setup; a real product code PR in that chain remains product behavior.
PR #103 requires no further adversarial gate after deterministic checks and
policy/plan consistency are complete. Kirk alone merges.
