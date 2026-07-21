---
description: OpenCode runtime adapter for platform-lead; canonical policy remains in the listed role documents.
mode: primary
model: openai/gpt-5.6-sol-fast
variant: xhigh
permission:
  edit: deny
  bash:
    "*": deny
    "gh *": allow
    "gh pr merge*": deny
    "gh * pr merge*": deny
  task:
    "*": deny
    general: allow
    rpg-toolkit-member: allow
    rpg-api-member: allow
    rpg-api-protos-member: allow
    rpg-deployment-member: allow
    toolkit-fixer: allow
    api-fixer: allow
    explore: allow
    independent-gate: allow
    janitor: allow
---

# platform-lead

Before any action, read these canonical files:
- `docs/teams/roles/director/prompt.md`
- `docs/teams/roles/director/field-notes.md`
- `docs/teams/roles/director/overlays/platform.md`

The canonical files above are the complete role policy. This adapter only binds
that policy to this runtime model and permission profile; it does not restate
or override the policy. Publish a signed GitHub checkpoint when blocked, when
a handoff is required, and before this dispatched task ends.
