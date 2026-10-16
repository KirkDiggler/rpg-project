# Shared prop atlas resources and bindings

## Shape

```mermaid
flowchart LR
  S[Registered source images] --> D[Assets shared resource derivation]
  G[Published GLBs and roster] --> B[Final-byte material binding verification]
  W[Existing wall material records] --> C[Combined versioned material candidate]
  D --> C
  B --> C
  C --> A[Studio reader and adapter acceptance]
  A --> P[Provider publication and consumer pin]
  P --> V[Per-placement appearance selection]
```

A collection suggests reusable map choices. A binding describes the actual
editable material scope on an exact model. The two do not grant each other's
claims.

## Law

- The provider derives shared images once and verifies bindings against exact
  promoted GLB bytes. Display names and source/editor selectors do not grant
  runtime compatibility.
- An atlas choice uses authored UVs and sampling. It does not imply planar wall
  fitting, physical repeat mapping, geometry normalization or a normal texture.
- Color-only choices explicitly have no normal or emissive map. Applying a choice
  preserves non-texture material properties; no selection restores the complete
  original appearance.
- Collection membership is a recommendation, not an exhaustive allow-list for
  Studio experimentation. Technical target/resource validity remains mandatory.
- Bulk preparation selects an explicit provider roster and records actual target
  identities. Ambiguous, changed or unsupported targets refuse rather than being
  silently skipped or broadened to the whole model.
- Existing model bytes, UVs, default appearances, wall resources and approved
  wall mapping remain unchanged by atlas preparation.
- The shared material catalog has one publication authority. Schema2 contains
  `materials`, `collections` and `bindings`, retaining existing wall record shapes.
  Atlas rows use `mapping: authored-uv`, shared `baseColor` image facts and explicit
  null `normal` and `emissive` channels. Collections reference material identities;
  bindings carry exact asset/model identity and actual surface node, material,
  UV, default-image and sampler facts with `suggestedCollections` references.
- The initial prop atlas batch is color-only; optional-channel presets or UI are
  not prerequisites for its delivery.
- A new catalog version remains staged until consumer reader/adapter and publication
  checks accept it. An older reader refuses the new version; it never silently drops
  atlas records or misrepresents them as masonry.
- The provider owns source/resource custody and verified target facts. Studio owns
  user controls, placement state and history; admin curation and publication
  permissions require a separate agreement.
- Licensed resources and private verification evidence remain in the private
  provider or ignored staging paths.

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| A1 | settled | Shared atlas collection and bulk-generated binding/recommendation map for existing props | KirkDiggler | 2026-10-15 |
| A2 | settled | Preserve GLBs/defaults; authored UVs instead of masonry mapping; optional channels remain absent unless explicitly selected | KirkDiggler | 2026-10-15 |
| A3 | settled | Recommendations replace mandatory per-prop artistic admission; technical checks remain | KirkDiggler | 2026-10-15 |
| A4 | settled | Provider preparation here; Studio implementation belongs to its team | KirkDiggler | 2026-10-15 |
| A5 | open | Reader/publication migration, saved appearance identity and released consumer pin | — | — |
| A6 | open | Admin-authored recommendations, permissions and publication lifecycle | — | — |
| A7 | settled | Combined schema2 material/collection/binding wire with existing wall records and authored-UV atlas rows | KirkDiggler | 2026-10-15 |
| A8 | settled | Initial prop atlas batch is color-only, with no normal/emissive prerequisite | KirkDiggler | 2026-10-15 |

## Open

- **Technical adoption (A5):** The reader/adapter and publication migration must
  implement the agreed schema2 contract before release. Operator wire agreement
  does not establish consumer implementation or release readiness.
- **Future optional atlas channels (A2/A8):** Separate normal/emissive presets need
  explicit image-layout and renderer-basis evidence outside the initial batch.
- **Saved appearance identity (A5):** Studio coordinates stable choices and
  unresolved selections through save/reload and gameplay without treating the
  candidate's presence as a delivered integration.
- **Admin authority (A6):** Shared recommendations need an owner and publication
  lifecycle before a Studio admin can change the common library.
