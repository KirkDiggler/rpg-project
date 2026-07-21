# Typed Ref Vocabulary — the proto contract mirrors the toolkit

**Status:** Generator foundation shipped (protos#190/#191). Coexistence model, package
granularity, and most of the enum surface remain open — this is still a foundation design, not
a completed migration.
**Context:** Surfaced by the equipment slice (rpg-api-protos#188/#189). Resolves how the v1alpha2
wire represents rulebook content.

## The goal (why we're doing this, not just how)

Build a **typed, composable, extensible** proto foundation that the platform evolves on —
more content, more rulebooks, homebrew — on a solid contract. The wire is a **typed contract**,
not a generic string transport. We are explicitly **rejecting the generic-passthrough model**
(tried before, wrong abstraction): rulebook content — attributes, proficiencies, features,
weapons — is *tightly coupled* to the rulebook, and the contract should say so.

The line that keeps this honest: **thin ≠ generic.** rpg-api stays thin — it never interprets
what `rage` *does* (rules live in the toolkit). But the wire *knows what kind of thing* flows
through it: a `feature` is a feature; a different module would carry `traits`. Categories are
known and typed per module; behavior is not. That is the boundary rule stated precisely —
orchestrate by typed key, never by calculation.

## Principles

1. **Protos mirror the toolkit's packaging.** `core` = the generic `Ref` envelope; the typed
   vocabulary lives under a per-module umbrella (`dnd5e`) structured like
   `rulebooks/dnd5e/{weapons,abilities,features,…}`.
2. **Typed transport.** Known, module-owned vocabulary is a proto **enum** (`weapon.Longsword`),
   not a magic string. Same anti-wild-west discipline the toolkit already has with
   `WeaponID` consts — expressed proto-native so Go *and* TS get typed constants + wire validation.
3. **Closed-world, by choice.** A homebrew item / a second module ships proto enums to match.
   That is the ecosystem model — deliberately chosen over open extensibility.
4. **One source of truth.** The rulebook owns what content exists. Prefer **generating** the
   proto enums *from* the toolkit registry (`weapons.All`, the `refs` package) over hand-keeping
   two lists in sync.
5. **Typed is for vocabulary, not for display.** Enum the *things* (weapons, abilities, slots,
   item kinds). Server-composed *display text* (`stat_line` = "1d8 slashing · versatile") stays a
   string — it's authored prose, not a vocabulary. This is where the earlier "keys" instinct was
   right and where it was wrong: right for display strings, wrong for content identity.

## What shipped: the generator foundation

**[rpg-api-protos#190/#191](https://github.com/KirkDiggler/rpg-api-protos/pull/191)** (merged
2026-07-21) proved principle 4 first, on the two content types the equipment slice needed:

- `tools/refgen/` — an isolated Go module (own go.mod/go.sum, depends on rpg-toolkit) that reads a
  toolkit registry's map keys and reconciles them against the existing generated enum.
  **Append-only numbering**: existing id→number assignments are preserved, new ids get the next
  free number, ids no longer in the registry become `reserved` entries rather than being deleted
  or renumbered — the wire-stability discipline a hand-kept enum would have to remember manually.
- `dnd5e/api/v1alpha2/weapons/weapons.proto` — `enum Weapon`, 38 concrete weapons, 1-to-1 with
  the toolkit's `weapons.All` registry (category placeholders like `simple-weapon` excluded —
  they're choice-requirement consts, not weapons).
- `dnd5e/api/v1alpha2/armor/armor.proto` — `enum Armor`, 13 concrete armors, same pattern.

This is the tooling half of the design — the mechanism now exists to generate a typed enum from
any toolkit registry on demand. What it does **not** yet do is decide where every future enum
lives or which surfaces should switch to using it.

## Decision since: don't build vocabulary ahead of a use

The original design sketched `ItemKind` and `EquipmentSlot` enums alongside Weapon/Armor. **They
were deliberately not generated in this pass.** The equipment slice's routes (protos#188,
rpg-toolkit#812, rpg-api#682) don't need them yet — `Item.kind` and `SlotDef.key`/`slot_keys`
shipped as open strings, per the original CONTRACT.md's explicit ask ("open vocabulary,
server-owned data, not a client enum" — classes/homebrew can add slots without a web release).
Typing them later is a cheap regenerate once `tools/refgen` exists; there was no reason to build
that vocabulary before a concrete consumer needs the closed-world guarantee. This is the same
"lean = what we need, done properly, not speculative" discipline the equipment design applied
throughout.

## Decision since: `Ref`→`core` relocation deferred

The equipment slice (protos#188) surfaced the layering question this doc opened with — `Ref`
only lives in `dnd5e.api.v1alpha2.encounter`, so the new `character` package had to import
`encounter` just to name a `Ref`. A relocation to `api.v1alpha2.core` was built and briefly
merged into the equipment PR, then reverted. The finding that drove the revert, recorded here
for whoever picks up **rpg-api-protos#189** next:

- **The web imports `RefSchema` directly** from the generated `dnd5e/api/v1alpha2/encounter/types_pb`
  (`useSetReactionReady.ts`). Deleting `Ref` from `encounter` — even with a forwarding shim on the
  wire — breaks the web's *build*, not just the wire contract. A clean cut needs the web migrated
  to import from the new location in the same coordinated change, not sprung as a protos-only
  breaking PR.
- **Silent-fallback risk is low.** `encounter.Ref` and a future `core.Ref` would be structurally
  identical (`module`, `type`, `id` — same three string fields). Staying on `encounter.Ref` a
  while longer costs nothing functionally; it's a layering cleanliness question, not a
  correctness one, which is exactly why it was safe to defer rather than force through under
  time pressure.
- **A worked leading-dot gotcha, for whenever #189 lands:** a bare `core.Ref` referenced from
  inside `dnd5e.api.v1alpha2.*` files fails to resolve. Protobuf's relative-scope name resolution
  is greedy — it walks up the current package's parent scopes trying `parent_scope + "." + name`,
  and since `v1alpha2` also exists as a package component under `dnd5e.api.v1alpha2.*`, it locks
  onto a nonexistent `dnd5e.api.v1alpha2.core.Ref` before ever trying the true root package. The
  fix is the fully-qualified `.api.v1alpha2.core.Ref` (leading dot) on every reference; `buf lint`
  names this exact fix in its own error message when the bare form is used.

Net: #189 stays open, framed as a **coordinated protos+web migration** (deprecate `encounter.Ref`,
add `core.Ref`, migrate the web's `RefSchema` import, remove the deprecated one on a later cut) —
not a protos-only relocation.

## Proposed package layout (mirrors the toolkit) — partially realized

```
api/v1alpha2/core/            ← mirrors rpg-toolkit/core                          NOT YET (deferred, #189)
  ref.proto                   message Ref { string module; string type; string id; }  (the envelope)

dnd5e/api/v1alpha2/weapons/   ← SHIPPED (protos#191)
  weapons.proto                enum Weapon { WEAPON_UNSPECIFIED=0; WEAPON_BATTLEAXE=1; … } (38 values)
dnd5e/api/v1alpha2/armor/     ← SHIPPED (protos#191)
  armor.proto                  enum Armor { … } (13 values)
dnd5e/api/v1alpha2/refs/      ← mirrors rpg-toolkit/rulebooks/dnd5e/{abilities,...}         NOT YET
  abilities.proto  skills.proto  features.proto  conditions.proto  spells.proto  (later, same shape)

dnd5e/api/v1alpha2/equipment/ ← domain enums the equipment surface might need    NOT YET (see
  enum ItemKind  { WEAPON, SHIELD, ARMOR, GEAR, … }                              "don't build
  enum EquipmentSlot { MAIN_HAND, OFF_HAND, ARMOR, … }                           ahead of a use" above)
```

Note the shipped Weapon/Armor packages landed as their own top-level `dnd5e/api/v1alpha2/{weapons,armor}`
packages rather than a single consolidated `refs/` package — the granularity question below is
still open; this is what protos#191 happened to pick for the first two, not a settled precedent.

## The key design decision: how `Ref` (envelope) and typed enums coexist

This is the piece to get right deliberately, because it sets the pattern for every future service.
**Still open** — the equipment slice shipped without forcing this call (Item.ref stayed a generic
`Ref`, not a typed Weapon/Armor enum; see Next increment below).

- **Typed enums are the default** for known, module-owned vocabulary a client references
  (an equipped weapon, a chosen ability). `weapon.Longsword` — Kirk's "easy switch."
- **The generic `Ref` envelope** is for the genuinely opaque-orchestration cases where the api
  forwards a toolkit-native ref and neither api nor client switches on the specific value —
  e.g. the `available_actions` two-level namespace, `status_effects` — or where a typed enum
  doesn't exist yet. `Ref` is the escape hatch and the cross-module envelope, not the default.

**Open question (yours):** do we go *fully typed* (enum wherever a category exists, `Ref` only for
the envelope / cross-module / not-yet-typed), or keep `Ref` as the default and add enums where a
client demonstrably switches? Your steer is typed transport, so the lean is **fully typed, `Ref` as the
deliberate exception** — but this is the call to make consciously since it defines the whole surface.

## Next increment: the typed-`Item` rebase

Now that Weapon/Armor enums exist (protos#191), the next natural step is rebasing the equipment
slice's `Item.ref` (currently a generic `Ref`) onto them where the referenced content is actually
a weapon or armor piece — `Item.kind` and `SlotDef.key`/`slot_keys` stay open strings per the
"don't build vocabulary ahead of a use" decision above, unless a concrete need for a closed-world
guarantee shows up on those specifically. This rebase is where the coexistence-model open question
gets forced: an `Item` can reference a weapon, a shield, an armor piece, or unenumerated "gear" —
so it's the first real test of "fully typed with `Ref` as exception" vs "`Ref`-default."

## Open questions to settle together (deliberately)

1. **Coexistence model** — fully-typed vs `Ref`-default (above). Defines the whole surface. Still open.
2. **Package granularity** — one `dnd5e/…/refs` package, or per-category packages (weapons, armor
   shipped as separate top-level packages — is that the pattern, or a one-off)?
3. **Generation** — the mechanism now exists (`tools/refgen`); the remaining question is which
   future categories (abilities, skills, features, conditions, spells) get generated next and when.
4. **`icon_key` / asset refs** — typed asset-reference, or stays a key? (Assets are their own module-ish.)
5. **v1alpha1 coexistence** — v1alpha1 keeps its string/enum surface (deprecate-don't-delete); the
   typed model is v1alpha2-forward only. Unchanged.
6. **The `type` field on `Ref`** — with typed enums carrying category *and* value, does the envelope's
   `type` string stay (for the opaque cases) or get reconsidered? Ties into the `core.Ref` relocation
   (#189) — worth deciding together rather than separately.

## What this is not

Not a reversal of the boundary rule — the toolkit still owns all rules; the web still renders
server-composed display. This types the *identity* of content on the wire so the platform is
composable and extensible, without the api ever learning what any given ref *does*.
