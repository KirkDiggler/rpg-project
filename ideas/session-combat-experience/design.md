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

The concept also proved that the current wire is already enough for initiative, movement remaining, one main-hand Attack, End Turn, typed attack outcomes, Story catch-up, Debug, map geometry, public member identity, and equipment. The unresolved facts are action identity/candidates and a modern owner-private HUD projection.

This design promotes the approved experience in one deliberately narrow production wave. It does not pretend that every concept fixture action is executable today.

## Scope ruling: current verbs first

The first production wave exposes only verbs the session stack can execute end to end:

1. **Attack** — the exact authored attack definition offered by the server;
2. **Move** — the existing path verb, bounded by server-reported movement remaining; and
3. **End Turn** — the existing consequential commit.

Dodge, Dash, Disengage, features, spells, items, reactions, self-targeting, area targeting, and position-targeting do not appear as clickable offers until a real session resolution path can execute them. The toolkit's older `character.AvailableAbility` shape is evidence for a future provider; it is not permission to route the session stack through the character package's legacy direct-bus activation methods.

Owned features, active conditions, and resource counts may still appear as private character information in the dock. Informational presence is not an executable offer.

This is not a temporary panel. The composition, action-selection state machine, Story/Debug split, dice boundary, and adapters are the permanent production shape. Later verbs add declarations to it.

## Chosen architecture

### Considered approaches

1. **Wire the concept only to today's flat `Declaration`.** Fastest, but the panel would show a generic Attack button, group duplicate target rows in the client, omit unavailable candidates, and still have no durable way to identify which authored attack the player selected.
2. **Design a universal `Declare` RPC now.** It could eventually cover abilities, features, spells, and items, but no non-Attack session resolution profile currently proves its target, suspension, event, or response semantics. Locking that executor now would turn fixture imagination into a public contract.
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
message ActionIdentity {
  // Full core.Ref.String(), for example
  // "dnd5e:weapons:longsword", from the authored attack definition.
  string ref = 1;
  string name = 2;
}

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

  // Opaque, bounded, deterministic for the current declaration inputs. The
  // client echoes it and never parses it.
  string id = 8;

  // Required for Attack. Absent for Move and End Turn because those are seam
  // verbs, not authored content refs. It may be absent on an unreadable sheet.
  ActionIdentity action = 9;

  TargetKind target_kind = 10;

  // Candidates the provider evaluated for this declaration, including
  // unavailable candidates and their server-authored reasons. Empty for Move
  // and End Turn.
  repeated TargetCandidate candidates = 11;
}
```

`Verb` adds `VERB_END_TURN`. `ShortfallReason` adds `TARGET_OUT_OF_REACH` for a named candidate that failed the reach gate. The existing `NO_TARGET_IN_REACH` remains the declaration-level answer when no candidate can be attacked; its meaning and number are not repurposed.

The first wave does not add future `TargetKind` values. `SELF`, `POSITION`, and `AREA` arrive with the first executable verb that requires them.

### Availability semantics

- `Declaration.available` answers whether the declaration passes turn, standing, economy, and action-compilation gates and, for Attack, has at least one available candidate.
- `Declaration.why` is present exactly when it cannot; the server owns reason precedence. If only target gates fail, it reports `NO_TARGET_IN_REACH`.
- `TargetCandidate.available` answers the server's target-specific gate independently from the declaration-level gate. Executing against a member requires both booleans; a candidate may remain target-valid while an exhausted action slot disables the declaration.
- `TargetCandidate.why` is present exactly when the target-specific gate fails. Global budget/turn reasons are not duplicated onto every candidate.
- Attack candidates are already-observable members the rulebook/session provider considered for that authored attack, not a list the client creates by classifying sightings. Candidate enumeration may not reveal an unperceived member.
- A budget refusal may disable the declaration while still carrying target-valid candidates. The panel is disabled at the declaration level and does not reinterpret candidate reasons.
- Economy shapes light only when at least one server declaration using that `Slot` is available. They are not a display of raw unspent currencies. A bonus slot with no executable bonus offer correctly stays dark even if an internal ledger still contains one.

### Declaration identity and dispatch

A declaration ID identifies an exact current offer variant: verb, authored action where one exists, compiled execution/profile variant, and spend profile/slot. The target is not encoded as client authority; Attack still sends the selected member explicitly.

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

The SDK does not persist an offer cache. Under the same load/lock used by the verb, it regenerates current declarations, finds the echoed ID, verifies the requested target/path against the current declaration, and then executes through the same rule gates used to produce the declaration. An unknown, stale, mismatched, or now-unavailable ID is `FAILED_PRECONDITION`; the web clears selection, refreshes Turn/Afford, and explains the server-provided refusal. It never retries the action automatically.

IDs are not bearer tokens and do not replace authorization. rpg-api still proves that the authenticated player controls the acting member.

For Attack, the selected `ActionIdentity.ref` must equal the `AttackRef.ref` reported by the response and typed outcome. The provider does not offer one weapon and silently resolve another.

### Direct-map shortcut

The web may dispatch an Attack from a map click only when exactly one current declaration and its clicked candidate are both available. If zero match, it does nothing except present the server reason already rendered. If more than one match, the panel asks the player to choose; the client never picks among main-hand, off-hand, bonus, feature-granted, or otherwise rules-equivalent options.

## Owner-private character HUD

The live `v1alpha2.character.CharacterService` remains the owner-private character surface. This wave extends the `CharacterData` it already returns instead of copying sheet state into SessionService or introducing a second private read.

The package location remains unchanged in this wave. Migrating the surviving CharacterService out of the old encounter type namespace is a separate contract migration and is not required to make this payload truthful.

### Wire sketch

```proto
message CharacterData {
  // Existing identity/equipment fields 1-8 unchanged.
  CharacterHud hud = 9;
}

message CharacterHud {
  int32 level = 1;
  HitPoints hit_points = 2;
  // Base sheet speed. Turn movement remaining comes only from Afford.
  int32 base_speed_feet = 3;
  repeated FeatureView features = 4;
  repeated ConditionView conditions = 5;
  repeated ResourceView resources = 6;
}

// Reuses this package's existing HitPoints {current, max, temp} message.
// temp remains zero until the toolkit character model owns temporary HP.

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

The HUD projects owned features and currently applied conditions. It does not expose persistence blobs, ability scores, death-save internals, private rule parameters, or another member's sheet.

`resource_key` states the feature/resource relationship authoritatively. The web may join that opaque key to display `Second Wind 1/1`; it may not infer the relationship from names or class.

### Toolkit projection

`rulebooks/dnd5e/character` gains one immutable `HUDView` display projection, sibling to `EquipmentView`. It owns:

- level, current/max HP, and base speed;
- feature refs, names, provider-composed details, and resource relationships;
- active condition refs, names, provider-composed details, and observable source member IDs;
- resource keys, names, and current/max counts.

The projection must not serialize a feature/condition to JSON and inspect fields. `ConditionBehavior.Ref()` from rpg-toolkit#971 is folded into this wave so a live condition can name itself honestly. Rulebook-owned descriptors compose names/details; a loaded effect with no descriptor fails the projection loudly instead of disappearing from the HUD.

The character handler uses strict `character.Load` plus `Attach` before composing `EquipmentView` and `HUDView`. It does not use the forgiving `LoadFromData` path that can silently drop an unreadable effect (#948). A malformed persisted feature, condition, or item fails the owner read as `INTERNAL`; it is never returned as a plausible but incomplete sheet and never mutated by the read.

### Refresh behavior

The private HUD is a pull projection:

1. Fetch once at session mount after ownership is established.
2. Replace the cached value directly from successful Equip/Unequip responses, which return the same `CharacterData` shape.
3. Coalesce one query invalidation after accepted session sequence advances and after successful local mutating RPCs. A burst of catch-up or movement events produces one refresh, not one request per event.
4. Fetch fresh on reconnect before replay presentation settles.

The web never subtracts `Struck.damage`, decrements a resource, applies a condition, or predicts a post-action HUD. It waits for the owner read. A refresh failure keeps the last confirmed values visibly stale/reconnecting and remains retryable; it does not replace them with zeroes.

A dedicated private-state invalidation event is deliberately not added in this wave. The existing pull-on-event pattern used by `GetRoster` is enough for correctness, while a new event would require deciding whether invalidation is a story beat or parallel stream metadata. The first real high-frequency pressure may earn that optimization later.

## Production web composition

The concept components move behind shared production inputs rather than being copied:

- the production session route owns adapters over Turn, Afford, GetRoster, GetCharacterData, StreamEvents/GetStory, Attack, Move, and EndTurn;
- the Concepts Lab fixtures continue to instantiate the same shared components and remain the exhaustive visual state bench;
- the old `CombatPanel` is replaced in the session composition rather than kept as a second authority;
- Story renders structured typed event facts; Debug continues to render every raw event;
- presentation grouping and icons may be keyed by authoritative refs with an honest `Other`/generic fallback; ref mapping is presentation, never action legality;
- End Turn is visually separate but enabled only from its server declaration;
- world-clock mode reads `Afford.declarations = []` as the complete answer and renders exploration guidance rather than combat actions.

Loading states keep the map usable and identify which surface is waiting. A failed private HUD read does not disable server-authored movement or combat declarations. A failed Afford read disables action dispatch and offers retry; it never falls back to locally calculated buttons.

## Dice in the first production wave

The authoritative d20 already exists in `AttackResponse` and `Struck`/`Missed`. The acting player's client feeds that result into the merged `DiceTrayPresentation`, holds it concealed, and waits indefinitely for Roll or grab/release. Gesture data changes choreography only.

A stable local presentation ID is derived from the authoritative session and story sequence, not generated as a second action identity. The actor may receive the same outcome through both `AttackResponse` and its stream event; the adapter reconciles them by session/sequence into one presentation, regardless of arrival order, and a duplicate never arms a second roll. Story may reveal the actor's grouped outcome after release; the underlying game result and stream sequence are never delayed or rewritten.

Shared roller/spectator release is not claimed in this wave:

- the acting player receives the explicit ritual;
- witnesses auto-settle or use the truthful semantic fallback from their own authoritative event;
- the published Original carved preset/default may be used, but no client claims it is an owned/equipped collectible;
- reconnect settles immediately and does not replay stale choreography.

Production shared release and equipped-preset projection remain the separately gated continuation in `ideas/interactive-dice-tray/design.md`. Its transport carries no result, target, damage, or HP and does not block this combat-panel promotion.

## Story and correlation

One `Struck` or `Missed` body is already self-contained enough to render the first-wave Story exchange, and its `seq` is sufficient for the local dice presentation ID. No new correlation field is required for this wave.

When one future declaration produces multiple typed outcomes—reaction, save, effect application/removal, multiple damage instances, or individual damage dice—the provider must either:

- assign one non-empty correlation to every event in that interaction; or
- emit one self-contained typed resolution body.

The web never groups by timing, adjacent sequence numbers, matching names, or guessed cause. That future rule is part of the first multi-event action design, not silently implemented here.

## Failure and trust behavior

- **Not authenticated / foreign character:** existing owner gate returns the same `NOT_FOUND` for foreign and missing IDs; no private data crosses SessionService.
- **Unknown or stale declaration ID:** `FAILED_PRECONDITION`; clear selection and refresh, no automatic retry.
- **Unavailable candidate:** do not dispatch from the panel or map; if state changed after display, the server repeats the authoritative refusal.
- **Unreadable action/effect:** declaration or HUD projection fails explicitly; never invent a generic executable action or silently omit an effect.
- **Afford unavailable:** disable action dispatch; keep map, Story, Debug, and private HUD readable.
- **Private HUD refresh unavailable:** retain last confirmed data with stale/retry state; never calculate replacements.
- **Stream gap:** recover with GetStory, apply events in sequence, refresh Turn/Afford/HUD, and settle old dice choreography.
- **Unknown ref/icon:** render server name with generic presentation; never turn a ref into an arbitrary asset URL.
- **Reduced motion/WebGL failure:** preserve explicit Roll and semantic result through the existing dice fallback.

## Repository responsibilities

### `rpg-api-protos`

- Reshape session `Declaration`, add `ActionIdentity`, `TargetKind`, `TargetCandidate`, `VERB_END_TURN`, and `TARGET_OUT_OF_REACH`.
- Add declaration IDs to Attack/Move/EndTurn requests.
- Add `CharacterHud` and its feature/condition/resource views to the existing owner-private CharacterData.
- This is an intentional in-place pre-alpha source break. Use `breaking-change-approved`, reserve removed fields, and move every consumer in the same wave.

### `rpg-toolkit`

- In `rulebooks/dnd5e/session`, project one nested declaration per action/spend variant, evaluate candidates, mint/revalidate opaque IDs, and execute the exact authored Attack definition.
- Keep the actual verb gates and the Afford gates on one code path.
- In `rulebooks/dnd5e`, add `ConditionBehavior.Ref()` and the immutable character `HUDView` projection without raw-JSON introspection.
- One toolkit branch carries the whole wave even though auto-tagging may publish both affected modules.

### `rpg-api`

- Translate the SDK types field-for-field; no rulebook imports in handlers beyond the existing orchestrated projection boundary and no offer/condition logic in the server.
- Preserve caller/member and owner/character authorization.
- Compose CharacterData once from strict character load, EquipmentView, and HUDView; Equip, Unequip, and GetCharacterData return the same shape.
- Map stale declarations and rule refusals to `FAILED_PRECONDITION` without leaking repository details.

### `rpg-dnd5e-web`

- Promote the shared concept composition into the session route through adapters.
- Render nested declarations and candidates verbatim, echo declaration IDs, and never calculate an unavailable reason.
- Coalesce private HUD refreshes, preserve last-confirmed state on failure, and reconcile stream/catch-up before enabling intent.
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
- Every active-turn Attack declaration has a non-empty ID, exact action ref/name, slot, target kind, and server-evaluated candidates.
- Available and unavailable candidates carry the ruled presence invariants for `why`.
- Afford and each real verb share gates; property/table tests prove they cannot disagree for current state.
- Declaration IDs are deterministic for unchanged state and distinct across action/spend/profile variants; regeneration rejects an echoed ID when current state no longer admits its execution, even if the opaque text itself remains stable.
- Attack response/event action ref equals the selected declaration action ref.
- Move requires an offered ID only on the turn clock; world-clock movement remains unchanged.
- Character HUD projection covers the four level-3 party fixtures, names every loaded condition through `Ref()`, relates feature resources explicitly, and refuses unreadable effects without dropping them.
- Strict HUD reads perform no writes.

### API

- Session projection round-trips every declaration/candidate/shortfall presence case.
- Missing, foreign, and malformed character cases preserve the owner-gate and failure policy.
- Get/Equip/Unequip all return one identical post-state CharacterData composition.
- Stale IDs and target mismatches map to `FAILED_PRECONDITION`; authentication failures retain their existing indistinguishable shapes.

### Web

- Component tests retain every concept state and add production-adapter cases.
- Panel-first targeting highlights only candidates whose declaration and target-specific availability are both true; target failures render candidate `why.text` and global failures render declaration `why.text`.
- The direct map shortcut refuses ambiguity.
- No client code computes reach, action cost, post-hit HP, resource decrements, target eligibility, or outcome.
- Successful intents and accepted stream batches invalidate/refetch provider queries without event storms.
- Reconnect restores Story/Debug, current HUD, current Turn/Afford, and settled dice without replaying stale choreography.
- Keyboard, focus, reduced motion, fallback, and the 1024×768 floor remain passing.

### Live journey gate

With two owner-authenticated browser sessions in the reference tomb:

1. both render the real roster, map, initiative, and their own private HUD;
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

The first real non-Attack session action—recommended proving cases are Dodge or Dash—earns the generic execution contract. That design may add `SELF`/`POSITION`/`AREA` target kinds, resource price presentation, and a `Declare`-shaped RPC. It must first move the chosen action through inert action data and the resolution machine; it may not expose the character package's legacy direct-bus activation as the new session seam.

### Shared dice presentation

Authoritative equipped-preset projection, compact release coordination, ordering, reconnect, and spectator fallback remain under the interactive-dice journey. The release never carries the game result.

### Public effects and hurt state

Exact self HP/effects remain private. Other members receive only the public hurt/effect facts explicitly approved for the sight channel, following `ideas/characters/presentation/`.

### Multi-event Story and damage dice

Stable interaction correlation and individual authoritative damage-die faces arrive when a real action produces them. The client does not reconstruct either from totals or event adjacency.

## Non-goals

- A universal action executor before a non-Attack resolution proves it.
- Clickable fixture-only Dodge, Dash, feature, spell, or item actions.
- Raw action-economy ledgers on the wire.
- Client-side legality, targeting, cost, hit, damage, condition, or resource logic.
- Another player's exact HP, feature list, condition list, inventory, or resources.
- Migrating the surviving CharacterService package namespace in this wave.
- A production collectible ownership/catalog system or shared dice-release transport.
- Individual damage dice before authoritative individual faces exist.
- Replacing the exhaustive Debug feed or the production SessionCanvas.

— ui-ux agent, on behalf of KirkDiggler
