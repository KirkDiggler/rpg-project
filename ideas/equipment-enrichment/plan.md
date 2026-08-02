---
name: Equipment Enrichment — Implementation Plan
description: Kirk's corrected phased decision — web-only rich dropdown rendering from existing live data now; concrete-options/toolkit/API (Monk eligibility) deferred
updated: 2026-08-02
status: approved — corrected phase 1 (web-only, live data) ready to start
confidence: high — phasing is Kirk's explicit call, not a proposal
---

# Plan: Equipment Enrichment — concrete-options wave, phased (corrected 2026-08-02)

Implements the "2026-08-01 update" section of
`rpg-project/ideas/equipment-enrichment/design.md` (the MEATY category-choice
wave) as the **deferred future phase**, and the **2026-08-02 correction**
section as the current phase. This document is the phasing decision and the
gates between the phases; it does not re-derive the architecture already
settled in `design.md`.

## 2026-08-02 correction — supersedes the original Phase 1 below

The original Phase 1 (below, kept for history) assumed the current
dropdown-rich-content work needed the additive proto field
(`EquipmentCategoryChoice.options`) plus a fixtures-only picker exercised in a
standalone Concepts Lab / Simulate Loading preview (port 3002) before any
production wiring. **That assumption was wrong.** Kirk's correction:

- `ListEquipmentByTypeResponse.equipment` (rpg-api-protos, already merged,
  already live) returns full `Equipment` per item today —
  `WeaponData`/`ArmorData`, cost, weight, description. The web already calls
  this endpoint for the production category dropdown. **No new contract,
  and no fixtures/concept lab, are needed to render rich content there** —
  the real data is already flowing.
- rpg-api-protos#204 (implementing the additive `options` field) is closed
  without merge. rpg-api-protos#202, rpg-toolkit#872, rpg-api#755 move to
  Todo/deferred — they remain the tracking issues for the *exact,
  toolkit-resolved* concrete-eligibility leg (Monk's full
  `simple-melee`/`simple-ranged` set, special exclusions), which is a real
  future need, just not part of this pass.
- rpg-dnd5e-web#668 is rescoped to formatting the *existing* production
  dropdown from `ListEquipmentByType`'s already-rich response, explicitly
  excluding Monk/exact category eligibility.

## Phase 1 (corrected) — now: web-only rich dropdown formatting from live data

**Scope:** improve the rendering of the existing production equipment
dropdown (the one already backed by `ListEquipmentByType`) so it shows the
`Equipment` fields already present on the wire — damage dice/type,
properties, AC, weight, cost, etc. — using `EquipmentCard` (design.md Layer
4) per option. No proto change. No fixtures. No standalone preview. Ships
directly against live data in the existing dropdown.

1. **Web only** — format `EquipmentCard`-shaped rich content for each item in
   the existing `ListEquipmentByType`-backed dropdown, reading fields already
   present in the live `Equipment` message (no schema change).
2. **No concept/fixtures lab** — dropped from this pass; live data already
   exists, so there is nothing a fixture-only preview would prove that the
   production dropdown itself doesn't already prove.
3. **Explicitly excluded from this pass:** exact, toolkit-resolved category
   eligibility (which weapon IDs are valid for "choose a martial weapon",
   Monk's full set, special exclusions). The dropdown keeps whatever
   category→type filtering it does today; only the *rendering* of each
   listed item gets richer. That eligibility work is Phase 2 (deferred, was
   design.md's 2026-08-01 decision), unchanged.

**Phase 1 (corrected) gate:**
- [ ] Web PR renders rich `EquipmentCard` content in the existing production
      dropdown using only fields already on `ListEquipmentByType`'s live
      `Equipment` response — no proto change, no fixture-only code path
- [ ] No client-side eligibility reconstruction is added or changed as part
      of this pass (that stays exactly as it is today until Phase 2)
- [ ] rpg-dnd5e-web#668 checklist matches this scope (rich formatting only,
      Monk/exact eligibility explicitly out of scope)

## Phase 2 — deferred, unchanged from the 2026-08-01 design decision

**Scope:** the actual rules work — toolkit category-choice expansion (Monk
included, per design.md's "sharp edge" test) and the API translation layer
that maps it onto a new `options` field, plus the web swap that deletes
`ListEquipmentByType`'s client-side eligibility reconstruction. This is
exactly design.md's 2026-08-01 decision — postponed, not changed.

1. **Toolkit** (rpg-toolkit#872, deferred) — implement category-choice
   expansion per design.md's Decision section. Monk's `simple-melee` +
   `simple-ranged` expansion asserted against the full weapon registry
   count, not a hand-picked subset. Existing special-exclusion cases get
   exclusion tests. Tag and publish.
2. **Protos** (rpg-api-protos#202, deferred, tracking issue kept open) — land
   the additive `options` field on `EquipmentCategoryChoice` exactly as
   specified in design.md. PR #204 (the prior implementation attempt) was
   closed without merge as premature; a fresh PR opens against #202 once
   Phase 2 actually starts.
3. **API** (rpg-api#755, deferred) — bump to the toolkit's real published
   tag (no local `replace` left behind — see design.md's Development note),
   map the toolkit's resolved `options` onto the wire using the existing
   concrete-item `EquipmentItem` mapping (Cost/Weight conversion reused, not
   duplicated).
4. **Web** — the same PR that starts reading real `options` from the API is
   the PR that deletes `ListEquipmentByType` and the client-side eligibility
   reconstruction (design.md: "no dark period where both paths are live and
   could disagree"). Rich formatting built in Phase 1 (corrected) carries
   forward — only the data source changes.

**Phase 2 gate (ready-to-start check, before this phase's PRs open):**
- [ ] Phase 1 (corrected) shipped and verified in production
- [ ] Toolkit expansion test suite green, including the Monk full-registry
      assertion and exclusion-carry-over tests
- [ ] API mapping test asserts wire `options` matches toolkit expansion 1:1
- [ ] Fresh PRs opened against rpg-api-protos#202, rpg-toolkit#872,
      rpg-api#755 (moved back to In Progress at that point)

## Compatibility and rollback

- **Phase 1 (corrected):** pure rendering change against an already-live,
  already-shipped response shape — no schema change, so rollback is a
  standard web revert with no cross-repo coordination.
- **Phase 2 (deferred, unchanged):** additive proto field, standard
  toolkit→API→web publish/bump discipline; see design.md's Migration section
  (unchanged by this correction).

## What this plan does NOT do (deferred, tracked by design.md)

- Does not implement toolkit category-choice expansion or Monk resolution —
  that is all of Phase 2, deferred.
- Does not implement the API translation layer or the additive proto field —
  Phase 2, same gate. rpg-api-protos#204 (prior implementation attempt) is
  closed without merge; issue #202 stays open as the deferred tracking issue.
- Does not build a fixtures-only picker or standalone preview — dropped
  entirely from this pass; live data already exists.
- Does not touch dungeon-builder's dev server (port 3001) or its lab
  API/Redis stack — moot for this pass since no standalone preview is being
  built at all.

## Citations

- Design: `rpg-project/ideas/equipment-enrichment/design.md` (2026-03-21 base
  design + 2026-08-01 concrete-options architecture decision + 2026-08-02
  correction)
- Boundary Rule: `rpg-project/AGENTS.md`
- Merge-order convention: `docs: how a wave is shaped — develop outside-in,
  merge inside-out` (rpg-project commit `cedc2d4`)
- Working agreements — unmerged provider work discipline:
  `working-agreements.md`

## History (superseded original Phase 1 — kept for record)

The original 2026-08-01 plan called for landing the additive proto field
*and* a fixtures-only, accessible web picker in a standalone preview (port
3002) as "Phase 1," before any Phase 2 toolkit/API work. That plan is
superseded by the 2026-08-02 correction above: it turned out no new contract
or fixture lab was needed at all, because `ListEquipmentByType` already
returns rich `Equipment` data live. The standalone-preview infrastructure
described there (port 3002, dungeon-builder's port 3001/lab stack left
untouched) was never built and is not needed under the corrected plan.
