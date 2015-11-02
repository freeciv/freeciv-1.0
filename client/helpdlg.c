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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/SimpleMenu.h> 
#include <X11/Xaw/Command.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/AsciiText.h>

#include "xstuff.h"

#include "helptab.c"

extern Widget toplevel, main_form;

Widget help_dialog_shell;
Widget help_form;
Widget help_viewport;
Widget help_list;
Widget help_text;
Widget help_close_command;

void update_help_dialog(Widget help_list);

void help_close_command_callback(Widget w, XtPointer client_data, 
				 XtPointer call_data);
void help_list_callback(Widget w, XtPointer client_data, 
			         XtPointer call_data);

void create_help_dialog(void);
void select_help_item(int item);


/****************************************************************
 popup the dialog 10% inside the main-window 
*****************************************************************/
void popup_help_dialog(int item)
{
  Position x, y;
  Dimension width, height;

  if(!help_dialog_shell)
    create_help_dialog();

  XtVaGetValues(toplevel, XtNwidth, &width, XtNheight, &height, NULL);

  XtTranslateCoords(toplevel, (Position) width/10, (Position) height/10,
		    &x, &y);
  XtVaSetValues(help_dialog_shell, XtNx, x, XtNy, y, NULL);

  XtPopup(help_dialog_shell, XtGrabNone);

  select_help_item(item);
}


/**************************************************************************
...
**************************************************************************/
void create_help_dialog(void)
{
  help_dialog_shell = XtCreatePopupShell("helpdialog", 
					 topLevelShellWidgetClass,
					 toplevel, NULL, 0);

  help_form = XtVaCreateManagedWidget("helpform", 
				      formWidgetClass, 
				      help_dialog_shell, NULL);
  

  help_viewport = XtVaCreateManagedWidget("helpviewport", 
					  viewportWidgetClass, 
					  help_form, 
					  NULL);
  
  help_list = XtVaCreateManagedWidget("helplist", 
				      listWidgetClass, 
				      help_viewport, 
				      XtNlist, 
				      (XtArgVal)help_indexlist,
				      NULL);

  help_text = XtVaCreateManagedWidget("helptext", 
				       asciiTextWidgetClass, 
				       help_form,
				       XtNeditType, XawtextRead,
				       XtNscrollVertical, XawtextScrollAlways, 
				       NULL);

  
  help_close_command = XtVaCreateManagedWidget("helpclosecommand", 
					       commandWidgetClass,
					       help_form,
					       NULL);
  

  XtAddCallback(help_close_command, XtNcallback, 
		help_close_command_callback, NULL);

  XtAddCallback(help_list, XtNcallback, 
		help_list_callback, NULL);

  XtRealizeWidget(help_dialog_shell);
}



/**************************************************************************
...
**************************************************************************/
void help_list_callback(Widget w, XtPointer client_data, 
			XtPointer call_data)
{
  XawListReturnStruct *ret;
  
  ret=XawListShowCurrent(help_list);

  if(ret->list_index!=XAW_LIST_NONE)
    select_help_item(ret->list_index);
}


/**************************************************************************
...
**************************************************************************/
void help_close_command_callback(Widget w, XtPointer client_data, 
				 XtPointer call_data)
{
  XtDestroyWidget(help_dialog_shell);
  help_dialog_shell=0;
}

/**************************************************************************
...
**************************************************************************/
void select_help_item(int item)
{
  XtVaSetValues(help_text, XtNstring, help_items[item], NULL);
  XawListHighlight(help_list, item); 
}
