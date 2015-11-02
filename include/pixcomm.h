/*

    Canvas.h - public header file for the Canvas Widget
    -  a widget that allows programmer-specified refresh procedures.
    Copyright (C) 1990,93,94 Robert H. Forsman Jr.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#ifndef _Pixcomm_h
#define _Pixcomm_h

#include <X11/Xaw/Command.h>

#ifndef XtNBackPixmap
#define XtNBackPixmap		"backPixmap"
#define XtCBackPixmap		"BackPixmap"
#endif


extern WidgetClass pixcommWidgetClass;

typedef struct _PixcommClassRec *PixcommWidgetClass;
typedef struct _PixcommRec      *PixcommWidget;

Pixmap XawPixcommPixmap(Widget w);
void XawPixcommCopyTo(Widget w, Pixmap src);
void XawPixcommClear(Widget w);
#endif
