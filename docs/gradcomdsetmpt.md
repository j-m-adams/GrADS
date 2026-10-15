---
title: set mpt
---

# set mpt

`set mpt `*`type off | <<col><style><thick>>`*

command to control map background behavior. *`type`* is the map type; it can be a number from `0` to `255`, or it can be an asterick(\*) to indicate this command applies to all the type values. The `color` can be set to `-1`, which indicates to GrADS to use the <a href="gradcomdsetmap.html">`set map`</a> settings for this map type, rather than the settings specified by the `set mpt` command.

## Usage Notes

### Examples

1.  Lets say you want to use the hires data set and plot political boundaries, but not state boundaries:
    - ` set mpt * off`\
      `set mpt 0 -1`\
      `set mpt 1 -1`

2.  Lets say you want to use the hires data set, and have coastlines be thicker, and a different color, than political boundaries:
    - ` set mpt * off`\
      `set mpt 0 1 1 6`\
      `set mpt 1 15 1 1`

3.  The <a href="gradcomdsetmpdraw.html">`set mpdraw`</a> and <a href="gradcomddrawmap.html">`draw map`</a> commands work as before; you can do some interesting line types by overlaying:

    - ` set mpt 51 7 1 12`\
      `draw map`\
      `set mpt 51 0 1 1`\
      `draw map`

    This would produce two yellow lines parallel and close together for map type 51.
