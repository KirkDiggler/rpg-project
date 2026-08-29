# Feature activation — a player can use what their character already carries

**Slice:** rpg-project#300 · **Journey:** #253 · **Follows:** #295 (the clock, combat end, rage)

Scope ruled 2026-08-27: **monk, fighter, barbarian, rogue, at level 1.** All other classes
deferred.

---

## 0. The finding, and why it is the third of its kind

Every character already carries six combat abilities — Attack, Dash, Dodge, Disengage, Help,
Hide — attached at draft time by `Draft.initializeStandardCombatAbilities` (whose own doc
comment still names only four of them). Each has a real `CanActivate`
and `Activate`. Each declares its own `ActionType`. The barbarian additionally carries Rage,
the fighter Second Wind.

**None of it is reachable from the wire.** `Character.ActivateAbility` has zero callers outside
its own package. `ActivateFeature` exists in the protos on `EncounterService` — and rpg-api
registers Authoring, Character, Dice, Lobby and Session. There is no EncounterService. The verb
did not survive the encounter rip-out (rpg-api#801) and `SessionService` never grew a
replacement.

So **#294 made Dodge expire correctly at the start of your next turn, and nobody can Dodge.**
The only reason rage appeared in #295's live proof is that I wrote the condition into Redis by
hand.

This is the same defect as the last two slices — built, correct, unwired — one layer further
out. The lesson to carry: **fixing a rule's lifecycle proves nothing about whether a player can
ever invoke it.** #294 and #295 both shipped green, both were verified live, and both were
verified on a condition no player could have created.

---

## 1. From the panel back

### 1.1 What the dock does today

`ActionDock.tsx` renders declarations verbatim and derives no rules — a button per declaration,
with a label, an icon, a cost badge, a disabled state and the server's refusal as the tooltip.
It already handles everything a new verb needs, with four exceptions.

| what it already does | line of evidence |
|---|---|
| Renders the slot as A / B / R / ◇ | `CostBadge` — all four `Slot` values, including `NONE` |
| Disables and explains a refusal | `title={declaration.why?.text}`, plus a `semanticOnly` span |
| Tolerates many rows per verb | key is `` `${declaration.id}:${index}` `` |
| Distinguishes clock kinds and "not your turn" | three passive rows before any button |

### 1.2 The four things that change, and one that must not

1. **`executableDeclarations` is an allow-list**: `verb === ATTACK || verb === MOVE`. A
   `VERB_ACTIVATE` declaration is **silently dropped** by today's client. That is exactly the
   rollout property we want — an un-upgraded client ignores a verb it cannot execute rather
   than rendering a dead button — and it also means the web change is mandatory, not optional.
   The proto and the SDK can land ahead of it with no visible effect, which is what makes the
   four-repo chain safe to merge bottom-up.

2. **`declarationLabel` falls through to `'Move'`** for anything that is not `ATTACK`. It reads
   `declaration.attack?.name`. An activation needs its own identity to read, which is what §2
   adds. A fall-through label is a producer defect made visible, not hidden: "Move" on a Rage
   button is loud.

3. **`declarationIcon`** is a two-way ternary on `ATTACK`. Same shape.

4. **Grouping.** The dock has one group, labelled "Actions", holding attack and move together.
   Adding five action-slot abilities and a bonus-slot rage to that one row is the honest
   minimum, and the badge already distinguishes them. **I am not proposing a new dock, a
   sub-menu, or a slot-grouped layout in this slice.** The reason is rule 5: a layout decision
   made before a player has ever pressed one of these buttons is a guess. Ship the row, watch
   Kirk press Rage, then decide whether it needs shaping.

The one that must **not** change: `exactlyOne(declarations, Verb.END_TURN)` returns `undefined`
when more than one row matches. That encodes a one-row-per-verb assumption, it is applied only
to End Turn, and End Turn stays single. It is correct where it is and would be a bug anywhere
else — worth a comment saying so, since the rest of the file is about to stop being
one-row-per-verb.

### 1.3 What the player sees

On a barbarian's turn, at level 1, with a greataxe:

```
Actions   [⚔ Greataxe A]  [➜ Move 30ft ◇]  [✦ Rage B]  [✦ Dodge A]  [✦ Dash A]
          [✦ Disengage A]  [✦ Help A]  [✦ Hide A]                    [End turn →]
```

After pressing Rage: the Rage button goes disabled with "bonus action: 1 needed, 0 left", the
character's condition list shows raging, and the next hit rolls +2 damage. At the end of the
tenth round it expires on its own — which is the machinery #295 built and nobody could reach.

---

## 2. The proto, field by field

All additive. `buf breaking` guards it. No hand-written tests (rpg-project convention).

### 2.1 `Verb` gains one value

```proto
// Activate — using a combat ability or feature the character already carries:
// Dodge, Dash, Disengage, Help, Hide, Rage, Second Wind. It arrives here the
// day the SDK gates it (session/vNEXT), which is this one.
VERB_ACTIVATE = 4;
```

The enum's own doc says "a value arrives here the day the SDK gates the verb, never ahead of
it." This is that day.

### 2.2 `Declaration` gains an identity field

```proto
// The sole public activation identity. Present on every compiled Activate
// declaration, including one disabled by budget or feature gates; absent for
// Attack, Move, End Turn, and early per-verb blockers. Mirrors
// Declaration.attack's presence law exactly.
AbilityRef ability = 12;
```

```proto
// AbilityRef identifies WHAT is being activated. Mirrors AttackRef's shape and
// its reason for existing: the seam has an identity the client must render and
// must never parse.
message AbilityRef {
  // Fully-qualified core.Ref — "dnd5e:combat_abilities:dodge",
  // "dnd5e:features:rage". Display keys off `name`; this is for correlation
  // and for the client's own icon table, never for a rules decision.
  string ref = 1;
  // Display name — "Dodge", "Rage". Authored by the ability, not derived from
  // the ref by the client.
  string name = 2;
}
```

`attack` and `ability` are siblings rather than a `oneof` for the same reason `attack` is not
one today: a `oneof` would make it a wire-level error for a future verb to carry both, and
`Requires`-gated bonus attacks (§5) are the case where one declaration plausibly carries an
attack identity *and* an activation origin — a case we can name rather than a hypothetical.

The asymmetry decides it without needing a principle: a `oneof` adopted now cannot be undone
without a break, while one adopted later can be.

### 2.3 The refusal vocabulary needs one value

Today `ShortfallReason` has NO_BUDGET, NOT_YOUR_TURN, NO_TARGET_IN_REACH, DOWNED, UNREADABLE,
TARGET_OUT_OF_REACH. `Currency` has ACTION, BONUS, REACTION, MOVEMENT.

Two activation refusals have nowhere to land:

- **"no rage charges left"** — a ledger ran out, but not one `Currency` names.
- **"you are already raging"** / **"you are at full hit points"** — not a ledger at all. A
  feature-state precondition, which is what `Feature.CanActivate` returns.

Proposal:

```proto
// The feature's own precondition failed — already raging, already at full hit
// points, nothing to stand up from. NOT a budget: no currency ran out, and
// none is populated. `text` carries the feature's own words.
SHORTFALL_REASON_UNAVAILABLE = 7;
```

```proto
// Charges of a named feature resource — rage uses, Second Wind uses, ki
// points. A count, like the three slots. Which resource is named in `text`;
// this seam does not enumerate the rulebook's resource keys, for the same
// reason Verb does not enumerate the rulebook's actions.
CURRENCY_CHARGES = 5;
```

That keeps the seam's own law: **structured so the UI can act on it, text for narration.** A
client can dim a button and say "0 uses left" without parsing prose, and can distinguish
"come back next turn" (NO_BUDGET) from "come back next rest" (NO_BUDGET + CHARGES) from "this
will never light while you are raging" (UNAVAILABLE) — three different things a player wants
to be told differently.

### 2.4 The RPC

```proto
// Activate uses a combat ability or feature the member already carries.
// FAILED_PRECONDITION when the declaration is stale, the slot is spent, the
// charges are gone, or the feature refuses — each of which Afford reports
// ahead of the attempt.
rpc Activate(ActivateRequest) returns (ActivateResponse);

message ActivateRequest {
  string session_id = 1;
  string member_id = 2;
  // Non-empty opaque selector echoed from Afford.
  string declaration_id = 3;
  // Required when the declaration's target_kind is MEMBER (Help); empty
  // otherwise.
  string target_id = 4;
}
```

The response is the open question in §9.

---

## 3. What the SDK can truthfully answer today

| the declaration needs | today | gap |
|---|---|---|
| identity (ref, name) | `AvailableAbility.Ref` / `.Name` | none |
| slot | `.EconomySlot`, from the ability's own `ActionType()` | none |
| target kind | `.TargetKind`, from `targetKindForRef` — *"the toolkit owns what does this action target"* | **the two features are missing from that table** — see §7 |
| available | `.CanUse` — economy **and** `f.CanActivate` | none |
| why | `.Reason` — **a prose string** | must become a structured `Shortfall` |
| resource state | `.ResourceCurrent` / `.ResourceMax` | feeds `needed`/`left` |
| selector id | — | `selectorIDFor` already exists; needs a variant per ability |

`buildAvailableAbilities()` is, field for field, **already the declaration** — assembled in the
rulebook, for a menu that no longer exists, and never once crossing the session seam. The work
is a projection and a selector, not a new pricing engine.

The one genuine conversion is `Reason` → `Shortfall`. Prose in, structure out, and the mapping
has to be authored rather than parsed: `actionTypeExhaustedReason` produces the budget cases
(we know the slot, so we know the currency), `Rage.CanActivate`'s error produces the charges
and precondition cases. **A projection that string-matched the reason would be the exact
inversion this seam forbids** — the SDK renders `text` from the structure, never the reverse.

---

## 4. The pricing question, and why it is smaller than it looks

Afford's law is explicit: read the slot off *"the SAME SpendProfile the door would charge —
never a table of what a verb 'usually' costs."* Attack obeys it by compiling a
`combatActions.Definition` with a `Cost`, hashing it into the selector, and asking
`combat.CanPay`.

Activation has a *second* path — `canUseAbilityByActionType(f.ActionType())` — and the obvious
worry is that we are about to run two pricing engines.

**We are not, and the distinction is worth stating once so it stops being re-litigated.**

- The ability's `ActionType()` **is** authored data. Dodge declares that it costs an action.
  That is not a table asserting what a verb usually costs; it is the door's own price.
- Both paths spend **the same ledger**. `consumeActionType` and `SpendProfile` spending both
  land on `c.actionEconomy`.
- What `SpendProfile` adds over `ActionType()` is *composite* prices — a slot **and** a pool,
  a requirement that is checked but not spent. Of our seven, only Rage and Second Wind have a
  second cost, and both already enforce it inside their own `CanActivate`/`UseResource`.

So: **project `AvailableAbility` for the seven.** Do not author seven `Definition`s to
re-express prices the abilities already state.

The place `SpendProfile` genuinely earns its keep is the two that are **not built** — see §5.

The debt this leaves, named honestly: an activation's price is not in the selector hash the way
an attack's is, so a selector stays valid across a price change that an attack's would
invalidate. For v1 that is not reachable — nothing changes an ability's `ActionType` mid-turn —
and Afford's regenerate-before-execute check catches the availability change regardless. It is
a real seam difference and it goes in the plan's ledger, not into this slice.

---

## 5. The two that are not built

### 5.1 Martial Arts' third part

`MartialArtsCondition` implements two of the feature's three parts: DEX substitution on the
attack chain, and the scaling damage die on the damage chain. The third — *"when you take the
Attack action with an unarmed strike or a monk weapon, you can make one unarmed strike as a
bonus action"* — is not built. `GrantedMartialArtsBonus` is an enum key with **zero writers and
zero readers**, tests included. `GrantedOffHandAttack` is in the same state. Same
dead-vocabulary pattern as `DurationType` and the turn topics.

This one is a **different shape from a feature activation**: it is a bonus-action *attack*. It
rides `VERB_ATTACK` with `SLOT_BONUS`, and it is gated on what the member already did this
turn.

And the gate is already designed. `SpendProfile.Requires` — *"keyed capacity that must be
PRESENT and is never spent"* — carries this in its own doc:

> The monk's bonus strike wants to know whether the Attack action was taken this turn, and the
> sheet ALREADY RECORDS THAT as banked capacity: the post-strike grant files it the moment the
> swing lands.

`Pools` and `Requires` are both marked *"Expressible and unexercised in v1."* **This slice is
what exercises them.** We are not inventing a price vocabulary for the monk; we are using the
one that was authored ahead of it and then left idle — which is the difference between building
on a design and re-deciding it.

### 5.2 Order

**Surface the seven first, build the two after.** Surfacing proves the seam end to end against
things that already work; the two missing bonus attacks then ride a path that is known good
rather than being debugged simultaneously with it. If the two slip, a player can still Dodge,
Dash, Disengage, Help, Hide, Rage and Second Wind — which is the slice's actual promise.

---

## 6. The bus, which is where this can silently fail

`Rage.Activate` does **not** attach the raging condition to the character. It publishes a
`ConditionAppliedEvent` carrying the condition, and `SheetKeeper` — subscribed via
`subscribeSelf` — applies it. Dodge, Hide, Help and Reckless Attack all use the same path.

That makes the bus load-bearing, and it is exactly the trap that bit the equip path
(rpg-api#842): **a condition published on a bus nobody is attached to is not an error.** It
succeeds, returns nil, and persists a sheet with no condition on it. `EffectiveAC` silently
falls back to base. Nothing in the call stack says anything went wrong.

So activation must run **with every participant attached to the interaction's own bus**, and
must save what comes back dirty. That is precisely what `resolution` exists to do (ADR-0038:
this package is the only place a bus exists), and the machine that already has this exact shape
is the one #295 built:

> **`NewBoundary`** — *"Every attached effect hears them on the interaction's own bus, so a
> condition scoped to a turn — dodging, disengaging, raging, reckless attack ... gets its chance
> to expire, tick, or fire, and comes back dirty."*

An activation is the same journey with a player as the declaring actor instead of time: attach
everyone, do one thing on the bus, collect dirty sheets.

**So it is `NewActivation`, a sibling of `NewBoundary` — not a second arm on `NewAction`.** An
earlier draft of this section said the arm, and the arm contradicts §4. `NewAction` dispatches
on a populated profile of a `combatActions.Definition`, and §4 ruled that the seven do NOT get
authored Definitions, because each already states its own price. A verb with no Definition
cannot arrive through a constructor whose first act is `in.Definition.Validate()`.

The two constructors are not redundant. `NewAction` says *"here is a compiled, priced definition
— resolve it"*, and Attack is the only thing that has one. `NewActivation` says *"this member is
using the thing they carry"*, where the ability itself is the authority for both its price and
its effect. Forcing the second through the first would mean minting a Definition for Dodge
purely so a dispatcher recognises it — a compiled price that no door charges, which is exactly
the shape §4 exists to refuse.

**The test that proves it must be able to fail.** An integration test that activates and then
reads the same in-memory character will pass whether or not the bus was live, because
`Rage.Activate` mutates resource state on the object regardless. The proof has to be: activate,
**persist**, reload from the repository, and assert the raging condition is on the reloaded
sheet — the same discipline #295's dissolve test needed, for the same reason.

---

## 7. Known traps

- **rpg-toolkit#1093** — `ActivateAbility` nil-derefs its input. This slice puts that path into
  production for the first time. Fixed on the way in, not discovered.
- **`AvailableAbilities()` returns empty when `!c.InCombat()`.** Afford is only meaningful on
  the turn clock, so this is consistent — but it means the projection must not treat "no
  abilities" as "this character has none". Empty from a cold sheet is a different fact from
  empty from a spent turn, and only one of them should reach the panel as a blocker.
- **`targetKindForRef` does not know the features.** It switches on `refs.CombatAbilities.*`
  only, so Rage and Second Wind fall to its `default` and come back
  `TargetKindUnspecified` — deliberately, *"so a new ref surfaces as a visible defect rather
  than silently defaulting."* The defect is now visible: the table has to grow the two feature
  refs, or every feature declaration ships an UNSPECIFIED target kind, which the proto calls a
  producer defect. **This is the one gap in §3's table that is not a projection.**
- **The rulebook declares six target kinds, the table emits four, the seam has three.** Of the
  four the table emits, two collapse into one at the seam. The rulebook distinguishes
  `TargetKindSelf` (Dodge, Disengage, Hide — grant a condition on the actor) from
  `TargetKindNone` (Dash — fires with no prompt). The seam has `NONE`, `MEMBER`, `PATH`. Both
  collapse to `NONE`, and that is right: they are the same instruction to a client — *do not
  prompt.* The distinction the rulebook keeps is about who the effect lands on, which is the
  rulebook's business and not the panel's. Worth one line in the projection saying so, since a
  reader who finds four values going into three will otherwise assume something was dropped.
- **Help targets a member; the other six do not.** `TargetKindSingleEntity` maps to
  `TARGET_KIND_MEMBER`, and the candidate-universe machinery Attack already uses applies to
  Help unchanged.
- **Hide publishes observer passive perceptions** (`ObserverPassivePerceptions` on
  `ActivateAbilityInput`). Session has to supply them, exactly as it supplies sight. This is the
  one of the seven that is not a pure self-activation, and it is the one most likely to reveal
  a missing input late. Worth building second, right after Dodge.

---

## 8. What proves it

Not a green suite — #294 and #295 both had one while nobody could reach the feature.

**Kirk rages a barbarian from the panel**, on the local stack, and:

1. the Rage button lights on his turn and goes dark with "bonus action: 1 needed, 0 left"
   after he presses it,
2. the persisted sheet — read out of Redis, not out of a test double — carries the raging
   condition,
3. his next hit rolls +2 damage,
4. it expires on the clock, by the machinery #295 built, without anyone touching Redis.

Step 2 is the one that distinguishes this slice from a plausible-looking one, and step 4 is the
one that makes #294 and #295 real.

---

## 9. Ruled — Kirk, 2026-08-27

All three answered before the first proto line. Recorded here rather than in chat so the build
inherits the reasons and not just the answers.

### 9.1 `ActivateResponse` is an empty ack, and a refusal is an ERROR

> *"Yeah empty ack for success or an error right?"*

Success carries nothing; the client re-reads Afford, which stays the one place that answers
"what can I still declare." The condition's arrival reaches everyone else through the event
stream rather than through the actor's response.

**And a refusal is a gRPC error, not a `success:false` field.** That is the part worth writing
down, because the rulebook underneath disagrees: `ActivateAbilityOutput` carries
`Success bool` + `Error string`, and returns `(output, nil)` for "not in combat", "unknown
ability", "no rage charges left" and every feature refusal.

So **rpg-api's handler owns a translation**, and it is a real one rather than a pass-through:

| rulebook returns | seam returns |
|---|---|
| `(out{Success:true}, nil)` | empty `ActivateResponse`, no error |
| `(out{Success:false, Error:...}, nil)` | `FAILED_PRECONDITION`, `Error` as the message |
| `(nil, err)` | the mapped error for `err` |

A handler that forwarded `Success:false` as a successful response would hand the client a
silent no-op — the exact failure the empty ack exists to make impossible, since an empty
success and an empty refusal would then be the same bytes. **This needs a test at the rpg-api
layer specifically**, because neither side has one today: the rulebook's contract is
success-as-a-field and the seam's is success-as-absence-of-error, and nothing currently sits
between them to be wrong.

### 9.2 Both new enum values land

`SHORTFALL_REASON_UNAVAILABLE` and `CURRENCY_CHARGES`, as proposed in §2.3. A client can tell
"come back next turn" from "come back next rest" from "this will never light while you are
raging" without parsing prose.

### 9.3 The dock stays one flat row

Grouping by slot is deferred until Kirk has pressed the buttons. The A/B/R badge already
distinguishes the shapes; the question of whether eight buttons in one row is too many is
answered by looking at eight buttons in one row.

— platform agent, on behalf of KirkDiggler
