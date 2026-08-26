# Effect Context — brainstorm

**Date:** 2026-08-26
**Status:** Brainstorm. Territory mapped, scope cut. The buildable slice is in [design.md](./design.md).
**Umbrella:** `ideas/session-combat/` — the sub-idea home Kirk created alongside `experience/`.
**Journey:** rpg-project#253 (Play the Dungeon on the Session Stack)

---

## What prompted this

The production combat experience shipped (protos#253 → toolkit#1250 → rpg-api#845 → web#822).
A character's features, conditions, and resources now reach the dock and render as badges.
They are **visible and inert**: `design.md` in `experience/` says so directly — *"Informational
presence is not an executable offer."*

Two questions followed. Are conditions actually hooked into the new combat system? Can we
activate character features? The answers were no and no, and chasing why led somewhere more
interesting than a wiring gap.

Kirk's framing set the direction:

> *"from my view this is a resolution need. this is where the bus lives. now that we have the
> machines I would like to think about how we would want it... this is one of those crossroads
> that will have major implications if we get it wrong."*

---

## The finding that reframed it: two doctrines, both in `main`

`resolution/doc.go` says game context belongs to resolution, populated on demand:

> *"No game context is installed. Effect predicates read world state through five separate
> installers in the gamectx package... **Resolution is where they will be populated, because it
> is the one place that holds both the world and the effects; the first predicate that needs one
> brings its installer with it.** Populating a registry nothing reads would be building the wrong
> thing convincingly."*

`events/events.go:122` says the opposite, and names gamectx as the thing being replaced:

> *"OwnerAware is an opt-in a ConditionBehavior implements when it needs a live view of its OWN
> character's sheet... **instead of reaching for a context-installed registry** (rpg-toolkit#1178:
> gamectx.GameContext is a registry the session stack never installs)."*

Neither is wrong. They answer different radii — *my own sheet* vs *everything else* — and nobody
reconciled them. #1178 was a retreat under fire (Protection was crashing every attack that
character made) that got written down as doctrine.

---

## Diagnosing gamectx honestly

The instinct to blame "ambient state" does not survive contact with the code. `gamectx.WithRoom`
is ambient and it is the healthiest thing in the picture:

- installed **every time**, `resolution/resolve.go:335`
- pinned **structurally** by `TestNoCodePathProducesARoomlessInteraction`
- read by four predicates — prone, sneak attack, protection, opportunity attack
- read **defensively** — `Room(ctx)` returns `(room, ok)` and prone treats `!ok` as *unknown*

Context is also the right idiom here: resolution *is* a request, every chain handler already takes
`ctx`, and the lifetime is exact — the ctx dies with the bus.

**The two real defects are narrower than "ambient":**

### Optional

| installer | installs (non-test) | readers |
|---|---|---|
| `gamectx.WithRoom` | **1** (`resolve.go:335`) | 4 — **works** |
| `gamectx.WithGameContext` (CharacterRegistry) | 0 | 4, all broken |
| `gamectx.WithCombatants` (CombatantRegistry) | 0 | 0 |
| `gamectx.WithCombatState` | 0 | 0 |
| `gamectx.WithReactionReadiness` | 0 | 0 |
| `combat.WithCombatantLookup` — *a sixth, in another package* | 0 | 0 |

`WithGameContext` and `NewGameContext` have **zero non-test call sites in the toolkit** (verified
2026-08-26 against `origin/main` @ `5b54360`). The toolkit says so itself at
`conditions/fighting_style_protection.go:150`: *"a live registry the session stack never installs."*

An optional ambient dependency fails **silently**. Three conditions read a registry nobody installs
and `return c, err` into a chain fold:

- `conditions/unarmored_defense.go:191`
- `conditions/martial_arts.go:144`, `:257`
- `conditions/unarmored_movement.go:128`

`Character.EffectiveAC` swallows that error (`character/character.go:1398-1405`, `if err == nil`),
so AC silently falls back to base. **This is why Kirk's barbarian fought at AC 11 instead of 14.**
Worse than a missing bonus: an errored fold drops *every* AC contributor, not just Unarmored
Defense.

### Not everything ambient is broken — the distinction that matters

*Added 2026-08-26 during implementation, after the first pass got this wrong.*

`ReactionReadiness` looked like a fifth dead registry — zero installs. It is not, and the difference
is the sharpest thing in this whole diagnosis.

It has two live production readers (`conditions/opportunity_attack.go:167`,
`conditions/shield_spell.go:186`), and its absent-value behaviour is deliberate:

> *"Not-ready is the safe default — reactions that haven't been explicitly readied must never fire
> prompts, preventing accidental spell-slot burns."* — `gamectx/reaction_readiness.go:37-41`

So Opportunity Attack and Shield are **not** firing today, and that is **correct** — no affordance
to ready a reaction exists anywhere in the stack, so no reaction should be spending itself.

- **Fail silently by accident** — absent means a rule that should fire doesn't. Nobody chose that,
  nothing says so, and three of the four make it worse by erroring into a fold that swallows it.
- **Fail closed by design** — absent means the safe answer, written down at the point of failure.

An unwired feature with a correct default is not the same defect as an ambient dependency that
silently disables a working rule. The lesson generalizes past gamectx: **the test for an ambient
dependency is not "is it installed?" — it is "does its absent value say what the author meant?"**

### Plural

Five installers in gamectx plus a sixth in `combat`, with its own context key and accessor and no
relation to the others. That is not an abstraction, it is a bag — the only thing the five share is
a delivery mechanism. "What can an effect read?" got answered piecemeal by whoever needed
something, and nobody ever named the noun.

`doc.go` warns against populating a registry nothing reads. The mirror mistake — **defining
registries nothing populates** — happened anyway, twice over, in two packages that do not know
about each other.

**Fix optional and plural and context is fine.** That is a much smaller move than replacing it,
and it keeps the decoupling.

---

## The reframe: two channels, one of them one-fifth built

- **The bus carries what *happens*.** Events, chains, folds, provenance. This is the part that
  works, and the strike breakdown (toolkit#1238/#1239) proved it carries meaning, not just numbers.
- **Context carries what *is true*.** Where everyone stands, who is on whose side, what is
  currently affecting whom.

Channel two contains geometry and nothing else. That is why every cross-participant predicate in
the codebase is either commented out, erroring, or faked.

---

## Three vocabularies — two named, one smuggled

**Ruled by Kirk, 2026-08-26:** *"I really like the breakdown of the chain, notification and
instruction. those are very clear to me and I think we'll work well. they are explicit and you
don't have to interpret what they do."*

| kind | means | handlers | returns |
|---|---|---|---|
| **Chain** | "I am about to do X — everyone contribute" | many, folded in stage order | the folded event |
| **Notification** | "X happened" | many observers | nothing; nobody acts |
| **Instruction** | "do X to yourself" | exactly one: the subject | nothing |

This came out of Kirk recalling a pattern: *"we had at one point a condition firing a request event
even requesting the character take damage... each character and maybe every monster would listen to
these request events and modify themselfs with them."*

That pattern is real, and its current status is the whole lesson:

**It works today for conditions.** `resolution/contest.go:121` publishes `ConditionAppliedEvent`;
`character.onConditionApplied` (`character/character.go:1145`) applies the condition to itself,
appends it, marks dirty. The wolf's prone bite runs on it end to end.

**It broke for damage**, and `resolution/strike.go:526` records exactly why:

> *"DamageReceivedTopic has FIVE subscribers across THREE meanings. One applies damage
> (monster.onDamageReceived calls TakeDamage — the instruction). Three are genuine rules that only
> observe: Undead Fortitude's survival save, Unconscious's death-save failures, and Rage's
> was-I-hit upkeep. One is the encounter layer, which captures the events and DRAINS them precisely
> to stop the damage landing twice."*
> *"The topic MEANS 'damage has landed' — a notification. **The subscriber that treats it as an
> instruction is the defect, not the topic.**"*

Measured, not theorized — a 4-damage bite took a wolf from 11 to 3 with the publish in place.

**So the lesson is not "requests are bad." It is that one topic carried both meanings and nobody
could tell which they had subscribed to.** Name the third vocabulary and the pattern is safe.

The one thing instructions cannot do is answer back — "apply 8 slashing" cannot report what landed
after resistance. That is a chain. The split is really *does the sender need the answer*, and both
machines already exist.

---

## Lifetimes — settled, and smaller than feared

The worry was that turn-scoped memory ("have I sneak attacked this turn?") needed a home outside
the effect. **Kirk ruled it does not, and the code already agrees:**

> *"a condition has data. have I sneak attacked this turn is somethign the state on that condition
> should manage like rage knows if i hot somethign or somethign hit me kind of thing."*

```go
// conditions/raging.go:38
type RagingCondition struct {
    TurnsActive       int
    WasHitThisTurn    bool
    DidAttackThisTurn bool
}
```

`SneakAttackCondition.UsedThisTurn bool \`json:"used_this_turn"\`` (`sneak_attack.go:35`) is
persisted deliberately, with a comment saying so.

**Consequence, and it is a big simplification: the read channel only ever answers questions about
*others* and *the world*, never about the past.** Each condition is its own memory.

### But the clock never ticks

Seven conditions subscribe to `TurnStartTopic` / `TurnEndTopic` — dodging, disengaging, raging,
reckless_attack, helped, sneak_attack, unconscious. **Nothing in session, encounter, or resolution
publishes either.** The only lifecycle topic published anywhere in the new stack is
`ConditionAppliedTopic`, at `contest.go:121`.

| lifetime | examples | how it ends today |
|---|---|---|
| one interaction | advantage from prone | recomputed each fold ✓ |
| one turn | dodging, reckless attack, sneak-attack-used, reaction spent | **never published** |
| one encounter | raging, initiative order | **never published** |
| until rest | second wind, action surge uses | **never published** |
| permanent | unarmored defense, martial arts | n/a ✓ |

The two that work are the two that need no clock. Rage never ending, Dodge being unimplementable,
sneak attack never resetting, Second Wind never recovering — one defect wearing five hats.

### Kirk's proposal: resolution fires the clock

> *"we do need a way for the resolution package to know the clock ticks and when a turn ends. i
> think resolution could fire those events."*

Agreed, and it is close to forced: **resolution is the only place a bus exists** (ADR-0038), so
anywhere else that wanted to publish `TurnEnd` would have to make one, which is the thing the ADR
forbids.

What makes it fit rather than stretch: **a turn boundary becomes an interaction.** The session
already has EndTurn as a verb. That verb calls `Resolve` with a turn-end machine, everyone attaches
per R3, the event publishes, each condition decides whether that was its turn, dirty sheets come
back, host persists. No new concepts — and it does not force the step vocabulary open, because
`doc.go` already has a step that acts on the bus and folds nothing (imposing a contest's
consequence).

Same shape then retires every other dead clock: encounter end, short rest, long rest. Five dead
topics, one pattern.

### The hole that blocks it

`resolution/resolve.go:496` — `if !ok || !ch.IsDirty() { continue }`. Only dirty participants come
back.

**Nothing a condition does marks its owner dirty.** Grepped for any dirty-marking path reachable
from a condition (`MarkDirty`, `SetDirty`, anything) across `character/`, `events/`, `conditions/`:
**there is none.** `sneak_attack.go:214` does `s.UsedThisTurn = true` and nothing else happens.

Today it survives by luck. The rogue who sneak attacks also paid an action, and paying economy sets
dirty (`character/action_economy.go:48`). The raging barbarian who was hit also took damage, and
damage sets dirty. State changes ride along with something else that happened to mark the sheet.

**A turn-end interaction has no such luck. Its entire purpose is condition state change, and
condition state change is exactly what does not mark dirty.** Every expiry would fire correctly and
then be silently discarded.

**So dirty is a prerequisite for the clock, not a sibling of it.**

---

## Sides

### Kind is one name doing two jobs

`encounter/turndriver.go` — a `TurnDriver` is handed a `MonsterView` and decides. Its view of
everyone else is `Seen []SeenMember`, and `SeenMember` carries:

```go
// Kind is whether they are a player or a monster.
Kind MemberKind    // "character" | "monster"
```

That is the whole world-model a monster brain has. A duergar looking at a hobgoblin sees
`Kind: monster` and has no way to know it should stab it.

- **Kind answers "who drives you"** — a player declares; a monster has a TurnDriver;
  `encounter/encounter.go:1247` filters on `KindMonster` to know whose turns to auto-run.
- **Allegiance answers "who are you against"** — a rules and AI question.

One field carrying both is the same defect class as `DamageReceivedTopic`: one name, two meanings.

Good news, verified: **nothing in session, encounter, or resolution assumes an attacker or target
is a character.** The Striker takes `attacker, target MemberID` and hands both to the same
resolution path. Monster-vs-monster is structurally fine already.

### The rules already need more than two sides

`conditions/sneak_attack.go:257`:

```go
// Check if any "character" type entity (ally) is near the target
```

**Entity type is the ally proxy.** And it is wrong even in a two-sided world, because RAW Sneak
Attack is *"another **enemy of the target** is within 5 feet of it."* The relation is measured
**from the target's point of view, not the attacker's.** With two factions those coincide; with
three they come apart.

`monstertraits/pack_tactics.go:128` is a **stub** — the entire mechanic is commented out with
*"TODO: In full implementation, check if ally is adjacent to target"* and a note that it would need
a perception/spatial service. Pack Tactics is a wolf trait, and the wolf already ships.

### Kirk's use case

> *"dungeon has two monster types in it. those monsters are hostile to each other and both are
> hostile to us"*

Payoff: with three factions, **the rogue gets Sneak Attack on the duergar because a hobgoblin is
standing next to it.** That is RAW, it is emergent, and neither the current `Kind`-proxy nor a
binary ally/enemy model can produce it. N sides is a *gameplay* feature, not robustness.

### Allegiance shifts — and that forces a relation table

> *"allegiances can change. maybe the hobgoblins are hostile to us because their artifact is
> missing. if a player returns it it could shift allegiance"*
> *"my party is caught pickpocketing somebody and what used to be a friend turns into a hostile"*

Under "faction ID, different means hostile," the only way to shift hobgoblins toward you is to move
them into your faction. That works once. Then make peace with the duergar too and
`H == P == D` — **the two monster factions become allies purely because they each made peace with
you separately.** They should still hate each other. And "neutral" is inexpressible entirely.

**A shift is pairwise; a faction ID is global.** Changing an ID changes a relation to everyone when
the quest changed a relation to one party.

So: a **directed** relation between factions. Directed rather than symmetric is justified by two
real cases — 5e charm is one-sided (the charmed treats the charmer as a friend and cannot attack
them; the charmer is under no such restriction), and provoked infighting starts one-sided (the
hobgoblin the duergar shot is angry; the duergar may not have noticed).

### Two layers

- **Standing** — the relation between two factions. World state, changed by quest events, survives
  across encounters within its scope.
- **Effective** — standing plus temporary overrides, computed per interaction. Charm lives here.
  Provoked-by-a-stray-arrow lives here.

Same shape as the condition split: durable state in one place, per-interaction computation on top.

### It cannot live on a sheet

If allegiance were a field on a monster, returning the artifact means rewriting every hobgoblin's
persisted sheet — and every hobgoblin spawned later has to be told. As a relation in run state it
is one write that applies to hobgoblins that do not exist yet. More fundamentally, the fact is
about a **pair**; it has no natural home on either creature.

### Scope ruling

**Kirk, 2026-08-26:** *"Right now. I think we're scope dungeon. someday we'll have a campaign that
lives outside of it and that will be where we keep some of this. for now I think the dungeon is a
dungeon and what happens in the dungeons stays in the dungeon."*

Verified: there is **no world-state or flag concept** anywhere in `encounter/` or `session/` today.
Allegiance is not the feature — it is the first consumer of a run-scoped world-state facility that
does not exist. "The artifact was returned," "the lever was pulled," "the captain is dead" are the
same shape. Worth knowing before building a bespoke allegiance store and then a second flag store a
month later.

---

## What we are deliberately NOT building

**Kirk, 2026-08-26:** *"I also want to be clear we are not here necessarily to design all of these
things. we just need to be aware of them. so when we design what we need we don't paint ourselves
into corners."*

Parked, with the corner each one implies:

| parked | the corner it must not close |
|---|---|
| Allegiance data + stance table | no rule may compare faction IDs itself — rules ask relational questions |
| Charm / provoked overrides | effective must be able to layer over standing without touching rules |
| Run-scoped world state / quest flags | stance should be able to sit *on* a general facility later |
| Campaign scope | the read channel answers "right now" and never knows where facts came from |
| The turn/rest clock | dirty must land first; a boundary must be able to become an interaction |
| Feature activation (Verb, self-target, `Character.ActivateAbility`) | separate slice; needs the execution contract |
| The 13 missing standard conditions | `IsHostile` must not be the only relational question the channel can grow |
| Reactions / suspension | the read channel must be a deterministic projection of input data (R4) |

---

## Parked questions

- **Default stance** for a faction with no authored relation — hostile (dungeon default, current
  tomb unchanged) or neutral (creatures ignore you until provoked, which is what makes *provoking*
  possible)? Game-feel call.
- **What ends a three-way fight?** "All hostiles to the party are down" and "one faction remains"
  are different rules, and the second lets the party walk away from a running fight. Lands on top
  of rpg-project#277's thin ending set.
- **Do factions fight unobserved?** A living dungeon means simulating combat nobody sees; scenery
  means it only animates when watched.
- **Can the party provoke infighting deliberately?** The fun version. Requires runtime-mutable
  stance, same machinery as charm.
- **Presentation is mandatory, not optional.** A hobgoblin that stops attacking reads as a bug
  unless stance reaches the wire and the client shows it. `Participant` carries `kind` and
  `standing` and nothing else.

---

## What the buildable slice is

**Kirk, 2026-08-26:** *"Right now. we need the functionality of the game context where conditions
can look up the state of the world and Mark things dirty when they're dirty."*

See [design.md](./design.md).
