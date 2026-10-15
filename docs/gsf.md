---
title: Dynamic Loading of Script Functions
---

# Dynamic loading of script functions

GrADS script variables are normally local to the script function in which they
are defined. A variable whose name begins with an underscore (`_`) is global:
its value remains available after the function returns.

Before dynamic loading was introduced, every function that used a global
variable had to be included in the main script. Dynamic loading allows GrADS to
load script functions from separate `.gsf` files when they are called.

Script function names may contain up to 16 characters, including underscores,
but must begin with a letter. Names are case-sensitive. Error messages include
the path of the file in which the error occurred.

## Enabling dynamic loading

Add this statement near the beginning of the main script:

```text
rc = gsfallow("on")
```

The `gsfallow` function enables loading of external script-function files.

## How GrADS finds the main script

Suppose the user runs:

```text
ga-> do_eof
```

GrADS searches the current directory first. If the file is not found, it tries
the same name with `.gs` appended. It then searches each directory in
`GASCRP`, trying both forms in order.

For example, with:

```text
GASCRP="/usr/local/gradslib /usr/homes/myhome"
```

the search order is:

| Order | Candidate |
| ---: | --- |
| 1 | `./do_eof` |
| 2 | `./do_eof.gs` |
| 3 | `/usr/local/gradslib/do_eof` |
| 4 | `/usr/local/gradslib/do_eof.gs` |
| 5 | `/usr/homes/myhome/do_eof` |
| 6 | `/usr/homes/myhome/do_eof.gs` |

The first matching file is used. Its directory becomes the **main-function
prefix** for subsequent `.gsf` searches.

## How GrADS finds a script function

If the main script calls a function that has not already been loaded, GrADS
searches for `<function-name>.gsf` in this order:

| Order | Location |
| ---: | --- |
| 1 | `<main-function-prefix>/<function-name>.gsf` |
| 2 | `<main-function-prefix>/<private-path>/<function-name>.gsf` |
| 3 | Each directory in `GASCRP` / `<function-name>.gsf` |

The optional private-path list is configured with `gsfpath`:

```text
rc = gsfpath("dirlist")
```

Place this statement immediately after `gsfallow("on")` in the main script.
The argument is a blank-separated list of directories.

## Complete example

Assume the main script is executed with:

```text
run /usr/local/gradslib/do_eof
```

and begins with:

```text
rc = gsfallow("on")
rc = gsfpath("math1 string2")
```

If the script calls `str_chop` and that function is not defined in the main
script, GrADS searches for:

1. `/usr/local/gradslib/str_chop.gsf`
2. `/usr/local/gradslib/math1/str_chop.gsf`
3. `/usr/local/gradslib/string2/str_chop.gsf`
4. Each `GASCRP` directory followed by `str_chop.gsf`
