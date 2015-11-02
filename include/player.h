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
#ifndef __PLAYER_H
#define __PLAYER_H

#include "registry.h"
#include "tech.h"
#include "genlist.h"
#include "unit.h"
#include "city.h"

#define PLAYER_DEFAULT_TAX_RATE 50
#define PLAYER_DEFAULT_SCIENCE_RATE 50
#define PLAYER_DEFAULT_LUXURY_RATE 0

enum goverment_type { 
  G_ANARCHY, G_DESPOTISM, G_MONARCHY, G_COMMUNISM, G_REPUBLIC, G_DEMOCRACY,
  G_LAST
};

enum race_type {
  R_ROMAN, R_BABYLONIAN, R_GERMAN, R_EGYPTIAN, R_AMERICAN, R_GREEK, R_INDIAN, 
  R_RUSSIAN, R_ZULU, R_FRENCH, R_AZTEC, R_CHINESE, R_ENGLISH, R_MONGOL, R_LAST
};

struct player_race {
  char name[MAX_LENGTH_NAME];
  char name_plural[MAX_LENGTH_NAME];
  int aggressive;
  int expanding;
  int paranoid;
};

struct player_economic {
  int gold;
  int tax;
  int science;
  int luxury;
};

struct player_research {
  int researched;     /* # bulps reseached */
  int researchpoints; /* # bulps to complete */
  int researching;    /* invention being researched in */
  unsigned char inventions[A_LAST];
};

struct player_score {
  int happy;
  int content;
  int unhappy;
  int taxmen;
  int scientists;
  int elvis;
  int wonders;
  int techs;
  int techout;
  int landmass;
  int cities;
  int units;
  int polution;
  int literacy;
  int bnp;
  int mfg;
};


struct player {
  int player_no;
  char name[MAX_LENGTH_NAME];
  enum goverment_type goverment;
  enum race_type race;
  int turn_done;
  int nturns_idle;
  int is_alive;
  int got_tech;
  int revolution;
  int capital; /* bool used to give player capital in first city. */
  int embassy;
  struct unit_list units;
  struct city_list cities;
  struct player_score score;
  struct player_economic economic;
  struct player_research research;
  int is_connected;
  struct connection *conn;
  char addr[MAX_LENGTH_ADDRESS];
};

void player_load(struct player *plr, int plrno, struct section_file *file);
void player_save(struct player *plr, int plrno, struct section_file *file);
void player_init(struct player *plr);
struct player *find_player_by_name(char *name);
void player_set_unit_focus_status(struct player *pplayer);
int player_has_embassy(struct player *pplayer, struct player *pplayer2);

int can_change_to_goverment(struct player *pplayer, 
			    enum goverment_type);
char *get_goverment_name(enum goverment_type type);
char *get_ruler_title(enum goverment_type type);
char *get_race_name(enum race_type race);
char *get_race_name_plural(enum race_type race);

extern struct player_race races[];

#endif
