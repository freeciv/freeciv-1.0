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
#include <ctype.h>
#include <stdarg.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Toggle.h>     

#include "game.h"
#include "player.h"
#include "mapview.h"
#include "optiondlg.h"
#include "shared.h"
#include "packets.h"
#include "xstuff.h"

extern Widget toplevel, main_form;

extern struct connection aconnection;
extern Display	*display;

extern int use_solid_color_behind_units;
extern int sound_bell_at_new_turn;

/******************************************************************/
Widget option_dialog_shell;
Widget option_bg_toggle;
Widget option_bell_toggle;
/******************************************************************/


void create_option_dialog(void);

void option_ok_command_callback(Widget w, XtPointer client_data, 
			        XtPointer call_data);

/****************************************************************
... 
*****************************************************************/
void popup_option_dialog(void)
{
  create_option_dialog();
  XtVaSetValues(option_bg_toggle, XtNstate, use_solid_color_behind_units, NULL);
  XtVaSetValues(option_bell_toggle, XtNstate, sound_bell_at_new_turn, NULL);
  
  xaw_set_relative_position(toplevel, option_dialog_shell, 25, 25);
  XtPopup(option_dialog_shell, XtGrabNone);
  XtSetSensitive(main_form, FALSE);
}




/****************************************************************
...
*****************************************************************/
void create_option_dialog(void)
{
  Widget option_form, option_label, option_bg_label, option_bell_label;
  Widget option_ok_command;
  
  option_dialog_shell = XtCreatePopupShell("optionpopup", 
					  transientShellWidgetClass,
					  toplevel, NULL, 0);

  option_form = XtVaCreateManagedWidget("optionform", 
				        formWidgetClass, 
				        option_dialog_shell, NULL);   

  option_label = XtVaCreateManagedWidget("optionlabel", 
					 labelWidgetClass, 
					 option_form, NULL);   
  
  option_bg_label = XtVaCreateManagedWidget("optionbglabel", 
					    labelWidgetClass, 
					    option_form, NULL);
  
  option_bg_toggle = XtVaCreateManagedWidget("optionbgtoggle", 
					     toggleWidgetClass, 
					     option_form,
					     NULL);

  option_bell_label = XtVaCreateManagedWidget("optionbelllabel", 
					      labelWidgetClass, 
					      option_form, NULL);

  
  option_bell_toggle = XtVaCreateManagedWidget("optionbelltoggle", 
					       toggleWidgetClass, 
					       option_form,
					       NULL);
    
  
  option_ok_command = XtVaCreateManagedWidget("optionokcommand", 
					      commandWidgetClass,
					      option_form,
					      NULL);
  
  XtAddCallback(option_ok_command, XtNcallback, 
		option_ok_command_callback, NULL);

  XtRealizeWidget(option_dialog_shell);

  xaw_horiz_center(option_label);
}





/**************************************************************************
...
**************************************************************************/
void option_ok_command_callback(Widget w, XtPointer client_data, 
			       XtPointer call_data)
{
  Boolean b;
  
  XtSetSensitive(main_form, TRUE);
  XtDestroyWidget(option_dialog_shell);

  XtVaGetValues(option_bg_toggle, XtNstate, &b, NULL);
  use_solid_color_behind_units=b;
  XtVaGetValues(option_bell_toggle, XtNstate, &b, NULL);
  sound_bell_at_new_turn=b;
}
