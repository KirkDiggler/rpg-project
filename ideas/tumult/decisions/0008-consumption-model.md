## DR-008 · 2026-06-27 · Consumption model: vendor tumult source + compile as a host module (not link a prebuilt lib)
Seam/primitive:  the host↔library edge (the consumption contract)
Decision:        a host vendors tumult + rpgkit at pinned tags and compiles
                 tumult's stateful `.cpp` (`encounter`, `vulnerable`,
                 `tough_skin`, `bleed`) as part of its own UE module; rpg::core
                 is header-only, so nothing is linked — only headers go on the
                 include path.
Rejected:        (a) ship a prebuilt static/object library (awkward across UE
                 toolchains; premature packaging for one host); (b) "link
                 rpgkit core" (it's an INTERFACE target — there is no library to
                 link).
Why:             matches how UE modules build (UBT auto-compiles a module's
                 sources) and the verified header-only reality; defers any
                 packaging story until a real second consumer needs it.
Interface delta: recipe pins tumult `v0.1.0` and rpgkit `v0.3.0` (the rpgkit tag
                 tumult `v0.1.0` builds against — not `v0.1.0`, which predates
                 the receipt API tumult consumes).
