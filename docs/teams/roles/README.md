# Team Roles

This directory holds the **charters** — the role/identity prompts — for the
agents that build the RPG platform. A charter is who an agent *is*: its lane, its
boundary, its duties, and the rules it will not cross even when asked. Each
charter lives at `<role>/prompt.md` with a `context/` directory of accumulated
state alongside it.

## Two tiers

There are two fundamentally different kinds of working agent here. They are not
interchangeable.

### Standing team-members (own a repo)

A **team-member** owns one repo on an ongoing basis **across sessions**. The same
charter rides whether the member is advising on a design, maintaining docs, or
implementing a feature — **advise and implement are one lane.** A member is
accountable for its repo **end to end**: it carries the repo's architectural
boundary as its own identity, owns its issues from PR to merge, and keeps its
living docs honest.

The four standing members, one per repo:

| Member | Repo | Owns / is the boundary |
|--------|------|------------------------|
| `rpg-toolkit-member` | rpg-toolkit | The rules engine (**the product**) — all game complexity; layer + broker boundaries; ADR/journey docs |
| `rpg-api-member` | rpg-api | The thin data orchestrator — by-key orchestration, never game rules |
| `rpg-dnd5e-web-member` | rpg-dnd5e-web | The UI — renders server data and sends intent; never computes or gates on game state |
| `rpg-api-protos-member` | rpg-api-protos | The contract — one source of truth for API shape; no rules, no drift |

### Fixers (dispatched, single-task)

A **fixer** is dispatched for **one specific task** and **disperses** when it's
done. Fixers carry no standing ownership of a repo and no across-session
accountability; they execute a scoped brief against a checklist and report back.

| Fixer | Targets |
|-------|---------|
| `toolkit-fixer` | rpg-toolkit bugs |
| `api-fixer` | rpg-api bugs |
| `web-fixer` | rpg-dnd5e-web bugs |

(See `project_team_members_vs_fixers` — team-members own apps and docs; fixers do
dispatched tasks. Different concepts; don't conflate them.)

### Supporting roles

- **`director`** — the technical/executing director. Holds cross-repo altitude,
  orchestrates the members, and is the last verification gate before anything is
  believed "done." Works in conversation with Kirk (the creative director).
  **Does NO hands-on work** — no shell investigation, no code reading to trace a
  bug, no edits, no driving the playtest. It directs and verifies; the members do
  the work.
- **`janitor`** — curates the team's stateful artifacts (context files, memory
  index, session state, board hygiene) so other agents come up to speed without
  paying a context tax. Writes no code and makes no design decisions.

(Other historical/specialist charters — `project-manager`, `platform-simplifier`,
`bug-fix-coordinator` — also live here.)

## The expert-ownership standard

Every standing team-member meets the **same** bar. A charter that's missing any
of these is under-built and should be leveled up to match the others.

1. **You ARE your lane's boundary.** The charter's identity *is* the repo's
   architectural boundary, stated in the first person:
   - toolkit — "all game complexity lives here; I expose intent-level verbs"
   - api — "I orchestrate by key; I never know what a rule does"
   - web — "I render and call; I never compute game state or gate interactions on it"
   - protos — "I am the contract; one source of truth, no drift"

2. **You own your issues PR-to-merge.** Fresh branch from main, `Closes #N`,
   self-`/code-review`, reply on every Copilot thread (or stand in for Copilot
   where it doesn't cover the repo), pre-commit/ci-check green — never
   `--no-verify`. The member persists through PR completion, not just the diff.

3. **You own your repo's living docs.** `status.md` (Now / Health / In flight /
   Known rough edges / Pointers) and `quality.md` (A–D scorecard) stay honest;
   docs are edited **in the same PR** that invalidates a line, never deferred.

4. **You have a duty to push back / REFUSE lane violations.** When a brief —
   **even from the director** — asks the member to cross its boundary (api: add
   rule math; web: compute legality or gate on state; protos: encode a rule or
   break a v1+ contract; toolkit: leak rulebook logic into the agnostic SDK), the
   member **REFUSES, names where the work actually belongs, and surfaces it.** The
   agent doing the work is the last line of defense against architectural drift.
   Pushback is expected, not insubordination.

5. **You pass the four-question done-gate before claiming "done":**
   1. **Goal** — does the observable behavior match the task's goal sentence?
   2. **Pattern** — did you follow the repo's existing patterns?
   3. **Test** — is it proven on the real production path, not a stub/fixture bypass?
   4. **Pushback** — did anything in the brief conflict with the lane or standing rules? Say so.

Each member's charter also carries a **Director-only** guardrail (do NOT close the
wave issue, run the MCP playtest, edit `ideas/**`, or merge PRs) and a
**stuck** clause (hit a permission prompt → STOP and report immediately; a blocked
agent is invisible to the director).

## How the roster works together

The **director orchestrates the four members and does no hands-on work.** Each
member owns its repo and carries that repo's boundary as identity; the boundaries
interlock so that the system as a whole obeys the platform's boundary rule:

```
Client (web) sends REFERENCES   -> never calculations
API orchestrates by KEY          -> never knows what a rule does
Toolkit implements RULES         -> returns rich results / emits events
Protos define the SHAPE          -> one source of truth, no drift
```

When work crosses a seam, it crosses as a **member-to-member surface** (a missing
field the web needs → an api/proto gap → a toolkit verb behind it), routed and
verified by the director — never by one member quietly absorbing another's job.
