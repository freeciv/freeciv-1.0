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
#ifndef __CITY_H
#define __CITY_H

#include "genlist.h"

struct player;

enum improvement_type_id {
  B_APOLLO=0, B_AQUEDUCT, B_BANK, B_BARRACKS,B_BARRACKS2, B_BARRACKS3,  
  B_CAPITAL, B_CATHEDRAL, B_CITY,B_COASTAL,
  B_COLOSSEUM, B_COLLOSSUS, B_COPERNICUS, B_COURTHOUSE, B_CURE,
  B_DARWIN, B_FACTORY, B_GRANARY, B_GREAT, B_WALL, B_HANGING, B_HARBOUR,  
  B_HOOVER, B_HYDRO, B_ISAAC, B_BACH, B_LIBRARY, B_LIGHTHOUSE,
  B_MAGELLAN, B_MANHATTEN, B_MARKETPLACE, B_MASS, B_MFG,
  B_MICHELANGELO, B_NUCLEAR, B_ORACLE, B_PALACE, B_POWER,
  B_PYRAMIDS, B_RECYCLING, B_SDI, B_SETI, B_SHAKESPEARE,
  B_TEMPLE, B_UNITED,  B_UNIVERSITY, B_WOMENS, B_LAST
};


struct improvement_type {
  char name[MAX_LENGTH_NAME];
  int is_wonder;
  int tech_requirement;
  int build_cost;
  int shield_upkeep;
  int obsolete_by;
};

enum specialist_type {
  SP_ELVIS, SP_SCIENTIST, SP_TAXMAN
};

enum city_tile_type {
  C_TILE_EMPTY, C_TILE_WORKER, C_TILE_UNAVAILABLE
};


#define min(X, Y) ((X)>(Y) ? (Y) : (X))
#define max(X, Y) ((X)<(Y) ? (Y) : (X))
#define get_goverment(X) (game.players[X].goverment)

#define CITY_MAP_SIZE 5

struct city {
  int id;
  int owner;
  int x, y;
  char name[MAX_LENGTH_NAME];

  /* the people */
  int size;

  int ppl_happy[5], ppl_content[5], ppl_unhappy[5];
  int ppl_elvis, ppl_scientist, ppl_taxman;

  /* trade routes */
  int trade[4];

  /* the productions */
  int food_prod, food_surplus;
  int shield_prod, shield_surplus;
  int trade_prod, corruption;

  /* the totals */
  int luxury_total, tax_total, science_total;
  
  /* the physics */
  int food_stock;
  int shield_stock;
  int polution;
  int incite_revolt_cost;
   
  int is_building_unit;    /* boolean unit/improvement */
  int currently_building;
  
  unsigned char improvements[B_LAST];
  
  enum city_tile_type city_map[CITY_MAP_SIZE][CITY_MAP_SIZE];

  struct unit_list units_supported;
  
  /* turn states */
  int did_buy, did_sell, is_updated;
  int anarchy;                /* anarchy rounds count */ 
  int was_happy;
};


struct city_list {
  struct genlist list;
};


char *get_city_name_suggestion(struct player *pplayer);


/* Improvement stuff */

int can_build_improvement(struct city *pcity, enum improvement_type_id id);
int can_build_unit(struct city *pcity, enum unit_type_id id);
int building_value(enum improvement_type_id id);
char *building_name(enum improvement_type_id id);
int build_cost(struct city *pcity);
struct improvement_type *get_improvement_type(enum improvement_type_id id);
char *get_improvement_name(enum improvement_type_id id);
int building_obsolete(struct player *pplayer, enum improvement_type_id id);
int can_sell_building(struct city *pcity, int id);
int city_got_building(struct city *pcity,  enum improvement_type_id id); 
int can_establish_trade_route(struct city *pc1, struct city *pc2);


/* Wonder Stuff */
int wonder_is_obsolete(enum improvement_type_id id);
int is_wonder(enum improvement_type_id id);
struct city *find_city_wonder(enum improvement_type_id id);
int city_affected_by_wonder(struct city *pcity, enum improvement_type_id id);

/* City management stuff */
struct player *city_owner(struct city *pcity);
struct city *find_city_by_id(int id);
struct city *find_city_by_name(int id);
void city_list_init(struct city_list *this);
struct city *city_list_get(struct city_list *this, int index);
struct city *city_list_find_id(struct city_list *this, int id);
struct city *city_list_find_coor(struct city_list *this, int x, int y);
struct city *city_list_find_name(struct city_list *this, char *name);

void city_list_insert(struct city_list *this, struct city *pcity);
int city_list_size(struct city_list *this);
void city_list_unlink(struct city_list *this, struct city *pcity);

/* Happy Stuff */
int is_city_happy(struct city *pcity);                  /* love the king ??? */
int is_city_unhappy(struct city *pcity);                /* anarchy??? */


int get_population(struct city *pcity);                 /* pop. of city */
int get_shields_tile(int x, int y, struct city *pcity); /* shield on spot */
int get_trade_tile(int x, int y, struct city *pcity);   /* trade  on spot */
int get_food_tile(int x, int y, struct city *pcity);    /* food   on spot */
int nr_specialists(struct city *pcity);                 /* elv+tax+scie */
int is_worker_here(struct city *pcity, int x, int y); 
int city_got_citywalls(struct city *pcity);
int player_owns_city(struct player *pplayer, struct city *pcity);
enum city_tile_type get_worker_city(struct city *pcity, int x, int y);
int city_got_barracks(struct city *pcity);
int city_population(struct city *pcity);
void happy_copy(struct city *pcity, int i);



/* City calculation stuff. */
void set_worker_city(struct city *pcity, int x, int y, 
		     enum city_tile_type type); 
void set_food_trade_shields(struct city *pcity);
void citizen_happy_units(struct city *pcity, int unhap);
void set_food_prod(struct city *pcity);
void set_trade_prod(struct city *pcity);
void set_tax_income(struct city *pcity);
void set_food_trade_shields(struct city *pcity);
void set_polution(struct city *pcity);

void citizen_happy_size(struct city *pcity);
void citizen_happy_luxury(struct city *pcity);
void citizen_happy_units(struct city *pcity, int unhap);
void citizen_happy_buildings(struct city *pcity);
void citizen_happy_wonders(struct city *pcity);
void add_buildings_effect(struct city *pcity);
void unhappy_city_check(struct city *pcity);
int calc_gold_remains(struct city *pcity);
int is_city_celebrating(struct city *pcity);

int trade_between_cities(struct city *pc1, struct city *pc2);
int establish_trade_route(struct city *pc1, struct city *pc2);
struct city *find_palace(struct player *pplayer);
#endif
