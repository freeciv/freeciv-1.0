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
#include "game.h"
#include "player.h"
#include "map.h"
#include "mapgen.h"
#include "city.h"

struct civ_game game;

/*
struct player_score {
  int happy;
  int content;
  int unhappy;
  int taxmen;
  int scientists;
  int elvis;
  int wonders;
  int techs;
  int landmass;
  int cities;
  int units;
  int polution;
  int literacy;
  int bnp;
  int mfg;
};


*/


int research_time(struct player *pplayer)
{
  int timemod=(game.year>0) ? 2:1;
  return timemod*pplayer->research.researchpoints*game.techlevel;
}

/**************************************************************************
...
**************************************************************************/

int total_player_citizens(struct player *pplayer)
{
  return (pplayer->score.happy
	  +pplayer->score.unhappy
	  +pplayer->score.content
	  +pplayer->score.scientists
	  +pplayer->score.elvis
	  +pplayer->score.taxmen);
}

/**************************************************************************
...
**************************************************************************/
int civ_score(struct player *pplayer)
{
  int i;
  struct genlist_iterator myiter;
  struct city *pcity;
  genlist_iterator_init(&myiter, &pplayer->cities.list, 0);
  pplayer->score.happy=0;                       /* done */
  pplayer->score.content=0;                     /* done */   
  pplayer->score.unhappy=0;                     /* done */
  pplayer->score.taxmen=0;                      /* done */
  pplayer->score.scientists=0;                  /* done */
  pplayer->score.elvis=0;                       /* done */ 
  pplayer->score.wonders=0;                     /* done */
  pplayer->score.techs=0;                       /* done */
  pplayer->score.techout=0;                     /* done */
  pplayer->score.landmass=0;
  pplayer->score.cities=0;                      /* done */
  pplayer->score.units=0;                       /* done */
  pplayer->score.polution=0;                    /* done */
  pplayer->score.bnp=0;                         /* done */
  pplayer->score.mfg=0;                         /* done */
  pplayer->score.literacy=0;
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    pcity=(struct city *)ITERATOR_PTR(myiter);
    pplayer->score.happy+=pcity->ppl_happy[4];
    pplayer->score.content+=pcity->ppl_content[4];
    pplayer->score.unhappy+=pcity->ppl_unhappy[4];

    pplayer->score.taxmen+=pcity->ppl_taxman;
    pplayer->score.scientists+=pcity->ppl_scientist;
    pplayer->score.elvis=pcity->ppl_elvis;
    pplayer->score.cities++;
    pplayer->score.polution+=pcity->polution;
    pplayer->score.techout+=(1+pcity->science_total);
    pplayer->score.bnp+=pcity->trade_prod;
    pplayer->score.mfg+=pcity->shield_surplus;
    if (city_got_building(pcity, B_UNIVERSITY)) 
      pplayer->score.literacy+=city_population(pcity);
    else if (city_got_building(pcity,B_LIBRARY))
      pplayer->score.literacy+=(city_population(pcity)/2);
  }
  for (i=0;i<A_LAST;i++) 
    if (get_invention(pplayer, i)==TECH_KNOWN) 
      pplayer->score.techs++;
 
  genlist_iterator_init(&myiter, &pplayer->units.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct unit *punit=(struct unit *)ITERATOR_PTR(myiter);
    if (is_military_unit(punit)) pplayer->score.units++;
  }
  
  for (i=0;i<B_LAST;i++) {
    if (is_wonder(i) && (pcity=find_city_by_id(game.global_wonders[i])) && 
	player_owns_city(pplayer, pcity))
      pplayer->score.wonders++;
  }
  return (total_player_citizens(pplayer)+pplayer->score.happy+pplayer->score.techs*2+pplayer->score.wonders*5);
}

/**************************************************************************
Count the # of citizen in a civilisation.
**************************************************************************/
int civ_population(struct player *pplayer)
{
  int ppl=0;
  struct genlist_iterator myiter;
  struct city *pcity;
  genlist_iterator_init(&myiter, &pplayer->cities.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    pcity=(struct city *)ITERATOR_PTR(myiter);
    ppl+=city_population(pcity);
  }
  return ppl;
}


/**************************************************************************
...
**************************************************************************/
struct city *game_find_city_by_coor(int x, int y)
{
  int i;
  struct city *pcity;

  for(i=0; i<game.nplayers; i++)
    if((pcity=city_list_find_coor(&game.players[i].cities, x, y)))
      return pcity;

  return 0;
}

/**************************************************************************
...
**************************************************************************/
struct city *game_find_city_by_id(int city_id)
{
  int i;
  struct city *pcity;

  for(i=0; i<game.nplayers; i++)
    if((pcity=city_list_find_id(&game.players[i].cities, city_id)))
      return pcity;

  return 0;
}

/**************************************************************************
...
**************************************************************************/
struct city *game_find_city_by_name(char *name)
{
  int i;
  struct city *pcity;

  for(i=0; i<game.nplayers; i++)
    if((pcity=city_list_find_name(&game.players[i].cities, name)))
      return pcity;

  return 0;
}



/**************************************************************************
...
**************************************************************************/
struct unit *game_find_unit_by_id(int unit_id)
{
  int i;
  struct unit *punit;

  for(i=0; i<game.nplayers; i++)
    if((punit=unit_list_find(&game.players[i].units, unit_id)))  
      return punit;

  return 0;
}



/**************************************************************************
...
**************************************************************************/
void game_remove_unit(int unit_id)
{
  struct unit *punit;
  
  if((punit=game_find_unit_by_id(unit_id))) {
    struct city *pcity;

    pcity=city_list_find_id(&game.players[punit->owner].cities, 
			    punit->homecity);
    if(pcity)
      unit_list_unlink(&pcity->units_supported, punit);

    unit_list_unlink(&map_get_tile(punit->x, punit->y)->units, punit);
    unit_list_unlink(&game.players[punit->owner].units, punit);
    
    free(punit);
  }
}

/**************************************************************************
...
**************************************************************************/
void game_remove_city(int city_id)
{
  struct city *pcity;
  
  if((pcity=game_find_city_by_id(city_id))) {
    city_list_unlink(&game.players[pcity->owner].cities, pcity);
    map_set_city(pcity->x, pcity->y, 0);
    free(pcity);
  }

}

/***************************************************************
...
***************************************************************/
void game_init(void)
{
  int i;
  game.globalwarming=0;
  game.warminglevel=8;
  game.gold=GAME_DEFAULT_GOLD;
  game.tech=GAME_DEFAULT_TECHLEVEL;
  game.skill_level=0;
  game.timeout=GAME_DEFAULT_TIMEOUT;
  game.end_year=GAME_DEFAULT_END_YEAR;
  game.year=-4000;
  game.min_players=GAME_DEFAULT_MIN_PLAYERS;
  game.max_players=GAME_DEFAULT_MAX_PLAYERS;
  game.nplayers=0;
  game.techlevel=GAME_DEFAULT_RESEARCHLEVEL;
  game.settlers=GAME_DEFAULT_SETTLERS;
  game.unhappysize=GAME_DEFAULT_UNHAPPYSIZE;
  game.heating=0;
  strcpy(game.save_name, "civgame");
  game.save_nturns=10;
  
  map_init();
  
  for(i=0; i<MAX_PLAYERS; i++)
    player_init(&game.players[i]);
  for (i=0; i<A_LAST; i++) 
    game.global_advances[i]=0;
  for (i=0; i<B_LAST; i++)
    game.global_wonders[i]=0;
  game.player_idx=0;
  game.player_ptr=&game.players[0];
}

void initialize_globals()
{
  int i,j;
  struct player *plr;
  struct city *pcity=NULL;
  struct genlist_iterator myiter;
  for (j=0;j<game.nplayers;j++) {
    plr=&game.players[j];
    for (i=0;i<A_LAST; i++) {
      if (get_invention(plr, i)==TECH_KNOWN)
	game.global_advances[i]++;
    }
    
    genlist_iterator_init(&myiter, &plr->cities.list, 0);
    for (; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
      pcity=(struct city *)ITERATOR_PTR(myiter);
      for (i=0;i<B_LAST;i++) {
	if (city_got_building(pcity, i) && is_wonder(i))
	  game.global_wonders[i]=pcity->id;
      }
    }
  }
}

/***************************************************************
...
***************************************************************/
void game_next_year(void)
{
  if(game.year<1000)
    game.year+=20;
  else if(game.year<1500)
    game.year+=10;
  else if (game.year<1700)
    game.year+=5;
  else if (game.year<1800)
    game.year+=2;
  else
    game.year++;
}


