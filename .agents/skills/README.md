# Canonical skills catalog

Shared repeatable procedures belong here only after individual review and approval.

The canonical skills:

- `story/SKILL.md`: the story issue, its slice sub-issues, and the names (story, S<n>, R<n>) everyone uses.
- `design/SKILL.md`: design, planning and architectural-gap judgment.
- `build-slice/SKILL.md`: the builder's contract for one slice of a plan.
- `walk/SKILL.md`: the walk on a real stack, run as the standing QA script.
- `pr-review/SKILL.md`: the independent review round and its QA handoff.
- `merge-out/SKILL.md`: merging a wave inside-out and shipping the slice.

Changes to shared procedures follow the operator's agreement; a skill is not permission to add approval ceremonies.

`.opencode/skills` and `.claude/skills` are legacy candidates, not inputs. They may inspire a reviewed procedure, but they are not automatically part of this namespace.

AGENTS invariants and live state must never be copied into a skill. Keep stable project law in AGENTS/CLAUDE files and current state in linked issues and PRs.

Repository-only procedures belong in that repository's `.agents/skills/`, not in this cross-repository rpg-project catalog.
