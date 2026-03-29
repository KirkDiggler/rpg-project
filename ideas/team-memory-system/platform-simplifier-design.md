# Platform Simplifier — Design Spec

## Goal

Add a "Platform Simplifier" team role that reads accumulated knowledge and upcoming work across all repos, identifies structural friction, and briefs the team before workers start — so cross-cutting cleanup opportunities (like unified-entity-state) are caught automatically instead of discovered after three point fixes.

## Problem

Individual workers see their repo. The PM sees operational status. Nobody sees the structural picture: that three unrelated bugs share a root cause, or that the planned feature will fight the current abstraction. Today this requires Kirk to manually spot patterns across bug fixes, PRs, and lessons learned — which only happens when enough pain accumulates.

The unified-entity-state case is the canonical example: three separate web bugs (#351, #357, #358) each traced to entity data split across multiple React state containers. Each got a point fix. Only after all three were fixed did the pattern become obvious — a unified entity store would have eliminated the entire class of bugs. A pre-session analysis role would have flagged this before the first fix was written.

## Role Identity

**Name:** Platform Simplifier

**One-liner:** Reads accumulated knowledge and upcoming work, identifies structural friction, and briefs the team before workers start.

**Analogy:** The platform engineer who walks into standup and says "before you start, here's what I'm seeing." Doesn't fix bugs, doesn't make architecture decisions — sees the terrain before the team hikes in.

**What it is:** The team member who's read every post-mortem, every lesson learned, every pattern — and cross-references that against what's about to be worked on. It sees what no single-repo worker would.

**What it isn't:** It doesn't implement code, doesn't make architectural decisions, doesn't block work. It's advisory. Kirk (creative director) decides what to act on.

## Position in Team Lifecycle

Reports first, before PM and before workers are assigned:

```
1. Platform Simplifier  — briefs the room (structural observations)
2. Project Manager       — reports operational status (PRs, blockers, work items)
3. Kirk (creative director) — decides what to work on, assigns tasks
4. Workers (fixers, etc.) — execute
```

The simplifier and PM have complementary views:
- **PM:** "Here's what's open, here's what's blocked, here's what merged"
- **Simplifier:** "Here's what I see underneath that — patterns, friction, drift"

**When not to run it:**
- Hot-fix sessions where you already know exactly what to do
- Single-issue work where there's no cross-cutting concern to analyze

## Inputs

On spawn, the Platform Simplifier reads (in this order):

### 1. Accumulated Knowledge (all repos)

`.claude/knowledge/context/` across rpg-toolkit, rpg-api, rpg-dnd5e-web, rpg-api-protos. Patterns, lessons learned, decisions — the institutional memory that individual workers have written over time.

### 2. Recent Git History (all repos)

Recent commits and merged PRs (last ~2 weeks or since last session). Looking for: clusters of changes in the same area, repeated fix patterns, multi-repo changes that touched the same concern.

### 3. Upcoming Work

- Project board (GitHub Projects #10)
- Session's issue list / milestone bugs
- Any active `ideas/` that are in-progress

### 4. Existing Ideas (to avoid duplication)

`rpg-project/ideas/` — scan what's already been captured so it doesn't re-discover known problems.

### 5. PM State (if available)

`docs/teams/roles/project-manager/context/` — active PRs, blockers, work items. Gives the operational picture alongside the structural one.

### 6. Own Previous State

`docs/teams/roles/platform-simplifier/context/` — previous briefs and carried-forward observations, so it doesn't repeat itself.

The key insight: no single-repo worker reads all of this. The simplifier's power comes from the cross-repo, cross-concern view.

## Analysis

Three analysis passes over its inputs:

### Pass 1: Pattern Clustering

Look for multiple knowledge entries that point at the same underlying concern.

- Multiple lessons/patterns referencing the same files or concepts
- Multiple recent PRs modifying the same area across different issues
- Workarounds that share a shape (merge utilities, sync helpers, extra state tracking)

The unified-entity-state case: three separate bug fixes, three separate lessons learned, all touching entity state sync. No single entry says "structural problem" — the cluster does.

### Pass 2: Friction Forecasting

Cross-reference upcoming work against current architecture. Ask: "given what we're about to do, what's going to be harder than it needs to be?"

- Does the planned feature touch an area with known workarounds?
- Does the issue list cluster around a subsystem that has accumulated lessons?
- Would a structural change unblock multiple planned items at once?

### Pass 3: Drift Detection

Look for signs that implementation has drifted from the intended architecture:

- Knowledge entries that describe working around a boundary violation
- Patterns that exist because the "right" abstraction doesn't yet
- Repeated "sync X to Y" patterns that suggest X and Y should be unified

Each pass produces observations, not conclusions. The simplifier flags what it sees — Kirk decides what matters.

## Outputs

Three output types, escalating in effort:

### 1. Session Brief (always)

A structured report delivered to the team at startup. Short, scannable, actionable.

```
## Platform Simplifier Brief — 2026-03-28

### Friction Points for Today's Work
1. **Entity state fragmentation** (high) — Issues #357, #358, #351 all trace to
   entity data split across 3 state containers. Point fixes will work but the
   pattern will keep producing bugs.
   -> Recommendation: Consider unified entity store before fixing individually.

2. **Action economy sync** (medium) — Two recent PRs added sync helpers between
   character state and encounter state. Upcoming combat feature work will hit
   this again.
   -> Recommendation: Worth watching. Not blocking yet.

### No Issues Found
- Toolkit event bus patterns look clean
- Proto contract alignment is solid
```

Each item gets a severity (high/medium/low) based on how much it affects today's planned work.

### 2. Knowledge Updates (when discoveries are made)

Appends cross-cutting patterns to `.claude/knowledge/context/` in the relevant repos. Even if Kirk doesn't act on a finding today, the knowledge persists for future sessions. This is how the simplifier's observations feed back into the system that workers read.

### 3. Idea Seeds (for high-confidence findings)

Creates `rpg-project/ideas/<topic>/CLAUDE.md` with the problem statement, evidence, and questions to explore. Seeds the brainstorming pipeline. Only for findings where the pattern clustering is strong — not for speculative stuff.

The brief is the main deliverable. Knowledge updates and idea seeds happen as side effects when warranted.

## Role State

**Directory:** `docs/teams/roles/platform-simplifier/`

Lighter than the PM since this role is primarily analytical, not tracking operational state:

```
platform-simplifier/
  prompt.md                        <- Spawn instructions
  context/
    previous-briefs.json           <- Past briefs (prevents re-flagging resolved items)
    active-observations.json       <- Observations carried forward between sessions
  archive/
    resolved-observations.json     <- Observations that were acted on or became irrelevant
```

### previous-briefs.json

The key state file. Prevents the simplifier from repeating itself. If it flagged entity state fragmentation last session and Kirk chose not to act on it, it knows. It can escalate ("flagged this before, now 2 more bugs hit it") or stay quiet based on whether new evidence has appeared.

Schema: bare JSON array of brief entries.

```json
[
  {
    "type": "note",
    "id": "brief-2026-03-28",
    "created": "2026-03-28",
    "status": "active",
    "title": "Session brief 2026-03-28",
    "content": "Flagged entity state fragmentation (high), action economy sync (medium). Kirk chose to point-fix entity bugs, deferred structural work.",
    "tags": ["brief"],
    "references": ["rpg-dnd5e-web#351", "rpg-dnd5e-web#357", "rpg-dnd5e-web#358"]
  }
]
```

### active-observations.json

Carries forward things that aren't idea-worthy yet but are worth watching — the "medium" severity items from the brief. Uses the existing `note` schema type.

```json
[
  {
    "type": "note",
    "id": "action-economy-sync-drift",
    "created": "2026-03-28",
    "status": "active",
    "title": "Action economy sync helpers accumulating",
    "content": "Two PRs added sync helpers between character and encounter state. Not critical yet but the pattern is growing. Watch for a third instance.",
    "tags": ["observation", "drift"],
    "references": ["rpg-api#447"]
  }
]
```

## Interaction with Existing Systems

### Knowledge System

- **Reads** from `.claude/knowledge/context/` across all repos (consumer of what workers write)
- **Writes** cross-cutting patterns back to knowledge files (things no single worker would see)
- **Writes** idea seeds to `rpg-project/ideas/` (feeds the brainstorming pipeline)

### Project Manager

- **Reads** PM's `active-work-items.json` to understand what's planned
- Does NOT write to PM state — advisory, not operational

### Workers

- No direct interaction. Workers benefit indirectly: if the simplifier's brief changes what Kirk assigns, workers get better-scoped tasks. Workers also benefit from knowledge entries the simplifier writes.

## File Structure

```
rpg-project/
  docs/teams/roles/
    platform-simplifier/
      prompt.md                        <- Spawn instructions
      context/
        previous-briefs.json           <- Past session briefs
        active-observations.json       <- Carried-forward observations
      archive/
        resolved-observations.json     <- Acted-on or irrelevant observations
  ideas/
    platform-simplifier/
      CLAUDE.md                        <- This idea's context
      design.md                        <- This file
```
