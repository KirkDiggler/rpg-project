# The combat turn on the session stack

**Status:** DRAFT for Kirk's ruling, 2026-08-22.
**Journey:** rpg-project#91 (friends playtest → Dungeon Night).
**Measure:** web#533's success bar — *a friend who has never seen the game takes a complete, correct turn without verbal instructions* — on the 3D session route, against the reference tomb's skeleton.
**Design already ruled, reused not re-derived:** web#525 (panel hierarchy: "what can I do NOW", End Turn reads as consequential), web#533 (turn orientation, armed-action guidance, end-of-turn clarity), web#564 (names, not ids), rpg-project#94 / protos#187 / api#680 / web#571 (equipment is on the wire and has a screen), ADR-0042 (Afford answers in declarations), ADR-0043 (a monster's turn has a driver), toolkit#1169 (Move spends movement on the turn clock — in flight, #1171).

This document is the contract for ONE feature across four repos. The proto section is what merges first; everything else builds against it in parallel (memory: how-we-build).

## 1. The turn, as the player lives it

1. **Free roam.** You walk the tomb. The panel is a quiet pill: *Free roam*. Your equipped weapon shows in the equipment screen (already exists), reachable from the route.
2. **Contact.** The skeleton sights you → the fight forms. The panel becomes the combat panel: *Round 1*, the initiative order **by name** with the active one marked, your three shapes (action / bonus / reaction) and *Movement: 30 ft*, and one beat line: *"A fight begins: Aldric, skeleton-1."*
3. **Your turn.** An unmistakable "your turn" moment (#533). The floor is a walk surface bounded to the server's feet; a longer hover path is red. Enemies **in reach** are highlighted; hovering one shows *Attack skeleton-1*. Clicking it swings (longsword, or a punch if your hands are empty — the rule, not a refusal). The beat line narrates from typed facts: *"You hit skeleton-1 — 17 vs AC 13, 6 slashing."* The action shape dims; a second hover says *action: 1 needed, 0 left*. End Turn reads as the consequential button; it's the only thing left once you've spent.
4. **The monster's turn.** *"skeleton-1's turn."* is a visible beat, then *"skeleton-1 does nothing"* (the Pass driver today; real behavior later through the same seam). The clock comes back: *Round 2, your turn*, movement refreshed.
5. **The end.** The skeleton drops: *"skeleton-1 is downed. The fight is over."* The panel returns to *Free roam*. You walk on.

Everything above is rendered from the wire. The client computes no rule: not reach, not cost, not who is up.

## 2. Contract: panel element → wire → SDK today → work

| Panel element | Wire (session v1alpha1) | SDK today | Work |
|---|---|---|---|
| Initiative order by name, active marked, up/downed | `TurnResponse.participants[] {member, name, kind, standing, active}` | `Turn.Order` is ids only; names only on Join/Spawn outputs; standing on no read (#1137) | toolkit: roster read carries name+standing; api: project |
| Enemy names on the map / beat line | `Sighting.name` | `Sighting.Subject` id only | toolkit: name on Sighting; api: project |
| Who is up/downed at a glance (cold client) | `Seen.standing` (sight-channel knowledge, ADR-0041) + `participants[].standing` | composition knows (`Standing` capability, `noticeDown`); not projected | toolkit: project standing; api |
| Movement: N ft; floor bounded to it | `Declaration{verb: MOVE, remaining}` (protos v0.1.131 ✓) | #1171 (in flight) | toolkit merge; api/web |
| Enemies in reach; Attack per target | `Declaration{verb: ATTACK, target, slot, affordable, shortfall}` — one per candidate target **in reach**; none in reach → one declaration `affordable:false, shortfall:"no target in reach"` | no reach anywhere (#1010); legacy `encounter/range_gate.go` has `checkReach` to port | toolkit: reach in session.Attack + Afford per target; api; web |
| Punch when empty-handed | no change | `ErrBadAttack` "nothing equipped" (#1168) | toolkit: compile to `unarmed-strike` |
| Beat line: "You hit X — 17 vs AC 13, 6 slashing" | `AttackResponse.attack {ref, name, damage_type}` | output has numbers only; weapon identity dropped at the seam (#866) | toolkit: carry `AttackProfile.Ref` + damage type; api |
| Beat line from the stream (others' swings, downed, monster's turn, fight start/end) | **typed event bodies**: `Event.body` oneof `{turn_ended{member}, downed{member}, struck{attacker,target,roll,total,against,damage,damage_type,attack}, missed{attacker,target,roll,total,against}, fight_started{members[]}, fight_ended{cause}}` | all in opaque JSON `payload` (session `kindOf` is "TEMPORARY SHAPE pending toolkit#941") | toolkit: typed beats at the seam (#941); protos; api; web stops ignoring the stream |
| "skeleton-1's turn" as a moment | `turn_ended{member}` per driven member (already one beat each) | beats exist, unnamed on the wire | falls out of typed events; pacing is presentation (web#561) |
| Fight over, why | `fight_ended{cause: DEFEAT\|DECISION}` + `Turn` flips to WORLD | cause only in payload | falls out of typed events |
| Equipped weapon / equip | character v1alpha2 `CharacterData.equipment`, `EquipItem` (#187/#680) + the equipment screen (#571) | done on the old route | web: mount the screen on the session route; **no auto-equip** (Kirk) |
| Fight-lock message | gone: `Move` refuses `not your turn` / `movement: N ft needed, M ft left` (protos v0.1.131 ✓) | #1171 | web: replace fightLocked with the two refusals |

## 3. The proto, whole (dnd5e.api.session.v1alpha1, additive)

```proto
// types.proto
enum Standing { STANDING_UNSPECIFIED = 0; STANDING_UP = 1; STANDING_DOWNED = 2; }

message Participant {           // one member of the fight the asker is in
  string member = 1;
  string name = 2;              // display name; never empty for a member the server can name
  MemberKind kind = 3;
  Standing standing = 4;
  bool active = 5;              // exactly one true on the turn clock
}

message Seen { Position position = 1; Standing standing = 2; }   // sight-channel knowledge (ADR-0041)
message Sighting { ... string name = N; }                          // beside subject

message Declaration {
  Verb verb = 1; Slot slot = 2; bool affordable = 3; string shortfall = 4;   // shortfall (string) kept for v0.1.131 readers; superseded by 7
  optional int32 remaining = 5;   // MOVE (v0.1.131)
  optional string target = 6;     // ATTACK: the candidate target this declaration prices; one declaration per target in reach
  Shortfall why = 7;              // present exactly when affordable == false
  // A monk's Martial Arts bonus strike is simply another declaration {ATTACK, slot: BONUS, target}; off-hand and flurry the same.
  // Unarmed strike is the catalog ref "unarmed-strike" with BLUDGEONING; the monk's die/DEX are rules inside the toolkit, never on the wire.
}

enum DamageType { DAMAGE_TYPE_UNSPECIFIED = 0; ACID = 1; BLUDGEONING = 2; COLD = 3; FIRE = 4; FORCE = 5; LIGHTNING = 6; NECROTIC = 7;
                  PIERCING = 8; POISON = 9; PSYCHIC = 10; RADIANT = 11; SLASHING = 12; THUNDER = 13; }   // closed set → enum, the UI branches on it (Kirk)
message AttackRef { string ref = 1; string name = 2; DamageType damage_type = 3; }  // ref is the open set the client already maps

enum ShortfallReason { SHORTFALL_REASON_UNSPECIFIED = 0; NO_BUDGET = 1; NOT_YOUR_TURN = 2; NO_TARGET_IN_REACH = 3; DOWNED = 4; UNREADABLE = 5; }
enum Currency { CURRENCY_UNSPECIFIED = 0; ACTION = 1; BONUS = 2; REACTION = 3; MOVEMENT = 4; }
message Shortfall {             // structured so the UI can act on it; text stays for narration
  ShortfallReason reason = 1; Currency currency = 2; int32 needed = 3; int32 left = 4; string text = 5;  // "action: 1 needed, 0 left"
}
```
```proto
// service.proto
message TurnResponse { ClockKind clock = 1; string active = 2; int32 round = 3; repeated string order = 4;
  repeated Participant participants = 5; }      // order[] stays; participants carries what order cannot
message AttackResponse { ... AttackRef attack = 10; }
```
```proto
// events.proto — typed bodies replace reading payload; payload stays for kinds not yet typed
message Event { ... bytes payload = 7;
  oneof body {
    TurnEnded turn_ended = 10; Downed downed = 11; Struck struck = 12; Missed missed = 13;
    FightStarted fight_started = 14; FightEnded fight_ended = 15; Moved moved = 16;
  } }
message TurnEnded   { string member = 1; string next = 2; }
message Downed      { string member = 1; }
message Struck      { string attacker = 1; string target = 2; int32 roll = 3; int32 total = 4; int32 against = 5; int32 damage = 6; AttackRef attack = 7; bool critical = 8; }
message Missed      { string attacker = 1; string target = 2; int32 roll = 3; int32 total = 4; int32 against = 5; AttackRef attack = 6; }
message FightStarted{ repeated string members = 1; }   // in initiative order
message FightEnded  { DissolveKind cause = 1; }
message Moved       { string member = 1; Position to = 2; }
```
Refusals (documented on the RPCs, no new shapes): Attack → FAILED_PRECONDITION `not your turn` / `no target in reach` / `action: 1 needed, 0 left`; Move → as v0.1.131.

Not in this proto (deliberately): monster behavior, ranged weapons and cover, reactions/opportunity attacks, death saves, a session-level equip verb (equipment stays in the character service).

## 4. What each repo builds (in parallel, behind the merged proto)

**rpg-toolkit** (one PR per module where the go.mod forces it; otherwise one):
- `session.Attack`: reach gate before pricing (port `checkReach` semantics: melee 1 hex, reach property 2; ranged weapons stay refused as today) → `ErrOutOfReach`; `Afford` prices Attack per target in reach (same gate), `Declaration.Target`. Closes #1010 for melee.
- Empty main hand compiles to `unarmed-strike` (#1168); `ErrBadAttack` stays for unreadable sheets and then Afford reports it as the shortfall.
- Roster read carries `Name` and `Standing` (`Turn` participants; `Sighting.Name`; `Seen.Standing`). Closes #1137.
- Typed beats at the seam: session projects `TurnEnded/Downed/Struck/Missed/FightStarted/FightEnded/Moved` as typed values instead of `kindOf`-over-JSON; `AttackOutput.Attack` (#866, #941).
- #1171 merges as is.

**rpg-api**: projections + status mappings (`ErrOutOfReach`, `ErrNotYourTurn` → FAILED_PRECONDITION); pins to the merged toolkit tags; mock regen.

**rpg-dnd5e-web** (grow PR #769 into the panel #525/#533 describe):
- Panel: round; participants by name with active + downed; shapes + movement; per-target Attack affordances drive the floor highlight (reach comes from the declarations, never computed); End Turn as the consequential commit; beat line from typed events only (no payload decoding, ever).
- Floor: your turn → walk within `remaining`; hover an in-reach target → Attack; not your turn → inert, "X's turn".
- Equipment screen mounted on the session route (reuse #571).
- Teaching moments per #533 (turn start, armed action, end-of-turn).

## 5. Process for this feature
1. This doc → Kirk rules. 2. Proto PR (whole §3) → merge → tag. 3. Three builds in parallel; rpg-api pins toolkit branches by pseudo-version; web against the branch api. 4. One walk on :3003 against #533's bar. 5. Merge bottom-up in one sitting. No ADR: nothing here is an open seam decision (typed event bodies are toolkit#941's accepted direction; reach/unarmed/names/standing are projections of rules the composition already holds).

## 6. Ruled (Kirk, 2026-08-22)
- **Closed sets are enums**: `DamageType`, `Shortfall.reason`/`currency`, `Standing`. Refs stay strings (the open set the client maps).
- **Per-target Attack declarations** — the monk's bonus strike, off-hand, flurry are further declarations of the same shape.
- **Typed event bodies now.** toolkit#941 is open and unstarted (session's `kindOf` still unmarshals the payload); this feature does #941 properly — beats record a declared kind and a typed body, session projects them. No stopgap: "we should not build anything to cover a gap that will be thrown away later."
- **The complete proto merges first**; a later addition is one additive field, not a PR chain.
