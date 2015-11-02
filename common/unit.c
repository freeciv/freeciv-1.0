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
#include "game.h"
#include "unit.h"
#include "tech.h"
#include "map.h"
#include "player.h"

struct unit_type unit_types[]={
  {"Armor",      LAND_MOVING,  80,  3, 10,  5,  3*3, A_AUTOMOBILE,  1,  0, 30, 2, -1, 0},
  {"Artillery",  LAND_MOVING,  60,  1, 12,  2,  2*3, A_ROBOTICS,    1,  0, 30, 1, -1, 0},
  {"Battleship", SEA_MOVING,  160,  1, 12,  12,  4*3, A_STEEL,       2,  0, 40, 2, -1, 0},
  {"Bomber",     AIR_MOVING,  120,  1, 12,  1,  8*3, A_ADVANCED,    2,  0, 30, 1, -1, 2},
  {"Cannon",     LAND_MOVING,  40,  1,  8,  1,  1*3, A_METALLURGY,  1,  0, 30, 1, A_ROBOTICS, 0},
  {"Caravan",    LAND_MOVING,  50,  1,  0,  1,  1*3, A_TRADE,       1,  0, 10, 1, -1, 0},
  {"Carrier",    SEA_MOVING,  160,  1,  1, 12,  5*3, A_ADVANCED,    2,  8, 30, 1, -1, 0},
  {"Catapult",   LAND_MOVING,  40,  1,  6,  1,  1*3, A_MATHEMATICS, 1,  0, 20, 1, A_METALLURGY, 0},
  {"Cavalry",    LAND_MOVING,  20,  1,  2,  1,  2*3, A_HORSEBACK,   1,  0, 10, 1, A_CONSCRIPTION, 0},
  {"Chariot",    LAND_MOVING,  40,  1,  4,  1,  2*3, A_WHEEL,       1,  0, 10, 1, A_CHIVALRY, 0},
  {"Cruiser",    SEA_MOVING,   80,  1,  6,  6,  6*3, A_COMBUSTION,  2,  0, 30, 1, -1, 0},
  {"Diplomat",   LAND_MOVING,  30,  1,  0,  0,  2*3, A_WRITING,     1,  0, 10, 1, -1, 0},
  {"Fighter",    AIR_MOVING,   60,  1,  4,  2, 10*3, A_FLIGHT,      1,  0, 30, 1, -1, 1},
  {"Frigate",    SEA_MOVING,   40,  1,  2,  2,  3*3, A_MAGNETISM,   1,  4, 20, 1, A_INDUSTRIALIZATION, 0},
  {"Ironclad",   SEA_MOVING,   60,  1,  4,  4,  4*3, A_STEAM,       1,  0, 30, 1, A_COMBUSTION, 0},
  {"Knights",    LAND_MOVING,  40,  1,  4,  2,  2*3, A_CHIVALRY,    1,  0, 20, 1, A_AUTOMOBILE, 0},
  {"Legion",     LAND_MOVING,  20,  1,  3,  1,  1*3, A_IRON,        1,  0, 10, 1, A_CONSCRIPTION, 0},
  {"Mech. Inf",  LAND_MOVING,  50,  3,  6,  6,  3*3, A_LABOR,       1,  0, 30, 1, -1, 0},
  {"Militia",    LAND_MOVING,  10,  1,  1,  1,  1*3, A_NONE,        1,  0, 10, 1, A_GUNPOWDER, 0},
  {"Musketeers", LAND_MOVING,  30,  1,  2,  3,  1*3, A_GUNPOWDER,   1,  0, 20, 1, A_CONSCRIPTION, 0},
  {"Nuclear",    AIR_MOVING,  160,  1, 99,  0, 16*3, A_FISSION,     1,  0, 30, 1, -1, 1},
  {"Phalanx",    LAND_MOVING,  20,  1,  1,  2,  1*3, A_BRONZE,      1,  0, 10, 1, A_GUNPOWDER, 0},
  {"Riflemen",   LAND_MOVING,  30,  1,  3,  5,  1*3, A_CONSCRIPTION,1,  0, 20, 1, -1, 0},
  {"Sail",       SEA_MOVING,   40,  1,  1,  1,  3*3, A_NAVIGATION,  1,  3, 20, 1, A_MAGNETISM, 0},
  {"Settlers",   LAND_MOVING,  40,  1,  0,  1,  1*3, A_NONE,        1,  0, 10, 1, -1, 0},
  {"Submarine",  SEA_MOVING,   50,  1, 10,  2,  3*3, A_MASS,        2,  0, 20, 1, -1, 0},
  {"Transport",  SEA_MOVING,   50,  1,  0,  3,  4*3, A_INDUSTRIALIZATION,2, 8,20, 1, -1, 0},
  {"Trireme",    SEA_MOVING,   40,  1,  1,  0,  3*3, A_MAPMAKING,   1,  2, 10, 1, A_NAVIGATION, 0}
};




/***************************************************************
...
***************************************************************/
struct unit *find_unit_by_id(int id)
{
  int i;

  for(i=0; i<game.nplayers; i++) {
    struct unit *punit;
    if((punit=unit_list_find(&game.players[i].units, id)))
      return punit;
  }
  return 0;
}


/**************************************************************************
bribe unit
investigate
poison
make revolt
establish embassy
sabotage city
**************************************************************************/

/**************************************************************************
...
**************************************************************************/
int diplomat_can_do_action(struct unit *pdiplomat,
			   enum diplomat_actions action, 
			   int destx, int desty)
{
  struct city *pcity=map_get_city(destx, desty);
  struct tile *ptile=map_get_tile(destx, desty);
  
  if(is_tiles_adjacent(pdiplomat->x, pdiplomat->y, destx, desty)) {
    if(pcity) {  
      if(pcity->owner!=pdiplomat->owner) {
	if(action==DIPLOMAT_SABOTAGE)
	  return 1;
        if(action==DIPLOMAT_EMBASSY && 
	   !player_has_embassy(&game.players[pdiplomat->owner], 
			       &game.players[pcity->owner]))
	   return 1;
	if(action==DIPLOMAT_STEAL)
	  return 1;
	if(action==DIPLOMAT_INCITE)
	  return 1;
      }
    }
    else {
      if(action==DIPLOMAT_BRIBE && unit_list_size(&ptile->units)==1 &&
	 unit_list_get(&ptile->units, 0)->owner!=pdiplomat->owner)
	return 1;
    }
  }
  return 0;
}



/**************************************************************************
...
**************************************************************************/
int unit_can_help_build_wonder(struct unit *punit, struct city *pcity)
{
  return is_tiles_adjacent(punit->x, punit->y, pcity->x, pcity->y) &&
    punit->owner==pcity->owner && !pcity->is_building_unit  && 
    is_wonder(pcity->currently_building);
}


/**************************************************************************
...
**************************************************************************/
int unit_can_defend_here(struct unit *punit)
{
  if(is_ground_unit(punit) && map_get_terrain(punit->x, punit->y)==T_OCEAN)
    return 0;
  
  return 1;
}

/**************************************************************************
...
**************************************************************************/
int is_transporter_with_free_space(struct player *pplayer, int x, int y)
{
  int none_transporters, total_capacity=0;
  struct genlist_iterator myiter;

  none_transporters=0;
  total_capacity=0;
  
  genlist_iterator_init(&myiter, &map_get_tile(x, y)->units.list, 0);
  
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct unit *punit=(struct unit *)ITERATOR_PTR(myiter);
    
    if(get_transporter_capacity(punit))
      total_capacity+=get_transporter_capacity(punit);
    else
      none_transporters++;
  }
  
  return total_capacity>none_transporters;
}


/**************************************************************************
...
**************************************************************************/
void transporter_cargo_to_unitlist(struct unit *ptran, struct unit_list *list)
{
  struct genlist_iterator myiter;
  struct unit_list *srclist;
  int cargo;
  
  unit_list_init(list);
    
  srclist=&map_get_tile(ptran->x, ptran->y)->units;
  
  genlist_iterator_init(&myiter, &srclist->list, 0);
  
  cargo=get_transporter_capacity(ptran);
  
  for(; cargo && ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct unit *punit=(struct unit *)ITERATOR_PTR(myiter);
    
    if((is_ground_unit(punit) || 
       (is_air_unit(punit) && punit->type==U_CARRIER)) &&
       (!map_get_city(punit->x, punit->y) || 
	punit->activity==ACTIVITY_SENTRY)) {
      unit_list_unlink(srclist, punit);
      unit_list_insert(list, punit);
      cargo--;
    }
  
  }
  
}

/**************************************************************************
...
**************************************************************************/
void move_unit_list_to_tile(struct unit_list *units, int x, int y)
{
  struct genlist_iterator myiter;
  
  genlist_iterator_init(&myiter, &units->list, 0);

  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct unit *punit=(struct unit *)ITERATOR_PTR(myiter);
    punit->x=x;
    punit->y=y;
    unit_list_insert_back(&map_get_tile(x, y)->units, punit);
  }
  
}

/**************************************************************************
...
**************************************************************************/
int get_transporter_capacity(struct unit *punit)
{
  return unit_types[punit->type].transport_capacity;
}

/**************************************************************************
...
**************************************************************************/
int is_sailing_unit(struct unit *punit)
{
  return (unit_types[punit->type].move_type==SEA_MOVING);

}

/**************************************************************************
...
**************************************************************************/
int is_air_unit(struct unit *punit)
{
  return (unit_types[punit->type].move_type==AIR_MOVING);
}

/**************************************************************************
...
**************************************************************************/
int is_ground_unit(struct unit *punit)
{
  return (unit_types[punit->type].move_type==LAND_MOVING);
}

/**************************************************************************
...
**************************************************************************/
int is_water_unit(enum unit_type_id id)
{
  return (unit_types[id].move_type==SEA_MOVING);
}

/**************************************************************************
...
**************************************************************************/
int is_military_unit(struct unit *this_unit)
{
  if (this_unit->type == U_CARAVAN || this_unit->type == U_DIPLOMAT ||
      this_unit->type == U_SETTLERS)
    return 0;
  return 1;
}

/**************************************************************************
...
**************************************************************************/
int is_field_unit(struct unit *this_unit)
{
  switch (this_unit->type) {
  case U_BATTLESHIP:
  case U_BOMBER:
  case U_CRUISER:
  case U_FIGHTER:
  case U_FRIGATE:
  case U_IRONCLAD:
  case U_NUCLEAR:
  case U_SUBMARINE:
    return 1;
    break;
  default:
    break;
  }
  return 0;
}

/**************************************************************************
...
**************************************************************************/

int unit_value(enum unit_type_id id)
{
  return (unit_types[id].build_cost);
}


/**************************************************************************
...
**************************************************************************/
char *unit_name(enum unit_type_id id)
{
  return (unit_types[id].name);
}

struct unit_type *get_unit_type(enum unit_type_id id)
{
  return &unit_types[id];
}


/**************************************************************************
...
**************************************************************************/
void raise_unit_top(struct unit *punit)
{
  struct tile *ptile;
  struct unit *yo;

  ptile=map_get_tile(punit->x, punit->y);

  unit_list_unlink(&ptile->units, punit);
  unit_list_insert(&ptile->units, punit);

  yo=unit_list_get(&ptile->units, 0);
}

/**************************************************************************
...
**************************************************************************/
int can_unit_build_city(struct unit *punit)
{
  if(punit->type!=U_SETTLERS)
    return 0;

  if(map_get_city(punit->x, punit->y))
    return 0;
    
  if(map_get_terrain(punit->x, punit->y)==T_OCEAN)
    return 0;

  return 1;
}

/**************************************************************************
...
**************************************************************************/
int can_unit_do_activity(struct unit *punit, enum unit_activity activity)
{
  struct tile *ptile;
  struct tile_type *type;
  
  ptile=map_get_tile(punit->x, punit->y);
  type=get_tile_type(ptile->terrain);
  
  if(activity==ACTIVITY_IDLE)
    return 1;

  if(punit->activity==ACTIVITY_IDLE) {
    if(activity==ACTIVITY_FORTIFY)
      return 1;
    if(activity==ACTIVITY_SENTRY)
      return 1;

    if(activity==ACTIVITY_PILLAGE) {
      if(punit->type==U_SETTLERS)
        if((ptile->special&S_ROAD) || (ptile->special&S_RAILROAD) ||
	   (ptile->special&S_IRRIGATION))
	   if(!is_unit_activity_on_tile(ACTIVITY_PILLAGE, punit->x, punit->y))
	     return 1;
       return 0;
    }

    if(activity==ACTIVITY_FORTRESS) {
      if(punit->type==U_SETTLERS)
        if(!(ptile->special&S_FORTRESS) && ptile->terrain!=T_OCEAN)
          if(!is_unit_activity_on_tile(ACTIVITY_FORTRESS, punit->x, punit->y))
            return 1;
      return 0;
    }
    
    if(activity==ACTIVITY_POLUTION) {
      if(punit->type==U_SETTLERS)
	if(ptile->special&S_POLUTION)
	  if(!is_unit_activity_on_tile(ACTIVITY_POLUTION, punit->x, punit->y))
	    return 1;
      return 0;
    }
    
    if(activity==ACTIVITY_IRRIGATE) {
      if(punit->type==U_SETTLERS)
	if((ptile->terrain==type->irrigation_result &&
	    !(ptile->special&S_IRRIGATION) && 
	    is_water_adjacent_to_tile(punit->x, punit->y)) ||
	   (ptile->terrain!=type->irrigation_result &&
	    type->irrigation_result!=T_LAST))
	  if(!is_unit_activity_on_tile(ACTIVITY_IRRIGATE, punit->x, punit->y))
	    return 1;
      return 0;
    }
    
    if(activity==ACTIVITY_ROAD) {
      if(punit->type==U_SETTLERS && ptile->terrain!=T_OCEAN && 
	 (ptile->terrain!=T_RIVER || 
	  get_invention(&game.players[punit->owner], A_BRIDGE)==TECH_KNOWN)) {
	if(!(ptile->special&S_ROAD))
	  if(!is_unit_activity_on_tile(ACTIVITY_ROAD, punit->x, punit->y))
	    return 1;
      }
      return 0;
    }
    
    if(activity==ACTIVITY_RAILROAD) {
      if(punit->type==U_SETTLERS && ptile->terrain!=T_OCEAN && 
	 (ptile->terrain!=T_RIVER || 
	  get_invention(&game.players[punit->owner], A_BRIDGE)==TECH_KNOWN)) {
	if((ptile->special&S_ROAD) && !(ptile->special&S_RAILROAD)) {
	  if(get_invention(&game.players[punit->owner], A_RAILROAD)==
	     TECH_KNOWN && 
	     !is_unit_activity_on_tile(ACTIVITY_RAILROAD, punit->x, punit->y))
	    return 1;
	}
      }
      return 0;
    }

    if(activity==ACTIVITY_MINE) {
      if(punit->type==U_SETTLERS)
	if(type->mining_result!=T_LAST && !(ptile->special&S_MINE))
	  if(!is_unit_activity_on_tile(ACTIVITY_MINE, punit->x, punit->y))
	    return 1;
      return 0;
    }

  }
  
  return 0;
}
/**************************************************************************
...
**************************************************************************/
int is_unit_activity_on_tile(enum unit_activity activity, int x, int y)
{
  struct genlist_iterator myiter;

  genlist_iterator_init(&myiter, &map_get_tile(x, y)->units.list, 0);

  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    if(((struct unit *)ITERATOR_PTR(myiter))->activity==activity)
      return 1;

  return 0;
}
int is_sailing_unit_tile(int x, int y)
{
  struct genlist_iterator myiter;
  genlist_iterator_init(&myiter, &map_get_tile(x,y)->units.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct unit *punit=(struct unit *)ITERATOR_PTR(myiter);
    if (is_sailing_unit(punit)) 
      return 1;
  }
  return 0;
}

/**************************************************************************
... 
**************************************************************************/

int is_my_zoc(struct unit *myunit, int x0, int y0)
{
 
  struct unit_list *punit_list;
  struct unit *punit;
  int x,y;
  int owner=myunit->owner;
  for (x=x0-1;x<x0+2;x++)
    for (y=y0-1;y<y0+2;y++) {
      punit_list=&map_get_tile(x, y)->units;
      if((punit=unit_list_get(punit_list, 0)) && punit->owner!=owner) {
	if (is_sailing_unit(myunit)) {
	  if (is_sailing_unit_tile(x,y)) 
	    return 0;
	} else 
	  return 0;
      }
    }
  return 1;
}

/**************************************************************************
... 
**************************************************************************/

int zoc_ok_move(struct unit *punit,int x, int y)
{
  struct unit_list *punit_list;

  punit_list=&map_get_tile(x, y)->units;
    if(unit_list_get(punit_list, 0)) 
      return 1;
  if (punit->type==U_DIPLOMAT || punit->type==U_CARAVAN || is_air_unit(punit))
    return 1;
  
  return (is_my_zoc(punit, punit->x, punit->y) || 
      is_my_zoc(punit, x, y)); 
}

/**************************************************************************
... 
**************************************************************************/
int can_unit_move_to_tile(struct unit *punit, int x, int y)
{
  struct tile *ptile,*ptile2;
  
  if(punit->activity!=ACTIVITY_IDLE && punit->activity!=ACTIVITY_GOTO)
    return 0;
  
  if(x<0 || x>=map.xsize || y<0 || y>=map.ysize)
    return 0;
  
  if(!is_tiles_adjacent(punit->x, punit->y, x, y))
    return 0;
/*  if (punit->moves_left==0)
    return 0;*/

  ptile=map_get_tile(x, y);
  ptile2=map_get_tile(punit->x, punit->y);
  if(is_ground_unit(punit)) {
    if(ptile->terrain==T_OCEAN)  {
      if(!is_transporter_with_free_space(&game.players[punit->owner], x, y))
	return 0;
    }
    if (ptile2->terrain==T_OCEAN && map_get_city(x,y)  
	&& map_get_city(x,y)->owner!=punit->owner) /* no marines */
      return 0;
  }
  else if(is_sailing_unit(punit)) {
    if(ptile->terrain!=T_OCEAN && ptile->terrain!=T_UNKNOWN)
      if(!map_get_city(x, y) || map_get_city(x, y)->owner!=punit->owner)
	return 0;
  }
  return (zoc_ok_move(punit, x, y));
}

/**************************************************************************
 ...
**************************************************************************/
char *unit_description(struct unit *punit)
{
  struct city *pcity;
  static char buffer[512];

  pcity=city_list_find_id(&game.player_ptr->cities, punit->homecity);

  sprintf(buffer, "%s\n%s\n%s", 
	  get_unit_type(punit->type)->name, 
	  unit_activity_text(punit), 
	  pcity ? pcity->name : "");

  return buffer;
}

/**************************************************************************
 ...
**************************************************************************/
char *unit_activity_text(struct unit *punit)
{
  static char text[64];
   
  switch(punit->activity) {
   case ACTIVITY_IDLE:
    if(punit->moves_left%3) {
      if(punit->moves_left/3>0)
	sprintf(text, "Moves: %d %d/3", punit->moves_left/3, 
		punit->moves_left%3);
      else
	sprintf(text, "Moves: %d/3", punit->moves_left%3);
    }
    else
      sprintf(text, "Moves: %d", punit->moves_left/3);

    if(is_military_unit(punit) && get_unit_type(punit->type)->no_attacks>1)
      sprintf(text+strlen(text), " Hits: %d", punit->attacks_left);

    return text;
   case ACTIVITY_POLUTION:
    return "Polution";
   case ACTIVITY_ROAD:
    return "Road";
   case ACTIVITY_RAILROAD:
    return "Railroad";
   case ACTIVITY_MINE: 
    return "Mine";
    case ACTIVITY_IRRIGATE:
    return "Irrigation";
   case ACTIVITY_FORTIFY:
    return "Fortify";
   case ACTIVITY_FORTRESS:
    return "Fortress";
   case ACTIVITY_SENTRY:
    return "Sentry";
   default:
    break;
  }
  return 0;
}

/**************************************************************************
...
**************************************************************************/
void unit_list_init(struct unit_list *this)
{
  genlist_init(&this->list);
}

/**************************************************************************
...
**************************************************************************/
struct unit *unit_list_get(struct unit_list *this, int index)
{
  return (struct unit *)genlist_get(&this->list, index);
}

/**************************************************************************
...
**************************************************************************/
struct unit *unit_list_find(struct unit_list *this, int id)
{
  struct genlist_iterator myiter;

  genlist_iterator_init(&myiter, &this->list, 0);

  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    if(((struct unit *)ITERATOR_PTR(myiter))->id==id)
      return ITERATOR_PTR(myiter);

  return 0;
}

/**************************************************************************
...
**************************************************************************/
void unit_list_insert(struct unit_list *this, struct unit *punit)
{
  genlist_insert(&this->list, punit, 0);
}

/**************************************************************************
...
**************************************************************************/
void unit_list_insert_back(struct unit_list *this, struct unit *punit)
{
  genlist_insert(&this->list, punit, -1);
}


/**************************************************************************
...
**************************************************************************/
int unit_list_size(struct unit_list *this)
{
  return genlist_size(&this->list);
}

/**************************************************************************
...
**************************************************************************/
void unit_list_unlink(struct unit_list *this, struct unit *punit)
{
  genlist_unlink(&this->list, punit);
}
