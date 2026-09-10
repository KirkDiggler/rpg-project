# The area source — a spell declares a shape in space, and the engine derives who is in it

**Status:** proposed. Two slices, in order.
**Supersedes:** the Thunderclap half of [#415](https://github.com/KirkDiggler/rpg-project/pull/415).
That document surveyed the ground before Bane shipped and before the spike below was run; three of
its conclusions are now wrong and are corrected in §7 rather than quietly dropped.

## 0. What this is, and what it is not

This is not "add Thunderclap." It is **the capability that content can name a region of space and
have the engine work out who is standing in it.** Thunderclap is the cheapest thing that exercises
that capability end to end, which is the only reason it is here — the spell's book, list and level
are irrelevant to whether the capability is right.

Everything the toolkit can hit today, it hits because somebody *named* it. A target arrives as an
ID the client clicked. Nothing in the stack can express "wherever this lands, whoever is there."
That is one missing primitive, and it is under Fireball, Thunderwave, Burning Hands, Spirit
Guardians, Shatter, Web, Grease, Moonbeam, Cloud of Daggers, Darkness, Faerie Fire, breath weapons,
and every trap with a blast radius.

The test we hold this against, per Kirk: **the burning floor Fireball leaves behind must still be
expressible after these decisions.** A design that gets Thunderclap working but makes a *lingering*
area unrepresentable has bought one spell and sold the primitive. §2.4 is where that bites, and it
is the reason for the one structural choice in this document that looks like more than the slice
needs.

---

## 1. The two slices

| | Slice 1 — Thunderclap | Slice 2 — Thunderwave |
|---|---|---|
| **Capability proved** | content declares a footprint; the engine derives its members | a footprint that is not centred on the caster; a save that buys something |
| **Footprint** | radius, origin = caster | directed volume, origin = caster, extending along a chosen axis |
| **Save** | negated on success — the shape the door already accepts | **half** on success — three validators currently refuse it by name |
| **New geometry** | none. `Distance()` already answers it | yes — a direction, and what a cube means on hex |
| **Deliberately not in it** | one-roll-for-all damage; cantrip scaling | the 10-foot push |

Slice 1 introduces no geometry. That is the single most important finding in this document and it
is the opposite of what #415 concluded.

---

## 2. Slice 1 — Thunderclap

### 2.1 The four claims, separately checkable

A slice earns its place by what it makes possible next, so each claim is worth being able to fail
on its own:

1. **Content declares a footprint, not a target list.** The declaration says *where*, never *who*.
2. **The member set is derived from the roster**, and is kind-tagged, so a non-combatant standing in
   the blast is *visible* in the result rather than silently absent.
3. **A caught member that nothing can resolve is recorded as caught.** Absence and immunity are
   different facts and must not share a representation.
4. **A footprint that can never catch anything is refused at declaration**, not shipped.

### 2.2 Where the derivation can live — three seats, and why only one works

This was the open question. It has an answer, and the answer falls out of the existing seams rather
than needing a new one.

**Seat A — `resolution`, inside the cast machine.** This is where the profile is read, so it looks
like the obvious home. **It cannot work.** `resolution`'s participants are *sheets*
(`resolution/doc.go`, R2/R3): character data, conditions, ledgers. There is no map in there. A
machine is built by `NewAction` before `Resolve` is ever called, and `Machine.Start` is pure
preflight over attached sheets. Positions live in the encounter and never cross into resolution as
anything a machine could measure. Seat A would require teaching resolution about geometry — blurring
a seam the lens says is deliberate.

**Seat B — `encounter`, as a new query.** Encounter owns geometry and the roster, so a
`MembersWithin(origin, cells, excluding)` would be trivial to write. **It should not be written.**
It is a *producer*: an API that returns a set. Encounter already exposes the two pieces such a
producer would be built from, and exposes them in exactly the right shapes:

- `Members()` (`encounter/encounter.go:975`) — the **declared universe**. Everyone the composition
  places, with `Kind` and dungeon-absolute `Position`.
- `Distance(a, b)` (`encounter/encounter.go:1038`) — the **predicate**, in the same units the
  composition's own reach and sight checks use.

`Distance`'s own doc already argues this: it "takes cells rather than member IDs because every
caller with a reach question already has both positions in hand." Adding a producer beside it would
be the special case above rather than the primitive below — and the primitive is already there.

**Seat C — `session`, folding the universe with the predicate.** This is the answer. Session already
holds both halves at the moment it needs them: `session/cast.go:179` calls `scope.enc.Members()` for
the roster, and session already uses `enc.Distance` to gate Attack on weapon reach. Deriving "who is
inside a declared 5-foot radius" is arithmetic over facts it already has, and — the test that
matters — **it contains no die, no modifier and no threshold.** The spell decided the shape; session
finds who is standing in it. That is precisely the `standing.go` precedent: *what this package
contributes is the lookup, not the rule.*

**The spike.** Written, run against a live encounter, and deleted:

```go
func caughtBy(enc *encounter.Encounter, origin encounter.MemberID, radiusCells float64) ([]encounter.Member, error) {
    all, err := enc.Members()
    // ...
    for _, m := range all {
        if m.ID == origin { continue }                       // "other than you"
        if enc.Distance(at.Position, m.Position) <= radiusCells {
            caught = append(caught, m)
        }
    }
}
```

Output: `vendor pos=(1,2) dist=1.00 kind=world` — caught, kind-tagged, from the roster. **Fifteen
lines, zero new methods on any module.** The capability's cost is a declaration and a fold, not a
geometry project.

*(The spike also caught its own trap and it is worth recording: members were placed with a test
helper that takes **offset** coordinates while `Distance` reasons in **axial**, so the first
"adjacent" ally measured 2.00. Same class as rpg-toolkit#1141 and #1150. The acceptance tests below
place members by axial coordinate for this reason.)*

### 2.3 The two universes — the sharpest reason this must not reuse the offer path

Session already computes something that looks exactly like what an area needs: `buildTargetPreflight`
(`session/offers.go:701`) walks candidates around the actor and marks each in or out of range. It is
the wrong list, and reusing it would be a bug that never throws.

That function's candidate universe is built from `holdings []intel.Holding` and skips every subject
with `len(h.CurrentVia) == 0` — **it ranges over what the caster can currently see.** Correct for an
offer, which answers *"who may you choose?"*, and wrong for an area, which answers *"who is
standing there?"* An invisible creature adjacent to the bard is not a legal click target and **is**
caught by a thunderclap.

Two questions, two universes:

| Question | Universe | Producer |
|---|---|---|
| Who may the caster choose? | the caster's intel — what they perceive | the compiled offer |
| Who is caught by this footprint? | the roster — what is actually there | the fold in §2.2 |

**This is the predicate/producer rule with a body.** A set-producing question is only as correct as
the universe it ranges over, and the universe must be *declared* — because when it is implicit, the
answer silently depends on what happened to be loaded, and the failure is invisible. A thunderclap
that quietly spares the unseen would pass every test anybody thought to write.

Consequence for the code: derived area members **bypass the offer's per-candidate gate** in
`castTargets` (`session/cast.go:411`), and that bypass is deliberate and must be commented as such,
because that gate exists to stop a client echoing a stale click. A derived set was never clicked.

### 2.4 The footprint is a shape; who it catches is a projection of it

The burning-floor test forces this apart, and it is the one place slice 1 builds a hair more
structure than slice 1 alone requires.

Thunderclap catches "every creature other than you." Fireball's lingering floor covers the same
volume and catches **everyone**, including the caster, including anyone who walks in later. Same
shape, different projection. If the exclusion lives *inside* the shape, then a stored footprint
carries a field that is meaningless for half its intended uses — which is a zero value that lies,
the exact defect `CastConcentration`'s "a pointer rather than a bool" rationale already names.

So:

```go
// Footprint is a shape in space. It says WHERE. It never says WHO.
// A cast uses one; a persistent effect could hold one and re-project it every
// time somebody moves. That second customer does not exist yet and this slice
// does not build it — the split is here so that it remains buildable.
type Footprint struct {
    Shape    AreaShape  // AreaRadius is the only value this slice ships
    SizeFeet int
    Origin   AreaOrigin // AreaOriginCaster is the only value this slice ships
}

// CastArea is one cast's use of a footprint: the shape, and the projection
// this particular spell takes of it.
type CastArea struct {
    Footprint Footprint
    Catches   AreaCatches // AreaCatchesOthers | AreaCatchesEveryone
}
```

Both enums ship with exactly one or two values and every switch over them is closed. A second shape
arrives with the spell that needs it.

### 2.5 The declaration, and why it is an enum *and* a pointer

```go
// spells/cast.go — castContent
Thunderclap: {
    name: "Thunderclap",
    build: func(in castBuildInput) actions.CastProfile {
        return actions.CastProfile{
            RangeFeet: ThunderclapRadiusFeet, // 5
            Target:    actions.CastTargetArea, // NEW
            Area: &actions.CastArea{           // NEW
                Footprint: actions.Footprint{
                    Shape:    actions.AreaRadius,
                    SizeFeet: ThunderclapRadiusFeet,
                    Origin:   actions.AreaOriginCaster,
                },
                Catches: actions.AreaCatchesOthers,
            },
            Save: &saves.SaveGate{
                Abilities:  []abilities.Ability{abilities.CON},
                DC:         saves.DCStatic(in.SpellSaveDC),
                OnSuccess:  saves.Negated,
                Recurrence: saves.RecurrenceNone,
            },
            Damage: []damage.Damage{{Dice: ThunderclapDamage, Type: damage.Thunder}},
        }
    },
},
```

**`CastTargetArea` is a new `CastTargetRule` value and not merely a nil-check on `Area`.** #415
argued for the pointer alone, on the grounds that a new enum value costs arms in three closed
switches. That cost is the feature. The alternative is precedence-by-nil — `Target: CastTargetSelf`
with `Area != nil` meaning "actually, not self" — and `newCast` (`resolution/action.go:141-146`)
would then have a self arm that rewrites `targetIDs` to the caster for a spell that must never hit
the caster. Implicit precedence between two fields is how one value comes to mean two things.

`CastProfile.Validate` gains one arm binding them together, so neither can exist without the other:

- `CastTargetArea` requires `Area != nil`, and `MinTargets == MaxTargets == 0` (the caller names
  nobody).
- `Area != nil` requires `Target == CastTargetArea`.
- **`encounter.CellsFromFeet(SizeFeet) >= 1`.** `encounter/units.go:33` is `feet / FeetPerCell`,
  integer division — a 1-to-4-foot footprint validates and can then never catch anything. Refuse it
  at declaration. (Same class as the goblin shipped with `ReachFeet: 1`, which cannot melee at all.)

### 2.6 The shopkeeper — caught, and not resolvable

Kirk's ruling: *"it can hit the npc if so but the npc can decide it doesn't take damage. there is no
infra to allow them to take damage at this time... today they are not wired into the bus."*

The spike's caught member came back `kind=world`. It has no character sheet, so
`compileResolutionCast` produces no participant for it and no contest can run. Three ways to handle
that:

1. **Derive only members that have sheets.** The shopkeeper is silently absent — indistinguishable
   from having been out of range. Rejected: it is the invisible failure this whole document is
   about.
2. **Refuse the cast when the footprint catches an unresolvable member.** Fails closed and loudly,
   and makes Thunderclap uncastable anywhere near a merchant. Rejected as a worse game for no
   correctness gain.
3. **Carry the caught member with an outcome that says what happened.** *Caught but unresolved* is a
   third fact, distinct from "saved" and from "not caught", and it is the truth: the blast reached
   them, and nothing in this build models what it does to them.

Take 3. It costs one outcome variant and it makes the missing capability *visible in the output*
rather than a gap somebody rediscovers. When NPCs are wired to the bus, that variant stops
appearing on its own — which is what a cost not yet paid should look like.

### 2.7 The wire kind

`session.TargetKind` is a closed set carrying its own policy: *"A new kind arrives only with a
proven executor for it, never in advance"* (`session/types.go:2414-2424`). Slice 1 **is** that
executor, so `TargetArea` lands here.

It must not reuse `TargetNone`. Blade Ward already uses `TargetNone` for a self cast that prompts
for nobody; an area centred on the caster also prompts for nobody, and collapsing them makes one
value mean both *"lands on you"* and *"lands on everyone but you."* There is a product reason too —
a client that knows the shape can preview the ring and say "everyone within 5 feet" instead of
offering no selector and explaining nothing.

`character.TargetKindArea` already exists as a label (`character/action_economy_types.go:71`) that
`targetKindOfAbility` collapses to `TargetNone`. That is a **name to reuse, not a mechanism to
inherit** — and the collapse is not an oversight, it is this slice's own precondition written down
in advance:

> *"Position and Area have no seam value and become TargetNone rather than an invented one, because
> nothing at level 1 uses them and a target kind arrives here with a proven executor, never in
> advance."* — `session/activations.go`

Slice 1 is the proven executor that comment was waiting for.

### 2.8 What ships knowingly wrong, and why that is not a gap

**Damage rolls once per target, not once for the area.** `newGatedCast` carries `profile.Damage` into
each target's own contest (`resolution/action.go:565`), so three caught creatures roll three separate
1d6s where the tabletop rolls one. #415 flagged this as trivial-to-fix-before-Bane; **Bane has since
shipped and that window is closed** — the per-target loop at `resolution/action.go:141-156` is now
the established shape and hoisting the roll is its own change with its own blast radius. It is a
real divergence, it is named here, and it is not slice 1's job. The capability "an area rolls one
pool and applies it to everyone" gets built when we decide the feel is wrong, not pre-emptively.

**No cantrip damage scaling at levels 5/11/17.** Dice are fixed string constants and nothing in the
cast path reads a level (`spells/cast.go:67` already declares higher-level scaling outside this cast-content slice). Sacred Flame
shipped unscaled; Thunderclap ships unscaled the same way.

**`BardCantrips` needs a contract change, and it is ours to make, not a ruling to escalate.**
`spells/cast.go:87-92` declares it as *"the 2014 PHB bard cantrip list, in book order — ALL ELEVEN,
even though this build can cast two of them"*, and `spells/cast_test.go:186` pins
`s.Len(spells.BardCantrips, 11)`.

Read carefully, that list is defined as **a transcription of one book**, which is the thing this
project is explicitly not doing. We are building capabilities; the published lists are a supply of
proof cases, not a scope to complete, and content that is in no rulebook at all is fair game. A list
whose contract is "what the 2014 PHB printed" can never hold anything we invent.

So its contract widens to **the cantrips this build offers a bard** — book order first, then what we
have added — and Thunderclap joins it. The genuinely good idea already in that comment survives
untouched and is worth restating: *the list is what a bard's cantrips ARE; which of them have
behavior is a separate question, answered by `Castable`.* That separation is why the list could
always hold entries with no profile, and it is what lets this widen without lying.

**The length assertion gets deleted, not bumped to 12.** Pinning a list's length asserts nothing
about the list; it only converts every future addition into a required edit to a test that was never
about the addition. What is worth pinning is that a named cantrip is present and castable, and tests
that do that already exist.

### 2.9 Acceptance contract

- A bard casting Thunderclap adjacent to two creatures produces two caught members; each rolls its
  own CON save; a failure takes 1d6 thunder and a success takes none.
- A creature **five cells away** is not in the derived set. A creature **the caster cannot see** but
  is adjacent to **is** — pinned explicitly, because this is the bug that would otherwise never
  surface.
- The caster is never in the derived set under `AreaCatchesOthers`.
- A `world`-kind member adjacent to the caster is returned as *caught but unresolved*, and the
  recorded beat names it.
- Declaring a footprint of 4 feet is refused by `Validate`, with a message naming cells.
- A `CastTargetArea` profile with `Area == nil`, and an `Area` on any other target rule, are both
  refused.
- Members are placed by **axial** coordinate in every test that reasons about distance.

---

## 3. Slice 2 — Thunderwave

Slice 2 exists to prove two things slice 1 deliberately avoided.

**A footprint that is not centred on the caster.** Thunderwave is a 15-foot cube *originating from*
you — it has a direction. That is the geometry slice 1 skipped, and it is the same geometry Burning
Hands (cone) and Lightning Bolt (line) need. Two sub-questions, both real: where does the direction
come from at the wire (a chosen facing? a target cell?), and what does a cube mean on a hex field.
Neither has a precedent in the tree.

**A save that buys something.** Half-on-success is already representable at the bottom:
`saves/gate.go:206` accepts `Negated` **or** `Half`. What refuses it is everything above — the cast
validator (`combat/actions/cast.go:172`, *"cast save must negate the cast on success"*) and the
contest machine (`resolution/contest.go:184`, *"a condition contest must negate on success"*). So
slice 2 is not inventing a save policy; it is letting an existing one reach the door, and then
deciding what half of a damage pool means where the pool is rolled per target (§2.8).

That is what makes Thunderwave the right second slice rather than Shatter or Burning Hands: those
need the same direction work and nothing else, so they become cheap the moment slice 2 lands.

**Known state.** Thunderwave already has a ref, an id and a `Level: 1` row; it is **not** on any
bard list (wizard/sorcerer/Tempest only). Its description strings say **2d8** where the 2014 rules
say 3d8 (`spells/types.go:370`, `spells/data.go:165`) — descriptions only, nothing computes from
them, and worth correcting on whichever branch next touches that file.

**The 10-foot push is deferred**, and the framing matters: it is not that Thunderwave is incomplete.
**The toolkit has no "an effect moves a creature" capability at all** — the same absence that made
Dissonant Whispers ship as damage-only ([#414](https://github.com/KirkDiggler/rpg-project/pull/414)).
That primitive gets its own slice, with its own customer, and it will serve both. Absent is a cost
not yet paid.

---

## 4. What this deliberately does not build

- **Runtime-mintable regions.** Regions are authored cell sets today, with no runtime minting, and
  `MembersIn` (`encounter/region.go:108`) has zero production callers. It is not dead code — it is
  the projection a *persistent* footprint will need, waiting for its customer. Its customers are
  Wall of Fire, Spirit Guardians, Web, Grease, Moonbeam, Cloud of Daggers, Fog Cloud, Darkness and
  Fireball's floor. §2.4 exists so that when the first of them arrives, the shape it needs is
  already separable.
- **Cones and lines.** `tools/spatial` ships `GetPositionsInCone` in four implementations with
  **zero callers anywhere, including tests**, and `AxialHexGrid`'s takes `acos` of a dot product
  over raw axial q/r (`tools/spatial/hex_grid.go:467`) — not the right shape for a hex cone.
  It is also **not on the `Grid` or `Room` interface** (`tools/spatial/interfaces.go` carries
  `Distance`, `GetPositionsInRange` and `GetEntitiesInRange`, and no shape query beyond them), so
  the encounter canvas physically cannot reach it without a concrete type assertion nothing in the
  tree performs. "The geometry already exists" is true and, today, useless. The first spell that
  genuinely needs a cone should expect to fix it rather than call it.
- **One-roll-for-all area damage** (§2.8).
- **Forced movement** (§3).

---

## 5. Rejected alternatives

| Alternative | Why not |
|---|---|
| Derive inside `resolution` | Resolution's participants are sheets. There is no map in there, and putting one there blurs a deliberate seam. |
| A new `MembersWithin` on `encounter` | A producer where the universe and the predicate already exist separately and compose in fifteen lines. The special case above instead of the primitive below. |
| Reuse `buildTargetPreflight` | Its universe is the caster's intel, not the roster. Thunder would spare the unseen, and no test anybody thought to write would fail. |
| `Area *CastArea` with no new target rule | Precedence-by-nil between two fields; `newCast`'s self arm would rewrite targets to the caster for a spell that must never hit the caster. |
| Reuse `TargetNone` for an area | One value meaning both "lands on you" and "lands on everyone but you." |
| Exclusion inside `Footprint` | A field meaningless for a persistent area — a zero value that lies, and it forecloses the burning floor. |
| Drop unresolvable caught members | Silently indistinguishable from out of range. The invisible failure this document exists to prevent. |
| Bump the `BardCantrips` length test to 12 | It asserts nothing about the list and taxes every future addition. Delete it. |
| Thunderwave first | Bundles the area with a slot, a new save policy, a direction, a cube-on-hex ruling and a missing movement primitive. A failure would tell you nothing about the area. |

---

## 6. Delivery

Develop outside-in, merge inside-out. One nearest-`go.mod` module per PR.

1. `rulebooks/dnd5e` — `Footprint`, `CastArea`, `CastTargetArea`, `Validate` arms, the Thunderclap
   content row, the `BardCantrips` comment correction and the deletion of the length pin.
2. `rulebooks/dnd5e/resolution` — the `CastTargetArea` arm in `newCast`; no geometry.
3. `rulebooks/dnd5e/session` — the fold, the `TargetArea` wire kind and its executor, the
   caught-but-unresolved outcome.
4. rpg-api / web pick it up after the tags mint.

Slice 2 opens only once slice 1 has been walked on a local stack.

---

## 7. Corrections to the survey in #415

Recorded rather than dropped, because the reasoning was sound on the evidence available then:

1. **"Expose one derived query the canvas can actually reach" is not needed.** The spike showed
   `Members()` + `Distance()` compose to the answer with no new method on any module. §2.2.
2. **"A self-origin area needs no new wire kind for its first cut"** — withdrawn. It is true that
   Thunderclap prompts for nobody, but Blade Ward has since shipped on `TargetNone` and sharing it
   would overload one value. §2.7.
3. **"Trivial now and invasive after Bane ships"** on the once-for-all damage roll — Bane has
   shipped. The window closed; the divergence is now named and deferred rather than folded in. §2.8.
4. **The `BardCantrips` question was escalated as a content-policy ruling for Kirk.** That was the
   wrong question. We are building capabilities, and the published lists are a supply of proof cases
   rather than a scope to complete. It is an ordinary honest edit. §2.8.
