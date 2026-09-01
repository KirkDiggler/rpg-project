# Run Readiness and Activation Story — Implementation Decisions

## Slice B: first-admission LongRest persistence ordering

**Date:** 2026-09-01  
**Issues:** rpg-project#341, rpg-toolkit#1396

### Approved design before implementation

The first-admission Join design originally treated LongRest as tentative until projection, placement, and discovery checks had all succeeded. The rested character would be saved immediately before encounter commit, so a failed placement would leave both character and encounter unchanged.

### Implementation discovery

Session placement is not a closed local operation. `encounter.Join` may refresh sight, consult `standingSeam`, form combat, and drive a monster turn. Those callbacks read character records through `CharacterRepository`. Holding the rested record only in a Join-local variable therefore creates two simultaneous truths:

- projection and the eventual Join response see the rested record;
- standing/cast/driven-resolution callbacks see the old persisted record.

A zero-HP admission can consequently be observed as down during placement, and a driven result can be overwritten by the later rest save. Session commit preparation also has fallible stream-numbering work after placement.

### Final approved behavior

LongRest persistence is an independently valid between-runs transition:

1. On first-ever Join, Session calls `resolution.LongRest`.
2. Session immediately persists the rested `character.Data` and records `character:<id>` as written.
3. Projection, placement, standing/cast callbacks, fight formation, and driven turns all read the same rested repository truth.
4. Session commits the encounter normally.
5. If any step after the character save fails, the rest remains durable and the returned `SaveError`/`SaveReport` names every earlier write.
6. Retry is safe because LongRest is idempotent and a failed placement does not persist `EverMembers`.

### Rejected alternative

A transaction-style character overlay on `writeScope` was considered. It would require every standing, cast, striker, announcer, check, concealment, and dirty-save path to read and mutate staged records before a final flush. That broader Session transaction system is disproportionate to the current rule and obscures the future removal point.

The chosen early-save block is explicit and temporary. When town play introduces an in-world LongRest action, remove the first-admission `resolution.LongRest → SaveCharacter` block from Join; no Session-wide transaction infrastructure needs to be unwound.
