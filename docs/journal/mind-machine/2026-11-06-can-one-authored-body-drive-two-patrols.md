# Can one authored body drive two different patrols?

Written: **2026-11-06**; implementation began 2026-11-05.
Work: [NPC behavior S2 — #871](https://github.com/KirkDiggler/rpg-project/issues/871),
part of [#859](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [What existing bundles already bind](2026-11-05-what-do-existing-bundles-already-bind.md).

## Question

After R1–R3 selected authored data, private bindings/progress and finite expansion
during binding, can one real producer run the two- and three-waypoint example
without a Patrol-specific factory or a test helper constructing its graph?

The operator also identified repeated pauses after agreeing on the work. The
response was a bounded delivery issue and checked task plan, followed by code and
observable tests, rather than another request to approve the same direction.

## Evidence

[Toolkit #2284](https://github.com/KirkDiggler/rpg-toolkit/pull/2284), head
`eba23e1dbe6ec4ff93e82c5f2d8af55392f908dc`, adds `ExpandSequence` in
`mind/behavior`. Its canonical
[Patrol definition](https://github.com/KirkDiggler/rpg-toolkit/blob/eba23e1dbe6ec4ff93e82c5f2d8af55392f908dc/mind/behavior/testdata/sequence-patrol.json)
is data. The producer emits ordinary recipe/table instances and completion/source
maps; existing machine/composer execution and dependency pins are unchanged.
The [source-to-expanded-output guide](https://github.com/KirkDiggler/rpg-toolkit/blob/eba23e1dbe6ec4ff93e82c5f2d8af55392f908dc/mind/behavior/sequence.md)
shows the supplied values and generated program shape.

The [joined test](https://github.com/KirkDiggler/rpg-toolkit/blob/eba23e1dbe6ec4ff93e82c5f2d8af55392f908dc/mind/behavior/sequence_patrol_test.go)
uses that producer, real machines, the real composer and the real picker:

| Path | Observed result |
|---|---|
| Two-/three-item loops | West targets 0 → 4 → 0; Court 10 → 12 → 14 → 10; each singleton selection uses d100 face 1 |
| Selection versus completion | Selecting retains the leg; West completion leaves Court unchanged; an inactive step's completion is inert |
| Interruption/resumption | Investigation wins on exact d200 faces 1 and 101, then the unfinished `(4,0)` leg resumes |
| No movement budget | Hold, zero selection rolls, unchanged continuation |
| JSON reload | West resumes `(4,0)`, Court `(14,0)`; incompatible configuration is refused; load does not rebind or roll |
| Non-Patrol control | A second authored body says alpha → beta → terminal through the same producer |

An actual defect surfaced: flattening `$next` into a string too soon let an
authored local name capture the generated destination. The assertion expected
`beta` and got `captured`. The producer now retains escaped reference locations,
so literal local destinations and explicit next references remain distinct.

Two intentional production mutations also failed at assertions: wrong-next
returned West 0 instead of 4 / Court 10 instead of 12; misrouted completion
returned `ErrSelectionRequired`. Both were restored and the production file hash
matches the pre-mutation receipt. The initial missing-API compilation failure and
a later dice-helper literal typo are not counted as behavioral regression proof.

Full owning-module race tests passed (root 1.204s, sound 1.015s, stage 1.016s;
deed has no tests), lint reported zero issues, and the actual scoped pre-commit
hook passed with its cache disabled. Exact-head CI is green for applicable checks;
`test-all` is a scoped skip, not a full-workspace pass.

The [independent review](https://github.com/KirkDiggler/rpg-toolkit/pull/2284#pullrequestreview-5793883425)
read all eleven changed files and published **MERGE-READY at the exact head**,
with no Critical, Important or Minor findings. Its additional probes checked
integer precision, escaping, non-recursive insertion, strict nested-field
refusals, detachment and absence of fact-reader execution during binding.

## Interpretation

The accepted component path is now executable: one data body really produces
differently sized programs with private progress. The genericity claim is
supported by a second body with no movement word, not by renaming Patrol.

The capture defect clarifies a compiler responsibility: a reference's identity
cannot safely be inferred from its final string value. The correction belongs in
the producer; no new runtime operation was needed.

This is component proof, not authoring-system or game-path completion. The source
is canonical component JSON, not new studio/dungeonspec fields. Catalog lookup,
explicit defaults, coordinate/domain validation and editor adapters remain with
existing producers. Completion is attachment/item/table addressing, not an async
job-token or cross-cycle deduplication protocol. No physical movement, spatial
rejoin policy, keep-scale performance or live deployment is demonstrated.

## Next

The implementation is reviewed and unmerged. Merge/release requires separate
operator authorization and the actual CI-published tag; #871 remains open until
its delivery requirements hold. The broader source/Studio/world adapter proof
remains on #859 and the live journey on #866. Do not turn this successful
component checkpoint into a claim that either consumer path already works.
