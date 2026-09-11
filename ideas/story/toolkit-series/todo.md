# Toolkit story series TODO

## Shared presentation

- [x] Design a D20 favicon for the series.
- [x] Apply the same favicon to `kirkdiggler.github.io/rpg-project/`, `kirkdiggler.github.io/rpg-toolkit/`, and every future field-note chapter.
- [x] Include standard browser favicon metadata plus an Apple touch icon where practical.
- [x] Keep the icon legible at 16×16; use a simple silhouette/face pattern rather than tiny numbers or detailed linework.

## Publishing workflow

- Once Kirk approves a story draft, publish it directly to the existing `gh-pages` story branch.
- Do not open a PR or request another review unless Kirk explicitly asks. This is an informal storytelling series, not formal project delivery.
- Keep the research ledger and local draft; add production metadata, the social card, analytics, and chapter navigation during publication.

## Layer 2: The Nervous System

Published at https://kirkdiggler.github.io/rpg-toolkit/the-nervous-system/.

- [x] Use Rage as the opening persistence/behavior problem.
- [x] Make Bless the visual and emotional center of the page.
- [x] Show what the first event bus could actually do before `.On(bus)`.
- [x] Show the concentration relationship tying one caster to several Blessed conditions.
- [x] Be explicit that the early demo announced a failed concentration save and broke the relationship directly; it did not yet pause for player input or resolve the save.
- [x] Show the later Rage lock regression, typed topics, and chained topics in chronological order.
- [x] Close on the synchronous limitation: `The event bus could ask every rule in memory. It could not wait for a person who was not there yet.`
- [x] Prepare the copy-ready LinkedIn introduction at `ideas/story/linkedin/layer-2-nervous-system-post.txt`.

## Layer 3: Rage Is Not Raging

Published at https://kirkdiggler.github.io/rpg-toolkit/rage-is-not-raging/.

- [x] Show the early Rage Feature owning activation and active-state behavior.
- [x] Separate Rage from Raging.
- [x] Use the compiler-enforced circular dependency as the architectural turn.
- [x] Assign activation, active state, condition collection, and handoff vocabulary to focused owners.
- [x] Show the final typed `ConditionAppliedEvent` handoff through `dnd5e/events`.
- [x] End on the source-backed `8 + 3 + 2 = 13` damage breakdown.
- [x] Keep interruption, resolution machines, Unarmored Defense, and the repository gaps outside the cut.
- [x] Prepare the copy-ready LinkedIn introduction at `ideas/story/linkedin/layer-3-rage-is-not-raging-post.txt`.

## Layer 4: When a Rule Needs the Room

Published at https://kirkdiggler.github.io/rpg-toolkit/when-a-rule-needs-the-room/.

- [x] Show Rage, Second Wind, and Unarmored Defense exposing reusable seams.
- [x] Use Dueling as the first rule that needs runtime knowledge outside the event.
- [x] Expand the need through Sneak Attack and Protection.
- [x] Show why events, chain callbacks, cached condition state, and bus registries are the wrong homes.
- [x] Make request-scoped `gamectx` reads the visual and emotional center.
- [x] End on the missing owner-controlled write path.
- [x] Preserve the caveat that dirty tracking shipped while the generic save sweep remained intended.
- [x] Keep request-shaped writes and resolution outside the cut.
- [x] Prepare the copy-ready LinkedIn introduction at `ideas/story/linkedin/layer-4-when-a-rule-needs-the-room-post.txt`.

## Layer 5: A Request Needs an Owner

Published at https://kirkdiggler.github.io/rpg-toolkit/a-request-needs-an-owner/.

- [x] Establish the dungeon prototype as deliberate seam-learning work.
- [x] Explain the request as a boundary between what a rule wants and how the state owner applies it.
- [x] Show Actions becoming things a Character can hold.
- [x] Use Flurry of Blows granting two temporary strikes as the stress test.
- [x] End on the missing interaction driver.
- [x] Prepare the copy-ready LinkedIn introduction at `ideas/story/linkedin/layer-5-a-request-needs-an-owner-post.txt`.
