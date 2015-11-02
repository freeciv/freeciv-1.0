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
#ifndef __UNITHAND_H
#define __UNITHAND_H

#include "packets.h"
#include "unit.h"

void handle_diplomat_action(struct player *pplayer, 
			    struct packet_diplomat_action *packet);
void unit_versus_unit(struct unit *attacker, struct unit *defender);

void handle_unit_change_homecity(struct player *pplayer, 
				 struct packet_unit_request *req);
void handle_unit_goto_tile(struct player *pplayer, 
			   struct packet_unit_request *req);
void update_unit_activities(struct player *player);
void handle_unit_info(struct player *pplayer, struct packet_unit_info *pinfo);
void handle_unit_move_request(struct player *pplayer, struct unit *punit,
			      int dest_x, int dest_y);
void handle_unit_activity_request(struct player *pplayer, struct unit *punit, 
				  enum unit_activity new_activity);
void send_unit_info(struct player *dest, struct unit *punit, int dosend);

void handle_unit_build_city(struct player *pplayer, 
			    struct packet_unit_request *req);
void handle_unit_establish_trade(struct player *pplayer, 
				 struct packet_unit_request *req);
void handle_unit_help_build_wonder(struct player *pplayer, 
				   struct packet_unit_request *req);
void send_remove_unit(struct player *pplayer, int unit_id);

void create_unit(struct player *pplayer, int x, int y, enum unit_type_id type,
		 int make_veteran, int homecity_id);

void handle_unit_disband(struct player *pplayer, 
			 struct packet_unit_request *req);
void player_restore_units(struct player *pplayer);

int unit_on_fortress(struct unit *punit);
int get_defense_power(struct unit *punit);
void raze_city(struct city *pcity);

void wipe_unit(struct player *dest, struct unit *punit);
void kill_unit(struct player *dest, struct unit *punit);


#endif
