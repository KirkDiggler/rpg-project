# Old way vs new way: what the rebuild must reach, seen side by side

## Status: Comparison run 2026-07-09 — both paths driven live in browsers, same api server

Kirk's ask: seeing the two side by side reinforces the design and identifies gaps. Both
paths were driven the same day against the same running rpg-api (main, post-lobby):

- **Old way**: rpg-dnd5e-web @ `9b0e1d1` (last commit before GameView) on port 3003 —
  v1alpha1 lobby → `StartCombat` → `BattleMapPanel`. Alice + Bob.
- **New way**: rpg-dnd5e-web main (post `#443`) on port 3001 — LobbyService v1alpha1 →
  `StartEncounter` → `EncounterView`. Charli + Finn.

Screenshots: `old-lobby`, `old-combat-alice`, `old-combat-bob`, `new-encounter-charli`
(shared in session 2026-07-09; representative shots, not committed to the repo).

## What the old way delivers today (verified by driving it, not from memory)

- **Lobby**: 6-char join code; **dungeon settings** — theme (Crypt/Cave/Ruins),
  difficulty, length (3–10 rooms), host-only; party panel with ready states; character
  re-select dropdown ("Unready to change character"); **Check Equipment**; leave.
- **StartCombat lands in a real fight instantly**: TURN_BASED, Round 1, **initiative
  tracker** (Bob → monster → Alice), "YOUR TURN" banner, **a monster in the order**.
- **A real room**: walls, an obstacle, floor plane. **Both players visible to each
  other.** Per-player targeted views verified: Bob's browser highlights *his* hex and
  *his* movement range; Alice's doesn't show them — same room, each player's own view.
- **Full character HUD**: HP/AC, six ability modifiers, equipment slots (main:
  shortsword/greataxe, off: empty), features (Sneak Attack 1d6; Rage as a BONUS chip),
  movement 30/30 ft, action/bonus/reaction economy dots.
- **Action bar**: Attack / Dash / Dodge / Disengage / Help / Hide with tooltip
  descriptions, End Turn, inventory button, abandon-encounter.
- **Combat Log panel** with round tracking ("Combat Started — Round 1").

## What the new way delivers today

- **Lobby machinery that is architecturally better**: opaque join ref, snapshot-then-
  deltas stream, first-class presence (`is_connected`), host migration, idempotent
  join/rebind, server-enforced all-ready gate — verified 4-browser 2026-07-08/09.
- **Encounter**: FREE_ROAM, round 0, each player **alone on a single hex** — no room,
  no geometry, no monster, no economy/menu (correctly waiting for TURN_BASED), End
  turn disabled. Bare-bones by construction: `ActionMenu`/`EconomyBar` render only
  what the server pushes, and the server has nothing to push yet.

## Why this reinforces the design (not an indictment of it)

The old way's richness is real, but it is built on the architecture we're deleting:
action menus computed client-side from RPC response payloads, rules knowledge in the
web, 42 legacy markers, dead defeat detection. The new way's emptiness is the honest
surface of a server-authored model whose server content hasn't been built yet. The
comparison says: **the spine is right, the content now has to be earned back on it** —
and every missing piece maps onto a boarded leg.

## The gap list (old-has / new-needs), mapped to the board

| # | Old-way capability | New-way home | Status |
|---|--------------------|--------------|--------|
| 1 | Players see each other | `StartEncounter` seeds `SightRange` (visibility, not geometry — diagnosis corrected 2026-07-09, see rpg-api#632) | **rpg-api#632** — one-field fix in flight |
| 1b | Room with walls/obstacles | Encounter grows a real Space (toolkit-led: ADR-0034 consolidation, `environments.QuickRoom` → `spatial.Room`, room-aware perception). NOTE: `tools/spawn` room placement is a dead stub today (`getRoomFromSpatial` always errors) — budget for it | The Dungeon leg — folds into the multi-room trailblazer design |
| 2 | TURN_BASED entry, initiative, a monster | Combat entry + monster seeding on the new stack | The Dungeon leg (tools/spawn trailblazer — see 1b's stub caveat) + the FREE_ROAM-only ledger item on rpg-project#81 |
| 3 | Dungeon settings in lobby (theme/difficulty/length) | Dungeon config on the encounter stack | The Dungeon leg — config feeds the multi-room trailblazer, not a lobby-service concern |
| 4 | Full character HUD (mods, equipment, features) | Game-grade treatment on EncounterView (the gap map's "richer treatment" item) | Game Screen leg — new issue when slice opens |
| 5 | Combat Log panel | Render the v2 event stream (events already arrive) | Game Screen leg |
| 6 | Initiative tracker overlay | HexGrid supports it; wire from v2 `initiativeOrder` state | Game Screen leg |
| 7 | Per-player movement-range highlight | v2 map adapter (`TurnState`/movement data is server-pushed) | Game Screen leg |
| 8 | Equipment check + character re-select UI in lobby | LobbyFlow affordances (idempotent rejoin already supports rebind server-side) | Game Screen leg, small |

Ordering intuition from the journey: #1 (filed) unblocks "party sees itself" — the
Party Assembles bar. #2–#3 are The Dungeon's entry point. #4–#8 are Game Screen polish
that becomes meaningful once there's a fight to render.

## Verified-claims note

Kirk's recollection — "we had rooms online and all events being sent and targeted to
the players with their own view of the room" — verified true in the run: the room came
from v1 `StartCombat`'s server-generated 20×20 hex room, and per-player views were
confirmed by comparing Alice's and Bob's simultaneous screenshots of the same fight.
