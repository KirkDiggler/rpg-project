# Fog of War Handoff

Date: 2026-07-26

Read `design.md` first — it is the contract and it is current. This file only
covers what that document cannot: where the work stands right now, and the
things that cost time to discover.

## Where the durable knowledge already lives

Do not re-derive any of this:

| Question | Answer lives in |
| --- | --- |
| What is the contract, and why? | `design.md` §"The event layer" |
| What order does the work land in? | `plan.md` |
| Which decisions are settled, and why? | `progress.json` — 11 entries, no open questions |
| What did the concept learn about the wire? | `rpg-dnd5e-web/src/concepts/fog-of-war/CONTRACT.md` |
| What does the proto need to say? | rpg-api-protos#197 (rewritten against the proven shape) |

The contract in one line: **the hex is the unit of truth, a visible record is
total, and nothing is ever deleted.** If a proposal needs a removal message,
it has misread the design.

## State as of this date

**Merged**

- rpg-dnd5e-web#602 — remembered *geometry* rendering (crypt tint, key sets).
- rpg-dnd5e-web#614 — remembered *entities* render frozen and inert.

**Open, green, awaiting approval**

- rpg-dnd5e-web#611 — the playable concept. Head `2d67fdf`. All five checks
  pass, `mergeStateStatus: CLEAN`. Closes #605, #606, #613.
- rpg-project#137 — design + plan. Stays open as the tracking surface until
  implementation is done, per the cross-repo workflow.

**Next work, in this order**

1. rpg-api-protos#197 — transcribe the proven shape. Already rewritten; it is
   ready to be worked as-is.
2. rpg-api#725, then #724 — project and translate.
3. rpg-toolkit#850, #851 — supply the authority.
4. rpg-dnd5e-web#609 — swap the data source.

This inverts the dependency order written in rpg-project#147. That order is
still right for *landing*, but the contract is already decided, so the toolkit
goes last: it does not have to invent the answer, it has to reproduce a
transcript we already have. All child issues carry a comment explaining the
supersession.

## Things that cost time — do not rediscover them

**Concepts render placeholder geometry unless you ask for assets.**
`HexGrid`'s `syntyDungeon` prop defaults to `false`. Without it you get the
procedural `ShadedHexFloor`/`ShadedHexWall` path, and since remembered
geometry is a *tint on Synty materials*, fog has nothing to act on. Pass
`syntyDungeon` and `spaceTheme="crypt"`. Run `npm run assets:sync` in the
worktree first — `public/models/synty/` is gitignored.

**Rooms must be rectangles in column/row space.** The wall renderer draws a
room's envelope from its column/row *bounding box*
(`wallRuns.envelopeRunsForRegion`), using the toolkit's parity-corrected
offset: `col = x`, `row = z + trunc((x - (x&1))/2)`. A room authored in some
other axial convention lands staggered across a larger box and the walls do
not fit the floor. `authority/world.test.ts` asserts this now, so it fails in
`npm test` rather than in a screenshot.

**The door cell belongs to no room.** Give it `zoneId: ''` —
`regionInputsFromHexes` skips empty zones. Give it its own zone and it becomes
a one-hex room with an envelope drawn all the way around it.

**An entity with no mapped GLB renders through `MediumHumanoid`**, not
`ClassCharacterModel`. Both need the crypt treatment; tinting only the GLB
path looks correct in review and wrong on screen. `monsterRefId: 'skeleton'`
resolves; `'goblin'` does not.

**Several dev servers run at once on this machine.** Ports 3001–3009 are
frequently taken by other worktrees, and 3001 is often the main checkout — a
URL that answers is not necessarily *your* build. Always confirm the listening
pid maps to your worktree:

```
ss -ltnp | grep :<port>
pgrep -af vite | grep <your-worktree>
```

Run plain `npm run dev` and use the port vite reports rather than forcing one.

## Working rules that bit us

- **Never force-push.** Bring `main` in with a merge, not a rebase.
- `rm` is blocked; use `git clean -f <path>` for untracked files.
- Do not merge PRs — Kirk merges.
- A **seam is a review point, not a merge point**. The concept landed as one
  PR because a third of a concept delivers nothing, and because merging early
  turns a correction into a follow-up PR instead of an edit.

## Open threads

- Frame `03-frozen-memory.png` is current, but the wall-run envelope offset is
  tuned for the authored dungeon's room sizes, not the 3×3 fixture — walls sit
  slightly proud of the floor.
- `CONTRACT.md` finding 4: nothing on the wire carries facing. That may be the
  same need as rpg-dnd5e-web#590 (character facing) approached from another
  direction. Worth reconciling before either is asked for separately.
- The concept runs one viewer. Single-viewer success is *not* evidence of
  multi-viewer isolation; rpg-project#148 still has to prove that.
