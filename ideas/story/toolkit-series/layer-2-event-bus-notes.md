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

## Tone

The page should be honest about the example test that produced confidence before the architecture could scale.

Possible closing note from Kirk:

> It gave us false hope. But rebellions are built on hope.
