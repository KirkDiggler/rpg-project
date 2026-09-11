# Rage Is Not Raging Visual Draft Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Build the complete six-panel, self-contained, unpublished `file://` draft for Layer 3, **Rage Is Not Raging**.

**Architecture:** Create one HTML document using the established D20 field-notes visual system. Each panel carries one narrative responsibility and one SVG diagram. Preserve the approved chronology safeguards and finish on the exact source-backed `13` damage breakdown.

**Tech Stack:** Semantic HTML, embedded CSS, inline SVG, Google Fonts, Python structural checks, Playwright screenshot harness.

**Spec:** `ideas/story/toolkit-series/layer-3-focus-brief.md`

## Global Constraints

- Create a local draft only; do not edit either `gh-pages` branch.
- The draft opens directly through `file://`; do not start a server.
- Keep the visible `Local draft · unpublished` banner.
- Use exactly six numbered panels.
- Use one main SVG diagram per panel.
- Use no more than two short prose paragraphs before each diagram.
- Use one law or personal line after each diagram.
- Use `#11141a` ground, `#7189d7` seam blue, and `#c96d62` pressure rust.
- Use Source Serif 4, IBM Plex Sans Condensed, and IBM Plex Mono according to the focus brief.
- Use the existing relative `favicon.svg` and `apple-touch-icon.png` assets.
- Do not include the `Data any` intermediate, repository pause, Unarmored Defense, broad Monster architecture, chain interruption, or resolution machines.
- Do not commit or publish the draft before Kirk reviews it.

---

### Task 1: Create the page shell and continuity handoff

**Files:**
- Create: `ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html`
- Reference: `ideas/story/toolkit-series/site/the-nervous-system-field-notes.html`

**Interfaces:**
- Consumes: D20 field-notes CSS patterns and the approved Layer 3 focus brief.
- Produces: one self-contained document with header, timeline rail, six panel slots, footer, favicon metadata, and mobile breakpoints.

- [x] **Step 1: Establish the structural failure check**

Run before creating the page:

```bash
test -f ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html
```

Expected: non-zero because the draft does not exist.

- [x] **Step 2: Create the semantic shell**

Create:

```text
Local draft · unpublished
Layer 3 · Actions and active state
Rage Is Not Raging
six-panel timeline
Evidence trail
```

The header links back to:

```text
https://kirkdiggler.github.io/rpg-project/
https://kirkdiggler.github.io/rpg-toolkit/the-nervous-system/
```

- [x] **Step 3: Add the approved visual system**

Carry forward the field-note typography, rail, ruled sections, SVG classes, accessible focus styles, mobile swipe cue, and reduced-motion treatment. Use relative favicon links:

```html
<link rel="icon" type="image/svg+xml" href="favicon.svg">
<link rel="apple-touch-icon" href="apple-touch-icon.png">
```

- [x] **Step 4: Verify the shell**

Run a Python `HTMLParser` check asserting:

```text
lang="en"
Local draft · unpublished
Rage Is Not Raging
six data-panel attributes numbered 1 through 6
no duplicate IDs
```

Expected: PASS.

---

### Task 2: Implement Panels 1–3, from overload to compiler refusal

**Files:**
- Modify: `ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html`

**Interfaces:**
- Consumes: the early Rage responsibility inventory and chronology in the focus brief.
- Produces: the problem half of the story, ending at the compiler-enforced dependency cycle.

- [x] **Step 1: Add Panel 1, One object doing everything**

Show one rust Rage Feature with wires to:

```text
uses
activation
attack subscription
damage bonus
resistance
activity tracking
unsubscribe
ending
```

Label the brief rewind: commit `c578c911`, one day before final `.On(bus)`.

- [x] **Step 2: Add Panel 2, Rage is not Raging**

Split the overloaded node into:

```text
Rage Feature                 Raging Condition
can activate                 is active
spends a use                 changes calculations
starts the handoff           watches its ending
```

End with:

> Features activate. Conditions persist.

- [x] **Step 3: Add Panel 3, The compiler agrees**

Draw the rust package loop among features, conditions, and D&D event definitions. Place a concise Go circular-import error in the center.

Do not mention the repository pause or `Data any` intermediate.

- [x] **Step 4: Verify the first half**

Assert the document contains:

```text
c578c911
Rage is not Raging
Features activate. Conditions persist.
circular dependency
```

Assert it does not contain:

```text
48-day
Data any
Unarmored Defense
```

Expected: PASS.

---

### Task 3: Implement Panels 4–6, from responsibility to observable result

**Files:**
- Modify: `ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html`

**Interfaces:**
- Consumes: the final November package split, Character handoff, and archived damage values.
- Produces: the solution half of the story and its visible proof.

- [x] **Step 1: Add Panel 4, Give the job to the thing that can own it**

Reveal `Action[T]` as the earlier contract already waiting, not a later invention. Show:

```text
Rage             CanActivate + Activate
Raging           temporary behavior and ending
Character        active-condition collection
dnd5e/events     shared handoff contract
```

End with:

> Put each responsibility with the thing that can do it well and own the result.

- [x] **Step 2: Add Panel 5, Give the handoff neutral ground**

Replace the Panel 3 cycle with one-way dependencies on `dnd5e/events`. Then show:

```text
Rage creates RagingCondition
ConditionAppliedEvent carries ConditionBehavior
Character receives, applies, and stores it
Raging owns what happens next
```

Make the event bus the route, not the owner.

- [x] **Step 3: Add Panel 6, Why is this 13 damage?**

Keep the combat setup to:

```text
Barbarian → Monster → DamageChain
```

Use the archived values exactly:

```text
1d12 weapon damage   8
Strength modifier   +3
Rage bonus          +2
total damage        13
```

Make the `Rage +2` line blue and visually dominant without overpowering the total.

End with:

> Composable rules still need to explain themselves.

- [x] **Step 4: Add the evidence trail**

Cite the minimal commit set:

```text
b553c735
c578c911
65516107
a6a87e09
6f428d47
8c1c0606
0228d23a
0fd81d75
```

Do not add a next-chapter promise.

---

### Task 4: Validate the complete local draft

**Files:**
- Verify: `ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html`
- Output only: `/tmp/rage-is-not-raging-*.png`

**Interfaces:**
- Consumes: the finished local HTML document.
- Produces: structural and visual evidence for Kirk's review; no publication changes.

- [x] **Step 1: Run structural validation**

Use Python `HTMLParser` to verify:

```text
exactly six panels numbered 1–6
unique panel title IDs
one figure per panel
local draft banner present
favicon and touch icon present
approved title and laws present
exact breakdown values present
```

Also reject every explicit exclusion in the focus brief.

- [x] **Step 2: Run whitespace and corruption scans**

Search for malformed fragments, placeholder markers, accidental escaped newlines, duplicate panel numbers, and draft-writing artifacts. Use a pattern equivalent to:

```text
T[B]D|T[O]DO|^text$|^5$|\\n|LoadFrom meltData|ConditionAppliedEvent<ConditionBehavior>
```

Expected: no matches.

- [x] **Step 3: Render through direct file URLs**

From `tools/browser/`, run `screenshot.mjs` against:

```text
file:///home/kirk/game-dev/rpg-project/ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html
```

Capture:

```text
/tmp/rage-is-not-raging-top-desktop.png       1440×1000
/tmp/rage-is-not-raging-split-desktop.png     1440×1000
/tmp/rage-is-not-raging-compiler-desktop.png  1440×1000
/tmp/rage-is-not-raging-handoff-desktop.png   1440×1000
/tmp/rage-is-not-raging-breakdown-desktop.png 1440×1000
/tmp/rage-is-not-raging-top-mobile.png        390×844
/tmp/rage-is-not-raging-split-mobile.png      390×844
/tmp/rage-is-not-raging-breakdown-mobile.png  390×844
```

- [x] **Step 4: Inspect every render**

Check:

```text
no clipped body content
headings wrap intentionally
smallest labels remain readable
diagrams scroll inside figures rather than widening the body
rust cycle and blue foundation are immediately distinguishable
13-damage breakdown is the final visual climax
```

- [x] **Step 5: Report the draft for review**

Provide the direct URL:

```text
file:///home/kirk/game-dev/rpg-project/ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html
```

State explicitly that nothing was published or committed.
