# Behavior regimes — named, reusable, parameterized `on:` blocks at the site root

**Status:** IDEA — not yet ruled. Written 2026-09-22 from a World-Builder walk
(rpg-dnd5e-web#1192) where the missing piece surfaced as a real authoring trap.
Builds on the shipped creature-table design (`ideas/creature-table/design.md`,
SHIPPED 2026-09-18). Nothing here is a proposal yet; this is the thing to be
ruled on.

## The lens

> "we are not here to make things work, we are here to build composeable,
> extensible components that we can build a solid foundation for our game to
> evolve on… we are not making independent features, we are architecting this
> thing so it can do a full DND implementation."

This is read through that. The ask is not "make a monster that attacks back"; it
is "give the author the primitive by which a site's monsters behave *as a kind*
and a *site author* can re-skin them."

## The one sentence

A behavior **regime** is a named, authored, parameterized block of answer-table
entries that lives at the site root, is referenced by a creature or faction
*rather than pasted*, and expands the way point-wise templates do — so "guard",
"patrol", "sentinel" and a monster's kind-default are all the same kind of thing,
and overriding one in an encounter is a reference change, not a rewrite.

## Where it came from (the walk that formed it)

Authoring the attacked-changes-target case in the World Builder, Kirk hit the
trap that opens this idea:

- He authored a single `time:` row — `{ when: { attacked: { within: 3 } },
  attack: attacker }` — on a creature.
- The creature **said its line but never approached or attacked.**
- Not a toolkit bug. Layers are **wholesale, key by key**: writing `on.time`
  replaced the kind's **whole default `time` list**, which carries the
  `enemy: seen -> toward` row that closes distance. The author deleted it
  without meaning to, because the default was **invisible** in the builder.
- Kirk's correction, verbatim thinking: *"monsters have a default behavior
  around the time node. We will also want to create a time block that can be
  reused. Guard, patrol etc... these tables should be able to be stored and
  referenced. I think the storage of them will be its own repository but we can
  store them on the encounter for now. Like a patrol would need locations to
  patrol between, pause timings. Maybe those become templates to fill in. The
  when list will also grow."*

The trap is the design's chance: the reason the default was invisible is that
there is no *named* thing to show or reference — it is an anonymous constant
behind every ref. Naming it, and letting a site reference and override it, is
the primitive this opens.

## What a regime is

**A regime is a named block of answer-table entries plus its parameters.** Two
examples that are NOT expressible today, chosen because they are the test.

**THESE ARE STRAWS, NOT GRAMMAR.** The words `enemy-route` and bare `patrol` do
not exist and are not proposed here — no outcome word is being asked for. They
stand in for *"the row needs an input the table cannot state on its own,"* which
is the whole point. The real spelling is a later, separate design; what this one
asks is whether the shape must *have room* for such inputs.

```yaml
# A guard: stand over this, watch the door, and hold still if the party just
# walks past — but a sentinel that has seen a fight starts to pace.
regimes:
  guard:
    on:
      time:
        - { when: { enemy: seen }, toward: enemy }
        - { when: { enemy: reach }, attack: enemy }
        - { when: { enemy: none },  hold: {} }
    params:
      anchor: { prop: barracks-door }   # stand near this placed thing
      pause: 2                          # rounds of nothing before re-acting

  patrol:
    on:
      time:
        - { when: { enemy: seen }, enemy-route },  # route is a param
        - { when: { enemy: reach }, attack: enemy }
        - { when: { enemy: none },  patrol }       # patrol is a param
    params:
      route: [ { q: 0, r: 0 }, { q: 3, r: -2 }, { q: 0, r: 0 } ]
      pause: 1
```

The point of showing these **is not the exact syntax**. It is that guard and
patrol cannot be a flat `map[string][]AnswerEntry` — patrol needs a *route* and
a *pause*, and a pause is **not a new `when` word**; it is a parameter that
changes how the table is *expanded*, not what row fires. So the regime shape
must allow a **parameter list** or it forecloses the exact behavior Kirk named
as the unlock.

## The default becomes a named regime

Today `monster/table/default.go` holds one `generic` table behind every ref.
Under this shape it is simply **the unnamed regime** — the one every creature
answers with when nothing names another. The builder can then offer, honestly:

> This creature answers by its kind's default regime. Give it a regime of its
> own and you REPLACE the default's rows (wholesale) — use "start from this
> kind's regime" to keep the closing rows.

## Site-root residence and reference — mirror `intel`/`factions`

A regime is declared at the root (like `intel[]` is) and *referenced* by a
creature or faction (like `holds` references a record):

- `factions[].` and `monsterBindings[id].` gain something like
  `"regime": <id>` — or a placement may author its own inline `on:` and that
  is the override.
- **Wholesale remains the law** — but now it is *visible*, because "this
  creature overrides regime X" names the thing being replaced instead of
  silently deleting anonymous bytes.

**"Time" is a bad root key.** "time" conflates *the moment a creature acts* with
*a named behavior regime.* The site root should have a regime key whose name says
"behavior," not "when" — `regimes:` or `behaviors:` — and `on.time` stays what a
single entry itself is *triggered* by. Naming the root key poorly now would bake
the wrong seam in.

## What is NOT decided, and why this is an IDEA

- **Flat reuse vs parameterized template.** The split above assumes parameters
  (route/pause) are first-class. If they are, the earlier "reuse named table
  bytes" framing is wrong and the primitive is a point-evaluated template. This
  is THE ruling this doc is opened to get.
- **Repository vs on-encounter.** Kirk: storage is eventually its own repo; keep
  it on the encounter (site document) for now. Defer repo, lean into encounter.
- **The loader/embedding question.** Earlier I raised whether rules content
  should move to configuration (an `embed`/content-source primitive). That is
  **deferred** — it is a toolkit-wide question (stat blocks are code too), and
  this slice is the *encounter-side* regime primitive, not the storage one.
- **The `when` vocabulary grows** (Kirk: "will grow as we want more complex
  behaviors"). A patrol pause is not a `when`; it is a parameter. Keep those two
  growth paths separate: `when` extends **conditions**; a regime's `params`
  extend **what a behavior needs as input.**

## What has already been learned on the way

- The wholesale-override rule is invisible because the default table is
  anonymous. Naming regimes is the fix, not a warning label.
- A builder that offers "start from this kind's regime" is composition, not a
  band-aid: it is the site holding a reference, exactly as it holds `intel`.
- A patrol that needs a route is proof a flat named table is not enough.

## Done-when (for the eventual design, after ruling)

- A site can declare named regimes at the root and reference one on a creature
  or faction, overriding wholesale but *visibly*.
- The kind's default is a named no-name regime the builder can show and offer
  to edit from.
- A regime can carry parameters (at least route + pause) and the encounter
  expands them; patrol and guard are demonstrable end to end.
- The root key is named for behavior, not conflated with the `time` trigger.

## Ownership

Started by the ui/ux lane from the World-Builder walk. The encounter/engine
side is rpg-toolkit's; the site document shape crosses rpg-dnd5e-web,
rpg-api-protos and rpg-toolkit. Reviewable as an rpg-project design PR before
any code, per the cross-repo design rule.

## The question to rule on

**Is a behavior regime a parameterized template (route/pause as first-class
input the encounter expands), or flat named table bytes?** The former unlocks
patrol/guard; the latter is a smaller slice but forecloses them. Everything else
in this idea (site-root, reference-by-id, wholesale-but-visible, default-as-
named-regime, defer repo/loader) holds either way.