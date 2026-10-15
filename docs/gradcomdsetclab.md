---
title: set clab
---

# **set clab**

`set clab `*`option`*

Controls contour labeling. The *`option`*argument may be one of the following:

- `on       `'fast' contour labels are plotted where the contour lines are horizontal\
  `off      `no contour labels\
  `forced   `an attempt is made to label all contour lines\
  *`format`*`   `gives a C-language template for conversion of the contour value to a string\
  `masked   `(version 2.0.a6+) contour lines have gaps for the labels, so rectangles for label background are not drawn; contour labels never overlap.\

## Usage Notes

1.  Changes to the contour labels are reset by clear, but not display.
2.  When `'set clab masked'` is used, the contour lines are masked out wherever the labels are drawn. The mask creates small gaps in the contour lines, so the labels can be read clearly without the small rectangles that are usually drawn behind the contour label. The mask also ensures that labels do not overlap. If additional contour plots are overlaid, the new labels do not interfere with labels already drawn. The end result is a less cluttered graphic that is much more legible.
3.  When `'set clab masked'` is used, you can defer drawing the map until after all the labeled contours have been drawn, and then the label mask will also create gaps in the map outine for ultimate contour label legibility.
4.  The `'clear mask'` command resets the contour label mask.

### Examples

1.  This command would cause all contour labels to have 2 digits after the decimal point:\
    `   set clab %.2f`\
    \
2.  For contouring temperatures, this command would add a degree symbol and the letter "C":\
    ``    set clab %.0f`3.`1C  ``\
    \
3.  Here are two graphics that illustrate the effect of using `'set clab masked'` in a display containing shaded contours and an overlay of labeled line contours. The default behavior is shown on the left; masked contour labels are shown on the right. The script fragments used to draw the plots are also provided below the images. Observe that contour lines and coastal boundaries are not drawn underneath the masked labels, so the white rectangles are not needed as a background. In addition, the masked labels do not overlap, making all of them legible.\

<table width="700" data-border="0">
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr>
<td><img src="_static/clab_default.png" width="490" height="555" alt="default" /></td>
<td><img src="_static/clab_masked.png" width="484" height="554" alt="masked" /></td>
</tr>
<tr>
<td data-valign="top"><p><code>cl='480 490 500 510 520 530 540 550 560 570 580 590 600 610'</code><br />
<code>cc='   9  14   4  11   5  13   3  10   7  12   8   2   6'</code><br />
<code>'set rgb 16 70 70 70'</code><br />
<code>'set annot 16'</code><br />
<code>'set map 0'</code><br />
<code>* draw shaded contours</code><br />
<code>'set gxout shaded'</code><br />
<code>'set clevs 'cl</code><br />
<code>'set ccols 'cc</code><br />
<code>'set xlint 40'</code><br />
<code>'d z(lev=500)/10'</code><br />
<code>* draw labeled contours</code><br />
<code>'set gxout contour'</code><br />
<code>'set ccolor 16'</code><br />
<code>'set clevs 'cl</code><br />
<code>'set clopts 1'</code><br />
<code>'d z(lev=500)/10'</code><br />
</p></td>
<td><p><code>cl='480 490 500 510 520 530 540 550 560 570 580 590 600 610'</code><br />
<code>cc='   9  14   4  11   5  13   3  10   7  12   8   2   6'</code><br />
<code>'set rgb 16 70 70 70'</code><br />
<code>'set annot 16'</code><br />
<code>'set map 0'</code><br />
<code>* turn off map</code><br />
<code>'set mpdraw off'</code><br />
<code>* draw shaded contours</code><br />
<code>'set gxout shaded'</code><br />
<code>'set clevs 'cl</code><br />
<code>'set ccols 'cc</code><br />
<code>'set xlint 40'</code><br />
<code>'d z(lev=500)/10'</code><br />
<code>* draw labeled contours</code><br />
<code>'set gxout contour'</code><br />
<code>'set ccolor 16'</code><br />
<code>'set clevs 'cl</code><br />
<code>'set clopts 1'</code><br />
<code>'set clab masked'</code><br />
<code>'d z(lev=500)/10'</code><br />
<code>* draw the map</code><br />
<code>'set mpdraw on'</code><br />
<code>'draw map'</code></p></td>
</tr>
</tbody>
</table>
