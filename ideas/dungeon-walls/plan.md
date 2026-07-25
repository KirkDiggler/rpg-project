# Dungeon Walls — Implementation Plan

**Status:** execution plan (design approved on this PR, 2026-07-25)
**Owner:** asset-pipeline team (client seams) — all code work in `rpg-dnd5e-web`
**Dependencies:** none on toolkit/proto/api — v1 is client-derived. toolkit#849
and the rpg-api encounter v0.44.1 bump improve the interim look and make
connector wall entries uniform on the wire, but nothing below depends on their
timing (see W2's category rule, which is robust to both wire shapes).

## Issue / PR structure

- **One implementation issue** in rpg-dnd5e-web (sub-issue of rpg-project#132;
  board 19: Feature="The Dungeon", Team=Assets, Kind=Build) with the four
  slices below as its checklist.
- **Two PRs**: PR-A = W1+W2 (run computation + placeholder render — the first
  visible win), PR-B = W3+W4 (Synty meshes + tuning). Each PR lands runnable
  with screenshot evidence — no long dark stretches.
- Repo law: `npm run ci-check` before push; playtest-evidence screenshots on
  the PR; ready-for-review only, never draft.

## W1 — Run computation (pure module + tests)

A pure module (`wallRuns` or similar): inputs are exactly what the encounter
map already receives — regions' hex sets, doors, combined space dimensions;
outputs are per-room **envelope runs** and per-connector **column runs** in
world coordinates, each with door gaps.

Rules (from the design's implementation notes):
- Room bounding rect = min/max over the region's `Hexes` (no width/offset
  fields exist on the wire).
- Connector column = in-bounds cells belonging to no region; the door cell
  (from Doors data) splits that column's run into two segments.
- Use the client's existing hex→world transform for endpoints; never assume
  same-row column adjacency (odd-q parity — real neighbor math only).
- Envelope line = outer envelope + a configurable outward offset (the
  clip-clearance dial, tuned in W4).

**Acceptance:** unit tests against a reference-tomb-shaped fixture (3 rooms,
2 connectors, shared doorRow, locked door) plus a boss-room fixture (the
deliberately open full-width doorRow must NOT produce gaps in the envelope).

## W2 — Render runs + category filter (placeholder geometry)

Replace boundary-wall rendering in the encounter map with a **positive
category rule** — draw exactly these, and nothing else in the walls channel:

- (a) W1's runs, as straight placeholder geometry (simple extruded boxes);
- (b) interior pattern walls — degenerate (`From == To`) entries whose cell
  lies inside a region's hex set (crypt has these; reference-tomb doesn't);
- (c) door frames keyed off door entries (`Kind = DOOR_*`, the only non-nil
  `Id`s), oriented along the connector column axis — not off
  `doorPassageNeighbor`'s arbitrary first-neighbor pick.

Because the rule is positive, it renders correctly against both the
pre-#849 wire (degenerate connector flanks) and the post-bump wire
(boundary-edge flanks) without branching. Also in this slice: extend the
floor slab/skirting to the wall line (cover the half-hex scallops).

**Acceptance:** reference-tomb screenshot set — both door areas and all four
sides of each room; explicit invisible-wall check (every connector flanking
cell visually covered by its run); ci-check green. **This is the first
visible milestone: screenshots go to Kirk before W3 starts.**

## W3 — Synty modular pieces

- Inventory the POLYGON Dungeon wall modules already in the contract tree
  (`harness/models/synty/`) vs. those needing ingest from the source pack
  (`scripts/ingest-assets.sh` → rpg-game-assets flow). Asset gaps here are
  our own pipeline work, not blockers on someone else.
- Map runs to segment / corner / door-frame pieces; theme-appropriate
  material/tint via `Space.theme`; locked-door visual preserved.

**Acceptance:** side-by-side screenshots against the Synty promo bar,
matching camera angles on the door areas.

## W4 — Clip-check, tuning, Kirk walk

- Clip-check with the largest character/monster mesh standing in boundary
  cells on both flush and scalloped columns — no clipping; tune W1's offset
  dial outward if needed, never mask cells.
- Kirk walks the reference tomb. Findings triage: render bugs fix in PR-B;
  asset-quality gaps (mesh/material/glow) go to the M3 backlog.

## Review rigor (calibrated)

W1 carries the only real logic — unit tests plus one review pass. W2–W4 are
presentation: judged by the screenshot harness and Kirk's eyes against the
bar, light single-pass review, no confirm ceremony.

## Traps for the implementer (do not rediscover)

- Screenshot harness on game routes needs `waitUntil: 'domcontentloaded'` —
  the encounter stream holds networkidle open forever.
- Kill vite by port (`lsof -ti :3001`), never `pkill -f vite` (self-match).
- GLBs cache hard: hard-reload after any asset sync. `npm run assets:sync`
  pulls rpg-game-assets' default branch only and is `rsync --delete`.
- If wire fields look missing locally: stale installed protos —
  `rm -rf node_modules package-lock.json && npm install`.
- Always start a fresh encounter after an api image swap (Redis persists).

**Pointers:** design = `design.md` in this folder · wire-data facts = the
findings comment on toolkit#848 · hex parity math =
`tools/spatial/position.go:176-217` (toolkit).

— asset-pipeline agent, on behalf of KirkDiggler
