# Base action damage — the next provider delivery

[Project#543](https://github.com/KirkDiggler/rpg-project/issues/543) ·
[Delivery guide](delivery.md) · [Design R3](design.md)

The player needs to see **what this offered action is using before its contextual
effects**. The existing effects path is not being replaced or expanded.

## Player-facing example

For an explicitly authored, one-handed Longsword fixture using STR +3:

```text
Longsword
Base damage   1d8 + STR modifier (+3) · Slashing
Grip          One-handed
Costs         Action

Effects
…existing provider effect rows, shown separately…
```

The +3 is an example fixture value, not an inference about a live character.
Two-handed use would carry the assembled 1d10 profile instead. A weapon using
DEX or a provider-selected alternative must name that actual ability rather
than always saying STR. This describes a damage expression, not damage dealt.

## Facts the toolkit must provide

| Fact | Authoritative source | What must not happen |
|---|---|---|
| Damage dice for each pool | The compiled action's damage pools | Browser lookup by weapon name; always showing a catalogue's one-handed die |
| Damage type for each pool | Each same compiled pool | Collapsing different damage types into one number |
| Intrinsic flat bonus, if present | The compiled pool | Dropping a declared bonus or adding it twice |
| Chosen ability and its modifier | The assembled attack's ability evidence | UI choosing the highest ability, inferring class/level, or using ability score rather than modifier |
| Whether that modifier participates in base damage | The same policy execution uses, including off-hand rules | Adding a positive off-hand modifier that only a later effect may restore; treating zero as absent |
| Grip/attack variant, where applicable | The definition actually offered | Showing a hypothetical grip or another inventory item's profile |

**Assembled base is not necessarily raw catalogue base.** If the provider has
already selected an ability/die replacement while assembling the action, the
inspection reads that resulting profile. Contextual effect rows still explain
what applies; neither API nor UI re-applies their contributions or folds a total.
Only current damage-bearing profiles emit damage facts. An action with no damage
must not acquire a made-up `0 damage` line.

## Existing wire is sufficient for this display

Protos#384 is already merged. The consumer already reads:

```json
{
  "information": {
    "description": "Provider-authored action explanation",
    "details": [
      {"label": "Base damage", "value": "1d8 + STR modifier (+3) · Slashing"},
      {"label": "Grip", "value": "One-handed"}
    ]
  }
}
```

These are **provider-formatted display facts**, not expressions the browser
parses. Ordered/repeated detail labels support multiple damage pools. Existing
`AttackRef.damage_type` alone cannot supply the missing dice/modifier; reading a
second equipment catalogue in the client would describe the item rather than
the action currently offered.

No additional proto field is required for this display slice. Typed dice or
arithmetic fields would need a concrete additional consumer use case; they are
not prerequisites for showing the current provider-authored facts.

## Work by owner

1. **Toolkit root — #1987 / draft#1985:** complete the read-only projection from
   the compiled action. The draft already starts `actions.Describe`; verify the
   actual profile/variant cases rather than manufacture a parallel weapon table.
2. **Resolution owner:** share the existing base-modifier participation policy
   with the projection. This is consistency work, not new damage behavior or a
   new effect-assessment system.
3. **Session owner:** carry the projection as seam-owned declaration information
   on available and unavailable compiled offers. Keep current effect/candidate
   fields unchanged. Display text is not selector material.
4. **API — #1084:** adopt the provider SDK and copy the fields into the published
   protobuf shape. No arithmetic or content decisions in the adapter.
5. **UI:** render the supplied facts and the existing effect rows. The concept
   explicitly authors the example payload above; it is not a runtime fallback.

The first demonstrable milestone is the real Longsword offer carrying its base
facts through Afford to the existing card. Other action/option descriptions
remain tracked under #1987; this milestone does not claim their entire coverage.
The UI team does not take over toolkit/API implementation or unrelated repairs.

## Proof before calling it in-game

- One-handed/two-handed Longsword uses the corresponding assembled dice.
- Finesse or another provider-selected ability shows the actual chosen ability.
- Positive/zero/negative modifiers retain execution's base participation rules;
  off-hand behavior is not duplicated in the presentation compiler.
- Multiple pools/types stay separate; additional effects are not summed into
  base damage. Changing only an effect answer cannot rewrite that base expression.
- Unavailable offers retain their facts; missing facts remain missing, not zero.
- The API response and displayed text agree exactly after refresh/reload.
- Hovering the weapon or a target performs no gameplay operation. Target effects
  continue through the already-established candidate-answer path.

A fixture screenshot proves the intended presentation. A real provider response
and joined stack walk prove delivery. Both are needed, and they are different.
