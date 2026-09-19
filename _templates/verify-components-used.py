#!/usr/bin/env python3
"""Every assembled component must be reachable from the mod's own code.

This is the check that the old copy-a-whole-template rule could not make. A
template broad enough for six mods is broader than any one of them needs, so
pasting it whole GUARANTEES dead code in every adopter - which is what three
successive upstream reviews of this family measured by hand.

The rule: for each component the mod assembles, at least one of the entry
points its manifest declares as `api` must be referenced from the mod's own
body, i.e. from outside the assembled region. A component nothing calls is
surface area the maintainer has to review and the next reader has to rule out,
so it is an error, not a warning.

    python _templates/verify-components-used.py <mod-dir>
"""

import json
import pathlib
import re
import sys

TEMPLATES = pathlib.Path(__file__).resolve().parent
COMPONENTS = TEMPLATES / "components"

BEGIN = "// ==ModComponents=="
END = "// ==/ModComponents=="


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: verify-components-used.py <mod-dir>", file=sys.stderr)
        return 2

    mod_dir = pathlib.Path(sys.argv[1]).resolve()
    name = mod_dir.name
    sources = sorted(mod_dir.glob("*.wh.cpp"))
    if len(sources) != 1:
        print(f"{name} COMPONENT_USE_ERROR: expected one *.wh.cpp")
        return 1
    manifest_path = mod_dir / "components.list"
    if not manifest_path.is_file():
        print(f"{name} COMPONENT_USE_SKIPPED (not an assembled mod)")
        return 0

    source = sources[0].read_text(encoding="utf-8")
    region = re.search(
        re.escape(BEGIN) + r"(.*?)" + re.escape(END), source, re.S
    )
    if not region:
        print(f"{name} COMPONENT_USE_ERROR: no assembled region found")
        return 1

    # The mod's own code is everything outside the assembled region, minus the
    # metadata blocks - a name mentioned in prose or in a setting description
    # is documentation, not a call site.
    body = source[: region.start()] + source[region.end():]
    body = re.sub(
        r"//\s*==WindhawkMod(Readme|Settings)==.*?"
        r"//\s*==/WindhawkMod(Readme|Settings)==",
        "",
        body,
        flags=re.S,
    )

    prefix = None
    ids = []
    for raw in manifest_path.read_text(encoding="utf-8").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        if line.startswith("prefix:"):
            prefix = line.split(":", 1)[1].strip()
        else:
            ids.append(line)

    unused = []
    for cid in ids:
        data = json.loads(
            (COMPONENTS / cid / "component.json").read_text(encoding="utf-8")
        )
        namespace = f"{prefix}_{data['suffix']}"
        # A mod usually reaches a component through a short namespace alias, so
        # accept the namespace itself, any alias bound to it, or a bare api
        # name qualified by such an alias.
        alias_names = set(
            re.findall(
                r"namespace\s+(\w+)\s*=\s*" + re.escape(namespace) + r"\s*;",
                body,
            )
        )
        qualifiers = {namespace} | alias_names
        referenced = False
        for entry in data["api"]:
            for qualifier in qualifiers:
                if re.search(
                    r"\b" + re.escape(qualifier) + r"::" + re.escape(entry)
                    + r"\b",
                    body,
                ):
                    referenced = True
                    break
            if referenced:
                break
        if not referenced:
            unused.append((cid, sorted(data["api"])))

    if unused:
        for cid, api in unused:
            print(
                f"{name} COMPONENT_UNUSED: '{cid}' is assembled but the mod "
                f"never calls any of its entry points ({', '.join(api)}). "
                f"Remove it from components.list, or use it."
            )
        return 1

    print(f"{name} COMPONENT_USE_OK ({len(ids)} components, all reached)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
