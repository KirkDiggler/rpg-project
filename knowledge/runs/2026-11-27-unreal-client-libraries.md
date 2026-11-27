# Run — 2026-11-27 — Unreal client S1 the libraries' own build

Slice [rpg-project#1249](https://github.com/KirkDiggler/rpg-project/issues/1249) of story [rpg-project#1237](https://github.com/KirkDiggler/rpg-project/issues/1237). Base `rpg-dnd5e-unreal` `main` at `30be6ff`; built in `.worktrees/libs` on Linux, Unreal 5.9.0 Preview 2 (CL 60738873). One line per entry; every line ends with a link.

## Shipped

- `scripts/build-libs.sh`: gRPC 1.81.1, protobuf 6.33.4, abseil 20260107.1, re2 2025-11-05, c-ares 1.34.8, OpenSSL 3.6.5 and zlib 1.3.2, as vcpkg `2026.10.16` (`9eec002d`) pins them, built with the engine's clang 20.1.8 and libc++, no exceptions, no RTTI, hidden visibility; `protoc 33.4` and `grpc_cpp_plugin 1.81.1` for the machine from the same pin — [rpg-dnd5e-unreal#4](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/4).
- `scripts/libs/vcpkg.pin`, `scripts/libs/ue-linux.toolchain.cmake`, `scripts/libs/triplets/x64-linux-ue.cmake`: the pin is one file; the toolchain file carries the flags and their reasons — [rpg-dnd5e-unreal#4](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/4).
- `scripts/fetch-protos.sh` generates the contract with `buf build` then those generators; `buf.gen.yaml` and its registry plugins are gone — [rpg-dnd5e-unreal#4](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/4).
- `RpgWire.Build.cs`: one include tree, the libraries' own OpenSSL and zlib, `RpgPbGuard.h` force-included ahead of every source file — [54a7514](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/54a7514).
- `scripts/test-libs.sh`, written first and failing on `main` for the reasons the issue names — [eab35fb](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/eab35fb).
- README and CLAUDE.md say where the libraries come from and what the test measures — [2dd4118](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/2dd4118).
- `--clean-after-build` on both installs and no debug tree kept: 11 GB of build trees, 4.8 GB of staging and 4.5 GB of debug libraries were left on the machine by one run otherwise — [d9be9ce](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/d9be9ce).

## Decided

- A package tree pinned to one commit in classic mode, over a manifest: the manifest's install root hid every built package from the next one's `find_package`, and a tree named by its commit needs no baseline bookkeeping; it rests on R3's "versions pinned, by a recipe kept in the client's repository" — [ruling R3](https://github.com/KirkDiggler/rpg-project/blob/main/ideas/unreal-client/design.md#rulings).
- The flags live in the toolchain file as `CMAKE_<LANG>_FLAGS_INIT`, not in the triplet: vcpkg does not apply a triplet's flags when a toolchain is chained in — [F6](https://github.com/KirkDiggler/rpg-project/issues/1249).
- The libraries carry OpenSSL 3.6.5 and zlib 1.3.2, as gRPC was compiled against them, rather than the engine's OpenSSL 1.1.1t; hidden visibility keeps them the module's own — [F10](https://github.com/KirkDiggler/rpg-project/issues/1249).
- Release only, and the unsecure builds and the plugin's support library left out: the module links Release for every configuration but Debug, and a second definition of the same symbol is a choice nobody should make by link order — [the issue's Path](https://github.com/KirkDiggler/rpg-project/issues/1249).
- The `verify` collision is answered in a guard header of the module's own, force-included first; no `-D`, no edit to a generated file — [the gap report](https://github.com/KirkDiggler/rpg-project/issues/1249#issuecomment-7229624512).
- Only `-Wunreachable-code-return` is put off, in that header; the engine's setting is for the whole group, and the rest stay errors for this module's own code — [F12](https://github.com/KirkDiggler/rpg-project/issues/1249).
- The slice issue was written only after the recipe was measured, with a probe's numbers in its Rests on — [the story's facts](https://github.com/KirkDiggler/rpg-project/issues/1237#issuecomment-7228056935).
- The build was reported as a gap and the issue amended before the guard header was written, by the lead in the same session: the decision lives in the issue, not in the builder's memory — [the amendment](https://github.com/KirkDiggler/rpg-project/issues/1249#issuecomment-7229625043).

## Found

- Eight packages build for the engine's target in 8.4 min on 32 cores, and for the host in under three; from nothing, including the generators and the copy into the module's shape, 648 s; again, 1 s — [rpg-dnd5e-unreal#4](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/4).
- `libprotobuf.a`, `libgrpc++.a`, `libgrpc.a`, `libabsl_strings.a` and `libre2.a` measure 0 RTTI typeinfo symbols and 0 `__gxx_personality` references, as the vendored ones do; 117 libraries against 92 — [the story's facts](https://github.com/KirkDiggler/rpg-project/issues/1237#issuecomment-7228056935).
- A chainloaded toolchain costs two things: a root-path rule of ONLY hides each package from the next, and the triplet is read before the toolchain it chains, so `${CMAKE_SYSROOT}` is empty there — [F6](https://github.com/KirkDiggler/rpg-project/issues/1249).
- The SDK's C headers are at `<SDK>/usr/include`, its C++ headers at `<SDK>/include/c++/v1`; the build tool uses `-nostdinc++` only, and `-nostdinc -isystem <SDK>/include` fails with `'stdio.h' file not found` — [F7](https://github.com/KirkDiggler/rpg-project/issues/1249).
- The contract has no registry dependencies: 36 files, and a plain `buf build` gives `protoc` a descriptor set of 39 entries — [F8](https://github.com/KirkDiggler/rpg-project/issues/1249).
- Unreal defines a macro `verify(expr)` for every file it compiles, and abseil's btree has a member `verify()` in five places — [F11](https://github.com/KirkDiggler/rpg-project/issues/1249).
- clang 20 flags a constexpr shape protobuf 33 writes for every message, and this engine made it an error in 5.8 — [F12](https://github.com/KirkDiggler/rpg-project/issues/1249).
- vcpkg `2026.10.16` writes no `vcpkginfo.json` in a classic install; `vcpkg list` is the record — [8748a04](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/8748a04).
- `ModuleRules.PrivateForceIncludeFiles` does not exist in 5.9; `ForceIncludeFiles` does, "for every source file in this module" — [the amendment](https://github.com/KirkDiggler/rpg-project/issues/1249#issuecomment-7229625043).
- The hand-written code needed no edit for gRPC 1.81.1: three includes, and it compiled — [54a7514](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/54a7514).

## Mistaken

- Four toolchain files before one built; each failure was mine, not the packages', and each was found by reading the build's own cache and logs, never by guessing a fix — [F6 and F7](https://github.com/KirkDiggler/rpg-project/issues/1249).
- A table of zeros was reported as evidence; it was a missing `llvm-nm` with its error sent to `/dev/null`. Redone with the system `nm` and nothing suppressed; a count of zero from a failed command is not a fact — [the story's facts](https://github.com/KirkDiggler/rpg-project/issues/1237#issuecomment-7228056935).
- The first `SOURCE` had a blank version line: a file assumed from memory, and the failure hidden inside a command substitution while `set -e` looked on. The script now stops on a missing version, and the test refuses a hole — [8748a04](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/8748a04).
- The test that refuses that hole was written with the wrong word count, 13 for a line of 20; it failed on the right record, and its own printed count showed why — [2dd4118](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/2dd4118).
- The slice issue named a build-rule member that does not exist, and the code copied it; the build refused it in five seconds. Read the engine's definition; do not recall it — [the amendment](https://github.com/KirkDiggler/rpg-project/issues/1249#issuecomment-7229625043).
- Edits anchored on text as it was remembered — a semicolon from a listing's separator, an indentation from a guess, a sentence changed under the anchor; the asserts stopped before writing, which is what they are for — [rpg-dnd5e-unreal#4](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/4).
- The recipe left 20 GB on the machine and nobody noticed until a disk reading in an unrelated check; the fix is in the script and the number is in its comment — [d9be9ce](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/d9be9ce).
- A chain of commands that began with `cd` into a directory named wrongly kept running in the game-dev root, up to a `git add && commit && push` that only an empty commit stopped. Every git command takes `git -C <path>`, as the builder's contract already said — [this record's PR](https://github.com/KirkDiggler/rpg-project/pull/1251).
- The engine's test server was killed by its wrapper's pid and kept its port; by port, it let go; the lesson's durable home is the harness's own teardown note — [zz_ue_spike_test.go at rpg-api e9606b8](https://github.com/KirkDiggler/rpg-api/blob/e9606b8aee4eae5111e345c22794ef796fcb7ac8/internal/integration/session/zz_ue_spike_test.go#L22).
