# The first item consumer exposes the grant seam

Written: 2026-11-04. Work: [Keep signature combat and loot S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [Normal recovery as configuration](2026-11-04-normal-recovery-as-configuration.md).

## Question

Can a separate item consumer compose the existing effects tools, and what must
that consumer provide? The operator selected one charge, then pointed out that
we were discussing final configuration checks instead of investigating the tool.
The next step became a concrete seam probe, not another item-setting question.

## Evidence

[Toolkit PR #2273](https://github.com/KirkDiggler/rpg-toolkit/pull/2273) adds
[test-only `examples/items`](https://github.com/KirkDiggler/rpg-toolkit/tree/6169f235/examples/items).
It consumes real pushed-commit pseudo-versions of `examples/effects`,
`mechanics/resources` and the D&D module's `healing` package, all from toolkit
main `3182088c`. No local source override, provider change or public item API.

The consumer fixture supplies two output kinds to `effects.Compose`:
`grant_healing` attaches a local inert offer; `recover_pool` attaches a targeted
rest listener. The item owns its resource; `healing.Resolve` returns a sourced
calculation. HP mutation and production action economy are not implemented.

At probe head `6169f235`, from `examples/items`:

```text
go test -count=1 -race ./...             PASS: eight suite cases and one example
go test -count=1 -run Example -v ./...   PASS: expected transcript
go vet ./...                           PASS
golangci-lint run ./...                 0 issues
```

The level-1/level-3 cases produce 7/9 from a controlled d10 result of 6. Two
same-definition rings spend independently and identify their own source item.
Transfer/save-load preserve the spent budget. A matching short or long rest
restores it; another holder's rest does not. A long-only variant tests authored
policy without selecting a new rule for the first ring.

The executable example verifies:

```text
source: ring-17; healing calculation: 9
charges after use: 0
charges after transfer: 0
charges after old holder rests: 0
charges after current holder rests: 1
```

A targeted mutation removed the recovery handler's holder filter. The transfer
check then failed because the old holder's rest restored the ring; replacing
the filter returned the suite to green. The initial incomplete scaffold failed
to compile before fixture helpers existed; that is not a provider defect or a
behavioral RED test. One line-length lint issue was fixed before publication.

## Interpretation

The existing effects composer can host this consumer wiring, but the useful
finding is what it does **not** supply:

- Both rings receive `example:items:second_wind_ring#present.0` as the output ID.
  That identity is local to the recipe/state/output. The item consumer must carry
  instance context; a content ref cannot silently become an instance ID.
- The effects tool owns output validation and attachment, not an existing D&D
  effect implementation. Consumer vocabulary still supplies the meanings.
- Item pool state is outside the machine bag, and its updates do not trigger the
  effects machine's Changed hook. Reporting and persistence remain consumer work.
- The local healing offer is not `actions.Definition`. The real grant/availability,
  payment, resolution and HP-delivery contract has not been demonstrated.
- One attached state suffices. This probe does not need machine transitions and
  supplies no argument for extracting a shared machine.

The test fixture associates a holder explicitly. That is not a decision about
worn/carried/attuned items or detached-item recharge. Its refund after a failed
pure calculation is not a general transactional guarantee. No class-pool
coexistence or native acquisition is proved by two-item independence.

## Next

Review the candidate grant/activation seam: what reusable declaration should
item, class or other composers expose, and which existing consumer should accept
it? Use the working probe to locate that contract, rather than promoting its
private healing offer or adding a fake spell profile. Keep the passing wiring as
evidence and the remaining production boundaries explicit in the Proposed ADR.
