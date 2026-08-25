# Session Combat Experience — production contract

**Status:** Design for review

**Tracking:** [rpg-project#270](https://github.com/KirkDiggler/rpg-project/issues/270), under journey [#253](https://github.com/KirkDiggler/rpg-project/issues/253)

**Consumer proof:** [rpg-dnd5e-web#809](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/809) / [PR #810](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/810), merged 2026-08-25

## North star

A player on the session route can understand their current combat state, choose an action the server says is available, select only a server-evaluated target, see the authoritative outcome through the approved Story and dice presentation, and end the turn without opening the raw Debug feed.

The web renders facts and sends intent. It never calculates action legality, cost, reach, target eligibility, hit, damage, conditions, feature resources, or outcomes.

## What the concept proved

The merged `?concept=session-combat` surface settled the player-facing composition and interaction:

- production `SessionCanvas` remains the dominant surface;
- initiative, private vitals, effects, economy shapes, actions, Story, Debug, and End Turn have stable homes;
- targeted actions are selected panel-first, then aimed only at server-declared candidates;
- a direct map shortcut is safe only when one server offer is unambiguous;
- the authoritative d20 remains concealed until explicit Roll or grab/release;
- Story groups readable exchanges while Debug preserves every wire fact;
- fresh turn, spent turn, spectating, world-clock, and reconnect states fit at the 1024px floor.

The concept also proved that the current wire is already enough for initiative, movement remaining, one main-hand Attack, End Turn, typed attack outcomes, Story catch-up, Debug, map geometry, public member identity, and equipment. The unresolved facts are action identity/candidates and a modern owner-private character projection.

This design promotes the approved experience in one deliberately narrow production wave. It does not pretend that every concept fixture action is executable today.

## Scope ruling: current verbs first

The first production wave exposes only verbs the session stack can execute end to end:

1. **Attack** — the exact authored attack definition offered by the server;
2. **Move** — the existing path verb, bounded by server-reported movement remaining; and
3. **End Turn** — the existing consequential commit.

Dodge, Dash, Disengage, class features, mundane items, reactions, self-targeting, and position-targeting do not appear as clickable offers until a real session resolution path can execute them. The toolkit's older `character.AvailableAbility` shape is evidence for a future provider; it is not permission to route the session stack through the character package's legacy direct-bus activation methods.

**There is no magic system in the game.** Barbarian, Fighter, Monk, and Rogue were selected deliberately so the first four-player dungeon could postpone every spellcasting decision. This contract therefore reserves nothing for spells, spell slots, concentration, magical items, magical resources, or magical targeting. A future magic system requires its own journey and design rather than entering as another arm of this generalized-action work.

Owned features, active conditions, and non-magical class-resource counts may still appear as private character information in the dock. Informational presence is not an executable offer.

This is not a temporary panel. The composition, action-selection state machine, Story/Debug split, dice boundary, and adapters are the permanent production shape. Later verbs add declarations to it.

## Chosen architecture

### Considered approaches

1. **Wire the concept only to today's flat `Declaration`.** Fastest, but the panel would show a generic Attack button, group duplicate target rows in the client, omit unavailable candidates, and still have no durable way to identify which authored attack the player selected.
2. **Design a universal `Declare` RPC now.** It could eventually cover martial abilities, class features, and mundane items, but no non-Attack session resolution profile currently proves its target, suspension, event, or response semantics. Locking that executor now would turn fixture imagination into a public contract.
3. **Recommended: reshape declarations around the current executable verbs, then let the first non-Attack action earn the generic executor.** This gives the production panel exact server-authored actions and target candidates now without committing future rules to an unproved mechanism.

The design chooses approach 3.

### Data ownership

| Fact | Owner and transport |
| --- | --- |
| Clock, initiative, active participant, movement allowance, executable declarations, target candidates, attack outcomes, Story, Debug | `SessionService`, mirroring `rulebooks/dnd5e/session` |
| Public identity and body refs | `SessionService.GetRoster`, per `ideas/characters/presentation/` |
| Exact self HP, AC, base speed, equipment, owned features, active conditions, feature resources | owner-gated `v1alpha2.character.CharacterService.GetCharacterData` |
| Other members' standing and later public hurt tier | session participant/sight projections; never another player's private character read |
| Icons, tone, grouping labels, typography, drawer expansion, transient guidance | web presentation keyed by authoritative refs/facts |
| Attack result | session attack response and typed `Struck`/`Missed` event |
| Dice gesture and choreography | `DiceTrayPresentation`; local in this wave, shared transport later |

The private/public ruling from character presentation remains unchanged. Session does not grow a private-self sheet projection. The existing owner gate continues to bind the character ID in the request to the authenticated player and returns the same `NOT_FOUND` for missing and foreign characters.

## Session declaration contract

ADR-0042's durable ruling remains: **declarations, not remaining currencies**. The server answers what can be done; the client does not receive a raw ledger and infer which rules consume it.

The concept showed that the current flat one-row-per-reachable-target shape is not the final consumer shape. One declaration now represents one action/cost variant and carries the candidates evaluated for that variant.

### Wire sketch

The exact field numbers preserve the useful current tags while making the source-level break explicit:

```proto
enum TargetKind {
  TARGET_KIND_UNSPECIFIED = 0; // producer defect
  TARGET_KIND_NONE = 1;        // End Turn
  TARGET_KIND_MEMBER = 2;      // Attack
  TARGET_KIND_PATH = 3;        // Move
}

message TargetCandidate {
  string member = 1;
  // The server's target-specific gate, considered independently from the
  // declaration's turn/economy gate.
  bool available = 2;
  // Present exactly when this target-specific gate is false.
  Shortfall why = 3;
}

message Declaration {
  Verb verb = 1;
  Slot slot = 2;

  // Source-level rename from affordable. This means executable under the
  // server's full current gate, not merely that one currency can pay.
  bool available = 3;

  // Field 4 / shortfall is removed and reserved. why.text is the one prose.
  reserved 4;
  reserved "shortfall";

  // Present for Move; absent for Attack and End Turn.
  optional int32 remaining = 5;

  // Field 6 / target is replaced by candidates and reserved.
  reserved 6;
  reserved "target";

  // Present exactly when available is false. The server chooses refusal
  // precedence; the web never combines reasons.
  Shortfall why = 7;

  // Opaque, bounded, deterministic selector for the current compiled offer.
  // The client echoes it and never parses it. Empty on an early verb-level
  // blocker that deliberately did not compile an offer.
  string id = 8;

  // The one existing public Attack identity. Required for a compiled Attack;
  // absent for Move, End Turn, and early verb-level blockers.
  AttackRef attack = 9;

  TargetKind target_kind = 10;

  // Candidates the provider evaluated for this declaration, including
  // unavailable candidates and their server-authored reasons. Empty for Move
  // and End Turn.
  repeated TargetCandidate candidates = 11;
}
```

`Verb` adds `VERB_END_TURN`. `ShortfallReason` adds `TARGET_OUT_OF_REACH` for a named candidate that failed the reach gate. The existing `NO_TARGET_IN_REACH` remains the declaration-level answer when no candidate can be attacked; its meaning and number are not repurposed.

`AttackRef` is the sole public Attack identity before and after execution; this design does not add a second ref/name message. The declaration field is named `attack`, not a falsely general `action`; the first non-Attack executable may earn its own identity shape later. In the same breaking wave, `AttackRef.ref` changes from the current bare definition ID (`longsword`) to the full `core.Ref.String()` (`dnd5e:weapons:longsword`) everywhere: Declaration, AttackResponse, Struck/Missed, toolkit projection, API translation, Story/Debug, and web presentation. `name` and `damage_type` remain the provider-authored display facts. All consumers move together.

The first wave does not add future `TargetKind` values. `SELF` or `POSITION` arrives only with the first non-magical executable verb that requires it.

### Availability semantics

- `Declaration.available` answers whether the declaration passes turn, standing, economy, and action-compilation gates and, for Attack, has at least one available candidate.
- `Declaration.why` is present exactly when it cannot; the server owns reason precedence. If only target gates fail, it reports `NO_TARGET_IN_REACH`.
- `TargetCandidate.available` answers the server's target-specific gate independently from the declaration-level gate. Executing against a member requires both booleans; a candidate may remain target-valid while an exhausted action slot disables the declaration.
- `TargetCandidate.why` is present exactly when the target-specific gate fails. Global budget/turn reasons are not duplicated onto every candidate.
- The candidate universe is every current live sight holding for the actor (`CurrentVia` non-empty), excluding the actor. Stale memories and undisclosed members are excluded; a live holding with missing position data is an Afford failure, never a silently omitted candidate. Every member in that universe appears once, including members whose target preflight returns `TARGET_OUT_OF_REACH` or another refusal.
- A budget refusal may disable the declaration while still carrying target-valid candidates. The panel is disabled at the declaration level and does not reinterpret candidate reasons.
- Economy shapes light only when at least one server declaration using that `Slot` is available. They are not a display of raw unspent currencies. A bonus slot with no executable bonus offer correctly stays dark even if an internal ledger still contains one.

### One compiled-offer path

The session provider builds one internal compiled offer per verb/action/spend variant. A compiled Attack offer holds the complete inert action definition, spend profile, declaration slot, and one target-preflight result for every member in the ruled candidate universe. Afford only projects that object. Attack regenerates and selects that same object before resolution performs its final defensive validation. Move and End Turn use the same pattern for their verb-level gates; path pricing remains execution-time because no path exists during Afford.

The target preflight is one shared provider function consumed by both projection and regenerated execution. “Same code path” therefore means shared compiled data and preflight, not merely tests asserting that two independent implementations agree.

Blockers are evaluated per verb so one unreadable Attack never erases an independently executable Move or End Turn:

| Refusal | Attack | Move | End Turn |
| --- | --- | --- | --- |
| `NOT_YOUR_TURN` | blocked before sheet load | blocked before sheet load | blocked |
| `DOWNED` | blocked | blocked | available when the real EndTurn gate otherwise permits it |
| unreadable whole character | blocked with `UNREADABLE` | blocked with `UNREADABLE` | available when its clock gate permits; it does not require a sheet |
| `UNREADABLE` compiled Attack | blocked with `UNREADABLE` | continues through its own sheet/economy gate | continues through its clock gate |
| `NO_BUDGET` or target refusal | compiled but unavailable | compiled independently | unaffected |

`UNREADABLE` is broadened from “the Attack price did not compile” to “the provider dependency required by this verb could not be read.” Its text distinguishes an unreadable character from an unreadable Attack while the structured reason lets the web render the same non-executable treatment. Session/world load failures still fail Afford as a whole; only the member's character/action dependency is projected per verb.

A blocked declaration has `available=false`, the authoritative `why`, empty `id`, absent `attack`, no candidates, and the verb's fixed `target_kind` (`MEMBER`, `PATH`, or `NONE`). It does not compile unrelated detail merely to decorate a button the player cannot use. Every compiled Attack—including `NO_BUDGET` and target-only refusals—carries its exact `AttackRef` and non-empty ID. Every compiled turn-clock Move and End Turn declaration also carries a non-empty ID.

### Declaration identity and dispatch

A declaration ID identifies an exact current compiled offer variant: verb, authored action where one exists, compiled execution/profile variant, and spend profile/slot. The target is not encoded as client authority; Attack still sends the selected member explicitly.

```proto
message AttackRequest {
  string session = 1;
  string attacker = 2;
  string target = 3;
  string declaration_id = 4;
}

message MoveRequest {
  string session = 1;
  string member = 2;
  repeated Position path = 3;
  // Empty on the world clock; required on the turn clock.
  string declaration_id = 4;
}

message EndTurnRequest {
  string session = 1;
  string member = 2;
  string declaration_id = 3;
}
```

The SDK does not persist an offer cache. Under the same load/lock used by the verb, it regenerates current compiled offers, finds the echoed ID, verifies the requested target/path against the current offer, and then executes through the same gates used to produce it. An unknown, stale, mismatched, or now-unavailable ID is `FAILED_PRECONDITION`; the web clears selection, refreshes Turn/Afford, shows “That option changed; review your current actions,” and adds a refreshed `why.text` only when one exists. It never retries the action automatically.

ID construction is normative:

1. Build this selector document as a JSON value with exactly these keys:

   ```json
   {
     "domain": "session-declaration:v1",
     "session": "<session id>",
     "member": "<member id>",
     "verb": "<session verb string>",
     "slot": "<session slot string>",
     "variant": null
   }
   ```

   `null` is a schematic placeholder only and is replaced before canonicalization.

2. For Attack, `variant` is the parsed JSON value produced by serializing the validated complete `actions.Definition`, including its `SpendProfile` and populated profile union. The definition's existing JSON tags define presence: nil and empty maps/slices omitted by `omitempty` are intentionally the same selector material; a non-nil empty `SpendProfile` remains `{}` and is distinct from a nil cost. Embedded raw JSON such as condition parameters must parse successfully. For Move and End Turn, `variant` is respectively the exact string `session:move:v1` or `session:end-turn:v1`.
3. Canonicalize the entire selector document with the JSON Canonicalization Scheme, RFC 8785. That standard fixes UTF-8/string escaping, object-key ordering (including every spend-profile map), number rendering, array framing/order, field order independence, and whitespace. All current rule quantities are integers within the RFC's interoperable exact range; a future profile that cannot satisfy RFC 8785 requires a selector-version bump rather than an approximation.
4. Hash the canonical UTF-8 bytes with SHA-256 and encode the full digest as unpadded base64url after the prefix `v1.`; no truncation is allowed.
5. If two non-identical current compiled offers produce the same ID, Afford and execution fail closed as an internal provider defect rather than selecting either.
6. The ID is a selector, not an idempotency key or authorization token. The same offer may legitimately receive the same ID again when the same state recurs.

The exact Go helper and hash/canonicalization package belong in `plan.md`; the RFC 8785 document shape, definition presence semantics, domain, variant strings, full digest, and collision behavior do not.

IDs do not replace authorization. rpg-api still proves that the authenticated player controls the acting member.

For Attack, the selected declaration's `AttackRef` must equal the `AttackRef` reported by the response and typed outcome. The provider does not offer one weapon and silently resolve another.

### Direct-map shortcut

The web may dispatch an Attack from a map click only when exactly one current declaration and its clicked candidate are both available. If zero match, it does nothing except present the server reason already rendered. If more than one match, the panel asks the player to choose; the client never picks among main-hand, off-hand, bonus, feature-granted, or otherwise rules-equivalent options.

## Owner-private character data

The live `v1alpha2.character.CharacterService` remains the owner-private character surface. This wave extends the `CharacterData` it already returns instead of copying sheet state into SessionService or introducing a second private read. The fields are character facts, not a consumer-specific envelope; the combat dock, equipment screen, and character sheet may all consume the same projection.

The package location remains unchanged in this wave. Migrating the surviving CharacterService out of the old encounter type namespace is a separate contract migration and is not required to make this payload truthful.

### Wire sketch

```proto
message CharacterData {
  // Existing identity/equipment fields 1-8 unchanged.
  int32 level = 9;
  // Reuses this package's existing HitPoints {current, max, temp} message.
  // temp remains zero until the toolkit character model owns temporary HP.
  HitPoints hit_points = 10;
  // Base sheet speed. Turn movement remaining comes only from Afford.
  int32 base_speed_feet = 11;
  repeated FeatureView features = 12;
  repeated ConditionView conditions = 13;
  repeated ResourceView resources = 14;
}

message FeatureView {
  Ref ref = 1;
  string name = 2;
  // Server/toolkit-composed, optional display text; never raw feature JSON.
  string detail = 3;
  // Present when this feature consumes a projected resource.
  optional string resource_key = 4;
}

message ConditionView {
  Ref ref = 1;
  string name = 2;
  // Server/toolkit-composed, optional display text; never raw condition JSON.
  string detail = 3;
  // Present when the condition has a player-observable member source.
  optional string source_member = 4;
}

message ResourceView {
  // The rulebook's opaque core/resources.ResourceKey.
  string key = 1;
  string name = 2;
  int32 current = 3;
  int32 maximum = 4;
}
```

`CharacterData` projects owned features and currently applied conditions. It does not expose persistence blobs, ability scores, death-save internals, private rule parameters, spellcasting fields, or another member's sheet. `ResourceView` is limited to resources actually owned by the current Barbarian, Fighter, Monk, and Rogue builds; it is not a shelf for future spell slots.

`resource_key` states the feature/resource relationship authoritatively. The web may join that opaque key to display `Second Wind 1/1`; it may not infer the relationship from names or class.

Feature-owned resources require an explicit provider contract. Second Wind and Action Surge currently keep their `RecoverableResource` inside the feature object rather than in `Character.resources`; serializing their persistence JSON to discover uses is forbidden. The toolkit therefore adds a narrow, non-mutating feature-status descriptor that receives an owner through a resource-reader interface and returns ref, name, optional detail, and an optional resource `{key, name, current, maximum}`. Feature-private resources report themselves; features sharing character-owned pools such as Ki report the same stable key through the owner reader. `StatusView` deduplicates by key and fails if two providers report conflicting facts. Stable non-magical keys are defined for Second Wind and Action Surge. `Data.SpellSlots` and legacy `Data.ClassResources` are explicitly excluded.

### Toolkit projection

`rulebooks/dnd5e/character` gains one immutable `StatusView` display projection, sibling to `EquipmentView`. It owns:

- level, current/max HP, and base speed;
- feature refs, names, provider-composed details, and resource relationships;
- active condition refs, names, provider-composed details, and observable source member IDs;
- resource keys, names, and current/max counts.

The projection must not serialize a feature/condition to JSON and inspect fields. `ConditionBehavior.Ref()` from rpg-toolkit#971 is folded into this wave so a live condition can name itself honestly. Rulebook-owned descriptors compose names/details; a loaded effect with no descriptor fails the projection loudly instead of disappearing from `CharacterData`.

One shared character application path uses strict `character.Load` plus `Attach` before GetCharacterData, EquipItem, or UnequipItem can proceed. It does not use the forgiving `LoadFromData` path that can silently drop an unreadable effect (#948). Get composes `EquipmentView` and `StatusView` from that strict sheet. Equip/Unequip strictly load and validate the pre-state, mutate the in-memory sheet, compose and validate the complete post-state CharacterData **before** writing, then persist and return that already-composed result. No fallible post-write reload or projection participates in response success. A malformed persisted feature, condition, item, or post-state projection therefore fails as `INTERNAL` before a write; the API never reports an application error after persisting a forgiving or unrenderable partial sheet.

### Refresh behavior

The owner-private character data is a pull projection:

1. Fetch once at session mount after ownership is established.
2. Replace the cached value directly from successful Equip/Unequip responses, which return the same `CharacterData` shape.
3. Coalesce one query invalidation after accepted session sequence advances and after successful local mutating RPCs. A burst of catch-up or movement events produces one refresh, not one request per event.
4. While the session route is mounted, call `GetStory(lastSeq + 1)` at a bounded interval no longer than five seconds and immediately when the document regains focus/visibility. Merge recovered and streamed events through the same sequence deduper. Any recovered advance invalidates CharacterData, Turn, and Afford; `ErrStoryTrimmed` restarts from sequence zero and performs fresh reads.
5. Fetch fresh on reconnect before replay presentation settles.

The bounded catch-up poll closes the existing best-effort stream's terminal-loss hole: if the final `Struck` delivery is dropped, no later live event is required to reveal the gap. Ordinary delivery remains immediate; polling is recovery, not a second event model.

The web never subtracts `Struck.damage`, decrements a resource, applies a condition, or predicts post-action character data. It waits for the owner read. A refresh failure keeps the last confirmed values visibly stale/reconnecting and remains retryable; it does not replace them with zeroes.

A dedicated private-state invalidation event is deliberately not added in this wave. On the same best-effort stream it would share the terminal-loss problem, while periodic GetStory recovery already uses the canonical persisted source. The first real high-frequency pressure may earn a different transport optimization later.

## Production web composition

The concept components move behind shared production inputs rather than being copied:

- the production session route owns adapters over Turn, Afford, GetRoster, GetCharacterData, StreamEvents/GetStory, Attack, Move, and EndTurn;
- the Concepts Lab fixtures continue to instantiate the same shared components and remain the exhaustive visual state bench;
- the old `CombatPanel` is replaced in the session composition rather than kept as a second authority;
- Story renders structured typed event facts; Debug continues to render every raw event;
- presentation grouping and icons may be keyed by authoritative refs with an honest `Other`/generic fallback; ref mapping is presentation, never action legality;
- End Turn is visually separate but enabled only from its server declaration;
- world-clock mode reads `Afford.declarations = []` as the complete answer and renders exploration guidance rather than combat actions.

Loading states keep the map usable and identify which surface is waiting. A failed private character read does not disable server-authored movement or combat declarations. A failed Afford read disables action dispatch and offers retry; it never falls back to locally calculated buttons.

## Dice in the first production wave

The authoritative d20 already exists in `AttackResponse` and `Struck`/`Missed`. The acting player's client feeds that result into the merged `DiceTrayPresentation`, holds it concealed, and waits indefinitely for Roll or grab/release. Gesture data changes choreography only.

A stable local presentation ID is derived from the authoritative session and story sequence, not generated as a second action identity. The actor may receive the same outcome through both `AttackResponse` and its stream event; the adapter reconciles them by session/sequence into one presentation, regardless of arrival order, and a duplicate never arms a second roll.

Authoritative events are always ingested immediately, but the actor's Story exchange, semantic/live result, and visible verdict are buffered behind the matching presentation release. Response-first and event-first arrival follow the same gate and cannot reveal or announce the roll early. Reconnect/catch-up settles the presentation immediately and reveals the exchange instead of replaying stale choreography. The underlying game result and stream sequence are never delayed or rewritten.

Debug remains the exhaustive raw diagnostic exception and may reveal an ingested result immediately. Production promotion places it behind a developer/diagnostic control; it is not part of the normal player flow and a closed Debug surface contributes no hidden live-region announcement. Story is never allowed to use Debug as a fallback.

Shared roller/spectator release is not claimed in this wave:

- the acting player receives the explicit ritual;
- witnesses auto-settle or use the truthful semantic fallback from their own authoritative event;
- the published Original carved preset/default may be used, but no client claims it is an owned/equipped collectible;
- reconnect settles immediately and does not replay stale choreography.

Production shared release and equipped-preset projection remain the separately gated continuation in `ideas/interactive-dice-tray/design.md`. Its transport carries no result, target, damage, or HP and does not block this combat-panel promotion.

## Story and correlation

One `Struck` or `Missed` body is already self-contained enough to render the first-wave Story exchange, and its `seq` is sufficient for the local dice presentation ID. Its ingestion is immediate and its actor-facing rendering obeys the mandatory release gate above. No new correlation field is required for this wave.

When one future declaration produces multiple typed outcomes—reaction, save, effect application/removal, multiple damage instances, or individual damage dice—the provider must either:

- assign one non-empty correlation to every event in that interaction; or
- emit one self-contained typed resolution body.

The web never groups by timing, adjacent sequence numbers, matching names, or guessed cause. That future rule is part of the first multi-event action design, not silently implemented here.

## Failure and trust behavior

- **Not authenticated / foreign character:** existing owner gate returns the same `NOT_FOUND` for foreign and missing IDs; no private data crosses SessionService.
- **Unknown or stale declaration ID:** `FAILED_PRECONDITION`; clear selection, show the safe generic changed-option message, refresh, and append current `why.text` only when present; no automatic retry.
- **Unavailable candidate:** do not dispatch from the panel or map; if state changed after display, the server repeats the authoritative refusal.
- **Unreadable action/effect:** declaration or character-status projection fails explicitly; never invent a generic executable action or silently omit an effect.
- **Afford unavailable:** disable action dispatch; keep map, Story, Debug, and private character data readable.
- **Private character refresh unavailable:** retain last confirmed data with stale/retry state; never calculate replacements.
- **Stream gap or terminal delivery loss:** periodic/focus GetStory recovery applies events through the sequence deduper, refreshes Turn/Afford/CharacterData, and settles old dice choreography.
- **Concealed actor result:** ingest immediately but buffer Story, verdict, and semantic announcement until release; only the developer-gated raw Debug surface is exempt.
- **Unknown ref/icon:** render server name with generic presentation; never turn a ref into an arbitrary asset URL.
- **Reduced motion/WebGL failure:** preserve explicit Roll and semantic result through the existing dice fallback.

## Repository responsibilities

### `rpg-api-protos`

- Reshape session `Declaration`, reuse `AttackRef` as its sole Attack identity, add `TargetKind`, `TargetCandidate`, `VERB_END_TURN`, and `TARGET_OUT_OF_REACH`; update `UNREADABLE` docs for per-verb character/action dependency failures.
- Migrate `AttackRef.ref` from a bare ID to full `core.Ref.String()` consistently across declarations, responses, and events.
- Add declaration IDs to Attack/Move/EndTurn requests.
- Add level, hit points, base speed, and feature/condition/resource views directly to the existing owner-private CharacterData; no consumer-specific wrapper.
- This is an intentional in-place pre-alpha source break. Use `breaking-change-approved`, reserve removed fields, and move every consumer in the same wave.

### `rpg-toolkit`

- In `rulebooks/dnd5e/session`, build one internal compiled offer per action/spend variant, evaluate the complete live-sight candidate universe through shared target preflight, project it through Afford, regenerate it at execution, and execute the exact authored Attack definition.
- Implement the versioned canonical full-SHA-256 declaration selector and fail closed on duplicate IDs.
- Keep actual verb gates and Afford on the shared compiled-offer/preflight path, preserve the ruled per-verb blocker matrix, and retain resolution's final defensive validation.
- In `rulebooks/dnd5e`, add `ConditionBehavior.Ref()`, the non-mutating feature-status/resource descriptor, and immutable character `StatusView` without raw-JSON introspection.
- One toolkit branch carries the whole wave even though auto-tagging may publish both affected modules.

### `rpg-api`

- Translate the SDK types field-for-field; no rulebook imports in handlers beyond the existing orchestrated projection boundary and no offer/condition logic in the server.
- Preserve caller/member and owner/character authorization.
- Use one strict load/projection application path before Get, Equip, or Unequip; malformed data causes no write. Compose CharacterData once from EquipmentView and StatusView, and return the same post-state shape.
- Map stale declarations and rule refusals to `FAILED_PRECONDITION` without leaking repository details.

### `rpg-dnd5e-web`

- Promote the shared concept composition into the session route through adapters.
- Render nested declarations and candidates verbatim, echo declaration IDs, and never calculate an unavailable reason.
- Coalesce private character refreshes, add bounded/focus GetStory recovery for terminal stream loss, preserve last-confirmed state on failure, and reconcile stream/catch-up before enabling intent.
- Buffer actor Story/verdict/semantic result behind dice release; keep immediate raw output developer-gated in Debug.
- Reuse `SessionCanvas`, `DiceTrayPresentation`, roster identity, equipment, Story, and Debug; do not create replacement renderers or event vocabularies.
- Keep the Concepts Lab route as the durable visual regression surface.

### `rpg-project`

- Keep this design PR open through implementation, add `plan.md` after design approval, and record any provider-driven amendment before the implementation that depends on it merges.

## Development and merge order

Develop outside-in, merge inside-out:

1. Concept proof — already merged.
2. Protos — merge the approved wire first so every branch builds against one shape.
3. rpg-api and web adapters state what they consume; toolkit implements the exact provider requirements on one wave branch.
4. Live integration from the web consumer drives corrections back into the same provider branches.
5. Merge toolkit and tags, then rpg-api pins those tags, then web pins the merged API/proto contract.
6. Kirk walks the exact integrated branches and alone merges the remaining PRs.
7. Merge this rpg-project design/tracking PR after the production wave is complete.

## Verification

### Contract and toolkit

- Proto lint/generation passes and removed tags/names are reserved.
- Every compiled Attack, turn-clock Move, and End Turn declaration has a non-empty ID. Every early per-verb blocker has empty ID, absent attack, and that verb's fixed non-UNSPECIFIED target kind. Every compiled Attack also has one exact full-ref `AttackRef`, slot, and server-evaluated candidates.
- Every current live sight holding except the actor appears once; stale/undisclosed holdings do not, and missing live position fails rather than omits.
- Available and unavailable candidates carry the ruled presence invariants for `why`.
- Afford and each real verb consume the shared compiled offer and target preflight; mutation tests prove a changed preflight affects both.
- RFC 8785 selector golden tests cover object insertion order, Definition `omitempty` nil/empty normalization, embedded raw JSON canonicalization, every execution-relevant profile field, scope/domain/version, sealed Move/EndTurn variants, full digest length, recurrence, and duplicate-ID fail-closed behavior. Regeneration rejects an echoed ID when current state no longer admits its execution, even if the opaque text remains stable.
- Declaration, Attack response, and Struck/Missed carry the same full-ref `AttackRef`.
- The per-verb blocker matrix is table-tested: NOT_YOUR_TURN blocks all; DOWNED and unreadable character/action dependencies block only the verbs whose execution needs them; End Turn remains selectable whenever its real clock gate permits. Move requires an offered ID only on the turn clock; world-clock movement remains unchanged.
- Character status projection covers the four level-3 party fixtures, names every loaded condition through `Ref()`, projects feature-private Second Wind/Action Surge and shared Ki through stable non-magical keys, deduplicates matching resources, refuses conflicts, excludes SpellSlots/ClassResources, and refuses unreadable effects without dropping them.
- Strict Get/Equip/Unequip paths perform no write on malformed data; successful writes return CharacterData from the same strict post-state.

### API

- Session projection round-trips every declaration/candidate/shortfall presence case.
- Missing, foreign, and malformed character cases preserve the owner-gate and failure policy.
- Get/Equip/Unequip all use the strict path and return one identical post-state CharacterData composition; malformed feature/condition/item fixtures prove both writes remain untouched.
- Stale IDs and target mismatches map to `FAILED_PRECONDITION`; authentication failures retain their existing indistinguishable shapes.

### Web

- Component tests retain every concept state and add production-adapter cases.
- Panel-first targeting highlights only candidates whose declaration and target-specific availability are both true; target failures render candidate `why.text` and global failures render declaration `why.text`.
- The direct map shortcut refuses ambiguity.
- No client code computes reach, action cost, post-hit HP, resource decrements, target eligibility, or outcome.
- Successful intents and accepted stream/catch-up batches invalidate/refetch provider queries without event storms.
- A dropped terminal event is recovered by bounded GetStory polling without a later stream event; focus recovery and `ErrStoryTrimmed` also converge.
- Response-first and event-first attacks both ingest once and produce no actor-facing Story, verdict, or semantic roll announcement before release; closed Debug stays silent, while enabled developer Debug remains the documented raw exception.
- Reconnect restores Story/Debug, current private character data, current Turn/Afford, and settled dice without replaying stale choreography.
- Keyboard, focus, reduced motion, fallback, and the 1024×768 floor remain passing.

### Live journey gate

With two owner-authenticated browser sessions in the reference tomb:

1. both render the real roster, map, initiative, and their own private character data in the dock;
2. the active player selects the authored Attack and sees available/unavailable server candidates;
3. an unavailable target cannot dispatch and explains the provider reason;
4. the available target dispatches the echoed declaration ID and target;
5. the actor explicitly releases the authoritative d20 while the witness settles from the same typed outcome without receiving gesture authority;
6. Story shows the readable exchange and Debug shows every raw event;
7. exact HP refreshes from the owner read rather than damage subtraction;
8. spent declarations and economy shapes update, movement remains usable if offered, and End Turn advances initiative;
9. killing the stream and reconnecting restores current state through GetStory plus fresh reads;
10. returning to the world clock removes combat declarations and preserves the dungeon background.

## Later, separately designed continuations

### General executable actions

The first real non-Attack session action—recommended proving cases are Dodge or Dash—earns the generic execution contract. That design may add `SELF` or `POSITION` target kinds, class-resource price presentation, and a `Declare`-shaped RPC. It must first move the chosen action through inert action data and the resolution machine; it may not expose the character package's legacy direct-bus activation as the new session seam. It does not design for spells or reserve a magic profile.

### Shared dice presentation

Authoritative equipped-preset projection, compact release coordination, ordering, reconnect, and spectator fallback remain under the interactive-dice journey. The release never carries the game result.

### Public effects and hurt state

Exact self HP/effects remain private. Other members receive only the public hurt/effect facts explicitly approved for the sight channel, following `ideas/characters/presentation/`.

### Multi-event Story and damage dice

Stable interaction correlation and individual authoritative damage-die faces arrive when a real action produces them. The client does not reconstruct either from totals or event adjacency.

## Non-goals

- A universal action executor before a non-Attack resolution proves it.
- Clickable fixture-only Dodge, Dash, feature, or item actions.
- Magic, spells, spell slots, concentration, magical items/resources, or magical targeting; those require a separate future design.
- Raw action-economy ledgers on the wire.
- Client-side legality, targeting, cost, hit, damage, condition, or resource logic.
- Another player's exact HP, feature list, condition list, inventory, or resources.
- Migrating the surviving CharacterService package namespace in this wave.
- A production collectible ownership/catalog system or shared dice-release transport.
- Individual damage dice before authoritative individual faces exist.
- Replacing the exhaustive Debug feed or the production SessionCanvas.

— ui-ux agent, on behalf of KirkDiggler
