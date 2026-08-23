# Slice 4 — the audience shelf

Journey #253 · follows #257 · shelf for rpg-toolkit#940 (perception-scoped audiences, deferred).

## 1. Kirk's ruling (2026-08-24)

"Until we get to v1.0 we intend on giving all the data down the combat log. Limiting what others see is a later concern — but we want a shelf that it can sit upon when needed." Ranged monsters are coming soon; the archer/identity questions (see #940's design-input comment) get answered when the limiting is actually turned on, not now.

## 2. What this slice builds: the shelf, not the policy

Today the audience decision is smeared across ten `appendBeat` call sites in `rulebooks/dnd5e/encounter`, each hand-rolling `rosterIDs()`/`allMemberIDs`/an ad-hoc slice. The shelf is ONE decision point:

- `audienceFor(beat beatClass, subjects ...MemberID) []MemberID` — every append site calls it, passing what the beat is ABOUT: `subjectBeat` (moved/struck/missed/downed/joined, with the actor/targets as subjects), `bubbleBeat` (fight formed/turn-ended/transferred/dissolved, with the bubble's members as subjects), `tableBeat` (scene-opened/tick/exited/ended).
- **The v1 policy inside is: everyone.** All three classes return the full roster (table beats keep `allMemberIDs` where they use it today). Behaviour change on the wire: NONE. Every existing test stays green untouched — that is the acceptance criterion.
- Each call site's classification is thereby recorded in code (reviewed once, here), so flipping #940 on later is a change to ONE function's policy — subjects ∪ current-sight-holders for subject beats, bubble membership for bubble beats — with the classification argument already correct everywhere. The intel-scan mechanics and the open rulings (actor-vs-target sight, identity redaction, join-on-sight recruiting into the order) live on #940 and get decided then.
- Door beats (doorverbs.go) already compute a sight-shaped audience; they keep it and simply route through the shelf with it, documented as the one early adopter.

## 3. Also on the shelf (small, same PR or its own)

`Joined{member}` body on the wire's Event oneof is missing — the feed shows `kind=JOINED body=null` (seen in #786's evidence). Protos addition + session projection, so the log can say who joined. Data-down-the-log, so it belongs now.

## 4. Not in this slice

Any actual filtering; the intel reverse scan; sight-based fight recruitment (noted on #940 — Kirk: "someone in the combat sees me and I join… once I can see them I go into the turn order"); unseen-summary beats.

## 5. Order

1. toolkit encounter: `audienceFor` + route all ten sites, zero wire change, tests untouched-green; tag (patch).
2. protos: `Joined{member}` body; rpg-api projects it; web formatter line. (Protos-first chain, small.)

## 6. Gate

No walk needed: the encounter suite green with no test edits proves the shelf changed nothing; #786's feed shows "joined <name>" once chain 2 lands.
