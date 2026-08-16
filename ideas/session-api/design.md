# API session integration — design

**Status:** PROPOSED (approved in-session by Kirk 2026-08-15; ratifies when
this PR merges). Issue: rpg-project#227. Why: `brainstorm.md`. How: `plan.md`.

## 0. The world-model ruling — RESOLVED 2026-08-15

The toolkit-lane census (on #227) surfaced a fork this design refused to
decide silently: the new composition is **room-first** (rooms, portal
connections, room-local positions, `Traverse` as a verb) while the ratified
dungeon-builder target and the live authored dialect are **one-canvas**.

**Kirk ruled:** *"the encounter has rooms but projects the absolute geo of
the dungeon so the session package sees it as all one map."* Rooms remain
an internal structure of the encounter composition; the seam projects
dungeon-absolute geometry; the session package — and therefore this proto
surface — speaks **one map**.

Consequences, now normative:

- **No `Traverse` RPC.** A doorway crossing is an ordinary move step on the
  wire. The seam's current `Traverse` verb retires with the SDK's
  absolute-projection reshape.
- **Every wire position is dungeon-absolute.** Room IDs do not cross the
  seam; semantic scoping stays the dungeon-builder model (regions on a
  canvas).
- **Door state and locks are unaffected** — the fork-independent gap
  (census gap 4: the reference tomb's locked DC-12 connector) arrives with
  its own capability work.

W1 still freezes nothing for move/position/read shapes until the SDK seam
projects absolute geometry (retiring room-local outputs and `Traverse`) and
the self-position read lands (rule 11, toolkit#933) — see plan W1
preconditions. The stream, errors, fight verbs, creation, coexistence, and
cutover were never gated.

## Scope

A new proto surface mirroring `rulebooks/dnd5e/session`, a thin rpg-api
implementation of it beside the existing path, a parallel web implementation,
and a cutover that deletes the old encounter path. Non-goals: any capability
the SDK does not have today (monster attackers, action economy, defeat
endings, stealth); DM/authoring surfaces; changes to lobby or character
protos.

## 1. Proto package

`dnd5e.api.session.v1alpha1`, service `SessionService`, in rpg-api-protos —
additive (no change to existing packages). RPCs mirror the SDK verbs:

| RPC | SDK verb | Kind |
|---|---|---|
| `Join` | `Join` | roster |
| `Exit` | `Exit` | roster |
| `Move` | `Move` | free roam (path, not cell; doorway crossings are ordinary steps per §0) |
| `Attack` | `Attack` | fight (character attackers only in v1) |
| `Turn` | `Turn` | fight |
| `EndTurn` | `EndTurn` | fight |
| `Dissolve` | `Dissolve` | fight (cause required) |
| `End` | `End` | declared external endings |
| `GetStatus` | `Status` | read |
| `GetStory` | `Story` | read — the resync source of truth |
| `GetView` | `View` | read (sightings) |
| `GetAtlas` | `Atlas` | read (static; client caches per encounter) |
| `StreamEvents` | `EventStream` | server-streaming |

`StartSession` and `Spawn` are **not** exposed: creation belongs to the lobby
(rule 5), which calls the manager in-process.

## 2. Rules

1. **MUST: proto messages mirror the SDK's exported input/output types
   field-for-field.** rpg-api invents no vocabulary. A proto field with no SDK
   counterpart is a design change and goes through this document first.
2. **MUST: a verb's response describes only the caller's own action.**
   Everything else — monsters acting, other members moving, the world waiting —
   reaches a client only through `StreamEvents`, single-player included.
3. **MUST: the proto `Event` mirrors `session.Event`** (`session`, `seq`,
   `at`, `correlation`, `recipient`, `kind`, `payload` bytes). The SDK
   designed it flat and non-polymorphic for exactly this mapping. `kind`
   mirrors the SDK's `EventKind` values; adding a kind is compatible.
4. **MUST NOT: rpg-api filters or re-derives visibility.** Events arrive from
   the SDK already projected per audience; `StreamEvents(session, member)`
   delivers events whose `recipient` is that member, verbatim. A host that
   filtered would be reimplementing perception, and its first mistake would
   leak fog of war.
5. **MUST: creation is the lobby's.** `LobbyService.StartEncounter` re-points
   internally to `StartSession` + `Join` per ready player + `Spawn` per
   seeded monster. No creation RPC on `SessionService` v1.
6. **MUST: delivery is best-effort; the story log is truth.** `seq` is
   monotonic and gapless per session; a client that notices a gap re-queries
   `GetStory` from its last known value; `ErrStoryTrimmed` means resync from
   zero. The stream carries no replay obligation.
7. **MUST: one tested error-translation table.** The SDK's sentinel vocabulary
   maps to gRPC codes in a single place (e.g. `ErrNoSession`/`ErrNoMember` →
   NOT_FOUND, `ErrBrokenPath`/`ErrBadPosition`/`ErrEmptyPath` →
   INVALID_ARGUMENT, `ErrInBubble`/`ErrNotInFight`/`ErrClosed` →
   FAILED_PRECONDITION, `ErrSessionExists` → ALREADY_EXISTS, `ErrSaveFailed` →
   UNAVAILABLE with retry guidance from the SaveReport). Every exported
   sentinel appears in the table; unmapped errors are INTERNAL.
8. **MUST NOT: any rule in rpg-api** (the Boundary Rule). Handlers are pure
   translation with zero rulebook imports (depguard-enforced, as today);
   the orchestrator loads, calls a verb, returns.
9. **MUST NOT: any path between `SessionService` and the old encounter
   stack.** The two never call, import, or share state with each other.
10. **MUST: capabilities are supplied, never defaulted** (toolkit law from
    resolution#1033): `Config.Dice` and every repository are wired explicitly
    at construction; construction is total.
11. **MUST: a cold client can learn its own position from reads alone.**
    Today no SDK read answers "where am I" — `View` skips self and sight
    does not cross doorways (a structural consequence of separate room
    containers, not a ratified rule — the ADR behind it was never accepted),
    `Status` lacks member positions (toolkit#933) —
    so a reconnecting client cannot place itself even with a full story.
    The SDK-side fix lands before W1 freezes the read shapes; the proto
    read (`GetStatus` or successor) transcribes it.

## 3. rpg-api shape

- `internal/handlers/dnd5e/session/v1alpha1/` — proto ↔ SDK translation only.
- `internal/orchestrators/session/` — owns the one `session.Manager`;
  implements `SessionRepository`, `EncounterRepository` (new Redis key-value
  stores; blobs are opaque, round-tripped never built), `CharacterRepository`
  (adapts the existing character repo), `EventStream` (publishes to a broker
  keyed by session+recipient, which `StreamEvents` subscribes to).
- **Coexistence:** while both stacks exist, server configuration selects
  which stack `StartEncounter` creates on — exactly one, never both. Local
  dev runs the new stack; the deployed default stays old until cutover.

## 4. Web

New game route consuming `SessionService` + `StreamEvents`, rendering server
truth, branching on `kind`, resyncing via `GetStory` on gap or reconnect.
Parallel implementation starts when the protos are decided (Kirk). The old
route keeps working until cutover. UI/UX lane owns the shape beyond this.

## 5. Cutover (= the rip-out)

Precondition: acceptance criterion 1 verified live in the local stack.
Deletions, one coordinated swap: web old game route; rpg-api old
`EncounterService` handler (~17.8k lines), encounter orchestrator v2,
encounters/v2 repo, old integration suite, dev seed fixtures for the old
stack; the `dnd5e.api.v1alpha2.encounter` proto package (in-place removal per
the versioning trigger — every consumer moves in this swap). The unwired
legacy components (`internal/components/dungeon/`, `spawner/`, ~11k lines,
unreachable from the server binary) are deleted earlier, in the rpg-api wave.

## 6. Acceptance criteria

1. A party can, entirely through `SessionService` against the local stack:
   create a lobby → start → **the shipped reference tomb compiles into a
   runnable new-stack world and a player walks entrance → hall → tomb**
   (doorway crossings as ordinary move steps per §0) → sight forms a fight → take
   turns → a character attacks a monster and damage applies → dissolve the
   fight → disconnect and resume, learning own position from reads (rule
   11) plus story resync. Verified live, evidence on the implementing PRs.
2. After cutover: zero old-stack imports in rpg-api; `go.mod` pins the new
   toolkit stack; the old proto package is gone from rpg-api-protos.
3. Handlers carry zero rulebook imports (depguard) and no visibility logic.

## 7. Ownership

Platform lane: rpg-api-protos + rpg-api. UI/UX lane: web. Director: triplet,
sequencing, verification. One branch per repo per wave; the consumer defines
done.
