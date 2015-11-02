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
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "player.h"
#include "unit.h"
#include "city.h"
#include "map.h"
#include "shared.h"
#include "tech.h"

struct player_race races[]= {
  {"Roman","Romans",0,0,0},
  {"Babylonian","Babylonians",0,0,0},
  {"German","Germans",0,0,0},
  {"Egyptian","Egyptians",0,0,0},
  {"American","Americans",0,0,0},
  {"Greek","Greeks",0,0,0},
  {"Indian","Indians",0,0,0},
  {"Russian","Russians",0,0,0},
  {"Zulu","Zulus",0,0,0},
  {"French","French",0,0,0},
  {"Aztec","Aztecs",0,0,0},
  {"Chinese","Chinese",0,0,0},
  {"English","English",0,0,0},
  {"Mongol","Mongols",0,0,0}
};


char *goverment_names[G_LAST] = {
  "Anarchy", "Despotism", "Monarchy",
  "Communism", "Republic", "Democracy"
};

char *ruler_titles[G_LAST] = {
  "Mr.", "Emperor", "King",
  "Comrade", "President", "President"
};



/***************************************************************
...
***************************************************************/
int player_has_embassy(struct player *pplayer, struct player *pplayer2)
{
  return pplayer->embassy & (1<<pplayer2->player_no);
}

/***************************************************************
...
***************************************************************/
char *get_race_name(enum race_type race)
{
  return races[race].name;
}

/***************************************************************
...
***************************************************************/
char *get_race_name_plural(enum race_type race)
{
  return races[race].name_plural;
}


/***************************************************************
...
***************************************************************/
char *get_ruler_title(enum goverment_type type)
{
  return ruler_titles[type];
}


/***************************************************************
...
***************************************************************/
char *get_goverment_name(enum goverment_type type)
{
  return goverment_names[type];
}



/***************************************************************
...
***************************************************************/
int can_change_to_goverment(struct player *pplayer, enum goverment_type gov)
{
  struct city *pcity;

  if (gov>=G_LAST)
    return 0;

  switch (gov) {
  case G_ANARCHY:
  case G_DESPOTISM: 
    return 1;
    break;
  case G_MONARCHY:
    if (get_invention(pplayer, A_MONARCHY)==TECH_KNOWN)
      return 1;
    break;
  case G_COMMUNISM:
    if (get_invention(pplayer, A_COMMUNISM)==TECH_KNOWN)
      return 1;
    break;
  case G_REPUBLIC:
    if (get_invention(pplayer, A_REPUBLIC)==TECH_KNOWN)
      return 1;
    break;
  case G_DEMOCRACY:
    if (get_invention(pplayer, A_DEMOCRACY)==TECH_KNOWN)
        return 1;
    break;
  default:
    return 0;
    break;
  }

  pcity=find_city_by_id(game.global_wonders[B_PYRAMIDS]);

  if (!pcity) 
    return 0;
  return (player_owns_city(pplayer, pcity));
}


/***************************************************************
...
***************************************************************/
void player_init(struct player *plr)
{
  plr->player_no=plr-game.players;

  strcpy(plr->name, "YourName");
  plr->goverment=G_DESPOTISM;
  plr->race=R_LAST;
  plr->capital=0;
  unit_list_init(&plr->units);
  city_list_init(&plr->cities);
  /*  init_tech(plr, 4); */
  strcpy(plr->addr, "---.---.---.---");
  plr->is_alive=1;
  plr->embassy=0;
   
  plr->economic.tax=PLAYER_DEFAULT_TAX_RATE;
  plr->economic.science=PLAYER_DEFAULT_SCIENCE_RATE;
  plr->economic.luxury=PLAYER_DEFAULT_LUXURY_RATE;
}


/***************************************************************
...
***************************************************************/
void player_set_unit_focus_status(struct player *pplayer)
{
  struct genlist_iterator myiter;
  
  genlist_iterator_init(&myiter, &pplayer->units.list, 0);

  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    ((struct unit *)ITERATOR_PTR(myiter))->focus_status=FOCUS_AVAIL;
}

/***************************************************************
...
***************************************************************/
struct player *find_player_by_name(char *name)
{
  int i;

  for(i=0; i<game.nplayers; i++)
     if(!mystrcasecmp(name, game.players[i].name))
	return &game.players[i];

  return 0;
}






