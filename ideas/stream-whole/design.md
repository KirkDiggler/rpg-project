# Slice 3 — the stream is whole

Journey #253 · umbrella #257 · follows #251 (the combat turn) and #254 (the monster's turn).

## 1. Why

Kirk's slice-2 walk (2026-08-23): the skeleton's round-2/3 `struck` beats were in the story log with him in the audience and were never narrated; a round-4 EndTurn failed with `clock is idle`. He asked: "are events coming through now? would multiplayer work?" Nothing in slices 1–2 ever had two human subscribers on one session.

## 2. What is already true (found, not designed here)

- **Toolkit publish path is whole.** One baseline per verb; the whole driven cascade mutates one in-memory encounter and publishes once; seqs contiguous across rounds (7→8, gap 0); `Recipient` byte-equal to the Join member. Pinned by rpg-toolkit#1204.
- **The wire already carries the replay contract.** `Event.seq` gapless per recipient; `StreamEvents` deliberately no-replay; `GetStory{from_seq}` resumes inclusive from any point, zero = everything held, aged-out is an explicit error. Service doc rule 6: a recipient that notices a gap re-queries GetStory. **No proto change in this slice.**
- **The clock contract had an unhit case** — a driven monster's own strike dissolving the fight it is in — fixed in rpg-toolkit#1202 (encounter v0.30.7).

## 3. What is broken

| Where | Defect | Issue |
|---|---|---|
| web | `useSessionEventStream` has no reconnect and no seq tracking; rule 6 is implemented nowhere. A dead stream looks like "some events arrive, one doesn't" because the HUD's transitions come from GetTurn/Afford refetches. | rpg-dnd5e-web#779 |
| rpg-api | `Broker.Publish` drops on a full 32-slot channel with `select … default:` — silently; `DeliveryReport.Failed` can never be true; no per-recipient send trace exists to tell forwarder from client. | rpg-api#819 |
| nobody | Two humans in one session has never been proven: each sees the other's moves/swings; turn order with two players; End Turn hands to the right member; a second subscriber gets its own addressed copies. | toolkit + api tests (this slice) |
| web | No place to read the wire raw. | rpg-dnd5e-web#740 |

## 4. The seam, stated once

The story log is the truth; the stream is a best-effort live copy; **the client owns catching up.** That was the design (S9/S10, rule 6) and it stands. What changes is that every party now honours it:

- **toolkit** — nothing new in the publish path. Adds the two-player proof: a session-level test with two joined characters and two capturing `EventStream` subscribers asserting each beat reaches exactly its audience, addressed per recipient, seqs contiguous per recipient, and EndTurn's `Next` walks player → monster → player correctly. Encounter v0.30.7 pinned into session (one PR).
- **rpg-api** — the forwarder never loses an event without saying so: count + log per recipient and return the error so `DeliveryReport.Failed` is true (ruled: never block the publisher on a viewer). Per-recipient debug send trace on `StreamEvents`. Pin encounter v0.30.7 + the session tag. Acceptance test: two subscribers on one session, one lagging, nothing lost or the loss reported.
- **web** — #779 (rule 6: last seq, reconnect with backoff, `GetStory{from_seq: last+1}` catch-up through the same handler, de-dupe by seq, aged-out → full resync) and #740 (raw feed with seq, stream state visible). Web lane; filed, not built here.

## 5. Rulings needed from Kirk

1. Broker policy on a slow subscriber — **RULED 2026-08-23 (Kirk): drop, never silently.** The stream stays a fan-out; a slow viewer never holds the acting player's verb. The forwarder counts and logs the drop per recipient and returns the error so `DeliveryReport.Failed` is true; the client's rule-6 catch-up (web#779) closes the hole from the story log.
2. Two-player walk: fighter + barbarian, two browsers, same session. Recommendation: yes, that is the gate.

## 6. Gate

Kirk and a second browser in the tomb; a skeleton's full turn and the other player's turn readable end to end from the raw feed on both screens, with contiguous seqs; kill the stream on one browser mid-fight and watch it catch up. Then merge bottom-up: toolkit → api → web.

## 7. Order

1. toolkit: two-player test + pin encounter v0.30.7 (one PR, session tag).
2. rpg-api: #819 + pins + two-subscriber acceptance test.
3. web (web lane): #779 then #740.
4. Walk.
