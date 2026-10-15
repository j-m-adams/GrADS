---
title: Graphics Primitives
---

# Graphics Primitives

Various commands are provided to allow control and display of various graphics primitives: These enable you to enhance your data plot by adding customised "artwork". Alternatively, you can use these commands to create, for example, a map-based diagram with no data plot involved.

## <u>Drawing Commands</u>

- <a href="gradcomddrawmap.html">draw map</a>\
  <a href="gradcomddrawxlab.html">draw xlab</a>\
  <a href="gradcomddrawylab.html">draw ylab</a>\
  <a href="gradcomddrawstring.html">draw string</a>\
  <a href="gradcomddrawline.html">draw line</a>\
  <a href="gradcomddrawrec.html">draw rec</a>\
  <a href="gradcomddrawrecf.html">draw recf</a>\
  <a href="gradcomddrawmark.html">draw mark</a>\
  <a href="gradcomddrawpolyf.html">draw polyf</a>\
  <a href="gradcomddrawwxsym.html">draw wxsym</a>\


## <u>Controlling drawing commands</u>

- <a href="gradcomdsetfont.html">set font</a>\
  <a href="gradcomdsetline.html">set line</a>\
  <a href="gradcomdsetstring.html">set string</a>\
  <a href="gradcomdsetstrsiz.html">set strsiz</a>\
  <a href="gradcomdsetrgb.html">set rgb</a>\


## <u>Plot clipping</u>

- You may specify a clipping area for drawing graphics primitives such as lines and strings. When you do a <a href="gradcomddisplay.html">`display`</a> command, GrADS sets the clipping region to the parea, draws the graphic, then sets the clipping region to the entire page. Even if you have set the clipping region, a display command will reset it to the entire page. To clip the display of the various draw commands:

  <a href="gradcomdsetclip.html">`set clip`</a>` xlo xhi ylo yhi`

  where `xlo,xhi,ylo,yhi` are the clipping coordinates in real page inches.
