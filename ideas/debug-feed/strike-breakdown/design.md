# Strike breakdown in the debug feed

Design for [rpg-project#265](https://github.com/KirkDiggler/rpg-project/issues/265), a slice of the [Debug Feed journey](https://github.com/KirkDiggler/rpg-project/issues/235).

This is contract plumbing, not a new event architecture. Resolution already owns the facts. The slice preserves a deliberately small projection through replay and renders it in the existing one-line debug formatter.

## The panel

Today a struck line ends after aggregate damage and attack identity:

```text
seq=7 clock=42 struck attacker=Fighter target=Wolf roll=18 total=23 against=13 damage=9 crit=false attack.ref=dnd5e:weapons:longsword attack.name="Longsword" type=SLASHING
```

With detail present, the same line appends raw ordered collections:

```text
seq=7 clock=42 struck attacker=Fighter target=Wolf roll=18 total=23 against=13 damage=9 crit=false attack.ref=dnd5e:weapons:longsword attack.name="Longsword" type=SLASHING components=[{source=weapon ref=dnd5e:weapons:longsword dice=1d8 final_rolls=[4] flat=0 type=SLASHING}, {source=ability ref=dnd5e:abilities:strength final_rolls=[] flat=3 type=SLASHING}, {source=condition ref=dnd5e:conditions:raging final_rolls=[] flat=2 type=SLASHING}] advantage=[{ref=dnd5e:conditions:hidden source=Fighter}]
```

This remains one copyable debug line produced by `debugLogLine.ts`; it does not add a component tree or change the player-facing Story sentence. The formatter displays supplied values and performs no arithmetic. `damage=9` and `crit=false` remain the authoritative resolved answers.

The formatter appends only non-empty collections, in component/advantage/disadvantage order. Absent attribution stays quiet. When all three collections are empty, it emits today's line unchanged; that is the fallback for old story entries.

## The minimum contract

`Struck` gains three ordered collections:

```proto
message DamageComponent {
  string source = 1;
  string source_ref = 2;       // empty means absent
  string dice = 3;             // empty for flat or multiplier components
  repeated int32 final_rolls = 4;
  int32 flat_bonus = 5;
  DamageType damage_type = 6;
  optional double multiplier = 7; // absent = additive; 0 = immunity
}

message AttackModifierSource {
  string source_ref = 1; // empty means absent
  string source_id = 2;  // entity that supplied the modifier; may be empty
}

message Struck {
  // existing fields 1–8 stay unchanged
  repeated DamageComponent damage_components = 9;
  repeated AttackModifierSource advantage_sources = 10;
  repeated AttackModifierSource disadvantage_sources = 11;
}
```

Fields 9–11 are the next free `Struck` tags. Source category and source ref remain strings because they are open toolkit vocabulary. Damage type reuses SessionService's existing closed `DamageType` enum—the same vocabulary already carried by `AttackRef`—rather than creating a second representation. References use `core.Ref.String()`'s canonical `module:type:id` form.

Multiplier alone requires scalar presence: absence means an additive component, while `0` is the real immunity factor. No other optional scalar needs presence semantics.

## Field mapping

No layer derives a missing fact:

| Wire field | Existing resolution source |
|---|---|
| `damage_components[].source` | `DamageComponents[].Source` |
| `damage_components[].source_ref` | `DamageComponents[].SourceRef.String()` when non-nil |
| `damage_components[].dice` | `DamageComponents[].Dice` |
| `damage_components[].final_rolls` | `DamageComponents[].FinalDiceRolls` |
| `damage_components[].flat_bonus` | `DamageComponents[].FlatBonus` |
| `damage_components[].damage_type` | `DamageComponents[].DamageType` |
| `damage_components[].multiplier` | `DamageComponents[].Multiplier`, preserving nil versus zero |
| `advantage_sources[]` | `StrikeOutcome.Folded.AdvantageSources` |
| `disadvantage_sources[]` | `StrikeOutcome.Folded.DisadvantageSources` |
| modifier `source_ref/source_id` | the same fields on `AttackModifierSource` |

The path is one projection:

```text
resolution.StrikeOutcome
  -> session recordFor
  -> encounter RecordInput and persisted story payload
  -> session structBody and StruckBody
  -> rpg-api setEventBody
  -> SessionService Struck
  -> web formatDebugLine
```

Encounter owns primitive carrier structs and the JSON keys `damage_components`, `advantage_sources`, and `disadvantage_sources`. It preserves slice order and does not import or validate D&D enums. Session is the only layer that converts rule-owned types and refs into those primitives. API copies fields; web prints them.

Both live delivery and `GetStory` are projections of the encounter story payload. They must produce equal typed `Struck` messages for the same sequence; there is no session-only enrichment path.

## Compatibility and refusal rules

- New JSON keys and proto fields are additive.
- Old payloads decode to empty collections and retain today's aggregate debug line.
- No story migration or backfill is performed.
- Encounter retains its existing `RecordInput` structural validation but does not become a second rule validator for source categories, damage types, dice notation, or refs.
- Component and modifier order is never sorted or grouped.
- Both advantage and disadvantage collections may be populated; the client displays both and does not infer the effective roll mode.
- Empty optional strings are displayed as absent rather than guessed.
- A present multiplier is printed, including `0`; it is never applied by API or web.

## Proof

- Encounter: one rich struck payload has stable JSON and round-trips in order; an old payload still decodes.
- Session: a representative strike maps the selected fields only; live projection and story catch-up yield equal `StruckBody` values.
- Protos: format, lint, breaking, and generated Go/TypeScript checks pass.
- API: one converter test proves direct mapping, including a zero multiplier; StreamEvents and GetStory use the same converter.
- Web: formatter tests pin the exact rich line and the unchanged old-event line. Non-empty modifier `source_id` values join the existing hover-ID collection.
- Integrated gate: observe one enriched strike live, reconnect, and observe the caught-up line unchanged.

## Deliberately not carried

Original dice rolls, reroll history, reroll reasons, damage properties, per-component critical flags, component totals, human-readable modifier reasons, conditions/save outcomes, cancellation/reaction attribution, and attack-bonus decomposition remain internal. `Missed` is unchanged. Adding any of them requires a concrete debug-feed question that the minimum contract cannot answer.

`AttackModifierSource.Reason` is intentionally not persisted: it is human-readable prose, while encounter's `RecordInput` makes caller-authored prose inexpressible. Source ref plus source entity ID provide attribution without weakening that boundary. This slice does not invent a replacement reason enum.

There is no new event kind, arbitrary metadata bag, ADR, story migration, player-facing prose, or staged combat presentation in this slice.

## Delivery issues

Proto contract [rpg-api-protos#246](https://github.com/KirkDiggler/rpg-api-protos/issues/246) merges first. Toolkit then publishes encounter [#1238](https://github.com/KirkDiggler/rpg-toolkit/issues/1238) before session [#1239](https://github.com/KirkDiggler/rpg-toolkit/issues/1239). API [#836](https://github.com/KirkDiggler/rpg-api/issues/836) pins both released providers, and web [#805](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/805) consumes the generated TypeScript contract.
