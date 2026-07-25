# Active handoff — 2026-07-24 (late)

> Living handoff — **rewritten, not appended.** If it's longer than a screen it's
> failing. Conventions and how-we-work live in `CLAUDE.md` + auto-loading memory;
> this file is only **direction**: what we're chasing and what's live.

## North star

A multiplayer D&D 5e dungeon crawler playable as a Discord Activity. Right now the
push is making an authored multi-room dungeon actually **playable and legible** —
real monsters instead of placeholders, game state you can read at a glance, and
characters that move like characters.

## Now (in flight)

- **Assets — crypt monster roster:** `rpg-game-assets#28`. 7 undead + downed
  variants promoted, forward axis measured (+Z), root-wrapper verified, mesh-stats
  clean. **The standing idle is defective** — see Don't. Downed/corpse poses are
  good.
- **Web — monster models:** `rpg-dnd5e-web#594`. Resolver, facing constant, plumbing,
  tests; CI green. **DO NOT MERGE AS-IS** — the alive-monster placeholder gate was
  specified but never landed, so merging would mount T-posing skeletons.
- **Process:** `rpg-project#125` (three-lane ownership + built-in DoD), `#126`
  (first retro), `game-dev#5` (pointer to the role system). All open, all ready.

## Next (impact-ranked)

1. **Asset team learns ARP in Blender** — produce monster **default stances, idle,
   and movement** to the same standard the player characters already have. This is
   *the* blocker for living monsters looking right. Kirk has driven ARP
   interactively before (fighter + 3 classes, `rpg-game-assets#27`); the technique
   exists but is not yet a repeatable pipeline step — no `.py` calls
   `bpy.ops.arp.*`, only `.blend` checkpoints. ARP exposes 167 headless operators,
   so it *can* become one. Open question only Kirk can answer cheaply: does ARP's
   existing remap carry across rigs with identical bone names (the undead rigs match
   the Fantasy Rivals rig byte-for-byte in bone naming)?
2. **Corpse presentation window** (client-side) — `rpg-dnd5e-web#471`. Unblocks the
   downed variants already built and shipped.
3. **Unconscious character can move in FREE_ROAM** — `rpg-toolkit#845`.
4. **Weapons visible on class models** — equipment is resolved and then dropped;
   `ClassCharacterModel` accepts no weapon props, so a greataxe fighter renders
   empty-handed. Rigs already expose `Hand_R`/`Hand_L`; 17 weapon GLBs already
   promoted; per-class socket transforms already published in the manifest.
5. **Skating** — `rpg-dnd5e-web#593`. Characters cross ~2.4 hexes per walk cycle.

## Don't (decided, or traps — don't re-litigate)

- **Don't fix corpse-lingering in the toolkit.** `EntityDied` → `EntityRemoved`
  published synchronously is *correct rules behaviour*; how long a corpse stays on
  screen is presentation, so the window belongs client-side (Boundary Rule).
- **Don't name npc assets by rules ref-id.** No toolkit ref exists for
  `ghost`/`specter`, and Skeleton maps to 2+ looks. Asset-source filenames, with the
  rules mapping carried in the manifest's `rulesRef`.
- **Don't accept "the clip animates" as "the clip poses the character."** `Idle_Base`
  has real motion but stays within **1.95° of the bind pose** — a T-pose on screen.
  The gate is *max angular divergence from bind pose*, not clip presence.
- **Don't infer a rig's forward axis from mesh bbox data** — it ignores the node
  hierarchy and gives wrong answers. Render it. (Every rig measured so far: **+Z**.)
- **Don't trust a worktree's local tooling.** A worktree has no `node_modules` and
  silently resolves the parent's, which drifts from the lockfile. `npm ci` first.
- **Don't open draft PRs** — they skip Kirk's review queue.
- **Don't re-derive what Kirk knows.** Ask him. He built this pipeline.

## Not updated this session

**Platform lane** — last known state 2026-07-20: Slice 2 (two-chamber dungeon)
delivered and deployed, partial sign-off, `rpg-dnd5e-web#562` (walls don't render on
the default Synty dungeon) the outstanding gate. **Refresh before relying on it.**
Dungeon-authoring YAML work (`rpg-project#117`/`#121`) is in flight separately.
