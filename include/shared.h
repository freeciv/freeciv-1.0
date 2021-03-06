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
#ifndef __SHARED_H
#define __SHARED_H

#define FREECIV_NAME_VERSION "Freeciv version 1.0a"

#define MAX_PLAYERS 14
#define MAX_LENGTH_NAME 32
#define MAX_LENGTH_ADDRESS 32

#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#define MIN(x,y) (((x)<(y))?(x):(y))
#endif
char *create_centered_string(char *s);

char *get_option_text(char **argv, int *argcnt, int max_argcnt,
		      char short_option, char *long_option);
char *int_to_text(int nr);
char *get_sane_name(char *name);
char *textyear(int year);
char *get_dot_separated_int(unsigned val);
char *mystrdup(char *);
char *minstrdup(char *);
int mystrcasecmp(char *str0, char *str1);

#endif
