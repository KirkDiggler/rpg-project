# Implementation planning

Read this guide after design agreement and before implementing a deliberate
change. [The design skill](SKILL.md) governs agreement and architectural-gap
judgment; this guide makes the translation into executable tasks concrete.

## The outcome

A plan carries the decisions a fresh implementer cannot derive alone. It names
what to change, the contracts to preserve or introduce, and what proves the
result. It is not a transcript of the future program and not an outline that
leaves the architecture to the worker.

A faithful checked plan is authorized by the approved design. Make it visible
in the issue or PR, or a linked working document, without adding a second
approval ceremony. Architectural gaps return only the affected decision to the
operator. Do not hide required decisions in implementation tasks or silently
weaken acceptance criteria.

## Ground and decompose

1. Read the approved design, brief, rulings and acceptance scenarios. Identify
   in-scope requirements and explicit deferrals. Give requirements stable labels
   when they do not already have them; labels identify agreed behavior, not new
   rulings.
2. Inspect the owning code and nearest repository/package instructions. Map
   files and their responsibilities before writing tasks. Follow actual symbols
   and callers; do not guess paths or provider capabilities. Record the inspected
   revision and supporting references in the plan or linked issue, not the law.
3. Name changed seams from the consumer's needs. Identify existing capabilities,
   provider work, adapters and integration wiring. Check applicable persistence,
   events, generated bindings, fixtures, dependency pins and cleanup of replaced
   paths; mark irrelevant categories as such rather than manufacturing tasks.
4. Decompose into bounded deliverables with their own verification. Fold setup
   and documentation into the deliverable that needs them. Split where a task
   has a distinct responsibility, test surface or meaningful review boundary;
   do not split merely to reach a task count or time estimate.
5. Name development dependencies separately from merge/release dependencies.
   Follow the owning instructions and shared outside-in development/inside-out
   merge policy. Do not imply a task can consume an unbuilt interface, or that
   local integration proves released dependency pins.

A task need not ship independently. Its prerequisites must make its own checks
runnable, and the plan must include the integration deliverable that proves the
whole result. Independent unit tests are not proof of a changed wire or UI seam.

## Plan header

Every plan names:

- **Goal and scope:** the intended outcome and explicit non-goals.
- **Authority:** links to the approved design/brief and recorded agreement.
- **Constraints:** binding invariants and exact agreed values, plus references
  to the owning instructions. Do not copy entire policy documents.
- **Inspected baseline:** repositories/modules and revisions used for planning.
- **Sequence:** task dependencies, integration steps, applicable merge/release
  order, and verification after adopting released pins.
- **Open items:** resolved assumptions, explicit agreed deferrals, and blockers.
  A required unresolved architectural decision blocks the affected tasks.

Choose an accessible working-document location under the idea's existing
convention or keep a small plan in the issue/PR. Link it from the paper trail.
The four-section design law remains separate.

## Task contract

Use this structure for every task; mark a field not applicable with a reason
rather than leaving it implicit. The fields below describe what to fill in;
replace them with concrete inspected paths, names, values and checks.

```markdown
### Task N: Concrete deliverable

**Delivers:** Requirement/ruling IDs and observable behavior.
**Owner:** Repository, module and responsibility.
**Prerequisites:** Task IDs, available provider capabilities, environment/fixtures.
**Files:** Exact create/modify/test paths; existing symbols to change.
**Interfaces:** Consumed and produced names, parameter/return types or wire
fields, exact agreed values, and semantics later tasks depend on.
**Behavior:** Inputs, outputs, state transitions and failure/absence cases.
**Tests:** Test names, concrete inputs and expected assertions for each behavior.

- [ ] Add/update the named tests or acceptance fixture.
- [ ] Run the focused check; confirm the expected pre-change failure where applicable.
- [ ] Implement the named changes while preserving the stated contracts.
- [ ] Run focused and seam checks; record observed results.
- [ ] Update owning documentation or remove replaced paths when this task requires it.

**Verification:** Exact commands, working directory, prerequisites and expected
pass/fail signals. Include the integration scenario when this task owns it.
**Completion evidence:** Changed behavior, passing assertions, command results
and any required artifact or commit references.
```

Adapt the action sequence to the work: documentation, generated bindings and
mechanical updates do not need artificial failing unit tests. Every step still
has a checkable result. For behavior changes, the tests must distinguish the
intended behavior from the old or broken behavior.

### Precision without transcription

- **Files:** Use exact paths and existing symbols, not brittle line numbers as
  the only locator. Distinguish proposed new files from inspected existing ones.
- **Interfaces:** Specify names, types and semantics where tasks meet. A later
  task consumes the same contract an earlier task produces. References to that
  contract are enough; do not duplicate implementation bodies across tasks.
- **Tests:** Name the test and its inputs and assertions, including exact values
  fixed by the design. Use code when prose leaves the assertion ambiguous.
  Include in-scope negative, absent-value and boundary cases, not just success.
- **Code steps:** Name the file/symbol, intended change and constraints. Leave
  local algorithms and helpers to the implementer unless a particular choice is
  required by the agreement or another task. Show code only when it settles a
  decision that signatures and tests do not convey.
- **Checks:** Give commands that actually exercise the changed behavior and
  describe expected signals. During planning these are expectations, not claims
  that tests ran. User-visible scenarios name setup, action and observable result.

Instructions such as "handle edge cases", "add appropriate validation", "wire
it up" or "write tests" decide nothing on their own. Replace them with named
cases, contracts and assertions. Do not refer to a new type/function that no
task defines or an existing capability that inspection has not established.

## Visible plan checks

Before implementation, check the complete plan against the agreement and the
current code. Publish these two tables with the plan; a bare "plan checked"
verdict is not the check. A one-task plan may have one row per table. For work
with no changed seam, state that explicitly with the reason.

### Requirement coverage

| In-scope requirement / acceptance scenario | Implementing task(s) | Concrete proof |
|---|---|---|

Account for every in-scope requirement, not only the ones easy to test. Each
proof points to a task's named assertions or observable acceptance check.
Identify explicit deferrals separately; do not turn an uncovered requirement
into a deferral without authority from the design/operator.

### Provider/consumer seams

| Provider task/module | Consumer task/module | Produced vs consumed contract | Availability/dependency | Proof |
|---|---|---|---|---|

Cover every changed interface and cross-task dependency. Compare names, types,
values, identity and absence semantics, not just whether both sides mention the
same concept. Name existing providers and how their capability was established;
for new ones, name the producing task. Name verification of the joined path.
For tasks sharing files, state ordering or isolation so neither silently
invalidates the other's work.

Then scan each task and the plan as a whole:

- Does each task's prescribed implementation agree with its test assertions?
- Does every requirement have an owner and proof? Is every consumed capability
  available or produced by a named prerequisite?
- Are adapters, persistence, multiplayer events, generated bindings, dependency
  pins, fixtures and removal of replaced paths accounted for where needed?
- Does the integration check exercise the real changed seam, rather than only
  isolated fakes? Are user-visible outcomes observable?
- Are commands appropriate to the owning repository and environment? Are final
  readiness checks and applicable release/repin checks included?
- Are placeholders, contradictory signatures, hidden assumptions or required
  unsettled decisions still present? Is any task so broad it needs decomposition?
- Is detail resolving necessary decisions, or merely transcribing code? Remove
  redundant bodies without losing contract precision.

Record findings and their disposition beside the checks. Correct gaps the
agreement answers. For architectural gaps, pause affected work and bring back
the specific decision under the design skill. Do not require a new review agent
or approval gate merely to run these checks.

## Handoff readiness and execution

The final readiness test is:

> Can a fresh implementer complete this task from its brief, linked design,
> owning instructions and declared dependencies, without reconstructing the
> conversation or inventing ownership, public behavior or a cross-task interface?

If not, repair the plan before starting affected work. A worker may choose local
implementation details; a worker must not rediscover the design.

For direct execution, work task-by-task from the same checked plan. For
operator-authorized delegation, provide the selected task, relevant constraints,
design/issue links, exact prerequisite interfaces, workspace/base and required
instruction chain from the working agreements. Linked material must be reachable
from the worker's environment. Handoff readiness does not authorize delegation,
parallel writes, a new task store or bypassing repository review policy.

Require a compact completion report:

- completed task and changed files/commit references;
- actual verification commands and observed results, with evidence links;
- deviations from the plan and their effect on the agreement;
- remaining concerns or blockers, explicitly stating incomplete required work.

Mark completion only when the task's evidence exists. A concern that prevents
acceptance is a blocker, not "done with a follow-up". Update affected task
contracts and check tables when code inspection or implementation changes the
plan; retain the correction and evidence in the issue/PR. Report the integrated
result and remaining gaps without claiming that task-level passes alone prove
the whole design.
