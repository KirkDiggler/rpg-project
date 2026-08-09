# Monster-targeting slice-1 playtest — handoff

Everything a fresh runner needs to reproduce or finish the remaining beat.
Evidence lives beside this file in `/home/kirk/.claude/jobs/7ec60a75/tmp/gameplay-evidence/`.
Driver scripts live in `/home/kirk/.claude/jobs/7ec60a75/tmp/drive/`.

## What is already proven (do not redo)

| Claim | Evidence |
| --- | --- |
| `target_rationale` renders in-game as " — turns on the most wounded" | `22-bob-after-decisive.png`, `44-RATIONALE-log-line.png` |
| Field crosses the wire on `ActionResolved` | `46-stream-target-rationale.json` → `"targetRationale": "dnd5e:targeting:lowest-hp"` |
| Client decodes both refs; control renders no clause | `45-target-rationale-decoded.json` (`lowest-hp` + `closest`) |
| Wounded chosen over closer healthy, both non-adjacent | `41-before-decisive.json` (wounded 4/12 @ dist 5; healthy 11/11 @ dist 2) + `47-combat-log-full.txt` |
| Authored YAML reaches persistence | Redis `data_json.targeting == 2` on `monster-hall-0` only |
| Ally + monster friendly-fire opportunity attacks | `62-run3b-ally-OA-downs-carl.txt`, `52-run3-log-at-drop.txt` |

## Remaining beat — DONE (run 4, encounter `dea3b271-b03b-4a0e-9d00-c00250ab91e9`)

Two-candidate choice captured with both characters conscious and non-adjacent:

- BEFORE (`70-FINAL-BEFORE-decisive.json`): Eve 12/12 @ **dist 2**,
  Finn **8/12** @ **dist 6**, `monster-hall-0` at `(12,-9,-3)`.
- `monster-hall-0` moved `(12,-9,-3)` → `(7,-7,0)` — five hexes AWAY from the
  closer healthy Eve and TO the wounded Finn, then attacked him.
- AFTER (`71-FINAL-AFTER-decisive.json`): Finn dist 1, Eve dist 3.
- Log (`72-FINAL-combat-log.txt`), screenshot (`73-FINAL-RATIONALE-screenshot.png`),
  decoded refs (`74-FINAL-rationale-decoded.json`).
- Control `monster-hall-1` attacked the CLOSER Eve, no rationale clause.

The trick that made it work: wound the target deliberately at R0 in free roam
via the ally OA (stand adjacent, move the target away → ~4 dmg), so the wound
costs no skeleton exposure and no retreat is ever needed.

NOTE: `45-target-rationale-decoded.json` was overwritten by run 4 and now holds
the same content as `74-`. Run 2's wire proof is intact in
`46-stream-target-rationale.json`.

## Stack state (leave running)

- `rpg-api` container: image `rpg-api:local`, `sha256:2a070d3c9909…`, built from
  `/home/kirk/game-dev/rpg-api/.worktrees/feat-779`.
- Go pins (UNCOMMITTED, intentional): `rpg-toolkit/encounter
  v0.50.2-0.20260808234131-99c6a3a67964`, `rulebooks/dnd5e
  v0.71.1-0.20260808234131-99c6a3a67964`, `rpg-api-protos/gen/go
  v0.0.0-20260809000620-cb8ee2cb8ea6`. `go build ./...` passes.
- Web: vite on **:3007** (NOT :3001 — another job owns 3001; the `/connect`
  proxy is port-independent). Worktree
  `/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/feat-733-combat-log`.
- **Uncommitted and needed by the web PR**: protos bumped `#v0.1.120` →
  `#v0.1.121` in `package.json` AND `package-lock.json`. A clean reinstall alone
  does NOT pick up `target_rationale` — the dep is pinned to a git TAG, and
  `v0.1.121` is exactly generated-branch commit `cb8ee2c`.
- Browser daemon: `node drive/daemon.mjs` → headed Chrome, throwaway profile,
  CDP on `127.0.0.1:9333`. Short step-scripts attach via `chromium.connectOverCDP`
  so the session survives between calls. Restart it if tabs pile up — many live
  `StreamEncounter` tabs make the UI flaky.

## Which skeleton is which (verify every run)

The reference tomb hall has TWO skeletons. Confirm from Redis DataJSON, never
by position alone:

```bash
docker exec rpg-redis-dev redis-cli GET 'enc:v2:<id>' | python3 -c "
import sys,json,base64
d=json.loads(sys.stdin.read())
for mid,m in d['monsters'].items():
    dj=json.loads(base64.b64decode(m['data_json']))
    print(mid,(m['position']['Q'],m['position']['R'],m['position']['S']),'targeting=',dj.get('targeting'))"
```

- `monster-hall-0` — authored `[5,3]`, `targeting: 2` = **TargetLowestHP**. The subject.
- `monster-hall-1` — authored `[7,5]`, field **absent** → default closest. The control.
- Numbering: `0=Unspecified, 1=Closest, 2=LowestHP, 3=LowestAC`.

## Targeting semantics (read before interpreting anything)

From `rulebooks/dnd5e@…/monster/monster.go`:

- `selectStrategyTargetIndex` compares **absolute current HP** with a strict
  `<` over a list **sorted by distance** → equal HP falls through to the
  CLOSEST. A tie proves nothing; the HP gap must be real.
- Ranged attacks (`TypeRangedAttack`, skeletons carry shortbows) filter to
  **non-adjacent** enemies BEFORE applying the strategy. So keep BOTH
  characters non-adjacent or the choice is conflated.
- `moveTowardEnemy` walks toward the strategy-chosen target even out of another
  enemy's melee — movement direction is itself evidence.
- Unconscious (0 HP) characters drop out of perception, so the wounded target
  MUST stay conscious or the beat evaporates.

## Lobby flow that works

1. `http://localhost:3007/?playerId=<id>` — playerId is read from the URL each
   load (no localStorage), so tabs are independent.
2. Click the character card → `Play` → `Create lobby`. Join code is in
   `[data-testid="join-code-display"]` as `join_<uuid>`.
3. Guest: card → `Play` → set the input `input[placeholder="paste a join code"]`
   via the native value setter + `input` event → click the button whose
   innerText is exactly `Join` (a loose "Join" match hits "or join a friend's").
4. Host: **`Choose a dungeon` FIRST**, then `The Tomb of the Captain`, then
   verify `[data-testid="dungeon-picker"]` reads it. Skipping the open step
   silently starts a DIFFERENT dungeon (monsters named `entrance-entrance`).
5. Both `Ready up`, host clicks `[data-testid="start-encounter-button"]`.

**Player ids are single-use.** `Leave` only navigates the client home; the
server keeps resuming that player into the encounter forever, so every run
needs brand-new ids. Check who is free:
`drive/lobby-scan.py` (lists players tied to an active lobby).

## Fiber walking (three.js is not reachable by a11y clicks)

Components are `React.memo`-wrapped, so the name can hide one level down —
this is the finder that works (`drive/lib.mjs`):

```js
function fiberName(node) {
  const t = node.type;
  return t?.name || t?.displayName || t?.type?.name || t?.type?.displayName
      || t?.render?.name || node.elementType?.name || null;
}
// walk from #root's __reactContainer* key, pushing child/sibling and
// node.stateNode?.current
```

Useful props:
- `EncounterMap.memoizedProps`: `myEntityId`, `isMyTurn`, `activeEntityId`,
  `round`, `movementRemaining`, `openDoorIds`, `initiativeOrder`, `onDoorClick`.
- `HexGrid.memoizedProps`: `entities`, `floorTiles` (Map keyed `"x,y,z"`),
  `walls`, `onMoveComplete(path)`, `onEntityClick(id)`, `onDoorClick(id)`.
- `EncounterDock.memoizedProps`: `onEndTurn()`, `endTurnDisabled`.
- `CombatLog.memoizedProps.entries[]` carries the decoded `targetRationale`.

Door: `reference-tomb-door-entrance-hall`, edge `(6,-7,1)`↔`(7,-8,1)`.
Wall kinds: `1=solid, 2=closed door, 3=open door, 5=locked`.

**Move with BFS, not a straight line.** `getHexLine` crosses non-floor hexes and
the server silently truncates the move to zero. `drive/goto.mjs` BFSes over
known floor honoring wall edges and occupied hexes. Note each player has their
own fog, so a hex one player knows may be unknown to the other.

## Screenshots / stream

- Always `waitUntil: 'domcontentloaded'` — `StreamEncounter` never goes idle.
- To capture raw stream events, hook `console.log` IN-PAGE with a BigInt-safe
  replacer (`sequence` is a BigInt, which makes Playwright's `jsonValue()` bail).
  See `drive/hook-capture.mjs`.
- Ground truth: `drive/snapshot.py <encounterId> <label>` writes
  positions + HP + per-monster distances to a labelled JSON.

## Traps that cost real time

1. **Ally opportunity attacks.** Party members OA each other, and it downed the
   wounded character in all three runs. `Disengage` does NOT prevent it. A
   retreat from melee costs TWO OAs (skeleton + ally) — a wounded character
   usually cannot pay that. **Never let the two characters stand adjacent.**
   (This can be used deliberately: to wound a character on purpose, stand them
   adjacent in free roam and move the intended target away — the OA fires at R0,
   before combat.)
2. Monsters also OA each other (`monster-hall-0` → `monster-hall-1`).
3. Client HP display can disagree with Redis (dock showed 10/10 while Redis said
   0). Trust Redis.
4. A player's client may miss the other party member until the first knowledge
   update; it self-heals on any state change.
5. Chargen is flaky in exactly three places: the martial-weapon `-- Select item --`
   is a CUSTOM dropdown (not `<select>`; options are nested divs — match on the
   first text line); the Defense fighting style hides under a collapsed
   `Additional Choices ▶`; the background step needs the **`Choose Background`
   section card**, not the stepper button, then pick e.g. `⚔️\nSoldier` and
   `Select Background`. Working chain:
   `chargen.mjs` → `chargen-weapon.mjs` → `chargen-rest.mjs` → `bg-click.mjs`
   → act(`⚔️\nSoldier`, `Select Background`, `⚔️ Begin Adventure!`).
   Result: level-1 Human Fighter, 12/12 HP, AC 12.

## Recipe for the remaining beat

1. Create two fresh characters (identical Fighters = current HP is the only variable).
2. Fresh lobby, tomb, start. Verify skeleton identity from DataJSON.
3. **In the entrance, before combat**: stand them adjacent, move the intended
   target one hex away → ally OA wounds the target (~4 dmg → ~8/12). Repeat if it
   misses. This gets a real wound with zero skeleton pressure.
4. Move both into the hall keeping them NON-adjacent to each other and to the
   skeleton: healthy strictly closer (dist ~2), wounded farther (dist ~5).
5. `snapshot.py … BEFORE`, end turns so `monster-hall-0` acts, then
   `snapshot.py … AFTER` + screenshot with the log line
   (`drive/logshot.mjs <player> <name>` scrolls to and highlights it).
6. Expect `monster-hall-0` → wounded (farther), `monster-hall-1` → healthy (closer).
