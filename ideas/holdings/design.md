# Holdings — a member's own knowledge is not a view of the world's

**Status:** the ruling below is settled. The change it implies is small and
stated in full here. Everything else this document once contained was struck —
see *What was struck* at the end.

---

## The ruling

> *"we have a global who is up and who is down and we are referencing that to
> maintain what I see. i think that's the break. the global table is fine, but
> my player would never actually know the whole table and it should not feed
> into my intel. if i dont know someone is down, i dont know it."*
>
> *"if I have a holding on goblin-1 I do not get any info from the world ledger
> on that."*
>
> — Kirk, 2026-09-12

**Vocabulary:** the **ledger** is the world's truth; **holdings** are what one
member knows. `play/intel` already owns holdings and does not change.

---

## The defect

`session/read.go:370` computes standing across the whole roster and stamps it
onto every one of a member's sightings, live or memory. So a ghost reports the
world's current standing rather than the standing its observer last saw.

Position and equipment already come from the testimony. Standing is the one
that was never moved — rpg-toolkit#1668, whose own body records that `Name` and
`Kind` were *decided* not to be perception facts while `down` was simply
inherited from before testimony existed.

## What #1668 is, whole

1. `refreshSight` takes the participation reading **once at its top** and hands
   it to both `rebuildPercepts` and `applyTrigger` → `noticeDown`. Net consults
   unchanged; `noticeDown` takes one today.
2. `rebuildPercepts` writes `Down` into `SightTestimony`. The field already
   exists and is already `*bool`, because "not observed" and "observed upright"
   are different claims.
3. `projectSeen` reads standing off the testimony instead of the live map, and
   its doc paragraph beginning *"STANDING IS STILL THE LIVE ANSWER"* is deleted
   rather than amended.
4. `projectSightings` drops the `down` parameter.

**The walk:** see a monster, break line of sight, have it go down — your ghost
should still show it on its feet.

**Wire:** unchanged. `STANDING_UNSPECIFIED` already exists in the proto enum.
No protos PR, and no rpg-api code change — but rpg-api needs a **pin bump PR**,
which is also what carries the walk.

---

## Known, not designed

Three further defects are real and recorded, with no design behind them yet:

- **Beats reach everyone.** Kirk's rule for it: *"writing is happening wherever
  it is. if I have a holding on it then I get the event."* That needs
  `HoldersOf(subject)` in `play/intel` — the inverse of the existing
  `HeldBy(observer)`, the same map read the other way, and the one thing intel
  is genuinely missing. rpg-toolkit#940.
- **Walk testimony is batched.** `walkPath` beats per cell; `settleWalk`
  testifies once, so a fleeing monster ghosts on its START cell.
  rpg-toolkit#1670.
- **Ghosts are never disproved.** `correctArrivedLocations` flips a remembered
  location to unknown, but only for a driven move, only for the mover, only at
  the exact cell it landed on. Looking somewhere and seeing nothing is
  information we discard. Unfiled.

`encounter/projection.go` filters structure (doors, regions) at read time,
which is the same need solved the other way round. Noted, not judged.

---

## What was struck

This document previously proposed an `Actual` world-row type, a god observer
inside `intel`, write-on-change delivery, and a four-way fork over participation
ordering. All of it was scaffolding for a first-contact problem that does not
exist — Kirk: *"how does a goblin up vs down on first sighting differ?"* It
does not. You look, and you see whether they are on their feet, exactly as you
see where they are.

It also claimed `names` and `kinds` were unfiled defects. They were decided,
with reasoning, in doc comments on `Sighting`.

Left visible rather than quietly rewritten, per this repo's own rule. The
process failure behind all of it is rpg-project#442, which is where that
conversation belongs rather than here.

**This is a record of a fix, not a design.** It describes the mechanism we
have. A design that starts from what we *want* should not start from this file.
