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
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/AsciiText.h>  
#include "pixcomm.h"

#include <X11/IntrinsicP.h>

#include "game.h"
#include "player.h"
#include "city.h"
#include "citydlg.h"
#include "shared.h"
#include "genlist.h"
#include "cityicon.ico"
#include "canvas.h"
#include "mapview.h"
#include "mapctrl.h"
#include "map.h"
#include "packets.h"
#include "dialogs.h"
#include "inputdlg.h"
#include "xstuff.h"
#include "colors.h"

extern Display	*display;
extern Widget toplevel, main_form, map_canvas;
extern int display_depth;
extern struct connection aconnection;
extern int map_view_x0, map_view_y0;
extern int tile_sprite_width, tile_sprite_height;
extern GC fill_bg_gc;

#define NO_UNITS_SHOWN  12
#define NO_CITIZENS_SHOWN 25

struct city_dialog {
  struct city *pcity;
  Widget shell;
  Widget main_form;
  Widget cityname_label;
  Widget citizen_labels[NO_CITIZENS_SHOWN];
  Widget production_label;
  Widget output_label;
  Widget storage_label;
  Widget polution_label;
  Widget sub_form;
  Widget map_canvas;
  Widget sell_command;
  Widget close_command, rename_command, trade_command;
  Widget building_label, progress_label, buy_command, change_command;
  Widget improvement_viewport, improvement_list;
  Widget support_unit_label;
  Widget support_unit_pixcomms[NO_UNITS_SHOWN];
  Widget present_unit_label;
  Widget present_unit_pixcomms[NO_UNITS_SHOWN];
  Widget change_list;
  Widget rename_input;
  
  enum improvement_type_id sell_id;
  
  int citizen_type[NO_CITIZENS_SHOWN];
  int support_unit_ids[NO_UNITS_SHOWN];
  int present_unit_ids[NO_UNITS_SHOWN];
  char improvlist_names[B_LAST+1][64];
  char *improvlist_names_ptrs[B_LAST+1];
  
  char *change_list_names_ptrs[B_LAST+1+U_LAST+1+1];
  char change_list_names[B_LAST+1+U_LAST+1][200];
  int change_list_ids[B_LAST+1+U_LAST+1];
  int change_list_no_improvements;

  int is_modal;
};

struct genlist dialog_list;
int dialog_list_has_been_initialised;

struct city_dialog *get_city_dialog(struct city *pcity);
struct city_dialog *create_city_dialog(struct city *pcity, int make_modal);
void close_city_dialog(struct city_dialog *pdialog);

void city_dialog_update_improvement_list(struct city_dialog *pdialog);
void city_dialog_update_title(struct city_dialog *pdialog);
void city_dialog_update_supported_units(struct city_dialog *pdialog, int id);
void city_dialog_update_present_units(struct city_dialog *pdialog, int id);
void city_dialog_update_citizens(struct city_dialog *pdialog);
void city_dialog_update_map(struct city_dialog *pdialog);
void city_dialog_update_production(struct city_dialog *pdialog);
void city_dialog_update_output(struct city_dialog *pdialog);
void city_dialog_update_building(struct city_dialog *pdialog);
void city_dialog_update_storage(struct city_dialog *pdialog);
void city_dialog_update_polution(struct city_dialog *pdialog);

void sell_callback(Widget w, XtPointer client_data, XtPointer call_data);
void buy_callback(Widget w, XtPointer client_data, XtPointer call_data);
void change_callback(Widget w, XtPointer client_data, XtPointer call_data);
void close_callback(Widget w, XtPointer client_data, XtPointer call_data);
void rename_callback(Widget w, XtPointer client_data, XtPointer call_data);
void trade_callback(Widget w, XtPointer client_data, XtPointer call_data);

void elvis_callback(Widget w, XtPointer client_data, XtPointer call_data);
void scientist_callback(Widget w, XtPointer client_data, XtPointer call_data);
void taxman_callback(Widget w, XtPointer client_data, XtPointer call_data);
void rename_ok_return_action(Widget w, XEvent *event, String *params, 
			     Cardinal *nparams);

void present_units_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);

char *dummy_improvement_list[]={ 
  "Copernicus' Observatory  ",
  "Copernicus' Observatory  ",
  "Copernicus' Observatory  ",
  "Copernicus' Observatory  ",
  "Copernicus' Observatory  ",
  0
};

char *dummy_change_list[]={ 
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  "Copernicus' Observatory  125 turns",
  0
};

/****************************************************************
...
*****************************************************************/
struct city_dialog *get_city_dialog(struct city *pcity)
{
  struct genlist_iterator myiter;

  if(!dialog_list_has_been_initialised) {
    genlist_init(&dialog_list);
    dialog_list_has_been_initialised=1;
  }
  
  genlist_iterator_init(&myiter, &dialog_list, 0);
    
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    if(((struct city_dialog *)ITERATOR_PTR(myiter))->pcity==pcity)
      return ITERATOR_PTR(myiter);

  return 0;
}

/****************************************************************
...
*****************************************************************/
void refresh_city_dialog(struct city *pcity)
{
  struct city_dialog *pdialog;
    
  if((pdialog=get_city_dialog(pcity))) {
    city_dialog_update_improvement_list(pdialog);
    city_dialog_update_title(pdialog);
    city_dialog_update_supported_units(pdialog, 0);
    city_dialog_update_present_units(pdialog, 0);
    city_dialog_update_citizens(pdialog);
    city_dialog_update_map(pdialog);
    city_dialog_update_production(pdialog);
    city_dialog_update_output(pdialog);
    city_dialog_update_building(pdialog);
    city_dialog_update_storage(pdialog);
    city_dialog_update_polution(pdialog);
  }

}

/****************************************************************
...
*****************************************************************/
void refresh_unit_city_dialogs(struct unit *punit)
{
  struct city *pcity_sup, *pcity_pre;
  struct city_dialog *pdialog;

  pcity_sup=city_list_find_id(&game.player_ptr->cities, punit->homecity);
  pcity_pre=game_find_city_by_coor(punit->x, punit->y);
  
  if(pcity_sup && (pdialog=get_city_dialog(pcity_sup)))
    city_dialog_update_supported_units(pdialog, punit->id);
  
  if(pcity_pre && (pdialog=get_city_dialog(pcity_pre)))
    city_dialog_update_present_units(pdialog, punit->id);
}




/****************************************************************
popup the dialog 10% inside the main-window 
*****************************************************************/
void popup_city_dialog(struct city *pcity, int make_modal)
{
  struct city_dialog *pdialog;
  
  if(!(pdialog=get_city_dialog(pcity)))
    pdialog=create_city_dialog(pcity, make_modal);

  xaw_set_relative_position(toplevel, pdialog->shell, 10, 10);
  XtPopup(pdialog->shell, XtGrabNone);
}

/****************************************************************
popdown the dialog 
*****************************************************************/
void popdown_city_dialog(struct city *pcity)
{
  struct city_dialog *pdialog;
  
  if((pdialog=get_city_dialog(pcity)))
    close_city_dialog(pdialog);
}



/****************************************************************
...
*****************************************************************/
void city_map_canvas_expose(Widget w, XEvent *event, Region exposed, 
			    void *client_data)
{
  struct city_dialog *pdialog;
  
  pdialog=(struct city_dialog *)client_data;
  city_dialog_update_map(pdialog);
}


/****************************************************************
...
*****************************************************************/
struct city_dialog *create_city_dialog(struct city *pcity, int make_modal)
{
  int i;
  struct city_dialog *pdialog;
  Pixmap icon_pixmap; 
  
  pdialog=(struct city_dialog *)malloc(sizeof(struct city_dialog));
  pdialog->pcity=pcity;

  pdialog->shell=XtVaCreatePopupShell(pcity->name,
				      make_modal ? transientShellWidgetClass :
				      topLevelShellWidgetClass,
				      toplevel, 
				      XtNallowShellResize, True, 
				      NULL);
  
  pdialog->main_form=
    XtVaCreateManagedWidget("citymainform", 
			    formWidgetClass, 
			    pdialog->shell, 
			    NULL);

  pdialog->cityname_label=
    XtVaCreateManagedWidget("citynamelabel", 
			    labelWidgetClass,
			    pdialog->main_form,
			    NULL);

  pdialog->citizen_labels[0]=
    XtVaCreateManagedWidget("citizenlabels",
			    commandWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->cityname_label,
			    XtNbitmap, get_citizen_pixmap(2),
			    NULL);


  for(i=1; i<NO_CITIZENS_SHOWN; i++)
    pdialog->citizen_labels[i]=
    XtVaCreateManagedWidget("citizenlabels",
			    commandWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->cityname_label,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->citizen_labels[i-1],
			    XtNbitmap, get_citizen_pixmap(2),
			    NULL);
    
  pdialog->sub_form=
    XtVaCreateManagedWidget("citysubform", 
			    formWidgetClass, 
			    pdialog->main_form, 
			    XtNfromVert, 
			    (XtArgVal)pdialog->citizen_labels[0],
			    NULL);


  pdialog->production_label=
    XtVaCreateManagedWidget("cityprodlabel", 
			    labelWidgetClass,
			    pdialog->sub_form,
			    NULL);

  pdialog->output_label=
    XtVaCreateManagedWidget("cityoutputlabel", 
			    labelWidgetClass,
			    pdialog->sub_form,
			    XtNfromVert, 
			    (XtArgVal)pdialog->production_label,
			    NULL);

  pdialog->storage_label=
    XtVaCreateManagedWidget("citystoragelabel", 
			    labelWidgetClass,
			    pdialog->sub_form,
			    XtNfromVert, 
			    (XtArgVal)pdialog->output_label,
			    NULL);

  pdialog->polution_label=
    XtVaCreateManagedWidget("citypolutionlabel", 
			    labelWidgetClass,
			    pdialog->sub_form,
			    XtNfromVert, 
			    (XtArgVal)pdialog->storage_label,
			    NULL);
  
  
  pdialog->map_canvas=
    XtVaCreateManagedWidget("citymapcanvas", 
			    xfwfcanvasWidgetClass,
			    pdialog->sub_form,
			    "exposeProc", 
			    (XtArgVal)city_map_canvas_expose,
			    "exposeProcData", 
			    (XtArgVal)pdialog,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->production_label,
			    NULL);

  
  pdialog->building_label=
    XtVaCreateManagedWidget("citybuildinglabel",
			    labelWidgetClass,
			    pdialog->sub_form,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->map_canvas,
			    NULL);
  
  pdialog->progress_label=
    XtVaCreateManagedWidget("cityprogresslabel",
			    labelWidgetClass,
			    pdialog->sub_form,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->map_canvas,
			    XtNfromVert, 
			    pdialog->building_label,
			    NULL);

  pdialog->buy_command=
    XtVaCreateManagedWidget("citybuycommand", 
			    commandWidgetClass,
			    pdialog->sub_form,
			    XtNfromVert, 
			    pdialog->building_label,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->progress_label,
			    NULL);


  pdialog->change_command=
    XtVaCreateManagedWidget("citychangecommand", 
			    commandWidgetClass,
			    pdialog->sub_form,
			    XtNfromVert, 
			    pdialog->building_label,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->buy_command,
			    NULL);
  
  pdialog->improvement_viewport=
    XtVaCreateManagedWidget("cityimprovview", 
			    viewportWidgetClass,
			    pdialog->sub_form,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->map_canvas,
			    XtNfromVert, 
			    pdialog->progress_label,
			    NULL);


  pdialog->improvement_list=
    XtVaCreateManagedWidget("cityimprovlist", 
			    listWidgetClass,
			    pdialog->improvement_viewport,
			    XtNforceColumns, 1,
			    XtNdefaultColumns,1, 
			    XtNlist, (XtArgVal)dummy_improvement_list,
			    XtNverticalList, False,
			    NULL);

  pdialog->sell_command=
    XtVaCreateManagedWidget("citysellcommand", 
			    commandWidgetClass,
			    pdialog->sub_form,
			    XtNfromVert, 
			    pdialog->improvement_viewport,
			    XtNfromHoriz, 
			    (XtArgVal)pdialog->map_canvas,
			    NULL);
  
  icon_pixmap = XCreateBitmapFromData(display,
				      RootWindowOfScreen(XtScreen(toplevel)),
				      cityicon_bits,
				      cityicon_width, cityicon_height);

  pdialog->support_unit_label=
    XtVaCreateManagedWidget("supportunitlabel",
			    labelWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->sub_form,
			    NULL);

  pdialog->support_unit_pixcomms[0]=
    XtVaCreateManagedWidget("supportunitcanvas",
			    pixcommWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->support_unit_label,
			    NULL);
  pdialog->support_unit_ids[0]=-1;


  for(i=1; i<NO_UNITS_SHOWN; i++) {
    pdialog->support_unit_pixcomms[i]=
      XtVaCreateManagedWidget("supportunitcanvas",
			      pixcommWidgetClass,
			      pdialog->main_form,
			      XtNfromVert, 
			      pdialog->support_unit_label,
			      XtNfromHoriz, 
			      (XtArgVal)pdialog->support_unit_pixcomms[i-1],
			      XtNinternalHeight, 0,
			      NULL);
    pdialog->support_unit_ids[i]=-1;
  }
    

  pdialog->present_unit_label=
    XtVaCreateManagedWidget("presentunitlabel",
			    labelWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->support_unit_pixcomms[0],
			    NULL);

  pdialog->present_unit_pixcomms[0]=
    XtVaCreateManagedWidget("presentunitcanvas",
			    pixcommWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->present_unit_label,
			    NULL);
  pdialog->present_unit_ids[0]=-1;

  for(i=1; i<NO_UNITS_SHOWN; i++) {
    pdialog->present_unit_pixcomms[i]=
      XtVaCreateManagedWidget("presentunitcanvas",
			      pixcommWidgetClass,
			      pdialog->main_form,
			      XtNfromVert, 
			      pdialog->present_unit_label,
			      XtNfromHoriz, 
			      (XtArgVal)pdialog->support_unit_pixcomms[i-1],
			      NULL);
    pdialog->present_unit_ids[i]=-1;
  }
    

  XtVaSetValues(pdialog->shell, XtNiconPixmap, icon_pixmap, NULL);

  pdialog->close_command=
    XtVaCreateManagedWidget("cityclosecommand", 
			    commandWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->present_unit_pixcomms[0],
			    NULL);

  pdialog->rename_command=
    XtVaCreateManagedWidget("cityrenamecommand", 
			    commandWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->present_unit_pixcomms[0],
			    XtNfromHoriz,
			    pdialog->close_command,
			    NULL);

  pdialog->trade_command=
    XtVaCreateManagedWidget("citytradecommand", 
			    commandWidgetClass,
			    pdialog->main_form,
			    XtNfromVert, 
			    pdialog->present_unit_pixcomms[0],
			    XtNfromHoriz,
			    pdialog->rename_command,
			    NULL);
  
  XtAddCallback(pdialog->sell_command, XtNcallback, sell_callback,
		(XtPointer)pdialog);

  XtAddCallback(pdialog->buy_command, XtNcallback, buy_callback,
		(XtPointer)pdialog);

  XtAddCallback(pdialog->change_command, XtNcallback, change_callback,
		(XtPointer)pdialog);
  
  XtAddCallback(pdialog->close_command, XtNcallback, close_callback,
		(XtPointer)pdialog);

  XtAddCallback(pdialog->rename_command, XtNcallback, rename_callback,
		(XtPointer)pdialog);

  XtAddCallback(pdialog->trade_command, XtNcallback, trade_callback,
		(XtPointer)pdialog);

  genlist_insert(&dialog_list, pdialog, 0);

  for(i=0; i<B_LAST+1; i++)
    pdialog->improvlist_names_ptrs[i]=0;

  for(i=0; i<NO_CITIZENS_SHOWN; i++)
    pdialog->citizen_type[i]=-1;

  
  XtRealizeWidget(pdialog->shell);

  refresh_city_dialog(pdialog->pcity);

  if(make_modal)
    XtSetSensitive(toplevel, FALSE);
  
  pdialog->is_modal=make_modal;
  
  return pdialog;
}

/****************************************************************
...
*****************************************************************/
void present_units_ok_callback(Widget w, XtPointer client_data, 
				XtPointer call_data)
{
  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void activate_unit(int unit_id)
{
  struct unit *punit;
  
  if((punit=unit_list_find(&game.player_ptr->units, unit_id))) {
    if(punit->activity!=ACTIVITY_IDLE) {
      if(can_unit_do_activity(punit, ACTIVITY_IDLE)) {
	request_new_unit_activity(punit, ACTIVITY_IDLE);
	set_unit_focus(punit);
      }
    }
    else
      set_unit_focus(punit);
  }
}


/****************************************************************
...
*****************************************************************/
void present_units_activate_callback(Widget w, XtPointer client_data, 
				     XtPointer call_data)
{
  activate_unit((int)client_data);
  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void present_units_activate_close_callback(Widget w, XtPointer client_data, 
					   XtPointer call_data)
{
  struct unit *punit;
  struct city *pcity;
  struct city_dialog *pdialog;

  activate_unit((int)client_data);

  destroy_message_dialog(w);

  if((punit=unit_list_find(&game.player_ptr->units, (int)client_data)))
    if((pcity=game_find_city_by_coor(punit->x, punit->y)))
      if((pdialog=get_city_dialog(pcity)))
	close_city_dialog(pdialog);
}



/****************************************************************
...
*****************************************************************/
void present_units_disband_callback(Widget w, XtPointer client_data, 
				    XtPointer call_data)
{
  struct unit *punit;

  if((punit=unit_list_find(&game.player_ptr->units, (int)client_data)))
    request_unit_disband(punit);

  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void present_units_homecity_callback(Widget w, XtPointer client_data, 
				     XtPointer call_data)
{
  struct unit *punit;
  
  if((punit=unit_list_find(&game.player_ptr->units, (int)client_data)))
    request_unit_change_homecity(punit);

  destroy_message_dialog(w);
}



/****************************************************************
...
*****************************************************************/
void present_units_cancel_callback(Widget w, XtPointer client_data, 
				XtPointer call_data)
{
  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void present_units_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{
  struct unit *punit;
  struct city *pcity;
  struct city_dialog *pdialog;
  
  if((punit=unit_list_find(&game.player_ptr->units, (int)client_data)))
    if((pcity=game_find_city_by_coor(punit->x, punit->y)))
      if((pdialog=get_city_dialog(pcity))) {
	popup_message_dialog(pdialog->shell, 
			     "presentunitsdialog", 
			     unit_description(punit),
			     present_units_activate_callback, punit->id,
			     present_units_activate_close_callback, punit->id, /* act+c */
			     present_units_disband_callback, punit->id,
			     present_units_homecity_callback, punit->id,
			     present_units_cancel_callback, 0, 0);
      }
}


/****************************************************************
...
*****************************************************************/
void rename_city_callback(Widget w, XtPointer client_data, 
			  XtPointer call_data)
{
  struct city_dialog *pdialog;
  struct packet_city_request packet;

  if((pdialog=(struct city_dialog *)client_data)) {
    packet.city_id=pdialog->pcity->id;
    strncpy(packet.name, input_dialog_get_input(w), MAX_LENGTH_NAME);
    packet.name[MAX_LENGTH_NAME-1]='\0';
    send_packet_city_request(&aconnection, &packet, PACKET_CITY_RENAME);
  }
  input_dialog_destroy(w);
}





/****************************************************************
...
*****************************************************************/
void rename_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;

  pdialog=(struct city_dialog *)client_data;
  
  input_dialog_create(pdialog->shell, 
		      "shellrenamecity", 
		      "What should we rename the city to?",
		      pdialog->pcity->name,
		      (void*)rename_city_callback, (XtPointer)pdialog,
		      (void*)rename_city_callback, (XtPointer)0);
}

/****************************************************************
...
*****************************************************************/
void trade_message_dialog_callback(Widget w, XtPointer client_data, 
				   XtPointer call_data)
{
  destroy_message_dialog(w);
}

/****************************************************************
...
*****************************************************************/
void trade_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int i;
  int x=0;
  char buf[512];
  struct city_dialog *pdialog;

  pdialog=(struct city_dialog *)client_data;

  sprintf(buf, "These trade routes have been established with %s:\n",
	  pdialog->pcity->name);
  
  for(i=0; i<4; i++)
    if(pdialog->pcity->trade[i]) {
      struct city *pcity;
      x=1;
      if((pcity=find_city_by_id(pdialog->pcity->trade[i]))) {
	sprintf(buf+strlen(buf), "%32s: %2d gold/year\n",
		pcity->name, 
		trade_between_cities(pdialog->pcity, pcity));
      } else {
	sprintf(buf+strlen(buf), "%32s: %2s Gold/Year\n","Unknown","??");
      }

    }
  if (!x)
    sprintf(buf+strlen(buf), "No trade routes exist.\n");
  
  popup_message_dialog(pdialog->shell, 
		       "citytradedialog", 
		       buf, 
		       trade_message_dialog_callback, 0,
		       0);
}


/****************************************************************
...
*****************************************************************/
void city_dialog_update_polution(struct city_dialog *pdialog)
{
  char buf[512];
  struct city *pcity=pdialog->pcity;

  sprintf(buf, "Polution:    %3d", pcity->polution);

  xaw_set_label(pdialog->polution_label, buf);
}



/****************************************************************
...
*****************************************************************/
void city_dialog_update_storage(struct city_dialog *pdialog)
{
  char buf[512];
  struct city *pcity=pdialog->pcity;
  
  sprintf(buf, "Granary: %3d/%-3d", pcity->food_stock,
	  10*pcity->size);

  xaw_set_label(pdialog->storage_label, buf);
}

/****************************************************************
...
*****************************************************************/

void city_dialog_update_building(struct city_dialog *pdialog)
{
  char buf[512], buf2[512];
  struct city *pcity=pdialog->pcity;
  
  if(pcity->is_building_unit) {
    sprintf(buf, "%3d/%3d", pcity->shield_stock, 
	    get_unit_type(pcity->currently_building)->build_cost);
    sprintf(buf2, "%s", get_unit_type(pcity->currently_building)->name);
  }
  else {
    sprintf(buf, "%3d/%3d", pcity->shield_stock, 
	    get_improvement_type(pcity->currently_building)->build_cost);
    sprintf(buf2, "%s", 
	    get_improvement_name(pcity->currently_building));
  }
    
  XtSetSensitive(pdialog->buy_command, !pcity->did_buy);
  XtSetSensitive(pdialog->sell_command, !pcity->did_sell);

  xaw_set_label(pdialog->building_label, buf2);
  xaw_set_label(pdialog->progress_label, buf);
}



/****************************************************************
...
*****************************************************************/
void city_dialog_update_production(struct city_dialog *pdialog)
{
  char buf[512];
  struct city *pcity=pdialog->pcity;
  
  sprintf(buf, "Food:    %2d (%+2d)\nProd:    %2d (%+2d)\nTrade:   %2d (%+2d)",
	  pcity->food_prod, pcity->food_surplus,
	  pcity->shield_prod, pcity->shield_surplus,
	  pcity->trade_prod+pcity->corruption, pcity->trade_prod);

  xaw_set_label(pdialog->production_label, buf);
}
/****************************************************************
...
*****************************************************************/
void city_dialog_update_output(struct city_dialog *pdialog)
{
  char buf[512];
  struct city *pcity=pdialog->pcity;
  
  sprintf(buf, "Gold:    %2d (%+2d)\nLuxury:  %2d\nScience: %2d",
	  pcity->tax_total, calc_gold_remains(pcity),
	  pcity->luxury_total,
	  pcity->science_total);

  xaw_set_label(pdialog->output_label, buf);
}


/****************************************************************
...
*****************************************************************/
void city_dialog_update_map(struct city_dialog *pdialog)
{
  int x, y;
  struct city *pcity=pdialog->pcity;
  
  for(y=0; y<CITY_MAP_SIZE; y++)
    for(x=0; x<CITY_MAP_SIZE; x++) {
      if(!(x==0 && y==0) && !(x==0 && y==CITY_MAP_SIZE-1) &&
	 !(x==CITY_MAP_SIZE-1 && y==0) && 
	 !(x==CITY_MAP_SIZE-1 && y==CITY_MAP_SIZE-1) &&
	 tile_is_known(pcity->x+x-CITY_MAP_SIZE/2, 
		       pcity->y+y-CITY_MAP_SIZE/2)) {
	pixmap_put_tile(XtWindow(pdialog->map_canvas), x, y, 
	                pcity->x+x-CITY_MAP_SIZE/2,
			pcity->y+y-CITY_MAP_SIZE/2, 1);
	if(pcity->city_map[x][y]==C_TILE_WORKER)
	  put_city_tile_output(XtWindow(pdialog->map_canvas), x, y, 
			       get_food_tile(x, y, pcity),
			       get_shields_tile(x, y, pcity), 
			       get_trade_tile(x, y, pcity) );
	else if(pcity->city_map[x][y]==C_TILE_UNAVAILABLE)
	  pixmap_frame_tile_red(XtWindow(pdialog->map_canvas), x, y);
      }
      else {
	pixmap_put_black_tile(XtWindow(pdialog->map_canvas), x, y);
      }
    }
}

/****************************************************************
...
*****************************************************************/
void city_dialog_update_citizens(struct city_dialog *pdialog)
{
  int i, n;
  struct city *pcity=pdialog->pcity;
    
  for(i=0, n=0; n<pcity->ppl_happy[4] && i<NO_CITIZENS_SHOWN; n++, i++)
    if(pdialog->citizen_type[i]!=5 &&  pdialog->citizen_type[i]!=6) {
      pdialog->citizen_type[i]=5+i%2;
      xaw_set_bitmap(pdialog->citizen_labels[i], 
		     get_citizen_pixmap(pdialog->citizen_type[i]));
      XtSetSensitive(pdialog->citizen_labels[i], FALSE);
      XtRemoveAllCallbacks(pdialog->citizen_labels[i], XtNcallback);
    }

  for(n=0; n<pcity->ppl_content[4] && i<NO_CITIZENS_SHOWN; n++, i++)
    if(pdialog->citizen_type[i]!=3 && pdialog->citizen_type[i]!=4) {
      pdialog->citizen_type[i]=3+i%2;
      xaw_set_bitmap(pdialog->citizen_labels[i], 
		     get_citizen_pixmap(pdialog->citizen_type[i]));
      XtSetSensitive(pdialog->citizen_labels[i], FALSE);
      XtRemoveAllCallbacks(pdialog->citizen_labels[i], XtNcallback);
    }
      
  for(n=0; n<pcity->ppl_unhappy[4] && i<NO_CITIZENS_SHOWN; n++, i++)
    if(pdialog->citizen_type[i]!=7) {
      xaw_set_bitmap(pdialog->citizen_labels[i], get_citizen_pixmap(7));
      pdialog->citizen_type[i]=7;
      XtRemoveAllCallbacks(pdialog->citizen_labels[i], XtNcallback);
      XtSetSensitive(pdialog->citizen_labels[i], FALSE);
    }
      
  for(n=0; n<pcity->ppl_elvis && i<NO_CITIZENS_SHOWN; n++, i++)
    if(pdialog->citizen_type[i]!=0) {
      xaw_set_bitmap(pdialog->citizen_labels[i], get_citizen_pixmap(0));
      pdialog->citizen_type[i]=0;
      XtRemoveAllCallbacks(pdialog->citizen_labels[i], XtNcallback);
      XtAddCallback(pdialog->citizen_labels[i], XtNcallback, elvis_callback,
		    (XtPointer)pdialog);
      XtSetSensitive(pdialog->citizen_labels[i], TRUE);
    }

  
  for(n=0; n<pcity->ppl_scientist && i<NO_CITIZENS_SHOWN; n++, i++)
    if(pdialog->citizen_type[i]!=1) {
      xaw_set_bitmap(pdialog->citizen_labels[i], get_citizen_pixmap(1));
      pdialog->citizen_type[i]=1;
      XtRemoveAllCallbacks(pdialog->citizen_labels[i], XtNcallback);
      XtAddCallback(pdialog->citizen_labels[i], XtNcallback, scientist_callback,
		    (XtPointer)pdialog);
      XtSetSensitive(pdialog->citizen_labels[i], TRUE);
    }
  
  for(n=0; n<pcity->ppl_taxman && i<NO_CITIZENS_SHOWN; n++, i++)
    if(pdialog->citizen_type[i]!=2) {
      xaw_set_bitmap(pdialog->citizen_labels[i], get_citizen_pixmap(2));
      pdialog->citizen_type[i]=2;
      XtRemoveAllCallbacks(pdialog->citizen_labels[i], XtNcallback);
      XtAddCallback(pdialog->citizen_labels[i], XtNcallback, taxman_callback,
		    (XtPointer)pdialog);
      XtSetSensitive(pdialog->citizen_labels[i], TRUE);
    }
  
  for(; i<NO_CITIZENS_SHOWN; i++) {
    xaw_set_bitmap(pdialog->citizen_labels[i], None);
    XtSetSensitive(pdialog->citizen_labels[i], FALSE);
    XtRemoveAllCallbacks(pdialog->citizen_labels[i], XtNcallback);
  }
}

/****************************************************************
...
*****************************************************************/
void support_units_callback(Widget w, XtPointer client_data, 
			      XtPointer call_data)
{
  struct unit *punit;
  struct city *pcity;
  struct city_dialog *pdialog;
  
  if((punit=unit_list_find(&game.player_ptr->units, (int)client_data)))
    if((pcity=find_city_by_id(punit->homecity)))
      if((pdialog=get_city_dialog(pcity)))
	popup_message_dialog(pdialog->shell,
			     "supportunitsdialog", 
			     unit_description(punit),
			     present_units_activate_callback, punit->id,
			     present_units_activate_close_callback, punit->id, /* act+c */
			     present_units_disband_callback, punit->id,
			     present_units_cancel_callback, 0, 0);
}

/****************************************************************
...
*****************************************************************/
void city_dialog_update_supported_units(struct city_dialog *pdialog, 
					int unitid)
{
  int i;
  struct genlist_iterator myiter;
  struct unit *punit;

  if(unitid) {
    for(i=0; i<NO_UNITS_SHOWN; i++)
      if(pdialog->support_unit_ids[i]==unitid)
	break;
    if(i==NO_UNITS_SHOWN)
      unitid=0;
  }
  
  genlist_iterator_init(&myiter, &pdialog->pcity->units_supported.list, 0);

  for(i=0; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter), i++) {
    punit=(struct unit*)ITERATOR_PTR(myiter);
        
    if(unitid && punit->id!=unitid)
      continue;
    put_unit_pixmap(punit, XawPixcommPixmap(pdialog->support_unit_pixcomms[i]), 
		    0, 0);

    
    put_unit_pixmap_city_overlays(punit, 
				  XawPixcommPixmap(pdialog->support_unit_pixcomms[i]),
				  punit->unhappiness, punit->upkeep);   
    xaw_expose_now(pdialog->support_unit_pixcomms[i]);
    pdialog->support_unit_ids[i]=punit->id;
    
    XtRemoveAllCallbacks(pdialog->support_unit_pixcomms[i], XtNcallback);
    XtAddCallback(pdialog->support_unit_pixcomms[i], XtNcallback, 
		  support_units_callback, (XtPointer)punit->id);
    XtSetSensitive(pdialog->support_unit_pixcomms[i], TRUE);
  }
    
  for(; i<NO_UNITS_SHOWN; i++) {
    XawPixcommClear(pdialog->support_unit_pixcomms[i]);
    pdialog->support_unit_ids[i]=0;
    XtSetSensitive(pdialog->support_unit_pixcomms[i], FALSE);
  }
}

/****************************************************************
...
*****************************************************************/
void city_dialog_update_present_units(struct city_dialog *pdialog, int unitid)
{
  int i;
  struct genlist_iterator myiter;
  struct unit *punit;

  if(unitid) {
    for(i=0; i<NO_UNITS_SHOWN; i++)
      if(pdialog->present_unit_ids[i]==unitid)
	break;
    if(i==NO_UNITS_SHOWN)
      unitid=0;
  }

  genlist_iterator_init(&myiter, 
	&map_get_tile(pdialog->pcity->x, pdialog->pcity->y)->units.list, 0);
  
  for(i=0; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter), i++) {
    punit=(struct unit*)ITERATOR_PTR(myiter);
    
    if(unitid && punit->id!=unitid)
      continue;
    
    
    put_unit_pixmap(punit, XawPixcommPixmap(pdialog->present_unit_pixcomms[i]),
		    0, 0);
    
    put_unit_pixmap_city_overlays(punit, 
			  XawPixcommPixmap(pdialog->present_unit_pixcomms[i]),
				  0,0);
    xaw_expose_now(pdialog->present_unit_pixcomms[i]);
    pdialog->present_unit_ids[i]=punit->id;
    
    XtRemoveAllCallbacks(pdialog->present_unit_pixcomms[i], XtNcallback);
    XtAddCallback(pdialog->present_unit_pixcomms[i], XtNcallback, 
		  present_units_callback, (XtPointer)punit->id);
    XtSetSensitive(pdialog->present_unit_pixcomms[i], TRUE);
  }
  
  for(; i<NO_UNITS_SHOWN; i++) {
    XawPixcommClear(pdialog->present_unit_pixcomms[i]);
    pdialog->present_unit_ids[i]=0;
    XtSetSensitive(pdialog->present_unit_pixcomms[i], FALSE);
  }
}

/****************************************************************
...
*****************************************************************/
void city_dialog_update_title(struct city_dialog *pdialog)
{
  char buf[512];
  String now;
  
  sprintf(buf, "%s - %s citizens",
	  pdialog->pcity->name, int_to_text(city_population(pdialog->pcity)));

  XtVaGetValues(pdialog->cityname_label, XtNlabel, &now, NULL);
  if(strcmp(now, buf)) {
    XtVaSetValues(pdialog->cityname_label, XtNlabel, (XtArgVal)buf, NULL);
    xaw_horiz_center(pdialog->cityname_label);
  }
}

/****************************************************************
...
*****************************************************************/
void city_dialog_update_improvement_list(struct city_dialog *pdialog)
{
  int i, n, flag;

  for(i=0, n=0, flag=0; i<B_LAST; ++i)
    if(pdialog->pcity->improvements[i]) {
      if(!pdialog->improvlist_names_ptrs[n] ||
	 strcmp(pdialog->improvlist_names_ptrs[n], get_improvement_name(i)))
	flag=1;
      strcpy(pdialog->improvlist_names[n], get_improvement_name(i));
      pdialog->improvlist_names_ptrs[n]=pdialog->improvlist_names[n];
      n++;
    }
  
  if(pdialog->improvlist_names_ptrs[n]!=0) {
    pdialog->improvlist_names_ptrs[n]=0;
    flag=1;
  }
  
  if(flag || n==0) {
    XawListChange(pdialog->improvement_list, pdialog->improvlist_names_ptrs, 
		  n, 0, False);  
    /* force refresh of viewport so the scrollbar is added.
     * Buggy sun athena requires this */
    XtVaSetValues(pdialog->improvement_viewport, XtNforceBars, False, NULL);
    XtVaSetValues(pdialog->improvement_viewport, XtNforceBars, True, NULL);
  }
}


/**************************************************************************
...
**************************************************************************/
void button_down_citymap(Widget w, XEvent *event, String *argv, Cardinal *argc)
{
  XButtonEvent *ev=&event->xbutton;
  struct genlist_iterator myiter;
  struct city *pcity;

  genlist_iterator_init(&myiter, &dialog_list, 0);
    
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    if(((struct city_dialog *)ITERATOR_PTR(myiter))->map_canvas==w)
      break;

  if((pcity=((struct city_dialog *)ITERATOR_PTR(myiter))->pcity)) {
    int xtile, ytile;
    struct packet_city_request packet;

    xtile=ev->x/tile_sprite_width;
    ytile=ev->y/tile_sprite_height;
    packet.city_id=pcity->id;
    packet.worker_x=xtile;
    packet.worker_y=ytile;
    packet.name[0]='\0';
    
    if(pcity->city_map[xtile][ytile]==C_TILE_WORKER)
      send_packet_city_request(&aconnection, &packet, 
			       PACKET_CITY_MAKE_SPECIALIST);
    else if(pcity->city_map[xtile][ytile]==C_TILE_EMPTY)
      send_packet_city_request(&aconnection, &packet, PACKET_CITY_MAKE_WORKER);
  }
}

/****************************************************************
...
*****************************************************************/
void elvis_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  struct packet_city_request packet;
  
  pdialog=(struct city_dialog *)client_data;

  packet.city_id=pdialog->pcity->id;
  packet.name[0]='\0';
  packet.specialist_from=SP_ELVIS;
  packet.specialist_to=SP_SCIENTIST;
  
  send_packet_city_request(&aconnection, &packet, 
			   PACKET_CITY_CHANGE_SPECIALIST);
}

/****************************************************************
...
*****************************************************************/
void scientist_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  struct packet_city_request packet;
  
  pdialog=(struct city_dialog *)client_data;

  packet.city_id=pdialog->pcity->id;
  packet.name[0]='\0';
  packet.specialist_from=SP_SCIENTIST;
  packet.specialist_to=SP_TAXMAN;
  
  send_packet_city_request(&aconnection, &packet, 
			   PACKET_CITY_CHANGE_SPECIALIST);
}

/****************************************************************
...
*****************************************************************/
void taxman_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  struct packet_city_request packet;
  
  pdialog=(struct city_dialog *)client_data;

  packet.city_id=pdialog->pcity->id;
  packet.name[0]='\0';
  packet.specialist_from=SP_TAXMAN;
  packet.specialist_to=SP_ELVIS;
  
  send_packet_city_request(&aconnection, &packet, 
			   PACKET_CITY_CHANGE_SPECIALIST);
}

/****************************************************************
...
*****************************************************************/
void buy_callback_yes(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  struct packet_city_request packet;

  pdialog=(struct city_dialog *)client_data;

  packet.city_id=pdialog->pcity->id;
  packet.name[0]='\0';
  send_packet_city_request(&aconnection, &packet, PACKET_CITY_BUY);

  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void buy_callback_no(Widget w, XtPointer client_data, XtPointer call_data)
{
  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void buy_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  int value;
  char *name;
  char buf[512];
  
  pdialog=(struct city_dialog *)client_data;

  if(pdialog->pcity->is_building_unit) {
    name=get_unit_type(pdialog->pcity->currently_building)->name;
  }
  else {
    name=get_improvement_name(pdialog->pcity->currently_building);
  }
  value=build_cost(pdialog->pcity);
 
  if(game.player_ptr->economic.gold>=value) {
    sprintf(buf, "Buy %s for %d gold?\nTreasure %d gold.", 
	    name, value, game.player_ptr->economic.gold);
    popup_message_dialog(pdialog->shell, "buydialog", buf,
			 buy_callback_yes, pdialog,
			 buy_callback_no, 0, 0);
  }
  else {
    sprintf(buf, "%s costs %d gold.\nTreasure %d gold.", 
	    name, value, game.player_ptr->economic.gold);
    popup_message_dialog(pdialog->shell, "buynodialog", buf,
			 buy_callback_no, 0, 0);
  }
  
}


/****************************************************************
...
*****************************************************************/
void change_to_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  XawListReturnStruct *ret;

  pdialog=(struct city_dialog *)client_data;

  ret=XawListShowCurrent(pdialog->change_list);

  if(ret->list_index!=XAW_LIST_NONE) {
    struct packet_city_request packet;
  
    packet.city_id=pdialog->pcity->id;
    packet.name[0]='\0';
    packet.build_id=pdialog->change_list_ids[ret->list_index];
    packet.is_build_id_unit_id=
      (ret->list_index >= pdialog->change_list_no_improvements);
    
    send_packet_city_request(&aconnection, &packet, PACKET_CITY_CHANGE);
  }
  
  XtDestroyWidget(XtParent(XtParent(w)));
  XtSetSensitive(pdialog->shell, TRUE);
}

/****************************************************************
...
*****************************************************************/
void change_no_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  
  pdialog=(struct city_dialog *)client_data;
  
  XtDestroyWidget(XtParent(XtParent(w)));
  XtSetSensitive(pdialog->shell, TRUE);
}


/****************************************************************
...
*****************************************************************/
void change_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget cshell, cform, clabel, cview, button_change, button_cancel;
  Position x, y;
  Dimension width, height;
  struct city_dialog *pdialog;
  int i, n;
  
  pdialog=(struct city_dialog *)client_data;
  
  cshell=XtCreatePopupShell("changedialog", transientShellWidgetClass,
			    pdialog->shell, NULL, 0);
  
  cform=XtVaCreateManagedWidget("dform", formWidgetClass, cshell, NULL);
  
  clabel=XtVaCreateManagedWidget("dlabel", labelWidgetClass, cform, 
				 NULL);   

  cview=XtVaCreateManagedWidget("dview", viewportWidgetClass,
				cform,
				XtNfromVert, 
				clabel,
				NULL);

  pdialog->change_list=XtVaCreateManagedWidget("dlist", listWidgetClass, 
					       cview, 
					       XtNforceColumns, 1,
					       XtNdefaultColumns,1, 
					       XtNlist, 
					       (XtArgVal)dummy_change_list,
					       XtNverticalList, False,
					       NULL);

  
  button_change=XtVaCreateManagedWidget("buttonchange",
					commandWidgetClass,
					cform,
					XtNfromVert, 
					cview,
					NULL);

  button_cancel=XtVaCreateManagedWidget("buttoncancel",
				    commandWidgetClass,
				    cform,
				    XtNfromVert, 
				    cview,
				    XtNfromHoriz,
				    button_change,
				    NULL);

  

  XtAddCallback(button_change, XtNcallback, 
		change_to_callback, (XtPointer)pdialog);
  XtAddCallback(button_cancel, XtNcallback, 
		change_no_callback, (XtPointer)pdialog);
  

  XtVaGetValues(pdialog->shell, XtNwidth, &width, XtNheight, &height, NULL);
  XtTranslateCoords(pdialog->shell, (Position) width/3, (Position) height/3,
		    &x, &y);
  XtVaSetValues(cshell, XtNx, x, XtNy, y, NULL);
  
  XtPopup(cshell, XtGrabNone);
  
  XtSetSensitive(pdialog->shell, FALSE);

  for(i=0, n=0; i<B_LAST; i++)
    if(can_build_improvement(pdialog->pcity, i)) {
      sprintf(pdialog->change_list_names[n], "%s (%d)", get_improvement_name(i),get_improvement_type(i)->build_cost);
      
      pdialog->change_list_names_ptrs[n]=pdialog->change_list_names[n];
      pdialog->change_list_ids[n++]=i;
    }
  
  pdialog->change_list_no_improvements=n;


  for(i=0; i<U_LAST; i++)
    if(can_build_unit(pdialog->pcity, i)) {
      sprintf(pdialog->change_list_names[n],"%s (%d)", get_unit_type(i)->name, get_unit_type(i)->build_cost);
      pdialog->change_list_names_ptrs[n]=pdialog->change_list_names[n];
      pdialog->change_list_ids[n++]=i;
    }
  
  pdialog->change_list_names_ptrs[n]=0;

  XawListChange(pdialog->change_list, pdialog->change_list_names_ptrs, 
		0, 0, False);
  /* force refresh of viewport so the scrollbar is added.
   * Buggy sun athena requires this */
  XtVaSetValues(cview, XtNforceBars, True, NULL);
}


/****************************************************************
...
*****************************************************************/
void sell_callback_yes(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  struct packet_city_request packet;

  pdialog=(struct city_dialog *)client_data;

  packet.city_id=pdialog->pcity->id;
  packet.build_id=pdialog->sell_id;
  packet.name[0]='\0';
  send_packet_city_request(&aconnection, &packet, PACKET_CITY_SELL);

  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void sell_callback_no(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  
  pdialog=(struct city_dialog *)client_data;

  destroy_message_dialog(w);
}



/****************************************************************
...
*****************************************************************/
void sell_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  struct city_dialog *pdialog;
  XawListReturnStruct *ret;
  
  pdialog=(struct city_dialog *)client_data;

  ret=XawListShowCurrent(pdialog->improvement_list);

  if(ret->list_index!=XAW_LIST_NONE) {
    int i, n;
    for(i=0, n=0; i<B_LAST; i++)
      if(pdialog->pcity->improvements[i]) {
	if(n==ret->list_index) {
	  char buf[512];
	  
	  if(is_wonder(i))
	    return;
	  
	  pdialog->sell_id=i;
	  sprintf(buf, "Sell %s for %d gold?", 
		  get_improvement_name(i),
		  building_value(i));

	  popup_message_dialog(pdialog->shell, "selldialog", buf,
			       sell_callback_yes, pdialog,
			       sell_callback_no, pdialog, 0);
	  
	  return;
	}
	n++;
      }
  }
}

/****************************************************************
...
*****************************************************************/
void close_city_dialog(struct city_dialog *pdialog)
{
  XtDestroyWidget(pdialog->shell);
  genlist_unlink(&dialog_list, pdialog);

  if(pdialog->is_modal)
    XtSetSensitive(toplevel, TRUE);
  free(pdialog);
}

/****************************************************************
...
*****************************************************************/
void close_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  close_city_dialog((struct city_dialog *)client_data);
}
