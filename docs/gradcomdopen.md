---
title: open
---

# **open**

`open `*`filename`*

Opens *`filename`*, which should be the name of a GrADS data descriptor file, also called a control file.

## Usage Notes

1.  The standard file extension for Grads data descriptor files is `.ctl`. Provided you adhere to this standard, there is no need to type the extension `.ctl` when issuing the `open` command.
2.  You will need to open at least one data-descriptor file before you can enter other GrADS commands.
3.  You can open more than one data-descriptor file. Each file is numbered according to the order in which it was opened.

### Examples

` open jandata.1966`\
`open jandata.1966.ctl `

Both of these commands will have the same effect -- opening the GrADS data descriptor file `"jandata.1966.ctl"`.
