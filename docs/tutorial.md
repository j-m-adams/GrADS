---
title: Tutorial (En Español)
orphan: true
---

# Tutorial (En Español)

**Tutorial ([En Español](_downloads/Tutorial_Espanol.doc))**

**What is it?**

This document presents a brief tutorial for Brian Doty's GridAnalysis and Display System (GrADS). The following samplesessionwill give you a feeling for how to use the basic capabilities ofGrADS. This sample session takes about 30 minutes to run through. Here is a copy of

**Before you start:download the sample data**

You will need the following sample data files in order to go throughthis tutorial:

- [`model.ctl   `](ftp://grads.iges.org/grads/sprite/tutorial/model.ctl)GrADS descriptor file (0.7 kb)\
  [`model.grb   `](ftp://grads.iges.org/grads/sprite/tutorial/model.grb)GrADS (GRIB) data file (579 kb)\
  [`model.gmp   `](ftp://grads.iges.org/grads/sprite/tutorial/model.gmp)GrADS gribmap index file (4 kb)\

This data file is described by the data descriptor file[`model.ctl`](ftp://grads.iges.org/grads/sprite/tutorial/model.ctl). You may want to look at this file beforecontinuing. The data descriptor file describes the actual data file,which in the case contains 5 days of global grids that are 72 x 46elements in size.

Please download these 3 files to a localdirectory before proceeding.

**Sample Session**

To start up GrADS, enter:

<a href="gradcomdgrads.html">`grads`</a>

If the grads executable is not in your current directory, or if it is not in your PATH somewhere, you may need to enter the full pathname, ie:

`/usr/homes/smith/grads/grads`

GrADS will prompt you with a landscape vs. portrait question;just press enter. At this point a graphics output window shouldopen on your console. You may wish to move or resize thiswindow. Keep in mind that you will be entering GrADS commandsfrom the window where you first started GrADS -- this window willneed to be made the 'active' window and you will not want toentirely cover that window with the graphics output window.

In the text window (where you started grads from), you should nowsee a prompt: `ga->` You will enter GrADS commands at thisprompt and see the results displayed in the graphics outputwindow.

The first command you will enter is:

<a href="gradcomdopen.html">`open`</a>` model.ctl`

You may want to see what is in this file, so enter:

<a href="gradcomdquery.html">`query`</a>` file`

One of the available variable is called `ps`, for surface pressure. We can display this variable by entering:

<a href="gradcomddisplay.html">`d`</a>` ps`

`d` is short for <a href="gradcomddisplay.html">`display`</a>. Youwill note that by default, GrADSwill display an X, Y plot at the first time and at the lowestlevel in the data set.

!

Now you will enter commands to alter the `dimension environment`. The <a href="gradcomddisplay.html">`display`</a> command (andimplicitly, the access,operation, andoutput of the data) will do things with respect to the currentdimension environment. You control the dimension environment with the `set` command:

- <a href="gradcomdclear.html">`clear`</a>`          `clears the display\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lon`</a>` -90    `sets longitude to 90 degrees West\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lat`</a>` 40     `sets latitude to 40 degrees North\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lev`</a>` 500    `sets level to 500 mb\
  <a href="gradcomdsetlatlonlevtimeens.html">`set t`</a>` 1        `sets time to first time step\
  <a href="gradcomddisplay.html">`d`</a>` z            `displays the variable 'z'\

In the above sequence of commands, we have set all four GrADSdimensions to a single value. When we set a dimension to asingle value, we say that dimension is "fixed". Since all thedimensions are fixed, when we display a variable we get a singlevalue, in this case the value at the location 90W, 40N, 500mb,and the 1st time in the data set.

If we now enter:

- <a href="gradcomdsetlatlonlevtimeens.html">`set lon`</a>` -180 0`     X is now a varyingdimension\
  <a href="gradcomddisplay.html">`d`</a>` z`

We have set the X dimension, or longitude, to vary. We have donethis by entering two values on the set command. We now have onevarying dimension (the other dimensions are still fixed), andwhen we display a variable we get a line graph, in this case agraph of 500mb Heights at 40N.

!

Now enter:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lat`</a>` 0 90`\
  <a href="gradcomddisplay.html">`d`</a>` z`

We now have two varying dimensions, so by default we get acontour plot. If we have 3 varying dimensions:

- <a href="gradcomdclear.html">`c`</a>\
  <a href="gradcomdsetlatlonlevtimeens.html">`set t`</a>` 1 5`\
  <a href="gradcomddisplay.html">`d`</a>` z`

we get an animation sequence, in this case through time.

Now enter:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lon`</a>` -90`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lat`</a>` -90 90`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lev`</a>` 1000 100`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set t`</a>` 1`\
  <a href="gradcomddisplay.html">`d`</a>` t`\
  <a href="gradcomddisplay.html">`d`</a>` u`

In this case we have set the Y (latitude) and Z (level)dimensions to vary, so we get a vertical cross section. We havealso displayed two variables, which simply overlay each other. You may display as many items as you desire overlaid before youenter the <a href="gradcomdclear.html">`clear`</a> command.

!

Another example, in this case with X and T varying (Hovmollerplot):

- <a href="gradcomdclear.html">`c`</a>\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lon`</a>` -180 0`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lat`</a>` 40`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lev`</a>` 500`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set t 1 5`\
  </a>` `<a href="gradcomddisplay.html">`d z`</a>

!

Now that you know how to select the portion of the data set toview, we will move on to the topic of operations on the data. First, set the dimension environment to an Z, Y varying one:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lon`</a>` -180 0`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lat`</a>` 0 90`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lev`</a>` 500`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set t`</a>` 1`

Now lets say that we want to see the temperature in Fahrenheitinstead of Kelvin. We can do the conversion by entering:

<a href="gradcomddisplay.html">`display`</a>` (t-273.16)*9/5+32`

Any expression may be entered that involves the standardoperators of +, -, \*, and /, and which involves operands whichmay be constants, variables, or functions. An example involvingfunctions:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomddisplay.html">`d`</a>` sqrt(u*u+v*v)`

to calculate the magnitude of the wind. A function is providedto do this calculation directly:

<a href="gradcomddisplay.html">`d`</a>` mag(u,v)`

!

Another built in function is the averaging function:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomddisplay.html">`d`</a>` ave(z,t=1,t=5)`

In this case we calculate the 5 day mean. We can also remove themean from the current field:

<a href="gradcomddisplay.html">`d`</a>` z - ave(z,t=1,t=5)`

We can also take means over longitude to remove the zonal mean:

- <a href="gradcomdclear.html">`clear`</a>` d z-`<a href="gradfuncave.html">`ave`</a>`(z,x=1,x=72)`\
  <a href="gradcomddisplay.html">`d`</a>` z`

We can also perform time differencing:

- <a href="gradcomdclear.html">clear</a>\
  <a href="gradcomddisplay.html">d</a> z(t=2)-z(t=1)

This computes the change between the two fields over 1 day. Wecould have also done this calculation using an offset from thecurrent time:

<a href="gradcomddisplay.html">`d`</a>` z(t+1) - z`

The complete specification of a variable name is:

`name.file(dim +|-|= value, ...)`

If we had two files open, perhaps one with model output, theother with analyses, we could take the difference between the twofields by entering:

<a href="gradcomddisplay.html">`display`</a>` z.2 - z.1`

Another built in function calculates horizontal relativevorticity via finite differencing:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomddisplay.html">`d`</a>` hcurl(u,v)`

Yet another function takes a mass weighted vertical integral:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomddisplay.html">`d`</a>` `<a href="gradfuncvint.html">`vint`</a>`(ps,q,275)`

Here we have calculated precipitable water in mm.

Now we will move on to the topic of controlling the graphicsoutput. So far, we have allowed GrADS to chose a default contourinterval. We can override this by:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetcint.html">`set cint`</a>` 30`\
  <a href="gradcomddisplay.html">`d`</a>` z`

We can also control the contour color by:

- <a href="gradcomdclear.html">`clear`</a><a href="gradcomdsetccolor.html">`set ccolor`</a>` 3`\
  <a href="gradcomddisplay.html">`d`</a>` z`

We can select alternate ways of displaying the data:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetgxout.html">`set gxout`</a>` shaded`\
  <a href="gradcomddisplay.html">`d`</a>` `<a href="gradfunchcurl.html">`hcurl`</a>`(u,v)`

This is not very smooth; we can apply a cubic smoother byentering:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetcsmooth.html">`set csmooth`</a>` on`\
  <a href="gradcomddisplay.html">`d`</a>` `<a href="gradfunchcurl.html">`hcurl`</a>`(u,v)`

!

We can overlay different graphics types:

- <a href="gradcomdsetgxout.html">`set gxout`</a>` contour`\
  <a href="gradcomdsetccolor.html">`set ccolor`</a>` 0`\
  <a href="gradcomdsetcint.html">`set cint`</a>` 30`\
  <a href="gradcomddisplay.html">`d`</a>` z`

and we can annotate:

<a href="gradcomddrawtitle.html">`draw title`</a>` 500mb Heights andVorticity`

We can view wind vectors:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetgxout.html">`set gxout`</a>` vector`\
  <a href="gradcomddisplay.html">`d`</a>` u;v`

!

Here we are displaying two expressions, the first for the Ucomponent of the vector; the 2nd the V component of the vector. We can also colorize the vectors by specifying a 3rd field:

<a href="gradcomddisplay.html">`d`</a>` u;v;q`

or maybe:

<a href="gradcomddisplay.html">`d`</a>` u;v;hcurl(u,v)`

You may display pseudo vectors by displaying any field you want:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomddisplay.html">`d`</a>` `<a href="gradfuncmag.html">`mag`</a>`(u,v) ;q*10000`

Here the U component is the wind speed; the V component ismoisture.

We can also view streamlines (and colorize them):

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetgxout.html">`set gxout`</a>` stream`\
  <a href="gradcomddisplay.html">`d`</a>` u;v;hcurl(u,v)`

Or we can display actual grid point values:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetgxout.html">`set gxout`</a>` grid`\
  <a href="gradcomddisplay.html">`d`</a>` u`

!

We may wish to alter the map background:

- <a href="gradcomdclear.html">`clear`</a>\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lon`</a>` -110 -70`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lat`</a>` 30 45`\
  <a href="gradcomdsetmpdset.html">`set mpdset`</a>` nam`\
  <a href="gradcomdsetdigsize.html.html">`set digsize`</a>` 0.2`\
  <a href="gradcomdsetdignum.html">`set dignum`</a>` 2`\
  <a href="gradcomddisplay.html">`d`</a>` u`

To alter the projection:

- <a href="gradcomdsetlatlonlevtimeens.html">`set lon`</a>` -140 -40`\
  <a href="gradcomdsetlatlonlevtimeens.html">`set lat`</a>` 15 80`\
  <a href="gradcomdsetmpvals.html">`set mpvals`</a>` -120 -75 25 65`\
  <a href="gradcomdsetmproj.html">`set mproj`</a>` nps`\
  <a href="gradcomdsetgxout.html">`set gxout`</a>` contour`\
  <a href="gradcomdsetcint.html">`set cint`</a>` 30`\
  <a href="gradcomddisplay.html">`d`</a>` z`

In this case, we have told grads to access and operate on datafrom longitude 140W to 40W, and latitude 15N to 80N. But we havetold it to display a polar stereographic plot that contains theregion bounded by 120W to 75W and 25N to 65N. The extra plottingarea is clipped by the map projection routine.

!

This concludes the sample session. At this point, you may wishto examine the data set further, or you may want to go throughthe GrADS documentation and try out the other options describedthere.
