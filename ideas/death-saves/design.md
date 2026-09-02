# Death Saves on the Session Stack

**Status:** Approved design; implementation not started
**Approved:** 2026-09-02
**Milestone:** Four-player Level-3 Dungeon

## 1. Outcome

A player character at zero hit points remains in initiative and, on their turn,
receives one explicit **Death Save** declaration. Selecting it starts the same
shared physical d20 experience as Attack. The dying player throws the die in the
dungeon scene, every party member can witness it, an off-table throw is retried,
and no result is revealed until an in-bounds settlement.

The provider owns eligibility, the roll, thresholds, state transition and turn
continuation. The client renders the declaration and provider-authored result; it
never derives Death Save rules.

This replaces the archived automatic-condition design. Death Save is a unique
session verb, not an Activate ability and not a hidden TurnStart callback.

## 2. Design principles

### 2.1 The table rolls the die

Death Saves use the existing Attack dice contract:

- one accepted game command creates one authoritative d20 result;
- the local player physically picks up and throws the die;
- the validated throw plan is shared for witness playback;
- Story and semantic outcome stay concealed until settlement;
- an off-table attempt retries presentation against the same authoritative
  result;
- presentation retry never repeats the game mutation.

The physics make the game feel like a tabletop. They do not create a second
source of rules truth.

### 2.2 Explicit verbs over hidden triggers

`VerbDeathSave` says exactly what the player is doing. It appears through
`Afford`, carries an opaque selector, and has a dedicated executor.

The legacy `UnconsciousCondition.onTurnStart` auto-roll path retires. TurnStart
may still expire or refresh conditions, but it does not secretly perform a
player's Death Save.

### 2.3 Derived life state, not dual state

No new persisted life-state enum is added. The rulebook derives life state from
the existing authoritative fields:

- current HP;
- Death Save successes and failures;
- Stabilized;
- Dead;
- the monster rulebook's defeated/down answer.

A stored `life_state` beside those facts would be another value that could
contradict them.

### 2.4 Provider declarations keep the client dumb

The client learns that Death Save exists only from an available declaration. It
does not inspect HP, count pips, compare a threshold, decide whose turn it is,
or infer whether a roll should end the turn.

The provider also reports remaining successes/failures and continuation. The web
uses those facts for presentation only.

### 2.5 Policies evolve around stable facts

V1 ends the run when no party member remains conscious. The underlying state
still distinguishes Dying, Stabilized and Dead so a later campaign policy can
continue an all-unconscious scene for capture, rescue or final saves without a
character-data migration.

### 2.6 Turn-based command ingress

V1 accepts one authoritative mutating command at a time per encounter/character.
At a Death Save slot there is one active member and one lawful Death Save
command; the client also fences duplicate in-flight dispatch. Per-session
locking/CAS is outside this slice by ruling.

If a future host allows simultaneous writers into one encounter, serialization
or conflict detection earns its own design and applies to every write verb, not
a Death Save-only lock.

## 3. Life-state contract

### 3.1 States

| State | Derived from | Ordinary verbs | Death Save | Initiative | Attack target |
|---|---|---|---|---|---|
| Conscious | HP > 0 | Available normally | Absent | Retained | Yes |
| Dying | HP = 0, not Stabilized, not Dead | Blocked | Own active turn | Retained | Yes |
| Stabilized | HP = 0, Stabilized | Blocked | Absent | Retained; auto-pass | Yes |
| Dead | Dead | Blocked | Absent | Removed | No |
| Defeated monster | monster rule reports defeated | — | — | Removed | No |

“Downed” remains suitable presentation vocabulary for a body on the floor. It
is not sufficient scheduling vocabulary.

### 3.2 Rulebook ownership

The D&D rulebook derives this state. It owns HP, Death Save state, monster defeat
exceptions, and the meaning of each state.

The encounter composition receives a capability answer. It does not import the
rulebook, compare HP, know that three is a threshold, or distinguish character
from monster by inventing D&D policy.

Session implements only the lookup side of the capability: load the character or
monster record, ask the rulebook, return the answer.

### 3.3 Encounter behavior

The encounter evolves beyond binary `Standing` so it can apply separate
participation dimensions:

- may form/join a contact side;
- remains in a turn order;
- waits for player input;
- auto-passes;
- is removed from the turn order;
- counts as conscious for the current run-outcome policy.

Required behavior:

- Conscious characters participate normally.
- Dying characters stay seated and wait for their Death Save verb.
- Stabilized characters stay seated and auto-pass at their slot.
- Dead characters and defeated monsters are removed from turn order but remain
  in the roster and on the map.
- A healed character was never removed, so their next turn is normal without
  initiative reinsertion.
- A Stabilized character made Dying again keeps the same slot and resumes Death
  Saves there.

## 4. Pragmatic v1 defeat policy

The run ends in defeat immediately when no party member is Conscious.

This means an all-unconscious party does not continue rolling toward a result it
cannot act on. Death Saves remain meaningful while another conscious party
member can still fight, heal, protect or eventually use a future stabilization
verb.

The end predicate consumes derived life-state facts through an explicit policy
boundary. It must not be encoded as “zero HP means dungeon over” in the clock or
session. A future policy may instead keep the scene active while anyone is
Dying, distinguish Stabilized from Dead, or hand the party to capture/rescue
resolution.

## 5. `VerbDeathSave`

### 5.1 Declaration

On the turn clock, `Afford` emits one compiled Death Save declaration when and
only when all are true:

- the member is a player character;
- the member is active in their turn bubble;
- derived life state is Dying;
- this turn's Death Save capacity remains unspent;
- the character record and Death Save rule are readable.

Shape:

- verb: `DEATH_SAVE`;
- slot: `NONE`;
- target kind: `NONE`;
- provider-authored display name: `Death Save`;
- non-empty opaque declaration selector;
- no candidates;
- available under the complete current gate.

On a Dying turn, Attack, Move and Activate remain blocked. End Turn remains an
independent clock-valid escape, including after an accepted save whose automatic
continuation was interrupted.

Healthy characters do not receive a permanently disabled Death Save row. The
verb is absent when the life-state question does not apply.

### 5.2 Once per turn

Death Save receives one banked capacity when an eligible Dying turn is readied.
The verb consumes that capacity without consuming Action, Bonus Action or
Reaction.

The spent capacity persists on the same character write as the Death Save
result. Therefore:

- the RPC cannot be safely repeated after an ambiguous response;
- reconnect cannot offer a second roll on the same turn;
- a presentation retry never reaches the verb;
- if automatic End Turn is lost, the only remaining control is End Turn.

Natural 20 does not refund the Death Save capacity. It restores one HP, making
the verb ineligible, while the ordinary action economy for the still-active turn
remains available.

### 5.3 Selector trust boundary

Execution reloads current state, regenerates only the Death Save offer, and
selects the echoed declaration ID before touching dice or persistence.

Empty, unknown, mismatched, spent, no-longer-Dying, and not-your-turn selectors
are refused before a die is rolled. An accepted command rolls exactly once.

A new verb extends the selector vocabulary deliberately. Selector versioning and
collision tests are updated rather than silently accepting a new verb under a
contract that declared the set closed.

## 6. Authoritative Death Save operation

### 6.1 Eligibility

The rulebook operation refuses unless the character is Dying. It refuses a nil
or absent host roller; no default randomness exists on this path.

Stabilized and Dead are terminal for rolling. Conscious is not eligible.

### 6.2 Roll table

The authoritative d20 produces:

- natural 1: two failures;
- 2–9: one failure;
- 10–19: one success;
- natural 20: restore one HP, clear Death Save progress, become Conscious.

After applying the roll:

- three successes: Stabilized;
- three failures: Dead.

The rulebook returns both stored totals and derived presentation facts:

- outcome classification;
- successes/failures added;
- running successes/failures;
- successes still needed to stabilize;
- failures remaining before death;
- Stabilized/Dead/Recovered;
- HP restored;
- turn continuation instruction.

The client never computes these values.

### 6.3 Dirty persistence

Every accepted roll changes either DeathSaveState or HP and marks the character
dirty. Natural 20 applies the reported HP restoration to the character before
snapshotting persistence data.

The operation returns a new data snapshot through resolution. No runtime sheet
or event bus crosses the session seam.

## 7. Damage and healing transitions

Damage at zero is part of Death Save correctness, not a separate UI feature. The character keeper's real `ApplyDamage` path applies it directly; it does not depend on a `DamageReceivedEvent` publication or a condition-owned second ledger:

- positive ordinary damage adds one failure;
- positive critical damage adds two;
- zero applied damage, including fully resisted or immune damage, adds nothing;
- a Stabilized character first loses stabilization, then takes the failure;
- reaching three failures becomes Dead;
- the state change is dirty and persisted.

Legacy persisted `UnconsciousCondition` data may remain readable during migration, but its TurnStart/damage Death Save handlers are inert. `Character.Data.DeathSaveState` is the sole authoritative progress.

A Dying character remains a legal Attack target. A Dead character and defeated
monster do not. Target selection remains a provider rule.

For a Dying or Stabilized character, healing above zero:

- applies the healed HP;
- clears successes, failures and Stabilized;
- derives Conscious;
- requires no initiative reinsertion because the character kept their seat.

Ordinary healing does not clear Dead. Dead characters remain out until the
between-session resurrection policy acts.

The first Death Save slice introduces no Medicine action, magic, or healing UI.
It integrates with lawful healing already present and leaves new healing verbs
to their own design.

## 8. Resolution boundary

A dedicated resolution entry owns the rules interaction:

1. receive character record and required roller;
2. clone the record at the boundary;
3. strictly load/attach the character on a transient resolution surface;
4. execute the authoritative Death Save operation;
5. snapshot the dirty character record;
6. tear down every registration;
7. return data plus typed result.

This follows existing data-in/data-out resolution entries. Session fetches
records; resolution builds truth and runs rules; the host never receives a
runtime character or bus.

The legacy `UnconsciousCondition` TurnStart auto-roll subscription is removed or
made structurally incapable of rolling. Persisted legacy condition data must not
create a second roll when attached.

## 9. Session execution and save ordering

The session Death Save verb:

1. opens the normal write scope;
2. verifies roster membership, character kind, turn clock and active member;
3. reloads the character once;
4. regenerates/selects the exact current declaration;
5. mints and validates one opaque presentation token through the host-supplied generator;
6. invokes resolution with the host roller;
7. persists the dirty character, including capacity and result;
8. records the typed result and opaque token in Story;
9. lets the encounter observe terminal life-state consequences;
10. commits world/story and event delivery;
11. returns result, continuation, recipient-local sequence, opaque token, save and delivery reports.

Character data is written before Story/world consequences, matching Attack's
reason: life-state capability reads must see the result that caused them.

If character persistence lands and Story/world commit fails, the returned
`SaveError` names the landed character write and failed world pieces. Retrying
the game verb is unsafe and is prevented by the spent capacity.

## 10. Turn continuation and settlement

The game result exists before physical presentation, as it does for Attack, but
semantic reveal and UI continuation wait for dice settlement.

The provider mints one opaque shared presentation token through a required host-supplied ID generator. That token is persisted in the Death Save Story detail and returned to every recipient. It contains no global Story sequence and is never parsed.

The presentation service's numeric `authority_seq` remains a separate recipient-local authority coordinate. The opaque token correlates actor and witnesses; no global record sequence crosses the session boundary.

The provider returns one continuation instruction:

- `END_TURN`: ordinary success/failure or Stabilized;
- `KEEP_TURN`: natural 20 recovery;
- `ALREADY_ADVANCED`: a terminal transition already removed the member or ended
  the run.

After an in-bounds settlement:

- `END_TURN` makes the web invoke the current End Turn declaration
  automatically;
- `KEEP_TURN` refreshes CharacterData, Turn and Afford, leaving the revived
  character active;
- `ALREADY_ADVANCED` refreshes authority and sends no second mutation.

A failed/off-table presentation reveals nothing and sends no continuation.
Presentation retry uses the same presentation ID plus a new attempt and the same
authoritative result.

If the web disappears after the game mutation but before automatic End Turn,
reconnect reads spent Death Save capacity and offers End Turn, not another save.

## 11. Stabilized and dead turns

A Stabilized character keeps the initiative slot. When that slot becomes active,
the provider auto-passes it without waiting for a client command. Boundary
announcement still occurs so turn-scoped effects observe lawful time.

A Dead character is removed from initiative when the terminal state is
observed. They remain:

- in the encounter roster;
- at their map position;
- visible as dead;
- addressable by Story and exit/carry-forward behavior.

There is no in-dungeon resurrection in this design. Resurrection happens
between dungeon sessions.

## 12. Public wire contract

### 12.1 Declaration and RPC

The session wire adds:

- `VERB_DEATH_SAVE`;
- `DeathSaveRequest { session, member, declaration_id }`;
- `DeathSaveResponse` mirroring the SDK result;
- the new continuation enum;
- selector and failure documentation.

The request names no die result, state, threshold or target. The player chooses
the verb; the provider chooses and applies the outcome.

### 12.2 Typed Story event

`EVENT_KIND_DEATH_SAVE_ROLLED` carries a typed body visible to the whole party:

- member;
- roll;
- outcome enum;
- successes/failures added;
- running successes/failures;
- successes needed;
- failures remaining;
- Stabilized, Dead and Recovered;
- HP restored;
- continuation;
- opaque correlation/presentation token containing no Story sequence.

The retired encounter wire's DeathSaveRolled message is a useful field census,
not a contract to import. The SessionService shape mirrors the new SDK.

### 12.3 Current-state projection

Reconnect must not reconstruct current Death Save state by replaying prose.
Public turn/participant projection carries derived life state and Death Save
progress while the character remains in initiative.

The owner-private character projection also carries the same provider-owned
Death Save progress beside HP. Shared fields remain whole-party facts because
the physical die and pips are table-visible by ruling.

Dead members no longer appear in turn participants; their roster/map state and
terminal Story event remain authoritative.

### 12.4 API boundary

rpg-api:

- binds member to the authenticated player;
- maps SDK fields and sentinels to proto/status values;
- supplies configured randomness;
- publishes no rules, thresholds or inferred continuation;
- reuses the session presentation service for shared physical dice.

## 13. Tabletop presentation

### 13.1 Roller

The active Dying player receives the local-world d20 pickup and throw control.
The die uses the same dungeon colliders, pre-simulation, accepted plan, witness
stream, terminal classification and retry behavior as Attack.

### 13.2 Witnesses

Every party member sees the validated throw. The roll and pips are not secret.
Witness Story stays gated until the authoritative throw settles, matching the
roller's reveal.

### 13.3 Narration

The web renders structured provider facts with table-facing copy, for example:

- **Death save! 2 successes — 1 to stabilize.**
- **Failure. 2 down — one more means death.**
- **Natural 1. Two failures.**
- **Natural 20! Back on your feet with 1 HP.**
- **Three successes — stabilized.**
- **Three failures — dead.**

Copy may evolve without changing the rules contract. Outcome enums and remaining
counts, not client threshold arithmetic, choose which copy applies.

### 13.4 Off-table retry

An off-table terminal is a presentation failure only:

- no result appears;
- no pips animate;
- Story remains concealed;
- initiative does not visibly continue;
- the player throws again;
- the same authoritative game result is retained;
- no second Death Save RPC is issued.

## 14. Error and recovery model

### Before rolling

Refuse without mutation when:

- request/member/declaration ID is missing;
- member is absent or not a character;
- member is not on the turn clock or not active;
- character is not Dying;
- Death Save capacity is spent;
- declaration is unknown, stale or mismatched;
- character/rule dependencies are unreadable;
- the host supplied no roller.

Expected state movement is a stale/failed-precondition response, not invalid
caller data.

### After rolling

The accepted command is never retried automatically. Transport ambiguity causes
a fresh CharacterData/Turn/Afford read.

Presentation failure retries presentation only. Automatic End Turn failure
invalidates authority and reconciles; it never repeats the save.

Persisted Story remains the catch-up source for the public result. Current-state
projections remain the source for progress and eligibility.

## 15. Pragmatic scope

### Included

- explicit Death Save verb;
- derived life states;
- Dying/Stabilized initiative retention;
- Stabilized auto-pass;
- Dead/defeated removal;
- authoritative roll and persistence;
- natural-20 HP restoration;
- damage-at-zero failures;
- existing-healing recovery integration;
- whole-party physical die and progress;
- no-conscious-party defeat;
- reconnect and partial-failure safety.

### Deferred

- Medicine/stabilize action;
- spells, magical healing and healing UI;
- in-dungeon resurrection;
- secret Death Saves;
- configurable player auto-skip controls;
- monster AI policy for finishing unconscious characters;
- capture, rescue or monsters-withdraw scenes after all players fall;
- campaign-configurable defeat policy;
- extracting `character` or the root D&D module;
- per-session locking/CAS or a general concurrent-writer model;
- unrelated legacy condition cleanup.

## 16. Delivery sequence

Implementation proceeds inside-out through released boundaries:

1. rulebook life-state derivation and authoritative Death Save/damage/healing
   transitions;
2. encounter participation capability, initiative retention/removal and v1
   defeat policy;
3. resolution data-in/data-out Death Save entry;
4. session declaration, verb, selector, persistence and typed Story event;
5. proto transcription;
6. thin API handler and acceptance path;
7. web declaration dispatch, shared dice flow, pips and narration;
8. real multiplayer/live-browser verification.

Exact repository issues and PR slicing belong to the implementation plan, not
this design. Each published consumer pins a released provider; no committed
local override crosses CI.

## 17. Required proof

### Rulebook

- every d20 band: 1, 2–9, 10–19, 20;
- threshold crossing from two successes/failures;
- natural 20 applies one HP and clears progress;
- ordinary/critical damage at zero;
- stabilization loss on damage;
- healing resets progress;
- every mutation marks dirty and round-trips;
- ineligible states roll nothing.

### Encounter

- Dying remains in the exact initiative slot;
- Stabilized remains seated and auto-passes;
- healing requires no reinsertion;
- Dying again resumes at the same slot;
- Dead and defeated monsters are removed;
- roster/map presence survives removal;
- no-conscious-party defeat fires from derived state;
- future policy can be substituted without changing stored character data.

### Session

- Afford emits one Death Save offer only for the active eligible Dying member;
- ordinary verbs remain blocked and End Turn remains independent;
- selector mutation/staleness refuses before dice;
- one accepted save consumes one capacity;
- reconnect cannot reroll;
- natural 20 exposes normal declarations on the same active turn;
- terminal outcomes project exact continuation;
- partial saves name landed and failed pieces;
- Story event and current-state projection agree.

### Wire/API

- field-for-field SDK/proto mapping;
- authenticated member binding;
- exact gRPC refusal codes;
- whole-party event delivery;
- owner/public progress projections disclose only the ruled facts;
- no host threshold or life-state inference.

### Web/presentation

- Death Save appears from a declaration, never HP inference;
- local player throws the d20;
- all party members witness the same accepted plan;
- off-table reveals nothing and retries the same result;
- in-bounds settle reveals once;
- success/failure pips and narration use provider fields;
- ordinary result automatically continues exactly once;
- natural 20 keeps the turn;
- lost continuation reconciles to End Turn without reroll;
- reconnect restores current pips, life state and Story.

### Live journey

With at least two player characters in a real local dungeon:

1. one character is reduced to zero while another remains conscious;
2. the Dying character remains in initiative;
3. on their turn the only ruled player action is Death Save;
4. the player throws the shared d20;
5. an off-table attempt reveals nothing and is retried;
6. an in-bounds result appears to the whole party with correct progress;
7. a nonterminal result advances initiative;
8. a natural 20 variant restores one HP and leaves the character active;
9. a three-success variant stabilizes and later auto-passes;
10. a three-failure variant removes the character from initiative but not the
    map/roster;
11. dropping the final conscious party member ends the run in defeat.

## 18. Durable decisions

1. Death Save is an explicit unique verb.
2. The player physically throws the shared d20.
3. Off-table retry retains the authoritative result.
4. Nothing semantic reveals before in-bounds settlement.
5. Ordinary results continue initiative automatically after reveal.
6. Natural 20 restores one HP and keeps the turn.
7. Dying and Stabilized retain their initiative slot.
8. Stabilized turns auto-pass.
9. Dead leaves initiative until the dungeon ends and is resurrected between
   sessions.
10. Death Save rolls/progress are visible to the whole party.
11. V1 ends the run when no conscious party member remains.
12. V1 includes no Medicine, magic, new healing UI or in-dungeon resurrection.
13. Life state is derived, not separately persisted.
14. No character-module extraction is assumed.
15. V1 command ingress is serialized by the turn-based host contract; this slice adds no lock/CAS subsystem.
16. Shared presentation identity is an opaque host-generated token; global Story sequence never crosses the session boundary.
17. Only positive applied damage advances Death Saves; zero/immune damage does not.
