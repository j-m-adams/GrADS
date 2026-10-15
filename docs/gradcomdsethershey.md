---
title: set hershey
---

# **set hershey**

`set hershey on` \| `off`

This command, which is available beginning with GrADS version 2.1, is a quick shortcut for users who would like to take advantage of Cairo fonts but do not know anything about font files on their system or do not want to bother editing the font controls in existing scripts. Invoking `'set hershey off'` will tell GrADS to use generic Cairo fonts that are reasonable approximations of Hershey fonts 0, 1, 2, 4, and 5. Font 3 is the Greek (symbol) font, and cannot be duplicated with generic controls. `'set hershey on'` will return GrADS to the use of traditional Hershey fonts.

## Usage Notes

This command 'sticks' until the user issues another` `<a href="gradcomdsethershey.html">`set hershey`</a>` `command, or a` `<a href="gradcomdreset.html">`reset`</a>` ` or` `<a href="gradcomdreinit.html">`reinit`</a>` `command is issued.

Please read the documentation page on <a href="fontcontrol.html">Font Control in GrADS</a> for more information about using fonts in GrADS version 2.1.

If your build is not enabled with the Cairo graphics library, `'set hershey off'` will have no effect.

###
