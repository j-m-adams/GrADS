---
title: Animation
---

# Animation

There are two different ways to animate images within GrADS.

Set the <a href="dimenv.html">dimension environment</a> to have three varying dimensions and then <a href="gradcomddisplay.html">`display`</a> a variable. GrADS will return an **animation sequence**. By default, the animation dimension is time, but you may specify a different dimension to animate by using the following command:

<a href="gradcomdsetloopdim.html">`set loopdim x|y|z|t`</a>

If you wish to animate a variable with fewer than three varying dimensions (i.e., animate a line graph), you can control animation by entering:

<a href="gradcomdsetlooping.html">`set looping on|off`</a>

Remember to <a href="gradcomdsetlooping.html">`set looping off`</a> when you are done animating, or you will get a surprise when you display your next expression!

Use double buffering, which means you have two display windows, one of which is always in the background. Double buffering is invoked with the following command:

<a href="gradcomdsetdbuff.html">`set dbuff`</a>` on`

When you issue a display command after turning on double buffering, the image is drawn to the backgound buffer. Then you issue the <a href="gradcomdswap.html">`swap`</a> command, and GrADS swaps the background and foreground buffers so you can see what you've displayed. <a href="gradcomdswap.html">`swap`</a> works like <a href="gradcomdclear.html">`clear`</a> in that it resets many graphics options. Here is a sample script demonstrating how to use double buffering:

```text

'open model.ctl'
'set dbuff on'
t = 1
'set gxout shaded'
while (t <= 5)
  'set t 't
  'draw title Temperature'
  'd t'  
  'cbarn'
  'swap'
  t = t + 1
endwhile

```

You may also control the speed of the animation by inserting a <a href="gradcomdqpos.html">`q pos`</a> following the <a href="gradcomdswap.html">`swap`</a> command -- then each click of the mouse would move to the next time step.
