/*

    Copyright (C) 1997-2011 by Arlindo da Silva <dasilva@opengrads.org>
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

/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 *   Simple GrADS GUI interface based on libsx. The script interpreter
 *   Custom_GUI() is implemented in file "gsgui.c".
 *
 *   REVISION HISTORY:
 *
 *   22May97   da Silva   First alpha version.
 *   09Jun97   da Silva   Fixed CB_Cmd which cause the command string
 *                        to be destroyed on the first click;
 *                        Fixed small bug on the CB_VAR window: now
 *                        the user can type an expression with blanks.
 *   10Jun97   da Silva   Added CmdWin() callback.
 *   19Sep97   da Silva   Fixed small bug in CB_Display().
 *   10Oct97   da Silva   Revised Default_GUI().
 *   11Mar06   da Silva   Explicitly adopted GPL.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "libsx.h"		
#if USEFREQ == 1
#include "freq.h"	
#endif
#include "grads.h"
#include "gx.h"

extern struct gacmn gcmn;

#include "gagui.h"

static char default_var[128];         /* default variable for display */
static char last_path_open[512];      /* remember last path names */
static char last_path_sdfopen[512];
static char last_path_exec[512];
static char last_path_run[512];

static Widget var_window;        /* window for variable selection */
static Widget file_window;       /* window for file selection */
static Widget expr_window;       /* window for variable selection */
static int hold_on = 0;          /* controls 'clear' before displaying */

#define MAXROWS 256             /* for temporary mem aloocation */
#define MAXCOLS 256

#define NCMD 64             /* Sixe of command buffer */
#define LCMD 132            /* Max length of command */
static int  CmdWinON = 0;              /* Make sure there is only one 
                                          Command Win up */
static char **CmdWinList;   /* Command buffer */
static Widget Cmd_window, CmdExpr_window, CmdList_window, CmdStr_window;

/*---------------------------------------------------------------------*

/*
 * This is the GaGUI entry point. Return is thru the quit button.
 *
 */

int gagui_main(int argc, char **argv)
{

/*
  static char *argv[] = { "GrADS", "-bg", "gray", NULL, NULL };
  static int   argc = 3;
*/

  char *path;

  if ( gcmn.batflg ) return 0;  /* batch mode */

  argc = OpenDisplay(argc, argv);
  if (argc == FALSE)
    return argc;

/*
  printf("Athena Widgets Support, (c) 1997 by Arlindo da Silva\n");
  printf("Data Assimilation Office, NASA/GSFC\n\n");
*/

  /* Default path for file loads */
  strcpy(last_path_open,"./");
  strcpy(last_path_sdfopen,"./");
  strcpy(last_path_exec,"./");
  strcpy(last_path_run,"./");
  if ( path = getenv("GADATA") ) {
       strcpy(last_path_open,path);
       strcpy(last_path_sdfopen,path);
  }
  if ( path = getenv("GASCRP") ) {
       strcpy(last_path_exec,path);
       strcpy(last_path_run,path);
  }
  default_var[0] = '\0';

  /* Widget request on startup */
  if ( path = getenv( "GAGUI" ) ) {

       /* Built in default GUI interface */
       if ( !strcmp(path,"default") ) {
          argc = Default_GUI(argc, argv, NULL);
          if (argc == 0) exit(0);
          printf("     <<< Click on the RED button for the 'ga>' prompt >>>\n\n");
          MainLoop();

       /* Widget set from a script */
       } else {
         if(Custom_GUI(path)) return 1;
         MainLoop();
       }

  }

}


/* List() - Allocates memory for character list */
char **List(int rows,int cols)
{
  int  i;
  char **p;

  /* Allocate pointers to rows */
  p = ( char ** ) calloc ( rows, sizeof(char *) );
  if(!p) {
    printf("Error: cannot allocate memory for list (rows)\n");
    return (char **)NULL;
  }

  /* Allocate rows and set pointers to them */
  for(i=0; i<rows; i++) {
      p[i] = ( char * ) calloc ( cols, sizeof(char) );
      if(!p[i]) {
	printf("Error: cannot allocate memory for list (cols)\n");
	return (char **)NULL;
      }
  }

  return (char **)p;
}

/* Free_List() - De-allocates memory for character list */
void Free_List(char **list, int rows)
{
   int i;
   for(i=0;i<rows;i++) free((char *)list[i]);
   free((char *)list);
}

/*
 * Return the directory component of a string or NULL if it's in
 * the current directory.
 */
char *dirname(char *str)
{
  char *ptr, *tmp;

  ptr = strrchr(str, '/');
  if (ptr == NULL)
    return NULL;

  *ptr = '\0';
  tmp = strdup(str);

  *ptr = '/';

  return tmp;
}



/*
 *  Default_GUI() - sets up default widget set.
 */

int 
Default_GUI(int argc, char **argv, void *data)
{
  Widget  root, file, print, options, quit, reinit,prompt;
  Widget  clear, hold, prev, play, next, dim, var;
  char str[256];
  int i, gold, blue, gray;

/*
  argc = OpenDisplay(argc, argv);
  if (argc == FALSE)
    return argc;
*/

  snprintf(str,255, "GrADS Version " GRADS_VERSION "\n");
  root = MakeLabel(str);

  file = MakeMenu ( "File" );
         MakeMenuItem(file, "Open",            CB_Load,    "open");
         MakeMenuItem(file, "SDF Open",        CB_Load,    "sdfopen");
         MakeMenuItem(file, "XDF Open",        CB_Load,    "xdfopen");
         MakeMenuItem(file, "File Selection ", CB_FileSel, NULL );
         MakeMenuItem(file, "_______________", NULL,       NULL );
         MakeMenuItem(file, "Exec",            CB_Load,    "exec");
         MakeMenuItem(file, "Run",             CB_Load,    "run");
         MakeMenuItem(file, "GUI",             CB_Load,    "gui");
         MakeMenuItem(file, "_______________", NULL,       NULL );
         MakeMenuItem(file, "Refresh",         CB_Cmd,     "  ");
         MakeMenuItem(file, "Reinit",          CB_Cmd,     "reinit");
         MakeMenuItem(file, "Exit",            CB_Exit,    NULL );

  print = MakeMenu ( "Print" );
          MakeMenuItem(print, "Print",         CB_Cmd,     "print");
          MakeMenuItem(print, "Enable  Print", CB_Load,    "enable print");
          MakeMenuItem(print, "Disable Print", CB_Cmd,     "disabled print");

  options = MakeMenu ( "Options" );
            MakeMenuItem(options, "Shaded",      CB_Cmd, "set gxout shaded" );
            MakeMenuItem(options, "Contour",     CB_Cmd, "set gxout contour" );
            MakeMenuItem(options, "Grid Fill",   CB_Cmd, "set gxout grfill" );
            MakeMenuItem(options, "Grid Values", CB_Cmd, "set gxout grid" );
            MakeMenuItem(options, "Vector",      CB_Cmd, "set gxout vector" );
            MakeMenuItem(options, "Streamlines", CB_Cmd, "set gxout stream" );
            MakeMenuItem(options, "Bar Chart",   CB_Cmd, "set gxout bar" );
            MakeMenuItem(options, "Line Plot",   CB_Cmd, "set gxout line" );
            MakeMenuItem(options, "Wind Barbs",  CB_Cmd, "set gxout  barb" );
            MakeMenuItem(options, "_______________",  NULL, NULL );
            MakeMenuItem(options, "Contour Interval",  CB_CmdStr, "set cint" );
            MakeMenuItem(options, "Draw Title", CB_CmdStr, "draw title " );
            MakeMenuItem(options, "Color Bar",  CB_Cmd, "run cbarn" );

  reinit  = MakeButton("Reinit",  CB_Cmd,     "reinit" );
  clear  = MakeButton("Clear",  CB_Cmd,     "clear" );
  quit   = MakeButton("Quit",   CB_Cmd,     "quit"  );
  prompt = MakeButton("ga>",  CB_CmdWin, NULL );

  dim    = MakeMenu("Dim");
           MakeMenuItem(dim, "Longitude", CB_CmdStr, "set lon " );
           MakeMenuItem(dim, "Latitude",  CB_CmdStr, "set lat " );
           MakeMenuItem(dim, "Level",     CB_CmdStr, "set lev " );
           MakeMenuItem(dim, "Time",      CB_CmdStr, "set time " );
           MakeMenuItem(dim, "_________", NULL,      NULL );
           MakeMenuItem(dim, "x",         CB_CmdStr, "set x " );
           MakeMenuItem(dim, "y",         CB_CmdStr, "set y " );
           MakeMenuItem(dim, "z",         CB_CmdStr, "set z " );
           MakeMenuItem(dim, "t",         CB_CmdStr, "set t " );

  var    = MakeButton("Var",    CB_VarSel,  NULL );
  hold   = MakeToggle("Hold",   FALSE, NULL, CB_Toggle,  "hold" );
  prev   = MakeButton("<<",   CB_Display, "<<" );
  play   = MakeButton("Display",   CB_Display, "DISPLAY" );
  next   = MakeButton(">>",   CB_Display, ">>" );


  /* First row */
  SetWidgetPos(file,    PLACE_UNDER, root, NO_CARE, NULL);
  SetWidgetPos(print,   PLACE_UNDER, root, PLACE_RIGHT, file );
  SetWidgetPos(options, PLACE_UNDER, root, PLACE_RIGHT, print );
  SetWidgetPos(dim,     PLACE_UNDER, root, PLACE_RIGHT, options );
  SetWidgetPos(reinit,     PLACE_UNDER, root, PLACE_RIGHT, dim);
  SetWidgetPos(prompt,  PLACE_UNDER, root, PLACE_RIGHT, reinit );

  /* Second row */
  SetWidgetPos(hold,   PLACE_UNDER, file, NO_CARE, NULL);
  SetWidgetPos(var,   PLACE_UNDER, file, PLACE_RIGHT, hold );
  SetWidgetPos(prev,  PLACE_UNDER, file, PLACE_RIGHT, var );
  SetWidgetPos(play,  PLACE_UNDER, file, PLACE_RIGHT, prev );
  SetWidgetPos(next,  PLACE_UNDER, file, PLACE_RIGHT, play );
  SetWidgetPos(clear, PLACE_UNDER, file, PLACE_RIGHT, next );
  SetWidgetPos(quit,  PLACE_UNDER, file, PLACE_RIGHT, clear );

  ShowDisplay();
  
  GetStandardColors();
  gold = GetNamedColor("gold");
  blue = GetNamedColor("LightSkyBlue");
  gray = GetNamedColor("gray");

  /* Color of widgets */
  SetFgColor(root,RED);
  SetFgColor(prompt,YELLOW);
  SetBgColor(prompt,RED);

  SetBgColor(file,blue);
  SetBgColor(print,blue);
  SetBgColor(options,blue);
  SetBgColor(dim,blue);

  SetBgColor(reinit,gray);
  SetBgColor(var,gray);
  SetBgColor(hold,gray);
  SetBgColor(clear,gray);
  SetBgColor(quit,gray);
  
  SetBgColor(prev,gold);
  SetBgColor(play,gold);
  SetBgColor(next,gold);

  return argc;
}

/*
 * CB_Open() - Callback function for (sdf) opening a file.  The opened file
 *             becomes the default and the user is asked to select a
 *             variable from the file. Unlike CB_Load(), the user must
 *             provide the file name.
 *
 */
void CB_Open(Widget w, void *data)
{

  char cmd[1024];
  int  rc;

  printf("%s\n", data); 
  gcmn.sig = 0;
  strncpy(cmd,data,1024);
  rc = gacmd(cmd,&gcmn,0);
  if(rc) Beep();
  else if (cmpwrd("open",data)||cmpwrd("sdfopen",data)) 
    { 
      snprintf(cmd,1023,"set dfile %d", gcmn.fnum);
      gcmn.sig = 0;
      gacmd(cmd,&gcmn,0);  /* new file becomes default */
      CB_VarSel(w, data);
    }
  
}

#ifndef HAVE_SIMPLEGETFILE
char *SimpleGetFile(char *path)
{
#ifndef GETFILE_SHORT_PROTOTYPE
  return GetFile("Simple file requestor", path, NULL, NULL);
#else
  return GetFile(path);
#endif
}
#endif

/*
 * CB_Load() - Callback function for the load button.  This just calls
 *          SimpleGetFile() to get a file name. In case of "open" or "sdfopen", 
 *          the file becomes the default and the user is asked to select a
 *          variable from this file.
 */
void CB_Load(Widget w, void *data)
{
  char *fname, *dname, *last_path, cmd[1024];
  int i,rc;

  if(strstr(data,"open"))         last_path = last_path_open;
  else if(strstr(data,"sdfopen")) last_path = last_path_sdfopen;
  else if(strstr(data,"exec"))    last_path = last_path_exec;
  else if(strstr(data,"run"))     last_path = last_path_run;
  else                            last_path = NULL;
  fname = SimpleGetFile(last_path);

  if(fname) 
    {

    /* save retrieved directory name for next time */
    dname = dirname(fname);
    if(strstr(data,"open"))           strcpy(last_path_open,dname);
    else if(strstr(data,"sdfopen"))   strcpy(last_path_sdfopen,dname);
    else if(strstr(data,"exec"))      strcpy(last_path_exec,dname);
    else if(strstr(data,"run"))       strcpy(last_path_run,dname);
    if(dname) free(dname);

    snprintf(cmd,1023,"%s %s", data, fname);
    printf("%s\n", cmd); 
    gcmn.sig = 0;
    rc = gacmd(cmd,&gcmn,0);
    if(rc) Beep();
    else if (cmpwrd("open",cmd)||cmpwrd("sdfopen",cmd)) 
      { 
        snprintf(cmd,1023,"set dfile %d", gcmn.fnum);
        gcmn.sig = 0;
        gacmd(cmd,&gcmn,0);  /* new file becomes default */
	CB_VarSel(w, data);
      }

    free(fname);
    }
  else {
    printf("%s cancelled\n", data);
    Beep();
  }

}


/*
 * CB_Cmd() - Callback funtion for a generic grads command  button.
 */
void CB_Cmd(Widget w, void *data)
{

    char cmd[1024];   /* temp space */
    int rc;
    if(data) {
      gcmn.sig = 0;
      strncpy(cmd,data,1024); /* need this or data will be overitten */
      printf("%s\n", cmd); 
      rc=gacmd(cmd,&gcmn,0);
      if(rc<0) {
        if(GetYesNo("About to exit GrADS. Really?") == TRUE ) {
	  gxend();
	  exit(0);  /* not sure if needed */
	}
      }
      if(rc) Beep();
    } else {
      Beep();
      printf("\n    ***** GUI option not implemented yet *****\n\n");
    }
}


/*
 *  CB_CmdLine() - Callback funtion for the command line button.
 */
void CB_CmdLine(Widget w, void *data)
{

  char cmd[1024];
  int rc=0;

  printf("\n     <<< Enter '.' to leave the command line interface >>>\n\n");

  while (rc>-1) {
#if READLINE == 1
    nxrdln(&cmd[0],"ga->> ");
#else
    nxtcmd(&cmd[0],"ga>>");
#endif
    if ( cmd[0] == '.' ) 
    {
       printf("     <<< Click on the RED button for the 'ga>' prompt >>>\n\n");
       return;
    }
    gcmn.sig = 0;
    rc = gacmd(cmd,&gcmn,0);
    /*    if(rc<0)
       if(GetYesNo("About to exit GrADS. Really?") == FALSE ) rc=0;
       */   /* Command liners know what they are doing ... */
  }
  gxend();
  exit(0);  /* not sure if needed */

}


/*
 * CB_VarSel() - Callback for selecting a variable.
 *
 */
void CB_VarSel(Widget W, void *data)
{
   Widget w[8];
   struct gacmn *pcm;
   struct gafile *pfi;
   struct gavar *pvar;
   int i;
   char **item_list, *var, tmp[MAXROWS];

   /* Make Variable List from GrADS data structures */
   pcm = &gcmn;
   if (pcm->pfi1==NULL) {
      Beep();
      printf("No Files Open\n");
      return;
   }
   pfi = pcm->pfid;
   pvar = pfi->pvar1;
   item_list = (char **) List(pfi->vnum+1,MAXCOLS);
   if(!item_list) return;
   for (i=0;i<pfi->vnum;i++) {
     /*      printf ("    %s %i %i %s\n",
              pvar->abbrv,pvar->levels,pvar->units[0],pvar->varnm);
	      */
      /* item_list[i] = pvar->abbrv; */
     snprintf(item_list[i],255,"%12.12s  %3i  %s", 
              pvar->abbrv, pvar->levels, pvar->varnm);
      pvar++;
      
    }
   item_list[pfi->vnum]=(char *)NULL; /* terminate list */


    /* Creates widgets, etc... */
  var_window = MakeWindow("Select a Variable", SAME_DISPLAY, EXCLUSIVE_WINDOW);

  w[0]  = MakeLabel(pfi->title);
  w[2]  = MakeScrollList(item_list, 500, 250, (void *)CB_VarList, item_list);
  w[3]  = MakeLabel("GrADS Expression: ");
  w[4]  = MakeStringEntry("", 300, (void *)CB_VarStr,         &item_list);
  expr_window = w[4];
  w[5]  = MakeButton("OK",         (void *)CB_VarOK,           NULL);
  w[6]  = MakeLabel("Click on a Variable from the List or Enter an Expression");
  w[7]  = MakeButton("Cancel",     (void *)CB_VarCancel,        &item_list);

  SetWidgetPos(w[2], PLACE_UNDER, w[0], NO_CARE,     NULL);
  SetWidgetPos(w[3], PLACE_UNDER, w[2], NO_CARE,     NULL);
  SetWidgetPos(w[4], PLACE_UNDER, w[2], PLACE_RIGHT, w[3]);
  SetWidgetPos(w[5], PLACE_UNDER, w[3], NO_CARE,     NULL);
  SetWidgetPos(w[6], PLACE_UNDER, w[3], PLACE_RIGHT, w[5]);
  SetWidgetPos(w[7], PLACE_UNDER, w[3], PLACE_RIGHT, w[6]);

  ShowDisplay();
  SetFgColor(w[0],BLUE);
  SetFgColor(w[6],RED);

  
  if(default_var[0]) var = default_var;
  else {
         strcpy(tmp,item_list[0]);
	 var = strtok(tmp," ");
  }
  SetStringEntry(expr_window, var);

  MainLoop();
  
  SetCurrentWindow(ORIGINAL_WINDOW);

  Free_List(item_list,pfi->vnum+1);

}


/*
 * CB_VarList() - Callback routine for Clicking on variable list button
 */


void CB_VarList(Widget w, char *str, int index, void *data)
{
  char *var, cmd[1024];
  var = strtok(str," ");
  strcpy(default_var,var);
  SetCurrentWindow(var_window);
  CloseWindow();
  if(!hold_on) CB_Cmd(w,"clear");
  snprintf(cmd,1023,"display %s", default_var);
  CB_Cmd(w,cmd);  /* display the variable */
}

void CB_VarStr(Widget w, char *str, int index, void *data)
{
  char cmd[1024];
  strcpy(default_var,str);
  SetCurrentWindow(var_window);
  CloseWindow();
  SyncDisplay();
  if(!hold_on) CB_Cmd(w,"clear");
  snprintf(cmd,1023,"display %s", default_var);
  CB_Cmd(w,cmd);  /* display the variable */
}

void CB_VarOK(Widget w, void *data)
{
   int index=1;
   char *str, *tmp;
   str = GetStringEntry(expr_window);
   if(str==NULL) {
     Beep();
     return;
   }
   CB_VarStr(w, str, index, data);
   if(str!=NULL) free(str);
}

void CB_VarCancel(Widget w, void *data)
{
  SetCurrentWindow(var_window);
  CloseWindow();
  Beep();
  printf("Variable selection cancelled\n");
  return;

}

/*-----------------------------------------------------------------------*/

/*
 * CB_CmdWin() - Callback for a command window.
 *
 */
void CB_CmdWin(Widget W, void *data)
{
   Widget w[8];
   int i;

   if(CmdWinON) {
     Beep();
     printf("Error: Command window already up!\n");
     return;
   }

   /* Initialize Command Win buffer */
   CmdWinList = (char **) List(NCMD+1,LCMD);
   if(!CmdWinList) return;
   for(i=0;i<NCMD;i++) CmdWinList[i][0] = '\0'; 
   CmdWinList[NCMD] = (char *) NULL;
   strncpy(CmdWinList[0], "clear",             LCMD);
   strncpy(CmdWinList[1], "reinit",            LCMD);
   strncpy(CmdWinList[2], "set gxout shaded",  LCMD);
   strncpy(CmdWinList[3], "set gxout contour", LCMD);
   strncpy(CmdWinList[4], "print",             LCMD);
   strncpy(CmdWinList[5], "quit",              LCMD);
   

    /* Creates widgets, etc... */
  Cmd_window = MakeWindow("GrADS Command Window", SAME_DISPLAY, 
                           NONEXCLUSIVE_WINDOW);

  w[0]  = MakeLabel("GrADS Command Window");
  w[1]  = MakeLabel("ga> ");
  w[2]  = MakeStringEntry("", 450, (void *)CB_CmdWinStr, NULL);
  CmdExpr_window = w[2];

  w[3]  = MakeScrollList(CmdWinList, 500, 200, (void *)CB_CmdWinList, 
                         CmdWinList);
  CmdList_window = w[3];
  w[4]  = MakeButton("OK",    (void *)CB_CmdWinOK,          NULL);
  w[5]  = MakeButton("Clear",  (void *)CB_CmdWinClear,  NULL);
  w[6]  = MakeButton("Classic Cmd Line",   (void *)CB_CmdLine,  NULL);
  w[7]  = MakeButton("Quit",  (void *)CB_Cmd,  "quit");

  SetWidgetPos(w[1], PLACE_UNDER, w[0], NO_CARE,     NULL);
  SetWidgetPos(w[2], PLACE_UNDER, w[0], PLACE_RIGHT, w[1]);
  SetWidgetPos(w[3], PLACE_UNDER, w[1], NO_CARE,     NULL);
  SetWidgetPos(w[4], PLACE_UNDER, w[3], NO_CARE,     NULL);
  SetWidgetPos(w[5], PLACE_UNDER, w[3], PLACE_RIGHT, w[4]); 
  SetWidgetPos(w[6], PLACE_UNDER, w[3], PLACE_RIGHT, w[5]);
  SetWidgetPos(w[7], PLACE_UNDER, w[3], PLACE_RIGHT, w[6]);

  ShowDisplay();

  SetFgColor(w[0],BLUE);
  SetFgColor(w[1],YELLOW);
  SetBgColor(w[1],RED);
  SetFgColor(w[4],BLUE);
  SetFgColor(w[5],BLUE);
  SetFgColor(w[6],BLUE);
  SetFgColor(w[7],BLUE);

  CmdWinON = 1;

}


void Add_CmdList( char *cmd )
{
  int i;
  char *tmp;
  tmp = CmdWinList[NCMD-1];
  for(i=NCMD-1;i>0;i--) CmdWinList[i] = CmdWinList[i-1];
  CmdWinList[0] = tmp;
  strncpy(CmdWinList[0],cmd,LCMD);
  ChangeScrollList(CmdList_window, CmdWinList);
}

void CB_CmdWinList(Widget w, char *str, int index, void *data)
{
  static time_t cur_click, last_click=0, tloc=0;
  float tdiff;
  char cmd[1024];

  
  cur_click = time(&tloc);
  tdiff = (float)(cur_click - last_click);
  strncpy(cmd,str,1024);
  if(tdiff > 1. )  /* not a double click */
   {
     last_click = cur_click;
     SetStringEntry(CmdExpr_window, cmd );
   } 
   else {
     CB_Cmd(w,cmd);
     Add_CmdList(cmd);
     SetStringEntry(CmdExpr_window, " " );
     last_click = 0;
   }

}

void CB_CmdWinStr(Widget w, char *str, int index, void *data)
{
  char cmd[1024];
  strncpy(cmd,str,1024);
  CB_Cmd(w,cmd); 
  Add_CmdList(cmd);
  SetStringEntry(CmdExpr_window, " " );
}

void CB_CmdWinClear(Widget w, void *data)
{
  char cmd[1024] = "clear";
  Add_CmdList(cmd);
  CB_Cmd(w,cmd); 
  SetStringEntry(CmdExpr_window, " " );
}

void CB_CmdWinOK(Widget w, void *data)
{
   int index=1;
   char *str;
   str = GetStringEntry(CmdExpr_window);
   if(str==NULL) {
     Beep();
     return;
   }
   CB_CmdWinStr(w, str, index, data);
   if(str!=NULL) free(str);
}

void CB_CmdWinDone(Widget w, void *data)
{
  SetCurrentWindow(Cmd_window);
  CloseWindow();
  printf("Command Window closed\n");
  CmdWinON = 0;
  Free_List(CmdWinList,NCMD+1);
  return;
}


/*-----------------------------------------------------------------------*/

/*
 * CB_Display() - Callback function for displaying the default variable.
 *
 */

void CB_Display(Widget w, void *data)
{

    struct gacmn *pcm;
    struct gafile *pfi;
    int  rc, t, tbeg, tend, tlast;
    int  v1, v2;
    char cmd[256];

    /* Any file open? */
    pcm = &gcmn;
    if (pcm->pfi1==NULL) {
       Beep();
       printf("No Files Open\n");
       return;
    }
    pfi = pcm->pfid;

    /* Default variable? */
    if(!default_var[0]) {
      Beep();
      printf("No default variable\n");
      return;
    }


    /* Just Display current variable */
    if ( strstr(data,"display") || strstr(data,"DISPLAY") ) { 
      if(!hold_on) CB_Cmd(w,"clear");
      snprintf(cmd,1023,"display %s", default_var );
      CB_Cmd(w,cmd);
      return;
    }


    /* Advance time or animate... */

    v1 = (int) t2gr(pfi->abvals[3],&(pcm->tmin));
    v2 = (int) t2gr(pfi->abvals[3],&(pcm->tmax));
    tlast = pfi->dnum[3];

    /* If dimenions are not varying ... */
    if ( v1 == v2 ) {

      if ( strstr(data,"<<") ) {
        v1--;
	tbeg = v1;
	tend = tbeg;
      }
      else if ( strstr(data,">>") ) {
        v1++;
	tbeg = v1;
	tend = tbeg;
      }
      else if ( strstr(data,"PLAY") ) {
	tbeg = v1; 
	tend = tlast;
      }
      
      /* time dim is varying, keep it */
    } else {
      tbeg = v1;
      tend = v2;
    }

    /* make sure all is within range */
    if(tbeg<1)      tbeg  = 1;
    if(tbeg>tlast)  tbeg  = tlast;
    if(tend<1)      tend  = 1;
    if(tend>tlast)  tend = tlast;

    /* Set time range and display variable */
    snprintf(cmd,1023,"set t %d %d", tbeg, tend);
    CB_Cmd(w,cmd);
    if(!hold_on) CB_Cmd(w,"clear");

    /* Display the variable: one frame or (continuous) animation */
    snprintf(cmd,1023,"display %s", default_var );
    CB_Cmd(w,cmd);

    /* If we setup a time range for PLAY, restore time to what it
       was before the animation. */
    if(v1==v2&&strstr(data,"PLAY")) {
       snprintf(cmd,1023,"set t %d %d", v1, v1);
       CB_Cmd(w,cmd);
    }

}


/*
 * CB_Toggle() - Callback funtion for a generic toggle button.
 */
void CB_Toggle(Widget w, void *data)
{

  if ( strstr(data,"hold") ) {
       hold_on = 1 - hold_on;
       if(hold_on) printf("Hold  ON: no clear screen before display\n");
       else        printf("Hold OFF: clear screen before display\n");
  }
}


/*
 * CB_FileSel() - Callback for selecting an already open file.
 *
 */
void CB_FileSel(Widget W, void *data)
{
   Widget w[8];
   struct gacmn *pcm;
   struct gafile *pfi;
   int j;
   char **item_list;

   /* Make File List from GrADS data structures */
   pcm = &gcmn;
   if (pcm->pfi1==NULL) {
      Beep();
      printf("No Files Open\n");
      return;
   } else {
      pfi = pcm->pfi1;
      item_list = (char **) List(MAXROWS,MAXCOLS);
      j = 0;
      while (pfi!=NULL && j<MAXROWS-1) {
        snprintf(item_list[j],255,"%12s  %s", pfi->name, pfi->title);
        pfi = pfi->pforw;
        j++;
      }
      item_list[j] = (char *) NULL;
    }

  /* Creates widgets, etc... */
  file_window = MakeWindow("Select a File", SAME_DISPLAY, EXCLUSIVE_WINDOW);

  w[0]  = MakeLabel("Files Open:");
  w[2]  = MakeScrollList(item_list, 500, 250, (void *)CB_FileList, item_list);
  w[6]  = MakeLabel("Click on a File");

  SetWidgetPos(w[2], PLACE_UNDER, w[0], NO_CARE,     NULL);
  SetWidgetPos(w[6], PLACE_UNDER, w[2], NO_CARE,     NULL);

  ShowDisplay();
  SetFgColor(w[0],BLUE);
  SetFgColor(w[6],RED);

  MainLoop();
  
  SetCurrentWindow(ORIGINAL_WINDOW);

  Free_List(item_list,MAXROWS);

}


/*
 * CB_FileList() - Callback routine for file list.
 */


void CB_FileList(Widget w, char *str, int index, void *data)
{
  char cmd[1024];
  SetCurrentWindow(file_window);
  CloseWindow();
  snprintf(cmd,1023,"set dfile %d", index+1);
  CB_Cmd(w,cmd);  /* set file as default */
}


/*
 * CB_CmdStr() - Callback funtion for a generic grads command with user input.
 *               *** GetString() not working **** 
*/
void CB_CmdStrOld(Widget w, void *data)
{
    char cmd[1024], *str;

    str = (char *) GetString(data, "");;
    printf("after GetSTrng\n");
    if(str!=NULL) {
      snprintf(cmd,1023,"%s %s", data, str);
      CB_Cmd(w,cmd);
      free(str);
    }

}


/*-----------------------------------------------------------------------*/

void CB_CmdTextStr(Widget w, char *str, void *data)
{
  char cmd[1024];
  strcpy(cmd,str);
  CB_Cmd(w,cmd); 
  CloseWindow();
}

void CB_TextOK(Widget w, void *data)
{
  char *str;
   str = GetStringEntry(CmdStr_window);
   if(str==NULL) {
     return;
   }
   CB_CmdTextStr(w,str,data);
}

void CB_TextCancel(Widget w, void *data)
{
  printf("Command <%s> cancelled\n",data);
  CloseWindow();
  return;

}

/*
 * CB_CmdText() - Callback for a command string window.
 *                Provide our own because GetString() is not
 *                working reliably.
 */
void CB_CmdStr(Widget W, void *data)
{
  Widget w[3], window;

  /* Creates widgets, etc... */
  window = MakeWindow(data, SAME_DISPLAY, NONEXCLUSIVE_WINDOW);

  w[0]  = MakeStringEntry(data, 200, (void *)CB_CmdTextStr, NULL);
  w[1]  = MakeButton("OK",         (void *)CB_TextOK,       NULL);
  w[2]  = MakeButton("Cancel",     (void *)CB_TextCancel,   data);

  CmdStr_window = w[0];

  SetWidgetPos(w[1], PLACE_UNDER, w[0], NO_CARE, NULL);
  SetWidgetPos(w[2], PLACE_UNDER, w[0], PLACE_RIGHT, w[1]);

  ShowDisplay();

}

/*-----------------------------------------------------------------------*/

/*
 * CB_Exit() - Exits GUI, well... fake it.
 *             NOTE: not working properly yet.
 */

void CB_Exit(Widget w, void *data)
{

/*  if(GetYesNo("About to exit Graphical User Interface. Really?") == FALSE) 
     return;
*/

  /* Get rid of GUI window */
  CloseWindow();
  SetCurrentWindow(ORIGINAL_WINDOW);
 
}

/*
 * CB_CloseWindow - what it says.
 */

void CB_CloseWindow(Widget w, void *data)
{
  CloseWindow();
  return;

}


/*----------------------- Text Viewer/Edit Widget --------------------*/

/* The code below is based on xmore.c by Dominic Giampolo */

void Editor_Quit(Widget foo, void *arg)
{
  WinInfo *wi=(WinInfo *)arg;

  *(wi->num_windows) = *(wi->num_windows) - 1;

  SetCurrentWindow(XtParent(XtParent(foo)));
  CloseWindow();
  
  /* if (*(wi->num_windows) == 0) exit(0); */

}


/*
 * Open a new file in the same window.
 */
void Editor_File(Widget foo, void *arg)
{
  WinInfo *wi=(WinInfo *)arg;
  char *fname;

  fname = SimpleGetFile(wi->cur_path);
  if (fname)
   {
     SetTextWidgetText(wi->text_widget, fname, TRUE);
     SetLabel(wi->label_widget, fname);

     if (wi->cur_path)
       free(wi->cur_path);
     
     wi->cur_path = dirname(fname);
   }
}



#define MAXLABEL  80

void make_text_viewer(char *fname, WinInfo *arg)
{
  Widget w[10];
  static char dummy_label[MAXLABEL];
  int i, width;
  XFont xf;

  for(i=0; i < MAXLABEL-1; i++)
    dummy_label[i] = ' ';
  dummy_label[i] = '\0';

  w[0] = MakeLabel(dummy_label);

/*  xf = GetWidgetFont(w[0]);
  if (xf != NULL)
    width = TextWidth(xf, dummy_label);
  else */
    width = 600;

  w[1] = MakeTextWidget(fname, TRUE, FALSE, width, 400);
  w[2] = MakeButton("Open", Editor_File, arg);
  w[3] = MakeButton("Quit", Editor_Quit, arg);
  
  SetWidgetPos(w[1], PLACE_UNDER, w[0], NO_CARE, NULL);
  SetWidgetPos(w[2], PLACE_UNDER, w[1], NO_CARE, NULL);
  SetWidgetPos(w[3], PLACE_UNDER, w[1], PLACE_RIGHT, w[2]);
  
  AttachEdge(w[0], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[0], BOTTOM_EDGE, ATTACH_TOP);
  
  AttachEdge(w[1], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[1], RIGHT_EDGE,  ATTACH_RIGHT);
  AttachEdge(w[1], TOP_EDGE,    ATTACH_TOP);
  AttachEdge(w[1], BOTTOM_EDGE, ATTACH_BOTTOM);
  
  AttachEdge(w[2], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[2], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[2], TOP_EDGE,    ATTACH_BOTTOM);
  AttachEdge(w[2], BOTTOM_EDGE, ATTACH_BOTTOM);

  AttachEdge(w[3], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[3], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[3], TOP_EDGE,    ATTACH_BOTTOM);
  AttachEdge(w[3], BOTTOM_EDGE, ATTACH_BOTTOM);

  arg->text_widget  = w[1];
  arg->label_widget = w[0];

  ShowDisplay();

  SetLabel(w[0], fname);   /* set the real filename */
}


void make_text_editor(char *fname, WinInfo *arg)
{
  Widget w[10];
  static char dummy_label[MAXLABEL];
  int i, width;
  XFont xf;

  for(i=0; i < MAXLABEL-1; i++)
    dummy_label[i] = ' ';
  dummy_label[i] = '\0';

  w[0] = MakeLabel(dummy_label);

/*  xf = GetWidgetFont(w[0]);
  if (xf != NULL)
    width = TextWidth(xf, dummy_label);
  else */
    width = 600;

  w[1] = MakeTextWidget(fname, TRUE, TRUE, width, 400);
  w[2] = MakeButton("Open", Editor_File, arg);
  w[3] = MakeButton("Save", NULL, arg);
  w[4] = MakeButton("Save as", NULL, arg);
  w[5] = MakeButton("Exec", NULL, arg);
  w[6] = MakeButton("Run", NULL, arg);
  w[7] = MakeButton("GUI", NULL, arg);
  w[8] = MakeButton("Quit", Editor_Quit, arg);
  
  SetWidgetPos(w[1], PLACE_UNDER, w[0], NO_CARE, NULL);
  SetWidgetPos(w[2], PLACE_UNDER, w[1], NO_CARE, NULL);
  SetWidgetPos(w[3], PLACE_UNDER, w[1], PLACE_RIGHT, w[2]);
  SetWidgetPos(w[4], PLACE_UNDER, w[1], PLACE_RIGHT, w[3]);
  SetWidgetPos(w[5], PLACE_UNDER, w[1], PLACE_RIGHT, w[4]);
  SetWidgetPos(w[6], PLACE_UNDER, w[1], PLACE_RIGHT, w[5]);
  SetWidgetPos(w[7], PLACE_UNDER, w[1], PLACE_RIGHT, w[6]);
  SetWidgetPos(w[8], PLACE_UNDER, w[1], PLACE_RIGHT, w[7]);
  
  AttachEdge(w[0], RIGHT_EDGE,  ATTACH_LEFT);
  AttachEdge(w[0], BOTTOM_EDGE, ATTACH_TOP);
  
  AttachEdge(w[1], LEFT_EDGE,   ATTACH_LEFT);
  AttachEdge(w[1], RIGHT_EDGE,  ATTACH_RIGHT);
  AttachEdge(w[1], TOP_EDGE,    ATTACH_TOP);
  AttachEdge(w[1], BOTTOM_EDGE, ATTACH_BOTTOM);
  
  for(i=2;i<=8;i++) {
     AttachEdge(w[i], LEFT_EDGE,   ATTACH_LEFT);
     AttachEdge(w[i], RIGHT_EDGE,  ATTACH_LEFT);
     AttachEdge(w[i], TOP_EDGE,    ATTACH_BOTTOM);
     AttachEdge(w[i], BOTTOM_EDGE, ATTACH_BOTTOM);
  }

  arg->text_widget  = w[1];
  arg->label_widget = w[0];

  ShowDisplay();

  SetLabel(w[0], fname);   /* set the real filename */
}


/*
 * CB_Browse() - Text viewer callback. Ideal for help files.
 *
 */

void CB_Browse(Widget w, void *data)
{

  char *fname;
  int  num_windows=1;
  Widget this;
  WinInfo *wi;


  fname = (char *) data;
  if ( !strcmp(fname,"NULL") ) fname = SimpleGetFile(NULL);
  if ( !fname ) return;

  this = MakeWindow("GrADS Text Viewer", SAME_DISPLAY, NONEXCLUSIVE_WINDOW);
  if ( w == NULL ) return;

  wi = (WinInfo *)calloc(sizeof(WinInfo), 1);
  if (wi == NULL) return;
     
  wi->num_windows = &num_windows;
  wi->cur_path = dirname(fname);
     
  make_text_viewer(fname, wi);

}

void CB_Edit(Widget w, void *data)
{

  char *fname;
  int  num_windows=1;
  Widget this;
  WinInfo *wi;

  fname = (char *)data;
  if ( !strcmp(fname,"NULL") ) fname = SimpleGetFile(NULL);
  if ( !fname ) return;

  this = MakeWindow("GrADS Text Editor", SAME_DISPLAY, NONEXCLUSIVE_WINDOW);
  if ( w == NULL ) return;

  wi = (WinInfo *)calloc(sizeof(WinInfo), 1);
  if (wi == NULL) return;
     
  wi->num_windows = &num_windows;
  wi->cur_path = dirname(fname);
     
  make_text_editor(fname, wi);

}


