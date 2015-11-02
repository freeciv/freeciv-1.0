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
#ifndef __CITYHAND_H
#define __CITYHAND_H

#include "packets.h"
#include "city.h"

void update_city_activities(struct player *pplayer);

void send_city_info(struct player *dest, struct city *pcity, int dosend);
void remove_city(struct city *pcity);

void handle_city_sell(struct player *pplayer, struct packet_city_request *preq);
void handle_city_buy(struct player *pplayer, struct packet_city_request *preq);
void handle_city_change(struct player *pplayer, struct packet_city_request *preq);
void handle_city_make_specialist(struct player *pplayer, 
				 struct packet_city_request *preq);
void handle_city_make_worker(struct player *pplayer, 
			     struct packet_city_request *preq);
void handle_city_change_specialist(struct player *pplayer, 
				   struct packet_city_request *preq);
void handle_city_rename(struct player *pplayer, 
			struct packet_city_request *preq);

void create_city(struct player *pplayer, int x, int y, char *name);
void remove_obsolete_buildings(struct player *plr);
void do_sell_building(struct player *pplayer, struct city *pcity, int id);
void auto_arrange_workers(struct city *pcity);
void advisor_choose_build(struct city *pcity);
void update_polution();
int city_refresh(struct city *pcity);
void city_support(struct city *pcity);
void city_incite_cost(struct city *pcity);
void city_auto_remove_worker(struct city *pcity);
int  add_adjust_workers(struct city *pcity);

#endif
