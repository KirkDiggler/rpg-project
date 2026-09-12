# Holdings — implementation plan

Companion to [`design.md`](design.md). Tracking PR: rpg-project#441.

Rungs 1–3 are **wave 1** and are planned in detail below. Rungs 4–6 are
sketched at the end and get their own plan when wave 1 has landed and we know
what it taught us.

---

## Wave 1 is toolkit-only

Confirmed before planning, because it decides the shape of the wave:

- **`STANDING_UNSPECIFIED = 0` already exists** in the `Standing` proto enum, so
  "standing was not observed" has a wire value today.
- **`name` and `kind` stay where they are.** They were ruled out of perception
  deliberately (design §4.1) — an earlier draft of this plan wrongly called
  them unfiled siblings of #1668. Only `down` moves.

**Therefore: no `rpg-api-protos` PR, and no rpg-api *code* change.** The wire
shape does not change; what fills it does.

**An rpg-api PR is still required** — the pin bump. rpg-api pins the toolkit
modules, so encounter and session minting new versions means rpg-api moves or
`dev` never sees any of this. It is also the PR that carries the walk: pinned
at pseudo-versions while we integrate, swapped to real tags at merge, per
`integrate-before-pr`. See PR 4.

One web check at the end (below), which may be a no-op.

That is a much smaller wave than the design's scope implies, and it is worth
saying why out loud: the previous wave already paid for the wire.

---

## PR 1 — encounter: fold the capability answers into `Actual`

**Module:** `rulebooks/dnd5e/encounter` · **Behaviour change: none** · closes nothing

The reviewable half. A pure refactor, which is exactly why it is its own PR.

1. Introduce `Actual` (design §5.2) and compose it in `rebuildPercepts`
   immediately after the capability asks, replacing the parallel `reach` /
   `hands` maps at the point of use. `reach` is an **observer** property and
   stays keyed separately — it is not a column of `Actual`.
2. Add `standingNow()` to the pass, **beside** `sightNow()` and
   `equipmentNow()`, asked once before the loop. This is the established C8
   pattern, not an exception to it.
3. Introduce `Standing` as a named encounter type (`Up` / `Downed`) rather than
   a bare `bool`, so the third state has somewhere to live in PR 2.
4. **Delete the stale comment at `encounter.go:1984`** and replace it with what
   is actually true. Its claim — that the pass has no participation reading —
   is the thing this PR makes visibly false.

**Acceptance:** every existing test green, untouched. No new beat, no wire
change, no testimony change. If a test had to move, something in this PR was
not a refactor.

**Cost check:** one `Actual` per member per pass. Record the number in
`voidcost_internal_test.go` now, before rung 4 multiplies it by cells.

---

## PR 2 — encounter: the row carries what was observed

**Module:** `rulebooks/dnd5e/encounter` · stacked on PR 1 · closes the encounter
half of rpg-toolkit#1668

1. `SightTestimony.Down` is **written for the first time** — the field already
   exists, and it is already a `*bool` because "not observed" and "observed to
   be upright" are different claims.
2. No new payload keys, so `sightPayloadFields` does not change. `Name` and
   `Kind` stay out of the row by the existing ruling (design §4.1).
3. Older testimony decodes with `Down` nil and says so honestly until the next
   refresh — which is the very next beat.

**Acceptance:** a testimony round-trip test per field, including the
never-observed case. One test that a ghost's standing does **not** change when
the subject goes down out of the observer's sight — the actual bug, asserted at
the layer that owns it.

---

## PR 3 — session: read the holdings and nothing else

**Module:** `rulebooks/dnd5e/session` · pins encounter's pseudo-version · closes
rpg-toolkit#1668

1. `session.Standing` gains a third value for "not observed", projected to
   `STANDING_UNSPECIFIED`. Today it has exactly two (`up` / `downed`), and a
   ghost from before this wave has neither.
2. `projectSeen` reads standing from the testimony, exactly as it already reads
   position and equipment. Its own doc comment currently says *"STANDING IS
   STILL THE LIVE ANSWER, and that is a known defect rather than a design"* —
   that paragraph gets deleted, not amended.
3. `projectSightings` loses **one parameter** — `down`. `names` and `kinds`
   stay: they were ruled out of perception deliberately (design §4.1).
4. `read.go:370` and `write.go:910` stop computing `standingSet` **for
   sightings**. Check each other caller of `standingSet` on its own merits —
   a roster read is a legitimate engine read (design §6) and must not be
   collaterally deleted.
5. Delete the stale C8 paragraph in `projectSeen`'s doc.

**Acceptance:** the defining test — an observer's view of a subject who went
down **outside their sight** reports the standing they last saw, not the
world's. That test fails today for the right reason.

**The rule becomes enforced here**, not merely intended: after this PR there is
no roster join left in the sighting path to leak through.

---

## PR 4 — rpg-api: bump the toolkit pin

**Repo:** `rpg-api` · branch off `dev` · **no code change expected**

1. Pin `encounter` and `session` to the pseudo-versions from PRs 2 and 3 while
   integrating, and stand the walk env up on it. This PR exists from the start
   of the walk, not after it.
2. Swap to the real tags at merge, bottom-up, each tag verified by **content**
   before the next consumer pins it (`newest-tag-is-not-the-merge`).
3. `scripts/bump-toolkit-pin.sh` in `game-dev` does the swap.

**Two traps this PR has hit before, both recorded:**

- **`go.sum` untidy reaching CI.** A pin-only commit stages no `.go` files, so
  rpg-toolkit's pre-commit hook runs nothing and CI is the first thing to
  notice. Run `go mod tidy` and inspect `go.sum` by hand before pushing — the
  hook will not save this commit (`pin-only-commits-skip-the-hook`).
- **Fallout in tests that constructed sightings by hand.** PR 3 changes
  `projectSightings`' signature; anything in rpg-api that built a `Sighting`
  fixture with a standing it supplied itself will now be asserting the old
  model. Fix those to assert what they claim rather than bumping expectations
  (`transcript-tests-pin-too-much`).

**Acceptance:** `dev` green, walk env up, no behaviour change visible except
the one the walk is looking for.

---

## Walk

One local env, one question: **stand where you can see a monster, break line of
sight, have it go down, and confirm your ghost still shows it on its feet.**

That is the whole ruling made visible. Everything else in wave 1 is machinery
for it.

Second check, cheap: a member you have **never** seen has no row at all — not a
row with blanks.

---

## Web

Likely a no-op, verified rather than assumed. The client may have an exhaustive
switch on `Standing` whose `STANDING_UNSPECIFIED` arm was previously
unreachable and now is not. The correct rendering for a never-observed standing
is *"draw no standing indicator"*, which is probably already what an
unspecified arm does. **Check it on the walk; only open a web PR if the walk
shows it wrong.**

`rpg-api` needs no code change — it passes `Seen` through — but it does need
the pin bump in PR 4. Verify the no-code-change half; do not assume it.

---

## Issues needed

Per the repo convention (no branch without an issue), wave 1 needs:

- **rpg-toolkit#1668** exists and covers standing. PRs 2 and 3 close it.
- **No second issue.** An earlier draft asked for one covering name and kind;
  that was based on a mistake (design §4.1). #1668 alone covers wave 1.
- PR 1 is a refactor enabling it and rides #1668.
- PR 4 is a pin bump and follows whatever rpg-api's convention is for those —
  it does not need an issue of its own if pin bumps ride the consumer wave.

Board entries are Kirk's call.

---

## Rungs 4–6 — sketched, not planned

Each gets its own plan. Recorded here only so the ordering is deliberate.

| rung | shape | note |
|---|---|---|
| **4 — per observation** | testimony per cell in `walkPath`, trigger detection still once in `settleWalk`; per-cell deltas accumulated with `mergeIntelDeltas` into one beat per walk | the precedent is in the same loop: `standingNow()` is already asked per cell. **And the beat audience must be computed per cell too** — who could see the mover *before* the step, so the step that takes them out of view is still seen. Closes rpg-toolkit#1670 |
| **4b — negative evidence** | generalize `correctArrivedLocations` from "the cell I landed on" to "any cell I can currently see" | the verb, the payload and the encoder all exist; only the trigger is narrow |
| **5 — channel projection** | a beat degrades by the channel that carried it; hearing writes a **row**, not a stream of lines | closes rpg-toolkit#940 and makes the hearing ladder a table |
| **6 — structure** | doors and props get `Actual`/`Observed` | retires `projection.go`'s read-time filter |

Rung 4 is the one Kirk hit in play (the fleeing monster remembered on its start
cell), so it is the obvious wave 2 — but it is planned **after** wave 1 on
purpose: a ghost on the right cell carrying the wrong standing is still a ghost
that lies.
