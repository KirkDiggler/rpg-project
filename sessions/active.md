# Active handoff — 2026-08-03: Specimen v0.1 provider loop; independent #173 delivery complete

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.
> Platform implementation is repository-owned; merge/release decisions remain with
> the director and Kirk. Planning PR #181 stays open through implementation and
> the final retro.

## Now

**Dungeon Builder / Specimen Pack v0.1: API #763 has consumed the open toolkit
provider locally and passed its live-lab evidence.** The provider remains held open
for related consumer asks; web #671 then #678 are the next ordered product-route
legs. Separately, the Party Assembles equipment-enrichment delivery is complete.

## Solid — code and verification

- **Schema authority:** [Specimen Pack v0.1](https://github.com/KirkDiggler/rpg-project/issues/175#issuecomment-5162198168)
  is the single canonical YAML grammar and acceptance specimen. Current
  `dungeonspec`/compiler behavior is prototype scaffolding, never a competing
  compatibility contract. This is recorded in the open
  [Dungeon Builder plan PR #181](https://github.com/KirkDiggler/rpg-project/pull/181).
- **Toolkit provider:** `rpg-toolkit` [PR #876](https://github.com/KirkDiggler/rpg-toolkit/pull/876)
  is at `bac22483f2e35812acf53b20bcfe94f3a76c55c1`. Generated-edge and party-start
  evidence is PASS at that head. It is intentionally the sole open provider PR for
  related Specimen v0.1 asks: do not merge or tag it per slice; every new commit
  requires refreshed review/evidence.
- **API consumption:** `rpg-api:feat/763-generated-floor-plan-edges` is pushed at
  `dec741058852183d006417dc6580c3c6230cabaa`. `StartEncounter` delegates party
  seats to the toolkit, preserves member order, and persists its returned positions;
  API performs no spawn arithmetic. Content load/authoring use `LoadWithConfig`
  with the same capacity injection.
- The API branch has exactly one local-only toolkit override: the encounter module
  pointed at #876 (`bac22483`). It is active through
  `scripts/toolkit-local-override.sh`, must not be committed, and is removed only
  after the provider is merged/tagged and API pins that released module.
- **Live lab:** `rpg-api-dungeon-builder-763` / Envoy are healthy on
  `localhost:8091`. Authenticated real authoring/lobby/ready/`StartEncounter`
  calls verified 196 generated physical edges and a four-player authored
  `start: [12, 4]`. The persisted entrance and ordered seats matched the toolkit
  result, with player one at the authored anchor. Reproducible local evidence is in
  ignored `rpg-api/local-dev/dungeon-builder-763/`.
- **Web sequencing:** [web #671](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/671)
  (product `/author` route) and [web #678](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/678)
  (generated-edge render/hit-test) are open and ordered. #671 starts fresh from
  `origin/dev`; #678 is a separate fresh branch after #671.
- **Independent Party Assembles delivery:** [web #692](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/692)
  merged to web `dev` at `4b4b411`; [rpg-project #173](https://github.com/KirkDiggler/rpg-project/pull/173)
  merged to project `main` at `cdbfc19`. The implementation issues are closed/Done.
  This consumer leg is independent of the Specimen #876/#763 provider loop.
- **Independent API test repair:** [rpg-api #765](https://github.com/KirkDiggler/rpg-api/issues/765)
  is the unrelated Sneak Attack fixture flake. Its [PR #766](https://github.com/KirkDiggler/rpg-api/pull/766)
  is merge-ready with CI and director gate PASS; it is independent of this provider
  loop.
- **Toolkit persisted-choice gate:** [rpg-toolkit #879](https://github.com/KirkDiggler/rpg-toolkit/pull/879)
  merged 2026-08-03. It closes the finalization bypass for persisted nested category
  selections: both `ValidateChoices` and `ToCharacter` reject an ineligible
  persisted Monk `unarmed-strike`, while persisted `club` and `shortbow` controls
  survive.

## Open questions / gates

- Keep #876 open while API or web has related toolkit asks. The exact current head,
  not a prior gate, is the only valid provider evidence after any further push.
- API cannot replace its local override until #876 has a released encounter-module
  version to pin. No second toolkit override is allowed.
- The product route still needs #671 before #678 can prove generated-edge behavior.

## Next

1. Run the normal review/gate process for API `dec7410`; retain its sole local
   override during that work.
2. Start web #671 from latest `origin/dev`, then cut the separate #678 branch from
   latest `origin/dev` once the product editor route is available.
3. Leave merge decisions to Kirk: this coordination update makes none.

## Decision log

- **2026-08-03 — Specimen authority:** Specimen Pack v0.1 wins on every grammar
  conflict; recorded in #181 (`25be6a6`).
- **2026-08-03 — provider loop:** #876 is deliberately held as one open toolkit
  provider until consumers stop asking; recorded in #181 (`cd483b3`) and on #876.
- **2026-08-03 — authored start:** API #763 validated the `bac22483` provider seam
  locally at `dec7410`, including generated-edge and four-player authored-start
  evidence; it does not merge the provider or commit the local override.

## Pointers

- Board: https://github.com/users/KirkDiggler/projects/19
- Design / plan: `ideas/dungeon-builder/{design,plan}.md` (PR #181)
- Toolkit provider: rpg-toolkit #876; API consumer: `rpg-api:feat/763-generated-floor-plan-edges`
- API local-override guide: `rpg-api/docs/how-to/local-toolkit-override.md`
