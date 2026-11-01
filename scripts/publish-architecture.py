#!/usr/bin/env python3
"""Copy every `ideas/<topic>/overview.html` to `<site>/<topic>/index.html` and
write `<site>/index.html` listing them.

Usage: publish-architecture.py <ideas dir> <site architecture dir>

A note's title and description are read from its own <title> and
<meta name="description">. The list is ordered by the note's number when its
eyebrow names one ("Architecture field note NN"), newest first, else by title.
Run with -I; it reads only the files named on the command line.
"""
import html
import pathlib
import re
import shutil
import sys

TITLE = re.compile(r"<title>(.*?)</title>", re.S)
DESC = re.compile(r'<meta name="description" content="(.*?)"', re.S)
NOTE = re.compile(r"Architecture field note (\d+)")


def main(ideas: pathlib.Path, site: pathlib.Path) -> int:
    site.mkdir(parents=True, exist_ok=True)
    notes = []
    for src in sorted(ideas.glob("*/overview.html")):
        topic = src.parent.name
        text = src.read_text(encoding="utf-8")
        title = html.unescape(TITLE.search(text).group(1).strip()) if TITLE.search(text) else topic
        desc = html.unescape(DESC.search(text).group(1).strip()) if DESC.search(text) else ""
        number = int(NOTE.search(text).group(1)) if NOTE.search(text) else 0
        out = site / topic
        out.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, out / "index.html")
        assets = src.parent / "overview-assets"
        if assets.is_dir():
            shutil.copytree(assets, out / "overview-assets", dirs_exist_ok=True)
        notes.append((number, title, desc, topic))
        print(f"published {topic}: {title}")
    notes.sort(key=lambda n: (-n[0], n[1]))
    items = "\n".join(
        f'      <li><a href="/rpg-project/architecture/{html.escape(t)}/">'
        f'<span class="k">{("Field note %02d" % n) if n else "Note"}</span>'
        f"<strong>{html.escape(title)}</strong><small>{html.escape(desc)}</small></a></li>"
        for n, title, desc, t in notes
    )
    (site / "index.html").write_text(LIST.replace("{{ITEMS}}", items), encoding="utf-8")
    print(f"wrote list with {len(notes)} notes")
    return 0


LIST = """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="color-scheme" content="light dark">
<title>Architecture Field Notes</title>
<meta name="description" content="The engine's architecture decisions, each written up with its mechanism and its numbers.">
<link rel="canonical" href="https://kirkdiggler.github.io/rpg-project/architecture/">
<link rel="icon" type="image/svg+xml" href="/rpg-project/favicon.svg">
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Bricolage+Grotesque:opsz,wght@12..96,700&family=Public+Sans:wght@400;600&family=JetBrains+Mono:wght@400;600&display=swap">
<style>
:root { --bg: #eef1f5; --paper: #f8f9fb; --fg: #1d2530; --muted: #5b6672; --line: #c9d1da; --brass-ink: #6e4708; }
@media (prefers-color-scheme: dark) { :root { --bg: #161b22; --paper: #1e252e; --fg: #e6eaef; --muted: #9aa5b1; --line: #3a4552; --brass-ink: #f0c57a; } }
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--fg); font: 16px/1.5 "Public Sans", system-ui, sans-serif; padding: 32px max(16px, env(safe-area-inset-right, 0px)) 64px max(16px, env(safe-area-inset-left, 0px)); }
main { max-width: 760px; margin: 0 auto; display: grid; gap: 24px; }
h1 { font-family: "Bricolage Grotesque", sans-serif; font-size: clamp(2rem, 6vw, 3rem); margin: 0; letter-spacing: -0.02em; }
p { margin: 0; max-width: 64ch; color: var(--muted); }
ul { list-style: none; margin: 0; padding: 0; display: grid; gap: 12px; }
li a { display: grid; gap: 4px; padding: 16px; background: var(--paper); border: 1px solid var(--line); border-radius: 6px; color: inherit; text-decoration: none; }
li a .k { font-family: "JetBrains Mono", monospace; font-size: 0.72rem; letter-spacing: 0.08em; text-transform: uppercase; color: var(--brass-ink); }
li a strong { font-family: "Bricolage Grotesque", sans-serif; font-size: 1.25rem; }
li a small { color: var(--muted); font-size: 0.95rem; }
nav { font-size: 0.9rem; color: var(--muted); }
nav a { color: var(--brass-ink); }
</style>
</head>
<body>
<main>
  <h1>Architecture Field Notes</h1>
  <p>Each note maps one engine decision: the mechanism drawn, where it lives in the stack, and the numbers before and after. The source of every note lives beside its design in <code>ideas/</code>.</p>
  <ul>
{{ITEMS}}
  </ul>
  <nav><a href="/rpg-project/">Where Everything Meets</a> · <a href="https://kirkdiggler.github.io/rpg-toolkit/">Field note 01, on the toolkit's site</a></nav>
</main>
</body>
</html>
"""

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__, file=sys.stderr)
        sys.exit(2)
    sys.exit(main(pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])))
