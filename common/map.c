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
#include <string.h>
#include <ctype.h>
#include "map.h"
#include "shared.h"
#include "log.h"
#include "registry.h"
#include "unit.h"
#include "city.h"
#include "mapgen.h"

/* the very map */
struct civ_map map;

struct tile_type tile_types[T_LAST]= 
{
  {"Arctic", "Seals",   0,0,0, 2,0,0, 2, 10, 3, T_LAST,0,      T_LAST,0},
  {"Desert", "Oasis",   0,1,0, 3,1,0, 1, 10, 1, T_DESERT,4,    T_DESERT,4},
  {"Forest", "Game" ,   1,2,0, 3,2,0, 2, 15, 3, T_PLAINS,4,    T_LAST,0},
  {"Grassland", "Resources", 2,0,0, 2,1,0, 1, 10, 1, T_GRASSLAND,4, T_FOREST,10},
  {"Hills", "Coals",    1,0,0, 1,2,0, 2, 20, 3, T_HILLS,10,    T_HILLS,7},
  {"Jungle", "Gems",    1,0,0, 1,0,4, 2, 15, 3, T_GRASSLAND,14,T_FOREST,14},
  {"Mountains","Gold",  0,1,0, 0,1,5, 3, 30, 5, T_LAST,0,      T_MOUNTAINS,9},
  {"Ocean", "Fish",     1,0,2, 3,0,2, 1, 10, 0, T_LAST,0,      T_LAST,0},
  {"Plains", "Horses",  1,1,0, 3,1,0, 1, 10, 1, T_PLAINS,4,    T_FOREST,14},
  {"River", "Resources",2,0,1, 2,1,1, 1, 15, 1, T_RIVER,4,     T_LAST,0},
  {"Swamp", "Oil",      1,0,0, 1,4,0, 2, 15, 3, T_GRASSLAND,14,T_FOREST,14},
  {"Tundra", "Game",    1,0,0, 3,0,0, 1, 10, 1, T_LAST,0,      T_LAST,0 }
};

char terrain_chars[]="adfghjm prst";
char dec2hex[]="0123456789abcdef";

struct tile void_tile={
  S_NONE, T_UNKNOWN, 0, 0
};


/***************************************************************
...
***************************************************************/
char *map_get_tile_info_text(int x, int y)
{
  static char s[64];
  struct tile *ptile=map_get_tile(x, y);
  
  if(ptile->special&S_SPECIAL)
    sprintf(s, "%s(%s)", 
	    tile_types[ptile->terrain].terrain_name,
	    tile_types[ptile->terrain].special_name);
  else
    sprintf(s, "%s", 
	    tile_types[ptile->terrain].terrain_name);

  return s;
}

/***************************************************************
...
***************************************************************/
int map_is_empty(void)
{
  return map.tiles==0;
}


/***************************************************************
 put some sensible values into the map structure
***************************************************************/
void map_init(void)
{
  map.xsize=MAP_DEFAULT_WIDTH;
  map.ysize=MAP_DEFAULT_HEIGHT;
  map.seed=MAP_DEFAULT_SEED;
  map.age=0;
  map.riches=MAP_DEFAULT_RICHES;
  map.is_earth=0;
  map.huts=MAP_DEFAULT_HUTS;
  map.landpercent=MAP_DEFAULT_LANDMASS;
  map.swampsize=MAP_DEFAULT_SWAMPS;
  map.deserts=MAP_DEFAULT_DESERTS;
  map.mountains=MAP_DEFAULT_MOUNTAINS;
  map.riverlength=MAP_DEFAULT_RIVERS;
  map.forestsize=MAP_DEFAULT_FORESTS;
  map.tiles=0;
}

/***************************************************************
...
***************************************************************/
struct tile_type *get_tile_type(enum tile_terrain_type type)
{
  return &tile_types[type];
}


/***************************************************************
...
***************************************************************/
int map_distance(int x0, int y0, int x1, int y1)
{
  int tmp;
  if(x0>x1)
    tmp=x0, x0=x1, x1=tmp;
  if(y0>y1)
    tmp=y0, y0=y1, y1=tmp;
  return MIN(x1-x0, map.xsize-x1+x0)+y1-y0;
}


/***************************************************************
...
***************************************************************/
int is_terrain_near_tile(int x, int y, enum tile_terrain_type t)
{
  if (map_get_terrain(x, y+1)==t) return 1;
  if (map_get_terrain(x-1, y-1)==t) return 1;
  if (map_get_terrain(x-1, y)==t) return 1;
  if (map_get_terrain(x-1, y+1)==t) return 1;
  if (map_get_terrain(x+1, y-1)==t) return 1;
  if (map_get_terrain(x+1, y)==t) return 1;
  if (map_get_terrain(x+1, y+1)==t) return 1;
  if (map_get_terrain(x, y-1)==t) return 1;
  return 0;
}

/***************************************************************
...
***************************************************************/

int is_water_adjacent_to_tile(int x, int y)
{
  struct tile *ptile, *ptile_n, *ptile_e, *ptile_s, *ptile_w;

  ptile=map_get_tile(x, y);
  ptile_n=map_get_tile(x, y-1);
  ptile_e=map_get_tile(x+1, y);
  ptile_s=map_get_tile(x, y+1);
  ptile_w=map_get_tile(x-1, y);

  return (ptile->terrain==T_OCEAN   || ptile->terrain==T_RIVER   || ptile->special&S_IRRIGATION ||
	  ptile_n->terrain==T_OCEAN || ptile_n->terrain==T_RIVER || ptile_n->special&S_IRRIGATION ||
	  ptile_e->terrain==T_OCEAN || ptile_e->terrain==T_RIVER || ptile_e->special&S_IRRIGATION ||
	  ptile_s->terrain==T_OCEAN || ptile_s->terrain==T_RIVER || ptile_s->special&S_IRRIGATION ||
	  ptile_w->terrain==T_OCEAN || ptile_w->terrain==T_RIVER || ptile_w->special&S_IRRIGATION);
}



/***************************************************************
...
***************************************************************/
int map_build_road_time(int x, int y)
{
  return tile_types[map_get_terrain(x, y)].road_time;
}

/***************************************************************
...
***************************************************************/
int map_build_irrigation_time(int x, int y)
{
  return tile_types[map_get_terrain(x, y)].irrigation_time;
}

/***************************************************************
...
***************************************************************/
int map_build_mine_time(int x, int y)
{
  return tile_types[map_get_terrain(x, y)].mining_time;
}


/***************************************************************
...
***************************************************************/
void map_irrigate_tile(int x, int y)
{
  enum tile_terrain_type now, result;
  
  now=map_get_terrain(x, y);
  result=tile_types[now].irrigation_result;
  
  if(now==result) {
    map_set_special(x, y, S_IRRIGATION);
  }
  else if(result!=T_LAST)
    map_set_terrain(x, y, result);
  map_clear_special(x, y, S_MINE);
}



/***************************************************************
...
***************************************************************/
void map_mine_tile(int x, int y)
{
  enum tile_terrain_type now, result;
  
  now=map_get_terrain(x, y);
  result=tile_types[now].mining_result;
  
  if(now==result) 
    map_set_special(x, y, S_MINE);
  else if(result!=T_LAST) 
    map_set_terrain(x, y, result);
  map_clear_special(x,y, S_IRRIGATION);
}



/***************************************************************
...
***************************************************************/

int tile_move_cost(struct unit *punit, int x, int y)
{
  struct tile *t=map_get_tile(x, y);
  if (t->terrain==T_OCEAN)
    return 3;
  if (t->special&S_RAILROAD)
    return 0;
  if (t->special&S_ROAD)
    return 1;
  return (get_tile_type(t->terrain)->movement_cost*3);
}

/***************************************************************
...
***************************************************************/
int map_move_cost(struct unit *punit, int x1, int y1)
{
  if (is_air_unit(punit) || is_sailing_unit(punit))
    return 3;
  return (tile_move_cost(punit, punit->x, punit->y)+
	  tile_move_cost(punit, x1, y1)+1)/2; 
}

/***************************************************************
...
***************************************************************/

int is_tiles_adjacent(int x0, int y0, int x1, int y1)
{
  return (((x0<=x1+1 && x0>=x1-1) || (x0==0 && x1==map.xsize-1) ||
	   (x0==map.xsize-1 && x1==0)) && (y0<=y1+1 && y0>=y1-1));
}


/***************************************************************
...
***************************************************************/
void map_save(struct section_file *file)
{
  int i, x, y;
  char *pbuf=(char *)malloc(map.xsize+1);

  secfile_insert_int(file, map.xsize, "map.width");
  secfile_insert_int(file, map.ysize, "map.height");
  secfile_insert_int(file, map.is_earth, "map.is_earth");

  for(i=0; i<R_LAST; i++) {
    secfile_insert_int(file, map.start_positions[i].x, "map.r%dsx", i);
    secfile_insert_int(file, map.start_positions[i].y, "map.r%dsy", i);
  }
    
  /* put the terrain type */
  for(y=0; y<map.ysize; y++) {
    for(x=0; x<map.xsize; x++)
      pbuf[x]=terrain_chars[map_get_tile(x, y)->terrain];
    pbuf[x]='\0';

    secfile_insert_str(file, pbuf, "map.t%03d", y);
  }

  /* get lower 4 bits of special flags */
  for(y=0; y<map.ysize; y++) {
    for(x=0; x<map.xsize; x++)
      pbuf[x]=dec2hex[map_get_tile(x, y)->special&0xf];
    pbuf[x]='\0';

    secfile_insert_str(file, pbuf, "map.l%03d", y);
  }


  /* put upper 4 bits of special flags */
  for(y=0; y<map.ysize; y++) {
    for(x=0; x<map.xsize; x++)
      pbuf[x]=dec2hex[(map_get_tile(x, y)->special&0xf0)>>4];
    pbuf[x]='\0';

    secfile_insert_str(file, pbuf, "map.u%03d", y);
  }

  /* put bit 0-3 of known bits */
  for(y=0; y<map.ysize; y++) {
    for(x=0; x<map.xsize; x++)
      pbuf[x]=dec2hex[(map_get_tile(x, y)->known&0xf)];
    pbuf[x]='\0';
    secfile_insert_str(file, pbuf, "map.a%03d", y);
  }

  /* put bit 4-7 of known bits */
  for(y=0; y<map.ysize; y++) {
    for(x=0; x<map.xsize; x++)
      pbuf[x]=dec2hex[((map_get_tile(x, y)->known&0xf0))>>4];
    pbuf[x]='\0';
    secfile_insert_str(file, pbuf, "map.b%03d", y);
  }

  /* put bit 8-11 of known bits */
  for(y=0; y<map.ysize; y++) {
    for(x=0; x<map.xsize; x++)
      pbuf[x]=dec2hex[((map_get_tile(x, y)->known&0xf00))>>8];
    pbuf[x]='\0';
    secfile_insert_str(file, pbuf, "map.c%03d", y);
  }

  /* put bit 12-15 of known bits */
  for(y=0; y<map.ysize; y++) {
    for(x=0; x<map.xsize; x++)
      pbuf[x]=dec2hex[((map_get_tile(x, y)->known&0xf000))>>12];
    pbuf[x]='\0';
    secfile_insert_str(file, pbuf, "map.d%03d", y);
  }
  
  free(pbuf);
}


/***************************************************************
...
***************************************************************/
void map_load(struct section_file *file)
{
  int i, x ,y;

  map_init();

  map.xsize=secfile_lookup_int(file, "map.width");
  map.ysize=secfile_lookup_int(file, "map.height");
  map.is_earth=secfile_lookup_int(file, "map.is_earth");

  for(i=0; i<R_LAST; i++) {
    map.start_positions[i].x=secfile_lookup_int(file, "map.r%dsx", i);
    map.start_positions[i].y=secfile_lookup_int(file, "map.r%dsy", i);
  }
  
  if(!(map.tiles=(struct tile*)malloc(map.xsize*map.ysize*
					 sizeof(struct tile)))) {
    log(LOG_FATAL, "malloc failed in load_map");
    exit(1);
  }

  for(y=0; y<map.ysize; y++)
    for(x=0; x<map.xsize; x++)
      tile_init(map_get_tile(x, y));


  /* get the terrain type */
  for(y=0; y<map.ysize; y++) {
    char *terline=secfile_lookup_str(file, "map.t%03d", y);
    for(x=0; x<map.xsize; x++) {
      char *pch;
      if(!(pch=strchr(terrain_chars, terline[x]))) {
	log(LOG_FATAL, "unknown terrain type in map at position (%d,%d)",
	    x, y, terline[x]);
	exit(1);
      }
      map_get_tile(x, y)->terrain=pch-terrain_chars;
    }
  }


  /* get lower 4 bits of special flags */
  for(y=0; y<map.ysize; y++) {
    char *terline=secfile_lookup_str(file, "map.l%03d", y);

    for(x=0; x<map.xsize; x++) {
      char ch=terline[x];

      if(isxdigit(ch))
	map_get_tile(x, y)->special=ch-(isdigit(ch) ? '0' : ('a'-10));
      else if(ch!=' ') {
	log(LOG_FATAL, "unknown special flag(lower) in map at position(%d,%d)",
	    x, y, ch);
	exit(1);
      }
      else
	map_get_tile(x, y)->special=0;
    }
  }


  /* get upper 4 bits of special flags */
  for(y=0; y<map.ysize; y++) {
    char *terline=secfile_lookup_str(file, "map.u%03d", y);

    for(x=0; x<map.xsize; x++) {
      char ch=terline[x];

      if(isxdigit(ch))
	map_get_tile(x, y)->special|=(ch-(isdigit(ch) ? '0' : 'a'-10))<<4;
      else if(ch!=' ') {
	log(LOG_FATAL, "unknown special flag(lower) in map at position(%d,%d)",
	    x, y, ch);
	exit(1);
      }


    }
  }


  /* get bits 0-3 of known flags */
  for(y=0; y<map.ysize; y++) {
    char *terline=secfile_lookup_str(file, "map.a%03d", y);
    for(x=0; x<map.xsize; x++) {
      char ch=terline[x];
      if(isxdigit(ch))
	map_get_tile(x, y)->known=ch-(isdigit(ch) ? '0' : ('a'-10));
      else if(ch!=' ') {
	log(LOG_FATAL, "unknown known flag(lower) in map at position(%d,%d)",
	    x, y, ch);
	exit(1);
      }
      else
	map_get_tile(x, y)->known=0;
    }
  }


  /* get bits 4-7 of known flags */
  for(y=0; y<map.ysize; y++) {
    char *terline=secfile_lookup_str(file, "map.b%03d", y);
    for(x=0; x<map.xsize; x++) {
      char ch=terline[x];
      if(isxdigit(ch))
	map_get_tile(x, y)->known|=(ch-(isdigit(ch) ? '0' : 'a'-10))<<4;
      else if(ch!=' ') {
	log(LOG_FATAL, "unknown known flag(lower) in map at position(%d,%d)",
	    x, y, ch);
	exit(1);
      }
    }
  }

  /* get bits 8-11 of known flags */
  for(y=0; y<map.ysize; y++) {
    char *terline=secfile_lookup_str(file, "map.c%03d", y);
    for(x=0; x<map.xsize; x++) {
      char ch=terline[x];
      if(isxdigit(ch))
	map_get_tile(x, y)->known|=(ch-(isdigit(ch) ? '0' : 'a'-10))<<8;
      else if(ch!=' ') {
	log(LOG_FATAL, "unknown known flag(lower) in map at position(%d,%d)",
	    x, y, ch);
	exit(1);
      }
    }
  }

  /* get bits 12-15 of known flags */
  for(y=0; y<map.ysize; y++) {
    char *terline=secfile_lookup_str(file, "map.d%03d", y);
    for(x=0; x<map.xsize; x++) {
      char ch=terline[x];

      if(isxdigit(ch))
	map_get_tile(x, y)->known|=(ch-(isdigit(ch) ? '0' : 'a'-10))<<12;
      else if(ch!=' ') {
	log(LOG_FATAL, "unknown known flag(lower) in map at position(%d,%d)",
	    x, y, ch);
	exit(1);
      }
    }
  }
}

/***************************************************************
...
***************************************************************/
void tile_init(struct tile *ptile)
{
  ptile->terrain=T_UNKNOWN;
  ptile->special=S_NONE;
  ptile->known=0;
  ptile->city_id=0;
  unit_list_init(&ptile->units);
}


/***************************************************************
...
***************************************************************/
struct tile *map_get_tile(int x, int y)
{
  if(y<0 || y>=map.ysize)
    return map.tiles; /* fix needed */
  else
    return map.tiles+map_adjust_x(x)+y*map.xsize;
}


/***************************************************************
...
***************************************************************/
enum tile_terrain_type map_get_terrain(int x, int y)
{
  if(y<0 || y>=map.ysize)
    return T_UNKNOWN;
  else
    return (map.tiles+map_adjust_x(x)+y*map.xsize)->terrain;
}

/***************************************************************
...
***************************************************************/
enum tile_special_type map_get_special(int x, int y)
{
  if(y<0 || y>=map.ysize)
    return S_NONE;
  else
    return (map.tiles+map_adjust_x(x)+y*map.xsize)->special;
}

/***************************************************************
...
***************************************************************/
void map_set_terrain(int x, int y, enum tile_terrain_type ter)
{
  (map.tiles+map_adjust_x(x)+
   map_adjust_y(y)*map.xsize)->terrain=ter;
}

/***************************************************************
...
***************************************************************/
void map_set_special(int x, int y, enum tile_special_type spe)
{
  (map.tiles+x+y*map.xsize)->special|=spe;
}

/***************************************************************
...
***************************************************************/
void map_clear_special(int x, int y, enum tile_special_type spe)
{
  (map.tiles+x+y*map.xsize)->special&=~spe;
}


/***************************************************************
...
***************************************************************/
struct city *map_get_city(int x, int y)
{
  int city_id;
  
  city_id=(map.tiles+map_adjust_x(x)+map_adjust_y(y)*map.xsize)->city_id;
  if(city_id)
    return find_city_by_id(city_id);
  return 0;
}


/***************************************************************
...
***************************************************************/
void map_set_city(int x, int y, struct city *pcity)
{
  (map.tiles+map_adjust_x(x)+map_adjust_y(y)*map.xsize)->city_id=
    (pcity) ? pcity->id : 0;
}


/***************************************************************
...
***************************************************************/
int map_get_known(int x, int y, struct player *pplayer)
{
  return ((map.tiles+map_adjust_x(x)+
	   map_adjust_y(y)*map.xsize)->known)&(1u<<pplayer->player_no);
}

/***************************************************************
...
***************************************************************/
enum known_type tile_is_known(int x, int y)
{
  return ((map.tiles+map_adjust_x(x)+
	   map_adjust_y(y)*map.xsize)->known);
}


/***************************************************************
...
***************************************************************/
void map_set_known(int x, int y, struct player *pplayer)
{
  (map.tiles+map_adjust_x(x)+
   map_adjust_y(y)*map.xsize)->known|=(1u<<pplayer->player_no);
}

/***************************************************************
...
***************************************************************/
void map_clear_known(int x, int y, struct player *pplayer)
{
  (map.tiles+map_adjust_x(x)+
   map_adjust_y(y)*map.xsize)->known&=~(1u<<pplayer->player_no);
}
