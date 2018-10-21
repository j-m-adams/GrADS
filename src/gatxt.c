/*
    Copyright (C) 2009 by Arlindo da Silva <dasilva@opengrads.org>
    All Rights Reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; using version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, please consult  
              
              http://www.gnu.org/licenses/licenses.html

    or write to the Free Software Foundation, Inc., 59 Temple Place,
    Suite 330, Boston, MA 02111-1307 USA

 */

/* Simple functions to produce color text using ANSI color sequences,
   see:

        http://en.wikipedia.org/wiki/ANSI_escape_code

   The main function used from grads is gatxtl(string,level) which
   colorizes a *string* given a "level" as in gaprnt(), except that
   level=-1 means the prompt.

   The actual colors used dependend on the color scheme specified
   during initialization; see gatxtl() below for the specific colors.
   Usually,

   color scheme          works well with a
   ------------          -----------------
       0                 black background
       1                 white background
       2                 green background

*/


#include <stdio.h>
#include "gatypes.h"

static gaint color_on = 0; /* off by default */
static gaint scheme   = 0;   /* color scheme */

/* Normal colors */
static char *black   = "[30m";
static char *red     = "[31m";
static char *green   = "[32m";
static char *yellow  = "[33m";
static char *blue    = "[34m";
static char *magenta = "[35m";
static char *cyan    = "[36m";
static char *white   = "[37m";
static char *reset   = "[39m";

static char *normal  = "[0m";
static char *bold    = "[1m";

/* Normal colors */
/* static char *Black   = "[90m"; */
/* static char *Red     = "[91m"; */
/* static char *Green   = "[92m"; */
/* static char *Yellow  = "[93m"; */
/* static char *Blue    = "[94m"; */
/* static char *Magenta = "[95m"; */
/* static char *Cyan    = "[96m"; */
/* static char *White   = "[97m"; */

void gatxti(gaint on, gaint cs) {  /* Turn this feature ON/OFF */
  color_on = on;
  if ( cs < 0 ) cs = 0;
  scheme = cs;
}

/* Print ANSI sequence associated with a color name.
   Available options for *nomal* intensite colors are:

          black  
          red    
          green  
          yellow 
          blue   
          magenta
          cyan   
          white  

    Bright colors are specified by capitalizing the first leter,
    e.g., "Red". Specify color=NULL for a reset.

*/

void gatxt(char *color) {
  if ( !color_on ) return;
  if ( color==NULL ) {
    printf("%s",reset);
    return;
  }

  /* Normal */
       if ( color[0]=='b' && 
            color[2]=='a' ) printf("%s",black);
  else if ( color[0]=='r' ) printf("%s",red);
  else if ( color[0]=='g' ) printf("%s",green);
  else if ( color[0]=='y' ) printf("%s",yellow);
  else if ( color[0]=='b' ) printf("%s",blue);
  else if ( color[0]=='m' ) printf("%s",magenta);
  else if ( color[0]=='c' ) printf("%s",cyan);
  else if ( color[0]=='w' ) printf("%s",white);

  else if ( color[0]=='o' ) printf("%s",normal);
  else if ( color[0]=='*' ) printf("%s",bold);

  /* Bright colors */
  else if ( color[0]=='B' && 
            color[2]=='a' ) printf("%s",black);
  else if ( color[0]=='R' ) printf("%s",red);
  else if ( color[0]=='G' ) printf("%s",green);
  else if ( color[0]=='Y' ) printf("%s",yellow);
  else if ( color[0]=='B' ) printf("%s",blue);
  else if ( color[0]=='M' ) printf("%s",magenta);
  else if ( color[0]=='C' ) printf("%s",cyan);
  else if ( color[0]=='W' ) printf("%s",white);

}

static char buffer[256];
#define COLORIZE(c) snprintf(buffer,255,"%s%s%s",c,str,reset)

char *gatxts(char *str, char *color) { /* colorize the string */


  if ( !color_on ) return str;

  /* Normal */
       if ( color[0]=='b' && 
            color[2]=='a' ) COLORIZE(black);
  else if ( color[0]=='r' ) COLORIZE(red);
  else if ( color[0]=='g' ) COLORIZE(green);
  else if ( color[0]=='y' ) COLORIZE(yellow);
  else if ( color[0]=='b' ) COLORIZE(blue);
  else if ( color[0]=='m' ) COLORIZE(magenta);
  else if ( color[0]=='c' ) COLORIZE(cyan);
  else if ( color[0]=='w' ) COLORIZE(white);

  /* Bright colors */
  else if ( color[0]=='B' && 
            color[2]=='a' ) COLORIZE(black);
  else if ( color[0]=='R' ) COLORIZE(red);
  else if ( color[0]=='G' ) COLORIZE(green);
  else if ( color[0]=='Y' ) COLORIZE(yellow);
  else if ( color[0]=='B' ) COLORIZE(blue);
  else if ( color[0]=='M' ) COLORIZE(magenta);
  else if ( color[0]=='C' ) COLORIZE(cyan);
  else if ( color[0]=='W' ) COLORIZE(white);

  buffer[255] = '\0';
  return (char *) buffer;

}

char *gatxtl(char *str, gaint level) { /* colorize according to level */

  if ( scheme==0 ) {
    if (level==-1) return gatxts(str,"Green"); /* prompt */
    if (level==0 ) return gatxts(str,"Red");
    if (level==1 ) return gatxts(str,"magenta");
    if (level==2 ) return gatxts(str,"yellow");
  }
  else if ( scheme==1 ) {
    if (level==-1) return gatxts(str,"Green"); /* prompt */
    if (level==0) return gatxts(str,"Red");
    if (level==1) return gatxts(str,"magenta");
    if (level==2) return gatxts(str,"blue");
  }
  else if ( scheme==2 ) {
   if (level==-1) return gatxts(str,"Blue"); /* prompt */
    if (level==0) return gatxts(str,"black");
    if (level==1) return gatxts(str,"magenta");
    if (level==2) return gatxts(str,"white");
  } 
  return (str);
}
