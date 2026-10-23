# Press release — Team memory

*2026-10-22 · Issue: rpg-project#676 · Law: [design.md](design.md) · Walkthrough: [README.md](README.md)*

## The team that remembers what it learned

Today the RPG workspace gains a memory. Every team role — platform, UI
programming, UI/UX, QA — now keeps what it learns, in its own words, with
the evidence attached, and wakes up next session already knowing it.

The operator of a one-human studio no longer re-explains. No more
re-deriving a story that was fully designed two months ago, no more
teaching the same constraint to the fourth session in a row, no more
discovering a rule went stale only after it cost an evening. A session
that learns something writes one line; the team owns it from then on.

## The problem this kills

Knowledge in this workspace has been complete and unfindable at the same
time. The mad king's castle carried its story — routes, turnable factions,
secrets — fully ruled and half-shipped, in a document too expensive to
reload. The gap was never the story; it was the cost of remembering.
Laws went stale silently because nothing watched for the pain that says
so. Forty flat memory notes existed with no graduation path, so curated
knowledge and raw dumping competed in the same files.

## How it works

Say it and move on: `game-dev note "given a creature with layered action
sources, augment rather than replace, because read-back must render the
merged layer set"`. One JSON file lands in a local inbox. Sessions never
touch the team's memory files directly.

When the operator asks, **ingest** reads the inbox, files each note with
its role, dedupes, and opens exactly one pull request — a diff of what
the team learned, reviewable like code. A note that contradicts what a
role believes is not silently adjudicated: it lands in that role's
`## Friction` section, which is the tripwire for the law-review loop that
reaffirms, evolves, or retires the rule in daylight.

`active.md` stops being a dumping ground and returns to its true job:
the operator's register of stories in flight and recently parked.

No databases, no vector stores, no new infrastructure. The memory is
markdown in git — diffable, reviewable, readable by every runtime this
workspace supports.
