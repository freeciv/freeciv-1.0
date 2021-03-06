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
#include <X11/Xaw/List.h>

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
extern struct advance advances[];

/******************************************************************/
Widget intel_dialog_shell;
/******************************************************************/


void intel_create_dialog(struct player *p);

void intel_close_command_callback(Widget w, XtPointer client_data, 
				  XtPointer call_data);

/****************************************************************
... 
*****************************************************************/
void popup_intel_dialog(struct player *p)
{
  intel_create_dialog(p);
  xaw_set_relative_position(toplevel, intel_dialog_shell, 25, 25);
  XtPopup(intel_dialog_shell, XtGrabNone);
  XtSetSensitive(main_form, FALSE);
}




/****************************************************************
...
*****************************************************************/
void intel_create_dialog(struct player *p)
{
  Widget form, title, close;
  char buf[64];

  static char *tech_list_names_ptrs[A_LAST+1];
  static char tech_list_names[A_LAST+1][200];
  int i, j;
  
  
  intel_dialog_shell = XtCreatePopupShell("intelpopup", 
					  transientShellWidgetClass,
					  toplevel, NULL, 0);

  form = XtVaCreateManagedWidget("intelform", 
				 formWidgetClass, 
				 intel_dialog_shell, NULL);
  
  sprintf(buf, "Intelligence Information for the %s Empire", 
	  get_race_name(p->race));
  
  title=XtVaCreateManagedWidget("inteltitlelabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);
  
  sprintf(buf, "Ruler: %s %s", 
	  get_ruler_title(p->goverment), p->name);
  XtVaCreateManagedWidget("intelnamelabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);   
  
  sprintf(buf, "Goverment: %s", get_goverment_name(p->goverment));
  XtVaCreateManagedWidget("intelgovlabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);   
  
  sprintf(buf, "Gold: %d", p->economic.gold);
  XtVaCreateManagedWidget("intelgoldlabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);   

  sprintf(buf, "Tax: %d%%", p->economic.tax);
  XtVaCreateManagedWidget("inteltaxlabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);   

  sprintf(buf, "Science: %d%%", p->economic.science);
  XtVaCreateManagedWidget("intelscilabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);   

  sprintf(buf, "Luxury: %d%%", p->economic.luxury);
  XtVaCreateManagedWidget("intelluxlabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);   

  sprintf(buf, "Researching: %s(%d/%d)", 
  	  advances[p->research.researching].name,
	  p->research.researched, 
	  research_time(p));

  XtVaCreateManagedWidget("intelreslabel", 
			  labelWidgetClass, 
			  form, 
			  XtNlabel, buf,
			  NULL);   

  

  for(i=1, j=0; i<A_LAST; i++)
    if(get_invention(p, i)==TECH_KNOWN) {
      if(get_invention(game.player_ptr, i)==TECH_KNOWN)
	strcpy(tech_list_names[j], advances[i].name);
      else
	sprintf(tech_list_names[j], "%s*", advances[i].name);
      tech_list_names_ptrs[j]=tech_list_names[j];
      j++;
    }
  tech_list_names_ptrs[j]=0;
  

  
  XtVaCreateManagedWidget("inteltechlist", 
			  listWidgetClass,
			  form,
			  XtNlist, tech_list_names_ptrs,
			  NULL);
  
  close = XtVaCreateManagedWidget("intelclosecommand", 
				  commandWidgetClass,
				  form, NULL);
  
  XtAddCallback(close, XtNcallback, intel_close_command_callback, NULL);
  XtRealizeWidget(intel_dialog_shell);

  xaw_horiz_center(title);
}





/**************************************************************************
...
**************************************************************************/
void intel_close_command_callback(Widget w, XtPointer client_data, 
				  XtPointer call_data)
{ 
  XtSetSensitive(main_form, TRUE);
  XtDestroyWidget(intel_dialog_shell);
}
