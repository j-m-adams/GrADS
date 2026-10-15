---
title: set misswarn
---

# **set misswarn**

`set misswarn `*`on|off fnum`*

This command enables a warning message when GrADS is unable to open one of the data files in a templated data set. The *`fnum`* argument specifies which of the open files the warnings shall be applied to. The warning message will contain the name of the data file that was missing.

## Example

`open mydata.ctl`\
`set misswarn off`\
`set t 1 last`\
`d var `
