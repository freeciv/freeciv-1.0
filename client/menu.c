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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include "menu.h"
#include "dialogs.h"
#include "plrdlg.h"
#include "mapctrl.h"
#include "repodlgs.h"
#include "ratesdlg.h"
#include "optiondlg.h"
#include "finddlg.h"
#include "helpdlg.h"
#include "civclient.h"

struct Menu *game_menu, *orders_menu, *reports_menu, *help_menu;

char *game_menu_entry_names[] = {
  "gamemenu",
  "Find City",
  "Options",
  "Rates",
  "Revolution",
  "Players",
  "Quit",
  NULL
};


char *orders_menu_entry_names[] = {
  "ordersmenu",
  "Build City        b",
  "Build Irrigation  i",
  "Build Fortress    F",
  "Build Mine        m",
  "Build Road        r",
  "Clean Polution    p",
  "Make homecity     h",
  "Fortify           f",
  "Sentry            s", 
  "Wait              w",
  "Go to             g",
  "Disband Unit      D",
  "Pillage           P",
  "Done            spc",
  NULL
};

char *reports_menu_entry_names[] = {
  "reportsmenu",
  "City Report",
  "Science Report",
  "Demographic",
  "Top 5 Cities",
  "Wonders of the World",
  NULL
};

char *help_menu_entry_names[] = {
  "helpmenu",
  "Help Controls",
  "Help Playing",
  "Help Improvements",
  "Help Units",
  "Help Technology",
  "Help Wonders",
  "Help Newbies",
  "Copying",
  "About",
  NULL
};


/****************************************************************
...
*****************************************************************/
void game_menu_callback(Widget w, XtPointer client_data, XtPointer garbage)
{
  int pane_num = (int)client_data;

  switch(pane_num) {
  case 1:
    popup_find_dialog();
    break;
   case 2:
    popup_option_dialog();
    break;
  case 3:
    popup_rates_dialog();
    break;
  case 4:
    popup_revolution_dialog();
    break;
   case 5:
    popup_players_dialog();
    break;
  case 6:
    exit(0);
    break;
  }
}

/****************************************************************
...
*****************************************************************/
void orders_menu_callback(Widget w, XtPointer client_data, XtPointer garbage)
{
  int pane_num = (int)client_data;

  switch(pane_num) {
   case 1:
    if(get_unit_in_focus())
      request_unit_build_city(get_unit_in_focus());
    break;
   case 2:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_IRRIGATE);
    break;
   case 3:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_FORTRESS);
    break;
   case 4:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_MINE);
    break;
   case 5:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_ROAD);
    break;
   case 6:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_POLUTION);
    break;
   case 7:
    if(get_unit_in_focus())
      request_unit_change_homecity(get_unit_in_focus());
    break;
   case 8:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_FORTIFY);
    break;
   case 9:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_SENTRY);
    break;
   case 10:
    if(get_unit_in_focus())
      request_unit_wait(get_unit_in_focus());
    break;
   case 11:
    if(get_unit_in_focus())
      request_unit_goto();
    break;
   case 12:
    if(get_unit_in_focus())
      request_unit_disband(get_unit_in_focus());
    break;
   case 13:
    if(get_unit_in_focus())
      request_new_unit_activity(get_unit_in_focus(), ACTIVITY_PILLAGE);
     break;
   case 14:
    if(get_unit_in_focus())
      request_unit_move_done();
    break;
  }

}


/****************************************************************
...
*****************************************************************/
void reports_menu_callback(Widget w, XtPointer client_data, XtPointer garbage)
{
  int pane_num = (int)client_data;

  switch(pane_num) {
   case 1:
    popup_city_report_dialog(0);
    break;
   case 2:
    popup_science_dialog(0);
    break;
   case 3:
    send_report_request(REPORT_DEMOGRAPHIC);
    break;
   case 4:
    send_report_request(REPORT_TOP_5_CITIES);
    break;
   case 5:
    send_report_request(REPORT_WONDERS_OF_THE_WORLD);
    break;
  }
}


/****************************************************************
...
*****************************************************************/
void help_menu_callback(Widget w, XtPointer client_data, XtPointer garbage)
{
  int pane_num = (int)client_data;

  switch(pane_num) {
  case 1:
    popup_help_dialog(HELP_CONTROLS_ITEM);
    break;
  case 2:
    popup_help_dialog(HELP_PLAYING_ITEM);
    break;
  case 3:
    popup_help_dialog(HELP_IMPROVEMENTS_ITEM);
    break;
  case 4:
    popup_help_dialog(HELP_UNITS_ITEM);
    break;
  case 5:
    popup_help_dialog(HELP_TECHS_ITEM);
    break;
  case 6:
    popup_help_dialog(HELP_WONDERS_ITEM);
    break;
  case 7:
    popup_help_dialog(HELP_NEWBIE_ITEM);
    break;
  case 8:
    popup_help_dialog(HELP_COPYING_ITEM);
    break;
  case 9:
    popup_help_dialog(HELP_ABOUT_ITEM);
    break;
  }

}




/****************************************************************
...
*****************************************************************/
void setup_menues(Widget parent_form)
{
  game_menu=ctor_menu(game_menu_entry_names, game_menu_callback, 
		      NULL, parent_form);
  orders_menu=ctor_menu(orders_menu_entry_names, orders_menu_callback, 
			game_menu, parent_form);
  reports_menu=ctor_menu(reports_menu_entry_names, reports_menu_callback,
			 orders_menu, parent_form);
  help_menu=ctor_menu(help_menu_entry_names, help_menu_callback, 
		      reports_menu, parent_form);
}




/****************************************************************
names[0] contains the text of the menu button
names[n] contains the text of the n'th item
names[last] contains NULL
menu_to_the_left is the Widget which this menu is placed right of
*****************************************************************/
struct Menu *ctor_menu(char *names[], 
		       void (*menucallback)(Widget, XtPointer, XtPointer),
		       struct Menu *menu_to_the_left, Widget parent)
{
  int i;
  struct Menu *mymenu;
  Widget entry;
  Arg menu_arglist[3];

  mymenu=(struct Menu *)malloc(sizeof(struct Menu));

  XtSetArg(menu_arglist[0], XtNright, (XtArgVal)XawChainLeft);
  XtSetArg(menu_arglist[1], XtNleft, (XtArgVal)XawChainLeft);

  if(menu_to_the_left) {
    XtSetArg(menu_arglist[2], 
	     XtNfromHoriz, (XtArgVal)menu_to_the_left->button);

    mymenu->button=XtCreateManagedWidget(names[0],
					 menuButtonWidgetClass, parent,
					 menu_arglist, 
					 (Cardinal) 3);
  }
  else
    mymenu->button=XtCreateManagedWidget(names[0],
					 menuButtonWidgetClass, parent,
					 menu_arglist, (Cardinal) 2);

  mymenu->shell=XtCreatePopupShell("menu", simpleMenuWidgetClass, 
				       mymenu->button, NULL, 0);

  i=1;
  while(names[i]) {
    entry = XtCreateManagedWidget(names[i], smeBSBObjectClass, 
				  mymenu->shell, NULL, 0);
    XtAddCallback(entry, XtNcallback, menucallback, (XtPointer) i);
    i++;
  }

  return mymenu;
}
/****************************************************************
free the menu. guess the widgets should be killed here too 
*****************************************************************/
void dtor_menu(struct Menu *mymenu)
{
  free(mymenu);
}



