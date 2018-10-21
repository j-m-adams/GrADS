/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/*  This file contains the function prototypes for Cairo/grads interface. */

/*
gxCaa       -- Turn anti-aliasing on or off
gxCbatch    -- Initialize an image surface in batch mode (for getting string lengths)
gxCbfil     -- Begin a polygon fill
gxCbgn      -- Initialize the X interface in interactive mode
gxCch       -- Draw a character
gxCclip     -- Set the clipping coordinates
gxCcol      -- Set the new color
gxCdrw      -- Draw to
gxCend      -- Close down the interactive session 
gxCfil      -- Draw a filled polygon
gxCflush    -- Finish up any pending actions
gxCfrm      -- Clear the frame
gxCftinit   -- Initialize the FreeType library
gxCftend    -- Close the FreeType Library
gxChinit    -- Initialize a hardcopy surface 
gxChend     -- End the hardcopy drawing 
gxCmov      -- Move to
gxCpattc    -- Create a tile pattern
gxcpattrset -- Reset a pattern
gxCqchl     -- Get the width of a character
gxCrec      -- Draw a filled rectangle
gxCrsiz     -- Resize the interactive surface
gxCselfont  -- Set the font 
gxCsfc      -- Make a surface active for drawing 
gxCswp      -- Swap action (for double buffering)
gxCwid      -- Set the line width
gxCxycnv    -- Convert position coordinates 
*/


#include <cairo.h>

void gxCcfg (void);
void gxCaa (gaint);
void gxCbatch (gadouble , gadouble);
void gxCbfil (void); 
void gxCbgn (cairo_surface_t *, gadouble , gadouble, gaint, gaint);
gadouble gxCch (char, gaint, gadouble, gadouble, gadouble, gadouble, gadouble);
void gxCclip (gadouble, gadouble, gadouble, gadouble);
void gxCcol (gaint);
void gxCdrw (gadouble, gadouble);
void gxCend (void);
void gxCfil (gadouble*, gaint);
void gxCflush (gaint);
void gxCfrm (void);
void gxCftinit (void);
void gxCftend (void);
gaint gxChinit (gadouble, gadouble, gaint, gaint, gaint, char *, gaint, char *, gadouble);
gaint gxChend (char*, gaint, char *);
void gxCmov (gadouble, gadouble);
void gxCpattc (gaint);
void gxCpattrset (gaint);
void gxCpop (void);
void gxCpush (void);
gadouble gxCqchl (char, gaint, gadouble);
void gxCrec (gadouble, gadouble, gadouble, gadouble);
void gxCrsiz (gaint, gaint);
void gxCselfont (gaint);
void gxCsfc (cairo_surface_t *);
void gxCswp (cairo_surface_t *,cairo_surface_t *);
void gxCwid (gaint);                
void gxCxycnv (gadouble, gadouble, gadouble*, gadouble*);
