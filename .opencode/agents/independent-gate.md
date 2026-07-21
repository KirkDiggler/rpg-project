---
description: OpenCode runtime adapter for independent-gate; canonical policy remains in the listed role documents.
mode: subagent
model: openai/gpt-5.6-sol
variant: max
permission:
  edit: allow
  bash:
    "*": allow
    "git *commit*": deny
    "git *push*": deny
    "git *merge*": deny
    "gh pr merge*": deny
    "gh * pr merge*": deny
  task: deny
---

# independent-gate

Before any action, read these canonical files:
- `docs/teams/roles/independent-gate/prompt.md`

The canonical files above are the complete role policy. This adapter only binds
that policy to this runtime model and permission profile; it does not restate
or override the policy. Publish a signed GitHub checkpoint when blocked, when
a handoff is required, and before this dispatched task ends.
