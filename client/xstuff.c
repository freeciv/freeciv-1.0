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
#include <string.h>

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>


void xaw_expose_now(Widget w);

/**************************************************************************
...
**************************************************************************/
void xaw_set_relative_position(Widget ref, Widget w, int px, int py)
{
  Position x, y;
  Dimension width, height;
  
  XtVaGetValues(ref, XtNwidth, &width, XtNheight, &height, NULL);
  XtTranslateCoords(ref, (Position) px*width/100, (Position) py*height/100, &x, &y);
  XtVaSetValues(w, XtNx, x, XtNy, y, NULL);
}


/**************************************************************************
...
**************************************************************************/
void xaw_horiz_center(Widget w)
{
  Dimension width, width2;

  XtVaGetValues(XtParent(w), XtNwidth, &width, NULL);
  XtVaGetValues(w, XtNwidth, &width2, NULL);
  XtVaSetValues(w, XtNhorizDistance, (width-width2)/2, NULL);
}

/**************************************************************************
...
**************************************************************************/
void xaw_set_bitmap(Widget w, Pixmap pm)
{
  XtVaSetValues(w, XtNbitmap, (XtArgVal)pm, NULL);
  xaw_expose_now(w);
}


/**************************************************************************
...
**************************************************************************/
void xaw_set_label(Widget w, char *text)
{
  String str;

  XtVaGetValues(w, XtNlabel, &str, NULL);
  if(strcmp(str, text)) {
    XtVaSetValues(w, XtNlabel, (XtArgVal)text, NULL);
    xaw_expose_now(w);
  }
  
}

/**************************************************************************
...
**************************************************************************/
void xaw_expose_now(Widget w)
{
  Dimension width, height;
  XExposeEvent xeev;
  
  xeev.type = Expose;
  xeev.display = XtDisplay (w);
  xeev.window = XtWindow(w);
  xeev.x = 0;
  xeev.y = 0;
  XtVaGetValues(w, XtNwidth, &width, XtNheight, &height, NULL);
  (XtClass(w))->core_class.expose(w, (XEvent *)&xeev, NULL);
}

/**************************************************************************
...
**************************************************************************/
void x_simulate_button_click(Widget w)
{
  XButtonEvent ev;
  
  ev.display = XtDisplay(w);
  ev.window = XtWindow(w);
  ev.type=ButtonPress;
  ev.button=Button1;
  ev.x=10;
  ev.y=10;

  XtDispatchEvent((XEvent *)&ev);
  ev.type=ButtonRelease;
  XtDispatchEvent((XEvent *)&ev);
}
