# Gameplay QA Checklist (Level 1)

Step-by-step test scripts for the 4-class multiplayer multi-room dungeon.
Each script tells you exactly what to do and what to expect.

---

## Test 1: Lobby & Character Creation

**Steps:**
1. Open the app
2. Create a new lobby
3. Create a character — pick any class
4. Verify: class-specific textures load on the character model
5. Verify: starting weapon is visible on the model (if applicable)
6. Join the dungeon

**Expected:**
- [ ] Lobby creates successfully
- [ ] Character creation completes without errors
- [ ] First room loads with hex grid, walls, and entities

---

## Test 2: Room Rendering

**Steps:**
1. After joining dungeon, look at the room
2. Pan/zoom the camera if possible

**Expected:**
- [ ] Hex grid renders correctly (no gaps, no overlapping tiles)
- [ ] Walls display around room edges
- [ ] Your character and monsters spawn at correct hex positions (not stacked, not off-grid)
- [ ] Door hex is visually distinct from floor tiles (brown/different color)
- [ ] Camera angle lets you see all entities
- [ ] Obstacles render if present (capsule shapes)

---

## Test 3: Turn & Initiative

**Steps:**
1. Combat starts automatically when you enter the room
2. Look at the turn order overlay at the top

**Expected:**
- [ ] Turn order overlay shows initiative order (your character + monsters)
- [ ] Clear indicator of whose turn it is
- [ ] On your turn: action economy shows 1 action (filled dot), 1 bonus action (filled dot), movement bar (full)

---

## Test 4: Basic Attack

**Do this with each class.**

**Steps:**
1. Wait for your turn
2. Click **Attack** in the abilities panel
3. Verify the action dot goes from filled to empty
4. A **Strike** button should appear
5. Click an enemy to target it
6. Click **Strike**

**Expected:**
- [ ] Attack consumes your action (dot goes empty)
- [ ] Strike button appears after Attack
- [ ] Combat log shows: attack roll (d20 + modifier), hit or miss
- [ ] If hit: damage roll shown with breakdown (dice + modifier)
- [ ] Monster HP updates after taking damage
- [ ] Cannot click Attack again (action already spent)

---

## Test 5: Movement

**Steps:**
1. On your turn, click a hex within movement range (don't use Attack first — movement is free)
2. Look for the path preview (highlighted hexes showing the route)
3. Confirm the move

**Expected:**
- [ ] Blue/highlighted border shows reachable hexes
- [ ] Clicking a hex shows a path preview
- [ ] Character moves along the path
- [ ] Movement bar decreases by the distance moved
- [ ] Cannot move to hexes beyond remaining movement
- [ ] Cannot walk through walls or other entities

---

## Test 6: Dash (Doubles Movement)

**Steps:**
1. On your turn, note your current movement remaining (e.g., 30ft)
2. Move about half your movement (e.g., 15ft) — movement bar should show ~15ft remaining
3. Click **Dash** in the abilities panel (this uses your action)
4. Your movement should now increase (roughly double what you started with minus what you already used)
5. Move again to verify you can go further than normal

**Expected:**
- [ ] Dash consumes your action (dot goes empty)
- [ ] Movement bar increases after Dash
- [ ] You can move further than a normal turn
- [ ] You can NOT attack this turn (action was spent on Dash)

---

## Test 7: Dodge

**Steps:**
1. On your turn, click **Dodge** in the abilities panel

**Expected:**
- [ ] Dodge consumes your action (dot goes empty)
- [ ] "Dodging" condition badge appears on your character
- [ ] You can still move and use bonus actions
- [ ] On the monster's turn: attacks against you should have disadvantage (two d20 rolls, takes lower) — **check combat log**
- [ ] Dodging badge clears at the start of your next turn

**Note:** Verifying disadvantage requires reading the combat log carefully. Look for two attack rolls where the monster uses the lower one.

---

## Test 8: Disengage

> **BLOCKED:** Disengage prevents opportunity attacks, but opportunity attacks (#505) are not yet implemented. You can verify the badge appears, but cannot verify the mechanical effect.

**Steps:**
1. On your turn, click **Disengage** in the abilities panel

**Expected (what you CAN verify):**
- [ ] Disengage consumes your action
- [ ] "Disengaging" condition badge appears on your character
- [ ] You can still move (walk away from enemies)
- [ ] Badge clears at the start of your next turn

**Cannot verify yet:**
- Opportunity attack does NOT trigger when moving away from enemy (needs AoO implementation #505)

---

## Test 9: End Turn

**Steps:**
1. On your turn, use your action (Attack, Dash, Dodge, or Disengage)
2. Click **End Turn**

**Expected:**
- [ ] Turn passes to the next entity in initiative order
- [ ] Temporary conditions clear (Dodging, Disengaging)
- [ ] Any temporary granted actions clear (Strike, Off-Hand Strike, etc.)
- [ ] On monster's turn: monster attacks and/or moves (watch combat log)
- [ ] Your next turn: action economy resets (1 action, 1 bonus action, full movement)

---

## Test 10: Kill a Monster

**Steps:**
1. Keep attacking a monster over multiple turns until its HP reaches 0

**Expected:**
- [ ] Monster HP decreases each time you hit
- [ ] When monster reaches 0 HP, it is removed from the board
- [ ] Combat continues with remaining monsters (if any)
- [ ] If all monsters dead, combat may end or door becomes available

---

## Test 11: Critical Hit

> This is luck-dependent — you need to roll a natural 20. You may need several attacks to see one.

**Expected (when it happens):**
- [ ] Combat log shows "Critical Hit" or "Natural 20"
- [ ] Damage dice are doubled (e.g., 1d8 becomes 2d8)
- [ ] Modifiers are NOT doubled (just the dice)

---

## Test 12: Door / Room Transition

**Steps:**
1. Clear all monsters in the room (or have door available)
2. On your turn, click the door hex
3. Watch the transition

**Expected:**
- [ ] Door shows hover text on mouseover (physical hint like "a wooden door")
- [ ] Door is only clickable on your turn
- [ ] Door shows loading state while processing
- [ ] Fade transition plays
- [ ] New room renders with new monsters
- [ ] Your character appears in the new room

**Known issues:**
- Old room disappears (no accumulated map yet — #310-313)
- You may spawn in the corner of the new room (room origin not applied — #312)
- Wall chunks may be visible (rendering artifact)

---

## Test 13: Cannot Act Out of Turn

**Steps:**
1. Wait for the monster's turn
2. Try to click abilities, hexes, or enemies

**Expected:**
- [ ] Abilities are grayed out / not clickable
- [ ] Cannot move
- [ ] Cannot target enemies for attacks
- [ ] Must wait for your turn

---

## Not Yet Implemented

These are known gaps — do not file bugs for these:

- Death saves / unconscious at 0 HP (#296)
- Opportunity attacks / reactions (#505)
- Help / Hide abilities
- Accumulated dungeon map across rooms (#310-313)
- Room origin / absolute positioning (#312)
- Short rest between rooms (#294)
- Multiplayer event broadcasting to all players (#369)
- Leveling / XP gain
