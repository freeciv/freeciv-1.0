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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/List.h>

#include "game.h"
#include "player.h"
#include "plrdlg.h"
#include "diplodlg.h"
#include "inteldlg.h"
#include "packets.h"
#include "clinet.h"
#include "chatline.h"
#include "xstuff.h"

extern Widget toplevel, main_form;
extern struct player_race races[];

Widget players_dialog_shell;
Widget players_form;
Widget players_label;
Widget players_list;
Widget players_close_command;
Widget players_int_command;
Widget players_meet_command;

void create_players_dialog(void);
void players_button_callback(Widget w, XtPointer client_data, 
			      XtPointer call_data);
void players_meet_callback(Widget w, XtPointer client_data, 
			   XtPointer call_data);
void players_intel_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);
void players_list_callback(Widget w, XtPointer client_data, 
			   XtPointer call_data);


/****************************************************************
popup the dialog 10% inside the main-window 
*****************************************************************/
void popup_players_dialog(void)
{
  if(!players_dialog_shell)
    create_players_dialog();

  xaw_set_relative_position(toplevel, players_dialog_shell, 25, 25);
  XtPopup(players_dialog_shell, XtGrabNone);
}


/****************************************************************
...
*****************************************************************/
void create_players_dialog(void)
{
  players_dialog_shell = XtCreatePopupShell("playerspopup", 
					  topLevelShellWidgetClass,
					  toplevel, NULL, 0);

  players_form = XtVaCreateManagedWidget("playersform", 
				       formWidgetClass, 
				       players_dialog_shell, NULL);

  players_label=XtVaCreateManagedWidget("playerslabel", 
					labelWidgetClass, 
					players_form, NULL);   

   
  players_list = XtVaCreateManagedWidget("playerslist", 
					 listWidgetClass, 
					 players_form, 
					 NULL);

  players_close_command = XtVaCreateManagedWidget("playersclosecommand", 
						  commandWidgetClass,
						  players_form,
						  NULL);

  players_int_command = XtVaCreateManagedWidget("playersintcommand", 
						commandWidgetClass,
						players_form,
                                                XtNsensitive, False,
						NULL);

  players_meet_command = XtVaCreateManagedWidget("playersmeetcommand", 
						 commandWidgetClass,
						 players_form,
						 XtNsensitive, False,
						 NULL);

  XtAddCallback(players_list, XtNcallback, players_list_callback, 
		NULL);
  
  XtAddCallback(players_close_command, XtNcallback, players_button_callback, 
		NULL);

  XtAddCallback(players_meet_command, XtNcallback, players_meet_callback, 
		NULL);
  XtAddCallback(players_int_command, XtNcallback, players_intel_callback, 
		NULL);
  
  update_players_dialog();

  XtRealizeWidget(players_dialog_shell);
}


/**************************************************************************
...
**************************************************************************/
void update_players_dialog(void)
{
   if(players_dialog_shell) {
    int i;
    Dimension width;
    static char *namelist_ptrs[MAX_PLAYERS];
    static char namelist_text[MAX_PLAYERS][256];
    
    for(i=0; i<game.nplayers; i++) {
      char idlebuf[32], statebuf[32];
      
      if(game.players[i].nturns_idle>3)
	sprintf(idlebuf, "(idle %d turns)", game.players[i].nturns_idle-1);
      else
	idlebuf[0]='\0';
      
      if(game.players[i].is_alive) {
	if(game.players[i].is_connected) {
	  if(game.players[i].turn_done)
	    strcpy(statebuf, "done");
	  else
	    strcpy(statebuf, "moving");
	}
	else
	  statebuf[0]='\0';
      }
      else
	strcpy(statebuf, "R.I.P");
       
      sprintf(namelist_text[i], "%-16s %-12s    %c     %-6s   %-15s%s", 
	      game.players[i].name,
	      races[game.players[i].race].name, 
	      player_has_embassy(game.player_ptr, &game.players[i]) ? 'X':' ',
	      statebuf,
	      game.players[i].addr, 
	      idlebuf);
	 
      namelist_ptrs[i]=namelist_text[i];
    }
    
    XawListChange(players_list, namelist_ptrs, game.nplayers, 0, True);

    XtVaGetValues(players_list, XtNwidth, &width, NULL);
    XtVaSetValues(players_label, XtNwidth, width, NULL); 

  }
}

/**************************************************************************
...
**************************************************************************/
void players_list_callback(Widget w, XtPointer client_data, 
			   XtPointer call_data)

{
  XawListReturnStruct *ret;

  ret=XawListShowCurrent(players_list);

  if(ret->list_index!=XAW_LIST_NONE) {
    if(player_has_embassy(game.player_ptr, &game.players[ret->list_index])) {
      if(game.players[ret->list_index].is_connected &&
	 game.players[ret->list_index].is_alive)
	XtSetSensitive(players_meet_command, TRUE);
      XtSetSensitive(players_int_command, TRUE);
      return;
    }
  }
  XtSetSensitive(players_meet_command, FALSE);
  XtSetSensitive(players_int_command, FALSE);
}


/**************************************************************************
...
**************************************************************************/
void players_button_callback(Widget w, XtPointer client_data, 
			      XtPointer call_data)
{

  XtPopdown(players_dialog_shell);
}

/**************************************************************************
...
**************************************************************************/
void players_meet_callback(Widget w, XtPointer client_data, 
			      XtPointer call_data)
{
  XawListReturnStruct *ret;

  ret=XawListShowCurrent(players_list);

  if(ret->list_index!=XAW_LIST_NONE) {
    if(player_has_embassy(game.player_ptr, &game.players[ret->list_index])) {
      struct packet_diplomacy_info pa;
    
      pa.plrno0=game.player_idx;
      pa.plrno1=ret->list_index;
      send_packet_diplomacy_info(&aconnection, PACKET_DIPLOMACY_INIT_MEETING,
				 &pa);
    }
    else {
      append_output_window("Game: You need an embassy to establish a diplomatic meeting.");
    }
  }
}

/**************************************************************************
...
**************************************************************************/
void players_intel_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{
  XawListReturnStruct *ret;

  ret=XawListShowCurrent(players_list);

  if(ret->list_index!=XAW_LIST_NONE)
    if(player_has_embassy(game.player_ptr, &game.players[ret->list_index]))
      popup_intel_dialog(&game.players[ret->list_index]);
}

