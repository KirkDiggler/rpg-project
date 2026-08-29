# Phase 6 survey — the handle's funeral, and the pile behind it

**Surveyed:** 2026-08-29, rpg-toolkit `origin/main` @ `8e6b2be` (tags at HEAD:
dnd5e v0.116.0, resolution v0.24.1, session v0.40.2). Baseline `go build` and
`go vet` clean. rpg-api (`dev` @ 582e4cc) references none of the symbols named
below — [[no-backcompat-baggage]] applies cleanly.

## Headline

The deletion Phase 6 was named for — `OwnerAware`/`SetOwner` and the two
loader handoffs — is **~45 lines in 3 files plus 2 doc paragraphs**. A
half-hour PR that no submodule even compiles against (the interface has no
implementors and no cross-module references), so it needs no downstream pin
chain at all.

The honest Phase 6, if the vestigial pile is swept rather than filed, is
**~1,000 lines of dead combat/ machinery** — and three of those deletions
carry rulings, not mechanics: two cost real end-to-end coverage with no
in-module replacement, and one (MarkClean) forces a three-PR cross-module
chain through nine test fakes.

**The structural finding (the "what did we learn" line for this slice):** the
owner handle was never on the path that mattered. Three ways a condition
reaches a bus; the SetOwner assertion sat on ONE of them:

- `character.Attach` (load.go:165) and `monstertraits.AttachMonster`
  (loader.go:289) call it;
- `Draft.Finalize` reaches the bus through `Character.subscribeToEvents` →
  `SheetKeeper.subscribeSelf` (character.go:1170-1176) and never enters that
  loop;
- the keeper's own runtime handler — the ordinary "a condition was applied
  mid-fight" path — calls `event.Condition.Apply(ctx, bus)` directly with no
  owner handoff at all (character.go:1193, monster.go:415).

So the handle was structurally absent for every condition applied during
play. Deleting it removes an inconsistency, not a guarantee. A handle wired
at one of three attach sites is not a mechanism, it is a coincidence — the
same shape as toolkit#1251.

## F1 — OwnerAware/SetOwner deletion scope (the named work)

Complete inventory, every reference repo-wide:

**Delete (code):**
- `events/events.go:136-159` — doc block + the interface.
- `character/load.go:160-168` — comment + the `if aware, ok :=` handoff
  (`dnd5eEvents` import stays; `BusForEffect` used at :158).
- `monstertraits/loader.go:273-292` — the twin (import stays likewise).

**Rewrite (prose):**
- `gamectx/doc.go:88-94` — closing paragraph describes a present state that
  ends, and `[...events.OwnerAware]` becomes a godoc link to nothing. Past
  tense, symbol as prose not doc-link.
- `docs/adr/0025-gamectx-pattern.md:15` — names both a superseded read path
  and the dying symbol; one clause in the superseded ADR's "what replaced
  this" section.
- `conditions/opportunity_attack.go:98` — the ONE place the inherited
  asymmetry fact is written; keep the paragraph, but "a monster's SetOwner
  never matched combat.Ledger" should become "a monster never satisfied
  combat.Ledger" once SetOwner names nothing.

Zero tests reference either symbol. Zero types declare `SetOwner`. The
past-tense "owner handle" prose in requests.go:20, shield_surface_test.go:24,
fighter_encounter_test.go:68, session/attack_test.go:355,
projection_test.go:106 is accurate history and stays.

## F2 — the inherited question: did the call site encode anything?

**No — and this time the facts were written down BEFORE the sweep.** Phase 5
did the work Phase 4 had to discover the hard way:

- The `purse != nil` kind-signal is recorded in THREE independent places:
  opportunity_attack.go:96-107 ("Where the asymmetry went"),
  monstertraits/loader.go:284-288, events.go:707-712 (SpendRequestedEvent's
  "A KEEPER MAY HAVE NO ROW FOR IT AT ALL").
- Nil-owner no-op semantics became an explicit fail-closed branch —
  `member(ctx,id)` !ok = NOT ELIGIBLE, a third answer the handle never had
  (opportunity_attack.go:109-119, fighting_style_protection.go:157-165).
  Strictly more information, not less.
- Ordering ("called once, before Apply"): nothing depends on it; no type
  implements the method.
- Kind information (`SetOwner(c)` vs `SetOwner(m)`): now lives in the keeper
  subscription tables and `combat.Member`'s two documented monster answers
  (combatant.go:93-123).

requests.go:15-24 already states the closing argument: "there is nothing
left for the handle to carry."

## F3 — the vestigial pile (gamectx is CLEAN; combat/ is the pile)

Every exported top-level identifier in combat/ and gamectx/ swept for
non-test external references. gamectx: every export has a live production
caller. combat/:

| Item | Size | Production callers | Cost of deletion |
|---|---|---|---|
| `TurnManager` family (4 files) | 378 lines | ZERO (incl. tests) | none — twice-documented dead (capacity.go:16-19, session/clockboundary_test.go:30); character.go:527-545 names it as `Character.EndCombat`'s paired deletion, "the two should go together in one pass" |
| `MoveEntity` + movement.go | 277 lines | only TurnManager.Move | the ONLY end-to-end Disengaging→MovementChain test (character/integration_test.go); successor `resolution.NewMovement` is in another module with zero production callers itself — **ruling #2** |
| `combat.WithRoom` + `roomContextKey` | — | only TurnManager.buildContext | none; it is a SECOND room context key, exactly the "sixth mechanism" shape resolution/doc.go:251-254 says was deleted, invisible to `TestOnlyTheDoorInstallsGameContext` (matches `gamectx.With*` names only) |
| `DealDamage` + private chain fold | ~170 lines + 310-line test suite | ZERO (docs/status.md:328 already recorded it) | the barbarian rage-resistance E2E test (partial cover exists: raging_test 0.5 multiplier, final_damage_test arithmetic, damage_custody_test live path); deletion also removes a DOCUMENTED latent double-apply (damage.go:172-180) — **ruling #3**. KEEP `DamageInstanceInput` + `FinalDamage` (strike.go:511 consumes them; the naive sweep lies here — `:=` never spells the type) |
| `combat/healing.go` (whole file) | 60 lines | ZERO — a chain nobody folds | none (do not confuse with live `events.HealingReceivedEvent`) — **ruling #7** |
| `Character.MarkClean` + `Monster.MarkClean` | — | ZERO (resolve.go:539,552 READS IsDirty, never clears) | interface removal breaks nine fakes across four packages + regenerated mock + resolution's `acRefusingTarget` (cross-module chain) — **ruling #4** |
| `Character.MarkDirty` | — | ZERO, incl. tests | its doc opens "This is the write half of an effect's owner handle" — the dead machinery. Monster.MarkDirty is kept-and-pinned with a written reason; the character twin is uncalled with a dead reason. Symmetry ruling: delete or pin — **ruling #4b** |
| `events.ResourceConsumedEvent` + topic | — | zero pubs, zero subs | none |
| `ReactionConsumption` / `AttackChainEvent.ReactionsConsumed` | — | written at fighting_style_protection.go:212-216, read by NOBODY in production | SpendRequestedEvent (three lines later) is the real channel now; ADR-0027:40's imagined consumer was never built; [[pre-v1-full-data-log]] gives "keep as shelf" a live argument — **ruling #6** |
| `resolution.NewMovement` | — | zero (test-only) | **FLAGGED, NOT-DELETE** — slice #316's driver; its doc carries Kirk's 2026-08-28 load-everything ruling verbatim. Different lane. |

Sweep false positives chased down and cleared: EffectiveACCalculator,
NewActionEconomy, CapacityTypes, IsCapacity, ACSourceType,
DamageInstanceInput, FinalDamage, AttackCategory.

## F4 — kind-named fields on both-kinds events

The convention already indicts its one offender: events.go:675-680 says
outright that `ConditionRemovedEvent` "is not the convention to copy here."
Full sweep of events.go:

- **The offender: `ConditionRemovedEvent.CharacterID`** (events.go:647). Both
  keepers subscribe; monster compares `event.CharacterID != m.id`
  (monster.go:447). Seven publishers in conditions/, each reading a
  `CharacterID` field on its own condition struct — the kind-naming runs one
  level deeper than the event. The event also has NO doc comment at all.
  Rename is breaking, ONE module, ~10 sites, no submodule constructs it —
  **ruling #5**.
- Doc-only: `DamageReceivedEvent.TargetID` and `HealingReceivedEvent.
  TargetID` docs say "character"; the fields are already kind-neutral.
- Genuinely character-only, keep: RestEvent, the death-save trio, the four
  monk events (all with zero subscribers today), plus the moot fields on
  types dying in F3.
- Character-only BY RULING, not nature: the four ability-activated events
  (Dodge/Disengage/Help/Hide) — `resolution.NewActivation` refuses a monster
  ("abilities are driven, not declared"). Renaming now is renaming ahead of
  the need. Flagged, not proposed.
- Correct shape already: `MovementChainEvent.EntityID/EntityType` carries
  kind as data.

## F5 — the ConditionRemoved filter divergence

Character (character.go:1210-1246): round-trips each condition through
`ToJSON()` → unmarshal → compare ref string; an error RETURNS mid-list,
leaving earlier removals un-applied and the removal silently dropped.
Monster (monster.go:446-466): `condition.Ref().String()` compare, no error
path. The monster's doc (431-436) already states the case for converting the
character — it predates conditions naming themselves (#971).

Observable change from converting: only for a condition violating
`ConditionBehavior.Ref()`'s own contract ("returns the same ref its ToJSON
embeds", events.go:117-120). Today that yields an error + dropped removal;
after conversion, a silently non-matching condition. Both wrong, second
quieter — pair the conversion with the `refOf`-style contract check
monstertraits already uses (loader.go:268-270).

## F6 — docs that aged at the reading seam

**False, must fix:** fighting_style_protection.go:180-183 ("the one registry
resolution.Resolve DOES install" — it installs three, and the file reads the
cast 18 lines earlier); session/attack_test.go:210-212 (same falsehood,
citing a doc that now says the opposite); Character.MarkDirty's doc +
resolution/dirty_test.go:25 ("write half of an effect's owner handle");
gamectx/doc.go:88-94; ADR-0025:15; damage.go:47 (names deleted
`gamectx.GetCombatant`) and :241 (error string names deleted
`ResolveDamageInput`) — both moot if DealDamage goes; a literal `%s` typo at
fighting_style_protection_test.go:86.

**Vacuous (describe the absence of symbols that no longer exist, so they pin
nothing):** attack_test.go:220-221, :251, :291-292, :405 ("No
gamectx.WithGameContext is installed" — the symbol is gone, absence is
unavoidable); fighting_style_protection_test.go:240-244 (claims a regression
"fails with ErrNoGameContext" — it cannot; it would fail on the disadvantage
count, the outcome the comment says it is NOT — actively misleading about
its own failure mode); truth_test.go:71-78 ("43 times… Phase 3 takes those
readers off the hand-installed path" — Phase 3 landed, count is now 45, and
hand-installing is what a one-fold test correctly DOES).

**Checked and correct:** gamectx/cast.go, resolution/doc.go:185-260,
resolution/activation.go, events.go:713-717 (ReactionUsed supersession
note), monster.go keeper docs, conditions/requests.go, combat/combatant.go.

## F7 — the "only the keeper names combat.Combatant" pin

After the F3 deletions, every production naming of the TYPE inside dnd5e is:
the definition, the generated mock, two `var _` keeper-proof assertions
(character/shield_surface_test.go:17, monster/shield_test.go:21 — the
allowed set MUST permit these), and the deliberate testdata probes. **The
interesting statement lives in resolution, a different Go module** —
strike.go:104/:117/:605, the legitimate keeper surface (cast.go:49-52:
"Resolution IS the keeper").

The existing widening pin cannot see it: `TestOnlyTheDoorWidensACastMember`
type-checks the dnd5e module only and skips go.mod-bearing dirs
(member_widening_test.go:456-482, acknowledged at :87-93). So the pin is TWO
pins, one per module:
- dnd5e: "nothing outside combat/ names combat.Combatant" — cheap, locks in
  the C/D deletions.
- resolution: "combat.Combatant is named only in strike.go, at the
  applyDamage seam and combatantFor" — the one that carries meaning. The
  widening pin says a rule cannot TURN a Member into the keeper's surface;
  this says nobody but the keeper NAMES that surface. Together they are the
  write law with the compiler behind both halves.

Model: truth_test.go's source-reading pin (matches references, not calls).

## F8 — keeper symmetry, side by side

character/sheet_keeper.go:140-162: applied · removed · healing ·
state-changed · **spend** (+ recoverable resources on/off the bus).
monster/load.go:199-221: **damage** · healing · applied · removed ·
state-changed.

Three asymmetries; ONE is the ruling (spend, documented three times). The
damage-received row is a KNOWN DEFECT awaiting toolkit#977 — the monster
treats a notification as an instruction (strike.go:538-564: "a 4-damage bite
took a wolf from 11 to 3 with the publish in place"), which is why resolution
refuses to publish DamageReceivedEvent and three genuine rules (Undead
Fortitude, Unconscious, Rage upkeep) don't fire for resolution-driven damage.
Phase 6 does NOT touch it — but the explanation lives in a different module
from the row it explains; a one-line pointer on monster/load.go:203-205 is a
cheap fix. Recoverable-resources-character-only is correct but unstated
monster-side. Handler semantics otherwise symmetric; only F5's matching
diverges.

## F9 — test blast radius

- **Die with the machinery:** combat/damage_test.go (310 lines),
  combat/combatant_dirty_test.go's MarkClean subtest.
- **Real rules through a dead vehicle** (rewrite or knowingly drop):
  integration/barbarian_encounter_test.go:282-330 (the only E2E "10 slashing
  becomes 5 and HP drops"); character/integration_test.go's MovementSuite
  (the only Disengaging→MovementChain→MoveEntity exerciser — successor is in
  another module, cannot port in place).
- **Mechanical edits if MarkClean leaves the interface:** nine fakes across
  four packages incl. resolution's acRefusingTarget (forces the go.mod
  chain), mock regenerated. The ~20 concrete-type `MarkClean()` call sites
  in character//monster/ suites keep compiling unless the METHODS go too.
- **Must pass byte-identical (the regression net):** all of conditions/ and
  monstertraits/, resolution dirty/strike/truth/world, all four widening-pin
  tests, session clockboundary/attack/no_bus, monster_markdirty. PR A is
  provably unreachable code — the correct evidence is "suites identical in
  outcome," not new tests.

## F10 — module and tagging shape

`rulebooks/dnd5e/events` has no go.mod — everything in F1–F5 lands in the
ONE dnd5e module (the top-level `/events` module is the generic bus,
untouched). Pre-1.0, so deletions are MINOR (`!` subject) per #1303/#1304
precedent. Consumers: resolution, session, encounter, behavior. Ordering:
PR A needs NO downstream chain; MarkClean forces dnd5e→resolution→session;
the CharacterID rename is single-PR (no submodule constructs the event).

## Rulings Kirk may need

1. **How big is Phase 6?** Named deletion (A+B) now and file the pile as its
   own slice, or sweep the pile now? **Recommend A+B now, pile separate** —
   its two costly items each need a decision, and bundling buries the
   decisions inside a phase whose stated purpose is cleanup.
2. **MoveEntity/WithRoom:** delete now (makes "one mechanism, one key" true,
   kills the second room key) at the cost of the only E2E Disengaging
   movement test — or wait for #316 to wire NewMovement so the coverage
   moves rather than evaporates?
3. **DealDamage:** same shape, cheaper — resolution already folds that chain
   live, and deletion removes a documented latent double-apply.
4. **MarkClean** (and Character.MarkDirty): delete from the interface + both
   sheets (nine fakes, cross-module chain), or keep as the persistence
   contract with no caller? Precedent is asymmetric: Monster.MarkDirty
   kept-and-pinned with a reason, Character.MarkDirty uncalled with a dead
   one.
5. **ConditionRemovedEvent.CharacterID → MemberID:** the code already argues
   for it; breaking, one module, ~10 sites. Now, or leave the indictment
   standing?
6. **ReactionsConsumed:** delete (nothing reads it; SpendRequested is the
   real channel) or keep as ADR-0027's combat-log shelf
   ([[pre-v1-full-data-log]] is the live argument for keeping)?
7. **combat/healing.go:** delete the unused file or keep as authored
   vocabulary? Same question shape as #6.
8. **The resolution-side Combatant pin:** worth it? Survey's read: yes — the
   cheapest item here, and it makes "resolution IS the keeper" checkable
   instead of asserted.

## Proposed PR split

- **A — feat(dnd5e)!: the owner handle's last two call sites, and the
  interface.** F1 complete + the three-attach-paths finding as the body's
  "what did we learn." Evidence: full suite byte-identical. No downstream.
- **B — chore(dnd5e): comments that outlived what they described.** F6's
  dnd5e items + the pointer comment on the monster damage row. Foldable
  into A.
- **B′ — chore(resolution)/chore(session): test comments that pin nothing.**
  F6's other-module items; test-only, no version bump needed.
- **C — feat(dnd5e)!: the turn manager and its movement, deleted together.**
  The pairing character.go:535-538 asks for by name. Carries ruling #2.
- **D — feat(dnd5e)!: one damage flow, not two.** DealDamage + healing.go if
  ruled. Carries rulings #3/#7.
- **E — feat(dnd5e)! + pin PRs: only the keeper's surface has the keeper's
  methods.** MarkClean off the interface. Carries ruling #4; lands after C/D
  or it fights them for the fakes.
- **F — test(dnd5e + resolution): only the keeper names combat.Combatant.**
  Two pins, two modules; lands last (allowed set is only small once C/D
  land).

**Minimum that closes the initiative: A + B + B′ + F.** C, D, E are the
pile, each carrying its ruling.
