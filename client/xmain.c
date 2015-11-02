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
#include <X11/bitmaps/xlogo16>

#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include "pixcomm.h"

#include "xmain.h"
#include "canvas.h"
#include "menu.h"
#include "colors.h"
#include "log.h"
#include "graphics.h"
#include "map.h"
#include "mapview.h"
#include "chatline.h"
#include "civclient.h"
#include "clinet.h"
#include "mapctrl.h"
#include "citydlg.h"
#include "freeciv.ico"
#include "inputdlg.h"
#include "dialogs.h"
#include "game.h"
#include "diplodlg.h"
#include "resources.h"


AppResources appResources;

extern String fallback_resources[];

/**************************************************************************
...
**************************************************************************/
XtResource resources[] = {
    { "usingFallback", "UsingFallback", XtRBoolean, sizeof(Boolean),
      XtOffset(AppResources *,usingFallback), XtRImmediate, (XtPointer)False},
};


extern int sound_bell_at_new_turn;
void timer_callback(caddr_t client_data, XtIntervalId *id);

struct game_data {
  int year;
  int population;
  int researchedpoints;
  int money;
  int tax, luxurary, research;
};

/**************************************************************************/
Display	*display;
int display_depth;
int screen_number;
enum Display_color_type display_color_type;
XtAppContext app_context;
extern Colormap cmap;

/* this GC will be the default one all thru freeciv */
GC civ_gc; 

/* this GC will be the default one all thru freeciv */
GC civ_gc_1plane; 

/* and this one is used for filling with the bg color */
GC fill_bg_gc;

Widget toplevel, main_form, menu_form, below_menu_form, left_column_form;
Widget bottom_form;
Widget map_form;
Widget map_canvas;
Widget overview_canvas;
Widget map_vertical_scrollbar, map_horizontal_scrollbar;
Widget inputline_text, outputwindow_text;
Widget turn_done_button;
Widget info_label, bulp_label, sun_label, goverment_label, timeout_label;
Widget unit_info_label;
Widget unit_pix_canvas;
Widget unit_below_canvas[4];
Pixmap unit_below_pixmap[4];
Widget more_arrow_label;
Window root_window;

/* this pixmap acts as a backing store for the map_canvas widget */
Pixmap map_canvas_store;
int map_canvas_store_twidth, map_canvas_store_theight;

/* this pixmap acts as a backing store for the overview_canvas widget */
Pixmap overview_canvas_store;
int overview_canvas_store_width, overview_canvas_store_height;

/* this pixmap is used when moving units etc */
Pixmap single_tile_pixmap;
int single_tile_pixmap_width, single_tile_pixmap_height;

extern int seconds_to_turndone;

XtInputId x_input_id;
XtIntervalId x_interval_id;

XtActionsRec Actions[] = {
  { "select-mapcanvas", butt_down_mapcanvas},
  { "select-overviewcanvas", butt_down_overviewcanvas},
  { "focus-to-next-unit", focus_to_next_unit },
  { "center-on-unit", center_on_unit },
  { "inputline-return", inputline_return },
  { "input-dialog-returnkey", input_dialog_returnkey},
  { "races-dialog-returnkey", races_dialog_returnkey},
  { "diplo-dialog-returnkey", diplo_dialog_returnkey},
  { "key-unit-irrigate", key_unit_irrigate },
  { "key-unit-road", key_unit_road },
  { "key-unit-mine", key_unit_mine },
  { "key-unit-homecity", key_unit_homecity },
  { "key-unit-clean-polution", key_unit_clean_polution },
  { "key-unit-pillage", key_unit_pillage },
  { "key-unit-disband", key_unit_disband },
  { "key-unit-fortify", key_unit_fortify },
  { "key-unit-goto", key_unit_goto },
  { "key-unit-sentry", key_unit_sentry },
  { "key-unit-wait", key_unit_wait },
  { "key-unit-done", key_unit_done },
  { "key-unit-north", key_unit_north },
  { "key-unit-north-east", key_unit_north_east },
  { "key-unit-east", key_unit_east },
  { "key-unit-south-east", key_unit_south_east },
  { "key-unit-south", key_unit_south },
  { "key-unit-south-west", key_unit_south_west },
  { "key-unit-west", key_unit_west },
  { "key-unit-north-west", key_unit_north_west },
  { "key-unit-build-city", key_unit_build_city },
  { "key-end-turn", key_end_turn },
  { "select-citymap", button_down_citymap}
};

int myerr(Display *p, XErrorEvent *e)
{
  puts("error");
  return 0;
}

/**************************************************************************
...
**************************************************************************/
void x_main(int argc, char *argv[])
{
  int i;
  Pixmap icon_pixmap; 
  XtTranslations TextFieldTranslations;

  /* include later - pain to see the warning at every run */
  /* XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL); */

  toplevel = XtVaAppInitialize(
	       &app_context,       /* Application context */
	       "Freeciv",          /* application class name */
	       NULL, 0,            /* command line option list */
	       &argc, argv,        /* command line args */
	       fallback_resources, /* for missing app-defaults file */
	       XtNallowShellResize, True,
	       NULL);              /* terminate varargs list */

  XtGetApplicationResources(toplevel, &appResources, resources,
                            XtNumber(resources), NULL, 0);
  
  display = XtDisplay(toplevel);
  screen_number=XScreenNumberOfScreen(XtScreen(toplevel));
  display_depth=DefaultDepth(display, screen_number);
  root_window=DefaultRootWindow(display);

  if(appResources.usingFallback) {
    log(LOG_NORMAL, "Application defaults file wasn't found. Using fallback resources");
  }
  
/*  XSynchronize(display, 1); 
  XSetErrorHandler(myerr);*/
 
  display_color_type=get_visual(); 
  if(display_color_type!=COLOR_DISPLAY) {
    log(LOG_FATAL, "only color displays are supported for now...");
    /*    exit(1); */
  }
  
  icon_pixmap = XCreateBitmapFromData(display,
				      RootWindowOfScreen(XtScreen(toplevel)),
				      freeciv_bits,
				      freeciv_width, freeciv_height );
  XtVaSetValues(toplevel, XtNiconPixmap, icon_pixmap, NULL);

  civ_gc = XCreateGC(display, root_window, 0, NULL);

  {
    XGCValues values;
    values.foreground = 0;
    values.background = 0;
    fill_bg_gc= XCreateGC(display, root_window, 
			  GCForeground | GCBackground, &values);
  }

  init_color_system();
  
  load_intro_gfx(); 
  load_tile_gfx();
  
  setup_widgets();
  
  XtSetKeyboardFocus(bottom_form, inputline_text);
  XtSetKeyboardFocus(below_menu_form, map_canvas);
  
  TextFieldTranslations = XtParseTranslationTable  /*BLAH!*/
		("<Key>Return: inputline-return()");
  XtOverrideTranslations(inputline_text, TextFieldTranslations);

  XtAppAddActions(app_context, Actions, XtNumber(Actions));

  XtAddCallback(map_horizontal_scrollbar, XtNjumpProc, 
		scrollbar_jump_callback, NULL);
  XtAddCallback(map_vertical_scrollbar, XtNjumpProc, 
		scrollbar_jump_callback, NULL);
  XtAddCallback(map_horizontal_scrollbar, XtNscrollProc, 
		scrollbar_scroll_callback, NULL);
  XtAddCallback(map_vertical_scrollbar, XtNscrollProc, 
		scrollbar_scroll_callback, NULL);
  XtAddCallback(turn_done_button, XtNcallback, end_turn_callback, NULL);

  XtRealizeWidget(toplevel);

  x_input_id=XtAppAddInput(app_context, aconnection.sock, 
			   (XtPointer) XtInputReadMask,
			   (XtInputCallbackProc) get_net_input, NULL);

  x_interval_id=XtAppAddTimeOut(app_context, 500,
				(XtTimerCallbackProc)timer_callback, NULL);

  map_canvas_store=XCreatePixmap(display, XtWindow(map_canvas), 
				 510, 300, display_depth);
  map_canvas_store_twidth=510/30;
  map_canvas_store_theight=300/30;


  overview_canvas_store_width=2*80;
  overview_canvas_store_height=2*50;

  overview_canvas_store=XCreatePixmap(display, XtWindow(overview_canvas), 
				      overview_canvas_store_width,
				      overview_canvas_store_height, 
				      display_depth);

  XSetForeground(display, fill_bg_gc, colors_standard[COLOR_STD_WHITE]);
  XFillRectangle(display, overview_canvas_store, fill_bg_gc, 0, 0, 
		 overview_canvas_store_width, overview_canvas_store_height);

  
  single_tile_pixmap_width=30;
  single_tile_pixmap_height=30;

  
  single_tile_pixmap=XCreatePixmap(display, XtWindow(overview_canvas), 
				   single_tile_pixmap_width,
				   single_tile_pixmap_height,
				   display_depth);


  for(i=0; i<4; i++)
    unit_below_pixmap[i]=XCreatePixmap(display, XtWindow(overview_canvas), 
				       30, 30, display_depth);  
  
  set_bulp_sol_goverment(0, 0, 0);
  XtAppMainLoop(app_context);
}


/**************************************************************************
...
**************************************************************************/
void setup_widgets(void)
{


  main_form = XtVaCreateManagedWidget("mainform", formWidgetClass, 
				      toplevel, 
				      NULL);   

  menu_form = XtVaCreateManagedWidget("menuform", formWidgetClass,	
				      main_form,        
				      NULL);	        
  setup_menues(menu_form); 

  below_menu_form = XtVaCreateManagedWidget("belowmenuform", 
					    formWidgetClass, 
					    main_form,
					    NULL);

  left_column_form = XtVaCreateManagedWidget("leftcolumnform", 
					     formWidgetClass, 
					     below_menu_form, 
					     NULL);

  map_form = XtVaCreateManagedWidget("mapform", 
				     formWidgetClass, 
				     below_menu_form, 
				     NULL);

  bottom_form = XtVaCreateManagedWidget("bottomform", 
					formWidgetClass, 
					main_form, 
					NULL);
  
  overview_canvas = XtVaCreateManagedWidget("overviewcanvas", 
					    xfwfcanvasWidgetClass,
					    left_column_form,
					    "exposeProc", 
					    (XtArgVal)overview_canvas_expose,
					    "exposeProcData", 
					    (XtArgVal)NULL,
					    NULL);
  
  info_label = XtVaCreateManagedWidget("infolabel", 
				       labelWidgetClass, 
				       left_column_form, 
				       XtNfromVert, 
				       (XtArgVal)overview_canvas,
				       NULL);   

  turn_done_button = XtVaCreateManagedWidget("turndonebutton", 
					     commandWidgetClass,
					     left_column_form,
					     NULL);
  
  bulp_label = XtVaCreateManagedWidget("bulplabel", 
				       labelWidgetClass,
				       left_column_form,
				       NULL);
  
  sun_label = XtVaCreateManagedWidget("sunlabel", 
				      labelWidgetClass, 
				      left_column_form,
				      NULL);

  goverment_label = XtVaCreateManagedWidget("govermentlabel", 
					    labelWidgetClass, 
					    left_column_form,
					    NULL);

  timeout_label = XtVaCreateManagedWidget("timeoutlabel", 
					  labelWidgetClass, 
					  left_column_form,
					  NULL);

  
  
  unit_info_label = XtVaCreateManagedWidget("unitinfolabel", 
					    labelWidgetClass, 
					    left_column_form, 
					    NULL);

  unit_pix_canvas = XtVaCreateManagedWidget("unitpixcanvas", 
					   pixcommWidgetClass,
					   left_column_form, 
					   NULL);

  unit_below_canvas[0] = XtVaCreateManagedWidget("unitbelowcanvas0",
					        pixcommWidgetClass,
						left_column_form, NULL);
  unit_below_canvas[1] = XtVaCreateManagedWidget("unitbelowcanvas1",
					        pixcommWidgetClass,
						left_column_form, NULL);
  unit_below_canvas[2] = XtVaCreateManagedWidget("unitbelowcanvas2",
					        pixcommWidgetClass,
						left_column_form, NULL);
  unit_below_canvas[3] = XtVaCreateManagedWidget("unitbelowcanvas3",
					        pixcommWidgetClass,
						left_column_form, NULL);

  more_arrow_label = XtVaCreateManagedWidget("morearrowlabel", 
					     labelWidgetClass, 
					     left_column_form,
					     NULL);

  map_vertical_scrollbar = XtVaCreateManagedWidget("mapvertiscrbar", 
						   scrollbarWidgetClass, 
						   map_form,
						   NULL);

  map_canvas = XtVaCreateManagedWidget("mapcanvas", 
				       xfwfcanvasWidgetClass,
				       map_form,
				       "exposeProc", 
				       (XtArgVal)map_canvas_expose,
				       "exposeProcData", 
				       (XtArgVal)NULL,
				       NULL);

  map_horizontal_scrollbar = XtVaCreateManagedWidget("maphorizscrbar", 
						     scrollbarWidgetClass, 
						     map_form,
						     NULL);



  outputwindow_text= XtVaCreateManagedWidget("outputwindowtext", 
					     asciiTextWidgetClass, 
					     bottom_form,
					     NULL);


  inputline_text= XtVaCreateManagedWidget("inputlinetext", 
					  asciiTextWidgetClass, 
					  bottom_form,
					  NULL);

}

/**************************************************************************
...
**************************************************************************/
void quit_civ(Widget w, XtPointer client_data, XtPointer call_data)
{ 
  exit(0);
}


/**************************************************************************
...
**************************************************************************/
void remove_net_input(void)
{
  XtRemoveInput(x_input_id);
}


/**************************************************************************
...
**************************************************************************/
void enable_turn_done_button(void)
{
  XtSetSensitive(turn_done_button, TRUE);

  if(sound_bell_at_new_turn)
    XBell(display, 100);
}


/**************************************************************************
...
**************************************************************************/
void end_turn_callback(Widget w, XtPointer client_data, XtPointer call_data)
{ 
  XtSetSensitive(turn_done_button, FALSE);

  user_ended_turn();
}



/**************************************************************************
...
**************************************************************************/
void timer_callback(caddr_t client_data, XtIntervalId *id)
{
  static int flip;
  
  x_interval_id=XtAppAddTimeOut(app_context, 500,
				(XtTimerCallbackProc)timer_callback, NULL);  
  
  if(game.player_ptr->is_connected && game.player_ptr->is_alive && 
     !game.player_ptr->turn_done) { 
    int i, is_waiting, is_moving;
    
    for(i=0, is_waiting=0, is_moving=0; i<game.nplayers; i++)
      if(game.players[i].is_alive && game.players[i].is_connected) {
	if(game.players[i].turn_done)
	  is_waiting++;
	else
	  is_moving++;
      }
    
    if(is_moving==1 && is_waiting) 
      update_turn_done_button(0);  /* stress the slow player! */
  }
  
  blink_active_unit();

  if(flip) {
    update_timeout_label();
    if(seconds_to_turndone)
      seconds_to_turndone--;
  }
  
  flip=!flip;
}

