---
title: set arrlab
---

# **set arrlab**

`set arrlab on|off`

Toggles drawing the vector arrow label for plots drawn with <a href="gradcomdsetgxout.html">`set gxout vector`</a>. The default is *`on`* and "sticks" until reset by another `set arrlab` command.

## Usage Notes

The position of the arrow label is fixed to be in the bottom right corner of the page, just under the plot. If you want it to appear somewhere else, see the example below which demonstrates how to draw an arrow key using GrADS graphical elements.

### Examples

This example is a script sample that demonstrates how to turn off the default vector arrow scale and draw another one at any specified location:

```text

'set gxout vector'
len = 0.3
scale = 2
xrit = 8.0
ybot = 0.5
'set arrscl 'len' 'scale
'set arrlab off'
'd u;v'
rc = arrow(xrit-0.25,ybot+0.2,len,scale)

function arrow(x,y,len,scale)
'set line 1 1 4'
'draw line 'x-len/2.' 'y' 'x+len/2.' 'y
'draw line 'x+len/2.-0.05' 'y+0.025' 'x+len/2.' 'y
'draw line 'x+len/2.-0.05' 'y-0.025' 'x+len/2.' 'y
'set string 1 c'
'set strsiz 0.1'
'draw string 'x' 'y-0.1' 'scale
return



```
