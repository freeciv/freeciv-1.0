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
#ifndef __MAP_H
#define __MAP_H

#include "player.h"
#include "genlist.h"
#include "registry.h"
#include "unit.h"

enum tile_special_type {
  S_NONE=0, S_SPECIAL=1, S_ROAD=2, S_IRRIGATION=4, S_RAILROAD=8,
  S_MINE=16, S_POLUTION=32, S_FORT=64, S_HUT=128, S_FORTRESS=256
};

enum tile_terrain_type {
  T_ARCTIC, T_DESERT, T_FOREST, T_GRASSLAND, T_HILLS, T_JUNGLE, 
  T_MOUNTAINS, T_OCEAN, T_PLAINS, T_RIVER, T_SWAMP, T_TUNDRA, T_UNKNOWN,
  T_LAST
};

enum known_type {
 TILE_UNKNOWN, TILE_KNOWN_NODRAW, TILE_KNOWN
};

struct map_position {
  int x,y;
};

struct tile {
  enum tile_terrain_type terrain;
  enum tile_special_type special;
  unsigned short known;
  int city_id;
  struct unit_list units;
};


/****************************************************************
tile_type for each terrain type
expand with goverment bonuses??
*****************************************************************/
struct tile_type {
  char *terrain_name;
  char *special_name;

  int food;
  int shield;
  int trade;

  int food_special;
  int shield_special;
  int trade_special;
  
  int movement_cost;
  int defense_bonus;

  int road_time;
  
  enum tile_terrain_type irrigation_result;
  int irrigation_time;

  enum tile_terrain_type mining_result;
  int mining_time;
};

struct civ_map { 
  int xsize, ysize;
  int seed;
  int age;
  int riches;
  int is_earth;
  int huts;
  int landpercent;
  int swampsize;
  int deserts;
  int mountains;
  int riverlength;
  int forestsize;
  struct tile *tiles;
  struct map_position start_positions[R_LAST];
};


char *map_get_tile_info_text(int x, int y);
void map_init(void);
int map_is_empty(void);
void map_fractal_create(void);
void map_load(struct section_file *file);
void map_save(struct section_file *file);
struct tile *map_get_tile(int x, int y);
int map_distance(int x0, int y0, int x1, int y1);

#define map_adjust_x(X) \
 (((X)<0) ?  (X)+map.xsize : (((X)>=map.xsize) ? (X)-map.xsize : (X)))
#define map_adjust_y(Y) \
  (((Y)<0) ? 0 : (((Y)>=map.ysize) ? map.ysize-1 : (Y)))

struct city *map_get_city(int x, int y);
void map_set_city(int x, int y, struct city *pcity);
enum tile_terrain_type map_get_terrain(int x, int y);
enum tile_special_type map_get_special(int x, int y);
void map_set_terrain(int x, int y, enum tile_terrain_type ter);
void map_set_special(int x, int y, enum tile_special_type spe);
void map_clear_special(int x, int y, enum tile_special_type spe);
void tile_init(struct tile *ptile);
int map_get_known(int x, int y, struct player *pplayer);
enum known_type tile_is_known(int x, int y);
void map_set_known(int x, int y, struct player *pplayer);
void map_clear_known(int x, int y, struct player *pplayer);

void send_full_tile_info(struct player *dest, int x, int y);
int is_water_adjacent_to_tile(int x, int y);
int is_tiles_adjacent(int x0, int y0, int x1, int y1);
int map_move_cost(struct unit *punit, int x1, int y1);
struct tile_type *get_tile_type(enum tile_terrain_type type);
int is_terrain_near_tile(int x, int y, enum tile_terrain_type t);
void map_irrigate_tile(int x, int y);
void map_mine_tile(int x, int y);
extern struct civ_map map;
int map_build_road_time(int x, int y);
int map_build_irrigation_time(int x, int y);
int map_build_mine_time(int x, int y);

#endif
