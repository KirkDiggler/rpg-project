# When a Rule Needs the Room Visual Draft Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Build the concise six-panel, self-contained, unpublished `file://` draft for Layer 4, **When a Rule Needs the Room**.

**Architecture:** One HTML document continues the D20 field-notes visual system. The page moves from reusable calculation seams to the `gamectx` read breakthrough, then ends where live mutation creates the persistence question.

**Tech Stack:** Semantic HTML, embedded CSS, inline SVG, Google Fonts, Python structural/content checks, system Chrome screenshots.

**Spec:** `ideas/story/toolkit-series/layer-4-focus-brief.md`

## Global Constraints

- Create only `ideas/story/toolkit-series/site/when-a-rule-needs-the-room-field-notes.html`.
- Do not edit or publish `gh-pages`.
- Open through `file://`; do not run a server.
- Keep `Local draft · unpublished` visible.
- Use exactly six panels and one SVG figure per panel.
- Give each panel one short lead paragraph, one figcaption, and one closing line.
- Keep each lead at or below 85 words, each figcaption at or below 35 words, and each closing line at or below 25 words.
- Keep visible non-footer prose below 850 words, excluding SVG labels.
- Keep diagrams to seven primary nodes or fewer where possible.
- Use the existing D20 colors, typography, favicon, and mobile swipe behavior.
- Keep `gamectx` central; treat mutation only as the final warning.
- Exclude detailed feature inventories, later chain wiring, the repository gap, request-shaped writes, and resolution machines.
- Do not commit before Kirk reviews the draft.

---

### Task 1: Create the shell and concise opening half

**Files:**
- Create: `ideas/story/toolkit-series/site/when-a-rule-needs-the-room-field-notes.html`
- Reference: `ideas/story/toolkit-series/site/rage-is-not-raging-field-notes.html`

**Interfaces:**
- Consumes: Layer 4 focus brief and existing D20 page structure.
- Produces: header plus Panels 1–3.

- [x] Verify the target file does not exist.
- [x] Create the semantic shell, unpublished banner, predecessor links, six-panel rail, footer, relative favicon metadata, and responsive style.
- [x] Add Panel 1: compact Rage, Second Wind, and Unarmored Defense seam rows with chronology caveats in prose rather than extra nodes.
- [x] Add Panel 2: Dueling expands through Sneak Attack and Protection from hands to room, teams, shield, and reaction.
- [x] Add Panel 3: one small event accumulates rust-colored world fields until it becomes a kitchen sink.
- [x] Verify three panels, three figures, prose budgets, and absence of later architecture.

### Task 2: Implement the exploration, breakthrough, and warning

**Files:**
- Modify: `ideas/story/toolkit-series/site/when-a-rule-needs-the-room-field-notes.html`

**Interfaces:**
- Consumes: Panels 1–3 and the verified December/January chronology.
- Produces: Panels 4–6 and evidence footer.

- [x] Add Panel 4: four rejected homes around one runtime-knowledge question.
- [x] Add Panel 5: `context.Context` realization, host/rulebook/core ownership, and three narrow gamectx reads. Give this panel the largest blue diagram.
- [x] Add Panel 6: full read-to-mutation path with blue read half, rust write half, dirty tracking, and explicit “intended, not yet fully wired” save label.
- [x] End with “Context gave rules a way to ask what was true. Then we let them change the answer.”
- [x] Add the minimal evidence trail from Journey 046, Journey 048, and the named commits in the focus brief.

### Task 3: Enforce digestibility and verify visually

**Files:**
- Verify: `ideas/story/toolkit-series/site/when-a-rule-needs-the-room-field-notes.html`
- Output only: `/tmp/when-a-rule-needs-the-room-*.png`

**Interfaces:**
- Consumes: complete local HTML.
- Produces: structural and visual evidence for review.

- [x] Parse the HTML and assert six panels, six unique panel-title IDs, six figures, draft banner, favicon links, approved laws, and exclusions.
- [x] Calculate lead, figcaption, closing-line, and total prose word counts; fail if any budget is exceeded.
- [x] Scan for malformed fragments, placeholders, escaped newlines, false `HealChain`/AC claims, and claims that automatic dirty saving already shipped.
- [x] Render top, all six panels, and key mobile views directly through `file://` using system Chrome.
- [x] Inspect every render for hierarchy, label contrast, body overflow, swipe cues, blue/rust distinction, and the visual dominance of Panel 5.
- [x] Report the direct file URL and state that nothing was committed or published.
