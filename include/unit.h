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
#ifndef __UNIT_H
#define __UNIT_H

#include "genlist.h"
#include "shared.h"

struct player;
struct city;

enum unit_type_id { 
  U_ARMOR, U_ARTILLERY, U_BATTLESHIP, U_BOMBER, U_CANNON, U_CARAVAN,
  U_CARRIER, U_CATAPULT, U_CAVALRY, U_CHARIOT, U_CRUISER, U_DIPLOMAT,
  U_FIGHTER, U_FRIGATE, U_IRONCLAD, U_KNIGHTS, U_LEGION, U_MECH,
  U_MILITIA, U_MUSKETEERS, U_NUCLEAR, U_PHALANX, U_RIFLEMEN, U_SAIL, 
  U_SETTLERS, U_SUBMARINE, U_TRANSPORT, U_TRIREME, U_LAST
};

enum unit_activity {
  ACTIVITY_IDLE, ACTIVITY_POLUTION, ACTIVITY_ROAD, ACTIVITY_MINE, 
  ACTIVITY_IRRIGATE, ACTIVITY_FORTIFY, ACTIVITY_FORTRESS, ACTIVITY_SENTRY,
  ACTIVITY_RAILROAD, ACTIVITY_PILLAGE, ACTIVITY_GOTO
};

enum unit_move_type {
  LAND_MOVING, SEA_MOVING, AIR_MOVING
};

enum unit_focus_status {
  FOCUS_AVAIL, FOCUS_WAIT, FOCUS_DONE  
};

enum diplomat_actions {
  DIPLOMAT_BRIBE, DIPLOMAT_EMBASSY, DIPLOMAT_SABOTAGE,
  DIPLOMAT_STEAL, DIPLOMAT_INCITE
};

struct unit {
  enum unit_type_id type;
  int id;
  int owner;
  int x, y;                           
  int veteran;
  int homecity;
  int attacks_left;
  int moves_left;
  int hp;
  int unhappiness;
  int upkeep;
  int fuel;
  int bribe_cost;
  enum unit_activity activity;
  int goto_dest_x, goto_dest_y;
  int activity_count;
  enum unit_focus_status focus_status;
};


struct unit_type {
  char name[MAX_LENGTH_NAME];
  enum unit_move_type move_type;
  int build_cost;
  int no_attacks;
  int attack_strength;
  int defense_strength;
  int move_rate;
  int tech_requirement;
  int vision_range;
  int transport_capacity;
  int hp;
  int firepower;
  int obsoleted_by;
  int fuel;
};


struct unit_list {
  struct genlist list;
};

void unit_list_init(struct unit_list *this);
struct unit *unit_list_get(struct unit_list *this, int index);
struct unit *unit_list_find(struct unit_list *this, int id);
void unit_list_insert(struct unit_list *this, struct unit *punit);
void unit_list_insert_back(struct unit_list *this, struct unit *punit);
int unit_list_size(struct unit_list *this);
void unit_list_unlink(struct unit_list *this, struct unit *punit);
char *unit_name(enum unit_type_id id);


int unit_bribe_cost(struct unit *punit);
int diplomat_can_do_action(struct unit *pdiplomat,
			   enum diplomat_actions action, 
			   int destx, int desty);
struct unit *find_unit_by_id(int id);
int unit_can_help_build_wonder(struct unit *punit, struct city *pcity);
int unit_can_defend_here(struct unit *punit);
int can_unit_move_to_tile(struct unit *punit, int x, int y);
int can_unit_do_activity(struct unit *punit, enum unit_activity activity);
int is_unit_activity_on_tile(enum unit_activity activity, int x, int y);
int unit_value(enum unit_type_id id);
int is_military_unit(struct unit *this_unit);           /* !set !dip !cara */
int is_field_unit(struct unit *this_unit);              /* ships+aero */
void raise_unit_top(struct unit *punit);
int is_water_unit(enum unit_type_id id);
int is_sailing_unit(struct unit *punit);
int is_air_unit(struct unit *punit);
int is_ground_unit(struct unit *punit);
int can_unit_build_city(struct unit *punit);

struct unit_type *get_unit_type(enum unit_type_id id);
char *unit_activity_text(struct unit *punit);
char *unit_description(struct unit *punit);
int is_transporter_with_free_space(struct player *pplayer, int x, int y);
int get_transporter_capacity(struct unit *punit);

void move_unit_list_to_tile(struct unit_list *units, int x, int y);
void transporter_cargo_to_unitlist(struct unit *ptran, struct unit_list *list);

#endif

