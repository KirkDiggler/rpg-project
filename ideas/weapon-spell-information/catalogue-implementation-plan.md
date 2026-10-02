# Catalogue implementation plan

Tracking: [#520](https://github.com/KirkDiggler/rpg-project/issues/520), design
[PR #521](https://github.com/KirkDiggler/rpg-project/pull/521), R11 catalogue-first.

The catalogue increment explains weapons/spells during selection and afterwards,
including unchosen alternatives. It does not calculate live effect contributions,
change affordability, implement new spells or redesign the combat log.

## Branches and checked starting points

Implementation branches are `feat/weapon-spell-catalogue`, each in its repository's
`.worktrees/weapon-spell-catalogue`:

| Repository | Base | Scope |
|---|---|---|
| rpg-toolkit | `26664812` (`origin/main`) | Root `rulebooks/dnd5e` module only; nested resolution/session/encounter excluded |
| rpg-api-protos | `e80efe0` (`origin/main`) | Catalogue contract only |
| rpg-api | `fa1a0779` (`origin/dev`) | Catalogue access and mapping; no game rules |
| rpg-dnd5e-web | `9cfd4ca6` (`origin/dev`) | Catalogue reads and information display; no mechanics derivation |

The two toolkit commits since reconnaissance change nested encounter/session
modules, not the root spell catalogue. Fresh instructions and production spell
read/compile paths were inspected. No consumer currently observes edits to this
new worktree without an explicit provider adoption.

## First provider checkpoint — complete and unify existing spell metadata

This is a bounded, implementation-ready root-module checkpoint using the existing
public catalogue APIs. It is not the complete catalogue increment or a claim that
R5's richer transport is settled.

### Requirement and evidence

Every executable spell must have a readable catalogue identity and description.
Catalogue names and executable action names must share one authority. Reading a
catalogue entry does not require a character or calling CastDefinition with dummy
values.

Current source:

- `spells/data.go`: public metadata catalogue and level reads.
- `spells/cast.go`: executable content table with separate names; six entries
  lack `SpellData` rows.
- `spells/types.go`: legacy name/description accessors with overlapping authored
  text and some names for entries outside the metadata catalogue.
- `spells/data_test.go`, `cast_test.go`: current behaviour anchors.

### Files and changes

1. Add `spells/catalog_coverage_test.go` using the testify suite pattern. Demonstrate
   the six missing executable entries through GetData/GetSpellsByLevel and assert
   identity, nonempty prose and level agreement with executable content.
2. Populate those six entries in `spells/data.go`: Blade Ward, Faerie Fire, Fog
   Cloud, Thunderclap, True Strike, Vicious Mockery. Author descriptions against
   this build's cast/effect behaviour, not a promise of absent rules.
3. Have executable spell names read the metadata owner rather than carry a second
   name field in `castContent`. Preserve casting costs, profiles, save DC binding,
   targeting, implementation/selection eligibility and mutable state.
4. Make legacy Name/Description reads prefer the canonical metadata. Preserve
   currently supported names outside the metadata catalogue rather than silently
   dropping them. Remove duplicated authored fallback entries where the canonical
   catalogue now owns the answer; do not create a third lookup table.
5. Document the catalogue authority and run the targeted regression, full spells
   package, and relevant character/choice tests. Capture provider readiness and
   independent review separately before calling the provider merge-ready.

### Acceptance

- All executable table entries resolve to catalogue entries and level listings.
- Their displayed name agrees with CastDefinition through the same source.
- Legacy accessors return the canonical description/name for catalogue entries.
- Unsupported/unknown lookups keep their established behaviour.
- Catalogue-only content is not made executable; existing spell choices and
  affordability rules do not widen.
- No character, dummy DC, roller, bus or session is needed by metadata reads.

### Commands

From the root dnd5e module (not the repository root):

```sh
go test -mod=readonly ./spells -run TestCatalogCoverageSuite -count=1
go test -mod=readonly ./spells ./character/choices ./character -count=1
```

Use offline dependencies when available. Run the owning-module/repository gates
required by its instructions at the PR readiness boundary; never bypass hooks.
The initial red test and subsequent green result are separate evidence.

### First checkpoint evidence

Published draft: [rpg-toolkit#1927](https://github.com/KirkDiggler/rpg-toolkit/pull/1927)
at `9ba2c222`. The first metadata checkpoint is implemented; the root-module PR
remains draft for the subsequent catalogue work and required independent review.

- The new catalogue suite was run red before the fix: all six missing executable
  entries failed, and existing Name/Description accessors disagreed with catalogue
  metadata for several other spells.
- The suite is now green, including executable-definition validation/name parity,
  unknown lookup behaviour and preservation of name-only identifiers outside the
  catalogue. Overlap between those fallback labels and the catalogue is rejected
  by the regression.
- `go test -mod=readonly -race ./spells ./character/choices ./character -count=1`:
  PASS, then the full root module `go test -mod=readonly -race ./... -count=1`: PASS.
  Log: `/tmp/weapon-spell-catalogue-root-tests.log`.
- Changed-package lint: PASS (`0 issues`) using the repository-pinned golangci-lint
  v2.3.1 installed into `/tmp/weapon-spell-catalogue-tools`, with cached Go 1.24.6.
  The ambient v1 linter rejected the v2 config, and the pinned v2 binary built
  with Go 1.24 panicked against ambient Go 1.26 standard-library sources. No repo
  pin or global tool was changed; the compatible local invocation is explicit.
  Log: `/tmp/weapon-spell-catalogue-spells-lint-go124.log`.
- Pre-commit ran uncached with the pinned linter/compatible cached Go toolchain:
  formatting, tidy, changed-package lint and race tests all passed. No hook was
  bypassed and go.mod/go.sum are unchanged.
- No API/proto/UI adoption, browser proof or independent review is claimed by
  this provider checkpoint. New structured base-mechanics projection remains
  the next checkpoint; metadata coverage is not the whole increment.

## Following checkpoints — specify before their source changes

1. **Actor-free base mechanics:** expose structured catalogue facts from canonical
   authored content. The cast table currently stores most mechanics inside
   caster-parameterized builders; do not call them with a fictional DC merely to
   render metadata. Establish the smallest content-owned projection and any
   necessary source reshaping, without entering the resolution-contribution work.
2. **Ref-complete read and wire contract:** supply the exact choices/grants and
   later catalogue inspection without the web guessing levels 0 and 1. Name the
   concrete endpoint/types, unknown-ref and implementation-status behaviour, and
   adapter tests before editing the public contract. Preserve selection refs and
   existing affordability authority.
3. **API adoption:** pin the provider, map its values without rebuilding rules,
   test missing/present fields and stable refs. Generated proto bindings remain
   CI-owned and require the operator-controlled proto merge/release before use.
4. **UI consumption:** reuse the equipment detail experience where possible;
   display spell descriptions/base facts and permit inspection after choosing.
   Never modify selections from an inspect action; show errors rather than
   silently making missing descriptions look complete.
5. **Integrated proof:** normal character creation, finalization/reload and
   inspection of both selected and unselected alternatives; unchanged command
   refusals; no fabricated live-character bonuses. Toolkit, adapter and browser
   evidence must each be recorded against exact versions.

Develop consumer requirements outside-in; publish/adopt providers inside-out,
with one PR per nearest toolkit Go module and released pins before consumer
merge. The first root-module checkpoint can proceed without waiting for the new
wire shape because its public APIs and gameplay behaviour remain unchanged.

## Remaining decisions and boundaries

The richer catalogue projection and exact public transport remain to be checked
against R5. They do not block the bounded metadata checkpoint above, and that
checkpoint must not be mistaken for completing them. Bring back an actual change
of ownership, public semantics or gameplay eligibility rather than asking the
operator to approve ordinary file choices.

No source changes are authorized in resolution, encounter, contribution
arithmetic, action permissions or the combat log by this catalogue plan.
