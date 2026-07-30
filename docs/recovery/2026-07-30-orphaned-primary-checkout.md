# Orphaned primary-checkout recovery — 2026-07-30

This note records the state preserved by [rpg-project issue #171](https://github.com/KirkDiggler/rpg-project/issues/171). It is a provenance record, not approval of any recovered design or plan.

## Provenance

- Read-only source checkout: `/home/kirk/game-dev/rpg-project`
- Source branch: `docs/toolkit-viewer-knowledge-patterns`
- Source HEAD: `29611d94b3d618d228c8a9d9fcf486b739872cfb`
- Recovery base: `origin/main` at `a840d6b7076917260aa1e6e599138478138d752a`, including merged [PR #168](https://github.com/KirkDiggler/rpg-project/pull/168)
- Source commit parent and merge base: `d6a7013c2e060902d0822b88b2d706a2ff507678`; the parent is an ancestor of the recovery base, while `29611d9` is not.

The source had one tracked modification and three untracked idea-document roots. The source status also exposed local `.claude/`, `.worktrees/`, and `rpg-project` roots. The four working-tree snapshots had no commit metadata tying them to an owning session, so their session ownership remains **unknown**. The pushed commit's authorship is preserved in Git. No ownership inference was made from filenames, nearby worktrees, or prior conversation.

## Recovered content

| Recovered path | SHA-256 | Provenance and related references |
| --- | --- | --- |
| `docs/teams/roles/rpg-toolkit-member/context/patterns.json` | `dbd445352a2ad533137674ef69f7b8dd16175e8f5ade10b15484323380b7da47` | Exact resulting blob after applying the semantic diff from pushed commit `29611d9`; the commit names `KirkDiggler/rpg-toolkit#851` and PR `KirkDiggler/rpg-toolkit#857`. |
| `docs/teams/roles/independent-gate/context/lessons-learned.json` | `124b90c66a693ffe1e955de1a215f10772f3d14cdf71aa84abb4b59a941a1f5e` | Exact tracked working-tree snapshot; no issue or PR reference is embedded in the document. |
| `ideas/crypt-monsters/design.LOCAL-DRAFT-2026-07-27.md` | `98ca3d25d38d3a598781fa80538cdc6c7f461da4b27386d58fc24fd4c7865564` | Exact untracked snapshot. Its filename remains `LOCAL-DRAFT`, and its body remains `Draft`. Embedded references: `KirkDiggler/rpg-project#110`, `KirkDiggler/rpg-dnd5e-web#559`, `#577`, and PR `#594`; `KirkDiggler/rpg-game-assets` PR `#28`; `KirkDiggler/rpg-toolkit` PR `#820`. |
| `ideas/locked-door-terminal-projection/plan.md` | `40bcb146327c3e68f4e33132f8ff841f28220b902dece927633fbc05bd2e280b` | Exact untracked recovered plan. Embedded references: `KirkDiggler/rpg-api#690` and `#700`. |
| `ideas/unarmed-character-animation-promotion/design.md` | `7bbbb660bf4e68d092fb035ccdea8ead0cfe38398cce5ad13cb62e5c575f772f` | Exact untracked snapshot; its body remains `Status: Draft`. No issue or PR reference is embedded in the document. |

Issue #171 also records dependencies on `KirkDiggler/game-dev#47` and `#37`. All three idea documents are preserved for later ownership and direction triage; recovery does **not** approve their claims, architecture, implementation steps, or merge gates.

## Source identity proof

Before and after recovery:

- Source status snapshot SHA-256: `103cb6c92f9acc366531dffe875e9647170b592c999ce48905618e6a55407b58`
- Five-file source hash manifest SHA-256: `b6753a9460c51592e45fbdb8fc8f944e4b75ce3ed200f1c74429d2f64e597920`
- Registered-worktree registry before and immediately after source copying: SHA-256 `fd595fd1eaf564859a10ab73400bcdd966486250388172990a02b92acf6f1d0c` (37 entries)
- Final registry excluding this active recovery worktree: before/after SHA-256 `6ab3fa5d7101491ba54dc9486eb273af3fb3db35746bf8e99aeaf191c53922a2`

Byte comparison passed for the source checkout's status and all five relevant source files. Source HEAD, branch, upstream parity, tracked/untracked status, and bytes were unchanged. Every other registered-worktree entry was byte-identical; the registry's only final change was this recovery worktree's expected HEAD advance from the recovery base to its recovery commit.

The exact unarmed-design snapshot contains intentional Markdown hard breaks on lines 3, 4, and 6. Default `git diff --check` reports those three source-authored trailing-space lines; they were not normalized because doing so would violate the required snapshot hash. The diff check passes when blank-at-EOL is excluded, with no other whitespace finding.

## Explicit exclusions

Recovery did not copy, stage, modify, delete, clean, switch, reset, or stash any of the following:

- source `.claude/settings.local.json`
- source `.claude/worktrees/**` and `.worktrees/**`
- the source `rpg-project` self-symlink
- `.git` links, files, directories, or other Git administrative content
- caches or generated content
- any of the 37 registered worktrees or content from their paths
- any source path not listed in **Recovered content**
