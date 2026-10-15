---
title: About the GrADS Documentation
---

# About the GrADS Documentation

The maintained documentation is written in Markdown and built with
[Sphinx](https://www.sphinx-doc.org/). Markdown support is provided by
[MyST-Parser](https://myst-parser.readthedocs.io/), which allows the pages to
use Sphinx directives such as `toctree` alongside regular Markdown. The site
uses the [PyData Sphinx Theme](https://pydata-sphinx-theme.readthedocs.io/)
for its navigation, responsive layout, search, and light/dark theme support.

The source files are in the [`docs/` directory](https://github.com/RogierFloors/GrADS/tree/main/docs).
The older HTML files in `doc/` are retained as historical reference material;
new documentation edits should be made in `docs/`. 

## Installing the documentation tools

The required Python packages are listed in
[`docs/requirements.txt`](https://github.com/RogierFloors/GrADS/blob/main/docs/requirements.txt):

| Package | Version constraint | Purpose |
| --- | --- | --- |
| `sphinx` | `<8` | Documentation builder. |
| `myst-parser` | `<4` | Markdown and Sphinx-directive support. |
| `pydata-sphinx-theme` | `<0.17` | Website theme and navigation. |

From a local checkout, install them in a virtual environment so the
documentation tools do not alter your system Python:

```console
$ python -m venv .venv
$ . .venv/bin/activate
$ python -m pip install --upgrade pip
$ python -m pip install -r docs/requirements.txt
```

An existing conda environment can be used instead; activate it first and run
the final `pip install` command there.

## Building the documentation

After installing the requirements, build the HTML site from the `docs/`
directory:

```console
$ cd docs
$ make html
```

The generated site is written to `docs/_build/html/`. Open
`docs/_build/html/index.html` in a browser to review it locally. The Makefile
also provides:

```console
$ make clean       # remove generated files
$ make linkcheck   # check documentation links
```

Run `make html` after editing and fix any warnings before submitting a change.

## Editing the documentation

The canonical source repository is
[github.com/RogierFloors/GrADS](https://github.com/RogierFloors/GrADS).
Documentation pages are Markdown files in `docs/`; the navigation is defined
with Sphinx `toctree` blocks in the guide index pages. A typical local editing
workflow is:

```console
$ git clone https://github.com/RogierFloors/GrADS.git
$ cd GrADS
$ git switch -c improve-documentation
$ $EDITOR docs/about.md
$ make -C docs html
$ git add docs/
$ git commit -m "Improve documentation"
$ git push -u origin improve-documentation
```

Then open a pull request on GitHub. For small changes, GitHub's web editor can
also be used: open the Markdown file in the repository, select **Edit this
file**, make the change, and commit it to a new branch or pull-request branch.
The local build is still recommended for checking Markdown structure, links,
and Sphinx warnings before merging.
