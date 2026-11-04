# What should the machine foundation let a designer express?

Written: **2026-11-04**.
Work: [NPCs that feel alive — #859](https://github.com/KirkDiggler/rpg-project/issues/859).
Discussion: [toolkit exploration PR #2271](https://github.com/KirkDiggler/rpg-toolkit/pull/2271).

This entry starts the investigation's use of the work journal. The earlier
conversation, source inspection and test receipt below are a **Retrospective
reconstruction** of work earlier on 2026-11-04, written today. They are not
new experiments or a claim that a journal existed throughout that work.

## Question

**What should an encounter designer be able to connect, and which part of that
belongs in a machine primitive?**

The desired delivery is NPC behavior that feels coherent: a guard has a duty,
responds to information it actually received, changes what it says or does,
and carries the consequences forward. The goal is not merely to consolidate
two implementations. It is to give a designer reusable ways to build encounters
without a new engine branch for every story.

The [keep's intended experience](../../../ideas/keep-dungeon/design.md) supplies
a concrete customer. Arresting guards later recognize the party in the
ballroom, while strangers do not. A passing pursuer wakes sleeping guards and
tells them the news. Heard gossip makes a new conversation possible. Those
connections matter as much as any individual transition.

If the tools compose well, they could also unlock:

- Duties that coexist with temporary behavior: searching while moving
  stealthily, or dealing with a disturbance without losing the original job.
- Conversations whose availability and answers depend on the listener's own
  memories, not only the player's knowledge or a global encounter flag.
- Authored propagation of reports and consequences, with different recipients
  reaching different conclusions.
- Memory policies that let a designer distinguish one noise from repeated
  disturbances, or a witnessed event from somebody else's account.
- Recovery of interrupted multi-step work without duplicating its completed
  consequences.
- A possible common foundation for effect lifetimes and NPC behavior, if both
  consumers demonstrate the same contract rather than merely similar code.

These are desired or possible capabilities, **not implemented deliverables**.
The agreed investigation route is to shape `mind/machine` in place before
considering extraction. The effects implementation stays separate. Delivery
scope and eventual slices remain in [#859](https://github.com/KirkDiggler/rpg-project/issues/859),
not in this journal.

## Evidence

### Supplied direction, not runtime observations

The operator's thinking-aloud proposals and request to adopt this journal are
captured in the [2026-11-04 discussion record](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6643743069):

- Delivered intel can trigger an interested behavior's wake path. Patrol,
  search and sneaking are distinct behaviors, rather than necessarily one
  exclusive state for the whole NPC.
- Incoming sounds have categories; channel/spatial selection should avoid
  examining every out-of-range NPC listener.
- Designers could control how memories are recorded and queried.
- Designers could order conflicting roles while behavior paths run
  simultaneously. Interruption, retention of progress and arbitration rules
  are not yet selected.
- Recovery matters. Bounded soft locks or reservations, retained rollback
  resources, and forward/backward operation steps were floated as possible
  mechanisms, including asynchronous recovery.

This is evidence of the operator's intent, not acceptance of those mechanisms.
In particular, it does not select an async NPC runtime, a shared package,
a memory schema, or a transaction protocol.

### Retrospective reconstruction — inspected implementation

The earlier inspection used toolkit
[`532f4621`](https://github.com/KirkDiggler/rpg-toolkit/commit/532f46217ef01e51a5f746a87ab14d94a0d22a7e).
The [first exploration receipt](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6642383444)
and [walkthrough at its published revision](https://github.com/KirkDiggler/rpg-toolkit/blob/03b1f5430599e2d910dc9e1784f1676eaa93dc03/docs/ideas/machine-foundation/README.md)
provide the earlier view. The sound/memory details below extend that view with
source read during the subsequent discussion.

| Observation | Source and limit |
|---|---|
| `mind/machine` already steps bus-free, accepts a supplied beat/fact vocabulary, and returns decisions plus listener/due information. | [`step.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/mind/machine/step.go), [`vocabulary.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/mind/machine/vocabulary.go). These interfaces do not establish a multiple-role coordinator. |
| Passage holding is a state binding owned outside the machine recipe. | [`dungeonspec/states.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/rulebooks/dnd5e/encounter/dungeonspec/states.go), `HoldsSpec` / `postsOf`. This demonstrates an ongoing contribution without putting doors in the kernel; it does not deliver the whole admission journey. |
| Machine wake inputs wait for a world-clock raise; the encounter's `react` path excludes actors on fight clocks. | [`wakebeats.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/rulebooks/dnd5e/encounter/wakebeats.go), `matchLanded` / `react`. This is the current binding's policy, not a requirement of `Program.Step`. |
| Dialogue requirements read the speaker's knowledge/held props, and a plain line pays no world round. | [`option.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/rulebooks/dnd5e/encounter/option.go), `requirementHolds` / `Speak`. This does not establish listener-state or listener-memory eligibility. |
| Sound categories already exist: `boom`, `crash`, `clang`, `voice`. Hearing uses partition/door attenuation and passive Perception; `hearersOf` enumerates the roster. | [`noise.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/rulebooks/dnd5e/encounter/noise.go), `SoundKinds` / `hearersOf`. No radius-index optimization or performance measurement was performed in this investigation. |
| Sound reports contain kind, location and time, not maker identity. Only the latest landed sound of each kind is held. | [`sound.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/mind/behavior/sound/sound.go), `Sound` / `Subject`. Repeated-event history is not supplied by that representation. |
| Intel stores observer/subject holdings with opaque payloads, channel provenance and timestamps, and reports changes. Reporting a known subject replaces its held payload. Named learned facts have a separate encounter journal path. | [`play/intel/intel.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/play/intel/intel.go), `Report` / `HeldBy` / `On`; [`encounter/world.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/rulebooks/dnd5e/encounter/world.go), `knowsFact`. Neither is evidence that a configurable episodic-memory policy already exists. |
| Effects has separate trackers, bus-driven transitions, active outputs and terminal cleanup ordering. | [`examples/effects/machine/machine.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/examples/effects/machine/machine.go), [`composed.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/532f46217ef01e51a5f746a87ab14d94a0d22a7e/examples/effects/composed.go). Similar state/guard vocabulary does not prove interchangeable lifecycle semantics. |

The earlier receipt records `go test -count=1 ./...` passing in `mind/machine`
and `examples/effects` (three package results), plus module-map, ADR-index and
walkthrough-link checks. Those are **unchanged-code baselines**, not tests of the
proposed capabilities. No new runtime tests, multiplayer walk, role-concurrency
experiment, hearing benchmark or recovery/failure-injection experiment was run
for this journal entry.

## Interpretation

The earlier walkthrough framed the question around a machine's inputs, state
and consequences. The subsequent discussion suggests that **cooperating
behaviors may be the more useful author-facing unit**. That is a refinement of
the [earlier view](https://github.com/KirkDiggler/rpg-toolkit/blob/03b1f5430599e2d910dc9e1784f1676eaa93dc03/docs/ideas/machine-foundation/README.md#edges-still-to-resolve),
not a decision that every role needs its own machine.

There are plausible alternatives: the current machine plus richer tables and
bindings might suffice; several small machines might need a coordinator; a
richer machine model might eventually be justified. The delivered complexity
and the designer's clarity should distinguish them, not a desire to extract a
package. We have not compared those alternatives through an executable example.

The useful boundaries appear to be:

- **Perceive, remember, respond, act.** A sensory recipient is not necessarily
  an active behavior subscriber. Recording information and waking an interested
  path need not grant movement or an action. The earlier immediate-versus-next-
  tick question needs this finer distinction before it becomes a policy.
- **Categorization versus delivery.** The sound kinds are already present;
  connecting them to machine triggers and bounding recipient selection are
  separate work. A spatial optimization must not silently replace the existing
  door/partition hearing model with a fixed radius or reveal an unseen maker.
- **Storage versus memory policy.** A designer's repeated-noise query may need
  retained history or an aggregate, not necessarily a different store. Recording,
  retention, replacement and conflict handling need explicit ownership. A second
  copy of authoritative knowledge inside every behavior bag is not an answer.
- **Parallel progress versus competing commands.** Memory collection can coexist
  with searching. Patrol and search may compete for movement; sneaking might
  qualify how movement happens. Whether an interrupted path resumes, restarts
  or ends is still open. Author priority alone does not answer all of those.
- **Recovery versus erasing history.** Retaining resources can make a compensating
  action possible, but does not alone make it safe after another writer changes
  them. Reservation/version rules, retry identity, expiry and visible external
  consequences remain questions. A rumor already heard and acted on is not
  equivalent to an uncommitted inventory write.

Recovery therefore looks like a possible cooperating operation tool, not an
automatic responsibility of the transition kernel. The operator's proposed
bounded recovery window deserves examination; it is neither accepted nor
rejected by the walkthrough's refusal to promise arbitrary rollback.

This **may lead to an ADR** if we choose a durable boundary—such as who owns
behavior coordination or recoverable operation execution—after examining real
alternatives and consequences. We do not yet have that choice. The journal can
supply its evidence without creating a competing decision record here. Any
later ADR belongs with the owner of that architectural consequence and links
the design contracts rather than replacing them.

## Next

The next useful probe is a bounded **designer-facing paper trace**, not a new
runtime or implementation plan: a patrol hears a clang, searches, receives a
report about the party while searching, and attempts an authored consequence.
This is a proposed example, not an addition to the canonical keep script.

Follow which information lands, which memory changes, which behavior wants
attention, and what happens when two behaviors want movement. Then interrupt
one consequence and ask what could safely resume or compensate. That should
help distinguish a missing vocabulary word from a coordination or lifecycle
primitive. The existing Rage prototype remains the contrasting consumer;
its semantics cannot be inferred from a guard example.

Record the trace and what changes our view in a later entry, preserving this
uncertainty. Any resulting design choice, bounded code experiment or delivery
slice stays linked through [#859](https://github.com/KirkDiggler/rpg-project/issues/859)
and [#2271](https://github.com/KirkDiggler/rpg-toolkit/pull/2271). This entry neither
selects an ADR nor authorizes a shared-machine extraction.
