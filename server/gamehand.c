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
#include "gamehand.h"
#include "unithand.h"
#include "maphand.h"
#include "packets.h"
#include "map.h"

/**************************************************************************
...
**************************************************************************/
void init_new_game(void)
{
  int i, x, y,j;
    
  for(i=0; i<game.nplayers; i++) {
    x=map.start_positions[game.players[i].race].x;
    y=map.start_positions[game.players[i].race].y;
    light_square(&game.players[i], x, y, 1);
    for (j=0;j<game.settlers;j++)
      create_unit(&game.players[i], x, y, U_SETTLERS, 0, 0);
  }
  
}


/**************************************************************************
...
**************************************************************************/
void send_year_to_clients(int year)
{
  int i;
  struct packet_new_year apacket;
  apacket.year=year;

  for(i=0; i<game.nplayers; i++) {
    game.players[i].turn_done=0;
    game.players[i].nturns_idle++;
    send_packet_new_year(game.players[i].conn, &apacket);
  }
}


/**************************************************************************
...
**************************************************************************/
void send_game_state(struct player *dest, int state)
{
  int o;
  struct packet_generic_integer pack;

  pack.value=state;
  
  
  for(o=0; o<game.nplayers; o++)
    if(!dest || &game.players[o]==dest)
      send_packet_generic_integer(game.players[o].conn, 
				  PACKET_GAME_STATE, &pack);
}



/**************************************************************************
dest can be NULL meaning all players
**************************************************************************/
void send_game_info(struct player *dest)
{
  int i, o;
  struct packet_game_info ginfo;
  
  ginfo.gold=game.gold;
  ginfo.tech=game.tech;
  ginfo.techlevel=game.techlevel;
  ginfo.skill_level=game.skill_level;
  ginfo.timeout=game.timeout;
  ginfo.end_year=game.end_year;
  ginfo.year=game.year;
  ginfo.min_players=game.min_players;
  ginfo.max_players=game.max_players;
  ginfo.nplayers=game.nplayers;
  ginfo.globalwarming=game.globalwarming;
  ginfo.heating=game.heating;
  for(i=0; i<A_LAST; i++)
    ginfo.global_advances[i]=game.global_advances[i];
  for(i=0; i<B_LAST; i++)
    ginfo.global_wonders[i]=game.global_wonders[i];

  
  for(o=0; o<game.nplayers; o++)           /* dests */
    if(!dest || &game.players[o]==dest) {
      struct player *pplayer=&game.players[o];
      ginfo.player_idx=o;
      send_packet_game_info(pplayer->conn, &ginfo);
    }


}
