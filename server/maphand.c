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
#include "map.h"
#include "game.h"
#include "maphand.h"
#include "packets.h"
#include "unithand.h"
#include "cityhand.h"


/**************************************************************************
...
**************************************************************************/
void global_warming(int effect)
{
  int x, y;
  int k=map.xsize*map.ysize;
  while(effect && k--) {
    x=lrand48()%map.xsize;
    y=lrand48()%map.ysize;
    if (map_get_terrain(x, y)!=T_OCEAN) {
      if(is_water_adjacent(x, y)) {
	switch (map_get_terrain(x, y)) {
	case T_FOREST:
	  effect--;
	  map_set_terrain(x, y, T_JUNGLE);
          relight_square_if_known(0, x, y);
	  break;
	case T_DESERT:
	case T_PLAINS:
	case T_GRASSLAND:
	  effect--;
	  map_set_terrain(x, y, T_SWAMP);
          relight_square_if_known(0, x, y);
	  break;
	default:
	  break;
	}
      } else {
	switch (map_get_terrain(x, y)) {
	case T_PLAINS:
	case T_GRASSLAND:
	case T_FOREST:
	  effect--;
	  map_set_terrain(x, y, T_DESERT);
          relight_square_if_known(0, x, y);
	  break;
	default:
	  break;
	}
      }

    
    }
  }
}



/**************************************************************************
...
**************************************************************************/
void give_map_from_player_to_player(struct player *pfrom, struct player *pdest)
{
  int x, y;

  for(y=0; y<map.ysize; y++)
    for(x=0; x<map.xsize; x++)
      if(map_get_known(x, y, pfrom) && !map_get_known(x, y, pdest)) {
	light_square(pdest, x, y, 0);
      }
}


/**************************************************************************
dest can be NULL meaning all players
**************************************************************************/
void send_all_known_tiles(struct player *dest)
{
  int o;
  
  
  for(o=0; o<game.nplayers; o++)           /* dests */
    if(!dest || &game.players[o]==dest) {
      int x, y;
      struct player *pplayer=&game.players[o];
      
      connection_do_buffer(pplayer->conn);
      for(y=0; y<map.ysize; y++)
	for(x=0; x<map.xsize; x++)
	  if(map_get_known(x, y, pplayer)) {
	    map_clear_known(x, y, pplayer);
	    light_square(pplayer, x, y, 0);
	  }
      connection_do_unbuffer(pplayer->conn);
    }
}


/**************************************************************************
dest can be NULL meaning all players

if(dest==NULL) only_send_tile_info_to_client_that_know_the_tile()
else send_tile_info

notice: the tile isn't lighten up on the client by this func
**************************************************************************/
void send_tile_info(struct player *dest, int x, int y, enum known_type type)
{
  int o;
  struct packet_tile_info info;
  struct tile *ptile;

  ptile=map_get_tile(x, y);

  info.x=x;
  info.y=y;
  info.type=ptile->terrain;
  info.special=ptile->special;

  for(o=0; o<game.nplayers; o++)           /* dests */
     if(!dest) {
       if(ptile->known & (1<<o)) {
	 info.known=TILE_KNOWN;
	 send_packet_tile_info(game.players[o].conn, &info);
       }
     }
     else if(&game.players[o]==dest) {
       info.known=type;
       send_packet_tile_info(game.players[o].conn, &info);
     }
}


/**************************************************************************
...
**************************************************************************/
void relight_square_if_known(struct player *pplayer, int x, int y)
{
  int o;
  for(o=0; o<game.nplayers; o++) {
    struct player *pplayer=&game.players[o];
    if(map_get_known(x, y, pplayer)) {
      connection_do_buffer(pplayer->conn);
      map_clear_known(x, y, pplayer);
      light_square(pplayer, x, y, 0);
     connection_do_unbuffer(pplayer->conn);
   }
 }
}


/**************************************************************************
...
**************************************************************************/
void light_square(struct player *pplayer, int x, int y, int len)
{
  int dx, dy;
  static int known_count=TILE_KNOWN;
  
  known_count++;

  for(dy=-len-1; dy<=len+1; ++dy)
    for(dx=-len-1; dx<=len+1; ++dx) {
      int abs_x=map_adjust_x(x+dx);
      int abs_y=map_adjust_y(y+dy);

      if(!map_get_known(abs_x, abs_y, pplayer)) {
	if(dx>=-len && dx<=len && dy>=-len && dy<=len)
	  send_tile_info(pplayer, abs_x, abs_y, TILE_KNOWN_NODRAW);
	else
	  send_tile_info(pplayer, abs_x, abs_y, TILE_UNKNOWN);
      }
    }

  for(dy=-len; dy<=len; ++dy)
    for(dx=-len; dx<=len; ++dx) {
      int abs_x=map_adjust_x(x+dx);
      int abs_y=map_adjust_y(y+dy);
      if(!map_get_known(abs_x, abs_y, pplayer)) {
	struct genlist_iterator myiter;
	genlist_iterator_init(&myiter, &map_get_tile(abs_x, abs_y)->units.list, 0);
	
	
	for(;ITERATOR_PTR(myiter) ;ITERATOR_NEXT(myiter)) {
	  send_unit_info(pplayer, (struct unit*)ITERATOR_PTR(myiter), 1);
	}
      }
    }
  
  for(dy=-len; dy<=len; ++dy)
    for(dx=-len; dx<=len; ++dx) {
      int abs_x=map_adjust_x(x+dx);
      int abs_y=map_adjust_y(y+dy);
      if(!map_get_known(abs_x, abs_y, pplayer)) {
	struct city *pcity;
	
	if((pcity=map_get_city(abs_x, abs_y))) {
	  send_city_info(pplayer, pcity, 1);
	}
      }
    }
  
  for(dy=-len; dy<=len; ++dy)
    for(dx=-len; dx<=len; ++dx) {
      int abs_x=map_adjust_x(x+dx);
      int abs_y=map_adjust_y(y+dy);

      if(!map_get_known(abs_x, abs_y, pplayer)) {
	map_set_known(abs_x, abs_y, pplayer);
	send_tile_info(pplayer, abs_x, abs_y, known_count);
      }
    }
}




/**************************************************************************
...
**************************************************************************/
void lighten_area(struct player *pplayer, int x, int y)
{
  connection_do_buffer(pplayer->conn);

  light_square(pplayer, x, y, 1);
  
  connection_do_unbuffer(pplayer->conn);
}



/**************************************************************************
...
**************************************************************************/
void send_map_info(struct player *dest)
{
  int o;
  struct packet_map_info minfo;

  minfo.xsize=map.xsize;
  minfo.ysize=map.ysize;
  minfo.is_earth=map.is_earth;
 
  for(o=0; o<game.nplayers; o++)           /* dests */
    if(!dest || &game.players[o]==dest) {
      struct player *pplayer=&game.players[o];
      send_packet_map_info(pplayer->conn, &minfo);
    }
}

