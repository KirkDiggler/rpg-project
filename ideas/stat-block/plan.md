# Stat blocks as content — implementation plan

**Goal:** an author declares `templates:` in a dungeon yaml, places them as `dnd5e:monsters:<id>`,
and the launched encounter holds monsters whose HP, AC, attacks and passive perception were
derived by the rulebook from the authored block. The studio shows the derived block it is told.

**Non-goals:** cross-dungeon template reuse (R10), trade on a monster (R11), allied flip (R12),
per-placement score overrides (R9), spawn-time hit dice (Open), multiattack spelling (Open),
converting the thirteen constructors (Open).

**Authority:** [design.md](design.md) R1–R12 · [README.md](README.md) · rpg-project#555 ·
design PR rpg-project#556.

**Constraints:**
- Design law C1 (`dungeonspec` never resolves content): the compiler carries a template as
  authored strings and checks shape only. Resolution, including the shadowing refusal of R2,
  happens where `monsters.ByRef` is consulted today: rpg-api `sessionworld.Compile` at
  authoring and `session.instantiate` at launch.
- Toolkit: one nearest-go.mod module per PR; develop on pseudo-versions, merge inside-out,
  walk before merge (`rpg-toolkit/CLAUDE.md`).
- HP = floor(hit dice average) + CON modifier × dice count. This reproduces every existing
  constructor comment (`2d8+2 → 11`, `5d8+10 → 32`).
- AC = armor base + min(DEX modifier, armor `MaxDexBonus`); no armor = 10 + DEX modifier.
- Experience absent = 0 = worth nothing; proficiency absent = 2 (existing `proficiencyBonusOf`).
- Attribution: platform charter signature on every commit and PR; no session or model trailer.

**Inspected baseline (2026-10-10):** rpg-toolkit `main` (modules `rulebooks/dnd5e`,
`rulebooks/dnd5e/encounter`, `rulebooks/dnd5e/session`; `dice` v0.3.2 pinned by dnd5e);
rpg-api `dev` (`internal/sessionworld/sessionworld.go`, `internal/dungeons/registry.go`,
`internal/orchestrators/authoring/orchestrator.go`); rpg-api-protos `main`
(`dnd5e/api/authoring/v1alpha1/service.proto`); rpg-dnd5e-web branch
`feat/1234-studio-doorways` (`src/concepts/encounter-studio/`).

**Sequence:**

```
develop:  T1 dnd5e ─┐
          T2 dungeonspec ─┼─► T3 session ─► T5 rpg-api ─► T6 studio panel
          T4 protos ──────┘                 (T4 bindings)
merge:    T1 → tag · T2 → tag · T3 (repin) → tag · T4 → tag · T5 (repin, pins T4) · T6
walk:     T5 brings the whole stack up on pseudo-versions; the walk scenario below runs before
          any toolkit PR merges (wave walks before it merges).
```

T1, T2 and T4 have no development dependency on each other. T3 needs T1 and T2 locally
(go.work replace, never committed). T5 needs T1–T4. T6 needs T4's bindings and T5 running.

**Open items:** none blocking. Resolved assumption: a template's `base` must be a rulebook
base (today only `human`); a template deriving from another declared template is refused by
name, deferred until an author asks. Resolved assumption: `Compiled.Templates` is a
dungeonspec-owned string struct, not `monster.Template`, so the encounter module gains no new
import and C1 holds by construction.

---

## Task contracts

### T1: Template assembly in the dnd5e rulebook

**Delivers:** R3, R4, R5, R6, R8. `monster.FromTemplate` derives a monster; `monsters.Human`
is the base; AC math is callable outside `character`.
**Owner:** rpg-toolkit module `rulebooks/dnd5e`, packages `monster`, `monster/monsters`,
`armor`.
**Prerequisites:** none. `dice.ParseNotation` + `Pool.Average()` available (dice v0.3.2).
`weapons.UnarmedStrike` exists in `SpecialWeapons`; `weaponattack.Input.AlwaysProficient`
exists.
**Files:**
- create `monster/template.go`: `Template`, `FromTemplate`, `Merge`
- create `monster/template_test.go`
- create `monster/monsters/human.go` + `human_test.go`; modify `monsters/registry.go` to add
  `BaseByRef(ref string) (monster.Template, bool)` beside `ByRef` (bases are not
  constructors; `ByRef("dnd5e:monsters:human")` stays false so a placement cannot spawn a
  bare base without going through a template — see Behavior)
- modify `armor/armor.go`: add `func ArmorClass(a *Armor, dexModifier int) int` (nil armor =
  10 + dex); modify `character/character.go` `calculateArmorAC` / `calculateDexModifier` to
  call it (behavior-preserving; existing character AC tests are the pin)
- modify `monster/monster.go` `Config`: add `Proficiencies []ProficiencyData` and
  `Senses SensesData` so a derived block can set them (today `Config` cannot; animated_armor.go
  documents the gap)

**Interfaces:**
```go
type Template struct {
    Base          *core.Ref             // required; rulebook base
    Name          string                // display name; absent = base's
    Abilities     map[abilities.Ability]int // partial; absent keys = base's
    HitDice       string                // "2d8"; absent = base's
    Armor         *armor.ArmorID        // nil = base's; explicit none is the base's unarmored
    Proficiency   int                   // 0 = base's
    Skills        []skills.Skill        // absent = base's
    Actions       []weapons.WeaponID    // absent = base's; present replaces wholesale
    Experience    int                   // 0 = worth nothing (R8), not inherited
}
type Base struct {            // a rulebook base block with its identity
    Ref      *core.Ref         // dnd5e:monsters:human
    Template Template          // names no Base of its own
}
type FromTemplateInput struct {
    ID       string
    Ref      *core.Ref   // the TEMPLATE's ref (dnd5e:monsters:guard), never the base's
    Template Template    // Template.Base is REQUIRED and must equal Base.Ref
    Base     Base
}
func FromTemplate(in *FromTemplateInput) (*Monster, error)
func (t Template) Merge(base Template) Template
// monsters.BaseByRef(ref string) (monster.Base, bool); monsters.Human is a Base
```
`FromTemplate` is the only derivation site (R6). The caller (T3) looks up `BaseByRef(spec.Base)`
and passes it; `FromTemplate` refuses a nil input, a nil ref, a template with no base, a template
whose named base differs from the base given, a base that names a base, and a merged result that
still lacks hit dice or abilities, naming the field. The pairing is checked inside the assembly,
never left as the caller's promise. The sheet's `Ref` is the template's; `CreatureType` comes
only from `Base.Ref`. `Template.Speed` (zero = base's) rides along so
no derived creature walks 0. Heavy armor applies no DEX modifier in either direction
(`armor.DexContribution` is the one rule; this corrects the character's prior negative-DEX
behavior, named in the PR).

**Behavior:**
- Merge is per field (R4): a stated key wins, an unstated key is the base's. `Experience`
  does not inherit (a captain's worth is authored, a cook's is nothing).
- HP: `dice.ParseNotation(HitDice)` → `floor(Average()) + count × conMod`. Current HP = max.
- AC: `armor.ArmorClass(armorOrNil, dexMod)`.
- Actions: each weapon through the existing `assembleWeapon`/`SetWeapons` path; unarmed strike
  assembled with `AlwaysProficient`.
- Skills: `Proficiencies = [{skill, proficiencyBonus}]` per listed skill. Passive perception =
  10 + wisMod + (proficiencyBonus if perception listed), written to `Senses.PassivePerception`.
- Refusals by name: bad hit dice notation; unknown armor id; unknown weapon id; base missing;
  an ability score outside 1–30.
- `monsters.Human`: name "Human", scores all 10, `1d8`, no armor, `[UnarmedStrike]`,
  proficiency 2, experience 0. `BaseByRef("dnd5e:monsters:human")` returns it.

**Tests** (`monster/template_test.go`, `monsters/human_test.go`):
- `TestFromTemplate_GuardDerivesTheSRDNumbers`: base Human, `{con:12, hitDice:"2d8",
  armor: chain-shirt, skills:[perception], actions:[spear]}` → HP 11, AC 13, spear attack bonus
  +3 (STR 13 +1, proficiency 2) and damage `1d6` with the +1 ability modifier, passive
  perception 12, proficiencies `[{perception, 2}]`.
- `TestFromTemplate_CaptainWithProficiency3`: `{str:15,dex:14,con:14,cha:14, hitDice:"10d8",
  armor: breastplate, proficiency:3, actions:[longsword, javelin]}` → HP 65, AC 16, longsword
  +5 `1d8` flat +2.
- `TestFromTemplate_CookInheritsEverythingButTheKnife`: `{actions:[dagger]}` → HP 4, AC 10,
  dagger +2 `1d4`, experience 0, name "Human".
- `TestFromTemplate_ConOverrideMovesHP`: same template at `con:10` vs `con:14` → HP 9 vs 13
  (the fail-silent hazard R3 guards; this test fails against any stored total).
- `TestFromTemplate_RefusesByName`: table of `hitDice:"2d"`, `armor:"plate-of-nothing"`,
  `actions:["dnd5e:weapons:lightsaber"]`, `str:31`, missing base → error text names the field.
- `TestHuman_IsAssembledByFromTemplate`: `FromTemplate("h", Template{}, Human)` → HP 4, AC 10,
  one action `dnd5e:weapons:unarmed-strike` with +2.
- `TestArmorClass_MatchesCharacterMath` in `armor`: leather+dex 3 → 14; chain shirt+dex 3 → 15
  (cap 2); plate+dex 3 → 18 (cap 0); nil+dex 3 → 13. Existing character AC tests unchanged
  and green are the refactor pin.

- [ ] Write the tests above; `go test ./monster/... ./armor/...` fails on missing symbols.
- [ ] Implement `Template`, `Merge`, `FromTemplate`, `Human`, `BaseByRef`, `ArmorClass`,
      `Config` additions.
- [ ] `go test ./...` in `rulebooks/dnd5e`; `golangci-lint run`; gorelease shows a minor bump.
- [ ] Doc: `monster/doc.go` gains a paragraph "Templates: authored, derived once" citing R3/R6.

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e && go test ./... && golangci-lint run`.
Pre-change: compile failure on `FromTemplate`. Post-change: all green, `-run
TestFromTemplate -v` shows every `=== RUN` above.
**Completion evidence:** PR link, test output, gorelease line, `BaseByRef` and `ByRef` both
answering for `human` (false for `ByRef`).

---

### T2: `templates:` dialect in dungeonspec

**Delivers:** R2 (shape half), R8. The compiler carries templates as strings; a placement may
reference a declared template id.
**Owner:** rpg-toolkit module `rulebooks/dnd5e/encounter`, package `dungeonspec`.
**Prerequisites:** none (no import of T1; C1).
**Files:**
- modify `single_room.go`: `SingleRoomSpec.Templates map[string]TemplateSpec
  \`yaml:"templates,omitempty"\``; new `TemplateSpec`
- modify `single_room_decode.go`: shape checks (strict `KnownFields` already applies; add
  template ref checks beside `monsterValues`)
- modify `compile.go`: `Compiled.Templates map[string]TemplateSpec`
- modify `single_room_compile.go`: copy templates through
- tests: `single_room_compile_test.go`, new fixture pair
  `testdata/world-builder-v4-castle-kitchen.yaml` / `.compiled.json`, decode-only refusal
  fixtures under `testdata/decode-only/`
- doc: `dungeonspec/doc.go` dialect section

**Interfaces:**
```go
type TemplateSpec struct {
    Base        string         `yaml:"base" json:"base"`                 // "dnd5e:monsters:human"
    Name        string         `yaml:"name,omitempty" json:"name,omitempty"`
    Abilities   map[string]int `yaml:"abilities,omitempty" json:"abilities,omitempty"` // str dex con int wis cha
    HitDice     string         `yaml:"hitDice,omitempty" json:"hitDice,omitempty"`
    Armor       string         `yaml:"armor,omitempty" json:"armor,omitempty"`       // "dnd5e:armor:<id>"
    Proficiency int            `yaml:"proficiency,omitempty" json:"proficiency,omitempty"`
    Skills      []string       `yaml:"skills,omitempty" json:"skills,omitempty"`
    Actions     []string       `yaml:"actions,omitempty" json:"actions,omitempty"`   // "dnd5e:weapons:<id>"
    Experience  int            `yaml:"experience,omitempty" json:"experience,omitempty"`
}
```
`Compiled.Templates` keyed by the authored id; a placement's `ref` of
`dnd5e:monsters:<id>` is carried unchanged (as today). T3 and T5 consume this struct.

**Behavior (shape only, C1):**
- `base` required and must parse as `dnd5e:monsters:*`; `armor` must parse as `dnd5e:armor:*`;
  each action as `dnd5e:weapons:*`; ability keys ∈ {str,dex,con,int,wis,cha}, values 1–30;
  `hitDice` must match `NdM` with N,M ≥ 1; `skills` entries lowercase identifiers. Each
  refusal is a `ValidationError` at the yaml path (`templates.guard.hitDice`), in the
  package's own words.
- A template id must be a legal ref id segment; two templates cannot share an id (map
  already enforces); a template id equal to another template's `base` id is refused
  ("templates derive from the rulebook, not from each other").
- A `monsterDeclarations[].ref` naming `dnd5e:monsters:<id>` where `<id>` is a declared
  template is legal and carried verbatim; whether it also names a rulebook monster is **not**
  checked here (C1; T3/T5 refuse that).
- Absence: no `templates:` → `Compiled.Templates` nil; every existing golden unchanged
  (`omitempty`).

**Tests:**
- `TestCastleKitchenCompilesTemplates`: fixture declares guard/captain/cook as in design.md,
  three declarations referencing them → `Compiled.Templates` has 3 entries with the authored
  strings; `Monsters[i].Ref == "dnd5e:monsters:guard"`.
- golden: `TestEveryContentFileCompilesToItsCommittedPicture` picks up the new pair.
- `TestTemplateShapeRefusals` table: `base: dnd5e:weapons:spear` → "must reference monsters";
  `hitDice: "d8"`; `abilities: {luck: 12}`; `abilities: {str: 0}`; `armor: chain-shirt` (bare,
  no ref) → each names its path.
- `TestTemplateCannotDeriveFromATemplate`: `captain: {base: dnd5e:monsters:guard}` with
  `guard` declared → refused by name.
- Existing goldens: zero diffs.

- [ ] Fixtures + tests first; run `go test ./dungeonspec/...` → fails on unknown key
      `templates` (strict decode proves the dialect is new).
- [ ] Implement struct, decode checks, compile copy-through.
- [ ] `go test ./...` in `rulebooks/dnd5e/encounter`; lint; gorelease minor.

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e/encounter && go test ./... && golangci-lint
run`. Expected: new golden committed, old goldens byte-identical (`git diff --stat testdata`
shows only the new pair).
**Completion evidence:** PR, test output, golden diff stat.

---

### T3: Session resolves a monster ref from a template

**Delivers:** R1, R2 (resolution half), R6 at launch. A placement whose ref names a declared
template spawns through `FromTemplate`; shadowing refused.
**Owner:** rpg-toolkit module `rulebooks/dnd5e/session`, `launch.go` + `entities.go`.
**Prerequisites:** T1 and T2 pushed. Preferred local form is a pseudo-version pin of each
pushed provider head (the toolkit's rule: develop on pushed commits); an out-of-repo go.work
replace is the fallback while a provider head is still moving, never committed. Pins move to
tags before merge.
**Files:** modify `launch.go` `resolveLaunchMonsters`; modify `entities.go` `instantiate`;
tests `launch_test.go` or `entities_internal_test.go`; fixture yaml reused from T2 via the
encounter module's testdata or a session testdata copy.

**Interfaces:** exported for rpg-api (R6, one assembly; the api never mirrors the conversion):
```go
type DeriveTemplateInput struct { ID string; Ref string; Spec dungeonspec.TemplateSpec }
type DerivedBlock struct {
    ID, Ref, Name string; HitPoints, ArmorClass, PassivePerception, ProficiencyBonus int
    Abilities map[string]int; Attacks []DerivedAttack; Experience int
}
type DerivedAttack struct { WeaponRef string; AttackBonus int; Damage string }
type DeriveTemplateOutput struct { Block DerivedBlock }
func DeriveTemplate(in *DeriveTemplateInput) (*DeriveTemplateOutput, error)
```
Session is the one package that imports both the dialect and the rulebook, so the
authoring-time derivation (T5) and the launch-time one share one assembly. The output is a
session-owned view read off the assembled monster: session's boundary law admits neither
`monster.Monster` nor an inspected `monster.Data` in an exported signature, and the api maps
the view field-for-field onto the proto. Internal:
`instantiate(id, ref string, actions []string, template *dungeonspec.TemplateSpec)
(*monster.Data, error)`; `resolveLaunchMonsters` passes `dungeon.Templates[idOf(ref)]` when
present. Conversion `TemplateSpec → monster.Template` lives here (one function
`templateOf(spec) (monster.Template, error)`), refusing unknown armor/weapon/skill ids with
`ErrUnknownContent` as `arm()` does today. The call is
`FromTemplate(&FromTemplateInput{ID, Ref: parsedPlacementRef, Template, Base})` with
`Template.Base` set from `spec.Base`, `Base` from `BaseByRef(spec.Base)`, and `Template.Name`
from `spec.Name` or, when absent, the template id: name is identity like the ref, never the
base's. `DerivedAttack.Damage` is notation with the ability modifier folded in (`1d6+1`).

**Behavior:**
- ref resolves to a constructor and no template → today's path.
- ref resolves to a template and no constructor → `monsters.BaseByRef(spec.Base)` (refuse
  unknown base by name) → `FromTemplate(id, templateOf(spec), base)` → optional `arm()` with
  the placement's `actions:` (R9: placement replaces wholesale) → `ToData()`.
- both → refuse: `ErrShadowedRef` "template %q shadows rulebook monster %q; rename the
  template" (fail closed at launch even though T5 refuses it at authoring).
- neither → today's `ErrUnknownContent`.
- `KindMonster`, faction, table, temper, holds: unchanged; the member is indistinguishable
  downstream (R1).

**Tests:**
- `TestLaunch_TemplateGuardSpawnsWithDerivedNumbers`: castle fixture → member `guard-1`
  sheet HP 11 AC 13, one spear action; `cook-1` HP 4 with a dagger.
- `TestLaunch_PlacementActionsReplaceTemplateWeapons`: binding `actions: [dnd5e:weapons:club]`
  on `guard-2` → club only.
- `TestLaunch_TemplateShadowingRulebookIsRefused`: template id `goblin` → `ErrShadowedRef`,
  nothing spawned.
- `TestLaunch_TemplateWithUnknownBaseIsRefused`: `base: dnd5e:monsters:elf` → error names
  `elf`.
- `TestLaunch_TemplateMemberIsAttackableAndTurnsHostile`: party attacks neutral `cook-1`
  (faction `kitchen`, `stance: neutral`) → `ErrNotATarget` is **not** returned; stance beat
  shows `kitchen`/party hostile (aggression law, existing); fight forms. This is the R1 pin.

- [ ] Tests first against T1/T2 via go.work; fail on signature.
- [ ] Implement; `go test ./...`; lint.
- [ ] Repin to T1/T2 tags once minted; `go mod tidy`; CI green.

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e/session && GOWORK=<outside-repo go.work> go
test ./... -run 'TestLaunch_Template' -v` shows the five `=== RUN`; after repin, plain `go
test ./...`.
**Completion evidence:** PR (draft until repinned), test output, go.mod pins.

---

### T4: Derived block on the authoring wire

**Delivers:** R7 (wire half). `PutDungeonResponse` carries one derived block per template.
**Owner:** rpg-api-protos `dnd5e/api/authoring/v1alpha1/service.proto`.
**Prerequisites:** none.
**Files:** modify `service.proto`; generated bindings via CI.
**Interfaces (additive):**
```proto
message PutDungeonResponse {
  repeated FieldError errors = 1;
  dnd5e.session.v1alpha1.GetAtlasResponse atlas = 2;
  repeated DerivedStatBlock templates = 3;   // empty when the yaml declares none or errors are present
}
message DerivedStatBlock {
  string template_id = 1;        // authored id, e.g. "guard"
  string ref = 2;                // "dnd5e:monsters:guard"
  string name = 3;
  int32 hit_points = 4;
  int32 armor_class = 5;
  int32 passive_perception = 6;
  int32 proficiency_bonus = 7;
  map<string, int32> abilities = 8;   // "str".."cha" after merge
  repeated DerivedAttack attacks = 9;
  int32 experience = 10;
}
message DerivedAttack { string weapon_ref = 1; int32 attack_bonus = 2; string damage = 3; }
```
Zero `hit_points` never occurs for a derived block (R8); a template that failed derivation
appears in `errors`, not in `templates`.

- [ ] `buf lint`, `buf format -d`, `buf breaking` against main: additive only.
- [ ] PR ready (protos PRs are not drafts); CI publishes bindings and a tag.

**Verification:** `buf lint && buf breaking --against '.git#branch=main'` clean.
**Completion evidence:** PR, tag, generated-code commit sha (the tag is on the generated
commit, not the merge).

---

### T5: rpg-api resolves templates and echoes the derived block

**Delivers:** R2 (authoring-time refusal), R7. Also the integration deliverable: the whole
stack on pseudo-versions for the walk.
**Owner:** rpg-api `internal/sessionworld/sessionworld.go`, `internal/dungeons/registry.go`,
`internal/orchestrators/authoring/orchestrator.go`, handler `put_dungeon.go`, content
`content/castle-kitchen.yaml` (new reference dungeon for the walk).
**Prerequisites:** T1, T2, T3 (pseudo-versions, then tags), T4 bindings.
**Files:** as above plus tests beside each.
**Interfaces:** `sessionworld.Compile` returns derived blocks alongside the compiled dungeon:
`type Compiled struct { ...; Templates []DerivedStatBlock }` (api-internal type, converted at
the handler). Derivation calls `session.DeriveTemplate` from T3 (R6: one assembly; the api calls it, it
does not reimplement or mirror the string-to-template conversion). The api keeps only the
shadowing check (constructor OR template) and the field-for-field map of
`session.DerivedBlock` onto the proto `DerivedStatBlock`.

**Behavior:**
- Ref check extended: `monsters.ByRef(ref)` or `compiled.Templates[idOf(ref)]`; both → FieldError
  at `templates.<id>` "shadows rulebook monster"; neither → today's "unknown monster".
- Each template is derived at compile; a derivation error becomes a FieldError at
  `templates.<id>.<field>`; success produces a `DerivedStatBlock`.
- `PutDungeon` with `validate_only` returns `templates` populated when `errors` is empty.
- `content/castle-kitchen.yaml`: the walk dungeon — factions `watch` (guards, captain) and
  `kitchen` (two cooks), both `stance: neutral`; `watch` has `until: {fact: alarm-raised}`;
  a `holds:` intel fact on the captain so stealing from him raises the alarm through presence
  transfer; party start in the kitchen.

**Tests:**
- `TestCompile_TemplatesDerive`: castle yaml → 3 derived blocks, guard HP 11 AC 13.
- `TestCompile_TemplateShadowRefused`: template `goblin` → FieldError path `templates.goblin`.
- `TestCompile_TemplateUnknownBaseRefused`: path `templates.guard.base`.
- `TestPutDungeon_ValidateOnlyEchoesTemplates` (handler): response `templates` has 3 entries,
  `errors` empty.
- Existing: all reference dungeons still compile with zero `templates`.

- [ ] Tests first; implement; `make ci-check` and grep the log (exit code lies).
- [ ] Bring the stack up (`game-dev/scripts/dev-env.sh up dev` on this branch) and run the
      walk scenario below; post the `- [ ]` checklist on rpg-project#555.
- [ ] Repin to toolkit tags after T1–T3 merge; `go mod tidy`; CI green; draft → ready.

**Walk scenario (owned here):** PutDungeon the castle yaml; launch; (1) roster shows two guards,
a captain, two cooks, all monster-kind, none hostile; (2) inspect guard-1: HP 11, AC 13,
spear +3; (3) take the alarm prop → `alarm-raised` fact → `watch` turns hostile, guards and
captain form a fight, cooks stay neutral. Each step is one `- [ ]` on #555 with what was seen.
There is no free-roam attack step: the standing ruling (disposition both-ways, 2026-09-22) is
that the ITEM triggers hostility and no attack verb exists off the turn clock; R1's
"attackable and turns hostile" pin is T3's unit test, which authors the turn clock. The alarm
is a holdable prop, not a `holds:` on the captain, because a mind never reveals a record it
already holds.

**Verification:** `cd rpg-api && make ci-check 2>&1 | tee /tmp/ci.log; grep -E 'FAIL|error' /tmp/ci.log`
empty; walk checklist on #555 all checked.
**Completion evidence:** PR, ci log, walk checklist with observations, go.mod pins at tags.

---

### T6: Studio template panel

**Delivers:** R7 (client half). The studio edits `templates:` and shows the echoed block.
**Owner:** rpg-dnd5e-web, lane ui-ux (charter `docs/teams/roles/ui-ux/prompt.md`), branch
based on the studio line (`feat/1234-studio-doorways` or its successor on dev).
**Prerequisites:** T4 bindings published (`@kirkdiggler/rpg-api-protos` bump); T5 running
locally for the echo. The studio does not call PutDungeon today; `usePutDungeonPreview`
(`src/author/DungeonBuilder.tsx`) and `useRoomPublishing` (`WorldBuildingConcept.tsx`) are
the existing validate-only callers to reuse.
**Files:** create `src/concepts/encounter-studio/StudioTemplatePanel.tsx` + test; modify
`studioSession.ts` (`templateEditing` beside `wallEditing`); modify
`src/concepts/world-building/singleRoomDungeon.ts` `encodeSingleRoomDungeon` /
`decodeSingleRoomDungeon` to round-trip `templates:`; modify `EncounterStudioWorkspace.tsx`
to mount the panel; modify `StudioArrangePanel.tsx` so a placement's ref picker lists declared
templates beside the rulebook palette.

**Interfaces:** panel edits produce `templates` in the document; the document encodes to the
T2 yaml shape exactly (field names `base, name, abilities, hitDice, armor, proficiency,
skills, actions, experience`). The echo is read from `PutDungeonResponse.templates` by
`template_id`.

**Behavior:**
- Fields: base (select with the single option `dnd5e:monsters:human`; the list grows with the
  rulebook, and listing bases over the wire is deferred until a second base exists), name, six ability inputs (blank = inherited, shown as the echoed
  merged value), hit dice text, armor select (from the existing armor palette data), skills
  multi-select, weapons (reuse the `actions` weapon picker from `SitePolicies`), experience.
- Derived strip: HP, AC, passive perception, attacks — **only** from the echo; while a
  validate call is in flight or errored, the strip reads "unanswered", never a locally
  computed number (R7 refusal).
- A FieldError at `templates.<id>.<field>` renders inline on that field.
- Placing: the arrange ref picker offers `dnd5e:monsters:<id>` for each declared template.

**Tests (vitest):**
- `StudioTemplatePanel.test.tsx`: editing con to 12 encodes `templates.guard.abilities.con: 12`
  and nothing else; derived strip shows "unanswered" with no response; shows 11/13 given a
  stubbed response; inline error for a stubbed FieldError.
- `singleRoomDungeon.test.ts`: encode/decode round-trip of the castle yaml preserves
  `templates:` byte-for-byte modulo key order.
- `EncounterStudioIntegration.test.tsx`: a declared template appears in the ref picker.

- [ ] Tests first; implement; `npm run lint && npm run typecheck && npm test` (fresh
      worktree: `npm ci` first, tsc false-green trap).
- [ ] Local walk with T5 running: author guard in the panel, see 11/13 appear after the
      validate round trip; place it; screenshot via `node tools/browser/screenshot.mjs`.

**Verification:** commands above; screenshot attached to the PR.
**Completion evidence:** PR, test output, screenshot.

---

## Requirement coverage

| In-scope requirement / scenario | Task(s) | Concrete proof |
|---|---|---|
| R1 combatant NPC is a monster; stance/aggression unchanged | T3, T5 | `TestLaunch_TemplateMemberIsAttackableAndTurnsHostile`; walk steps 3–4 |
| R2 template referenced as monster ref | T2, T3 | `TestCastleKitchenCompilesTemplates` (ref carried); `TestLaunch_TemplateGuardSpawns…` |
| R2 shadowing refused | T3, T5 | `TestLaunch_TemplateShadowingRulebookIsRefused`; `TestCompile_TemplateShadowRefused` |
| R3 author scores/dice, derive totals | T1 | `TestFromTemplate_GuardDerivesTheSRDNumbers`, `…ConOverrideMovesHP` |
| R4 base + per-field merge | T1 | `TestFromTemplate_CookInheritsEverythingButTheKnife` |
| R5 rulebook base `human` | T1 | `TestHuman_IsAssembledByFromTemplate` |
| R6 one assembly | T1, T3, T5 | api and session both call `FromTemplate`; grep shows no second HP/AC formula |
| R7 derived echo; client never computes | T4, T5, T6 | `TestPutDungeon_ValidateOnlyEchoesTemplates`; panel "unanswered" test |
| R8 absence inherits or refuses | T1, T2 | `TestFromTemplate_RefusesByName`; `TestTemplateShapeRefusals`; cook experience 0 |
| R9 placement `actions:` replaces | T3 | `TestLaunch_PlacementActionsReplaceTemplateWeapons` |
| Studio authors a template and places it | T6 | panel tests; integration picker test; screenshot |
| Existing content unchanged | T2, T5 | golden zero-diff; reference dungeons compile |
| Deferred (not covered by design) | — | R10, R11, R12, Open items |

## Provider/consumer seams

| Provider | Consumer | Contract | Availability | Proof |
|---|---|---|---|---|
| T1 `monster.FromTemplate`, `Template`, `monsters.BaseByRef` | T3 `templateOf`, T5 `sessionworld.Compile` | `Template` fields as listed; error text names the field | T1 tag before T3/T5 merge; go.work replace for development | T3/T5 tests compile against T1; repin commit |
| T1 `armor.ArmorClass` | T1 `character`, `FromTemplate` | `(a *Armor, dexMod int) int`, nil = unarmored | same PR | `TestArmorClass_MatchesCharacterMath`; character AC tests unchanged |
| T2 `Compiled.Templates map[string]TemplateSpec` | T3 `resolveLaunchMonsters`, T5 `Compile` | authored strings; refs as `dnd5e:<type>:<id>`; nil when absent | T2 tag | `TestLaunch_TemplateGuardSpawns…` reads it; T5 `TestCompile_TemplatesDerive` |
| T2 placement ref carried verbatim | T3 | `MonsterPlacement.Ref` unchanged | existing | existing goldens |
| dice v0.3.2 `ParseNotation`, `Average` | T1 | `"2d8"` → avg 9.0 | pinned today | T1 HP tests |
| `weapons.UnarmedStrike`, `AlwaysProficient` | T1 Human | exists in `SpecialWeapons`; `GetByID` accepts it | existing | `TestHuman_IsAssembledByFromTemplate` |
| T4 `PutDungeonResponse.templates`, `DerivedStatBlock` | T5 handler, T6 panel | field names/numbers above; empty when errors present | protos tag → go bindings (T5), npm bump (T6) | `TestPutDungeon_ValidateOnlyEchoesTemplates`; panel stub test uses the generated type |
| T5 running stack | T6 walk, #555 checklist | validate_only round trip | dev-env on T5 branch | screenshot; checklist |
| T6 encode `templates:` | T2 decode (strict) | exact key names | T2 merged | `singleRoomDungeon.test.ts` round-trip; a live validate of panel output |

**Shared files / ordering:** T1 touches `character/character.go` (refactor only) and
`monster/monster.go` `Config`; no other task touches the dnd5e module. T3 is the only session
change. T5 and T6 are separate repos. T2's `omitempty` keeps every golden still; the new
golden is the only testdata diff.

**Plan check findings:**
- *Shadowing site moved* from the compiler (design R2 wording "at compile") to the resolver,
  per C1. design.md R2 and the README dungeonspec row are corrected in this PR; the ruling's
  substance (refused, never silent) is unchanged.
- *`Config` cannot set senses or proficiencies* today; T1 adds the fields rather than writing
  through `Data` after construction, so a derived monster is complete at `New`.
- *The studio does not validate today*; T6 inherits the existing validate-only hook rather
  than adding a second caller shape. If the studio line lands its own publish flow first, T6
  reuses that instead; the contract (echo in, "unanswered" otherwise) does not move.
- *AC math lives on Character*; extracting it is in T1 because R6 needs it, and the character
  tests are the behavior pin.
- No task consumes an unbuilt interface: T3 waits on T1+T2, T5 on T1–T4, T6 on T4+T5.
- *Derivation refusals carry no field path* through `session.DeriveTemplate`; the api lands
  them at `templates.<id>` with session's sentence, and only shadowing and an unknown base get
  their own paths. Open, non-blocking: a later round has session name the field (rpgerr meta)
  so the studio can place the error inline.
