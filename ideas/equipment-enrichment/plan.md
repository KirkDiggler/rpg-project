---
name: Equipment Enrichment — Implementation Plan
description: Kirk's phased decision for the concrete-options wave — proto contract + additive web picker now, toolkit/Monk resolution and API translation deferred
updated: 2026-08-01
status: approved — phase 1 ready to start
confidence: high — phasing is Kirk's explicit call, not a proposal
---

# Plan: Equipment Enrichment — concrete-options wave, phased

Implements the "2026-08-01 update" section of
`rpg-project/ideas/equipment-enrichment/design.md` (the MEATY category-choice
wave), **split into two phases by Kirk's decision** rather than landed as one
cross-repo change. This document is the phasing decision and the gates between
the phases; it does not re-derive the architecture already settled in
`design.md`.

## Why phase instead of landing the whole wave

The design's Boundary Rule requires the toolkit to be the single owner of
category-choice eligibility (Monk's `simple-melee` + `simple-ranged` expansion
being the sharp edge that proves it). That toolkit work — and the API
translation layer that depends on it — is real, non-trivial rules work. Kirk's
call: **don't let the UI wait on that** when the proto contract and a rich,
accessible web picker can be built and reviewed now against typed fixtures,
with the toolkit/API work following once it's ready, without the web PR being
blocked or needing to be reopened.

This also protects the boundary itself: building the picker against fixtures
first — rather than against a fake, UI-side eligibility reconstruction wired
to production — means there is no client-side rules logic to *unwind* later.
The wave still ends exactly where `design.md` says it ends (toolkit resolves,
API translates, web renders); phasing only changes the order artifacts land
in, not the destination.

## Phase 1 — now

**Scope:** proto contract + additive, rich, accessible web picker, built and
reviewed against typed fixtures. No production wiring to fake eligibility.

1. **Protos** — land the additive `options` field on `EquipmentCategoryChoice`
   exactly as specified in `design.md` ("Proto: additive, repeated concrete
   options"). This is real, mergeable, non-breaking — existing consumers see
   an empty `options` until the API starts populating it.
2. **Web — typed fixtures** — hand-author a typed fixture set representing
   resolved `EquipmentCategoryChoice.options` payloads (concrete, enriched
   `EquipmentItem`s, `equipment_detail` populated) covering: a simple category
   (few options), a large category (Monk-shaped — enough options to exercise
   scrolling/filtering, not a hand-picked shortlist that hides layout bugs),
   and a `choose_count > 1` category. Fixtures are typed against the proto
   message so they can't silently drift from the real wire shape, and they
   are clearly named/located as fixtures (not confused with live data).
3. **Web — the picker component** — build the rich accessible picker
   (keyboard navigation, screen-reader labeling, visible selection state up to
   `choose_count`) using `EquipmentCard` (design.md Layer 4) per option,
   driven entirely by the typed fixtures. This is additive: it ships alongside
   the existing `ListEquipmentByType` + client-side eligibility path, not in
   place of it.
4. **Local preview** — the picker gets its own dev preview, run separately
   from the main game client so reviewers can look at it in isolation without
   spinning up the full backend stack:
   - **Equipment preview runs on port 3002.**
   - **Dungeon-builder's own dev server (port 3001) and its lab API/Redis
     stack (`rpg-api-lab`/`rpg-api-lab2`, primary Redis) are left completely
     untouched** — this preview does not compete for those ports or that
     backend, and does not require them running at all.

**Phase 1 gate (must all be true before Phase 2 starts):**
- [ ] Protos PR merged with the additive `options` field, `buf breaking`
      clean (additive-only)
- [ ] Typed fixtures exist and are reviewed — cover simple/large/multi-select
      cases
- [ ] Picker renders and is keyboard/screen-reader navigable against fixtures
      only — **no code path wires it to `ListEquipmentByType` or any
      client-side eligibility reconstruction**
- [ ] Preview runs standalone on port 3002; dungeon-builder (3001) and its
      lab API/Redis are verified untouched (start Phase 1 preview alone, spot
      check the primary/lab stacks are unaffected)
- [ ] Copilot review addressed on both PRs (protos, web-fixtures-and-picker)

## Phase 2 — deferred, opens once Phase 1 gate is clear

**Scope:** the actual rules work — toolkit category-choice expansion (Monk
included, per design.md's "sharp edge" test) and the API translation layer
that maps it onto `options` on the wire.

1. **Toolkit** — implement category-choice expansion per design.md's Decision
   section. Monk's `simple-melee` + `simple-ranged` expansion asserted against
   the full weapon registry count, not a hand-picked subset. Existing
   special-exclusion cases get exclusion tests. Tag and publish.
2. **API** — bump to the toolkit's real published tag (no local `replace`
   left behind — see design.md's Development note on the local-override
   discipline), map the toolkit's resolved `options` onto the wire using the
   existing concrete-item `EquipmentItem` mapping (Cost/Weight conversion
   reused, not duplicated).
3. **Web — swap fixtures for the live wire** — the same PR that starts reading
   real `options` from the API is the PR that deletes `ListEquipmentByType`
   and the client-side eligibility reconstruction (design.md: "no dark period
   where both paths are live and could disagree"). The Phase 1 picker
   component itself does not need to change — only what feeds it. Fixtures
   move from "the only input" to "test-only input."

**Phase 2 gate (ready-to-start check, before this phase's PRs open):**
- [ ] Phase 1 gate fully closed
- [ ] Toolkit expansion test suite green, including the Monk full-registry
      assertion and exclusion-carry-over tests
- [ ] API mapping test asserts wire `options` matches toolkit expansion 1:1

## Compatibility and rollback

- **Proto:** additive field on an existing message — old clients unaffected
  (empty `options`); no version bump beyond the normal additive process
  (matches design.md's Migration section).
- **Web, Phase 1:** the picker ships disabled from production traffic (fixture
  -only, not routed from the live equipment-choice flow) until Phase 2 wires
  real data — so Phase 1 can be reverted independently of Phase 2 by simply
  not merging/rolling back the picker's route registration; it carries no
  runtime dependency on the toolkit/API work.
- **Web, Phase 2:** the `ListEquipmentByType` deletion and the picker's
  data-source swap land in the same PR (per design.md), so there is no
  intermediate state where the old and new paths could disagree. Rollback of
  that PR restores the pre-Phase-2 path (`ListEquipmentByType` + client-side
  reconstruction) exactly, since it isn't deleted until that same PR.
- **Toolkit/API (Phase 2):** standard toolkit→API publish/bump discipline
  applies; no schema/data migration since this is a request/response shape
  change only (design.md's Migration section already covers this — unchanged
  by phasing).

## Later: local toolkit override for Phase 2 development

Per design.md's Development note, once Phase 2 starts, rpg-api's worktree for
this wave may point its toolkit module at a local toolkit worktree via the
documented local-override mechanism
(`rpg-api/docs/how-to/local-toolkit-override.md`) to iterate against
unpublished toolkit changes. This override is **never committed** — it is
stripped (real published tag substituted) as part of the API PR that actually
merges Phase 2's API work. Only the one `rpg-toolkit` module for this wave gets
overridden; needing more is the signal the wave was sliced too thin
(working-agreements.md `Unmerged provider work`). This does not apply to
Phase 1 — Phase 1 has no toolkit or API dependency to override.

## Merge order

**Inside-out, per rpg-project's wave-shape rule, applied per phase:**

Phase 1 (now):
1. **Protos** — additive `options` field, lands first (or in parallel with
   web fixture authoring, since fixtures are typed against the message
   whether or not it's merged yet — but the picker PR depends on the real
   generated types, so protos must merge before the picker PR does).
2. **Web** — typed fixtures + picker, lands after protos.

Phase 2 (deferred, opens on Phase 1 gate):
1. **Toolkit** — category-choice expansion, land and tag.
2. **Protos** — already landed in Phase 1; no further proto change needed
   unless review surfaces a gap.
3. **API** — bump to the toolkit's real published tag, map `options` onto the
   wire, remove any local `replace` directive before merge.
4. **Web** — consume live `options`, delete `ListEquipmentByType` + the
   client-side eligibility reconstruction. Lands last because it depends on
   the API being live with the real contract.

This is design.md's original toolkit→API→web order, preserved — phasing only
inserts a checkpoint between the proto/web-fixture work and the
toolkit/API/web-live work; it does not reorder anything within either phase.

## What this plan does NOT do (deferred, tracked by design.md)

- Does not implement toolkit category-choice expansion or Monk resolution —
  that is all of Phase 2, deferred until Phase 1's gate is clear.
- Does not implement the API translation layer — Phase 2, same gate.
- Does not wire the Phase 1 picker to any production data source or to a
  client-side eligibility reconstruction of any kind — fixtures only, by
  design, until Phase 2 supplies the real wire data.
- Does not touch dungeon-builder's dev server (port 3001) or its lab
  API/Redis stack (`rpg-api-lab`, `rpg-api-lab2`, primary Redis) — the
  equipment preview is fully standalone on port 3002.

## Acceptance criteria — Phase 1 PR(s) ready to merge

- [ ] Protos PR: additive `options` field only, `buf breaking` clean, no
      other message shape changes
- [ ] Web PR: typed fixtures (simple/large-Monk-shaped/multi-select cases) +
      accessible picker component, built against `EquipmentCard`
- [ ] Preview verified running on port 3002 in isolation
- [ ] Dungeon-builder (3001) and lab API/Redis spot-checked untouched during
      preview verification
- [ ] No production route wires the picker to fake/reconstructed eligibility
- [ ] Copilot review addressed, no `--no-verify` in commit history
- [ ] PR description links this plan.md and `design.md`'s 2026-08-01 section

## Citations

- Design: `rpg-project/ideas/equipment-enrichment/design.md` (2026-03-21 base
  design + 2026-08-01 concrete-options architecture decision)
- Boundary Rule: `rpg-project/AGENTS.md`
- Local toolkit override mechanism: `rpg-api/docs/how-to/local-toolkit-override.md`
- Merge-order convention: `docs: how a wave is shaped — develop outside-in,
  merge inside-out` (rpg-project commit `cedc2d4`)
- Working agreements — unmerged provider work discipline:
  `working-agreements.md`
