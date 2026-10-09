# The seat is visible — slices

Develop outside-in, merge inside-out. One nearest-go.mod module per toolkit PR;
consumers pin only gate-approved heads. Each slice is Sonnet-sized.

## 1. toolkit session — `Manager.Seat`

- `seats.go`: `type SeatInput struct{ Character string }`, `type SeatOutput struct{ Seat *SeatData }`,
  `func (m *Manager) Seat(ctx, *SeatInput) (*SeatOutput, error)`: nil input → `ErrNilInput`;
  empty character → `ErrNoCharacter`; `SeatRepository.GetSeat` → `ErrNoSeat` passes through
  (declare `ErrNoSeat` in errors.go if absent); otherwise the stored data, copied.
- Done when: a launched character's `Seat` names its session; after `Exit` it is `ErrNoSeat`;
  a never-seated character is `ErrNoSeat`; `Seat` writes nothing (repository call count).
- Mutants: Seat returns nil seat for a seated character; Seat creates a seat.

## 2. rpg-api-protos — `GetSeat` (additive)

- `dnd5e/api/session/v1alpha1/service.proto`: `rpc GetSeat(GetSeatRequest) returns (GetSeatResponse)`;
  `GetSeatRequest{ string character = 1; }`; `GetSeatResponse{ string session = 1; }` (add the
  stored fields `SeatData` carries beyond the session id, same names). Buf lint/format/breaking.
- No change to `Exit`.

## 3. rpg-api — handler

- Session handler `GetSeat`: owner gate like `Exit`'s (the caller must own the character);
  `sdk.ErrNoSeat` → `codes.NotFound`; field-for-field. Tests: seated, not seated, not owner.

## 4. web — "Abandon run"

- On a `FailedPrecondition` from `StartEncounter` that names a seated character, and on load
  when `GetSeat` answers a run the lobby does not report: offer **Abandon run** (web team names
  it). It calls `Exit(session, member)` and retries. Walk: launch as A, close the tab, open the
  lobby again, be offered the way out, abandon, relaunch.
