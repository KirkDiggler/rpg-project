# Dungeon Builder: Implementation Plan

**Parent:** rpg-project#175
**Design:** `ideas/dungeon-builder/design.md` (rpg-project#170, merged)
**Tracking surface:** this plan PR stays open until the cross-repo implementation it tracks is complete.

**Goal:** Deliver the Dungeon Builder authoring loop—available at `/author`
without a dev-only gate to normally Discord-authenticated users on Kirk's server
through existing auth—and its approved geometry dialect in the ordered slices
#176–#180, while keeping the toolkit as the canonical source of dungeon rules
and geometry, the API as a projection and orchestration layer, and the web as a
renderer/editor of server-provided truth.

## Working rules

- This plan implements the approved design; it does not reopen its settled
  semantic-region, canonical-edge, runtime-wire, or YAML-version decisions.
- Develop outside-in: the product editor need names the contract, protos
  transcribe that need, API names the toolkit requirement, and toolkit provides
  canonical behavior. Merge inside-out: providers release first, consumers pin
  released versions, and the product route follows.
- One branch per repository per slice. Keep fixes discovered while integrating a
  slice on that slice's branch; do not split them into stacked provider PRs.
- Branch from a freshly fetched remote base: `origin/main` for
  `rpg-toolkit` and `rpg-api-protos`; `origin/dev` for `rpg-api` and
  `rpg-dnd5e-web`. API and web PRs explicitly target `dev`.
- Every contract continues to project runtime wall geometry through
  `HexRecord.edges`; neither authoring nor runtime may restore the removed
  flat `EncounterService.Space.walls` field.
- The web renders server-provided floor-plan legality and geometry. It never
  reconstructs parity, room-chain placement, edge canonicalization, movement,
  LoS, or room identity.
- Keep the design's error split: malformed requests use `InvalidArgument`;
  well-formed YAML content failures return `success=false` plus field errors;
  a genuinely unavailable `AuthoringService` is `Unimplemented`.
- Each implementation issue/PR records its parent slice and ends GitHub bodies
  and comments with `— asset-pipeline agent, on behalf of KirkDiggler`.

## Prerequisite: product editor route — rpg-dnd5e-web#671

Before a slice asks the web to prove product-route behavior, #671 starts fresh
from the latest `origin/dev`. It never cherry-picks from or continues the
concept branch: the concept is evidence and learning only, not implementation
authority. Existing components already on `dev` may be retained only when they
are production-suitable. #671 delivers the `/author` product route without a
dev-only gate, available to normally Discord-authenticated users on Kirk's
server through existing auth, with honest live, unavailable, invalid, and
save-ready states; comment-preserving document editing; and the save → lobby →
Walk it handoff. It defines the consumer affordances without inventing client
rules or extending the concept route.

Fixture adapters remain concept/test concerns. The product route instead uses
local API endpoint integration with the real `AuthoringService`. Keep #671 and
#678 separate and ordered: #671 delivers the product editor shell; then a
separate #678 branch, fresh from the latest `origin/dev`, delivers the Slice
#176 generated-edge web leg on that route.

**Gate and evidence:** targeted comment-round-trip/destructive-board-edit tests
and `npm run ci-check`; route screenshots/recording of each availability state;
and a real product-path save/Walk it proof once the available authoring contract
permits it. Record the flattened-versus-hex product display decision in that PR.
The route remains the integration surface for every web leg below, not a second
parallel editor.

## Slice #176 — generated wall and door truth

**Outcome:** Read-only canonical generated solid-wall and door geometry reaches
`FloorPlan` and is rendered and hit-tested identically by the 2D and 3D product
editor. No YAML `walls:` field, editing, or topology change is included.

1. **Name the consumer contract first (rpg-api-protos, `origin/main`).** Define
   an authoring-local edge message/enum in the `FloorPlan` response that preserves
   canonical endpoints, kind, and source door identity where available. It is not
   `dnd5e.api.v1alpha2.encounter.Wall`/`WallKind`; its comments explicitly map it
   to runtime `HexRecord.edges`. Run buf checks and publish the proto release.
2. **Develop the API consumer seam (rpg-api, branch from `origin/dev`).** Start
   the API slice branch after the authoring contract is reviewable. It consumes
   the published proto version, projects toolkit-owned canonical edges through
   the authoring flow, and does not re-canonicalize or derive competing geometry.
   If the existing toolkit API cannot describe every required edge, state the
   narrowly named provider requirement before implementing it.
3. **Supply canonical toolkit support (rpg-toolkit, branch from `origin/main`).**
   Add only the canonical edge export/describe support required by the API
   consumer. The toolkit remains the source of generated edge truth; generated
   dungeon and runtime behavior otherwise remain unchanged. Release the toolkit
   provider version.
4. **Iterate locally without committing an unpublished dependency.** While the
   provider is unmerged, API may use `rpg-api/docs/how-to/local-toolkit-override.md`
   for **exactly one toolkit module**. The #176 API slice branch must harden and
   constrain `scripts/toolkit-local-override.sh` so it permits exactly one module
   override and verifies the resulting module list; that supporting script change
   belongs on the same API slice branch, not a separate wave. API consumes the
   published proto version rather than overriding protos locally.

   **Release-pin validation (before opening the API PR and again at the final
   API gate):** remove the override; run the API build/test validation with
   `GOWORK=off`; prove the committed diff contains no `replace` directive; and
   check that no active `go.work`, `go.work.sum`, or local-toolkit override path
   can redirect the build. Record the commands and their output in the API PR.
   A repository-local or user/global Go workspace is not an acceptable substitute
   for this proof.

   **Decision gate:** any recovery to a separate-PR workflow is an explicit Kirk
   decision/gate; this plan does not approve it.
5. **Deliver inside-out.** Release protos and toolkit provider support; update
   API to real released pins and remove its override; then, after #671, land the
   #176 web integration as the separate #678 branch fresh from the latest
   `origin/dev`, on the #671 product route. The web consumes the projected edge
   contract and does not infer missing geometry.

**Gate and evidence:**

- Contract coverage proves `showcase.yaml` exposes a generated door, exterior
  wall, and interior-facing segment without duplicate/conflicting client edges.
- API integration evidence captures wire state from that fixture against real
  released provider pins, plus tests that guard the projection rather than a
  duplicate canonicalizer.
- Product evidence captures 2D and 3D `/author` screenshots/recording showing
  matching coordinates, kinds, render, and hit targets; concept/sandbox-only
  evidence does not satisfy this gate.
- Existing generated-dungeon and runtime encounter regression gates pass.

**Stop point:** do not begin authored edge editing until provider-release pins,
wire state, and reachable product-route visual evidence all pass.

### #176 technical review checklist and evidence (reviewed 2026-08-02)

- [x] Delivery order, one-branch-per-repository discipline, and the temporary
  override boundary are explicit.
- [x] Release-pin validation explicitly requires `GOWORK=off`, no committed
  `replace`, and checks for active `go.work`, `go.work.sum`, and
  local-toolkit-override redirection before both the API PR and final API gate.
- [x] The #176 API slice owns the hardening of
  `scripts/toolkit-local-override.sh`: it must allow exactly one override and
  verify the module list.
- [ ] Execution evidence is not available yet: #176 has not delivered an API
  branch, released pins, command output, or product-route proof. These remain
  merge gates, not completed evidence.
- [ ] Separate-PR workflow recovery is **not approved**. It remains an explicit
  Kirk decision/gate; the supporting script change is in the same #176 API
  slice branch, not a separate wave.

## Slice #177 — authored start marker

**Outcome:** Optional dungeon-scoped `start: [c, r]` overrides the generated
entrance; omitted `start` exactly preserves existing behavior.

Plan the consumer-facing UI and wire need outside-in, then deliver provider
validation/compilation, API projection/startup plumbing, and web integration
inside-out. Keep static legality distinct from post-compilation/generated-floor
legality: the reserved door row is valid for `start` but not `place`; resolved
rolled content must reserve the resolved start; an unusable resolved start is a
hard failure, never a fallback spawn.

**Gate and evidence:** validation coverage for bounds, explicit non-floor cells,
coordinate declarations, authored blockers/placements, and absent-start
compatibility; compiled `FloorPlan` exposes one consistent resolved start; wire
state and a real Walk it run show the same spawn cell. Stop if preview and actual
encounter startup disagree.

## Slice #178 — floor-prop hex facing

**Outcome:** Existing room-scoped floor placements accept optional
`facing: E|NE|NW|W|SW|SE`; absence preserves current orientation.

Only add wire/API legs when orientation crosses that seam; do not broaden into
wall mounting, height, rectangular directions, or AI behavior.

**Gate and evidence:** strict six-value validation with accepted vocabulary in
errors; all six orientations compile; an asymmetric prop agrees across YAML,
authoring wire state, product preview, and runtime; legacy YAML remains visually
unchanged. Stop if the renderer requires client-derived orientation semantics.

## Slice #179 — authored canonical wall and door edges

**Outcome:** Dungeon-scoped `walls: [{from, to, kind}]` edits canonical solid and
door edges, including inner walls, without changing semantic room identity.

Build on #176's proven read-only edge seam. Specify deterministic generated-edge
overlay behavior before provider code; initially reject collisions with
connector-generated edges. Compile authored edges into canonical runtime
`HexRecord.edges`; do not make authored doors satisfy or replace chain
connectors.

**Gate and evidence:** clear validation for duplicate, conflicting, non-adjacent,
out-of-footprint, and connector-collision edges; one shared dungeon-owned edge
representation; movement/LoS and door-lifecycle evidence through the existing
runtime path; an inner wall changes movement/LoS while both cells retain the
same room ID; product preview and Walk it agree. Stop for a design decision if a
connector mapping rule becomes necessary.

## Slice #180 — cell-authored semantic room regions

**Outcome:** Authors draw stable-ID, explicit occupied-cell semantic regions,
including non-rectangular regions, while retaining linear-chain topology and
connector semantics.

Build on #179's canonical-edge support. Define the rectangular-to-explicit-cell
compatibility/precedence path before implementation. Region membership—not walls
or geometric flood-fill—continues to own reveal, placement, spawning, scripting,
and archetype identity.

**Gate and evidence:** author-facing failures for empty, disconnected,
overlapping, and invalid cell sets; at least two connected non-rectangular
regions preserve IDs and linear connector requirements; adding/removing an inner
wall does not alter membership; compatibility coverage protects existing
rectangular specs; wire state and real product walkthrough agree on
cell-to-region membership. Stop for a design decision if supporting branching
topology, holes, levels, or automatic room naming becomes necessary.

## Completion and tracking

Keep this PR open as #175's cross-repo tracking surface while the prerequisite
and ordered slices run. Update it with linked per-repo issues/PRs, provider
release pins, gate evidence, and explicit stops/decisions. Close it only after
#176–#180 have each met their stated product-path evidence and the final
cross-repo retro confirms the authoring loop remains live.

— asset-pipeline agent, on behalf of KirkDiggler
