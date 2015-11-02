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
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>  
#include <X11/Xaw/List.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>

#include "player.h"
#include "packets.h"
#include "game.h"
#include "mapctrl.h"
#include "tech.h"
#include "repodlgs.h"
#include "shared.h"
#include "xstuff.h"
#include "mapview.h"
#include "citydlg.h"

#define MAX_CITIES_SHOWN 256

extern Widget toplevel, main_form, map_canvas;

extern struct connection aconnection;
extern Display	*display;
extern int display_depth;
extern struct advance advances[];

extern int did_advance_tech_this_year;

void create_science_dialog(int make_modal);
void science_close_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);
void science_change_callback(Widget w, XtPointer client_data, 
			     XtPointer call_data);

void science_dialog_update(void);

/******************************************************************/
Widget science_dialog_shell;
Widget science_label;
Widget science_current_label;
Widget science_change_menu_button, science_list;
int science_dialog_shell_is_modal;
Widget popupmenu;


/******************************************************************/
void create_city_report_dialog(int make_modal);
void city_close_callback(Widget w, XtPointer client_data, 
			 XtPointer call_data);
void city_center_callback(Widget w, XtPointer client_data, 
			 XtPointer call_data);
void city_popup_callback(Widget w, XtPointer client_data, 
			 XtPointer call_data);

Widget city_dialog_shell;
Widget city_label;
Widget city_list, city_list_label;
int city_dialog_shell_is_modal;
int cities_in_list[MAX_CITIES_SHOWN];


/****************************************************************
...
****************************************************************/
void update_report_dialogs(void)
{
  city_report_dialog_update(); 
  science_dialog_update();
}

/****************************************************************
...
****************************************************************/
char *get_report_title(char *report_name)
{
  char buf[512];
  
  sprintf(buf, "%s\n%s of the %s\n%s %s: %s",
	  report_name,
	  get_goverment_name(game.player_ptr->goverment),
	  get_race_name_plural(game.player_ptr->race),
	  get_ruler_title(game.player_ptr->goverment),
	  game.player_ptr->name,
	  textyear(game.year));

  return create_centered_string(buf);
}


/****************************************************************
...
************************ ***************************************/
void popup_science_dialog(int make_modal)
{

  if(!science_dialog_shell) {
    Position x, y;
    Dimension width, height;
    
    science_dialog_shell_is_modal=make_modal;
    
    if(make_modal)
      XtSetSensitive(main_form, FALSE);
    
    create_science_dialog(make_modal);
    
    XtVaGetValues(toplevel, XtNwidth, &width, XtNheight, &height, NULL);
    
    XtTranslateCoords(toplevel, (Position) width/10, (Position) height/10,
		      &x, &y);
    XtVaSetValues(science_dialog_shell, XtNx, x, XtNy, y, NULL);
    
    XtPopup(science_dialog_shell, XtGrabNone);
  }

}


/****************************************************************
...
*****************************************************************/
void create_science_dialog(int make_modal)
{
  Widget science_form;
  Widget  close_command;
  static char *tech_list_names_ptrs[A_LAST+1];
  static char tech_list_names[A_LAST+1][200];
  int i, j, flag;
  Dimension width;
  char current_text[512];
  char *report_title;
  
  sprintf(current_text, "Researching %s: %d/%d",
	  advances[game.player_ptr->research.researching].name,
	  game.player_ptr->research.researched,
	  research_time(game.player_ptr));
  
  for(i=1, j=0; i<A_LAST; i++)
    if(get_invention(game.player_ptr, i)==TECH_KNOWN) {
      strcpy(tech_list_names[j], advances[i].name);
      tech_list_names_ptrs[j]=tech_list_names[j];
      j++;
    }
  tech_list_names_ptrs[j]=0;
  
  science_dialog_shell = XtVaCreatePopupShell("sciencepopup", 
					      make_modal ? 
					      transientShellWidgetClass :
					      topLevelShellWidgetClass,
					      toplevel, 
					      0);

  science_form = XtVaCreateManagedWidget("scienceform", 
					 formWidgetClass,
					 science_dialog_shell,
					 NULL);   

  report_title=get_report_title("Science Advisor");
  science_label = XtVaCreateManagedWidget("sciencelabel", 
					  labelWidgetClass, 
					  science_form,
					  XtNlabel, 
					  report_title,
					  NULL);
  free(report_title);

  science_current_label = XtVaCreateManagedWidget("sciencecurrentlabel", 
						  labelWidgetClass, 
						  science_form,
						  XtNlabel, 
						  current_text,
						  NULL);

  science_change_menu_button = XtVaCreateManagedWidget(
				       "sciencechangemenubutton", 
					menuButtonWidgetClass,
					science_form,
					NULL);
  
  science_list = XtVaCreateManagedWidget("sciencelist", 
					 listWidgetClass,
					 science_form,
					 XtNlist, tech_list_names_ptrs,
					 NULL);

  close_command = XtVaCreateManagedWidget("scienceclosecommand", 
					  commandWidgetClass,
					  science_form,
					  NULL);
  
  
  popupmenu=XtVaCreatePopupShell("menu", 
				 simpleMenuWidgetClass, 
				 science_change_menu_button, 
				 NULL);

  
  for(i=1, flag=0; i<A_LAST; i++)
    if(get_invention(game.player_ptr, i)==TECH_REACHABLE) {
      Widget entry=
      XtVaCreateManagedWidget(advances[i].name, smeBSBObjectClass, 
			      popupmenu, NULL);
      XtAddCallback(entry, XtNcallback, science_change_callback, 
		    (XtPointer) i); 
      flag=1;
    }
  
  if(!flag)
    XtSetSensitive(science_change_menu_button, FALSE);
  
  XtAddCallback(close_command, XtNcallback, science_close_callback, NULL);

  XtRealizeWidget(science_dialog_shell);

  width=500;
  XtVaSetValues(science_label, XtNwidth, &width, NULL);
}


/****************************************************************
...
*****************************************************************/
void science_change_callback(Widget w, XtPointer client_data, 
			     XtPointer call_data)
{
  char current_text[512];
  struct packet_player_request packet;
  int to;
  
  to=(int)client_data;
  
  sprintf(current_text, "Researching %s: %d/%d",
	  advances[to].name, 0, 
  	  research_time(game.player_ptr));
  
  XtVaSetValues(science_current_label, XtNlabel, current_text, NULL);
  
  packet.tech=to;
  send_packet_player_request(&aconnection, &packet, PACKET_PLAYER_RESEARCH);
}


/****************************************************************
...
*****************************************************************/
void science_close_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{

  if(science_dialog_shell_is_modal)
    XtSetSensitive(main_form, TRUE);
  XtDestroyWidget(science_dialog_shell);
  science_dialog_shell=0;
}

/****************************************************************
...
*****************************************************************/
void science_dialog_update(void)
{
  if(science_dialog_shell) {
    char text[512];
    static char *tech_list_names_ptrs[A_LAST+1];
    static char tech_list_names[A_LAST+1][200];
    int i, j, flag;
    char *report_title;
    
    report_title=get_report_title("Science Advisor");
    xaw_set_label(science_label, report_title);
    free(report_title);

    
    sprintf(text, "Researching %s: %d/%d",
	    advances[game.player_ptr->research.researching].name,
	    game.player_ptr->research.researched,
	    research_time(game.player_ptr));
    
    xaw_set_label(science_current_label, text);

    for(i=1, j=0; i<A_LAST; i++)
      if(get_invention(game.player_ptr, i)==TECH_KNOWN) {
	strcpy(tech_list_names[j], advances[i].name);
	tech_list_names_ptrs[j]=tech_list_names[j];
	j++;
      }
    tech_list_names_ptrs[j]=0;

    XawListChange(science_list, tech_list_names_ptrs, 0/*j*/, 0, 1);

    XtDestroyWidget(popupmenu);
    
    popupmenu=XtVaCreatePopupShell("menu", 
				   simpleMenuWidgetClass, 
				   science_change_menu_button, 
				   NULL);
    
      for(i=1, flag=0; i<A_LAST; i++)
      if(get_invention(game.player_ptr, i)==TECH_REACHABLE) {
	Widget entry=
	  XtVaCreateManagedWidget(advances[i].name, smeBSBObjectClass, 
				  popupmenu, NULL);
	XtAddCallback(entry, XtNcallback, science_change_callback, 
		      (XtPointer) i); 
	flag=1;
      }
    
    if(!flag)
      XtSetSensitive(science_change_menu_button, FALSE);

  }
  
}


/****************************************************************

                      CITY REPORT DIALOG
 
****************************************************************/

/****************************************************************
...
****************************************************************/
void popup_city_report_dialog(int make_modal)
{
  if(!city_dialog_shell) {
      Position x, y;
      Dimension width, height;
      
      city_dialog_shell_is_modal=make_modal;
    
      if(make_modal)
	XtSetSensitive(main_form, FALSE);
      
      create_city_report_dialog(make_modal);
      
      XtVaGetValues(toplevel, XtNwidth, &width, XtNheight, &height, NULL);
      
      XtTranslateCoords(toplevel, (Position) width/10, (Position) height/10,
			&x, &y);
      XtVaSetValues(city_dialog_shell, XtNx, x, XtNy, y, NULL);
      
      XtPopup(city_dialog_shell, XtGrabNone);
   }
}


/****************************************************************
...
*****************************************************************/
void create_city_report_dialog(int make_modal)
{
  Widget city_form;
  Widget close_command, center_command, popup_command;
  char *report_title;
  
  city_dialog_shell = XtVaCreatePopupShell("reportcitypopup", 
					      make_modal ? 
					      transientShellWidgetClass :
					      topLevelShellWidgetClass,
					      toplevel, 
					      0);

  city_form = XtVaCreateManagedWidget("reportcityform", 
					 formWidgetClass,
					 city_dialog_shell,
					 NULL);   

  report_title=get_report_title("City Advisor");
  city_label = XtVaCreateManagedWidget("reportcitylabel", 
				       labelWidgetClass, 
				       city_form,
				       XtNlabel, 
				       report_title,
				       NULL);
  free(report_title);

  city_list_label = XtVaCreateManagedWidget("reportcitylistlabel", 
				       labelWidgetClass, 
				       city_form,
				       NULL);
  
  city_list = XtVaCreateManagedWidget("reportcitylist", 
				      listWidgetClass,
				      city_form,
				      NULL);

  close_command = XtVaCreateManagedWidget("reportcityclosecommand", 
					  commandWidgetClass,
					  city_form,
					  NULL);
  
  center_command = XtVaCreateManagedWidget("reportcitycentercommand", 
					  commandWidgetClass,
					  city_form,
					  NULL);

  popup_command = XtVaCreateManagedWidget("reportcitypopupcommand", 
					  commandWidgetClass,
					  city_form,
					  NULL);

  city_report_dialog_update();
  XtAddCallback(close_command, XtNcallback, city_close_callback, NULL);
  XtAddCallback(center_command, XtNcallback, city_center_callback, NULL);
  XtAddCallback(popup_command, XtNcallback, city_popup_callback, NULL);
  
  XtRealizeWidget(city_dialog_shell);
}



/****************************************************************
...
*****************************************************************/
void city_close_callback(Widget w, XtPointer client_data, 
			 XtPointer call_data)
{

  if(city_dialog_shell_is_modal)
     XtSetSensitive(main_form, TRUE);
   XtDestroyWidget(city_dialog_shell);
   city_dialog_shell=0;
}

/****************************************************************
...
*****************************************************************/
void city_center_callback(Widget w, XtPointer client_data, 
			  XtPointer call_data)
{
  XawListReturnStruct *ret=XawListShowCurrent(city_list);

  if(ret->list_index!=XAW_LIST_NONE) {
    struct city *pcity;
    if((pcity=find_city_by_id(cities_in_list[ret->list_index])))
      center_tile_mapcanvas(pcity->x, pcity->y);
  }
}

/****************************************************************
...
*****************************************************************/
void city_popup_callback(Widget w, XtPointer client_data, 
			 XtPointer call_data)
{
  XawListReturnStruct *ret=XawListShowCurrent(city_list);

  if(ret->list_index!=XAW_LIST_NONE) {
    struct city *pcity;
    if((pcity=find_city_by_id(cities_in_list[ret->list_index]))) {
      center_tile_mapcanvas(pcity->x, pcity->y);
      popup_city_dialog(pcity, 0);
    }
  }
}


/****************************************************************
...
*****************************************************************/
void city_report_dialog_update(void)
{
  if(city_dialog_shell) {
    int i;
    Dimension width; 
    static char *city_list_names_ptrs[MAX_CITIES_SHOWN+1];
    static char city_list_names[MAX_CITIES_SHOWN][200];
    struct genlist_iterator myiter;
    char *report_title;
    
    report_title=get_report_title("City Advisor");
    xaw_set_label(city_label, report_title);
    free(report_title);
    
    genlist_iterator_init(&myiter, &game.player_ptr->cities.list, 0);
    
     for(i=0; ITERATOR_PTR(myiter) && i<MAX_CITIES_SHOWN; 
	 i++, ITERATOR_NEXT(myiter)) {
       char impro[64];
       struct city *pcity=(struct city *)ITERATOR_PTR(myiter);

       if(pcity->is_building_unit)
	 sprintf(impro, "%s(%d/%d)", 
		 get_unit_type(pcity->currently_building)->name,
		 pcity->shield_stock,
		 get_unit_type(pcity->currently_building)->build_cost);
       else
	 sprintf(impro, "%s(%d/%d)", 
		 get_improvement_name(pcity->currently_building),
		 pcity->shield_stock,
		 get_improvement_type(pcity->currently_building)->build_cost);
              
       sprintf(city_list_names[i], "%-15s %-10s %+3d   %+3d    %+3d   %s",
	       pcity->name, 
	       is_city_unhappy(pcity) ? "Disorder" : "Peace",
	       pcity->food_surplus, 
	       pcity->shield_surplus, 
	       pcity->trade_prod,
	       impro);
	       
       city_list_names_ptrs[i]=city_list_names[i];
       cities_in_list[i]=pcity->id;
     }
    if(i==0) {
      strcpy(city_list_names[0], 
	     "                                                             ");
      city_list_names_ptrs[0]=city_list_names[0];
      i=1;
      cities_in_list[0]=0;
    }
    city_list_names_ptrs[i]=0;

    XawListChange(city_list, city_list_names_ptrs, 0, 0, 1);

    XtVaGetValues(city_list, XtNwidth, &width, NULL);
    XtVaSetValues(city_list_label, XtNwidth, width, NULL); 
  }
  
}
