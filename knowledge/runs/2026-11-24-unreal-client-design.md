# Run — 2026-11-24 — Unreal client: design and spike

Idea [rpg-project#1154](https://github.com/KirkDiggler/rpg-project/issues/1154). Base: `rpg-project` `main` at [`8220d38`](https://github.com/KirkDiggler/rpg-project/commit/8220d38).

## Shipped

- The design: one law, R1 settled and R2 to R4 open — [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155).
- The spike's record and recipe: nine text files, no binaries and no generated output — [spike/README.md](../../ideas/unreal-client/spike/README.md).
- A contract gap named in the repository that owns the contract — [rpg-api-protos#453](https://github.com/KirkDiggler/rpg-api-protos/issues/453).
- The open decisions as a checklist on the design PR — [rpg-project#1155 comment](https://github.com/KirkDiggler/rpg-project/pull/1155#issuecomment-7140599369).

## Decided

- An Unreal client is a client of the running engine over the published contract, not a Go library in its process and not a native port; ruled by the operator in conversation 2026-11-23 — [design R1](../../ideas/unreal-client/design.md#rulings).
- Every law bullet beyond R1 cites R1 and no new ruling, because each is what a client like the web already is under the workspace law, and the operator's agreement to the doc settles them together — [design, Law](../../ideas/unreal-client/design.md#law).
- Three questions stay open rulings rather than session calls, the rename's placement, TurboLink's place and where the engine runs, because each changes what gets built — [rpg-project#1155 comment](https://github.com/KirkDiggler/rpg-project/pull/1155#issuecomment-7140599369).
- Commit pins and evidence live in this record and the spike's README and not in the design doc, because the doc-genre check reads a pin as case file — [spike, Pins](../../ideas/unreal-client/spike/README.md#pins).
- The PR signs as platform and not cross-team, because the design's outcome is contract and host architecture; end-to-end verification was the spike's job, which is done — [platform charter](../../docs/teams/roles/platform/prompt.md).
- The spike compiled a scratch copy of the contract with `descriptor` renamed, because the tag numbers are untouched, the wire is identical, and the session service compiles no other way — [rpg-api-protos#453](https://github.com/KirkDiggler/rpg-api-protos/issues/453).
- The client links the libraries TurboLink ships instead of a fresh gRPC build, because those are the bytes the Unreal plugin links, so the run proves that runtime and not a look-alike — [spike, Recipe](../../ideas/unreal-client/spike/README.md#recipe).
- The engine side reuses the record-reader suite's run inside a throwaway test rather than a character-and-lobby flow, because the question was the wire and not lobby orchestration — [go-side/zz_ue_spike_test.go](https://github.com/KirkDiggler/rpg-project/blob/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/go-side/zz_ue_spike_test.go).
- TurboLink is a decision (R3) and not a dependency of the design, because the client contains none of its code — [design R3](../../ideas/unreal-client/design.md#rulings).

## Found

- Two fields named `descriptor` fail to compile in C++ generated code on protobuf 4.23.4, and protoc v36.2 names the accessor `descriptor_()` — [rpg-api-protos#453](https://github.com/KirkDiggler/rpg-api-protos/issues/453).
- TurboLink's generator maps each of 54 `optional` fields to a one-member wrapper whose case defaults to its first member, so an unset field reads as set, in both directions — [spike, What it found](../../ideas/unreal-client/spike/README.md#what-it-found).
- TurboLink v1.4.2 vendors gRPC 1.57.0 and protobuf 4.23.4, both of July 2023, for UE 5.6 — [spike, Pins](../../ideas/unreal-client/spike/README.md#pins).
- 36 protos generate 144 files and about 903,000 lines of C++; the static test binary is 143 MB — [spike, What ran](../../ideas/unreal-client/spike/README.md#what-ran).
- A C++ client reads the record through the production auth boundary: 31 beats in 8 ms, then live rounds at 7, 27, 47 and 67 seconds; 63 events, none lagged — [spike, S4 in numbers](../../ideas/unreal-client/spike/README.md#s4-in-numbers).
- The record carries the engine's own reasoning beats beside the play beats, so a game client filters by beat name — [spike, What it showed](../../ideas/unreal-client/spike/README.md#what-it-showed).
- OpenSSL 3 renames `SSL_get_peer_certificate`, which the vendored gRPC references; Unreal's 1.1.1 has it — [client/openssl3_shim.cc](https://github.com/KirkDiggler/rpg-project/blob/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/client/openssl3_shim.cc).
- The record-reader fixture opens in a fight, so the live-beat verb is `Afford` then `EndTurn` with the declaration id; `Wait` is refused — [go-side/zz_ue_spike_test.go](https://github.com/KirkDiggler/rpg-project/blob/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/go-side/zz_ue_spike_test.go).

## Mistaken

- The vendored protobuf was first read as 40.23.4 from `PROTOBUF_VERSION 4023004`; the Buf registry rejected the version, and the macro encodes 4.23.4 — corrected in [buf.gen.spike.yaml](https://github.com/KirkDiggler/rpg-project/blob/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/buf.gen.spike.yaml) before any code was generated.
- The first client run reported no live beats because it had reached an engine left over from an earlier start, whose live-beat loop had ended: ending the `go test` process leaves its test binary serving the port, a name-pattern kill missed that binary and ended the shell running it, and the next start failed on a taken port. The rerun ends the holder of the port, and the recipe says so — [spike, Before re-running](../../ideas/unreal-client/spike/README.md#before-re-running).
- The link shim took two passes, because OpenSSL 3 also names `SSL_get_peer_certificate` as a macro, so a shim defining the symbol was itself renamed; an `#undef` fixed it — [client/openssl3_shim.cc](https://github.com/KirkDiggler/rpg-project/blob/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/client/openssl3_shim.cc).
