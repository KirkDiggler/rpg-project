# Individual dungeon knowledge

## Shape

```mermaid
flowchart TD
    G[Shared fixed room geometry] --> K[Encounter: individual room discovery]
    W[Mutable world state] --> O[Encounter: observation and latest intel]
    K --> S[Session: load, act, save and permitted projection]
    O --> S
    S --> D[API: identity-bound reads and existing event delivery]
    D --> N[Snapshot: known rooms and latest observations]
    D --> R[Room revealed: newly known fixed room data]
    D --> U[Observation updates: creatures, mutable props and door state]
    N --> V[Client: retain known geometry and render current or remembered intel]
    R --> V
    U --> V
    C[Authored appearance content] --> D
    C --> B[Separately authorized builder access]
```

The world, a character's knowledge, and the picture delivered to that character
are different answers. Observation changes knowledge; the client does not derive
knowledge from world truth. The delivery boundary uses the toolkit's disclosure
answer and does not implement another perception algorithm.

The [first-slice proof](first-slice-proof.md) defines the approved behavioral goal;
investigation
and evidence stay on [#508](https://github.com/KirkDiggler/rpg-project/issues/508).

The first-slice goal is **discovering an unknown room without receiving its
layout in advance, then remembering its changeable contents independently**.
Two characters start in room 1 with room 2 unseen behind a closed ordinary door.
Opening the door reveals room 2's complete fixed layout and scenery to the
opener. First observing any part of a room also reveals its fixed layout to that
observer. Internal LOS obstructions do not withhold floor or fixed scenery within
a revealed room. A holdable prop behind an altar remains absent until observed,
like a creature. Withdrawing around a corner leaves an observed prop and door
state remembered. A further closed door at the end of room 2 does not deliver
room 3's interior.

The Reference Tomb (with the heirloom) is an existing walk candidate; a dedicated
listed dungeon is also permitted. The fixture and positions must support unknown
layout, lawful sight separation and pickup without ending the run. Room 2 is not
pre-explored. Secret-door checks remain outside the slice.

One authoritative geometry map includes fixed scenery; it is shared on the
server, not downloaded whole by every player. Room discovery is individual.
Within a revealed room, fixed data remains known and unchanged for the first
slice; visibility may grey its presentation without removing that data. Fixed
scenery does not require a mutable intel record per prop. Creatures, movable or
holdable props and door open/closed state use current/remembered intel. Their
unseen live state cannot replace the character's latest observation.

The snapshot restores known room records and latest mutable observations. A
room-revealed event carries the newly known room's fixed data, not a replacement
of the entire known atlas. Mutable observations update independently through the
creature-style observation/memory lifecycle. Existing read, event and recovery
mechanisms carry these answers; exact DTOs and indexes remain open.

Appearance remains content, not a rendering document embedded in encounter
state. Player delivery selects only permitted content; separate builder authority
may permit the complete authored document. The mechanism and responsibility split
for that selection require a ruling before implementation.

The first-slice endpoint is room-level fixed discovery plus sight-shaped mutable
observation, with undiscovered rooms and unobserved mutable subjects absent from
player delivery. Finer LOS-clipped geometry discovery is deferred. Slice 1 does
not prove secret-door checks or compatibility with arbitrary authored dungeons. A
fixture-only or local-only proof is labeled as such, not shipped as complete
production knowledge protection.

## Law

### Settled behavior and endpoint

- **R1 — Individual knowledge.** A character learns through its own observation or
  discovery. Another character's discovery does not automatically teach it;
  explicit intel sharing is outside this work's first slice.
- **R2 — Discovery is not interior sight.** Finding a concealed door teaches the
  discoverer about that door, not the room behind it. A character who perceives it
  standing open can discover it independently; mutable room contents still
  require their own observation.
- **R3 — Latest knowledge, not history.** Loss of sight preserves the last-known
  observation. New evidence that disproves it replaces the stale knowledge rather
  than retaining an additional historical version.
- **R4 — Absence needs evidence.** An unseen world change does not update knowledge.
  Observing a remembered location empty replaces the disproved location belief
  without disclosing where the object went or who took it. Omission from a percept
  alone does not establish that the old location was observed empty.
- **R5 — Server disclosure boundary.** Undiscovered dungeon content does not reach
  the player's client. Reads, events, replay, reconnect and player-accessible
  content requests obey the same individual knowledge boundary. Hiding delivered
  truth in a renderer does not satisfy this rule.
- **R6 — Slices keep their limits visible.** Every intermediate slice states its
  proof and remaining gaps. Any temporary relaxation of R1–R5 requires its own
  explicit scope and ruling; permission to work incrementally grants no unnamed
  exception.

### First-slice behavioral contract

- **R12 — Unknown-room vertical proof.** The first slice starts in room 1 with
  room 2 undiscovered behind a closed door. Its fixed data is absent until room
  discovery; mutable subjects remain absent until their own observation. A
  visible further closed door does not disclose room 3's interior. The proof uses
  real toolkit discovery/observation, persistence, authenticated delivery and the
  game renderer.
- **R13 — Fixed geometry, mutable intel.** Fixed scenery belongs to the shared
  geometry map, with individual room discovery controlling delivery. Fixed room
  data excludes mutable placements and unobserved door state. Creatures, movable
  or holdable props and door state use current/remembered observations.
  Loss of sight preserves those observations without maintaining a mutable
  memory record for every fixed prop.
- **R14 — Room-level fixed discovery (first slice).** Opening a door into an
  undiscovered room reveals its full fixed layout and scenery to the opener.
  First observing any part of the room also reveals its fixed data to that
  observer. LOS obstructions within the revealed room do not clip that delivery;
  they still gate creature, mutable-prop and door-state observations. Known fixed
  room data remains retained when visibility is lost.
- **R15 — Delivery follows the two lifecycles.** A snapshot supplies known room
  records and latest mutable observations. A room-revealed event carries a newly
  known room's fixed data rather than replacing the entire known atlas. Mutable
  subjects update through the creature-style observation/memory lifecycle,
  independently of room revelation. Reconnect restores the character's own
  knowledge through existing read and event recovery mechanisms.
- **R8 — Existing knowledge mechanism.** The slice composes existing intel
  replacement semantics rather than creating an observation-history store. It
  supplies evidence using encounter-owned perception rules, including range,
  obstruction and applicable sight effects. Every sustained observation pass
  remains complete for its observer and channel.
- **R9 — One permitted view.** Prop and door reads, relevant action responses and
  event payloads agree with the observer's knowledge. The slice's player path
  cannot bypass that answer by fetching authored world truth or selecting another
  character's identity. Appearance selection consumes authorized disclosure;
  neither the API nor the client computes game visibility.
- **R10 — Refresh and reload agree.** Current observation replaces stale state
  without a visible interval in which stale and replacement facts are both
  presented as current. Save/load and reconnect restore each character's own
  latest knowledge; they do not regenerate remembered state from live world
  state. Remembered state is not itself permission to interact with an unseen
  object.
- **R11 — SDK disclosure, host transport.** The session SDK owns the observer and
  permitted-content answer across reads, live updates and replay. The API binds
  authenticated identity and implements transport/storage without adding game
  visibility rules. An SDK stream interface is an investigation target, not a
  requirement to rewrite pub/sub or select Redis as part of this slice.

## Rulings

The settled rows authorize the first-slice behavioral goal and its architectural
boundaries. They do not select the storage schema, content-delivery mechanism or
stream interface; the corresponding Open items need resolution before their
implementation.

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| R1 | settled | Individual knowledge; no automatic sharing | KirkDiggler | 2026-09-30 |
| R2 | settled | Door discovery separate from observing the interior | KirkDiggler | 2026-09-30 |
| R3 | settled | Latest knowledge replaces disproved memory; no history layer | KirkDiggler | 2026-09-30 |
| R4 | settled | Witnessed absence replaces stale location, not unseen truth | KirkDiggler | 2026-09-30 |
| R5 | settled | End-state player disclosure excludes undiscovered content | KirkDiggler | 2026-09-30 |
| R6 | settled | Incremental delivery with explicit compromises | KirkDiggler | 2026-09-30 |
| R7 | superseded | Pre-explored mutable-state proof; replaced by R12 | KirkDiggler | 2026-10-01 |
| R8 | settled | Reuse intel and authoritative observation; exact representation open | KirkDiggler | 2026-10-01 |
| R9 | settled | First-slice read/event/content and identity boundary | KirkDiggler | 2026-10-01 |
| R10 | settled | First-slice refresh/reload behavior and interaction distinction | KirkDiggler | 2026-10-01 |
| R11 | settled | SDK disclosure ownership; stream interface investigation only | KirkDiggler | 2026-10-01 |
| R12 | settled | Undiscovered room 2 absent until room discovery; mutable subjects require observation; further closed door does not deliver room 3 | KirkDiggler | 2026-10-01 |
| R13 | settled | Fixed scenery in shared geometry; creatures, movable/holdable props and door state in intel | KirkDiggler | 2026-10-01 |
| R14 | settled | First-slice whole-room fixed discovery on opening a door or first observing part of the room; mutable LOS remains | KirkDiggler | 2026-10-01 |
| R15 | settled | Snapshot of known records; room-revealed fixed data; separate mutable observation updates; exact DTOs open | KirkDiggler | 2026-10-01 |

## Open

- **Mutable observation support.** Select observation support for creature,
  movable/holdable-prop and door intel without storing competing answers about
  the same placement. The absence witness must describe the area it actually
  observed; partial footprint visibility, concealment and unavailable senses
  cannot silently count as a complete empty observation.
- **Shared geometry.** Choose the explored-geometry representation and its
  association with the content revision. A shared definition does not imply
  shared discovery, nor may a mutable content key rewrite a remembered picture.
- **Player content delivery.** Decide how the toolkit's disclosure answer selects
  appearance content without moving the World Builder's document into encounter
  or implementing visibility in the content service. Specify player versus
  builder authorization and resistance to direct document/identifier requests.
- **First-slice compatibility.** Name the supported authored shape and player
  route. State any interim limitation and its acceptance boundary explicitly;
  neither an unrestricted content fallback nor silent partial room support is
  implied by this draft.
- **Existing creature behavior.** Decide the scope of observed-absence
  replacement for creatures alongside props. Preserve existing observer and
  currency semantics; no creature-behavior change is silently bundled with the
  prop/door proof.
- **Knowledge lifetime.** Distinguish reloading/reconnecting to the same encounter
  from leaving and returning to it, starting a new encounter, or revisiting a
  different revision of a dungeon. The first slice covers reload and
  reconnect only.
- **Delivery payloads and existing stream seam.** Select the snapshot's room
  records, the room-revealed fixed-data payload and mutable observation updates
  using existing DTOs and mechanisms where they fit. Room discovery does not
  require a replacement transport or generic whole-atlas synchronization event.
  Preserve original recipient knowledge in replay; later discovery does not
  retroactively authorize hidden facts in earlier events. Exact payload types,
  ordering, cursors and recovery details remain to be checked against existing
  mechanisms.
- **Scale.** Choose representative geometry, prop and observer counts and a
  measured budget before selecting new indexing or compression.
- **Later slices.** Revisit finer LOS-clipped fixed-geometry discovery, specify
  the secret-door discovery proof, then broaden content delivery and compatibility
  beyond the first fixture.
  Later acceptance includes every player-facing path, not just the first-slice
  fixture.
