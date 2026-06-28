## DR-010 · 2026-06-28 · Host-observation edge: StrikeResolved notification + CombatantView snapshot
Seam/primitive:  the host↔encounter **observation edge** — how a host both reads
                 all combatant state and reacts to each resolved strike.
Decision:        two host-facing surfaces (both plain tumult structs, no host types).
                 (1) `strike(...)` publishes a `StrikeResolved` notification on the
                 encounter Bus **after** it mutates state, and **still returns
                 `StrikeResult`**. The return is the caller's synchronous answer
                 (pull); the event is the broadcast observers subscribe to once and
                 react to each loop iteration (push). `StrikeResolved` carries
                 `{attackerId, targetId, StrikeResult result}` — `StrikeResult` is
                 reused as the single outcome shape. Uses `Topic<T>` (notification
                 flavor), the same primitive already used for `turn.ended`.
                 (2) a snapshot read-model accessor returns `std::vector<CombatantView>`
                 by value, `CombatantView{id, name, curHp, maxHp, alive}`;
                 `findCharacter(id)` stays for point lookups. "Dead" is the `alive`
                 flag (`curHp == 0`), not an event.
Rejected:        - **Return-only (no event):** a HUD / combat-log sink would thread
                 the return value through the loop instead of subscribing once;
                 misses the named combat-log-sink demand (rpgkit-ue#8). Push and pull
                 serve different consumers — not redundant.
                 - **Event-only (no return):** breaks the direct caller's synchronous
                 answer and the A1 `host_consumer` + tests that assert on
                 `StrikeResult`; fail-fast bus delivery makes a single answer awkward
                 to extract.
                 - **Reuse `DamageEvent` for the outcome:** `DamageEvent` is the
                 *request* shape (`{attackerId, targetId, baseAmount}`) on the
                 combat.damage / raw / block topics; reusing it conflates request with
                 result. `StrikeResolved` is a distinct outcome event.
                 - **Live `const Character*` / per-id getters as the read-model:**
                 leaks internal state (the `block` field), couples HUD lifetime to the
                 Encounter (dangles after `shutdown`), and forces the host to pre-know
                 ids (no enumeration). A value snapshot is lifetime-safe, curates
                 display fields, and forward-fits the Group L N-combatant roster (host
                 already iterates a list).
                 - **A `DefeatedEvent` now:** A2's "dead" need is met by the `alive`
                 flag; a death event waits for the Group L trigger demand (enemy
                 intents / death triggers).
Why:             makes "what is everyone's HP now?" and "who caused this number?"
                 answerable at the edge through observable surfaces rather than
                 threaded returns — the "receipts as primary observability" thesis
                 reaching a host. Mirrors the established tumult mirror-not-wrap
                 pattern (`CombatantView`:`Character` :: `BreakdownStep`:`Chain::Step`,
                 T3) and the `turn.ended` notification precedent, so it adds **no new
                 primitive kind**.
Interface delta: - New: `struct StrikeResolved { std::string attackerId;
                 std::string targetId; StrikeResult result; };` + a
                 `TopicDef<StrikeResolved>` bound on the Encounter Bus (notification
                 flavor); `strike()` publishes it after mutation — return type
                 unchanged.
                 - New: `struct CombatantView { std::string id; std::string name;
                 int curHp; int maxHp; bool alive; };` + `[[nodiscard]]
                 std::vector<CombatantView> Encounter::combatants() const;`.
                 - Unchanged: `StrikeResult strike(...)` return; `findCharacter(id)`;
                 rpgkit (no bump — `Topic<T>` already shipped in v0.3.0).
                 Exact identifier / topic-id naming is the executor's call; the shapes
                 above are the contract.
