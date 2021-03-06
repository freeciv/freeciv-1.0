/********************************************************************** 
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Viewport.h>

#include "game.h"
#include "player.h"
#include "mapview.h"

extern Widget toplevel, main_form;
extern struct player_race races[];

Widget find_dialog_shell;
Widget find_form;
Widget find_label;
Widget find_viewport;
Widget find_list;
Widget find_center_command;
Widget find_cancel_command;

void update_find_dialog(Widget find_list);

void find_center_command_callback(Widget w, XtPointer client_data, 
				  XtPointer call_data);
void find_cancel_command_callback(Widget w, XtPointer client_data, 
				  XtPointer call_data);

char *dummy_city_list[]={ 
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  0
};

int ncities_total;
char **city_name_ptrs;

char change_list_names[B_LAST+1+U_LAST+1][200];

/****************************************************************
popup the dialog 10% inside the main-window 
*****************************************************************/
void popup_find_dialog(void)
{
  Position x, y;
  Dimension width, height;

  XtSetSensitive(main_form, FALSE);
  
  find_dialog_shell = XtCreatePopupShell("finddialog", 
					 transientShellWidgetClass,
					 toplevel, NULL, 0);

  find_form = XtVaCreateManagedWidget("findform", 
				      formWidgetClass, 
				      find_dialog_shell, NULL);

  
  find_label = XtVaCreateManagedWidget("findlabel", 
				       labelWidgetClass, 
				       find_form,
				       NULL);

  find_viewport = XtVaCreateManagedWidget("findviewport", 
				      viewportWidgetClass, 
				      find_form, 
				      NULL);
  
  
  find_list = XtVaCreateManagedWidget("findlist", 
				      listWidgetClass, 
				      find_viewport, 
				      XtNlist, 
				      (XtArgVal)dummy_city_list,
				      NULL);
  
  find_center_command = XtVaCreateManagedWidget("findcentercommand", 
						commandWidgetClass,
						find_form,
						NULL);

  find_cancel_command = XtVaCreateManagedWidget("findcancelcommand", 
						commandWidgetClass,
						find_form,
						NULL);

  XtAddCallback(find_center_command, XtNcallback, 
		find_center_command_callback, NULL);
  XtAddCallback(find_cancel_command, XtNcallback, 
		find_cancel_command_callback, NULL);
  

  XtRealizeWidget(find_dialog_shell);

  update_find_dialog(find_list);

  XtVaGetValues(toplevel, XtNwidth, &width, XtNheight, &height, NULL);

  XtTranslateCoords(toplevel, (Position) width/10, (Position) height/10,
		    &x, &y);
  XtVaSetValues(find_dialog_shell, XtNx, x, XtNy, y, NULL);

  XtPopup(find_dialog_shell, XtGrabNone);

  /* force refresh of viewport so the scrollbar is added.
   * Buggy sun athena requires this */
  XtVaSetValues(find_viewport, XtNforceBars, True, NULL);
}



/**************************************************************************
...
**************************************************************************/
void update_find_dialog(Widget find_list)
{
  int i, j;

  for(i=0, ncities_total=0; i<game.nplayers; i++)
    ncities_total+=city_list_size(&game.players[i].cities);

  city_name_ptrs=(char **)malloc(ncities_total*sizeof(char*));
  
  for(i=0, j=0; i<game.nplayers; i++) {
    struct genlist_iterator myiter;
    genlist_iterator_init(&myiter, &game.players[i].cities.list, 0);
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
      struct city *pcity=(struct city *)ITERATOR_PTR(myiter);
      *(city_name_ptrs+j++)=mystrdup(pcity->name);
    }
  }
  
  if(ncities_total)
    XawListChange(find_list, city_name_ptrs, ncities_total, 0, True);
}

/**************************************************************************
...
**************************************************************************/
void popdown_find_dialog(void)
{
  int i;
  
  for(i=0; i<ncities_total; i++)
    free(*(city_name_ptrs+i));
  
  XtPopdown(find_dialog_shell);
  free(city_name_ptrs);
  XtSetSensitive(main_form, TRUE);
}

/**************************************************************************
...
**************************************************************************/
void find_center_command_callback(Widget w, XtPointer client_data, 
				  XtPointer call_data)
{
  struct city *pcity;
  XawListReturnStruct *ret;
  
  ret=XawListShowCurrent(find_list);

  if(ret->list_index!=XAW_LIST_NONE)
    if((pcity=game_find_city_by_name(ret->string)))
      center_tile_mapcanvas(pcity->x, pcity->y);
  
  popdown_find_dialog();
}

/**************************************************************************
...
**************************************************************************/
void find_cancel_command_callback(Widget w, XtPointer client_data, 
				  XtPointer call_data)
{
  popdown_find_dialog();
}
