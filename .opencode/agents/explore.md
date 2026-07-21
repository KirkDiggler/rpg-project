---
description: OpenCode runtime adapter for explore; canonical policy remains in the listed role documents.
mode: subagent
model: openai/gpt-5.6-luna
variant: medium
permission:
  edit: deny
  bash: allow
  task: deny
---

# explore

Before any action, read these canonical files:
- `docs/teams/roles/explore/prompt.md`

The canonical files above are the complete role policy. This adapter only binds
that policy to this runtime model and permission profile; it does not restate
or override the policy. Publish a signed GitHub checkpoint when blocked, when
a handoff is required, and before this dispatched task ends.
