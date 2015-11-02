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
#ifndef __MENU_H
#define __MENU_H

#include <X11/Intrinsic.h>

void setup_menues(Widget parent_form);

struct Menu {
  Widget button, shell;
};

struct Menu *ctor_menu(char *names[], 
		       void (*menucallback)(Widget, XtPointer, XtPointer),
		       struct Menu *menu_to_the_left, 
		       Widget parent);
void dtor_menu(struct Menu *mymenu);

#endif

