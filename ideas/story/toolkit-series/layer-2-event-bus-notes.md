# Layer 2 source notes: The Nervous System

**Status:** Raw notes moved out of the character-creation chapter. Not approved prose. Not published.

## Opening state

Layer 1 ends with a valid Barbarian and a Rage feature that has become a mess.

Rage must:

- spend a limited use;
- add outgoing damage;
- grant advantage to Strength checks and saves;
- reduce incoming physical damage;
- track whether the Barbarian attacked;
- track whether the Barbarian was hit;
- end after inactivity;
- end on other rule-driven boundaries;
- persist its state;
- remove its behavior cleanly.

The next question is not how to create the character. It is how ongoing rule behavior participates without combat switching on every named feature and condition.

## The boundary inherited from Layer 1

Layer 1 ends with the `ToData` / `LoadFromData` breakthrough for the known character shape and the question: `Well, Rage is part of that data, right?`

Layer 2 must distinguish:

- `ToData` / `LoadFromData` for a known aggregate shape such as Character;
- `ToJSON` / `LoadJSON` for polymorphic features and conditions inside it;
- the structured `Ref` that lets a rulebook peek at identity and route the remaining opaque blob to the loader that owns it;
- restoring state versus restoring participation.

A loaded Rage object can have its fields and methods again without combat knowing when to call any of them. That earns the event bus.

The public boundary should say `Coming soon: The Nervous System`, not imply the page is already linked or published.

## Refs, ToJSON, and the homebrew loader dream

The extensibility question arrived before the final loader API. On 2025-07-29 the Artificer exploration states the requirement directly: a module must bring data **and behavior**, and external content should feel as real as core content after it is loaded.

The concrete pieces then landed together:

- **2025-08-11 15:13:** the identifier system moved into Core as `Ref`, with `Module`, `Type`, and `Value` segments. Its source comment explicitly names external modules and `artificer`.
- **2025-08-11 23:18:** the first complete `ToJSON` / loader examples appeared in the feature documentation.
- **2025-08-12:** the simplified feature and condition contracts adopted refs and JSON persistence.
- **2025-08-16:** the first D&D feature `LoadJSON` implementation parsed the ref, selected Rage by `ref.Value`, and returned a concrete value satisfying `core.Action[FeatureInput]`.

The intended address looked like:

```text
homebrew:classes:artificer
│         │       │
module    family  implementation
```

The ref did not itself contain executable code or magically load an unlinked Go package. It carried enough identity for a host/module router to choose the owning package, for that package to choose the right loader, and for the loader to turn its own JSON into a concrete object satisfying the shared interface.

```text
opaque JSON
  ref: homebrew:classes:artificer
                 ↓ peek address
homebrew module loader
                 ↓ owns JSON shape
concrete Artificer
                 ↓
shared Class interface
```

That is the bridge from Layer 1 into Layer 2. `ToData` can restore a Character because its aggregate shape is known. `ToJSON` plus a ref lets each polymorphic feature or condition name the package that knows how to restore its state and behavior.

Possible law:

> The JSON carries an address, not an implementation.

The next unresolved question remains: once Rage has been reconstructed as a live object satisfying the interface, how does its behavior reach attacks, damage, turns, and rests without combat knowing its name?

## The stress-test reflex

A repeated pattern across the work says something personal without turning into self-description:

> I do not trust an abstraction because it handles the example that inspired it. I trust it more after giving it the next real thing I think might break it, and sometimes the next next thing. I keep going until the lines connecting them no longer fit in my head.

The threshold is not line count. It is when the relationship graph exceeds working memory. At that point the architecture needs to make connections explicit, local, ordered, or representable as data rather than asking one person to hold the whole graph mentally.

The sequence:

```text
smallest useful seam
        ↓
hardest real example available
        ↓
hidden assumption becomes visible
        ↓
seam survives or moves
```

Examples to use across the series:

- **Configuration-driven character choices exist.** Try the Monk, whose choices contain choices and category lookups.
- **A polymorphic loader exists.** Ask whether an external package could bring `homebrew:classes:artificer`, its data, and its behavior.
- **A condition can subscribe to game moments.** Try Bless: multiple grantees, a fresh d4 on each roll, one caster's concentration, damage causing a save, and one failed save removing every granted condition.
- **Rage can manage itself.** Let it end and remove its own subscriptions; the rewritten bus deadlocks and exposes the unsafe lock.
- **Resolution can drive an interaction.** Ask it to stop for a player who may answer in another process later; a synchronous call stack cannot represent that pause.

This should become a visual motif. Each architecture page can mark the new seam in D20 blue and the real case intended to break it in restrained rust.

Possible repeated phrasing:

> The loader existed. Naturally, I asked whether it could load a class the toolkit had never heard of.

> Conditions existed. Naturally, I chose Bless.

This keeps the page personal without inserting a résumé paragraph. The work itself demonstrates the reflex.

## Event-bus evolution

- Begin before `Topic.On(bus)` existed.
- State what was wanted from condition/event-bus decoupling before showing the API.
- Conditions, features, equipment, spells, traps, and terrain need to react independently.
- The bus should transport typed moments while knowing no D&D.
- Rage should own its state and lifecycle; combat should never ask `is this character raging?`.
- The self-removal deadlock exposes that handlers cannot run while the bus holds the registry lock.
- The first answer was deferred operations; the lasting implementation snapshots handlers under lock and runs ordinary functions after unlocking.
- Typed topics eventually produce the explicit connection: `damage := DamageChain.On(bus)`.

## Bless as the hopeful proof

Bless is the visual and emotional center of this chapter. Rage creates the need for independent behavior, Bless demonstrates why the idea felt magical, and Rage later stress-tests the rewritten bus.

The early test of the decoupling idea should use Bless as well as Rage.

Bless needs:

- a condition on each grantee;
- a fresh d4 when a qualifying roll happens;
- concentration owned by the caster;
- damage to the caster triggering a Constitution saving throw;
- a failed save breaking concentration;
- breaking concentration removing every Bless condition granted by that cast.

A test had this functioning. It felt like proof that the model could express complex relationships without hard-coded combat branches.

It also gave false hope. The version worked in the example but did not scale cleanly. Preserve both truths.

Possible line:

> The test passed, and for a while that felt like the same thing as the architecture working.

## Chains

Events answer who can react. They do not answer the order in which several modifiers should change one calculation.

The string/mutable-event era exposed:

- registration order becoming rule order;
- magic priority numbers;
- base values and bonuses losing provenance;
- multiple effects mutating shared data unpredictably.

Turn-based resolution supplies a natural boundary: gather every contribution first, then execute once.

The Chain pattern keeps decoupling and adds deterministic rulebook order:

```text
base → features → conditions → equipment → final
```

Conditions contribute named functions to stages. The publisher executes the completed chain. The result can explain what changed it and why.

## Resolution machines, later

The early `pipelines all the way down` vision was larger than what had actually shipped. The modern resolution machines eventually delivered the properties wanted then:

- typed steps;
- deterministic transitions;
- pause as data;
- persistence across an RPC/process boundary;
- different interactions using different step sequences;
- return values rather than bus capture as the caller contract.

Do not collapse that later resolution architecture into the first event-bus breakthrough. It may belong at the end of Layer 2 or as the bridge into the composable-encounter chapter.

## Later storage/lifecycle evidence

The modern session SDK is not part of Layer 1. Preserve it here as later evidence that the repository idea returned at the right door:

```text
rpg-api repository implementations
        ↓ injected
session.Manager
  every verb: load → act → save → return
```

`IsDirty()` emerged while `rpg-api` owned load/save timing and survives as a rulebook change signal. The session SDK later pulled hydration, dirty collection, and save timing closer to the action while leaving database implementations in the host.

Both movements removed complexity:

1. repositories left toolkit internals;
2. repository contracts returned at the SDK boundary.

## Closing boundary

The chapter ends before resolution machines solve suspension:

> The event bus could ask every rule in memory. It could not wait for a person who was not there yet.

That preserves pause, persistence, and resume for the later resolution-machine chapter.

## Tone

The page should be honest about the example test that produced confidence before the architecture could scale.

Possible closing note from Kirk:

> It gave us false hope. But rebellions are built on hope.
