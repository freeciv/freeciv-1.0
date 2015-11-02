/* $XConsortium: Command.c,v 1.79 94/04/17 20:11:58 kaleb Exp $ */

/***********************************************************

Copyright (c) 1987, 1988, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*
 * Command.c - Command button widget
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Misc.h>
#include <X11/Xaw/XawInit.h>
#include <X11/Xmu/Converters.h>
#include <X11/extensions/shape.h>
#include "pixcomm.h"
#include "pixcommp.h"
#include "xstuff.h"

extern GC civ_gc;
extern GC fill_bg_gc;
extern Display	*display;
extern Window root_window;
extern int display_depth;


#define DEFAULT_HIGHLIGHT_THICKNESS 2
#define DEFAULT_SHAPE_HIGHLIGHT 32767

/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

/* Private Data */

static char defaultTranslations[] =
    "<EnterWindow>:	highlight()		\n\
     <LeaveWindow>:	reset()			\n\
     <Btn1Down>:	set()			\n\
     <Btn1Up>:		notify() unset()	";

#define offset(field) XtOffsetOf(CommandRec, field)
static XtResource resources[] = { 
   {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer), 
      offset(command.callbacks), XtRCallback, (XtPointer)NULL},
   {XtNhighlightThickness, XtCThickness, XtRDimension, sizeof(Dimension),
      offset(command.highlight_thickness), XtRImmediate,
      (XtPointer) DEFAULT_SHAPE_HIGHLIGHT},
   {XtNshapeStyle, XtCShapeStyle, XtRShapeStyle, sizeof(int),
      offset(command.shape_style), XtRImmediate, (XtPointer)XawShapeRectangle},
   {XtNcornerRoundPercent, XtCCornerRoundPercent, 
	XtRDimension, sizeof(Dimension),
	offset(command.corner_round), XtRImmediate, (XtPointer) 25}
};
#undef offset

static Boolean SetValues();
static void Initialize(), Redisplay();
static void Destroy();
static void ClassInitialize();
static void Realize(), Resize();

#define SuperClass ((CommandWidgetClass)&commandClassRec)

PixcommClassRec pixcommClassRec = {
  {
    (WidgetClass) SuperClass,		/* superclass		  */	
    "Command",				/* class_name		  */
    sizeof(CommandRec),			/* size			  */
    ClassInitialize,			/* class_initialize	  */
    NULL,				/* class_part_initialize  */
    FALSE,				/* class_inited		  */
    Initialize,				/* initialize		  */
    NULL,				/* initialize_hook	  */
    Realize,				/* realize		  */
    NULL, /*actionsList,*/			/* actions		  */
    0, /*XtNumber(actionsList),*/		/* num_actions		  */
    resources,				/* resources		  */
    XtNumber(resources),		/* resource_count	  */
    NULLQUARK,				/* xrm_class		  */
    FALSE,				/* compress_motion	  */
    TRUE,				/* compress_exposure	  */
    TRUE,				/* compress_enterleave    */
    FALSE,				/* visible_interest	  */
    Destroy,				/* destroy		  */
    Resize,				/* resize		  */
    Redisplay,				/* expose		  */
    SetValues,				/* set_values		  */
    NULL,				/* set_values_hook	  */
    XtInheritSetValuesAlmost,		/* set_values_almost	  */
    NULL,				/* get_values_hook	  */
    NULL,				/* accept_focus		  */
    XtVersion,				/* version		  */
    NULL,				/* callback_private	  */
    defaultTranslations,		/* tm_table		  */
    XtInheritQueryGeometry,		/* query_geometry	  */
    XtInheritDisplayAccelerator,	/* display_accelerator	  */
    NULL				/* extension		  */
  },  /* CoreClass fields initialization */
  {
    XtInheritChangeSensitive		/* change_sensitive	*/
  },  /* SimpleClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* LabelClass fields initialization */
  {
    0,                                     /* field not used    */
  },  /* CommandClass fields initialization */
  {
    0,
  }
};

  /* for public consumption */
WidgetClass pixcommWidgetClass = (WidgetClass) &pixcommClassRec;

/****************************************************************
 *
 * Private Procedures
 *
 ****************************************************************/

/* ARGSUSED */
static void 
Initialize(request, new, args, num_args)
Widget request, new;
ArgList args;			/* unused */
Cardinal *num_args;		/* unused */
{
  PixcommWidget pw = (PixcommWidget) new;
  pw->pixcomm.back_store=XCreatePixmap(display, root_window, 
				       pw->core.width, pw->core.height, 
				       display_depth);
  XSetForeground(display, fill_bg_gc, pw->core.background_pixel);
  XFillRectangle(display, pw->pixcomm.back_store, fill_bg_gc, 
		 0, 0, pw->core.width, pw->core.height);
  XtVaSetValues(new, XtNbitmap, pw->pixcomm.back_store, NULL);
}



/*
 * Repaint the widget window
 */

/************************
*
*  REDISPLAY (DRAW)
*
************************/

/* ARGSUSED */
static void 
Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
  if(!XtIsRealized(w))
    return;
  (*SuperClass->core_class.expose) (w, event, region);
}

static void 
Destroy(w)
Widget w;
{
  PixcommWidget pw=(PixcommWidget)w;
  XFreePixmap(display, pw->pixcomm.back_store);
}

/*
 * Set specified arguments into widget
 */

/* ARGSUSED */
static Boolean 
SetValues (current, request, new, args, num_args)
Widget current, request, new;
ArgList args;
Cardinal *num_args;
{
  return TRUE;
}

static void ClassInitialize()
{
  XawInitializeWidgetSet();
}

static void Realize(w, valueMask, attributes)
    Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
  (*pixcommWidgetClass->core_class.superclass->core_class.realize)
	(w, valueMask, attributes);
}

static void Resize(w)
    Widget w;
{
  PixcommWidget pw=(PixcommWidget)w;

  if(!XtIsRealized(w))
    return;
  
  if(pw->pixcomm.back_store)
    XFreePixmap(display, pw->pixcomm.back_store);

  pw->pixcomm.back_store=XCreatePixmap(display, root_window, 
				       pw->core.width, pw->core.height, 
				       display_depth);

  XSetForeground(display, fill_bg_gc, pw->core.background_pixel);
  XFillRectangle(display, pw->pixcomm.back_store, fill_bg_gc, 
		 0, 0, pw->core.width, pw->core.height);
  XtVaSetValues(w, XtNbitmap, pw->pixcomm.back_store, NULL);
  (*pixcommWidgetClass->core_class.superclass->core_class.resize)(w);
}


Pixmap XawPixcommPixmap(Widget w)
{
  PixcommWidget	pw = (PixcommWidget)w;

  return XtIsRealized(w) ? pw->pixcomm.back_store : 0;
}

void XawPixcommCopyTo(Widget w, Pixmap src)
{
  PixcommWidget pw = (PixcommWidget)w;
 
  XCopyArea(display, src, pw->pixcomm.back_store, 
	    civ_gc,
	    0, 0,
	    pw->core.width, pw->core.height,
	    0, 0);
  xaw_expose_now(w);
}

void XawPixcommClear(Widget w)
{
  PixcommWidget pw = (PixcommWidget)w;

  XSetForeground(display, fill_bg_gc, pw->core.background_pixel);
  XFillRectangle(display, pw->pixcomm.back_store, fill_bg_gc, 
		 0, 0, pw->core.width, pw->core.height);
  xaw_expose_now(w);
}

