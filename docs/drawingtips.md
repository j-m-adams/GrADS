---
title: Drawing Tips
---

# **Drawing Tips**

This page contains some suggestions for making your graphics looks as good as possible when using GrADS version 2.1+ with Cairo enabled.

1.  **Draw axis labels, grid lines, and the map outline only once per plot**

When drawing graphical elements such as lines or letters on a plot, Cairo's anti-aliasing algorithm blends the color of the object into the existing background to smooth the color transition and make the graphics look less pixelated and more beautiful. If you redraw the same object a second time, the existing background is uneven and the anti-aliasing doesn't work as well and the objects can end up looking blotchy or ragged. Below is an example of the same string ("AaBbCc") drawn once (first set on the left), twice (second set from the left), three times (middle set), four times (the fourth set), and five times (the set on the right). Each drawing repetition alters the appearance of the string -- the letters look best when drawn only once.
<img src="_static/images/letters.png" width="376" height="31" alt="letters" />
To avoid this problem when drawing overlays in a single plot, disable the labeling after the first display. For example:
<a href="gradcomddisplay.html">d</a> hgt <a href="gradcomdsetxlab.html">set xlab</a> off <a href="gradcomdsetylab.html">set ylab</a> off <a href="gradcomdsetgrid.html">set grid</a> off <a href="gradcomdsetmpdraw.html">set mpdraw</a> off <a href="gradcomddisplay.html">d</a> temp

2.  **Line thickness settings matter**

In earlier versions of GrADS, line thickness setting between 1 and 5 always looked the same in the display and in image output -- only the vector graphics formats would show any differences. With Cairo, each line thickness setting will change the rendered line thickness, not only for the vector graphics output, but also for the X window display and any image output. The anti-aliasing also allows for smooth, thin lines in the image output. However, thin lines that are less than one pixel wide may appear somewhat dim, with the color not fully saturated, because the color will always be partially blended with the background -- no single pixel will have the full color value. To compensate for this dimness, it is not advised to draw the lines twice, because of the problems mentioned above. It is better to tweak either the thickness setting or the RGB values of the color in order to achieve the desired effect of a bright yet thin line.

The default line widths for thickness settings between 1 and 12 are: `0.6, 0.8, 1.0, 1.25, 1.5, 1.75, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0.` The units (pixels or points) will depend on the surface being drawn to -- pixels for the display window and image output, points for vector graphics. Use the new command <a href="gradcomdsetlwid.html">`set lwid`</a> to define a new line thickness setting if the 12 default widths are unsatisfactory.

```text
 d hgt
 set xlab off
 set ylab off
 set grid off
 set mpdraw off
 d temp
```
