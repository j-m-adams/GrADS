#!/usr/bin/env python3
"""Fence block-level examples found in the original GrADS HTML pages."""
from __future__ import annotations

import html
import re
from pathlib import Path

from bs4 import BeautifulSoup

HTML_ROOT = Path("/home/rogier/programs/GrADS/doc")
MD_ROOT = Path("/home/rogier/programs/GrADS/docs")
FENCE_RE = re.compile(r"^```(?:console|python|c|fortran|text)?$")


def canonical(value: str) -> str:
    value = html.unescape(value).replace("\\", "")
    value = re.sub(r"<[^>]+>", "", value)
    value = value.replace("`", "").replace("*", "")
    return re.sub(r"\s+", "", value).lower()


def language(page: str, value: str) -> str:
    text = value.lower()
    # Shell snippets can occur on otherwise Python-oriented pages (for
    # example, the environment setup commands on python.html), so classify
    # them before applying the page-level Python hint.
    if re.search(r"(?:^|\n)\s*(?:gcc|gfortran|make|setenv|export|mamba|conda|#!/|\$ )", text):
        return "console"
    if re.search(r"\b(?:import|from)\s+(?:grads|gradspy|numpy|xarray)", text):
        return "python"
    if page in {"python", "gradspy"} and re.search(r"(?:^|\n)\s*(?:def|class|import|from|print\s*\(|[A-Za-z_]\w*\s*=)", text):
        return "python"
    if re.search(r"#include|\b(?:typedef|struct|void|int main)\b", text):
        return "c"
    if re.search(r"\b(?:subroutine|implicit none|real\s+\w|double precision|end do)\b", text):
        return "fortran"
    return "text"


def fenced_lines(lines: list[str], start: int, end: int, page: str, original: str) -> list[str]:
    selected = lines[start : end + 1]
    content = []
    for line in selected:
        if line.startswith("    "):
            content.append(line[4:])
        elif line.startswith("\t"):
            content.append(line[1:])
        else:
            content.append(line)
    # The original <pre> text is authoritative: Pandoc often collapses its
    # line breaks into a single indented Markdown line.
    content = original.replace("\r\n", "\n").replace("\r", "\n").split("\n")
    return [f"```{language(page, original)}", *content, "```"]


def find_span(lines: list[str], original: str, cursor: int) -> tuple[int, int] | None:
    source_lines = [line for line in original.replace("\r", "").split("\n") if line.strip()]
    if not source_lines:
        return None
    first = canonical(source_lines[0])[:16]
    last = canonical(source_lines[-1])[-16:]
    if len(first) < 4:
        return None
    start = None
    for i in range(cursor, len(lines)):
        if first in canonical(lines[i]):
            start = i
            break
    if start is None:
        return None
    if last and last in canonical(lines[start]):
        return start, start
    for i in range(start, len(lines)):
        if last and last in canonical(lines[i]):
            return start, i
    return None


def process(page: str) -> tuple[int, int]:
    source = HTML_ROOT / f"{page}.html"
    target = MD_ROOT / f"{page}.md"
    if not source.exists() or not target.exists():
        return 0, 0
    soup = BeautifulSoup(source.read_text(encoding="utf-8", errors="replace"), "html.parser")
    blocks = [node.get_text("\n", strip=False) for node in soup.find_all("pre")]
    lines = target.read_text(encoding="utf-8", errors="replace").splitlines()
    ranges: list[tuple[int, int, str]] = []
    cursor = 0
    unmatched_blocks: list[str] = []
    for original in blocks:
        if len(canonical(original)) < 4:
            continue
        span = find_span(lines, original, cursor)
        if span is None:
            unmatched_blocks.append(original)
            continue
        start, end = span
        if any(FENCE_RE.match(line.strip()) for line in lines[max(0, start - 2) : min(len(lines), end + 3)]):
            cursor = end + 1
            continue
        ranges.append((start, end, original))
        cursor = end + 1
    for start, end, original in reversed(ranges):
        lines[start : end + 1] = fenced_lines(lines, start, end, page, original)
    if ranges or unmatched_blocks:
        for original in unmatched_blocks:
            ranges.append((len(lines), len(lines) - 1, original))
            lines.extend(["", *fenced_lines([], 0, -1, page, original)])
        target.write_text("\n".join(lines).rstrip() + "\n", encoding="utf-8")
    return len(ranges), len(unmatched_blocks)


def main() -> None:
    converted = unmatched = 0
    missing_pages: list[str] = []
    for source in sorted(HTML_ROOT.glob("*.html")):
        count, missing = process(source.stem)
        converted += count
        unmatched += missing
        if missing:
            missing_pages.append(f"{source.stem} ({missing})")
    print(f"converted {converted} original pre blocks; unmatched {unmatched}")
    if missing_pages:
        print("unmatched pages: " + ", ".join(missing_pages))


if __name__ == "__main__":
    main()
