# The spike: a C++ client reads the engine's record

Throwaway by design, kept as the recipe. It ran on Linux, on one machine,
without Unreal, on 2026-11-23 and 24, for [../design.md](../design.md) and
idea [rpg-project#1154](https://github.com/KirkDiggler/rpg-project/issues/1154).

**Question.** How much of R1, the Unreal client as a gRPC client of the
running engine, can be proven before anyone installs an editor?

**Answer.** Everything that carried risk: the wire, generated code for the
whole contract, the auth boundary, catch-up-then-live streaming. It also
surfaced what has to be decided first.

## What ran

| Step | What | Result |
|---|---|---|
| S1 | `gen/cpp` for all 36 protos | 144 files, 37 MB, about 903,000 lines of C++, 24 s |
| S2 | The engine on TCP | a throwaway test reusing the record-reader suite's run, behind the production auth interceptors |
| S3 | TurboLink's generator over all 36 protos | 336 files, 126,000 lines, 1 s under mono |
| S4 | A headless C++ client | replay in 8 ms, then live rounds; below |
| S5 | The Unreal leg | not run: no editor on the machine |

### S4 in numbers

`client/main.cc`, as the seated host, for 75 seconds. Condensed from the run
log; long fields cut.

```
+5ms     GetWhere ok=1 has_position=1 position=(-2, 0)
+8ms     event #1  at=0 act=[] beat="scene-opened"
+8ms     event #2  at=0 act=[] beat="scene_content"   key "the-mad-king", engine v0.241.0
+8ms     …         joined, joined_at, cells_revealed, sighted, bubble-formed
+9ms     …         turn-ended, behavior_decision, missed (guard-1, Heavy Crossbow), heard (clang at 2,0)
+7260ms  event #32 at=1 beat="turn-ended" member "reader-alice"   ← a live round
+27286ms event #40 at=2 beat="turn-ended"                          ← live
+47317ms event #46 at=3 beat="turn-ended"                          ← live
+67347ms event #57 at=4 beat="turn-ended"                          ← live
stream finished after 75006ms: CANCELLED  events=63 lagged=0 restarted=0 (last event at +67347ms)
```

The client cancelled at 75 s because the stream stays open while the run
lives. A second run, as a signed-in caller naming no seat, received the whole
record in development mode: 70 beats.

Beat names seen: `scene-opened scene_content joined joined_at cells_revealed
sighted bubble-formed turn-ended behavior_config table answered
behavior_decision missed heard tick stance transferred`.

## Pins

| What | Revision |
|---|---|
| protos | `main` at `03cfdee` |
| rpg-api | `dev` at `ed4bfc65` |
| TurboLink release | v1.4.2 (2025-11-24), `TurboLink.UE56.zip`, 912,560,441 bytes, sha256 `9ad1ecd9e64d4166fff973bcb5b3f493756987a99f2d3c2e9933b9c2908eaf39` |
| TurboLink source | `thejinchao/turbolink` at `32cc00b9c1b5289e6b659da99db106f578efe166` |
| generator source | `thejinchao/protoc-gen-turbolink` at `184d0f1820dfe3806fdb9b1c896d331a29369fce` |
| generator binary | `Tools/protoc-gen-turbolink.exe`, sha256 `2d353f22d8d9a4fa87d00924f92af2d38096722ecba32bbe420c33a8b5ae40b3` |
| generator plugins | `buf.build/protocolbuffers/cpp` v23.4, `buf.build/grpc/cpp` v1.57.0 |

TurboLink v1.4.2 vendors protobuf 4.23.4, release v23.4, and gRPC C++ 1.57.0.
Both date from July 2023. Its Linux libraries are built for UE 5.6 with
clang 16, libc++, no exceptions and no RTTI, and reference OpenSSL, zlib and
c-ares from the engine. The generator pins match those runtimes, so the
client here and the Unreal plugin consume identical generated code.

## What it showed

- **The wire works as designed.** Unary and server streaming, through the
  production auth interceptor, generated C++ against the Go server.
- **The record is the feed a 3D client needs.** `heard` is a sound at a
  position. `sighted` is who sees whom. `missed` carries the attack.
  `cells_revealed` carries regions, props and doorways. The record also
  carries the engine's own reasoning (`behavior_config`, `table`, `answered`,
  `behavior_decision`), which a game client filters out by beat name.
- **A real `oneof` survives the wrapper surveyed.** It becomes a struct with a
  case enum and Blueprint Make and Break nodes.

## What it found

### A C++-only defect in the contract

`InteractResponse.descriptor` and `TradeResponse.descriptor` collide with the
method protobuf generates for every message, so C++ fails to compile on the
vendored runtime. Go and TypeScript never see it. protobuf v36.2 sidesteps it
by naming the accessor `descriptor_()`. Decision R2; gap record
[rpg-api-protos#453](https://github.com/KirkDiggler/rpg-api-protos/issues/453).

### The wrapper surveyed drops proto3 presence

Each `optional` field, 54 in the contract, becomes a one-member wrapper whose
case defaults to its first member. This is the generated marshaling for
`SetSneakingRequest.sneaking`, verbatim:

```cpp
enum class EGrpcDnd5eApiSessionV1alpha1SetSneakingRequest_sneaking : uint8
{
    Sneaking=0,
};
…
    EGrpcDnd5eApiSessionV1alpha1SetSneakingRequest_sneaking _sneakingCase{};
…
    switch(in->_sneaking_case())
    {
    case ::dnd5e::api::session::v1alpha1::SetSneakingRequest::kSneaking:
        out->_sneaking.Sneaking=in->sneaking();
        out->_sneaking._sneakingCase = EGrpcDnd5eApiSessionV1alpha1SetSneakingRequest_sneaking::Sneaking;
        break;
    }
```

An unset field matches no case, so the case stays at its default, which is
the first member: **unset reads as "set to false"**, and a default-constructed
request always sets. `GetStatusResponse.discovery_sharing` documents the
difference the wrapper erases: explicit false is private, absent does not
mean private. The raw stubs keep presence; the generated C++ has 1,079 `has_`
accessors. Decision R3.

### The runtime surveyed is from July 2023

gRPC 1.57.0. C++ advisories published since are worth a check before it ships
in a game binary. Building protobuf and gRPC ourselves makes the version a
choice.

### The corpus is large

36 protos, about 903,000 generated lines. The test binary is 143 MB, static
and unstripped, with no dead-code elimination. The contract's other services
(character creation, sandbox, authoring) are not needed by a game client; how
much excluding them saves is unmeasured.

## What it did not show

- Anything inside Unreal: the editor, gRPC's threads beside the game thread,
  build time on the target machine, the module's shape.
- The interaction model, O1 in the design.
- A local engine. The run used an in-process store inside a test; the server
  binary requires Redis (R4).

## Recipe

Run from a scratch copy outside any repository. `game-dev/envs/local/` is
git-ignored and is where this one ran. `SPIKE` below is that copy; inside the
containers it is mounted at `/spike`.

1. **Engine side.** In a worktree of `rpg-api`, copy
   `go-side/zz_ue_spike_test.go` to `internal/integration/session/` and run
   the command in its header. It writes `engine.json` and logs
   `UE SPIKE READY`.
2. **Generate.** From a checkout of `rpg-api-protos`:
   `buf generate --template $SPIKE/buf.gen.spike.yaml --output $SPIKE/gen`.
   The unmodified contract does not compile (R2); the machine that ran this
   used a scratch copy with `descriptor` renamed to `npc_descriptor`, tag
   numbers untouched.
3. **Libraries.** Fetch TurboLink release v1.4.2 and extract
   `Source/ThirdParty/{protobuf,grpc,abseil,re2}/include`,
   `Source/ThirdParty/{protobuf,grpc,abseil,re2}/lib/linux/Release` and
   `Tools/` into `$SPIKE/vendor/`.
4. **Build and run.** `docker build -f docker/toolchain.Dockerfile -t ue-spike-toolchain docker/`,
   then `docker run --rm --network host -v $SPIKE:/spike ue-spike-toolchain bash /spike/client/build.sh`,
   then the same image running
   `/spike/build/record_client <addr> <session> <player> [member|-] [seconds]`.
5. **The wrapper surveyed, optional.** `docker build -f docker/mono.Dockerfile -t ue-spike-mono docker/`,
   then run `turbolink_gen.sh` with the protos checkout mounted at `/protos`.

### Before re-running

- `go test` spawns the test binary. Killing the `go test` pid leaves that
  binary serving the port, and the next start fails on `bind: address already
  in use` while the old engine keeps answering. Kill the holder of the port
  (`ss -ltnp`).
- `openssl3_shim.cc` exists for the container only: OpenSSL 3 renamed
  `SSL_get_peer_certificate`, which the vendored gRPC references, and Unreal
  ships 1.1.1, which has it.

## Not in git

Generated output, vendored libraries, the release archive, binaries and run
logs. The recipe rebuilds each; the
[run record](../../knowledge/runs/2026-11-24-unreal-client-design.md) keeps
the evidence.
