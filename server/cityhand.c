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

#include "player.h"
#include "unithand.h"
#include "civserver.h"
#include "map.h"
#include "maphand.h"
#include "mapgen.h"
#include "cityhand.h"
#include "unit.h"
#include "city.h"
#include "player.h"
#include "tech.h"
#include "shared.h"
#include "plrhand.h"

void update_city_activity(struct player *pplayer, struct city *pcity);
extern struct unit_type unit_types[];
extern struct improvement_type improvement_types[];
void auto_arrange_workers(struct city *pcity);
void city_check_workers(struct player *pplayer, struct city *pcity);
long lrand48(void);


/**************************************************************************
...
**************************************************************************/
void create_city(struct player *pplayer, int x, int y, char *name)
{
  struct city *pcity;
  int i;
  
  pcity=(struct city *)malloc(sizeof(struct city));

  pcity->id=get_next_id_number();
  pcity->owner=pplayer->player_no;
  pcity->x=x;
  pcity->y=y;
  strcpy(pcity->name, name);
  pcity->size=1;
  pcity->ppl_elvis=1;
  pcity->ppl_scientist=pcity->ppl_taxman=0;
  pcity->ppl_happy[4]=0;
  pcity->ppl_content[4]=1;
  pcity->ppl_unhappy[4]=0;
  pcity->was_happy=0;
  for (i=0;i<4;i++)
    pcity->trade[i]=0;
  pcity->food_stock=0;
  pcity->shield_stock=0;
  pcity->trade_prod=0;
  pcity->is_building_unit=1;
  pcity->did_buy=1;
  if (can_build_unit(pcity, U_RIFLEMEN))
      pcity->currently_building=U_RIFLEMEN;
  else if (can_build_unit(pcity, U_MUSKETEERS))
      pcity->currently_building=U_MUSKETEERS;
  else if (can_build_unit(pcity, U_PHALANX))
      pcity->currently_building=U_PHALANX;
  else
      pcity->currently_building=U_MILITIA;

  for(y=0; y<CITY_MAP_SIZE; y++)
    for(x=0; x<CITY_MAP_SIZE; x++)
      pcity->city_map[x][y]=C_TILE_EMPTY;

  for(i=0; i<B_LAST; i++)
    pcity->improvements[i]=0;
  if(!pplayer->capital) {
    pplayer->capital=1;
    pcity->improvements[B_PALACE]=1;
  }
  pcity->anarchy=0;
  map_set_city(pcity->x, pcity->y, pcity);
  
  unit_list_init(&pcity->units_supported);
  city_list_insert(&pplayer->cities, pcity);

  city_check_workers(pplayer, pcity);
  auto_arrange_workers(pcity);

  city_refresh(pcity);
  city_incite_cost(pcity);
  send_city_info(0, pcity, 0);
}

/**************************************************************************
...
**************************************************************************/

void set_food_trade_shields(struct city *pcity)
{
  int x,y;
  int trade;
  pcity->food_prod=0;
  pcity->shield_prod=0;
  pcity->trade_prod=0;

  pcity->food_surplus=0;
  pcity->shield_surplus=0;
  pcity->corruption=0;

  for(y=0;y<5;y++) {
    for(x=0;x<5;x++) 
      if(get_worker_city(pcity, x, y)==C_TILE_WORKER) {
	pcity->food_prod+=get_food_tile(x, y, pcity);
	pcity->shield_prod+=get_shields_tile(x, y, pcity);
	trade=get_trade_tile(x, y, pcity);
	if(trade && city_affected_by_wonder(pcity, B_COLLOSSUS)) 
	  trade++;
	pcity->trade_prod+=trade;
      }
  }
  
  pcity->food_surplus=pcity->food_prod-pcity->size*2;
  set_trade_prod(pcity);
}

/**************************************************************************
...
**************************************************************************/
int city_refresh(struct city *pcity)
{
  set_food_trade_shields(pcity);
  citizen_happy_size(pcity);
  set_tax_income(pcity);                  /* calc base luxury, tax & bulps */
  add_buildings_effect(pcity);            /* marketplace, library wonders.. */
  set_polution(pcity);
  citizen_happy_luxury(pcity);            /* with our new found luxuries */
  citizen_happy_buildings(pcity);         /* temple cathedral colosseum */
  city_support(pcity);                    /* manage settlers, and units */
  citizen_happy_wonders(pcity);           /* happy wonders */
  unhappy_city_check(pcity);
  return (is_city_happy(pcity) && pcity->was_happy);
}

/**************************************************************************
...
**************************************************************************/
void city_settlersupport(struct city *pcity)
{
  struct genlist_iterator myiter;
  struct unit *this_unit;
  genlist_iterator_init(&myiter, &pcity->units_supported.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    this_unit=(struct unit*)ITERATOR_PTR(myiter);
    if (this_unit->type == U_SETTLERS) {
      pcity->food_surplus--;
      this_unit->upkeep=1;
      if (get_goverment(pcity->owner)>=G_REPUBLIC) {
	pcity->food_surplus--;
	this_unit->upkeep=2;
      }
    }
  }
}

void city_support(struct city *pcity)
{ 
  struct genlist_iterator myiter;
  int milunits=0;
  int unit=0;
  struct unit *this_unit;
  int unhap=0;
  int gov=get_goverment(pcity->owner);
  happy_copy(pcity, 2);
  city_settlersupport(pcity);
  
  genlist_iterator_init(&myiter, &pcity->units_supported.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    unit++;
    this_unit=(struct unit*)ITERATOR_PTR(myiter);
    this_unit->unhappiness=0;
    if (this_unit->type !=U_SETTLERS)
      this_unit->upkeep=0;
    if (is_military_unit(this_unit)) {
      milunits++;
      switch (gov) {
      case G_ANARCHY:
      case G_DESPOTISM:
	if (pcity->ppl_unhappy[3] && this_unit->x == pcity->x &&  
	    this_unit->y == pcity->y) {
	  pcity->ppl_unhappy[3]--;
	  pcity->ppl_content[3]++;
	}
	if (milunits>pcity->size) {
	  pcity->shield_surplus--;
	  this_unit->upkeep=1;
	} 
	break;
      case G_COMMUNISM:
      case G_MONARCHY:
	if (milunits>3) {
	  pcity->shield_surplus--;
	  this_unit->upkeep=1;
	} else {
	  if (pcity->ppl_unhappy[3] && this_unit->x == pcity->x &&  
	      this_unit->y == pcity->y) {
	    pcity->ppl_unhappy[3]--;
	    pcity->ppl_content[3]++;
	  }
	}
	break;
      case G_REPUBLIC:
	pcity->shield_surplus--;
	this_unit->upkeep=1;
	if(get_unit_type(this_unit->type)->attack_strength && 
	    !map_get_city(this_unit->x ,this_unit->y )) {
	  if (unhap)
	    this_unit->unhappiness=1;
	  unhap++;
	}
	if (is_field_unit(this_unit)) {
	  unhap++;
	  this_unit->unhappiness++;
	}
	break;
      case G_DEMOCRACY:
	pcity->shield_surplus--;
	this_unit->upkeep=1;
	if (get_unit_type(this_unit->type)->attack_strength &&
	    !map_get_city(this_unit->x, this_unit->y)) {
	  unhap+=2;
	  this_unit->unhappiness=2;
	} else	if (is_field_unit(this_unit)) {
	  this_unit->unhappiness=1;
	  unhap+=1;
	}
	break;
      default:
	break;
      }
    } 
  }
  if (gov==G_REPUBLIC) 
    unhap--;
  citizen_happy_units(pcity, unhap);
}

/**************************************************************************
...
**************************************************************************/
void update_polution()
{
  int x,y,count=0;
  
  for (x=0;x<map.xsize;x++) 
    for (y=0;y<map.ysize;y++) 
      if (map_get_special(x,y)&S_POLUTION) {
	count++;
      }
  game.heating=count;
  game.globalwarming+=count;
  if (game.globalwarming<game.warminglevel) 
    game.globalwarming=0;
  else {
    game.globalwarming-=game.warminglevel;
    if (lrand48()%200<=game.globalwarming) {
      fprintf(stderr, "Global warming:%d\n", count);
      global_warming(map.xsize/10+map.ysize/10+game.globalwarming*5);
      game.globalwarming=0;
      send_all_known_tiles(0);
      notify_player(0, "Game: Global warming has occurred! Coastlines have been flooded\nand vast ranges of grassland have become deserts.");
      game.globalwarming+=2;
    }
  }
    return;
}

/**************************************************************************
...
**************************************************************************/
void handle_city_change_specialist(struct player *pplayer, 
				   struct packet_city_request *preq)
{
  struct city *pcity;
  pcity=find_city_by_id(preq->city_id);
  if(!pcity) 
    return;
  if(!player_owns_city(pplayer, pcity))  
    return;
  if(preq->specialist_from==SP_ELVIS) {
    if(pcity->size<5) 
      return; 

    if(!pcity->ppl_elvis)
      return;
    pcity->ppl_elvis--;
  } else if(preq->specialist_from==SP_TAXMAN) {
    if (!pcity->ppl_taxman)
      return;
    pcity->ppl_taxman--;
  } else if (preq->specialist_from==SP_SCIENTIST) {
    if (!pcity->ppl_scientist)
      return;
    pcity->ppl_scientist--;
  } else {
    return;
  }
  switch (preq->specialist_to) {
  case SP_ELVIS:
    pcity->ppl_elvis++;
    break;
  case SP_TAXMAN:
    pcity->ppl_taxman++;
    break;
  case SP_SCIENTIST:
    pcity->ppl_scientist++;
    break;
  default:
    pcity->ppl_elvis++;
    break;
  }

  city_refresh(pcity);
  send_city_info(pplayer, pcity, 0);
}

/**************************************************************************
...
**************************************************************************/
void remove_obsolete_buildings(struct player *pplayer)
{
  int i;
  struct genlist_iterator myiter;
  struct city *pcity;
  genlist_iterator_init(&myiter, &pplayer->cities.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    pcity=(struct city *)ITERATOR_PTR(myiter);
    for (i=0;i<B_LAST;i++) {
      if(city_got_building(pcity, i) 
	 && !is_wonder(i) 
	   && building_obsolete(pplayer, i)) {
	do_sell_building(pplayer, pcity, i);
	notify_player(pplayer, "Game: %s is selling %s (obsolete) for %d", pcity->name, building_name(i), building_value(i)/2);
      }
      if (city_got_building(pcity, B_BANK) && 
	  !city_got_building(pcity, B_MARKETPLACE)) {
	do_sell_building(pplayer, pcity, B_BANK);
	notify_player(pplayer, "Game: %s doesn't have a Marketplace, selling Bank for %d", pcity->name,  building_value(B_BANK)/2);
      }
      if (city_got_building(pcity, B_UNIVERSITY) && 
	  !city_got_building(pcity, B_LIBRARY)) {
	do_sell_building(pplayer, pcity, B_UNIVERSITY);
	notify_player(pplayer, "Game: %s doesn't have a Library, selling University for %d", pcity->name,  building_value(B_UNIVERSITY)/2);
      }
      if (city_got_building(pcity, B_MFG) &&
	  !city_got_building(pcity, B_FACTORY)) {
	do_sell_building(pplayer, pcity, B_MFG);
	notify_player(pplayer, "Game: %s doesn't have a Factory, selling MFG Plant for %d", pcity->name,  building_value(B_MFG)/2);
      }
    }
  }
}

/**************************************************************************
...
**************************************************************************/
void handle_city_make_specialist(struct player *pplayer, 
				 struct packet_city_request *preq)
{
  struct city *pcity;

  pcity=find_city_by_id(preq->city_id);
  if(!pcity) 
    return;
  if (!player_owns_city(pplayer, pcity))  return;
  if (preq->worker_x==2 && preq->worker_y==2) {
    auto_arrange_workers(pcity);
    return;
  }
  if (is_worker_here(pcity, preq->worker_x, preq->worker_y)) {
    set_worker_city(pcity, preq->worker_x, preq->worker_y, C_TILE_EMPTY);
    pcity->ppl_elvis++;

    city_refresh(pcity);
    send_city_info(pplayer, pcity, 0);
  }
}


/**************************************************************************
...
**************************************************************************/
int is_worked_here(int x, int y)
{
  struct player *pplayer;
  struct genlist_iterator myiter;
  struct city *pcity;
  int mx,my,i;
  for(i=0; i<game.nplayers; i++) {
    pplayer=&game.players[i];
    genlist_iterator_init(&myiter, &pplayer->cities.list, 0);
    
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
      pcity=(struct city *)ITERATOR_PTR(myiter);
      mx=x+2-pcity->x;
      my=y+2-pcity->y;
      if(!(mx<0 || mx>=5 ||my<0 ||my>=5)) {
	if(get_worker_city(pcity, mx, my)==C_TILE_WORKER) return 1;
      }
    }
  }
  return 0;
}

/**************************************************************************
x and y are city cords in the range [0;4]
**************************************************************************/
int can_place_worker_here(struct city *pcity, int x, int y)
{
  return !(x==0 && y==0) && 
    !(x==0 && y==CITY_MAP_SIZE-1) &&
    !(x==CITY_MAP_SIZE-1 && y==0) &&
    !(x==CITY_MAP_SIZE-1 && y==CITY_MAP_SIZE-1) &&
    map_get_known(pcity->x+x-2, pcity->y+y-2, city_owner(pcity))
    && !is_worked_here(pcity->x+x-2, pcity->y+y-2);
}


/**************************************************************************
...
**************************************************************************/
void handle_city_make_worker(struct player *pplayer, 
			     struct packet_city_request *preq)
{
  struct city *pcity;
  pcity=find_city_by_id(preq->city_id);

  if(!pcity) 
    return;
  
  if(!player_owns_city(pplayer, pcity))
    return;

  if(preq->worker_x==2 && preq->worker_y==2) {
    auto_arrange_workers(pcity);
  }

  if(!nr_specialists(pcity)) 
    return;
  
  if(!can_place_worker_here(pcity, preq->worker_x, preq->worker_y))
    return;

  set_worker_city(pcity, preq->worker_x, preq->worker_y, C_TILE_WORKER);

  if(pcity->ppl_elvis) 
    pcity->ppl_elvis--;
  else if(pcity->ppl_scientist) 
    pcity->ppl_scientist--;
  else 
    pcity->ppl_taxman--;
  
  city_refresh(pcity);
  send_city_info(pplayer, pcity, 1);
}

/**************************************************************************
...
**************************************************************************/
void do_sell_building(struct player *pplayer, struct city *pcity, int id)
{
  pcity->improvements[id]=0;
  pplayer->economic.gold+=building_value(id);
}

/**************************************************************************
...
**************************************************************************/
void handle_city_sell(struct player *pplayer, struct packet_city_request *preq)
{
  struct city *pcity;
  pcity=find_city_by_id(preq->city_id);
  if (!pcity || !player_owns_city(pplayer, pcity) 
      || preq->build_id>=B_LAST) 
    return;
  
  if (pcity->did_sell) {
    notify_player(pplayer, "Game: You have already sold something here this turn.");
    return;
  }
 if (!can_sell_building(pcity, preq->build_id))
   return;

  pcity->did_sell=1;
  notify_player(pplayer, "Game: You sell %s for %d credits.", 
		building_name(preq->build_id), 
		building_value(preq->build_id));
  do_sell_building(pplayer, pcity, preq->build_id);

  city_refresh(pcity);
  send_city_info(pplayer, pcity, 1);
  send_player_info(pplayer, pplayer);
}
/**************************************************************************
...
**************************************************************************/
int best_food_tile(struct city *pcity, int x, int y, int bx, int by)
{
  if (get_food_tile(x, y, pcity)>get_food_tile(bx, by, pcity))
    return 1;
  if (get_food_tile(x, y, pcity)<get_food_tile(bx, by, pcity))
    return 0;
  return (3*get_shields_tile(x, y, pcity) + 2*get_trade_tile(x, y, pcity)
	  > 2*get_trade_tile(bx, by, pcity) + 3*get_shields_tile(bx, by, pcity));
}
/**************************************************************************
...
**************************************************************************/
int  add_adjust_workers(struct city *pcity)
{
  int workers=pcity->size;
  int iswork=0;
  int toplace;
  int x,y,bx,by;
  for (y=0;y<5;y++)
    for (x=0;x<5;x++) {
      if (get_worker_city(pcity, x, y)==C_TILE_WORKER) 
	iswork++;
    }
  iswork--;
  if (iswork+nr_specialists(pcity)>workers)
    return 0;
  if (iswork+nr_specialists(pcity)==workers)
    return 1;
  toplace=workers-(iswork+nr_specialists(pcity));

  do {
    bx=0;
    by=0;
    for(y=0;y<5;y++)
      for(x=0;x<5;x++) {
	if(can_place_worker_here(pcity, x, y) && get_worker_city(pcity, x,y)!=C_TILE_WORKER) {
	  if(bx==0 && by==0) {
	    bx=x;
	    by=y;
	  } else {
	    if(best_food_tile(pcity, x, y,bx, by)) {
	      bx=x;
	      by=y;
	    }
	  }
	}
      }
    if(bx || by) {
      set_worker_city(pcity, bx, by, C_TILE_WORKER);
      toplace--;
    }
  } while(toplace && (bx || by));
  pcity->ppl_elvis+=toplace;
  return 1;
}

void auto_arrange_workers(struct city *pcity)
{
  int workers=pcity->size;
  int taxwanted,sciwanted;
  int bx,by;
  int x,y;
  for (y=0;y<5;y++)
    for (x=0;x<5;x++) {
      if (get_worker_city(pcity, x, y)==C_TILE_WORKER) 
	set_worker_city(pcity, x, y, C_TILE_EMPTY);
      
    }
  set_worker_city(pcity, 2, 2, C_TILE_WORKER); 
  
  do {
    bx=0;
    by=0;
    for(y=0;y<5;y++)
      for(x=0;x<5;x++) {
	if(can_place_worker_here(pcity, x, y)) {
	  if(bx==0 && by==0) {
	    bx=x;
	    by=y;
	  } else {
	    if(best_food_tile(pcity, x, y,bx, by)) {
	      bx=x;
	      by=y;
	    }
	  }
	}
      }
    if(bx || by) {
      set_worker_city(pcity, bx, by, C_TILE_WORKER);
      workers--;
    }
  } while(workers && (bx || by));

  taxwanted=pcity->ppl_taxman;
  sciwanted=pcity->ppl_scientist;
  pcity->ppl_taxman=0;
  pcity->ppl_scientist=0;
  while (workers && (taxwanted ||sciwanted)) {
    if (taxwanted) {
      workers--;
      pcity->ppl_taxman++;
      taxwanted--;
    } 
    if (sciwanted && workers) {
      workers--;
      pcity->ppl_scientist++;
      sciwanted--;
    }
  }
  pcity->ppl_elvis=workers;

  city_refresh(pcity);
  send_city_info(city_owner(pcity), pcity, 1);
}

/**************************************************************************
...
**************************************************************************/
void handle_city_buy(struct player *pplayer, struct packet_city_request *preq)
{
  struct city *pcity;
  char *name;
  int cost, total, build;
  pcity=find_city_by_id(preq->city_id);
  if (!pcity || !player_owns_city(pplayer, pcity)) return;
 
  if (pcity->did_buy) {
    notify_player(pplayer, "Game: You have already bought this turn.");
    return;
  }
  build=pcity->shield_stock;
 
  if (!pcity->is_building_unit) {
    total=building_value(pcity->currently_building);
    name=improvement_types[pcity->currently_building].name;
    
  } else {
    name=unit_types[pcity->currently_building].name;
    total=unit_value(pcity->currently_building);
    if (pcity->anarchy) {
      notify_player(pplayer, 
		    "Game: Can't buy units when city is in disorder.");
    }

  }
  cost=build_cost(pcity);
   if (cost>pplayer->economic.gold)
    return;
  pcity->did_buy=1;
  pplayer->economic.gold-=cost;
  pcity->shield_stock=total;
  notify_player(pplayer, "Game: %s bought for %d", name, cost); 
  
  city_refresh(pcity);
  send_city_info(pplayer, pcity, 1);
  send_player_info(pplayer,pplayer);
}

/**************************************************************************
...
**************************************************************************/
void handle_city_change(struct player *pplayer, 
			struct packet_city_request *preq)
{
  struct city *pcity;
  pcity=find_city_by_id(preq->city_id);
   if(!player_owns_city(pplayer, pcity))
    return;
   if (preq->is_build_id_unit_id && !can_build_unit(pcity, preq->build_id))
     return;
   if (!preq->is_build_id_unit_id && !can_build_improvement(pcity, preq->build_id))
     return;
  if (pcity->did_buy && pcity->shield_stock) {
    notify_player(pplayer, "Game: You have bought this turn, can't change.");
    return;
  }

   if(!pcity->is_building_unit && is_wonder(pcity->currently_building)) {
     notify_player(0, "Game: The %s have stopped building The %s in %s.",
		   get_race_name_plural(pplayer->race),
		   get_improvement_name(pcity->currently_building),
		   pcity->name);
   }
  
  if(preq->is_build_id_unit_id) {
    if (!pcity->is_building_unit)
      pcity->shield_stock/=2;
      pcity->currently_building=preq->build_id;
      pcity->is_building_unit=1;
  }
  else {
    if (pcity->is_building_unit ||(is_wonder(pcity->currently_building)!=is_wonder(preq->build_id)))
      pcity->shield_stock/=2;
    
    pcity->currently_building=preq->build_id;
    pcity->is_building_unit=0;
    
    if(is_wonder(preq->build_id)) {
      notify_player(0, "Game: The %s have started building The %s.",
		    get_race_name_plural(pplayer->race),
		    get_improvement_name(pcity->currently_building));
    }
  }
  
  city_refresh(pcity);
  send_city_info(pplayer, pcity, 1);
}

/**************************************************************************
...
**************************************************************************/
void handle_city_rename(struct player *pplayer, 
			struct packet_city_request *preq)
{
  char *cp;
  struct city *pcity;
  
  pcity=find_city_by_id(preq->city_id);

  if(!player_owns_city(pplayer, pcity))
    return;

  if((cp=get_sane_name(preq->name))) {
    /* more sanity tests! any existing city with that name? */
    strcpy(pcity->name, cp);
    city_refresh(pcity);
    send_city_info(pplayer, pcity, 1);
  }
  else
    notify_player(pplayer, "Game: %s is not a valid name.", preq->name);
}

/**************************************************************************
...
**************************************************************************/
void update_city_activities(struct player *pplayer)
{
  struct genlist_iterator myiter;
  int gold;
  genlist_iterator_init(&myiter, &pplayer->cities.list, 0);
  gold=pplayer->economic.gold;
  pplayer->got_tech=0;
  for(; ITERATOR_PTR(myiter); ) {
    struct city *pcity=(struct city *)ITERATOR_PTR(myiter);
    ITERATOR_NEXT(myiter);
    update_city_activity(pplayer, pcity);
  }
  if (gold-(gold-pplayer->economic.gold)*3<0) {
    notify_player(pplayer, "Game: Warning, we're low on funds sire.");  
  }
}

/**************************************************************************
...
**************************************************************************/
void city_auto_remove_worker(struct city *pcity)
{
  if(pcity->size<1) {      
    remove_city(pcity);
    return;
  }
  if(nr_specialists(pcity)) {
    if(pcity->ppl_taxman) {
      pcity->ppl_taxman--;
      return;
    } else if(pcity->ppl_scientist) {
      pcity->ppl_scientist--;
      return;
    } else if(pcity->ppl_elvis) {
      pcity->ppl_elvis--; 
      return;
    }
  } 
  auto_arrange_workers(pcity);
  city_refresh(pcity);
}

/**************************************************************************
...
**************************************************************************/
void city_increase_size(struct city *pcity)
{
  if (city_got_building(pcity, B_GRANARY)) { 
    pcity->food_stock=(pcity->size+1)*5;
  }
  else
    pcity->food_stock=0;
  if (!city_got_building(pcity, B_AQUEDUCT) && pcity->size>=10) {/* need aquaduct */
    notify_player(city_owner(pcity),"Game: %s needs Aquaducts to grow any further", pcity->name);
    return;
  }
  pcity->size++;
  if (!add_adjust_workers(pcity))
    auto_arrange_workers(pcity);
}
/**************************************************************************
...
**************************************************************************/
void city_reduce_size(struct city *pcity)
{
  pcity->food_stock=0;
  pcity->size--;
  notify_player(city_owner(pcity), "Game: Famine feared in %s", pcity->name);
  
  pcity->food_stock=pcity->size*5;
  city_auto_remove_worker(pcity);
}
 
/**************************************************************************
...
**************************************************************************/
void city_populate(struct city *pcity)
{
  pcity->food_stock+=pcity->food_surplus;
  if(pcity->food_stock>pcity->size*10) 
    city_increase_size(pcity);
  else if(pcity->food_stock<0) {
    struct unit *punit;
    struct genlist_iterator myiter;
    genlist_iterator_init(&myiter, &pcity->units_supported.list, 0);
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
      punit=(struct unit *)ITERATOR_PTR(myiter);
      if (punit->type==U_SETTLERS) {
	send_remove_unit(0, punit->id);
	game_remove_unit(punit->id);
	notify_player(city_owner(pcity), "Game: Famine feared in %s, Settlers dies!", pcity->name);
	pcity->food_stock=0;
	return;
      }
    }
    city_reduce_size(pcity);
  }
}
void eval_buildings( struct city *pcity,int *values)
{
  int i;
  for (i=0;i<B_LAST;i++) {
    if (is_wonder(i) && can_build_improvement(pcity, i)) {
      if (wonder_is_obsolete(i))
	values[i]=1;
      else
	values[i]=99;
    } else
    values[i]=0;
  }
  
  if (can_build_improvement(pcity, B_GRANARY)) 
    values[B_GRANARY]=pcity->food_surplus*200;
  if (can_build_improvement(pcity, B_HARBOUR)) {
    values[B_HARBOUR]=pcity->size*60;
  }

  if (can_build_improvement(pcity, B_MARKETPLACE)) 
    values[B_MARKETPLACE]=pcity->trade_prod*100;
  if (can_build_improvement(pcity, B_BANK)) 
    values[B_BANK]=pcity->tax_total*101;
  if (can_build_improvement(pcity, B_LIBRARY)) 
    values[B_LIBRARY]=pcity->science_total*100;
  if (can_build_improvement(pcity, B_UNIVERSITY)) 
    values[B_UNIVERSITY]=pcity->science_total*100;
  if (can_build_improvement(pcity, B_BARRACKS))
    values[B_BARRACKS]=pcity->shield_prod*75;
  if (can_build_improvement(pcity, B_BARRACKS2))
    values[B_BARRACKS2]=pcity->shield_prod*75;
  if (can_build_improvement(pcity, B_BARRACKS3))
    values[B_BARRACKS3]=pcity->shield_prod*75;
  if (can_build_improvement(pcity, B_FACTORY)) 
    values[B_FACTORY]=pcity->shield_prod*150;
  if (can_build_improvement(pcity, B_AQUEDUCT)) 
    values[B_AQUEDUCT]=pcity->size*50+pcity->food_surplus*50;
  if (can_build_improvement(pcity, B_TEMPLE))
     values[B_TEMPLE]=pcity->ppl_unhappy[4]*300+pcity->ppl_elvis*300;
  if (can_build_improvement(pcity, B_COLOSSEUM))
    values[B_COLOSSEUM]=pcity->ppl_unhappy[4]*299+pcity->ppl_elvis*200;
  if (can_build_improvement(pcity, B_CATHEDRAL))
    values[B_CATHEDRAL]=pcity->ppl_unhappy[4]*298+pcity->ppl_elvis*150;
  if (can_build_improvement(pcity, B_COASTAL))
     values[B_COASTAL]=300;
  if (can_build_improvement(pcity, B_CITY))
    values[B_CITY]=pcity->size*35;
  if (can_build_improvement(pcity, B_COURTHOUSE))
     values[B_COURTHOUSE]=pcity->corruption*100;
  if (can_build_improvement(pcity, B_HYDRO))
    values[B_HYDRO]=pcity->shield_prod*100+pcity->polution*100;
  if (can_build_improvement(pcity, B_NUCLEAR))
    values[B_NUCLEAR]=pcity->shield_prod*100+pcity->polution*100;
  if (can_build_improvement(pcity, B_NUCLEAR))
    values[B_NUCLEAR]=pcity->shield_prod*100;
  if (can_build_improvement(pcity, B_MFG)) 
    values[B_MFG]=pcity->shield_prod*150;
  if (can_build_improvement(pcity, B_MASS)) 
    values[B_MASS]=pcity->polution*100+pcity->size*50;
  if (can_build_improvement(pcity, B_RECYCLING)) 
    values[B_RECYCLING]=pcity->polution*100+pcity->shield_prod*50;
  if (can_build_improvement(pcity, B_SDI))
    values[B_SDI]=250;
  if (can_build_improvement(pcity, B_CAPITAL))
    values[B_CAPITAL]=2*pcity->shield_prod;
}

/**************************************************************************
...
**************************************************************************/
void advisor_choose_build(struct city *pcity)
{
  int i;
  int id=-1;
  int want=0;
  int values[B_LAST];
  eval_buildings(pcity, values);
  for (i=0;i<B_LAST;i++)
    if (values[i]>0) {
      if (values[i]>want) {
	want=values[i];
	id=i;
      }

    }


  if (id!=-1) {
    pcity->currently_building=id;
    pcity->is_building_unit=0;
    return;
  }
  for (i=0;i<B_LAST;i++)
    if(can_build_improvement(pcity, i)) {
      pcity->currently_building=i;
      pcity->is_building_unit=0;
      return;
    }
  return;
}
void obsolete_building_test(struct city *pcity, int b1, int b2)
{ 
  if (pcity->currently_building==b1 && 
      can_build_improvement(pcity, b2))
    pcity->currently_building=b2;
}

void upgrade_building_prod(struct city *pcity)
{
  obsolete_building_test(pcity, B_BARRACKS,B_BARRACKS3);
  obsolete_building_test(pcity, B_BARRACKS,B_BARRACKS2);
  obsolete_building_test(pcity, B_BARRACKS2,B_BARRACKS3);
}

void obsolete_unit_test(struct city *pcity, int u1,int u2)
{
  struct player *pplayer=&game.players[pcity->owner];
  if (pcity->currently_building==u1 &&
      can_build_unit(pcity, u2)) {
    pcity->currently_building=u2;
    notify_player(pplayer, 
		  "Game: production of %s is upgraded to %s in %s",
		  get_unit_type(u1)->name, 
		  get_unit_type(u2)->name , 
		  pcity->name);
  }
}

void upgrade_unit_prod(struct city *pcity)
{
  obsolete_unit_test(pcity, U_CATAPULT, U_CANNON);
  obsolete_unit_test(pcity, U_CANNON, U_ARTILLERY);
  obsolete_unit_test(pcity, U_PHALANX, U_MUSKETEERS);
  obsolete_unit_test(pcity, U_MUSKETEERS, U_RIFLEMEN);
  obsolete_unit_test(pcity, U_TRIREME, U_SAIL);
  obsolete_unit_test(pcity, U_SAIL, U_FRIGATE);
  obsolete_unit_test(pcity, U_KNIGHTS, U_ARMOR);
  obsolete_unit_test(pcity, U_LEGION, U_RIFLEMEN);
  obsolete_unit_test(pcity, U_CHARIOT, U_KNIGHTS);
  obsolete_unit_test(pcity, U_CAVALRY, U_ARMOR);
  obsolete_unit_test(pcity, U_CHARIOT, U_ARMOR);
  obsolete_unit_test(pcity, U_IRONCLAD, U_CRUISER);
  obsolete_unit_test(pcity, U_FRIGATE, U_TRANSPORT);
}

/**************************************************************************
...
**************************************************************************/
void city_build_stuff(struct player *pplayer, struct city *pcity)
{
  if (pcity->shield_surplus<0) {
    struct unit *punit;
    struct genlist_iterator myiter;
    genlist_iterator_init(&myiter, &pcity->units_supported.list, 0);
     for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
       punit=(struct unit *)ITERATOR_PTR(myiter);
       if (is_military_unit(punit)) {
	 notify_player(pplayer, "Game: %s can't upkeep %s, unit disbanded",
		       pcity->name, get_unit_type(punit->type)->name);
	 send_remove_unit(0, punit->id);
	 game_remove_unit(punit->id);
	 break;
       } 
     }
  }
  if (pcity->shield_surplus<=0) 
    pcity->shield_surplus=1;
  pcity->shield_stock+=pcity->shield_surplus;
  if (!pcity->is_building_unit) {
    if (pcity->currently_building==B_CAPITAL) {
      pplayer->economic.gold+=pcity->shield_surplus;
      pcity->shield_stock=0;
    }    
    upgrade_building_prod(pcity);
    if (!can_build_improvement(pcity, pcity->currently_building)) {
      notify_player(pplayer, "Game: %s is building %s, which is no longer available",
	pcity->name,get_improvement_name(pcity->currently_building));
      return;
    }
    if (pcity->shield_stock>=building_value(pcity->currently_building)) {
      if (pcity->currently_building==B_PALACE) {
	struct genlist_iterator myiter;
	struct city *palace;
	genlist_iterator_init(&myiter, &pplayer->cities.list, 0);
	for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
	  palace=(struct city *)ITERATOR_PTR(myiter);
	  if (city_got_building(palace, B_PALACE)) {
	    palace->improvements[B_PALACE]=0;
	    break;
	  }
	}
      }
      pcity->improvements[pcity->currently_building]=1;
      pcity->shield_stock=0;
      if(is_wonder(pcity->currently_building)) {
	game.global_wonders[pcity->currently_building]=pcity->id;
	notify_player(0, "Game: The %s have finished building %s in %s.",
		      get_race_name_plural(pplayer->race),
		      get_improvement_name(pcity->currently_building),
		      pcity->name);
      }
      notify_player(pplayer, 
		    "Game: %s has finished building %s", pcity->name, 
		    improvement_types[pcity->currently_building].name
		    );

      if (pcity->currently_building==B_DARWIN) {
	notify_player(pplayer, 
		      "Game: Darwin's Voyage boost research, you gain 2 immediate advances.");
	update_tech(pplayer, 1000000); 
	update_tech(pplayer, 1000000); 
      }
      city_refresh(pcity);
      advisor_choose_build(pcity);
      notify_player(pplayer, 
		    "Game: %s is now building %s", pcity->name, 
		    improvement_types[pcity->currently_building].name
		    );
    } 
  } else {
    upgrade_unit_prod(pcity);
    if(pcity->shield_stock>=unit_value(pcity->currently_building)) {
      if (pcity->currently_building==U_SETTLERS) {
	if (pcity->size==1) {
	  notify_player(pplayer, "Game: %s can't build settler yet", pcity->name);
	  return;
	}
	pcity->size--;
	city_auto_remove_worker(pcity);
      }

      create_unit(pplayer, pcity->x, pcity->y, pcity->currently_building,
		  city_got_barracks(pcity), pcity->id);
      pcity->shield_stock=0;
      notify_player(pplayer, "Game: %s is finished building %s", pcity->name, unit_types[pcity->currently_building].name);
    }
  }
}

/**************************************************************************
...
**************************************************************************/
void pay_for_buildings(struct player *pplayer, struct city *pcity)
{
  int i;
  for (i=0;i<B_LAST;i++) 
    if (city_got_building(pcity, i)) {
      if (pplayer->economic.gold-improvement_types[i].shield_upkeep<0) {
	notify_player(pplayer, "Game: Can't afford to maintain %s in %s, building sold!", improvement_types[i].name, pcity->name);
	do_sell_building(pplayer, pcity, i);
	city_refresh(pcity);
     } else
      pplayer->economic.gold-=improvement_types[i].shield_upkeep;
    }
}

/**************************************************************************
1) check for enemy units on citymap tiles
2) withdraw workers from such tiles
3) mark citymap tiles accordingly empty/unavailable  
**************************************************************************/
void city_check_workers(struct player *pplayer, struct city *pcity)
{
  int x, y;
  
  for(y=0; y<CITY_MAP_SIZE; y++)
    for(x=0; x<CITY_MAP_SIZE; x++) {
      struct tile *ptile=map_get_tile(pcity->x+x-2, pcity->y+y-2);
      
      if(unit_list_size(&ptile->units)>0) {
	struct unit *punit=unit_list_get(&ptile->units, 0);
	if(pplayer->player_no!=punit->owner) {
	  if(get_worker_city(pcity, x, y)==C_TILE_WORKER)
	    pcity->ppl_elvis++;
	  set_worker_city(pcity, x, y, C_TILE_UNAVAILABLE);
	  continue;
	}
      }
      if(get_worker_city(pcity, x, y)==C_TILE_UNAVAILABLE)
	set_worker_city(pcity, x, y, C_TILE_EMPTY);
      if(get_worker_city(pcity, x, y)!=C_TILE_WORKER &&
	 !can_place_worker_here(pcity, x, y))
	set_worker_city(pcity, x, y, C_TILE_UNAVAILABLE);
    }
}
/**************************************************************************
 Add some Polution if we have waste
**************************************************************************/
void check_polution(struct city *pcity)
{
  int x,y;
  int k=100;
  if (pcity->polution && lrand48()%100<=pcity->polution) {
    while (k) {
      x=pcity->x+(lrand48()%5)-2;
      y=pcity->y+(lrand48()%5)-2;
      if ( (x!=pcity->x || y!=pcity->x) && 
	   (map_get_terrain(x,y)!=T_OCEAN && map_get_terrain(x,y)<=T_TUNDRA) &&
	   (!(map_get_special(x,y)&S_POLUTION)) ) { 
	map_set_special(x,y, S_POLUTION);
	send_tile_info(0, x, y, PACKET_TILE_INFO);
	notify_player(city_owner(pcity), "Game: Polution near %s", pcity->name);
	return;
      }
      k--;
    }
  }
}
void sanity_check_city(struct city *pcity)
{
  int size=pcity->size;
  int x,y;
  int iswork=0;
  for (y=0;y<5;y++)
    for (x=0;x<5;x++) {
      if (get_worker_city(pcity, x, y)==C_TILE_WORKER) 
	iswork++;
    }
  iswork--;
  if (iswork+nr_specialists(pcity)!=size) {
    printf("%s is bugged: size:%d workers:%d elvis: %d tax:%d sci:%d\n", pcity->name,size,iswork,  pcity->ppl_elvis, pcity->ppl_taxman, pcity->ppl_scientist); 
    auto_arrange_workers(pcity);
  }
}

void city_incite_cost(struct city *pcity)
{
  struct city *capital;
  int dist;
  
  if (city_got_building(pcity, B_PALACE)) 
    pcity->incite_revolt_cost=1000000;
  else
    pcity->incite_revolt_cost=pcity->ppl_happy[4]*100+(pcity->size-pcity->ppl_unhappy[4])*100; /* fix it*/
  if (city_got_building(pcity, B_COURTHOUSE)) {
    pcity->incite_revolt_cost*=2;
  }

  pcity->incite_revolt_cost+=100*unit_list_size(&pcity->units_supported);
  pcity->incite_revolt_cost+=250;

  if (pcity->incite_revolt_cost<250) 
    pcity->incite_revolt_cost=250;
  capital=find_palace(city_owner(pcity));
  if (capital)
    dist=min(50, map_distance(capital->x, capital->y, pcity->x, pcity->y)); 
  else 
    dist=50;
  pcity->incite_revolt_cost=(pcity->incite_revolt_cost*(100-dist))/100;
  
}
/**************************************************************************
 called at the beginning of a new year. produced units etc..
**************************************************************************/
void update_city_activity(struct player *pplayer, struct city *pcity)
{
  city_check_workers(pplayer, pcity);
  if (city_refresh(pcity) && 
      get_goverment(pcity->owner)>=G_REPUBLIC &&
      pcity->food_surplus>0 && pcity->size>4) {
    pcity->food_stock=pcity->size*10+1; 
  }
  city_build_stuff(pplayer, pcity);
  if (!pcity->was_happy && is_city_happy(pcity) && pcity->size>4) {
    notify_player(pplayer, 
		  "Game: We Love The %s Day celebrated in %s", 
		  get_ruler_title(pplayer->goverment),
		  pcity->name);
  }
  if (!is_city_happy(pcity) && pcity->was_happy && pcity->size>4) {
    notify_player(pplayer,
		  "Game: We Love The %s Day canceled in %s",
		  get_ruler_title(pplayer->goverment),
		  pcity->name);

  }
  pcity->was_happy=is_city_happy(pcity);

  
    {
      int id=pcity->id;
      city_populate(pcity);
      if(!city_list_find_id(&pplayer->cities, id))
	return;
    }
     
  pcity->is_updated=1;

  pcity->did_sell=0;
  pcity->did_buy=0;
  update_tech(pplayer, pcity->science_total);
  pplayer->economic.gold+=pcity->tax_total;
  pay_for_buildings(pplayer, pcity);
  if(is_city_unhappy(pcity)) { 
    pcity->anarchy++;
    notify_player(city_owner(pcity),
		  "Game: Civil disorder in %s", pcity->name);
  }
  else {
    if (pcity->anarchy)
      notify_player(city_owner(pcity),
		    "Game: Order restored in %s", pcity->name);
    pcity->anarchy=0;
  }
  check_polution(pcity);
  city_incite_cost(pcity);

  send_city_info(0, pcity, 0);
  if (pcity->anarchy>2 && get_goverment(pcity->owner)==G_DEMOCRACY) {
    notify_player(pplayer, "Game: The people have overthrown your democracy, your country is in turmoil");
    handle_player_revolution(pplayer);
  }
  sanity_check_city(pcity);
}

/**************************************************************************
...
**************************************************************************/

void send_city_info(struct player *dest, struct city *pcity, int dosend)
{
  int i, o, x, y;
  char *p;
  struct packet_city_info packet;
/*
  printf("sending: %s to %s\n", pcity->name, dest ? dest->name : "all" );
*/  
  
  packet.id=pcity->id;
  packet.owner=pcity->owner;
  packet.x=pcity->x;
  packet.y=pcity->y;
  strcpy(packet.name, pcity->name);

  packet.size=pcity->size;
  packet.ppl_happy=pcity->ppl_happy[4];
  packet.ppl_content=pcity->ppl_content[4];
  packet.ppl_unhappy=pcity->ppl_unhappy[4];
  packet.ppl_elvis=pcity->ppl_elvis;
  packet.ppl_scientist=pcity->ppl_scientist;
  packet.ppl_taxman=pcity->ppl_taxman;
  for (i=0;i<4;i++)
    packet.trade[i]=pcity->trade[i];

  packet.food_prod=pcity->food_prod;
  packet.food_surplus=pcity->food_surplus;
  packet.shield_prod=pcity->shield_prod;
  packet.shield_surplus=pcity->shield_surplus;
  packet.trade_prod=pcity->trade_prod;
  packet.corruption=pcity->corruption;
  
  packet.luxury_total=pcity->luxury_total;
  packet.tax_total=pcity->tax_total;
  packet.science_total=pcity->science_total;
  
  packet.food_stock=pcity->food_stock;
  packet.shield_stock=pcity->shield_stock;
  packet.polution=pcity->polution;
  packet.incite_revolt_cost=pcity->incite_revolt_cost;
  
  packet.is_building_unit=pcity->is_building_unit;
  packet.currently_building=pcity->currently_building;

  packet.did_buy=pcity->did_buy;
  packet.did_sell=pcity->did_sell;
  
  p=packet.city_map;
  for(y=0; y<CITY_MAP_SIZE; y++)
    for(x=0; x<CITY_MAP_SIZE; x++)
      *p++=get_worker_city(pcity, x, y)+'0';
  *p='\0';

  p=packet.improvements;
  for(i=0; i<B_LAST; i++)
    *p++=(pcity->improvements[i]) ? '1' : '0';
  *p='\0';
  
  for(o=0; o<game.nplayers; o++) {           /* dests */
    if(!dest || &game.players[o]==dest) {
       if(dosend || map_get_known(pcity->x, pcity->y, &game.players[o])) {
	send_packet_city_info(game.players[o].conn, &packet);
      }
/*      else {
	if(!map_get_known(pcity->x, pcity->y, &game.players[o]))
	  printf("%s cant see %s\n", game.players[o].name, pcity->name);
      }*/
    }
  }
}

void remove_trade_route(int c1, int c2) 
{
  int i;
  struct city *pc1, *pc2;
  
  pc1=find_city_by_id(c1);
  pc2=find_city_by_id(c2);
  if (pc1) {
    for (i=0;i<4;i++)
      if (pc1->trade[i]==c2)
	pc1->trade[i]=0;
  }
  if (pc2) {
    for (i=0;i<4;i++)
      if (pc2->trade[i]==c2)
	pc2->trade[i]=0;
  }
}

/**************************************************************************
...
**************************************************************************/
void remove_city(struct city *pcity)
{
  int o;
  struct unit *punit;
  struct genlist_iterator myiter;
  struct packet_generic_integer packet;
  for (o=0; o<4; o++)
    remove_trade_route(pcity->trade[0], pcity->id);
  packet.value=pcity->id;
  for(o=0; o<game.nplayers; o++)           /* dests */
    send_packet_generic_integer(game.players[o].conn,
				PACKET_REMOVE_CITY,&packet);
  genlist_iterator_init(&myiter, &pcity->units_supported.list, 0);

  for(; ITERATOR_PTR(myiter);) {
    punit=(struct unit *)ITERATOR_PTR(myiter);
    ITERATOR_NEXT(myiter);
    wipe_unit(0, punit);
  }
  game_remove_city(pcity->id);
}

