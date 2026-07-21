---
name: project-board-workflow
description: Use before starting repository work that needs a Project 19 issue, branch, PR, checkpoint, product gate, or merge handoff.
---

# Project 19 Workflow

Before executable work, create or identify one repository issue, add it to
Project 19, set Team, Feature, Kind, and Status, then create one fresh branch
from main when the work produces a repository diff. Publish WORK SESSION
STARTED on the issue. Keep issue, PR, branch, and checkpoint comments sufficient
for a replacement worker to resume without session state. For workflow setup,
run deterministic red then green checks, self-review, commit, push and open a
ready PR, then hand off Kirk's review and merge decision. For product behavior
(rules, proto/API contracts, player-facing web behavior, deployment/runtime, or
consumed asset output), add normal review plus exactly one independent Sol gate.
Route findings to the original implementer; the same reviewer performs the
focused recheck after remediation. Start a new unrestricted audit only after a
material scope rewrite. End every GitHub comment with the active role signature
on behalf of KirkDiggler. After an applicable merge, verify deployment to
terminal success before marking the Project item Done.

Every PR body must include exactly one closing keyword for its backing issue:
- Same-repository backing issue: `Closes #<issue-number>`.
- Cross-repository backing issue: `Tracks <owner>/<repository>#<issue-number>`.

After a human merge, wait for GitHub to close the backing issue and Project 19
automation to move the item to Done before manually reconciling fields. Repair
only fields automation failed to reconcile; do not preempt or duplicate the
automated transition.
