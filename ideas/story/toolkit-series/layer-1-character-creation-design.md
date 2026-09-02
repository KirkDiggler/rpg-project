# Layer 1: When Configuration Becomes a Programming Language

**Status:** Approved narrative direction for a local visual prototype. Not published.

## Boundary

This chapter stays in the character-creation era. It does not use the recent session SDK, current Action model, resolution machines, or the mature event-bus architecture to explain decisions that predated them.

The chapter begins with the open-source D&D 5e API used by the Mastodon bot and ends when a created Barbarian makes Rage the next unavoidable problem.

## Thesis

Configuration is not automatically simpler than code. Configuration trades domain behavior for an interpreter. When each branch has its own recursive shape and meaning, the interpreter becomes a programming language with worse tools.

The honest uncertainty stays:

> I still do not know whether the data model was wrong for what I wanted or whether I fell short of finding the right interpreter. I know I thought I could conquer it. I also know how much easier the work became when I stopped trying.

## Eight visual panels

1. **Good enough to begin**
   - The open-source project at `dnd5eapi.co` exposes D&D reference data through an API.
   - The Mastodon adventure bot used it.
   - Hosted API rate limits were aggressive, but the project could also run locally.
   - Good enough for the Mastodon bot meant good enough for the Discord bot.

2. **The Monk turns configuration into a language**
   - Character creation reads the real archived Monk shape.
   - Tool proficiency: choose one, whose options are themselves choices, each containing another options array of references.
   - Starting weapon: a counted reference or a nested choice backed by an equipment-category lookup.
   - The data is valid. A generic UI/validator must execute recursive semantics.
   - Callout: `Configuration-driven choices are overrated. I say that as someone who loves configuration.`

3. **Boundaries connected by hope**
   - The first README is a dashed map of `core`, `systems`, and `games/dnd5e`.
   - One README, zero Go packages, zero working rulebooks.
   - Storage, events, combat, and rulebooks are nouns, not implemented seams.

4. **Where does any of this get saved?**
   - The next day adds eight storage documents over five commits; two early versions are replaced that day.
   - Modules declare access patterns; repository implementations may use Redis, Mongo, memory, or files.
   - The backing datastore should not matter.
   - Do not mention the later session SDK. At this point nobody knows where repositories ultimately belong.

5. **Stop interpreting every possible shape**
   - The breakthrough is to code what Fighter grants and what decisions Fighter requires.
   - Host asks requirements, submits choices, receives validation.
   - Manual options are not a surrender. Meaning moves into the D&D rulebook.
   - Honest paragraph about not knowing whether the problem was the source model or the attempted interpreter.

6. **A character finally exists**
   - Ask class requirements, collect explicit selections, validate, and compile grants into a live character.
   - Show Fighter and Barbarian as concrete rulebook-owned results rather than generic external choice trees.
   - Character creation works in memory. Persistence is the final unanswered part.

7. **`ToData` changed everything**
   - This is the chapter climax, not a persistence footnote.
   - The API already holds the live character because it is using it.
   - Load: repository returns `CharacterData`; API calls `LoadFromData`; the result is a fully functioning character.
   - Save: API calls `character.ToData()` and passes the result to the repository.
   - The rulebook needs no repository; the repository needs no rulebook behavior.
   - Keep this at the scale understood then; no modern session cascade.
   - Law: `A repository stores data. A rulebook restores behavior. The API moves between them.`

8. **Well, Rage is part of that data, right?**
   - `ToData` solves the known character shape.
   - A valid Barbarian carries a Rage feature that must also return fully functioning.
   - Rage must spend a use, add damage, grant Strength advantage, reduce incoming damage, track attacks and hits, end after inactivity, survive save/load, and remove itself.
   - Do not solve polymorphic feature loading or the event bus here.
   - Closing line: `We had solved how to create and save a Barbarian. Rage asked what “fully functioning” actually meant.`
   - Coming soon: `The Nervous System`.

## Visual contract

- Timeline and diagrams carry the explanation.
- Short prose blocks only.
- American English.
- Cool graphite, neutral white, and D20 blue. No amber or cream.
- Small text must have stronger contrast, not weaker contrast. Use size, weight, and spacing for hierarchy.
- Pure HTML/CSS/SVG, no scripts or analytics in the local draft.
- Mobile diagrams must either stack or make horizontal interaction unmistakable.
