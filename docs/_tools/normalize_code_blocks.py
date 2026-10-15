#!/usr/bin/env python3
"""Merge adjacent legacy indented command examples into typed MyST fences."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path("/home/rogier/programs/GrADS/docs")
DESCRIPTOR = re.compile(r"^(?:DSET|DTYPE|INDEX|STNMAP|TITLE|UNDEF|XDEF|YDEF|ZDEF|TDEF|EDEF|VARS|ENDVARS|OPTIONS|PDEF|FILEHEADER|THEADER|TRAILERBYTES|XVAR|YVAR|ZVAR|TVAR)\b", re.I)
GRADS = re.compile(r"^(?:ga->\s*)?(?:open|sdfopen|xdfopen|set|display|d\s|q\s|query|define|undefine|clear|draw|run|exec|close|reinit|enable|disable|printim|say|pull|return|function|while|if|else|endif|endwhile|rc\s*=)\b", re.I)
SHELL = re.compile(r"^(?:#!/|\$\s|gcc\s|gfortran\s|make\b|cmake\b|export\s|setenv\s|source\s|mamba\s|conda\s|cd\s|pip\s|python(?:3)?\s)", re.I)
PYTHON = re.compile(r"^(?:from\s+\w|import\s+\w|def\s+\w|class\s+\w|if __name__|print\(|with\s+\w|for\s+\w+\s+in\s)", re.I)
C = re.compile(r"^(?:#include\b|#define\b|typedef\b|struct\b|static\s+\w|void\s+\w|int\s+\w+\s*\(|char\s+\w+\s*\[)")
FORTRAN = re.compile(r"^(?:program\b|subroutine\b|function\b|implicit\s+none|integer\b|real\b|double\s+precision|logical\b|call\b|do\s+\d|end\s*(?:program|subroutine|function)?\b|open\s*\(|write\s*\(|read\s*\()", re.I)


def score(value: str) -> int:
    value = value.strip()
    if not value:
        return 0
    if any(pattern.match(value) for pattern in (DESCRIPTOR, GRADS, SHELL, PYTHON, C, FORTRAN)):
        return 3
    if re.search(r"[=();{}]|\b(?:lon|lat|lev|time|tdef|xdef|ydef|zdef)\b", value, re.I):
        return 1
    return 0


def language(lines: list[str]) -> str:
    values = [line.strip() for line in lines if line.strip()]
    joined = "\n".join(values)
    if any(PYTHON.match(line) for line in values) or re.search(r"\b(grads|gacore|numpy|xarray)\b", joined):
        return "python"
    if any(C.match(line) for line in values) or "#include" in joined:
        return "c"
    if any(FORTRAN.match(line) for line in values):
        return "fortran"
    if any(SHELL.match(line) for line in values):
        return "console"
    return "text"


def normalize(path: Path) -> int:
    source = path.read_text(encoding="utf-8", errors="replace").splitlines()
    output: list[str] = []
    changed = 0
    i = 0
    while i < len(source):
        if not re.match(r"^(?:    |\t)\S", source[i]):
            output.append(source[i])
            i += 1
            continue
        # Inspect one legacy indented paragraph at a time, retaining prose
        # lines and only fencing code-like subsequences within it.
        paragraph: list[str] = []
        while i < len(source) and (re.match(r"^(?:    |\t)\S", source[i]) or source[i].strip() == ""):
            paragraph.append(source[i][4:] if source[i].startswith("    ") else source[i][1:] if source[i].startswith("\t") else "")
            i += 1
        j = 0
        while j < len(paragraph):
            if score(paragraph[j]) < 2:
                output.append("    " + paragraph[j] if paragraph[j] else "")
                j += 1
                continue
            start = j
            block: list[str] = []
            while j < len(paragraph):
                value = paragraph[j]
                if value.strip() and score(value) < 1:
                    break
                block.append(value)
                j += 1
            significant = [line for line in block if line.strip()]
            if len(significant) < 2 or sum(score(line) for line in significant) < 5:
                output.extend("    " + line if line else "" for line in paragraph[start:j])
                continue
            output.append(f"```{language(block)}")
            output.extend(block)
            output.append("```")
            changed += 1
    if changed:
        path.write_text("\n".join(output).rstrip() + "\n", encoding="utf-8")
    return changed


def main() -> None:
    total = sum(normalize(page) for page in sorted(ROOT.glob("*.md")) if page.name != "index.md")
    print(f"normalized {total} code-block sequences")


if __name__ == "__main__":
    main()
