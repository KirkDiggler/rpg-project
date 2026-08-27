---
name: Synty Subscription Race-Catalog Fast Lane
description: Discover candidate playable-race models across the private Synty subscription without applying production-promotion ceremony to exploration.
issue: https://github.com/KirkDiggler/rpg-project/issues/296
team: Assets
status: approved brainstorm pending written review
---

# Synty Subscription Race-Catalog Fast Lane

## Purpose

Build a private, browsable inventory of the Synty subscription so the project can
choose visual candidates for the classic 2014 Player's Handbook races before it
productionizes any model. Discovery must be fast enough to support early-game
iteration. Production rigor begins only after a model has been selected for the
game.

This work separates three questions that the current asset process has allowed
to collapse into one expensive operation:

1. **What assets did the subscription provide?**
2. **Which complete models might visually represent playable races?**
3. **Which selected models should become production runtime assets?**

The first two questions belong to this catalog. The third is the next goal and
receives its own implementation scope after Kirk reviews the gallery.

## Why this fast lane is needed

The current provider path protects licensed content and produces accountable
runtime assets, but it applies release-engineering standards too early. The
longsword/shortbow journey demonstrated the cost:

- design/plan PR: 2,195 additions and 12 commits;
- web Concept PR: 2,575 additions and 18 commits;
- roster Learn PR: 2,647 additions and 7 commits;
- provider branch through its first two GLBs: 5,939 additions and 19 commits;
- total through that checkpoint: 13,356 additions, 56 commits, and more than 26
  hours elapsed, before production equipment was wired on the real game route.

The reviews in that work found legitimate defects because the chosen design
introduced a custom semantic GLB transformer, defensive binary parser,
deterministic release seals, and transactional installer. The lesson is not to
stop reviewing complex release systems. It is to avoid building a release
system when the current question is merely, “Which model looks right?”

The project is pre-pre-alpha. License safety and honest rendering remain
non-negotiable; byte-perfect release ceremony does not precede visual
selection.

## Subscription snapshot

A read-only central-directory scan of the current subscription drop found:

| Fact | Observed inventory |
| --- | ---: |
| archives | 33 |
| compressed size | 3.31 GiB |
| estimated extracted size | 9.54 GiB |
| ZIP members | 23,157 |
| FBXs in ZIPs | 16,742 |
| `SK_` FBXs | 934 |
| `SM_` FBXs | 14,226 |
| PNGs | 1,733 |
| individually named complete-character candidates | about 127 |
| Modular Fantasy Hero skinned parts | 720 across 28 categories |
| dedicated Emotes/Taunts + Sword Combat animation FBXs | 526 |

The complete-character estimate normalizes known engine-format duplicates and
includes named candidates from Adventure, Boss Zombies, Dark Fantasy, Dungeon,
Dungeons Realms, Elven Realm, Fantasy Characters, Fantasy Kingdom, Pirates,
Werewolves, and the Viking Unity package. It is a discovery estimate, not a
promise that all 127 import or animate correctly.

Several packs store characters inside combined FBXs rather than one file per
body. Modular Fantasy Hero supplies parts rather than hundreds of complete
characters. Sidekick Modular is a higher-poly, later-generation system currently
available through Unreal-oriented content; its extraction, assembly, visual
fit, and runtime cost are a later investigation.

## Approved rulings

1. Index every subscription archive.
2. Render individually exported complete bodies and candidates recoverable from
   combined FBXs.
3. Index but defer Modular Fantasy Hero and Sidekick Modular.
4. Use neutral visual groups with optional, noncanonical race suggestions.
5. Use the classic 2014 PHB nine as the initial coverage checklist: Human, Elf,
   Dwarf, Halfling, Dragonborn, Gnome, Half-Elf, Half-Orc, and Tiefling.
6. Preserve additional useful discoveries outside the initial nine.
7. Prefer complete low-poly bodies plus bounded trait swaps—ears, horns, tail,
   hair, palette, and modest scale—over full modular assembly for the first
   playable-race direction.
8. The initial catalog discovers candidates; it does not promote runtime assets
   or decide canonical races.
9. After the catalog is reviewed, the first production goal is to choose and
   productionize a coherent nine-model race roster.

## Architecture: three separate truths

### Generated subscription inventory

The private provider repository owns:

```text
rpg-game-assets/library/synty-subscription/index.json
```

A generator scans an explicit `--source-root`. It never stores the operator's
absolute source path. For each archive and member it records factual,
regenerable data:

- archive identity, size, and hash;
- archive-relative member path, size, and extension;
- likely asset category: body, accessory, modular part, animation, weapon,
  prop, environment, or unknown;
- delivery form: individual FBX, combined FBX, Unity package, modular system,
  or Unreal-deferred system;
- duplicate basename groups without collapsing source identities;
- readiness: `individual`, `combined-needs-split`, `modular-deferred`,
  `unreal-deferred`, or `unknown`.

This generated layer makes no playable-race judgment.

### Curated race candidates

The same private scope owns:

```text
rpg-game-assets/library/synty-subscription/race-candidates.json
```

Curated entries reference exact generated inventory IDs and add human judgment:

```text
inventoryId
sourceDisplayName
neutralVisualGroup
possibleMappings[]
adaptations[]
adaptationEffort: low | medium | high
rigFamily
sourceReadiness
reviewStatus
notes
```

A model may suggest several races, and a race may retain several candidates.
The data says “possible mapping,” never “this asset defines the race.” Runtime
rules and canonical character options remain outside the asset index.

### Visual gallery

Issue-scoped evidence in private `rpg-game-assets/evidence/` contains labeled
contact sheets and the classic-nine coverage sheet. Rendered evidence links back
to candidate and inventory IDs. It never embeds source FBXs, source textures, or
other redistributable package contents.

This descriptive process record remains in `rpg-project`; it does not copy the
private generated inventory.

## Discovery flow

### Pass 1: scan archives without bulk extraction

The indexer reads ZIP central directories and Unity-package pathnames. It does
not invoke Blender and does not extract all 9.54 GiB merely to discover what is
present. Unknown naming remains visible rather than being guessed away.

The current `game-dev/scripts/ingest-assets.sh` is not the subscription scanner:

- it recognizes 32 of the 33 current archive names;
- only three currently have both a supported conversion profile and atlas
  configuration;
- nonempty output means “converted,” so a partial run cannot resume honestly;
- output is flattened by basename even though different source files can share a
  basename;
- one atlas is applied to every FBX in a pack despite multi-atlas packs;
- animation packs, Unity packages, combined character FBXs, and modular systems
  need different treatment.

The new index does not broaden that script into a universal converter. It first
makes the diversity explicit.

### Pass 2: classify character candidates conservatively

Generated heuristics nominate likely bodies, accessories, engine duplicates,
combined FBXs, modular parts, and animations. The curated file confirms or
corrects nominations. An uncertain entry stays `unknown`; classification is a
navigation aid, not a release gate.

### Pass 3: materialize only selected candidates

Only candidate FBXs and required character textures are extracted into an
explicit gitignored or temporary stage. Source-relative paths are preserved.

- Individual FBXs proceed to Blender inspection.
- Combined FBXs are imported once and enumerated by contained armature/object.
- Recoverable complete characters receive candidate IDs and renders.
- Modular Fantasy Hero and Sidekick stop at their deferred readiness labels.
- No candidate is copied into the runtime `harness/` tree.

### Pass 4: inspect and render in Blender

Blender imports candidates directly; discovery does not first manufacture
production GLBs. Inspection records:

- armature and mesh count;
- bone-name/hierarchy fingerprint;
- dimensions;
- material and texture references;
- separate accessory relationships;
- import warnings or failure.

The gallery uses labeled front and three-quarter rest-pose views. Animation,
downed poses, portraits, tactical cameras, browser proof, and per-frame evidence
wait until selection. One pack-level character atlas mapping may be authored
when source FBX material discovery cannot resolve it automatically; this is a
pack adapter, not per-model calibration.

### Pass 5: generate race-oriented review sheets

Pack contact sheets expose the breadth of complete models. A separate
classic-nine sheet shows the strongest reviewed candidates, adaptation ideas,
readiness, and real gaps. Every card says “candidate only—not game canon.”

## Race-candidate scorecard

Each classic-nine row uses one of five statuses:

- **Direct:** the existing silhouette plausibly reads as the race.
- **Trait swap:** a complete body needs bounded ears, horns, tail, hair, or
  palette work.
- **Scale variant:** modest scale may work without hiding a proportion problem.
- **Experimental:** a plausible candidate needs visual proof or combined-FBX
  recovery.
- **Gap:** no honest candidate is identified.

Interpretation rules:

- Human, Elf, and Dwarf have strong named source candidates.
- Half-Elf may derive from Human or Elf candidates with reduced or swapped ears.
- Tiefling may derive from Dark Fantasy Demon or Human bodies with horns, tail,
  and palette treatment.
- Halfling and Gnome may explore smaller complete bodies or modestly scaled
  candidates, but the gallery flags unsuitable proportions.
- Half-Orc is judged by silhouette and facial/body read; a palette-only “green
  human” is not automatically accepted.
- Dragonborn remains a gap unless a demon, gargoyle, reptilian, or combined-FBX
  candidate reads honestly in the gallery.
- Body presentation is cataloged separately from suggested race mapping.
- High-effort geometry surgery is deferred rather than described as a low-cost
  adaptation.

The sheet may retain several candidates per race and may leave a row as a gap.
Discovery is not required to force nine arbitrary green checks.

## Ceremony budget

The initial catalog uses the minimum durable project shape compatible with the
workspace:

- one `rpg-project` Decide record and descriptive design PR;
- one subsequent `rpg-game-assets` Build issue and PR for the complete initial
  index/gallery;
- no issue per archive, model, race, or rig family;
- one asset implementation branch;
- one final whole-change review;
- Kirk's gallery review as the product gate.

There is no task-by-task SDD review/fix/re-review loop. Internal implementation
steps do not each earn a PR, checkpoint comment, or human approval.

The discovery path explicitly avoids:

- double-build byte determinism;
- custom transaction/apply systems that duplicate Git rollback;
- exact JSON key-order tests without consumer significance;
- hash-bound screenshots and elaborate evidence receipts;
- class × model × animation × frame × camera evidence matrices;
- separate Design → Concept → Learn → Build sequences per ordinary asset;
- hostile-input GLB hardening before a real source failure justifies it;
- repeated human review for models in an already-understood rig family.

More ceremony is earned by an observed exception: a new skeleton family,
combined/modular assembly, geometry surgery, failed animation transfer, visible
runtime defect, measured performance problem, or new consumer contract.

## Failure behavior

- A malformed archive is reported and scanning continues; the command exits
  nonzero after writing no replacement index if the complete generation is not
  valid.
- A failed candidate render produces a visible error record/card and does not
  block unrelated candidates.
- Unknown classifications remain available for human correction.
- Duplicate basenames remain separate source records.
- Tracked generated output is replaced through an ordinary temporary file only
  after successful generation. Git provides rollback.
- No source-root absolute path appears in tracked data, logs intended for
  publication, or evidence.
- Raw licensed source and extracted textures never enter `rpg-project` or a
  public repository.
- Performance observations are informational during discovery. Budgets become
  gates when a candidate enters production promotion.

## Testing strategy

Automated coverage remains narrow and high value:

- fixture ZIP and Unity-package inventory;
- source-relative path preservation;
- duplicate basename detection;
- deterministic generated index bytes;
- conservative body/accessory/modular/animation classification;
- rejection of absolute paths in tracked output;
- explicit Modular and Unreal deferral;
- one standalone and one combined-FBX Blender inspection/render smoke.

Acceptance then runs one real scan against the 33 current archives and performs
human visual review of the generated gallery. The provider repository's broad
runtime release suite is not rerun after every internal catalog step; the final
repository gate runs once before review.

## Delivery order

1. Record this descriptive fast-lane decision.
2. Create one `rpg-game-assets` Build issue/Project 19 item and fresh branch.
3. Implement the archive scanner and generate the 33-archive inventory.
4. Curate complete-body, combined-FBX, trait, modular, and deferred candidates.
5. Add selective extraction and Blender inspection for individual bodies.
6. Enumerate combined FBXs; render separable candidates and record failures.
7. Generate pack contact sheets and the classic-nine coverage sheet.
8. Kirk reviews the gallery and identifies candidates for the subsequent
   nine-model production goal.
9. Run one final review and publish the asset-index PR.
10. Keep this design PR open through the asset implementation, then merge it as
    the durable decision record.

Initial gallery priority is Fantasy Kingdom, Fantasy Characters, Adventure,
Elven Realm, Dark Fantasy, Dungeons Realms, Pirates, Dungeon creatures, Boss
Zombies/Werewolves, and then combined-only packs. Priority controls feedback
order, not index completeness.

## Done gate

The discovery Build is complete when:

- all 33 current archives are represented;
- duplicate source identities and delivery formats remain explicit;
- every discovered character-related source has a readiness classification;
- individually exported complete bodies have labeled front/three-quarter
  renders or a visible render error;
- combined FBXs are enumerated and marked rendered or blocked;
- the classic-nine sheet shows candidates, adaptation effort, uncertainty, and
  real gaps;
- no raw licensed source is committed;
- no runtime alias, production GLB, game contract, customization UI, or
  canonical race decision is introduced; and
- Kirk can choose the next nine-model production goal from visual evidence
  rather than archive archaeology.

## Next goal after discovery

Kirk's gallery verdict selects a coherent candidate roster for the classic
nine. That production goal may use complete low-poly bodies plus bounded trait
swaps. It will calibrate each distinct rig family once and batch models that
share it. Sidekick and full Modular Fantasy Hero assembly remain separate later
problems unless the gallery proves that the low-poly complete-body inventory has
an honest gap they uniquely solve.
