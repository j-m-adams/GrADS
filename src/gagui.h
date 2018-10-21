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

#include "libsx.h"

int init_display(int argc, char **argv, void *data);
int Custom_GUI( char *fname );
int gagui_main(int argc, char **argv);

/* callback protos */
void CB_Exit       (Widget w, void *data);
void CB_CloseWindow(Widget w, void *data); 
void CB_Open       (Widget w, void *data);
void CB_Load       (Widget w, void *data);
void CB_Cmd        (Widget w, void *data);
void CB_CmdStr     (Widget w, void *data);
void CB_CmdLine    (Widget w, void *data);
void CB_Display    (Widget w, void *data);
void CB_Toggle     (Widget w, void *data);

void CB_VarSel     (Widget w, void *data);
void CB_VarOK      (Widget w, void *data);
void CB_VarCancel  (Widget w, void *data);
void CB_VarList    (Widget w, char *str, int index, void *data);
void CB_VarStr     (Widget w, char *str, int index, void *data);

void CB_CmdWin     (Widget w, void *data);
void CB_CmdWinOK   (Widget w, void *data);
void CB_CmdWinClear (Widget w, void *data);
void CB_CmdWinDone (Widget w, void *data);
void CB_CmdWinList (Widget w, char *str, int index, void *data);
void CB_CmdWinStr  (Widget w, char *str, int index, void *data);

void CB_FileSel    (Widget w, void *data);
void CB_Browse     (Widget w, void *data);
void CB_Edit       (Widget w, void *data);
void CB_FileList   (Widget w, char *str, int index, void *data);

/* kk --- 020619 added List and Free_List --- kk */
char **List(int rows,int cols);
void Free_List(char **list, int rows);
/* kk --- 020619 added List and Free_List --- kk */
typedef struct wininfo
{
  Widget window, text_widget, label_widget;
  int *num_windows;
  char *cur_path;
}WinInfo;
