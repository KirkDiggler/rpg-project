# The walkthrough

Read this guide when a design has a shape worth judging and when an agreed
design changes a seam. [The design skill](SKILL.md) governs agreement and the
law doc; this guide shapes the explanation that makes the law understandable.

## The outcome

The walkthrough explains the design to a teammate who writes code but was not
in the conversation. After reading it they can say which component owns each
noun, what crosses each seam and in which type, which tempting shortcut each
boundary refuses, what each decision costs, and where their own change would
land. The law says *what holds*. The walkthrough says *how the pieces meet and
why the cut is there*.

It is also the agreement surface. The operator judges a design by
understanding it. A diagram plus a rulings table invites agreement on a picture.
The walkthrough shows the responsibilities, contracts and costs that the
agreement is about.

## Where it lives

`README.md` beside the design doc, in the same `ideas/<topic>/` folder, or
under the owning repo's equivalent location. Use `walkthrough.md` instead when
the folder's README is already its index. It links the issue and the
design doc in its opening line. It never carries a `## Rulings` section;
rulings live in the design doc, and the walkthrough cites them by ID (R3).

When the two disagree, the law wins and the walkthrough is corrected in the
same PR that changes the law. A walkthrough that has drifted is worse than
none: its rationale reads as authority.

## The sections

Use these in order. Leave a section out when the design has nothing for it,
and do not pad one to fill the shape.

1. **What this is.** One paragraph: the new abstraction, what it reuses, and
   what it deliberately does *not* introduce, such as no second visibility
   authority or no new collision engine. Link the issue and the design doc.
2. **Component shape.** One mermaid diagram, grouped by owning repo or module,
   showing the forward data flow and the intent flowing back. Each node is a
   component a teammate can find in code.
3. **Ownership and contracts.** A table with one row per component:

   | Component | Owns | Input → output |
   |---|---|---|

   Name real types, verbs and fields, not concepts. Follow the table with the
   refusals: one sentence per boundary saying what that component must *not*
   do even though it could. For example: the adapter does not filter
   visibility, and the renderer does not compute gameplay from mesh bounds. The
   refusals are the seams' teeth. They are what a reviewer checks a diff
   against.
4. **Walk one thing through.** Follow one concrete authored or input value
   from source to screen. At each hop, name the type it becomes, which
   identity survives and which is derived, and what the hop adds or drops.
   Use a small, real example rather than a generic placeholder.
5. **Separations that look like one thing.** Where two questions are easy to
   conflate, give each its own source of truth:

   | Query | Source of truth |
   |---|---|

   Then state the hazard in the present tense: what an implementation that
   merges them would get wrong. This section prevents the bug a teammate
   would otherwise write in good faith.
6. **Trade-offs.**

   | Decision | Benefit | Cost / boundary | Not taken |
   |---|---|---|---|

   *Not taken* names the rejected alternative in a phrase, so the reader
   knows that road exists. The argument for rejecting it stays in the case
   file. Every cost names who pays it: the compiler, the author, the renderer.
7. **Edges.** What the model does not do and does not claim, such as a planar
   model, no derived room membership, or supported placements only. Each edge
   that a later use case could reopen points to its ruling or open question.
8. **Where a change lands.** Name two or three changes a teammate is likely to
   make, and which owner absorbs each, using only the components above. This
   tests the seams, not a roadmap. If a likely change touches every
   component, say so: that is a finding about the design.
9. **Source map.** A table mapping concern to path, at the package or file
   level, on the branch or module where the code lives. Use symbols, not line
   numbers.

## The voice

- **A teammate at a whiteboard.** Use plain sentences, real names, and one
  idea per paragraph. Assume the reader can code, not that they know the
  history.
- **Present tense, every why.** State why the cut is where it is and the cost
  it avoids, as a standing fact: "Appearance is separate from blocking, so a
  presentation change cannot alter rules." Do not narrate how the design was
  reached.
- **Bold the load-bearing phrase** of the few sentences a reader must not
  skim past. Bolding everything is the same as bolding nothing.
- **Tables for ownership, contracts and trade-offs. Prose for reasoning.**
  Prose for reasoning means sentences rather than bullets that each hold
  half a thought.

## What stays out

The walkthrough is loaded by future sessions, so the case-file rule applies to
it as it does to the law:

- **Delivery status.** Merged, draft and next-slice PR lists change weekly.
  They belong in the issue. A walkthrough that lists them rots between merges.
- **Correction stories.** "The earlier implementation returned …" is case
  file. State the present contract, the hazard it guards, and the test that
  pins it. Leave the story in the PR that fixed it.
- **Restated law.** Cite the ruling ID. Do not paraphrase the ruling into a
  second, subtly different authority.
- **Direction as narrative.** "The operator's direction is …" is a ruling
  that has not been recorded yet. Make it a ruling row in the design doc and
  cite it.
- **Implementation plans.** Task contracts belong to the plan
  ([planning.md](planning.md)). The walkthrough's ownership table and
  separations are the plan's input, not its tasks.

Run `game-dev/scripts/check-doc-genres.sh` on the folder before publishing.
The walkthrough is not gated by the check, so read its findings by hand.

## The test

Before showing the walkthrough to the operator, apply this test:

> Could a teammate who codes, given only this walkthrough and the design doc,
> place a new change in the right component, name the type it crosses each
> seam in, and say which refusal a wrong placement would violate?

If not, the walkthrough is missing an owner, a contract or a refusal. Fix the
gap before showing it.
