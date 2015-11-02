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
#include <string.h>

#include "game.h"
#include "packets.h"
#include "player.h"


/**************************************************************************
...
**************************************************************************/
void handle_chat_msg(struct player *pplayer, 
		     struct packet_generic_message *packet)
{
  int i;
  struct packet_generic_message genmsg;
  char *cp;
  
  cp=strchr(packet->message, ':');
  
  if(cp) { /* msg someone mode */
    struct player *pdest;
    char name[512];
    strncpy(name, packet->message, cp-packet->message);
    name[cp-packet->message]='\0';

    if((pdest=find_player_by_name(name))) {
      sprintf(genmsg.message, "->*%s* %s", name, cp+1+(*(cp+1)==' '));
      send_packet_generic_message(pplayer->conn, PACKET_CHAT_MSG, &genmsg);

      sprintf(genmsg.message, "*%s* %s",
	      pplayer->name, cp+1+(*(cp+1)==' '));
      send_packet_generic_message(pdest->conn, PACKET_CHAT_MSG, &genmsg);
    }
    else {
      sprintf(genmsg.message, "Game: there's no player by the name %s",
	      name);
      send_packet_generic_message(pplayer->conn, PACKET_CHAT_MSG, 
				  &genmsg);
    }
  
  }
  else {
    sprintf(genmsg.message, "<%s> %s",
	    pplayer->name,
	    packet->message);
    
    for(i=0; i<game.nplayers; i++)
      send_packet_generic_message(game.players[i].conn, PACKET_CHAT_MSG, 
				  &genmsg);
  }

}
