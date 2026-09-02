# Death Saves — Session-Stack Brainstorm

**Date:** 2026-09-02
**Status:** Design rulings captured; no implementation started
**Milestone:** Four-player Level-3 Dungeon

## Why this record exists

The original death-save idea was written against the old encounter stack. It
made `UnconsciousCondition` an automatic `TurnStartTopic` listener and assigned
zero-HP detection to rpg-api. Both choices predate the current session stack:
compiled declarations now tell a dumb client exactly which verbs exist, session
owns no rules, resolution owns each transient rules interaction, and the game
has a shared physical d20 presentation.

The legacy design and structured memory are preserved under `archive/`. This
brainstorm records the verified current state and Kirk's replacement rulings
before the new `design.md` states the contract.

## Correction made during the brainstorm

No active character-module extraction was found. The claim that one was in
progress was an incorrect inference from two unrelated facts:

1. Kirk said other work may eventually pull the current root D&D module out of
   the running system.
2. rpg-toolkit#946 is an unscheduled direction note proposing that `character`
   might someday gain its own version line.

Issue #946 has no Project item and has not been updated since 2026-08-13. This
design assumes no extraction. If module boundaries change later, the contracts
below move with their owners; they are not blocked on that work.

## Verified current-stack census

### The rule arithmetic exists

`rulebooks/dnd5e/saves/death_saves.go` already implements:

- natural 1: two failures;
- 2–9: one failure;
- 10–19: one success;
- natural 20: recovery result with one HP restored;
- three failures: Dead;
- three successes: Stabilized;
- damage at zero: one failure, or two on a critical hit.

`character.Data.DeathSaveState` persists successes, failures, stabilization and
death. `Character.MakeDeathSave`, `TakeDamageWhileUnconscious`, and
`GetDeathSaveState` exist.

The arithmetic is not the missing feature. Scheduling, authority, persistence,
projection and player interaction are.

### The current clock does the opposite of the intended behavior

`encounter.noticeDown` receives a binary Standing answer. Every reported member
is treated as a body:

- removed from contact sides;
- transferred from its turn bubble to the world clock;
- removed from initiative;
- used to decide defeat.

`session/death_test.go` explicitly pins that a downed member is absent from
`Turn.Order` and `Turn.Participants`. A downed player therefore cannot reach a
future turn on which a Death Save could be declared.

The session proto is already ahead of that implementation: its Downed event doc
says the member remains in initiative while Attack and Move are blocked. The
provider and wire documentation currently disagree.

### Binary Standing cannot express the required states

One Boolean answer currently conflates:

- a conscious character;
- a dying character who still needs a turn;
- a stabilized character who keeps a seat but needs no input;
- a dead character;
- a defeated monster.

Those states have different clock, targeting and outcome behavior. Adding a
Death Save verb without fixing this contract would create an unreachable
button.

### The old condition path is dormant and wrong for the desired experience

`UnconsciousCondition` can subscribe to TurnStart, damage and healing events,
but no production path applies it when a character reaches zero on the current
stack. Its TurnStart callback rolls automatically, which conflicts with the
new ruling that a player physically throws the shared d20.

The current boundary announcer still names automatic unconscious death saves as
a subscriber. If a condition were later applied without retiring that callback,
a client-driven verb and TurnStart would roll twice.

### The existing character operation is not yet authoritative

`Character.MakeDeathSave` currently:

- does not validate that the character is Dying;
- accepts default randomness through the lower saves package;
- updates the runtime state without marking the sheet dirty;
- reports natural-20 HP restoration but does not apply the HP;
- does not prevent a second save during the same turn.

The rule result can serialize, but resolution only returns dirty sheets. A clean
sheet whose DeathSaveState changed is a write that disappears.

### Afford is the right control surface

The current session stack already provides the needed trust boundary:

- `Afford` publishes exact server-compiled declarations;
- every executable offer has an opaque selector;
- execution regenerates and selects current state;
- the client renders declarations and never derives eligibility;
- `SlotNone` and `TargetNone` already express a no-economy, no-target verb;
- End Turn is independently clock-gated.

A dedicated Death Save declaration fits this model directly. It does not fit
Activate: Death Save is not a carried ability, Activate rejects a downed actor,
and Activate's thin acknowledgement cannot report the roll or terminal state.

### The physical die path already exists

Attack establishes the tabletop presentation contract:

- the game result is authoritative;
- the local player picks up and throws a d20 in the dungeon scene;
- the whole table can witness the validated throw plan;
- Story and semantic outcome remain release-gated;
- an off-table attempt retries presentation without mutating the game again;
- the retry retains the same authoritative result;
- only an in-bounds settled attempt reveals what happened.

Death Save must enter that same presentation path rather than add a second dice
experience.

### The old wire contains useful vocabulary, but the new session wire does not

The retired encounter stream already has a `DeathSaveRolled` shape containing
roll, running totals, critical flags, stabilized/dead, recovered and HP
restored. The current SessionService has no Death Save verb, RPC, event kind or
typed body, and owner CharacterData has no Death Save progress.

The old message is evidence for the facts the new contract needs, not a surface
to resurrect wholesale.

## Kirk's rulings — 2026-09-02

### Explicit verb

Death Save is unique enough to be its own session verb. It is not an Activate
variant and is not automatic TurnStart behavior.

### Tabletop roll

The dying player physically rolls the same shared local-world d20 used for
Attack. Everybody can see the die. Nothing semantic is revealed until an
in-bounds settlement.

An off-table throw retries the physical presentation against the same
authoritative result. It does not call the game verb again and cannot add
another success or failure.

### Turn continuation

After a settled ordinary success or failure, initiative should continue
automatically. The player should not need a ceremonial second click.

- ordinary success/failure: end the turn after reveal;
- third success: Stabilized, then end the turn;
- natural 20: restore one HP and keep the turn active so the character may act;
- third failure: Dead and out of initiative.

The provider authors the continuation instruction. The client does not infer it
from the d20.

### Initiative participation

A Dying character remains in initiative and receives a Death Save on their
turn. A Stabilized character also keeps the same initiative slot; future turns
auto-pass until they are healed or become Dying again. This avoids removing and
re-inserting a player at an invented initiative position.

Dead is dead for the rest of the dungeon. A Dead character leaves initiative,
remains on the map and roster for story, and is resurrected between sessions.

### Pragmatic party defeat

For v1, the dungeon run ends in defeat as soon as no conscious party member
remains. The system does not simulate an all-unconscious scene whose only
possible outcomes are stabilization, death, capture or rescue.

The distinct life states remain persisted. A later campaign policy may continue
that scene without changing character storage.

### Public table facts

Death Save rolls and progress are visible to the whole party. The table shares
the die and the running tension.

The UI may narrate the structured result with copy such as:

- “Death save! 2 successes — 1 to stabilize.”
- “Failure. 2 down — one more means death.”
- “Natural 20! Back on your feet with 1 HP.”
- “Three successes — stabilized.”

The provider supplies totals, remaining counts and outcome classification so the
client never learns the threshold arithmetic.

### Turn-based command ingress

V1 accepts one authoritative mutating command at a time per encounter/character. Death Save therefore has one possible command source at its active initiative slot, and the client fences a second in-flight dispatch. Per-session locking/CAS is not part of this slice. If the host later permits concurrent writers into one encounter, that concurrency model earns its own design rather than being smuggled into Death Saves.

### Focused initial scope

Build Death Saves only. No Medicine/stabilize action, no magic, no new healing
UI, no in-dungeon resurrection, no secret-save option, and no monster-AI policy
for finishing unconscious characters.

Existing healing and damage hooks remain compatible and the Death Save rules
still own damage-at-zero failures.

## Approaches considered

### A. Dedicated `VerbDeathSave` — chosen

A selector-bearing, SlotNone, TargetNone declaration appears only for the active
Dying character. A dedicated executor returns the typed result and persistence
report.

This is explicit, honest and compatible with the declaration trust boundary.

### B. Dynamic Activate ability — rejected

It would reuse some wire/client code, but would misclassify Death Save as a
carried ability, require bypassing Activate's downed gate, extend an executor
whose output cannot express the result, and obscure a unique turn rule behind a
generic ref.

### C. Automatic UnconsciousCondition roll — rejected

It is the legacy design. It removes player agency, bypasses the tabletop die,
uses an unreachable current turn path, and risks duplicate/default-random rolls.

## Key design insight

A Death Save verb is small only after life-state participation is correct. The
actual foundation is not “add one RPC”; it is replacing a binary body/not-body
answer with a derived state that can say:

- cannot take ordinary actions;
- still owns an initiative slot;
- needs player input this turn;
- may be targeted;
- may or may not count toward ending the run.

That richer answer is the seam that keeps future stabilization, capture and
resurrection policy additive rather than another rewrite.
