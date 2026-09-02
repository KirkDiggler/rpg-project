# Run Readiness and Activation Story Design

**Status:** Approved in design conversation on 2026-08-31

**Implementation decisions:** [`2026-08-31-run-readiness-and-activation-story/implementation.md`](2026-08-31-run-readiness-and-activation-story/implementation.md)

## Problem

Three gaps prevent a new dungeon run from feeling like a complete level-one D&D loop:

1. Character creation rejects choosing the same equipment option twice even when a requirement says to choose two. The character inventory already stores quantities and the toolkit can equip two copies of one item ID, but the owner projection drops quantity and the web hides an entire stack after one copy is equipped.
2. Dungeon launch uses a separate persisted-data arcade reset. It restores HP and character-owned pools such as Rage Charges, but it bypasses the attached character's real `LongRest` event path and therefore misses feature-owned resources such as Second Wind and does not prove condition cleanup.
3. A successful `Activate` persists its mechanical effects but records no session story beat. Neither Story nor Debug can render an event that the toolkit never authored.

These are separate behavior slices under one player outcome: a character can be created with truthful starting quantities, enter each dungeon after a normal long rest, and see both activations and their results in the session record.

## Design principles

- **Toolkit owns rules and facts.** Equipment-choice legality, quantity, rest behavior, condition lifetime, activation effects, and story facts originate in `rpg-toolkit`.
- **API stays transport and host orchestration.** `rpg-api` authenticates, validates request shape, resolves lobby/content IDs, calls the toolkit's ID-based session verbs, and persists API-owned envelopes. It does not load runtime D&D characters, create toolkit event buses, inspect weapon properties, special-case feature/condition refs, reset opaque rule data, or compose game outcomes.
- **Web renders and echoes.** `rpg-dnd5e-web` may count copies represented by server data for inventory presentation, resolve member names, and format typed event fields. It does not decide equipment legality, rest recovery, or activation effects.
- **Use existing host verbs.** Character draft selection, EquipItem, LobbyService StartEncounter, toolkit Session `Join`, SessionService Activate, Afford, and the session event stream remain the externally consumed mutation surfaces. Slice B adds an internal data-in/data-out LongRest operation to `resolution`; it adds no player-facing rest RPC or API-side lifecycle helper.
- **One durable meaning per representation.** Inventory quantity is the owned count of an item ref. Equipment slots are indexes into that stack. Session events are durable facts, not client acknowledgements.

## Considered approaches

### 1. Patch each symptom locally

Remove the duplicate guard in the web, mutate selected resource fields in `rpg-api`, and append a local log line after a successful Activate RPC.

Rejected because it would let malformed duplicate choices through to a toolkit that still refuses them, preserve two competing reset implementations, and show an activation only to the caller without durable catch-up or multiplayer delivery.

### 2. Extend the existing authoritative contracts

Allow repeated equipment selections and canonicalize them into quantity stacks, make a character's first-ever toolkit Session `Join` invoke the attached character's existing long-rest behavior before placement, and extend the existing session story spine with typed activation/effect facts.

Selected because it fixes each gap at its semantic owner while preserving thin consumers and existing RPCs.

### 3. Introduce item instances, a rest service, and a generalized effect journal

Give every physical item a unique instance ID, add a standalone rest API, and replace the session record with a generic effect journal.

Rejected for this slice. Item instances may be needed for individually mutable magic items later, but identical starting weapons have no independent lifecycle today. A new rest RPC is unnecessary for launch policy, and replacing the record is disproportionate to adding two event families.

## Slice A: quantity-aware starting equipment

### Authored choices and draft validation

An equipment category with `Choose: N` represents N picks, not N distinct IDs. Repeated equipment IDs are valid within that category when each occurrence is independently eligible.

For example, the Fighter option “Two martial weapons” may persist:

```text
longsword, longsword
```

The count remains exactly two. Eligibility is still checked for both positions. This relaxation applies only to equipment picks; skill, language, tool-proficiency, and other choice categories retain their own uniqueness rules.

The existing fixed bundles remain authoritative:

- Fighter: one light crossbow plus twenty bolts, or two handaxes.
- Barbarian: four javelins and the selected secondary equipment bundle.
- Monk: ten darts.
- Rogue and other classes retain their authored fixed quantities.

The character-creation web renders the declared number of slots and allows the same equipment option in more than one slot. It sends the repeated authoritative selection IDs in declared category order. It no longer disables sibling-held equipment options or treats a valid hydrated duplicate as corruption.

### Inventory canonicalization

A finalized inventory has one stack per equipment ID. Compilation folds matching IDs from fixed grants, fixed bundle items, and category selections by summing positive quantities while preserving the first occurrence's deterministic presentation order.

```go
InventoryItem{
    Equipment: handaxe,
    Quantity:  2,
}
```

All quantity-sensitive rule checks use the total owned count, not the number of inventory rows. Loading legacy data containing multiple rows with the same equipment ID remains safe: projections and ownership checks aggregate by ID, and the next legitimate full serialization may emit the canonical single-stack form.

Zero or negative authored quantities are invalid provider data. They are never interpreted as one by the toolkit or API.

### Equipping copies

`EquipmentSlots` continues to map slot key to item ID. The same item ID may occupy multiple compatible slots only when the character owns at least that many copies.

EquipItem counts how many other slots already reference the requested item and refuses an equip that would exceed owned quantity. A single-copy weapon therefore moves between hands; a two-copy handaxe stack may occupy both hands. Existing two-handed occupancy and compatibility rules remain unchanged.

No item-instance IDs are introduced. Identical mundane starting weapons have no independently mutable state that warrants separate identity.

The toolkit owner projection exposes a cloned `EquipmentSlots` map directly on `EquipmentView`. The old per-item singular `Slot` projection is removed: one stack can truthfully occupy both hands, so a field that can name only one slot is not merely inconvenient but lossy. `rpg-api` maps `CharacterData.equipped` from the authoritative projected slot map instead of reconstructing it by scanning items.

### Owner projection and wire contract

Toolkit `EquippedItemView` gains owned `Quantity`, and `EquipmentView` gains the authoritative `Equipped` slot map described above. The v1alpha2 encounter `Item` message gains:

```protobuf
int32 quantity = 7; // total copies owned; always positive
```

`rpg-api` copies the toolkit field without deriving or defaulting it. During a mixed-version rollout, the web may treat an absent wire value of zero as one for backward-compatible display only; updated producers must always send a positive value.

The web computes presentation-only carried count:

```text
carried = owned quantity - number of equipped slots referencing the item ref
```

A carried row remains clickable while `carried > 0` and displays `×N` for the remaining carried copies. After one handaxe from a stack of two is equipped, the carried list shows `Handaxe ×1` and targets the empty compatible hand. After both are equipped, no handaxe remains in the carried list, while both sockets resolve through the same inventory ref.

This subtraction is representation bookkeeping, not a legality decision. The server remains authoritative and may refuse any stale or illegal EquipItem request.

## Slice B: a real long rest before dungeon launch

### First admission policy

For the current single-dungeon game, the first time a character joins a toolkit session grants that character a normal 2014 long rest before projection and placement. The persisted encounter's `EverMembers` is the admission record: a character absent from it rests; a character already present in it does not. Reconnect does not call `Join`, and exit/rejoin remains in `EverMembers`, so neither path rests again. A genuinely new late join receives the same first-admission rest.

`rpg-api` remains unaware of this rule. Lobby StartEncounter continues to call the toolkit's existing `StartSession → Join → Spawn` sequence with IDs, authored world data, and positions. It removes the old `RestoreForLaunch` loop and does not import runtime character lifecycle or event-bus plumbing.

Inside the toolkit:

1. Session `Join` opens the persisted encounter and determines whether the member is absent from `EverMembers`.
2. It fetches that member's `character.Data` through `CharacterRepository`, as it already does.
3. On first admission, it passes the data to `resolution.LongRest`.
4. `resolution.LongRest` strictly loads the character, creates its transient interaction bus, attaches the sheet, invokes `Character.LongRest`, and returns the resulting `character.Data`. No runtime character or bus crosses the resolution/session seam.
5. Session immediately persists the rested character through `CharacterRepository` and records `character:<id>` as durable for this Join attempt.
6. Session projects and places the now-authoritative rested record through the existing Join path, then commits the encounter containing the join.

LongRest or character-save failure occurs before placement and writes no encounter mutation. Once the character save succeeds, that rest is intentionally durable even if projection, placement, stream-number preparation, or encounter persistence later fails: the rest is the valid between-runs transition, not a tentative side effect of successful placement. Every later error must carry a `SaveError`/`SaveReport` naming `character:<id>` (plus any later durable writes), so the host can distinguish a safe retry from partial progress. Retry is mechanically safe because LongRest is idempotent and `EverMembers` has not persisted the failed placement. This explicit early-save posture avoids a cross-verb character-overlay transaction that would otherwise spread through standing, cast, monster-turn, check, and concealment callbacks.

The existing parallel `RestoreForLaunch` arcade reset is retired so there is one recovery meaning. Long-term town play will replace this temporary first-admission policy with an explicit in-world rest action; no town/rest RPC is introduced in this slice.

### Normal long-rest effects

`Character.LongRest` remains the single rule owner and must persist all implemented normal-rest outcomes:

- Hit points become maximum. This also satisfies the launch requirement to top off HP.
- Death-save state clears.
- Persisted turn action economy clears by delegating to `Character.ExitCombat`; the next fight's normal `StartTurn` seeds fresh action, bonus action, reaction, movement, and capacity.
- Character-owned resources that reset on a short or long rest refill.
- Hit dice recover half their maximum, minimum one, without exceeding maximum.
- Feature-owned recoverable resources such as Second Wind and Action Surge hear the RestEvent and refill.
- Persisted spell slots reset their used count to zero.
- Conditions receive the RestEvent and apply their own rule-correct lifetime behavior.
- Deprecated or non-authoritative duplicate resource maps are not revived as a second mechanic.

First admission allows this long rest even when the prior run ended at zero HP. This is temporary run-admission policy for a game without permanent death; it is not a claim that an arbitrary dead character can choose a rest mid-encounter.

### Condition audit

Every condition routed by the shipped condition loader has an explicit long-rest expectation. Adding a new loader arm without extending this matrix fails the registry-wide test.

#### Retain passive capability

- Unarmored Defense
- Archery, Defense, Dueling, Great Weapon Fighting, Protection, and Two-Weapon Fighting styles
- Martial Arts
- Unarmored Movement
- Brutal Critical
- Improved Critical
- Sneak Attack
- Opportunity Attack

Retained conditions with mutable per-turn state reset that state. This includes Sneak Attack's used-this-turn meter and Opportunity Attack's reaction meter. They publish their normal state-changed fact so persistence sees the reset.

#### End on the rest

- Raging
- Reckless Attack
- Dodging
- Disengaging
- Hidden
- Helped
- Prone
- Unconscious
- Shield spell

Each temporary condition owns its RestTopic subscription and publishes the standard ConditionRemoved fact. The character's keeper removes it from the sheet. Character.LongRest does not maintain a second switch over condition refs.

The audit asserts behavior after a full `Load` → `Attach` → `LongRest` → `ToData` round trip, not merely that a callback was registered.

## Slice C: activation and result story facts

### Root cause and event order

Session Activate currently adopts and saves dirty sheets, then commits without calling the encounter record. The successful mechanical change therefore has no durable story fact.

After successful resolution and persistence, Activate records:

1. One activation beat.
2. Zero or more ordered result beats captured from that interaction.

Failed, refused, or stale activations record no beats. Recording follows Attack's existing ordering: persist the true sheets first, then record against that world, then commit delivery. A recording failure returns the established unrecorded-report error posture rather than acknowledging a silent action.

### Activation beat

The activation beat carries only identity and selection facts:

- actor member ID;
- ability ref and server-authored display name;
- target member ID when present.

Example Story line:

```text
Aldric uses Second Wind.
```

Example Debug fields:

```text
activated actor=Aldric ability.ref=dnd5e:features:second_wind ability.name="Second Wind"
```

### Result beats

Resolution captures typed effect facts from the interaction bus instead of inferring changes by diffing serialized sheets.

Supported result variants for the current activation catalog are:

- **Healing applied:** target, actual HP recovered after maximum-HP clamping, roll, modifier, requested healing, source ref and display name, HP before, and HP after.
- **Condition applied:** target, condition ref, and server-owned display name.
- **Condition removed:** target, condition ref, display name, and reason.
- **Capacity granted:** member and server-authored capacity description, such as additional movement.

The character keeper publishes a post-clamp healing-applied fact after it mutates HP. This distinguishes a 9-point Second Wind roll from the 3 HP actually recovered when the fighter was only 3 HP below maximum. Resolution captures every healing-applied fact produced during the scoped `Activate` interaction, so future activated healing features using the standard healing topic receive the same result path without API or web feature switches. Hit-die spending, LongRest recovery, natural-20 death-save healing, and other healing outside `Activate` do not create activation-result events in this slice.

Whenever an activation result carries a roll and modifier, both Story and Debug expose that arithmetic. Story presents it as readable detail; Debug preserves the complete typed values. The result keeps requested and applied healing distinct.

Each effect becomes its own result event in synchronous publish order. One activation may therefore produce more than one result line without forcing unrelated effects into one sparse message.

Examples:

```text
Aldric uses Second Wind.
Aldric recovers 2 HP.
Second Wind rolled 6 + 1 = 7; 2 applied (8 → 10 HP).

Aldric uses Rage.
Aldric begins Raging.
```

The corresponding condition Debug fact includes the canonical identity rather than requiring the client to infer it:

```text
condition_applied target=Aldric condition.ref=dnd5e:conditions:raging condition.name="Raging"
```

Debug renders the complete typed raw fields, including healing roll, modifier, requested amount, applied amount, source, and HP before/after. Story resolves member names and uses the server-supplied ability/condition names and numeric facts; it does not identify a feature ref and invent its rules.

### Session, proto, API, and web shape

The session SDK gains `EventActivated` and `EventActivationResult` with typed bodies. The encounter record owns audience selection using existing perception/intelligence state rather than broadcasting results to the whole party. The actor and members who can perceive the affected member receive the applicable beat; a party member in another room does not learn that the actor began Raging merely because they share a party. Audience is fixed when the encounter records the durable fact, so catch-up and live subscribers receive the same visible event set through the existing story conversion and broker. The API and web do not recalculate visibility.

The session proto gains matching enum values and oneof bodies. ActivationResult carries one typed effect variant, not an unstructured payload or client-readable JSON.

`rpg-api` adds direct enum/body conversion arms and no feature switch. SessionService Activate remains an acknowledgement; clients continue to refresh declarations from Afford and learn story facts from the event stream.

The web adds generic Story and Debug formatting branches. It does not append optimistic activation lines after the RPC, preventing duplicate caller-only narration and preserving multiplayer/catch-up behavior.

## Error handling and consistency

- Invalid repeated equipment choices fail at toolkit validation before draft persistence.
- Invalid or nonpositive inventory quantities fail strict loading/provider validation rather than being silently corrected by API.
- EquipItem refuses ownership overdraw even if a client displays stale carried count.
- A failed first-admission LongRest or character save writes no Join encounter mutation. No successful Join seats an unrested sheet.
- A RestEvent subscriber error fails `resolution.LongRest` and therefore Join; it is not logged and ignored.
- After the character save succeeds, any projection, placement, commit-preparation, or encounter-save failure reports the rested character as already written. The rested record is safe to reuse on retry because LongRest is idempotent and the failed placement did not persist `EverMembers`.
- Activation effect capture is interaction-scoped. Subscriptions are removed with the resolution bus and cannot leak into later actions.
- Only healing produced inside that activation scope becomes a healing activation result; unrelated healing remains on its owning flow.
- Activation events are authored only after successful resolution and sheet persistence.
- Encounter-authored perception determines each beat's audience once; live and catch-up never diverge and clients never broaden the audience.
- Unknown future event kinds retain the existing delivered-as-unknown behavior; known activation bodies are never encoded into opaque payload as a shortcut.

## Testing strategy

### Toolkit

- Red/green tests allow two identical eligible category selections and still reject wrong count or ineligible items.
- Finalization tests cover two selected martial weapons becoming one quantity-two stack and fixed javelin/dart/handaxe quantities remaining exact.
- Equip tests cover one-copy movement, two-copy dual equip, and overdraw refusal.
- Equipment projection tests cover positive quantity, one row per item ID, and the same quantity-two item ref present in both entries of the projected slot map.
- Root LongRest tests use persisted Fighter and Barbarian sheets with spent Second Wind, Rage Charges, hit dice, spell slots, HP, death saves, and stale spent action economy. They prove LongRest exits combat while ShortRest does not and the next StartTurn seeds fresh slots.
- A registry-completeness test requires every loadable condition to declare and prove retain/reset/end behavior through a real attached round trip.
- Resolution tests prove strict data-in/data-out LongRest owns the transient bus and returns the complete rested sheet without exposing runtime objects.
- Session tests prove only first-ever Join invokes LongRest, persists the returned character before any placement callback can consult standing/cast state, reports that durable rest on every later failure path, and leaves reconnect/exit-rejoin semantics unchanged.
- Activation tests prove activation-before-result event ordering, exact post-clamp healing and visible roll arithmetic, condition effects with canonical refs/display names, capacity effects, no events on refusal, catch-up/live parity, perception-scoped audience (including a non-observer in another room), and persistence-before-recording failure behavior.

### Protos

- Generated Go and TypeScript bindings include `Item.quantity`, activation kinds, typed bodies, and result variants.
- Descriptor/compatibility checks confirm only additive field and enum/oneof changes.

### API

- Character projection acceptance proves quantity crosses field-for-field.
- Launch acceptance starts from persisted spent state and proves the real long-rest outcomes remain persisted after seating.
- SessionService acceptance proves Second Wind emits both activated and actual-healing result events through GetStory and StreamEvents.
- Converter tests cover every new event arm and reject no valid toolkit variant.

### Web

- Character creation permits and rehydrates identical equipment picks.
- InventoryLight shows remaining carried quantity and allows the second copy to target the empty hand.
- Both equipped sockets render the same item ref when quantity permits.
- Story renders activation and result as separate ordered entries.
- Debug renders every typed raw activation/result field.
- Existing unknown-event fallback remains intact.

### Live route

Using a branch-built isolated API lab and unchanged generic game route:

1. Create a Fighter choosing the same martial weapon twice and verify the finalized owner inventory shows quantity two.
2. Equip one copy in each hand through the existing CharacterService RPC/UI.
3. Spend HP and Second Wind; separately spend Rage on a Barbarian fixture and leave temporary conditions persisted.
4. Start a new Reference Tomb run and verify full HP, normal hit-die recovery, restored Second Wind/Rage, retained passive conditions, and removed temporary conditions.
5. Activate Second Wind in combat and observe an activation line followed by the exact applied healing line in both Story and Debug.

## Delivery decomposition

This design lands as three independently reviewable slices under one Project 19 parent:

1. Quantity-aware equipment selection, inventory projection, and dual-copy equip.
2. First-admission normal long rest through root D&D rules, a resolution adapter, toolkit Session Join persistence, and a thin API consumer.
3. Durable activation and typed result events.

Each slice starts at the owning toolkit provider, publishes the required module tag, then advances through dependent toolkit modules and proto/API/web consumers only where its contract requires. Slice B publishes one PR/tag per toolkit module (`rulebooks/dnd5e`, then `resolution`, then `session`) before the thin API consumer removes its obsolete reset loop and pins those tags. Consumer work pins published tags; no permanent local replacements ship.

## Out of scope

- Thrown-weapon delivery, inventory consumption on throw, and retrieval. Tracked separately by [rpg-toolkit#1355](https://github.com/KirkDiggler/rpg-toolkit/issues/1355).
- Unique instance IDs for identical mundane items.
- Ammunition consumption and recovery.
- A player-triggered rest RPC or rest UI.
- New character-creation layout or combat-log visual design.
- Client-side equipment, rest, feature, or condition rules.

## Success criteria

1. A valid choose-two equipment requirement accepts the same weapon twice and finalizes it as one quantity-two stack.
2. Fixed starting quantities such as two handaxes, four javelins, and ten darts reach the owner UI truthfully.
3. A quantity-two compatible weapon stack can occupy main hand and off hand; the owner projection preserves both slot entries, and a quantity-one stack cannot occupy both.
4. A character's first-ever toolkit Session Join invokes resolution's attached normal LongRest path and persists its result before seating; reconnect and exit/rejoin do not rest again.
5. HP, death saves, hit dice, spell slots, character-owned resources, feature-owned resources, and stale prior-session action economy follow their normal long-rest behavior; the next fight begins with fresh action and bonus action.
6. Every shipped loadable condition has a tested long-rest retain/reset/end decision.
7. Successful activation produces a durable activation event followed by each actual result event; refused activation produces none.
8. Second Wind's result reports actual HP recovered after clamping and exposes its roll-plus-modifier arithmetic in both Story and Debug; future healing produced through `Activate` uses the same typed path.
9. Condition activation results show a readable server-authored name in Story and the canonical condition ref and name in Debug.
10. Encounter perception scopes activation/result audiences so a non-observer in another room receives neither live nor catch-up knowledge of a visible condition change.
11. Catch-up and live session streams carry identical typed activation facts.
12. `rpg-api` contains no equipment, rest, feature, or condition rules, and the web renders/echoes server-authored data without re-deriving them.
