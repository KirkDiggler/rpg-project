# Named behavior tables at the site root — a pointer, not a design

**Status:** NOT DESIGNED. Recorded 2026-09-23 so the idea has an owner and a
trigger, and so the toolkit idea does not carry it (rpg-project#497/#498).

## What this is

A place in the World Builder to **declare and name behavior tables at the site
root**, the way `factions`, `dispositions` and `intel` are declared today — a
named block carrying a creature's `time` table (and whatever an entry grows to
carry) that a creature or faction **references** rather than pastes.

Kirk, 2026-09-23:

> "I think having the place in the builder where we can name the tables at the
> root like we do disposition and others. this will obviously come in waves and
> slices."

## Why it lives here and not in the toolkit idea

The toolkit idea originally proposed this as a **parameterized regime primitive**
in the engine. Kirk deleted it from there:

> "we will be building them in the site. I think we name them there to start and
> when we want to carry over one dungeon's creature facts to another, then that
> idea flows out of there."

So: **the name is the site's**, the engine never learns one, and the idea is
earned by a *site* need — the first being **carrying creature facts from one
dungeon to another**.

## The trigger, and why it is deliberately unbuilt

**Not now.** Per "a use case brings the mechanism", this waits for the need it
was named for: an author with two dungeons and a table worth reusing. Until then
the site's root keys stay what they are, and an author overrides a creature's
`time` inline.

## What it will sit beside

The site root today is `version`, `key`, `play`, `room`, `factions`,
`dispositions`, `intel`, `exits`, `endings`, `scenarios`, `concealments`. A named
table would join them, with a panel beside `IntelPanel` — declared once, held by
many, referenced by id, exactly the pattern `intel[]` + `holds` already proves.

## Not decided (and not to be decided here)

- The key's name. (`time` was rejected as a root key — it conflates *when a
  creature acts* with *a named block*.)
- Whether a referenced table may carry inputs beyond its entries (the patrol
  route / guard pause question), or whether that is the engine's to answer
  first. **The engine question is separate and is what rpg-project#498 asks.**

— rpg-toolkit agent, on behalf of KirkDiggler