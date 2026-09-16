# Monster weapons — an author names what a placed monster can do

## Status: SHIPPED 2026-09-15 (walked by Kirk) — rpg-api-protos v0.1.192 · rulebooks/dnd5e v0.172.0 · encounter v0.83.0 · session v0.89.0 · rpg-api dev 25bee5fe; web palette outstanding. Two decisions corrected by the build, marked CORRECTED below. Originally resolved in session with Kirk 2026-09-15; this PR is the review surface. Sibling of [design.md](design.md) (the authoring file) and of rpg-project#191 (targeting keys); hangs off the Composable Dungeon Builder journey, rpg-project#169. Tracking issue: rpg-project#448.

North star (Kirk): **we are making a game based on the rules, not bound by them. The DM who authored the dungeon decides what each monster can do — some archers carry a short sword as backup, others do not — and the builder is the tool that lets them create rich experiences.**

Driving acceptance case: **in The Three Minds, place two goblin archers — one with a scimitar as backup, one with nothing but the bow — without touching Go.** The coward keeps its room and shoots; the one with a blade fights when cornered; the one without does not.

## Context — what exists today (verified 2026-09-15)

- **A monster has no hands.** Its arms are attack actions baked into its Go constructor (`rulebooks/dnd5e/monster/monsters/*.go`): the skeleton gets a shortsword and a shortbow, the goblin a scimitar, the thug a mace. Each is a hand-typed `combatActions.Definition` with the attack bonus and damage dice written out.
- **The driver picks by reach, in authoring order.** Both drivers (`rulebooks/dnd5e/behavior`) take the first action whose target is in reach. A skeleton next to you swings; farther away it shoots. Nothing is spent to switch and no hand is tracked. Melee is listed first, which is what makes the adjacent skeleton swing rather than shoot.
- **A placement already carries one per-instance word.** The dungeon file's monster line says `targeting: closest`; `dungeonspec.Placement.Targeting` carries it opaquely, rpg-api's `sessionworld` forwards it, and the session hands it to the driver. `Holds` (intel) rides the same road. A weapon list would be the third passenger on a road that exists.
- **Characters already derive attacks from held weapons.** `character.AssembleAttack` takes a weapon from the one weapons catalog (`rulebooks/dnd5e/weapons`) plus the sheet and produces the same `combatActions.Definition` the driver reads: finesse or ranged picks DEX, otherwise STR, proficiency added, delivery and damage read off the weapon, the weapon ref as the action ref, and the weapon named in the action's `WeaponContext`.
- **Monsters already have what that derivation needs.** `monster.Monster` carries ability scores and a CR-based proficiency bonus. Every SRD weapon line for the monsters we ship, and for the bandit we do not yet, is exactly what deriving from those scores produces:

  | monster | weapon | SRD line | derived |
  |---|---|---|---|
  | skeleton | shortsword, shortbow | +4, 1d6+2 | DEX +2, prof +2 |
  | goblin | scimitar, shortbow | +4, 1d6+2 | DEX +2, prof +2 |
  | thug | mace | +4, 1d6+2 | STR +2, prof +2 |
  | thug | heavy crossbow | +2, 1d10 | DEX +0, prof +2 |
  | bandit | scimitar, light crossbow | +3, 1d6+1 / 1d8+1 | DEX +1, prof +2 |

- **The builder sends the file, not a message.** `authoring/v1alpha1.PutDungeon` carries the dungeon as yaml text and answers with field errors. A new placement key needs no proto change; only a palette that wants to list weapons by name needs a door.
- **The retaliator reads hands.** `behavior.Retaliator`'s excuse asks whether the shooter is seen holding something ranged, through `weapons.Weapon.IsRanged`. It reads a character's hands only; a monster can never be excused today.

## The two cuts that broke, and why (recorded so they are not proposed again)

1. **"Ranged, melee, or both" as a stored placement field.** It is a category, not a thing held. It can only filter actions a definition already carries, so a goblin could never be made ranged by it. It collapses the field: the excuse needs "is it ranged", damage needs dice, the web needs a model in the hand. Naming a weapon answers all three; naming a category answers one and forces the rest to be reconstructed, and reconstructions lie. It also mixes the two states this initiative keeps apart: ranged versus melee is how the mind uses what it holds, not what it holds. It survives only as a **palette chip** that writes a weapon list.
2. **A catalog of per-monster action refs** (`skeleton-shortbow`, `goblin-shortbow`, ...). Kirk's question — *what is different between a skeleton shortbow and a goblin shortbow?* — has the answer: nothing. Both are +4, 1d6+2, 80/320, because both wielders have DEX 14 and proficiency +2. Such a catalog is a catalog of copies. The number is the wielder's, not the weapon's.

## Decisions (resolved in the 2026-09-15 session)

1. **A weapon is not a new kind of action.** It is the weapon-category attack definition a monster already has, assembled from the weapons catalog and the monster's own scores instead of typed by hand. Downstream — driver, resolution, projection, web — nothing knows the difference.
2. **The action list is the thing the author edits.** A placement names its actions in order. Weapon refs and authored non-weapon actions (claw, bite, multiattack, when they exist) sit in the same list. Empty means the definition's default. The author's order is the driver's preference order, so an archer with the blade listed first swings when adjacent.
3. **Assemble at spawn, store the result.** The session's rule stands: the sheet is what gets rehydrated. A saved run keeps the numbers it was spawned with, and a later change to the weapons catalog or the monster's scores does not silently re-arm a monster mid-run.
4. **Definitions become weapon refs too.** The goblin constructor says shortbow and scimitar rather than typing +4 and 1d6+2. This is how the goblin scimitar's reach-1 defect (`goblin.go`, "its unit defect is separate") goes away: it stops being authored.
5. **Fail closed at author time, not turn time.** A weapon ref the catalog does not know is refused as early as the module that sees it can: `dungeonspec` cannot import the weapons catalog, so `PutDungeon` and the shipped-file compile check only the ref's SHAPE and vocabulary (`dnd5e:weapons:*`); the catalog lookup happens in `session.Spawn`, which refuses the LAUNCH. **CORRECTED by the build:** the original text said a bad weapon refuses boot; it refuses one seam later, at launch. A turn never meets one.
6. **Nothing is spent to switch, and no hand is tracked.** Rules-as-written would charge an object interaction to stow and draw; nothing is asking for that, and RAW is not the authority here. If a weapon-swap cost ever earns its way in, it arrives with a use case, not with this slice.
7. **Proficient by default; the author may say otherwise (Kirk, 2026-09-16).** Proficiency is the creature's, not the weapon's, so the assembly asks the wielder whether it is proficient with the weapon handed to it — the same question it already asks a character. A monster answers YES for every weapon unless told otherwise. That reproduces every SRD line and makes a picked-up weapon right with no extra field. The switch is a reserved seat on the placement, whole-monster and not per weapon (`proficient: false`, meaning the proficiency bonus is left off every weapon this placement lists; damage keeps its ability bonus, which never came from proficiency): the shape is fixed here so the field can be added without a design, and it is NOT built by this slice. A per-weapon flag was considered and rejected as impractical to author. A definition-owned training set (categories and specific weapons, like a character's sheet) was considered and set aside: it answers a question nobody has asked yet, and the default-plus-switch covers the case that was.
8. **Minds are untouched.** A coward with a bow keeps its room and shoots. A mind says what it cares about, not what it is armed with (the MindCoward doc already says this). **CORRECTED by the build:** the original text said a berserker with only a crossbow still closes. It does not — the ladder ATTACKS (rung 1) before it moves TOWARD (rung 2), so any monster with a ranged weapon in reach shoots instead of closing. The SRD thug now shoots from across the hall unless its placement says `actions: ["dnd5e:weapons:mace"]`; that is the tool doing its job, and which the reference dungeon ships is Kirk's ruling. The two knobs — mind on the definition, weapons on the placement — are the story an author tells together.

## Components

### rpg-toolkit — rulebooks/dnd5e (root)

- The weapon-to-attack assembly moves to where a monster can reach it. Today it is `character.assembleWeaponAttack`, over a `*Character`. The slice extracts the wielder-facing part behind a small interface (ability modifier, proficiency bonus, proficiency-with-weapon) so `character` and `monster` both call one assembly; the character path's behaviour is pinned by its existing tests and must not change.
- `monster.Monster` gains `AddWeapon(weapons.WeaponID) error` (name to taste at build time; the door assembles and appends). The monster answers the assembly's proficiency question YES for every weapon (decision 7); the seam is the interface, so the reserved `proficient: false` switch lands as one field carried to that answer, not as a second assembly.
- `monsters/*.go`: skeleton, goblin, thug re-authored as weapon refs; a bandit definition added (CR 1/8, SRD scores, scimitar + light crossbow) as the proof that a new archer costs a stat block and two words.
- The `monster_actions` ref namespace loses its weapon members (`skeleton-shortsword`, `skeleton-shortbow`, `goblin-scimitar`, `thug-mace`) — an action's ref is now the weapon's ref, as it is for a character. Non-weapon actions keep the namespace. Check every reader of those refs (encounter `field.go` mentions one in a comment; tests pin several).

### rpg-toolkit — rulebooks/dnd5e/encounter/dungeonspec

- `Placement.Actions []string` (`yaml:"actions,omitempty"`), monsters only, refused on props the way `Targeting` is. Carried as refs, validated at compile against the weapons catalog and the monster's authored non-weapon actions; an unknown ref is a field error naming `place[i].actions[j]`.
- Compiled into the spawn instruction beside `Targeting` and `Holds`.

### rpg-toolkit — rulebooks/dnd5e/session

- `SpawnInput.Actions []string`. Non-empty replaces the instantiated monster's action list with the named ones, in order, assembled through the catalog; empty keeps the definition's list. Unknown ref → `ErrUnknownContent`, the sentinel a bad monster ref already returns.
- Proof: spawn a goblin with `[shortbow]` and one with `[scimitar, shortbow]`; the first has one action whose ref is the shortbow's and whose numbers are +4/1d6+2; the second lists the scimitar first. Mutant: drop the ordering and show the second goblin shoots when adjacent.

### rpg-api

- `sessionworld` forwards `Actions` beside `Targeting` and `Holds`. Shipped content: `content/reference-minds.yaml` gains the two goblin archers for the walk.
- Authoring service, additive: a `ListWeapons` (or a `weapons` field on an existing descriptor response) so the palette can offer names without holding the rulebook's vocabulary in the web — the same reason `ListScenarios` exists. One proto PR, `rpg-api-protos`, deprecate-don't-break.

### rpg-dnd5e-web — builder

- Per placed monster, a list of its actions from the palette, reorderable, with the definition's default pre-filled. A "ranged / melee / both" chip that writes the list and is not stored. Ships last and can trail the walk; the yaml is authorable by hand from the first toolkit tag.

## Order of work

1. rpg-api-protos: the weapon list door (additive). Merges first; a proto is a contract.
2. rpg-toolkit, one PR per module on pseudo-versions: root (assembly + monster door + re-authored definitions + bandit) → dungeonspec (`Actions`) → session (`SpawnInput.Actions`).
3. rpg-api: forward the field, serve the list, ship the two archers in The Three Minds.
4. Bring up `envs/local/mind.env`, Kirk walks, THEN merge inside-out and repin.
5. rpg-dnd5e-web palette, on the released tags.

## The walk

In The Three Minds: two goblins at the entrance, both cowards. Walk a player up to each. Both step away and shoot while they have room. Corner the one with `[scimitar, shortbow]` (blade FIRST — a bow reaches 80 ft, so bow-first could never swing): it swings. Corner the one with `[shortbow]`: it shoots point-blank, because no rule here says it cannot, and that is the author's choice showing on the board. Regression: the skeleton behaves exactly as walk 4 recorded; its numbers did not change, only where they come from. The thug is NOT a clean regression (see decision 8).

## Unpaid, on purpose

- Hands, stow/draw cost, two-handed grip, off-hand: none of it. The action carries the weapon ref so any of these can read it later.
- The retaliator excusing a monster: it reads a character's hands today and keeps doing so. Symmetry arrives when a scenario asks for it.
- `proficient: false` on the placement: the seat is reserved in decision 7 and the shape fixed; it is built when an author reaches for it.
- Loadout presets by word (`archer`, `blade`): a new word per tune is the cost #1745 just paid; a placement lists weapons instead. If the palette wants presets, they are palette-side templates over this list.
- Non-weapon authored actions (claw, bite, multiattack) keep their per-monster refs; nothing here changes them.

## Open items

- Name of the monster door and of the shared assembly package — builder's call, recorded in the PR.
- Whether `monster_actions` keeps a weapon-shaped member anywhere after the re-authoring; the builder lists every reader before deleting.
