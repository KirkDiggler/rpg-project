# Active handoff — 2026-08-13

## Now

**The session SDK lane** (`rulebooks/dnd5e/session`) — rpg-api's one interface to the toolkit.
W0/W1/W2 shipped. **W3 step 1** (characters load) → `session/v0.3.0`. **W3 step 2** (conditions
across a suspension) → PR #950 merged at **132f018**, tagged **`session/v0.3.1`** — a `test:`
commit takes a **patch** bump. **W3 step 3** (NPCs — `Join`/`Spawn`) → PR #952 merged at
**f0c7152**, tagged **`session/v0.4.0`**. Tracking
[toolkit#945](https://github.com/KirkDiggler/rpg-toolkit/issues/945).

**Remaining in W3: T3.6's scene only**, and it is blocked — see Open questions.
[Toolkit PR #953](https://github.com/KirkDiggler/rpg-toolkit/pull/953) is in flight with
ADR-0037 and the enforced decisions digest.

## Solid

**Shipped, tag verified against the merge commit in every case:**

| Module | Version | Merge | What it brought |
|---|---|---|---|
| `dnd5e/encounter` | v0.1.0 → v0.4.0 | #921/#924/#932/#939 | the composition; transitions; world anchoring; bounded story log |
| `dnd5e/session` | v0.1.0 | 407bd57 | shell, 10 free-roam verbs, event stream, the boundary test |
| `dnd5e/session` | v0.2.0 | 556b2e1 | the interrupt spine — suspend, persist, resume in a fresh process |
| `dnd5e/session` | v0.3.0 | 5c79152 | characters load through the host's repository |

**The strategy, and its falsifiable claim.** Wrap what exists → migrate rpg-api once at W4 →
replace behind it. The claim: *after the migration wave, no subsequent wave changes an rpg-api
source file.* Gated by `gorelease` from the first tag, and made mechanical by
`TestNoInnerTypeCrossesTheBoundary` — it parses the package's own AST and fails on any leaked
toolkit type. rpg-api imports **zero** of the composition today, so nothing is owed yet.

**Rulings that decide future calls — don't re-derive these:**

- **Start from the reversible direction.** Adding a `Config` field is compatible; removing one
  a host implemented is not. Loosening a rule is compatible; tightening breaks every host. So:
  path adjacency and the event stream were required from day one; repositories were deferred
  until something called them.
- **`gorelease` has a blind spot — a new *required* `Config` field.** It reports compatible, it
  compiles everywhere, and it fails every existing host at its first `NewManager` (S8:
  construction is total). Demonstrated live on #949. Discipline: **every required `Config` field
  lands at or before W4.**
- **The repo AUTO-TAGS on merge to main** (`auto-tag-modules-safe.yml`; the other two tag
  workflows are disabled). Bump comes from the conventional-commit prefix — `feat:` → minor,
  **`test:`/`fix:` → patch** (a test-only PR still consumes a version; #950 took `v0.3.1`).
  **A wave's version is decided by its FIRST merge, not its last** — never write a version next
  to a milestone.
- **The compat gate races the tagger, and it fails CLOSED.** `compat.yml` derives its base from
  the newest git tag, but `gorelease` resolves "most recent version" through the module proxy. A
  PR opened soon after a merge therefore passes a base the proxy has not indexed yet, and the job
  dies with *"Can only suggest a release version when compared against the most recent version of
  this major"* — which reads like an API problem and is not. **Re-run the job once the proxy
  catches up.** Cost this one hit: a red gate on PR #952 that had nothing to do with its code.
- **Contract types vs persistence shapes are two different promises.** A persistence shape
  (`encounter.EncounterData`, `interrupt.LedgerData`) is bytes the host round-trips and never
  builds — it promises *replaceability*. A contract type (`spatial.Position`, `character.Data`)
  is shared vocabulary the host constructs — it promises the opposite: a change is *announced*.
  Kirk's framing: a character is a thing, not an implementation detail we'd refactor without
  telling the API. The allow-list now names both categories with a disjointness pin.
- **There is no session process.** Every verb loads, attaches to a bus made for that call, acts,
  saves, and dies with the response; `Answer` is the same load-and-attach done again from data.
  Consequence: **`character.Cleanup` must not be called** — it nils the conditions `ToData`
  serializes, so cleaning up before the save persists a character with zero conditions, silently.
  Safe to skip because conditions intercept on the bus rather than mutating fields.
- **Stateless-per-call does not need a cache.** Benchmarked, not assumed: a player join costs
  ~5.1µs / 64 allocs more than a monster join — the character load exactly — so ~30µs at party
  scale. The caching-repository escape hatch stays theoretical.
- **Write world-then-session.** Partial failure must leave a collectable orphan, never a session
  pointing at nothing, and never a window resuming past cells nobody walked.

**The parked question W2 named honestly:** *"stop the walk when the walker sees something new"* is
a game rule living in a module whose charter says it owns no rules. It's there because no module
owns *when* a resolution should pause. It moves when the second checkpoint kind makes the deciding
thing's shape visible — **W4, not W3**; W3's bus carries observation only and nothing in it can
suspend.

## Open questions

- **`ConditionBehavior` cannot name itself, and it blocks T3.6.** The interface is
  `IsApplied/Apply/Remove/ToJSON` with no `Ref()`. Reporting a character's active conditions at
  the seam would mean unmarshalling `ToJSON()` and reading `ref` — the exact anti-pattern #941
  already files against us. Without it **T3.6's scene is not expressible** ("Alice raging when she
  is loaded, without the caller ever mentioning rage"). Needs a `Ref()` on the interface in
  `rulebooks/dnd5e/events` plus its ~24 implementers. **Kirk's call; not started.**
- **#946 — character as its own module.** Direction only, blocks nothing. The open part is whether
  the unit is `character` alone or a `dnd5e/core`-shaped module holding the shared enums.

**Resolved since the last handoff, recorded because the reasoning generalises:**

- *NPCs entering the session.* Kirk's ruling (2026-08-13): **a ref is loader routing** — it names
  the package that can load some data. That killed the `dnd5e:characters:<id>` idea on its own
  merits: no toolkit package can load a player character, so such a ref would claim something
  false. Hence **two verbs split by where the data comes from** — `Join` loads a player by ID,
  `Spawn` instantiates code-resident content by ref — an axis that survives durable NPCs and
  homebrew, where player-vs-monster does not. **The encounter needs no change**: `monster.Data`
  carries its own `Ref`, and `SessionData.NPCs` holds the instance, so an earlier "the encounter
  must record which monster a member is" finding was measuring the encounter as if it were the
  sole record.

_(#916's dangling `closes #916` references were removed on PR #950.)_

## Next

**The rest of W3** ([#945](https://github.com/KirkDiggler/rpg-toolkit/issues/945)): NPCs in
`SessionData` (session-scoped, **no `NPCRepository` until a durable NPC exists** — adding a Config
port later is compatible, removing one isn't), and **conditions surviving a suspension** — the
invariant W2 forced rather than chose: durable condition state must round-trip through the blob or
a suspension loses it. Takes `session/v0.4.0` on merge.

Then **W4 — combat, where rpg-api migrates.** The payoff wave: the version-bump promise starts
there, and its 22 files / ~6,700 lines of old-stack orchestration are *evidence about what a game
server needs, never a specification to port*.

## Decision log

| Date | Decision | Visible at |
|---|---|---|
| 2026-08-12 | Session SDK pivot — encounter is *the world*, session is *the table* | toolkit#935, PR #936 (merged) |
| 2026-08-12 | Six-wave plan; anything shaping a public type goes early | `docs/ideas/session-sdk/plan.md` |
| 2026-08-13 | Seam takes IDs; `character.Data` only on the repository | PR #947 (merged) |
| 2026-08-13 | W3 scoped to "get the character loaded", steps allowed | PR #949 (merged) |
| 2026-08-13 | Tag column stops predicting after W3 (auto-tag on merge) | `plan.md` |
| 2026-08-13 | Local `replace`/`go.work` overrides are fine — *committing* them is the ban | `rpg-toolkit/CLAUDE.md` |
| 2026-08-13 | Entity entry splits on **load-vs-instantiate**, not player-vs-monster; a ref names the package that can *load* the data | ADR-0037 |
| 2026-08-13 | Seam decisions get genuine options + trade-offs in the open, then an ADR with the rejects recorded | ADR-0037's process note |

## Carried follow-ups — filed, none blocking

- **[#948](https://github.com/KirkDiggler/rpg-toolkit/issues/948)** `character.LoadFromData`
  silently drops conditions it cannot load, and the next `ToData()` persists the loss. Pinned not
  fixed — the policy belongs to the character package. **Directly relevant to W3's second step.**
- **[#933](https://github.com/KirkDiggler/rpg-toolkit/issues/933)** `Members()` reports room but
  not position. Felt already: a W3 test had to use `Exit` because no read verb lists members.
- **[#940](https://github.com/KirkDiggler/rpg-toolkit/issues/940)** beats addressed to every member
  regardless of perception — harmless as a query, a fog-of-war leak as a push channel.
- **[#941](https://github.com/KirkDiggler/rpg-toolkit/issues/941)** story tags coarser than beats,
  so the SDK reads event kind from a payload it doesn't own — and fails *silently* if that shape
  changes.
- **[#934](https://github.com/KirkDiggler/rpg-toolkit/issues/934)** two validation asymmetries from
  the anchoring wave.
- **[#951](https://github.com/KirkDiggler/rpg-toolkit/issues/951)** `encounter.ErrNoMember` means
  an empty ID, an **absent** member, and a **duplicate** one — opposites under one sentinel. The
  SDK cannot translate what it cannot distinguish, so joining someone twice reports "no such
  member". An `ErrMemberExists` sentinel was written and **removed before shipping** rather than
  exported unpopulated; current (wrong) behaviour is pinned through both verbs so the fix announces
  itself.
- **[rpg-project#218](https://github.com/KirkDiggler/rpg-project/issues/218)** every hand-maintained
  module list in the toolkit **fails open** — 3 instances found in one afternoon. Rule to encode:
  *stable claims in hand-written docs, volatile claims derived or tested.*
- **Retention default → 8** — fold into the next encounter touch, not a standalone PR.

In every composition case the layering holds: **fixing it in the composition fixes the SDK for
free.**

## Other lanes

- **Semantic scope (#180)** — parked. Wave 0 is live-verified (proto root `v0.1.120`, toolkit
  `encounter/v0.49.1`, rpg-api#771; evidence on rpg-project#192). Wave 1 implementation has not
  started; issues can be cut when the lane is picked back up.
- **rpg-api#793 wiring** is Platform's whenever they pick it up. Nothing blocks it; the map surface
  is live and the Locate→Move trap is called out for them explicitly.

## Pointers

- **Decisions:** toolkit **`docs/adr/DECISIONS.md`** — the cliffnotes digest of all 38 ADRs, one
  or two lines each with the rule each generalises to. **Read this, not the ADR corpus:** loading
  38 ADRs is expensive and imports baggage (three propose modules never built; numbers collide;
  status fields are unmaintained). Enforced by `scripts/check-decisions.sh` in CI, so a new ADR
  fails until it is summarised. Open a full ADR only to contradict one or to get its trade-offs.
- **Deep record:** toolkit `docs/ideas/session-sdk/{design,plan}.md` (the triplet — design is
  ratified-and-annotated, plan is rewritten freely), `docs/ideas/encounter-transitions/`,
  `docs/ideas/encounter/`, `docs/journey/051+052`.
- **Grades and what would raise them:** toolkit `docs/quality.md` (session holds **A-**).
- **Doc ownership:** each code repo owns its own status/quality/architecture; rpg-project says how
  we *work*, not what was done — except `docs/ideas`, which records the plan as it stood.
- **Co-location rule:** each wave's implementation PR amends the design doc in the same commit that
  makes the amendment true. The drift #936 had to fix existed *because* the doc lived on a
  different branch from the code.
- **Full narrative history** of this lane, session by session, is in this file's git history —
  `git log -p sessions/active.md`, tip of the long-form version at **b21a49e**.
