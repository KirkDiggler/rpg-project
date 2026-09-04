# Recover the artifact — plan

**Status:** RULED 2026-09-04 (design.md R1–R10). Branch cut 2026-09-04. Builds design §3–§6 across
four repos, panel-back: the proto wave merges first; toolkit, rpg-api and
web build against pins in parallel; Kirk walks once; merge bottom-up.
**Journey:** rpg-project#326, slice 2. **Record:** rpg-project#368 stays
open through the build and merges on done-when (delegated).

## Simplifications found in the ground truth

1. **Holdings unify loot, hold and the parchment shelf.** One fact kind
   (`holds:<id>`), one transfer routine, two verbs. Loot is a second
   caller of `learnDoor` (encounter/conceal.go), cause `loot`.
2. **Endings come from scenarios** (R8). `sessionworld.endingsFor` today
   declares `withdrawn` (External, fired by the lobby's abandon) and
   `boss-down` from the authored flag; this slice appends every ending
   each bound scenario declares and leaves the flag arm alone. The
   **follow-up** (kill-the-captain) converts the reference tomb and
   deletes the flag. One new trigger arm, one `data.go` case. No goal
   engine.
3. **Exits reuse `start`'s shape**: an id and a floor cell.
4. **A taken prop is a projection fact, not a mutation.** The atlas stays
   construction-truth (its own doc: cached once). `AtlasFor` drops props
   with a `taken` fact — the move concealment already makes for doors —
   and the `taken` beat patches the client's held atlas the way
   `applyReveal` patches it today.
5. **`Exit` already records where the member stood** (canvas.go: "records
   where they stood on the way out"), which is the exit-cell check.

## Board filing (cut 2026-09-04)

Sub-issues of journey rpg-project#326, Kind=Build: rpg-api-protos#288
(wave 0); rpg-toolkit#1495 (encounter module, wave 1 A+B) and
rpg-toolkit#1496 (session module, wave 1 C); rpg-api#913 (wave 2);
rpg-dnd5e-web#921 (wave 3). Team and Initiative are blank on all five
because the journey carries neither — set them on #326 first if these
should surface in the Team or Current Initiative views. Each PR body: purpose, acceptance table,
evidence, every Copilot thread answered with the stance at decision time.
Ready PRs, never drafts. Findings outside this slice → `discovered`
issues on the owning repo, never fixed in passing.

## Wave 0 — protos (the contract; merges first)

Session service, `dnd5e/api/session/v1alpha1`:

- `rpc Loot(LootRequest) returns (LootResponse)` —
  `LootRequest{session, member, target, range}` mirroring the SDK input
  field for field (this service's rule 1); the response carries the
  seam's standard `saved` and `delivery` reports and nothing about what
  moved (design Q1, closed at wave 0).
- `rpc Hold(HoldRequest) returns (HoldResponse)` —
  `HoldRequest{session, member, target, range}` where `target` is the
  placement id of the prop (mirrors `HoldInput.Target`); response as
  Loot's. (Shipped as `Take` in #289; renamed by the wave-0 follow-up
  after R10.)
- `ListScenarios` is ungated: reading content mutates nothing
  (GetDungeon's precedent); PutDungeon stays behind its flag. rpg-api
  honours this at wave 2.
- `Exit` request unchanged. `Exited` body gains `repeated string holding`
  — the prop ids carried out — and `string exit` (the exit id, empty for
  a departure from elsewhere).
- New event kinds with typed bodies: `LOOTED` → `Looted{looter, body}`;
  `HELD` → `Held{holder, prop}` (was `TAKEN`/`Taken` in #289); `DROPPED` → `Dropped{member, prop,
  Position at}`. `Ended` unchanged; the key is the scenario's declared
  ending key.
- `AtlasProp` gains `string id` (empty when the author named none).
  `GetAtlas` omits held props for everyone; a dropped prop appears at
  its drop cell.
- **Wave-0 follow-up PR (after R10 and the web build's findings):**
  rename `Take`→`Hold`, `TAKEN`→`HELD`, `Taken`→`Held{holder, prop}`;
  `AtlasProp.holdable` (bool); `GetAtlasResponse.exits` (repeated
  `AtlasExit{id, at}`). Leave is offered everywhere; exits are for
  drawing the way out, not gating.

Authoring service, `dnd5e/api/authoring/v1alpha1`:

- `rpc ListScenarios(ListScenariosRequest) returns (ListScenariosResponse)`
  — `repeated ScenarioDescriptor{id, name, repeated ScenarioField}`;
  `ScenarioField{key, label, FieldType type, string kind, guidance}` with
  `FieldType{ENTITY_REF, CHECK}` and `kind` the open string `prop | exit |
  door | monster` (open vocabulary, deliberately not an enum — the
  descriptor is content).
- `PutDungeon` ships verbatim YAML: **no proto change** for `id`, `knows`,
  `holdable`, `exits`, `scenarios`. Refusals travel as they do today.

Evidence: `buf lint`, `buf format`, `buf breaking`, generate compiling. No
hand-written tests. Consumers pin the `generated` branch SHA, never the
v0.1.x tag.

## Wave 1 — toolkit (the engine)

One in-flight PR per module; A before B in the encounter module; C after
A's tag in the session module. Tags: the auto-tagger mints on merge — read
the tag on the merge SHA, never predict it.

### PR A — encounter module: dungeonspec, holdings, verbs, trigger

dungeonspec (`rulebooks/dnd5e/encounter/dungeonspec`):

- `PlaceSpec` gains `ID string`, `Knows []string` (door ids; monsters
  only), `Holdable *bool` (props only). `Spec` gains `Exits []ExitSpec{ID,
  At [2]int}` and `Scenario map[string]map[string]string`, carried
  opaquely: dungeonspec validates only that every binding names an id
  that exists (placement or exit). Refusals per design §3.3, each naming
  the line. `Compiled` exposes `Exits`, `Scenario`, and ids/`Knows`/
  `Holdable` on its placements.
- `PlaceSpec.Boss` **untouched** (R8: the follow-up retires it).
- Fixture: `reference-tomb.yaml` is **unchanged**. New
  `reference-tomb-heirloom.yaml` = the tomb plus a concealed vault, a
  holdable artifact with an id, `knows: [vault-door]` on the captain (no
  boss flag), `exits: [{id: entrance, at: <start>}]`, and
  `scenarios: {recover-the-artifact: {artifact, exit}}`. Copied
  byte-identical into rpg-api's content and the web Concepts Lab, each
  copy pinned by a test.

encounter:

- `FieldInput.Exits`; `PropInput.ID`, `.Holdable`; `MemberInput.Knows`
  seeded at setup as `holds:intel:door:<id>` facts on the monster —
  engine-internal, **never projected** (design P3: no wire ever says who
  carries intel).
- Fact kinds `holds:<id>`, `held:<prop>`, `dropped:<prop>@<cell>`.
- `Loot(LootInput{Member, Target, Range})` and `Hold(HoldInput{Member,
  Target, Range})` rule halves, validation orders as design §4.2/§4.3;
  the transfer routine moves every holding of the body to the looter —
  intel becomes `learnDoor(looter, door, "loot")` + the looter's
  DOOR_REVEALED; a prop becomes the looter's `holds`. Beats `looted`
  (looter, body — nothing of what moved) and `held` (holder, prop) to
  everyone present.
- In-combat rule (design §4.4): refused while the member's fight is on
  the turn clock and it is not their turn; free on their turn. Mirrors
  how Move refuses out of turn.
- `AtlasFor` drops props with a `held` fact and places dropped ones.
- `TriggerExitedHolding{Exit ExitID, Item PropID}`; `Exit` evaluates it
  after the departure beat when the member stood on the exit's cell and
  holds the item; a departure from any other cell while holding drops
  the holding at that cell (`dropped` beat). `data.go` gains the
  `exited_holding` arm. `ErrNoEnding` backstop unchanged.
- Tests: one scene per design §8 row; the secrecy scene (kill, never
  loot: the door stays a wall for everyone); mutation pass on the
  transfer routine, the exit-cell check and the trigger arm — a
  surviving mutant is a comment claiming a guarantee the test does not
  give.

### PR B — encounter module: `encounter/scenarios`

- `Descriptor()` per scenario; `All()` registry (what `ListScenarios`
  serves); `New(cfg, compiled) (Declared, error)` per scenario, where
  `Declared{Endings []EndingInput, ...bound ids}`.
- **One scenario now**: `recovertheartifact` (refusals: missing artifact,
  artifact not holdable, missing exit). Form-filler words. Nothing
  defaulted. `killthecaptain` is the follow-up's first file in this
  package.
- Pinning test: descriptor fields ↔ `Config` struct, both ways, run over
  `All()` so the second scenario cannot skip it.

### PR C — session module

- `SpawnInput.Knows` forwarded to `JoinInput.Knows` (the authored
  knowledge rides the spawn like the placement's other facts);
- `Manager.Loot`, `Manager.Hold` beside `Search`; `Exit` surfaces the
  fired ending through the existing `ExitOutput.Closed`; `kindFor` arms
  `looted`, `held`, `dropped`; typed event bodies; `AtlasOf` inherits
  the held filter by construction (it calls `AtlasFor`), but `exits`,
  `holdable` and the prop `id` are carried explicitly through the
  session's field-for-field copy — the plan's "inherit" was wrong for
  those three (session build, 2026-09-04).
- Scenes: two members, one carries, the other leaves first; carrier
  leaves from the vault (drop); carrier leaves at the exit (ended, names
  the carrier).

## Wave 2 — rpg-api (the translator)

- Pins: protos `generated` SHA; encounter and session tags. The toolkit
  override cannot target `rulebooks/dnd5e/encounter` (rpg-api#902) —
  pseudo-version pins in a walk-only worktree until tags exist.
- Handlers: `Loot`, `Hold` (session service) and `ListScenarios`
  (authoring service) — verbatim translation. Error map: the new refusals
  arrive as `FAILED_PRECONDITION` carrying the refusal text.
- `sessionworld`: at run start, `scenarios.New(cfg, compiled)` for every
  bound scenario; `endingsFor` = `withdrawn` + the flag arm (unchanged) +
  their endings; `Exits`, `Knows`, `Holdable`, ids flow from `Compiled`
  into setup.
- Content: the heirloom fixture **byte-identical to the toolkit's**, put on
  the box through the builder or the seed for the walk. No migration of
  the seeded tomb in this slice — that gate belongs to the follow-up that
  deletes the flag.
- Evidence: integration scene for both paths and the secrecy negative;
  `make ci-check` green (rpg-api#901: it destroys uncommitted work — commit
  first).

## Wave 3 — web (the proof)

Builder:

- Exits palette: a marker placed like `start`, with an id.
- The boss toggle on monsters **stays** in this slice; the follow-up
  removes it when the scenario panel takes over that fact.
- Placement id: an author field on props and monsters; the panel suggests
  a slug from the ref, the author may rename.
- `knows` on a monster: a multi-pick over the dungeon's doors, by id.
- `holdable` toggle on a prop; toggling on requires an id and says so.
- Scenario panel: `ListScenarios` → one form per scenario; the
  `entity_ref(prop)` picker lists holdable props by id, `entity_ref(exit)`
  lists exits; `PutDungeon` refusals render inline on the field they name.
- YAML emit/parse round-trip byte-exact for every new field; the Concepts
  Lab fixture stays a verbatim copy of the toolkit fixture (a test pins
  the bytes).

Game:

- **Loot on every downed body in range** — never only the captain (design
  P3). Hold on an adjacent prop whose `AtlasProp.holdable` is true. **Leave** at an exit cell calls
  `Exit`; the server decides what it means.
- Beats as statements: "Aldric looted the skeleton captain", "Aldric holds
  the heirloom", "Aldric left through the entrance with the heirloom",
  "Aldric dropped the heirloom". No beat says "interacted" or "took".
- `held` and `dropped` patch the held atlas (applyReveal precedent); the
  refetch lands afterwards with the server's answer. The `ended` overlay
  names the carrier.
- Evidence: vitest per apply function and per picker, **the whole test
  file run** (shared hoisted mocks); headless screenshots of the form,
  both paths and the drop.

## The walk — Kirk, once, two players, local stack

1. **Path 2.** Fight the captain; kill; one player Loots → DOOR_REVEALED to
   them alone, the other's map unchanged; open → both see the vault; Hold
   → the prop vanishes for both; carrier walks to the exit → Leave → the
   run ends for both; the overlay names the carrier.
2. **Path 1**, fresh run: Search finds the door; the same from there.
3. **Negatives.** The non-carrier Leaves first → departs, the run goes on.
   Kill and never loot → the door stays a wall for everyone. Carrier
   Leaves from the vault → the heirloom lies where they stood (R9); the
   other player Takes it and finishes.

## Follow-up, named (R8)

**kill-the-captain**: the second scenario (`boss: entity_ref(monster)` →
`TriggerMemberDown`), the reference tomb bound to it, `boss:` deleted and
refused by name, the flag arm removed from `endingsFor`, the builder's
boss toggle removed, and the seeded tomb re-put before dev→main. Filed
when this slice's walk is done, not before.

## Done-when

In the live game, two players, on a dungeon authored through the builder's
scenario form: the party recovers the artifact by either path and the run
ends naming the carrier; a party that kills the captain and never loots
finishes blind; every beat reads as a statement. Then rpg-project#368
merges.

## Sequence

| step | repo / module | waits on |
|---|---|---|
| 0 | rpg-api-protos: Loot, Take, ListScenarios, bodies, `AtlasProp.id` | — |
| 1A | toolkit encounter: dungeonspec + holdings + verbs + trigger | the design only — the toolkit imports no protos; runs in parallel with 0 |
| 1B | toolkit encounter: `scenarios` package | 1A merged |
| 1C | toolkit session: entries + event kinds | 1A tag |
| 2 | rpg-api: pins, handlers, sessionworld, content | 0 (generated SHA) + 1B, 1C tags (pseudo-versions to walk early) — the first wave that needs both |
| 3 | rpg-dnd5e-web: builder + game | 0 pinned; walks against 2 |
| walk | local stack from the wave-2 and wave-3 worktrees | 2, 3 CI green |
| merge | bottom-up: toolkit → api → web → #368 | Kirk's walk |

## Retro — the rule for the next slice (Kirk, 2026-09-04, end of the build)

This slice opened as five PRs and closed at roughly thirteen: a wire rename,
a refusal-text fix, join-time knowledge on two modules, the exited-beat
truth, a test-only follow-up, an api re-pin. Every one surfaced when the
NEXT wave integrated — the api launch path, its acceptance scenes — or when a
naming ruling landed after the first PR had opened. Each cost a PR, a Copilot
round, a tag and a downstream re-pin; the toolkit's monthly review quota was
spent by the end.

Kirk: "these toolkit things could be found locally… we have the local stack
that can be built from pseudo toolkit versions that are pushed up. we would
have had significantly less PRs and be able to iterate faster with less churn
overall." Rule: protos first; then build every wave on PUSHED branches and
pin them as pseudo-versions; stand the local stack up from the worktrees, run
the api scenes and walk it; fold the findings back on the branches; THEN open
one bounded PR per module, in dependency order, on real tags as they mint.

Sharpened the same hour, Kirk: "why not merge the toolkit code — in the end
api will need the bump. bumping it for every change seems silly." Toolkit PRs
merge as they come; the api builds against pushed pseudo-versions while the
slice is open and takes one pin at the end, never a re-pin per toolkit tag.
