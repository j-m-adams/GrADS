/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/XWDFile.h>
#include "gatypes.h"
#define FEEP_VOLUME 0

/* Window_Dump: dump a window to a file which must already be open for writing. */


#ifdef __alpha
#define NON_STANDARD_XCOLOR_SIZE 4
#endif

#undef NON_STANDARD_XCOLOR_SIZE

#ifdef NON_STANDARD_XCOLOR_SIZE
#define My_XColor_size 12
#else
#define My_XColor_size sizeof(XColor)
#endif

void _swaplong  (register char *,register unsigned);
void _swapshort (register char *,register unsigned);
gaint Get_XColors (XWindowAttributes *, XColor **);
gaint Image_Size (XImage *);
void outl(char *,char *,char *,char *,char *,char *,char *,char *);
void Window_Dump(Window,FILE *);
void set_display_screen (Display *, gaint);

static gaint format = ZPixmap;
static Bool nobdrs = True;
static Bool on_root = False;
static Bool debug = False;
static Bool use_installed = False;
static long add_pixel_value = 0;

static Display *dpy;                          /* The current display */
static gaint screen;                            /* The current screen */

void set_display_screen(d,s)
Display *d;                                 
gaint s;   
{
 dpy =d;
 screen=s;
}    

void Pixmap_Dump(window, out,x, y, width, height)
     Window window;
     FILE *out;
     gaint x, y, width, height;
{
    XImage *image;
    unsigned buffer_size;

    image = XGetImage (dpy, window, x, y, width, height, AllPlanes, format);  
    buffer_size = Image_Size(image);
    (void) fwrite(image->data, (gaint) buffer_size, 1, out);  
    if (debug)    fprintf(stderr,"scritti %d bytes immagine\n",buffer_size);
}                            

void Window_Dump(window, out)
     Window window;
     FILE *out;
{
    unsigned long swaptest = 1;
    XColor *colors;
    unsigned buffer_size;
    gaint win_name_size;
    gaint header_size;
    gaint ncolors, i;
    char *win_name;
    Bool got_win_name;
    XWindowAttributes win_info;
    XImage *image;
    gaint absx, absy, x, y;
    unsigned width, height;
    gaint dwidth, dheight;
    gaint bw;
    Window dummywin;
    XWDFileHeader header;

    
    /*
     * Get the parameters of the window being dumped.
     */
    if (debug) fprintf(stderr,"xwd: Getting target window information.\n");
    if(!XGetWindowAttributes(dpy, window, &win_info)) 
      { fprintf(stderr,"Can't get target window attributes."); exit(1); }

    /* handle any frame window */
    if (!XTranslateCoordinates (dpy, window, RootWindow (dpy, screen), 0, 0,
				&absx, &absy, &dummywin)) {
	fprintf (stderr, 
		  "unable to translate window coordinates (%d,%d)\n",
		 absx, absy);
	exit (1);
    }
    win_info.x = absx;
    win_info.y = absy;
    width = win_info.width;
    height = win_info.height;
    bw = 0;

    if (!nobdrs) {
	absx -= win_info.border_width;
	absy -= win_info.border_width;
	bw = win_info.border_width;
	width += (2 * bw);
	height += (2 * bw);
    }
    dwidth = DisplayWidth (dpy, screen);
    dheight = DisplayHeight (dpy, screen);


    /* clip to window */
    if (absx < 0) width += absx, absx = 0;
    if (absy < 0) height += absy, absy = 0;
    if (absx + width > dwidth) width = dwidth - absx;
    if (absy + height > dheight) height = dheight - absy;

    XFetchName(dpy, window, &win_name);
    if (!win_name || !win_name[0]) {
	win_name = "xwdump";
	got_win_name = False;
    } else {
	got_win_name = True;
    }

    /* sizeof(char) is included for the null string terminator. */
    win_name_size = strlen(win_name) + sizeof(char);

    /*
     * Snarf the pixmap with XGetImage.
     */

    x = absx - win_info.x;
    y = absy - win_info.y;

    if (on_root)
	image = XGetImage (dpy, RootWindow(dpy, screen), absx, absy, width, height, AllPlanes, format);
    else
	image = XGetImage (dpy, window, x, y, width, height, AllPlanes, format);

    if (!image) {
	fprintf (stderr, "unable to get image at %dx%d+%d+%d\n",
		 width, height, x, y);
	exit (1);
    }

    if (add_pixel_value != 0) XAddPixel (image, add_pixel_value);

    /*
     * Determine the pixmap size.
     */
    buffer_size = Image_Size(image);
    ncolors = Get_XColors(&win_info, &colors);

    /*
     * Inform the user that the image has been retrieved.
     */
    XBell(dpy, FEEP_VOLUME);
    XBell(dpy, FEEP_VOLUME);
    XFlush(dpy);

    /*
     * Calculate header size.
     */
    header_size = sizeof(header) + win_name_size;
    if (debug) fprintf(stderr,"header_size= %ld win_name_size= %d \n",sizeof(header),win_name_size);
    if (debug) fprintf(stderr,"x rs %d y res %d size pix %d\n",image->width,image->height,image->depth);

    /*
     * Write out header information.
     */
    header.header_size = (CARD32) header_size;
    header.file_version = (CARD32) XWD_FILE_VERSION;
    header.pixmap_format = (CARD32) format;
    header.pixmap_depth = (CARD32) image->depth;
    header.pixmap_width = (CARD32) image->width;
    header.pixmap_height = (CARD32) image->height;
    header.xoffset = (CARD32) image->xoffset;
    header.byte_order = (CARD32) image->byte_order;
    header.bitmap_unit = (CARD32) image->bitmap_unit;
    header.bitmap_bit_order = (CARD32) image->bitmap_bit_order;
    header.bitmap_pad = (CARD32) image->bitmap_pad;
    header.bits_per_pixel = (CARD32) image->bits_per_pixel;
    header.bytes_per_line = (CARD32) image->bytes_per_line;
    header.visual_class = (CARD32) win_info.visual->class;
    header.red_mask = (CARD32) win_info.visual->red_mask;
    header.green_mask = (CARD32) win_info.visual->green_mask;
    header.blue_mask = (CARD32) win_info.visual->blue_mask;
    header.bits_per_rgb = (CARD32) win_info.visual->bits_per_rgb;
    header.colormap_entries = (CARD32) win_info.visual->map_entries;
    header.ncolors = ncolors;
    header.window_width = (CARD32) win_info.width;
    header.window_height = (CARD32) win_info.height;
    header.window_x = absx;
    header.window_y = absy;
    header.window_bdrwidth = (CARD32) win_info.border_width;

    if (*(char *) &swaptest) {
	_swaplong((char *) &header, sizeof(header));
	for (i = 0; i < ncolors; i++) {
	    _swaplong((char *) &colors[i].pixel, sizeof(long));
	    _swapshort((char *) &colors[i].red, 3 * sizeof(short));
	}
    }

    (void) fwrite((char *)&header, sizeof(header), 1, out);
    (void) fwrite(win_name, win_name_size, 1, out);

    /*
     * Write out the color maps, if any
     */
#ifdef NON_STANDARD_XCOLOR_SIZE
    {
     XColor *pt;
     for(pt=colors; pt < colors + ncolors; pt++)
       fwrite((((char *) pt) + NON_STANDARD_XCOLOR_SIZE),My_XColor_size,1, out);
    }
#else
    (void) fwrite((char *) colors, My_XColor_size, ncolors, out);
#endif

    if(ncolors > 0) free(colors);         /* free the color buffer */
    if (got_win_name) XFree(win_name);    /* free window name string */
    XDestroyImage(image);                 /* free image */
}

/* Report the syntax for calling xwd. */
/* changed from usage() to Usage() to avoid PC/X11e conflict */
gaint Usage() 
{
    fprintf (stderr,"old usage of xwd\n");
    exit(1);
}


/* Error - Fatal xwd error */
extern gaint errno;

gaint Error(string)
	char *string;	/* Error description string. */
{
	fflush(stdout);
	fprintf(stderr, "xwd: Error => %s\n", string);
	fflush(stderr);

	if (errno != 0) {
	  perror("xwd");
	  fprintf(stderr,"\n");
	}
	exit(1);
}


/*
 * Determine the pixmap size.
 */

gaint Image_Size(image)
     XImage *image;
{
    if (image->format != ZPixmap)
      return(image->bytes_per_line * image->height * image->depth);

    return(image->bytes_per_line * image->height);
}

#define lowbit(x) ((x) & (~(x) + 1))

/*
 * Get the XColors of all pixels in image - returns # of colors
 */
gaint Get_XColors(win_info, colors)
     XWindowAttributes *win_info;
     XColor **colors;
{
    gaint i, ncolors;
    Colormap cmap = win_info->colormap;

    if (use_installed)
	/* assume the visual will be OK ... */
	cmap = XListInstalledColormaps(dpy, win_info->root, &i)[0];
    if (!cmap)
	return(0);

    ncolors = win_info->visual->map_entries;
    if (!(*colors = (XColor *) malloc (sizeof(XColor) * ncolors)))
      {fprintf(stderr,"Out of memory!"); exit(-1); }

    if (win_info->visual->class == DirectColor ||
	win_info->visual->class == TrueColor) {
	gaPixel red, green, blue, red1, green1, blue1;

	red = green = blue = 0;
	red1 = lowbit(win_info->visual->red_mask);
	green1 = lowbit(win_info->visual->green_mask);
	blue1 = lowbit(win_info->visual->blue_mask);
	for (i=0; i<ncolors; i++) {
	  (*colors)[i].pixel = red|green|blue;
	  (*colors)[i].pad = 0;
	  red += red1;
	  if (red > win_info->visual->red_mask)
	    red = 0;
	  green += green1;
	  if (green > win_info->visual->green_mask)
	    green = 0;
	  blue += blue1;
	  if (blue > win_info->visual->blue_mask)
	    blue = 0;
	}
    } else {
	for (i=0; i<ncolors; i++) {
	  (*colors)[i].pixel = i;
	  (*colors)[i].pad = 0;
	}
    }

    XQueryColors(dpy, cmap, *colors, ncolors);
    
    return(ncolors);
}

void _swapshort (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;

    while (bp < ep) {
	c = *bp;
	*bp = *(bp + 1);
	bp++;
	*bp++ = c;
    }
}

void _swaplong (bp, n)
    register char *bp;
    register unsigned n;
{
    register char c;
    register char *ep = bp + n;
    register char *sp;

    while (bp < ep) {
	sp = bp + 3;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	sp = bp + 1;
	c = *sp;
	*sp = *bp;
	*bp++ = c;
	bp += 2;
    }
}

/*
 * outl: a debugging routine.  Flushes stdout then prints a message on stderr
 *       and flushes stderr.  Used to print messages when past certain points
 *       in code so we can tell where we are.  Outl may be invoked like
 *       printf with up to 7 arguments.
 */
/* VARARGS1 */
void outl(msg, arg0,arg1,arg2,arg3,arg4,arg5,arg6)
     char *msg;
     char *arg0, *arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
{
	fflush(stdout);
	fprintf(stderr, msg, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
	fprintf(stderr, "\n");
	fflush(stderr);
}
