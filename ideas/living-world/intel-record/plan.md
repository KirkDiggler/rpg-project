# The intel record — plan

**Status:** RULED 2026-09-04. Built under the retro rule of slice 2
(recover-the-artifact/plan.md, "Retro"): **every wave on pushed branches
first, the local stack up from the worktrees, Kirk walks, findings folded
back on the branches, THEN one bounded PR per module.** Toolkit PRs merge as
they come; rpg-api takes ONE pin at the end.

## No proto wave

`PutDungeon` ships verbatim YAML; nothing new crosses the session wire.

## Wave 1 — toolkit (branches, pushed, not opened)

- **encounter module** (one branch): dungeonspec `IntelSpec{ID, Reveals{Door}}`,
  `PlaceSpec.Holds`, `Knows` deleted + refused by name, `Compiled.Intel` +
  `Holds`; encounter `MemberInput.Holds`/`JoinInput.Holds`, holdings fact
  `holds:intel:<record id>`, transfer reads `reveals` from the field's intel
  table, trust boundary extended; the heirloom fixture re-authored
  (`intel:` + `holds:`); scenes per design §7; mutation pass over the
  transfer indirection and the refusals.
- **session module** (one branch, pseudo-pinned to the encounter branch):
  `SpawnInput.Holds` forwarding; the spawn-and-loot scene on records.

## Wave 2 — rpg-api (branch, pseudo-pinned to both toolkit branches)

Forward `Holds` where `Knows` was forwarded (compiled ids); heirloom fixture
byte-identical to the toolkit's; acceptance scenes re-pointed at records.

## Wave 3 — web (branch, no re-pin)

The intel panel (design §5); yaml round-trip; `knows` refused; Concepts Lab
fixture byte-identical; one headless screenshot of the panel with a record
held by the captain.

## The walk (Kirk, on the local stack from the four worktrees)

Re-author the heirloom tomb's intel through the panel (or author your own),
kill the captain, Loot, the door reveals to the looter alone, out through the
exit. One finding round on the branches.

## Then the PRs

toolkit encounter → toolkit session (on the encounter tag) → rpg-api (one
pin, both tags) → web. Slice issues filed at that point under #326.

## Done-when

The heirloom tomb authors its intel through the panel, `knows` is gone from
every repo, and path 2 walks on the merged stack.
