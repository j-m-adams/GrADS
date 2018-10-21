
/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 *   gsgui: - A simple expression analyzer for gagui.
 *
 *   REVISION HISTORY:
 *
 *   22May97   da Silva   First alpha version.
 *   10Jun97   da Silva   Added CmdWin() callback.
 *   06Nov97   da Silva   Fixed small bug in SetWidgetIndex().
 *   18Feb98   da Silva   Removed "!" as a comment character.
 *   16Dec07   da Silva   Explicitly adopted GPL.

    Copyright (C) 1997-2007 by Arlindo da Silva <dasilva@opengrads.org>
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

#include <stdio.h>
#include <stdlib.h>

#include "libsx.h"
#if USEFREQ == 1
#include "freq.h"
#endif
#include "gagui.h"

/* Supported widget types */
#define OPENDISPLAY     1
#define SHOWDISPLAY     2
#define MAKELABEL       3
#define MAKEMENU        4
#define MAKEMENUITEM    5
#define MAKEBUTTON      6
#define MAINLOOP        7
#define SETWIDGETPOS    8
#define SETBGCOLOR      9
#define SETFGCOLOR      10
#define MAKETOGGLE      11
#define GETFONT         12
#define SETWIDGETFONT   13
#define MAKEWINDOW      14
#define CLOSEWINDOW     15
#define ALLWIDGETFONT   16
#define ALLFGCOLOR      17
#define ALLBGCOLOR      18
#define GETNAMEDCOLOR   19
#define DEBUGGUI        20
#define CHDIR           21

/* Widget function table */
struct Func_Table {
    char *name;
    int  func;
};
static
struct Func_Table Widgets[] = {   {"OpenDisplay",   OPENDISPLAY},
                                  {"ShowDisplay",   SHOWDISPLAY},
                                  {"MakeLabel",     MAKELABEL},
                                  {"MakeMenu",      MAKEMENU},
                                  {"MakeMenuItem",  MAKEMENUITEM},
                                  {"MakeButton",    MAKEBUTTON},
                                  {"MainLoop",      MAINLOOP}, 
                                  {"SetWidgetPos",  SETWIDGETPOS}, 
                                  {"SetFgColor",    SETFGCOLOR},
                                  {"SetBgColor",    SETBGCOLOR},
                                  {"MakeToggle",    MAKETOGGLE},
                                  {"GetFont",       GETFONT},
                                  {"SetWidgetFont", SETWIDGETFONT},
                                  {"MakeWindow",    MAKEWINDOW},
                                  {"CloseWindow",   CLOSEWINDOW},
                                  {"AllWidgetFont", ALLWIDGETFONT},
                                  {"AllFgColor",    ALLFGCOLOR},
                                  {"AllBgColor",    ALLBGCOLOR},
                                  {"GetNamedColor", GETNAMEDCOLOR},
                                  {"Debug",         DEBUGGUI},
                                  {"chdir",         CHDIR},
                                  {NULL,           -1} };




/* Callback function table */
struct CB_Table {
   char       *name;
   ButtonCB   func;
};
static
struct CB_Table CallBacks[] = {  {"NULL",     NULL},
                                 {"Exit",     CB_Exit},
                                 {"Open",     CB_Open},
                                 {"Load",     CB_Load},
                                 {"Cmd",      CB_Cmd},
                                 {"CmdStr",   CB_CmdStr},
                                 {"CmdWin",   CB_CmdWin},
                                 {"CmdLine",  CB_CmdLine},
                                 {"VarSel",   CB_VarSel},
                                 {"FileSel",  CB_FileSel},
                                 {"Display",  CB_Display},
                                 {"Toggle",   CB_Toggle},
                                 {"Browse",   CB_Browse},
                                 {"Edit",     CB_Edit},
                                 {"CloseWindow", CB_CloseWindow},
                                 {NULL,       NULL } };


/* Internal Macro Tables */ 
struct Macro_Table{
   char *name;
   int  value;
};
static
struct Macro_Table Macros [] = { {"NO_CARE",       NO_CARE},
                                 {"PLACE_RIGHT",   PLACE_RIGHT},
                                 {"PLACE_UNDER",   PLACE_UNDER},
                                 {"FALSE",         FALSE},
                                 {"TRUE",          TRUE},
                                 {NULL,            0} };


/* scanner specific data */

#define NONE      0            /* Token types */
#define DELIMITER 1
#define VARIABLE  2
#define STRING    3
#define COMMENT   4

#define NTOKEN  16             /* Maximum number of tokens per line */
#define LTOKEN  132            /* Maximum length of token */
static char *prog;             /* holds expression to be analyzed */
static char token[LTOKEN];     /* current token */
static char tok_type;          /* token type */


/* User defined widgets. TO DO: make this a linked list */
#define NWIDGETS 512       /* Maximum number of widgets */
static int iwidgets = -1;  /* index of last entry in user widget table */
struct User_Widgets {
    char    name[LTOKEN];  /* short name used by the script */
    Widget  w;             /* widget data structure */
    char    data[LTOKEN];  /* data to be passed to callbacks */
};
static struct User_Widgets UserWidgets[NWIDGETS];

/* User loaded fonts */
#define NFONTS 16          /* max number of user fonts allowed  */
#define LFONTS 80          /* max size of font string name */
static int ifonts=-1;      /* index of last entry in function table */
struct User_Fonts {
    char  name[LFONTS];    /* short name used by the script */
    XFont font;            /* font data structure */
};
static struct User_Fonts UserFonts[NFONTS];

/* User colors */
#define NCOLORS 32          /* max number of user fonts allowed (>=6) */
#define LCOLORS 32          /* max size of color string name */
static int icolors=-1;      /* index of last entry in function table */
static int FirstColor=1;    /* flag for standard colors creation */
struct User_Colors {
    char  name[LCOLORS];    /* short name used by the script */
    int   color;            /* font data structure */
};
static struct User_Colors UserColors[NCOLORS];

/*--------------------------- SCANNER ------------------------------------*/

/*
 * is_in()  -  Check if char is in string
 *
 */
int
is_in(char ch, char *s)
{
  while(*s) if(*s++==ch) return 1;
  return 0;
}


/*
 * iswhite()  -  Look for spaces and tabs 
 */
int
iswhite(char c)
{
  if(c == ' ' || c == 9) return 1;
  return 0;
}
			

/*
 * isdelim()  -  Look for delimiter 
 */
int
isdelim(char c)
{
  if(is_in(c,"(,)") || c==0) return 1;
  return 0;
}


/*
 *  get_token - return tokens from the input string.
 *
 */

void get_token()
{
  char *temp;
  
  tok_type = NONE;
  temp = token;

  while(iswhite(*prog)) ++prog;   /* skip over white spaces */

  if(is_in(*prog,"(,)")) {
    tok_type = DELIMITER;
    *temp++ = *prog++;            /* advance to next position */
  }
  else if(is_in(*prog,"#*")) {
    tok_type = COMMENT;
    *temp = '\0';
    *temp++ = *prog++;            /* advance to next position */
  }
  else if(isalpha(*prog)||isdigit(*prog)||is_in(*prog,"-+")) {
    while(!isdelim(*prog)) *temp++ = *prog++;
    tok_type = VARIABLE;
  }
  else if(is_in(*prog,"\"'")) {
    ++prog;  /* remove " or ' */
    while(!isdelim(*prog)) *temp++ = *prog++;
    tok_type = STRING;
  }

  /* Note: Variables can start with number, for now, at least */

  *temp = '\0';
  temp--;
  while(iswhite(*temp) ) {
        *temp='\0'; temp--;
  }
  if(tok_type==STRING ) {
     if(*temp=='"'||*temp=='\'') *temp='\0'; /* very forgiving... */
  }

}

/*-------------------------- USER TABLE MANAGEMENT -----------------------*/

/*
 * GetGuiFuncIndex() - returns widget function index
 *
 */
int
GetGuiFuncIndex(char *name)
{
   int i, func=-1;

   for(i=0;i<NWIDGETS;i++) {
       if(Widgets[i].name == NULL) break;
       if(!strcmp(Widgets[i].name, name)) {
          func = Widgets[i].func;
          break;
       }
   }
   return(func);
}


/*
 * GetCallBackIndex() - returns callback function index
 *
 */
int
GetCallBackIndex(char *name)
{
   int i, index=-1;

   for(i=0;;i++) {
       if(CallBacks[i].name == NULL) break;
       if(!strcmp(CallBacks[i].name, name)) {
          index = i;          
          break;
       }
   }
   return(index);
}

/*
 * GetMacroValue() - returns libsx "macro" values
 *
 */
int
GetMacroValue(char *name)
{
   int i, value=-1;  /* make sure this is not used */

   for(i=0;;i++) {
       if(Macros[i].name == NULL) break;
       if(!strcmp(Macros[i].name, name)) {
          value = Macros[i].value;
          break;
       }
   }
   return(value);
}

/*
 * SetWidgetIndex - creates an entry for a widget name.
 *
 */


/*
 * GetWidgetIndex - returns index for a widget name created with 
 *                  SetWidgetIndex().
 *
 */
int
GetWidgetIndex(char *name)
{
   int i, index=-1;
   for(i=0;i<=iwidgets;i++) {
       if(!strcmp(UserWidgets[i].name, name)) {
          index = i;
          break;
       }
   }
   return index;
}

int
SetWidgetIndex(char *name)
{
   int i;

   /* Very first time */
   if(iwidgets<0) {
      iwidgets=0;
      strcpy(UserWidgets[0].name, "NULL");
      UserWidgets[0].w = (Widget) NULL;  /* need this for SetWidgetPos() */
   }

   /* If a widget with same name exists, return its index */
   if ( iwidgets>0 ) {
        if ( (i=GetWidgetIndex(name)) >= 0 ) return i;
   }

   iwidgets++;
   if(iwidgets>NWIDGETS-1) {
      iwidgets--;
      printf("GUI Error: too many widgets\n");
      return -1;
   } else i = iwidgets;
   strcpy(UserWidgets[i].name, name);
   return i;
}

/*
 * LoadUserFont() - loads and creates an entry for a font
 *
 */

int
LoadUserFont(char *name,         /* a short name used by the script */ 
             char *Xfontname)    /* the actual X11 name used for the font */
{
   XFont font;

   ifonts++;
   if(ifonts>NFONTS-1) {
      ifonts--;
      printf("GUI Error: too many fonts");
      return -1;
   } 

   font = GetFont(Xfontname);
   if ( font == NULL ) {
        ifonts--;
        printf("GUI Error: could not get font %s\n", Xfontname);
        return -1;
   }
   strncpy(UserFonts[ifonts].name, name, LFONTS);
   UserFonts[ifonts].font = font;

   return ifonts;
}


/*
 * GetFontIndex - returns index for user fonts created with
 *                LoadUserFont()
 */
int
GetFontIndex(char *fontname)  /* fontname is a short name
                                 used by the script */
{
   int i, index=-1;
   for(i=0;i<=ifonts;i++) {
       if(!strcmp(UserFonts[i].name, fontname)) {
          index = i;
          break;
       }
   }
   return index;
}


/*
 * LoadUserColor() - load named color 
 *
 */

int
LoadUserColor(char *name,         /* a short name used by the script */ 
              char *colorname)    /* the actual X11 name used for the color */
{
   int color;

   if ( FirstColor ) {
      FirstColor = 0;
      GetStandardColors();
      strcpy(UserColors[0].name, "white"  );
      strcpy(UserColors[1].name, "black"  );
      strcpy(UserColors[2].name, "red"    );
      strcpy(UserColors[3].name, "green"  );
      strcpy(UserColors[4].name, "blue"   );
      strcpy(UserColors[5].name, "yellow" );
      UserColors[0].color = WHITE;
      UserColors[1].color = BLACK;
      UserColors[2].color = RED;
      UserColors[3].color = GREEN;
      UserColors[4].color = BLUE;
      UserColors[5].color = YELLOW;
      icolors = 5;
      if(!name) return;
   }

   icolors++;
   if(icolors>NCOLORS-1) {
      icolors--;
      printf("GUI Error: too many colors");
      return -1;
   } 

   color = GetNamedColor(colorname);
   if ( color<0 ) {
        icolors--;
        printf("GUI Error: could not get color %s\n", colorname);
        return -1;
   }
   strncpy(UserColors[icolors].name, name, LCOLORS);
   UserColors[icolors].color = color;

   return icolors;
}


/*
 * GetColorIndex - returns index for user colors created with
 *                 LoadUserColor()
 */
int
GetColorIndex(char *colorname)  /* colorname is a short name
                                  used by the script */
{
   int i, color=-1;

   if ( FirstColor ) {
        LoadUserColor(NULL, NULL);
   }

   for(i=0;i<=icolors;i++) {
       if(!strcmp(UserColors[i].name, colorname)) {
          color = UserColors[i].color;
          break;
       }
   }
   return color;
}


/*----------------------------- GUI INTERPRETER -----------------------------*/

/*
 *   Custom_GUI()  -  Reads a GUI script and executes it.
 */

int
Custom_GUI ( char *fname )
{

    FILE *script;
    int  argc;
    char **argv;
    char *p;
    int  Func, i, j, k, m1, m2;
    int  debug=0;

    /* open script file */
    script = fopen(fname,"r");
    if ( script == NULL ) {
      printf("GUI Error: cannot open GUI script file %s.\n", fname);
      return 1;
    }

    /* allocate space to hold script lines & tokens */
    p = malloc(256*sizeof(char));    
    argv = (char **) List(NTOKEN,LTOKEN);
    if(!argv) return 1;

    /* Main loop begins ... */
    do {
       
       /* Read and parse next line */
       prog = p;
       if(fgets(p, 256, script)==NULL) break ;
       argc = -1;
       do {
          get_token();
          if(tok_type==STRING||tok_type==VARIABLE) {
              argc++;
              if(argc>=NTOKEN) break;
              strcpy(argv[argc], token);
          } else if(tok_type==COMMENT) break;
       } while(token[0]);

       /* comment or blank line, skip it */
       if(argc<0) {
	 if(debug) printf("%s",p);
	 goto nxtline;  
       }

       /* if not a GUI function, must be a native GrADS command,
          go for it */
       if((Func=GetGuiFuncIndex(argv[0]))<0) {
           CB_Cmd(NULL,p);
           goto nxtline;
       }

       if(debug) printf("%s",p);

       /* Process each widget function */
       switch(Func) {


       case SHOWDISPLAY:
         ShowDisplay();
         break;

       case MAINLOOP:
         MainLoop();
         break;

       case MAKELABEL:
	 if (argc!=2) break;
         if((i=SetWidgetIndex(argv[1]))<0) break;
         UserWidgets[i].w = MakeLabel(argv[2]);
         break;

       case MAKEBUTTON:
	 if (argc!=4) break;
         if((i=SetWidgetIndex(argv[1]))<0) break;
         if((j=GetCallBackIndex(argv[3]))<0) break;
         strcpy(UserWidgets[i].data, argv[4]);
         UserWidgets[i].w = MakeButton(argv[2], CallBacks[j].func, 
                                       UserWidgets[i].data);         
         break;

       case MAKEMENU:
	 if (argc!=2) break;
         if((i=SetWidgetIndex(argv[1]))<0) break;
	 UserWidgets[i].w = MakeMenu( argv[2] );
	 break;

       case MAKEMENUITEM:
	 if (argc!=5) break;
         if((i=SetWidgetIndex(argv[1]))<0) break;
         if((j=GetWidgetIndex(argv[2]))<0) break;
         if((k=GetCallBackIndex(argv[4]))<0) break;
         strcpy(UserWidgets[i].data, argv[5]);
         UserWidgets[i].w = MakeMenuItem(UserWidgets[j].w, 
                                 argv[3], CallBacks[k].func, 
                                 UserWidgets[i].data);         
	 break;

       case MAKETOGGLE:
	 if (argc!=6) break;
         if((i=SetWidgetIndex(argv[1]))<0) break;
         if((m1=GetMacroValue(argv[3]))<0) break;
         if((j=GetWidgetIndex(argv[4]))<0) break;
         if((k=GetCallBackIndex(argv[5]))<0) break;
         strcpy(UserWidgets[i].data, argv[6]);
         UserWidgets[i].w = MakeToggle(argv[2], m1, UserWidgets[j].w,
                            CallBacks[k].func, UserWidgets[i].data);
         break;          

       case SETWIDGETPOS:
	 if (argc!=5) break;
         if((i=GetWidgetIndex(argv[1]))<0) break;
         if((m1=GetMacroValue(argv[2]))<0) break;
         if((j=GetWidgetIndex(argv[3]))<0) break;
         if((m2=GetMacroValue(argv[4]))<0) break;
         if((k=GetWidgetIndex(argv[5]))<0) break;
         SetWidgetPos(UserWidgets[i].w, m1, UserWidgets[j].w,
                                        m2, UserWidgets[k].w );
         break;

       case SETFGCOLOR:
	 if (argc!=2) break;
         if((i=GetWidgetIndex(argv[1]))<0) break;
         if((m1=GetColorIndex(argv[2]))<0) break;
         SetFgColor(UserWidgets[i].w, m1);
         break;

       case ALLFGCOLOR:
	 if (argc!=1) break;
         if((m1=GetColorIndex(argv[1]))<0) break;
         for(i=0; i<=iwidgets; i++ ) {
             SetFgColor(UserWidgets[i].w, m1);
         }
         break;

       case SETBGCOLOR:
	 if (argc!=2) break;
         if((i=GetWidgetIndex(argv[1]))<0) break; 
         if((m1=GetColorIndex(argv[2]))<0) break;
         SetBgColor(UserWidgets[i].w, m1);
         break;

       case ALLBGCOLOR:
	 if (argc!=1) break;
         if((m1=GetColorIndex(argv[1]))<0) break;
         for(i=0; i<iwidgets; i++ ) {
             SetBgColor(UserWidgets[i].w, m1);
         }
         break;

       case GETFONT:
	 if (argc!=2) break;
         LoadUserFont(argv[1],argv[2]);
         break;

       case GETNAMEDCOLOR:
	 if (argc!=2) break;
         LoadUserColor(argv[1],argv[2]);
         break;

       case SETWIDGETFONT:
         if (argc!=2) break;
         if((i=GetWidgetIndex(argv[1]))<0) break; 
         if((j=GetFontIndex(argv[2]))<0) break; 
         SetWidgetFont(UserWidgets[i].w,UserFonts[j].font);
         break;

       case ALLWIDGETFONT:
         if (argc!=1) break;
         if((j=GetFontIndex(argv[1]))<0) break; 
         for(i=0; i<=iwidgets; i++ ) {
             SetWidgetFont(UserWidgets[i].w,UserFonts[j].font);
         }
         break;

       case MAKEWINDOW:
         if (argc!=2) break;
         if((i=SetWidgetIndex(argv[1]))<0) break; 
	 UserWidgets[i].w = MakeWindow(argv[2], SAME_DISPLAY, 
                                                NONEXCLUSIVE_WINDOW);
         break;

       case CLOSEWINDOW:
         if (argc!=1) break;
         if((i=GetWidgetIndex(argv[1]))<0) break; 
         SetCurrentWindow(UserWidgets[i].w);
         CloseWindow();
         SetCurrentWindow(ORIGINAL_WINDOW);
         break;


       case DEBUGGUI:
	 if (argc<1) {
	    debug = 1 - debug;
	    break;
	 } else if ( argc == 1 ) {
	   if (!strcmp(argv[1], "ON" )  || !strcmp(argv[1], "on" )) debug=1;
	   if (!strcmp(argv[1], "OFF" ) || !strcmp(argv[1], "off")) debug=0;
	   break;
	 }
         break;
	 
       case CHDIR:
         if (argc!=1) break;
	 chdir(argv[1]);
         break;

       }
 
       nxtline:
         continue;

    } while(*p);


    /* close file, deallocate memory */
    fclose(script);
    if(p) free(p);
    Free_List(argv,NTOKEN);

    return 0;

}


/*------------------------ OPTIONAL TEST CODE ----------------------------*/

#ifdef TEST

main(int argc, char **argv)
{
    Custom_GUI("sample.gui");
}

/*
 *   fake callbacks for prototyping
 */


void CB_Load(Widget w, void *data)
{

  printf("*** Load callback: %s ***\n", data);

}


/*
 * CB_Cmd() - Callback funtion for a generic grads command  button.
 */
void CB_Cmd(Widget w, void *data)
{
  printf("*** Cmd callback: %s ***\n", data);

}


/*
 *  CB_CmdLine() - Callback funtion for the command line button.
 */
void CB_CmdLine(Widget w, void *data)
{
  printf("*** CmdLine callback ***\n");

}


/*
 * CB_VarSel() - Callback for selecting a variable.
 *
 */
void CB_VarSel(Widget W, void *data)
{
  printf("*** VarSel callback: %s ***\n", data);

}


/*
 * CB_Display() - Callback function for displaying the default variable.
 *
 */

void CB_Display(Widget w, void *data)
{
  printf("*** Display callback: %s ***\n", data);

}


/*
 * CB_Toggle() - Callback funtion for a generic toggle button.
 */
void CB_Toggle(Widget w, void *data)
{
  printf("*** Toggle callback: %s ***\n", data);

}


/*
 * CB_FileSel() - Callback for selecting an already open file.
 *
 */
void CB_FileSel(Widget W, void *data)
{
  printf("*** FileSel callback: ***\n");

}

/*
 * CB_CmdStr() - Callback funtion for a generic grads command with user input.
 */
void CB_CmdStr(Widget w, void *data)
{
  printf("*** CmdStr callback: %s ***\n", data);

}

#endif   /* TEST */


