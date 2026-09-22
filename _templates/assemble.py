#!/usr/bin/env python3
"""Assemble a mod's shared components into its single .wh.cpp.

Windhawk mods are one file, so there is no #include to share code with. The
old answer was to paste whole templates in by hand, which guaranteed two
things: every adopter carried code it could not reach, and hand-edits drifted
from the source. This replaces both.

A mod declares the components it uses in `<mod>/components.list`. This script
writes them between the `// ==ModComponents==` markers in the mod's source.
Nothing else in the file is touched.

    python _templates/assemble.py <mod-dir>            rewrite the region
    python _templates/assemble.py <mod-dir> --check    fail if it would change

What the assembler does for you, so the shipped file reads as the mod's own
code rather than as a copied library:

  * Namespaces are generated from the mod's own prefix, so a reader sees
    `tray_utility_layout`, not a library name from somewhere else.
  * Lab-only comment lines - the ones beginning `//!` - are stripped. Review
    history, PR numbers and cross-mod examples stay in `_templates/`, where
    they are useful, and never reach a file a stranger reads cold.
  * Components are emitted in dependency order, and a component may reach
    another only through an alias it declares.
  * Optional API is pruned. A component wraps an entry point that not every
    adopter needs in `//@part Name [Alias ...]` ... `//@end`. The block ships
    only when one of its names is referenced - by the mod's own code, or by
    component code that itself ships. Several blocks may share a part name
    (a struct field and the lines that fill it); they ship or drop together.
    A component is therefore allowed to be broader than any one mod, and
    no mod carries the difference.
"""

import argparse
import json
import pathlib
import re
import sys

TEMPLATES = pathlib.Path(__file__).resolve().parent
COMPONENTS = TEMPLATES / "components"

BEGIN = "// ==ModComponents=="
END = "// ==/ModComponents=="

RULE_WIDTH = 76


def fail(message: str) -> None:
    print(f"ASSEMBLY_ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def load_component(cid: str) -> dict:
    directory = COMPONENTS / cid
    manifest = directory / "component.json"
    body = directory / "body.h"
    if not manifest.is_file() or not body.is_file():
        fail(f"no such component: {cid}")
    data = json.loads(manifest.read_text(encoding="utf-8"))
    data["body"] = body.read_text(encoding="utf-8")
    for key in ("id", "suffix", "title", "summary", "api"):
        if key not in data:
            fail(f"{cid}: component.json is missing '{key}'")
    if data["id"] != cid:
        fail(f"{cid}: component.json declares id '{data['id']}'")
    return data


def read_manifest(mod_dir: pathlib.Path):
    path = mod_dir / "components.list"
    if not path.is_file():
        fail(f"no components.list in {mod_dir}")
    prefix = None
    ids = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        if line.startswith("prefix:"):
            prefix = line.split(":", 1)[1].strip()
            continue
        ids.append(line)
    if not prefix:
        fail("components.list must start with a 'prefix:' line")
    if not re.fullmatch(r"[a-z][a-z0-9_]*", prefix):
        fail(f"prefix must be lower_snake_case: {prefix}")
    if not ids:
        fail("components.list names no components")
    if len(set(ids)) != len(ids):
        fail("components.list names the same component twice")
    return prefix, ids


def order(selected: list, loaded: dict) -> list:
    """Dependency order. A component's aliases are its dependencies."""
    done, result, visiting = set(), [], set()

    def visit(cid, trail):
        if cid in done:
            return
        if cid in visiting:
            fail("alias cycle: " + " -> ".join(trail + [cid]))
        visiting.add(cid)
        for dep in loaded[cid].get("aliases", {}).values():
            if dep not in loaded:
                fail(
                    f"{cid} reaches '{dep}', which this mod does not list. "
                    f"Add {dep} to components.list."
                )
            visit(dep, trail + [cid])
        visiting.discard(cid)
        done.add(cid)
        result.append(cid)

    for cid in selected:
        visit(cid, [])
    return result


def strip_lab_notes(body: str) -> str:
    """Drop `//!` lines: lab-only provenance that must not ship."""
    kept = [ln for ln in body.splitlines() if not ln.lstrip().startswith("//!")]
    # A note block can leave a trailing bare `//` behind; collapse runs of
    # blank comment lines that a strip created at a block boundary.
    out, previous_bare = [], False
    for line in kept:
        bare = line.strip() == "//"
        if bare and previous_bare:
            continue
        out.append(line)
        previous_bare = bare
    return "\n".join(out)


PART_BEGIN = re.compile(r"^\s*//@part\s+(.+?)\s*$")
PART_END = re.compile(r"^\s*//@end\s*$")


def split_parts(cid: str, body: str) -> list:
    """Body -> [(part_names_or_None, text)], in source order."""
    segments, current, names = [], [], None
    for line in body.splitlines():
        begin, end = PART_BEGIN.match(line), PART_END.match(line)
        if begin:
            if names is not None:
                fail(f"{cid}: nested //@part")
            segments.append((None, "\n".join(current)))
            current, names = [], tuple(begin.group(1).split())
        elif end:
            if names is None:
                fail(f"{cid}: //@end without //@part")
            segments.append((names, "\n".join(current)))
            current, names = [], None
        else:
            current.append(line)
    if names is not None:
        fail(f"{cid}: //@part {' '.join(names)} is never closed")
    segments.append((None, "\n".join(current)))
    return segments


def code_only(text: str) -> str:
    """Strip comments and string/char literals, so a name mentioned in prose
    or inside a setting key ("Layout.Arrangement") does not keep a part."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            i = n if j < 0 else j + 2
            out.append(" ")
        elif c == '"' and i > 0 and text[i - 1] == "R":
            # Raw string: R"delim( ... )delim"
            k = text.find("(", i)
            delim = text[i + 1:k]
            j = text.find(")" + delim + '"', k)
            i = n if j < 0 else j + len(delim) + 2
            out.append('""')
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            out.append(c * 2)
        else:
            out.append(c)
            i += 1
    return "".join(out)


def referenced(name: str, text: str) -> bool:
    return re.search(r"(?<![A-Za-z0-9_])" + re.escape(name) +
                     r"(?![A-Za-z0-9_])", text) is not None


def prune(ids: list, loaded: dict, mod_code: str) -> dict:
    """Decide which parts ship. Returns {cid: [segment texts to emit]}."""
    segments = {cid: split_parts(cid, loaded[cid]["body"]) for cid in ids}
    keep = set()  # part-name tuples that ship
    while True:
        # Everything that ships so far, as code, minus the part being tested.
        shipped = {}
        for cid in ids:
            for names, text in segments[cid]:
                if names is None or names in keep:
                    shipped.setdefault(names, []).append(code_only(text))
        changed = False
        for cid in ids:
            for names, _ in segments[cid]:
                if names is None or names in keep:
                    continue
                others = mod_code + "\n".join(
                    "\n".join(texts) for key, texts in shipped.items()
                    if key != names)
                if any(referenced(n, others) for n in names):
                    keep.add(names)
                    changed = True
        if not changed:
            break
    return {
        cid: [text for names, text in segments[cid]
              if names is None or names in keep]
        for cid in ids
    }


def rule(title: str) -> str:
    dashes = max(3, RULE_WIDTH - len(title) - 5)
    return f"// -- {title} " + "-" * dashes


def render(prefix: str, ids: list, loaded: dict, kept: dict) -> str:
    # The banner ships, so it is written for someone reading the published mod
    # cold, not for the lab. Nothing here points at the component library.
    chunks = [
        BEGIN,
        "// Self-contained building blocks this mod is built from. Each",
        "// section below is one contract in its own namespace; the mod's own",
        "// code begins after them.",
    ]
    for cid in ids:
        component = loaded[cid]
        namespace = f"{prefix}_{component['suffix']}"
        chunks.append("")
        chunks.append(rule(component["title"]))
        for line in wrap_comment(component["summary"]):
            chunks.append(line)
        for alias, dep in sorted(component.get("aliases", {}).items()):
            chunks.append(
                f"namespace {alias} = {prefix}_{loaded[dep]['suffix']};"
            )
        chunks.append(f"namespace {namespace} {{")
        chunks.append("")
        body = "\n".join(kept[cid])
        # A dropped part can leave two blank lines meeting; keep one.
        body = re.sub(r"\n{3,}", "\n\n", strip_lab_notes(body))
        chunks.append(body.strip("\n"))
        chunks.append("")
        chunks.append(f"}}  // namespace {namespace}")
    chunks.append("")
    chunks.append(END)
    return "\n".join(chunks)


def wrap_comment(text: str) -> list:
    words, lines, current = text.split(), [], "//"
    for word in words:
        candidate = f"{current} {word}"
        if len(candidate) > RULE_WIDTH and current != "//":
            lines.append(current)
            current = f"// {word}"
        else:
            current = candidate
    if current != "//":
        lines.append(current)
    return lines


def splice(source: str, region: str) -> str:
    pattern = re.compile(
        re.escape(BEGIN) + r".*?" + re.escape(END), re.S
    )
    if not pattern.search(source):
        fail(
            "the mod source has no // ==ModComponents== / // ==/ModComponents== "
            "region to assemble into"
        )
    return pattern.sub(lambda _: region, source, count=1)


def dominant_newline(path: pathlib.Path) -> str:
    """Rewrite the file the way it is already written, not the way this
    platform would write it - otherwise assembly shows up as a whole-file
    diff and nobody can see the change that actually matters."""
    raw = path.read_bytes()
    return "\r\n" if raw.count(b"\r\n") > raw.count(b"\n") - raw.count(b"\r\n") \
        else "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("mod")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    mod_dir = pathlib.Path(args.mod).resolve()
    sources = sorted(mod_dir.glob("*.wh.cpp"))
    if len(sources) != 1:
        fail(f"expected exactly one *.wh.cpp in {mod_dir}")
    source_path = sources[0]

    prefix, ids = read_manifest(mod_dir)
    loaded = {cid: load_component(cid) for cid in ids}
    ordered = order(ids, loaded)

    original = source_path.read_text(encoding="utf-8")
    # The mod's own code is everything outside the region being written.
    mod_code = code_only(re.sub(
        re.escape(BEGIN) + r".*?" + re.escape(END), " ", original,
        count=1, flags=re.S))
    kept = prune(ordered, loaded, mod_code)
    region = render(prefix, ordered, loaded, kept)
    updated = splice(original, region)

    name = mod_dir.name
    if args.check:
        if original != updated:
            print(
                f"{name} ASSEMBLY_DRIFT: the component region does not match "
                f"the library. Run: python _templates/assemble.py {name}"
            )
            return 1
        print(f"{name} ASSEMBLY_OK ({len(ordered)} components)")
        return 0

    if original == updated:
        print(f"{name} ASSEMBLY_UNCHANGED ({len(ordered)} components)")
        return 0
    source_path.write_text(
        updated, encoding="utf-8", newline=dominant_newline(source_path)
    )
    print(f"{name} ASSEMBLED ({len(ordered)} components)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
