# Feature activation — build plan and ledger

Companion to `design.md`. This file stays open through the build and records what the
implementation **corrected** about the design, so the next slice inherits the corrections and
not just the conclusions.

## The chain

Four repos. Protos merge first; the rest build in parallel against the merged contract and
merge bottom-up in one sitting.

| # | repo | what | depends on |
|---|---|---|---|
| 1 | rpg-api-protos | `VERB_ACTIVATE`, `AbilityRef`, `Declaration.ability`, `SHORTFALL_REASON_UNAVAILABLE`, `CURRENCY_CHARGES`, `Activate` RPC | — |
| 2a | rpg-toolkit | fix #1093 (`ActivateAbility` nil-deref) | — |
| 2b | rpg-toolkit | `targetKindForRef` learns Rage and Second Wind | — |
| 2c | rpg-toolkit | `resolution`: the activation arm | 2a |
| 2d | rpg-toolkit | `session`: `VerbActivate` declarations out of `Afford` | 2b, 2c |
| 2e | rpg-toolkit | `session`: `Manager.Activate` | 2d |
| 3 | rpg-api | `SessionService.Activate` + the `Afford` projection | 1, 2e |
| 4 | rpg-dnd5e-web | the dock renders and fires the new verb | 1, 3 |

2a and 2b are independent of everything and can land first — they are the two smallest PRs and
both are defects rather than features.

## Order within the toolkit

**Dodge first, Hide second.** Dodge is the purest case: self-targeting, no resource, no input
beyond the actor, and a condition #294 already expires correctly — so the first end-to-end
proof exercises the whole seam with nothing else moving. Hide next, because it is the only one
of the seven that needs an input session must supply (`ObserverPassivePerceptions`), and a
missing input is much cheaper to find second than seventh.

Then Dash, Disengage, Help, then Rage and Second Wind (the two with charges, which are the two
that need `CURRENCY_CHARGES` to say anything useful).

## Ledger

*(corrections the build made to the design — filled in as they happen)*

- **Before the first line of code:** `targetKindForRef` was described in the slice issue as
  already knowing the answer for everything. It does not know the two features, by design —
  its `default` returns `TargetKindUnspecified` specifically so a new ref shows up as a defect.
  It showed up. Design §7 corrected; it is now step 2b rather than a surprise inside 2d.

## What does not go in this slice

- Grouping the dock by slot (design §1.2). Deferred until Kirk has pressed the buttons.
- Standing up from prone (toolkit#961) and the can't-act conditions (Incapacitated, Paralyzed,
  Stunned). Both are unblocked by this slice and neither is in it — `Afford` becomes the place
  a condition can remove a declaration, which is the seam they were waiting for.
- Any class above level 1, and any class outside monk / fighter / barbarian / rogue.
