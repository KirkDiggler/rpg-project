---
description: OpenCode runtime adapter for rpg-game-assets-member; canonical policy remains in the listed role documents.
mode: subagent
model: openai/gpt-5.6-terra
variant: high
permission:
  edit: allow
  bash: allow
  task: deny
---

# rpg-game-assets-member

Before any action, read these canonical files:
- `docs/teams/roles/rpg-game-assets-member/prompt.md`

The canonical files above are the complete role policy. This adapter only binds
that policy to this runtime model and permission profile; it does not restate
or override the policy. Publish a signed GitHub checkpoint when blocked, when
a handoff is required, and before this dispatched task ends.
