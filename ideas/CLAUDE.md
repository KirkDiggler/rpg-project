# ideas/ — how designs are filed

Cross-repo designs live here as `ideas/<group>/<topic>/design.md`, reviewed
as an rpg-project PR that stays open as the tracking surface until the
implementing repos have landed (root `AGENTS.md`). This file is about the
folders.

## Folders are domains and primitives, not packages

A group is named for the thing the designs are about, in the game's own
words: `spells`, `characters`, `living-world`, `battlemap`. It is never named
for the code package that implements it. `tools/spatial` is the ruler;
`battlemap` is the surface; `terrain` is what stands on it. If a folder name
is a Go package, that is the smell.

## The thing you are adding may be the second case

Before creating a top-level folder, look at what is already flat. If your
design has a sibling sitting at the top of `ideas/`, you are the second case:
create the group now, name it for the primitive, and move the sibling in
beside you in the same PR, repointing tracked references. Do not write "move
when next touched". That sentence has never once fired in this tree, and on
2026-09-11 five designs that had always been siblings were filed together
only because the sixth arrived and asked.

The rest of the flat folders get grouped the same way: when their similar
thing hits, not before, and not by a sweep.

## What a group holds

- `README.md` naming the primitive and listing the designs with one line each.
- `<topic>/design.md`, optionally `plan.md`, evidence, and specimens beside it.
- The design that defines the group's primitive sits at `<group>/<primitive>/`
  and later slices of it become siblings, the way `living-world/hold-out` did.
