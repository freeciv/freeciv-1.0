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

#include "player.h"
#include "unithand.h"
#include "packets.h"
#include "civserver.h"
#include "map.h"
#include "maphand.h"
#include "cityhand.h"
#include "unit.h"
#include "plrhand.h"
#include "city.h"
#include "log.h"
#include "mapgen.h"

extern struct advance advances[];

void update_unit_activity(struct player *pplayer, struct unit *punit);
void handle_unit_enter_city(struct player *pplayer, struct city *pcity);
void unit_restore_hitpoints(struct player *pplayer, struct unit *punit);
void unit_restore_movepoints(struct player *pplayer, struct unit *punit);
int hp_gain_coord(struct unit *punit);
void do_unit_goto(struct player *pplayer, struct unit *punit);
long lrand48(void);
int unit_ignores_citywalls(struct unit *punit);


/* May wish to make this global */
static
char* n_if_vowel(char ch)
{
	if (strchr("AEIOUaeiou",ch))
		return "n";
	else
		return "";
}

/**************************************************************************
...
**************************************************************************/
void handle_unit_goto_tile(struct player *pplayer, 
			   struct packet_unit_request *req)
{
  struct unit *punit;
  
  if((punit=unit_list_find(&pplayer->units, req->unit_id))) {
    punit->goto_dest_x=req->x;
    punit->goto_dest_y=req->y;

    punit->activity=ACTIVITY_GOTO;
    punit->activity_count=0;

    send_unit_info(0, punit, 0);
      
    do_unit_goto(pplayer, punit);  
  }
    
  
}

/**************************************************************************
...
**************************************************************************/
int unit_bribe_cost(struct unit *punit)
{  
  int cost;
  struct city *capital;
  int dist;
  cost=20*get_unit_type(punit->type)->build_cost;
  capital=find_palace(&game.players[punit->owner]);
  if (capital)
    dist=min(50, map_distance(capital->x, capital->y, punit->x, punit->y)); 
  else
    dist=50;
  cost=(cost*(100-dist))/100;
  return cost;
}

/***************************************************************
...
***************************************************************/
void diplomat_bribe(struct player *pplayer, struct unit *pdiplomat, struct unit *pvictim)
{
  if(pplayer->economic.gold>=pvictim->bribe_cost) {
    if(game.players[pvictim->owner].goverment==G_DEMOCRACY)
      notify_player(pplayer, "Game: You can't bribe a unit from a democratic nation.");
    else {
      pplayer->economic.gold-=pvictim->bribe_cost;
      notify_player(&game.players[pvictim->owner], 
		    "Game: One of your units was bribed!");
      notify_player(pplayer, "Game: Succeeded in bribing the enemy unit.");
      
      create_unit(pplayer, pvictim->x, pvictim->y,
		  pvictim->type, pvictim->veteran, pdiplomat->homecity);
      send_remove_unit(0, pvictim->id);
      game_remove_unit(pvictim->id);
      pdiplomat->moves_left=0;
      send_unit_info(pplayer, pdiplomat, 0);
      send_player_info(pplayer, pplayer);
    }
  }
}

void diplomat_get_tech(struct player *pplayer, struct unit *pdiplomat, struct city  *city)
{
  int tec;
  int i;
  int j=0;
  struct player *target=&game.players[city->owner];
  if (pplayer==target)
    return;
  
  for (i=1;i<A_LAST;i++) {
    if (get_invention(pplayer, i)!=TECH_KNOWN && get_invention(target, i)== TECH_KNOWN) {
      j++;
    }
  }
  if (!j) 
    return;
  j=(lrand48()%j)+1;
  for (i=1;i<A_LAST;i++) {
    if (get_invention(pplayer, i)!=TECH_KNOWN && 
	get_invention(target, i)== TECH_KNOWN) 
      j--;
    if (!j) break;
  }
  if (i==A_LAST) {
    printf("Bug in diplomat_a_tech\n");
    return;
  }
  set_invention(pplayer, i, TECH_KNOWN);
  pplayer->research.researchpoints++;
  notify_player(pplayer, "Game: Your diplomat stole %s from %s",
		advances[i].name, target->name); 
  notify_player(target, "Game: %s diplomat stole %s in the city.", 
		pplayer->name, advances[i].name); 
  if (pplayer->research.researching==i) {
    tec=pplayer->research.researched;
    choose_random_tech(pplayer);
    pplayer->research.researched=tec;
  }
  send_remove_unit(0, pdiplomat->id);
  game_remove_unit(pdiplomat->id);
}
void diplomat_incite(struct player *pplayer, struct unit *pdiplomat, struct city *pcity)
{
  struct player *cplayer;
  struct city *pnewcity, *pc2;
  int i;
  if (!pcity)
    return;
  cplayer=city_owner(pcity);
  if (cplayer==pplayer || cplayer==NULL) 
    return;
  if(game.players[cplayer->player_no].goverment==G_DEMOCRACY) {
      notify_player(pplayer, "Game: You can't subvert a city from a democratic nation.");
      return;
  }
  if (pplayer->economic.gold<pcity->incite_revolt_cost) 
    return;
  pplayer->economic.gold-=pcity->incite_revolt_cost;
  if (pcity->size >1) {
    pcity->size--;
    city_auto_remove_worker(pcity);
  }
  notify_player(pplayer, "Game: Revolt incited in %s, you now rule the city!", pcity->name);
  notify_player(cplayer, "Game: %s has revolted, %s influence suspected", pcity->name, get_race_name(pplayer->race));
  pnewcity=(struct city *)malloc(sizeof(struct city));
  *pnewcity=*pcity;
  remove_city(pcity);
  for (i=0;i<4;i++) {
    pc2=find_city_by_id(pnewcity->trade[i]);
    if (can_establish_trade_route(pnewcity, pc2))    
      establish_trade_route(pnewcity, pc2);
  }
  pnewcity->id=get_next_id_number();
  pnewcity->owner=pplayer->player_no;
  unit_list_init(&pnewcity->units_supported);
  city_list_insert(&pplayer->cities, pnewcity);
  map_set_city(pnewcity->x, pnewcity->y, pnewcity);
  raze_city(pcity);
  city_refresh(pnewcity);
  send_city_info(0, pnewcity, 0);
  send_player_info(pplayer, pplayer);
  send_remove_unit(0, pdiplomat->id);
  game_remove_unit(pdiplomat->id);
}

void diplomat_sabotage(struct player *pplayer, struct unit *pdiplomat, struct city *pcity)
{
  struct player *cplayer;
  char *prod;
  if (!pcity)
    return;
  cplayer=city_owner(pcity);
  if (cplayer==pplayer ||cplayer==NULL) return;
  switch (lrand48()%2) {
  case 0:
    pcity->shield_stock=0;
    if (pcity->is_building_unit) 
      prod=unit_name(pcity->currently_building);
    else
      prod=building_name(pcity->currently_building);
    notify_player(pplayer, 
    "Game: Your Diplomat succeeded destroying the production of %s in %s", 
    prod, pcity->name);
    notify_player(cplayer, "Game: The production of %s was destroyed in %s, %s are suspected for the sabotage.", prod, pcity->name, get_race_name_plural(cplayer->race));

    break;
  case 1:
    {
      int building;
      int i;
      for (i=0;i<10;i++) {
	building=lrand48()%B_LAST;
	if (city_got_building(pcity, building) 
	    && !is_wonder(building) && building!=B_PALACE) {
	  pcity->improvements[building]=0;
	  break;
	}
      }
      if (i<10) {
	notify_player(pplayer, "Game: Your Diplomat destroyed %s in %s.", 
		      building_name(building), pcity->name);
	notify_player(cplayer, "Game: The %s sabotaged %s in %s.", 
		      building_name(building), pcity->name);
      } else {
	notify_player(pplayer, "Game: Your Diplomat was caught in the attempt of industrial sabotage!");
	notify_player(cplayer, "Game: You caught a%s %s diplomat in industrial sabotage!",
		n_if_vowel(get_race_name(cplayer->race)[0]),
		get_race_name(cplayer->race));
      }
    }
    break;
  }
  send_city_info(cplayer, pcity, 1);

  send_remove_unit(0, pdiplomat->id);
  game_remove_unit(pdiplomat->id);
}

/***************************************************************
...
***************************************************************/
void handle_diplomat_action(struct player *pplayer, 
			    struct packet_diplomat_action *packet)
{
  struct unit *pdiplomat=unit_list_find(&pplayer->units, packet->diplomat_id);
  struct unit *pvictim=find_unit_by_id(packet->target_id);
  struct city *pcity=find_city_by_id(packet->target_id);
  
  if(pdiplomat && pdiplomat->moves_left>0) {
    pdiplomat->moves_left=0;
    send_unit_info(pplayer, pdiplomat, 0);
    switch(packet->action_type) {
     case DIPLOMAT_BRIBE:
       if(pvictim && diplomat_can_do_action(pdiplomat, DIPLOMAT_BRIBE,
					  pvictim->x, pvictim->y))
	 diplomat_bribe(pplayer, pdiplomat, pvictim);
      break;
     case DIPLOMAT_SABOTAGE:
       if(pcity && diplomat_can_do_action(pdiplomat, DIPLOMAT_SABOTAGE, 
					  pcity->x, pcity->y))
	 diplomat_sabotage(pplayer, pdiplomat, pcity);
       break;
     case DIPLOMAT_EMBASSY:
       if(pcity && diplomat_can_do_action(pdiplomat, DIPLOMAT_EMBASSY, 
					  pcity->x, pcity->y))
	 pplayer->embassy|=(1<<pcity->owner);
         send_player_info(pplayer, pplayer);
      break;
     case DIPLOMAT_INCITE:
       if(pcity && diplomat_can_do_action(pdiplomat, DIPLOMAT_INCITE, 
					  pcity->x, pcity->y))
	 diplomat_incite(pplayer, pdiplomat, pcity);
      break;
     case DIPLOMAT_STEAL:
       if(pcity && diplomat_can_do_action(pdiplomat, DIPLOMAT_STEAL, 
					  pcity->x, pcity->y))
	 diplomat_get_tech(pplayer, pdiplomat, pcity);
      break;
    }
  }
}

/***************************************************************
...
***************************************************************/
void player_restore_units(struct player *pplayer)
{
  struct genlist_iterator myiter, myiter2;
  struct unit *punit;
  
  genlist_iterator_init(&myiter, &pplayer->units.list, 0);

  for(; ITERATOR_PTR(myiter);) {
    punit=(struct unit *)ITERATOR_PTR(myiter);
    ITERATOR_NEXT(myiter);
    unit_restore_hitpoints(pplayer, punit);
    unit_restore_movepoints(pplayer, punit);
    
    punit->attacks_left=get_unit_type(punit->type)->no_attacks;
    
    if(is_air_unit(punit)) {
      punit->fuel--;
      if(map_get_city(punit->x, punit->y))
	punit->fuel=get_unit_type(punit->type)->fuel;
      else {
	genlist_iterator_init(&myiter2, 
			      &map_get_tile(punit->x, punit->y)->units.list,0);
	for (;ITERATOR_PTR(myiter2);ITERATOR_NEXT(myiter2)) {
	  if (((struct unit*)ITERATOR_PTR(myiter2))->type==U_CARRIER)
	    punit->fuel=get_unit_type(punit->type)->fuel;
	}
      }
      if(punit->fuel<=0) {
	send_remove_unit(0, punit->id);
	game_remove_unit(punit->id);
	notify_player(pplayer, "Game: Your %s has run out of fuel",
		unit_name(punit->type));
      }
    } else if (punit->type==U_TRIREME) {
      if (!is_coastline(punit->x, punit->y) && !(lrand48()%2)) {
	wipe_unit(pplayer, punit);
	notify_player(pplayer, "Game: Your Trireme has been lost on the high seas");
      }
    }
  }
}

/***************************************************************
...
***************************************************************/
void unit_restore_hitpoints(struct player *pplayer, struct unit *punit)
{
  struct city *pcity;
  
  punit->hp+=hp_gain_coord(punit);
  
  pcity=city_list_find_id(&pplayer->cities, game.global_wonders[B_UNITED]);

  if(pcity && !wonder_is_obsolete(B_UNITED))
    punit->hp++;
    
  if(punit->hp>get_unit_type(punit->type)->hp)
    punit->hp=get_unit_type(punit->type)->hp;
}
  

/***************************************************************
...
***************************************************************/
void unit_restore_movepoints(struct player *pplayer, struct unit *punit)
{
  punit->moves_left=get_unit_type(punit->type)->move_rate;
  
  if(is_sailing_unit(punit)) {
    struct city *pcity;
    
    pcity=city_list_find_id(&pplayer->cities, 
			    game.global_wonders[B_LIGHTHOUSE]);
    if(pcity && !wonder_is_obsolete(B_LIGHTHOUSE)) 
      punit->moves_left+=3;
    
    pcity=city_list_find_id(&pplayer->cities, 
			    game.global_wonders[B_MAGELLAN]);
    if(pcity && !wonder_is_obsolete(B_MAGELLAN)) 
      punit->moves_left+=3;
  }
}


/***************************************************************
...
***************************************************************/
int hp_gain_coord(struct unit *punit)
{
  int hp=1;
  struct city *pcity;
  if (unit_on_fortress(punit))
    hp=get_unit_type(punit->type)->hp/4;
  if((pcity=game_find_city_by_coor(punit->x,punit->y))) {
    if (city_got_barracks(pcity)) {
      hp=get_unit_type(punit->type)->hp;
    }
    else
      hp=get_unit_type(punit->type)->hp/3;
  }
  if(punit->activity==ACTIVITY_FORTRESS)
    hp++;
  
  return hp;
}



/**************************************************************************
... this is a crude function!!!! pikeman should be done here in time to come!
**************************************************************************/
int rate_unit(struct unit *punit, struct unit *against)
{
  int val;
  struct city *pcity=map_get_city(punit->x, punit->y);
  if(punit)
    val=get_defense_power(punit);
  if (pcity && !unit_ignores_citywalls(against))
    val*=3;
  else if (pcity || unit_on_fortress(punit))
    val*=2;
  else if (punit->activity==ACTIVITY_FORTIFY)
    val*=1.5;
  return val*100+punit->hp;
}

/**************************************************************************
get best defending unit which is NOT owned by pplayer
**************************************************************************/
struct unit *get_defender(struct player *pplayer, struct unit *aunit, int x, int y)
{
  struct unit_list *punit_list;
  struct unit *punit;
  struct unit *bestdef;
  int bestvalue=-1;
  struct genlist_iterator myiter;

  punit_list=&map_get_tile(x, y)->units;
  if (!(punit=unit_list_get(punit_list, 0))) 
    return 0;
  if (pplayer->player_no==punit->owner)
    return 0;
  genlist_iterator_init(&myiter, &punit_list->list, 0);
  bestdef=0;
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    punit=(struct unit *)ITERATOR_PTR(myiter);
    if(unit_can_defend_here(punit) && rate_unit(punit, aunit)>bestvalue) {
      bestvalue=rate_unit(punit, aunit);
      bestdef=punit;
    }
  }
    return bestdef;
}

/**************************************************************************
...
**************************************************************************/
int get_attack_power(struct unit *punit)
{
  int power;
  power=get_unit_type(punit->type)->attack_strength*10;
  if (punit->veteran)
    power*=1.5;
  if (punit->moves_left==1)
    return power/3;
  if (punit->moves_left==2)
    return (power*2)/3;
  if (!power) power=10;
  return power;
}

/**************************************************************************
...
**************************************************************************/
int get_defense_power(struct unit *punit)
{
  int power;
  int terra;
  struct city *pcity;
  pcity=game_find_city_by_coor(punit->x, punit->y);
  if (!punit || punit->type<0 || punit->type>=U_LAST)
    abort();
  power=get_unit_type(punit->type)->defense_strength*10;
  if (is_sailing_unit(punit) && pcity && power>10)  /* caught in port */
    power=10;
  if (punit->veteran)
    power*=1.5;
  
  terra=map_get_terrain(punit->x, punit->y);
  power=(power*get_tile_type(terra)->defense_bonus)/10;
  if (!power) power=10;
  return power;
}

/**************************************************************************
...
**************************************************************************/
int unit_ignores_citywalls(struct unit *punit)
{
  switch (punit->type) {
  case U_BOMBER:
  case U_ARTILLERY:
    return 1;
  default:
    return 0;
  }
}

/**************************************************************************
...
**************************************************************************/
int unit_behind_walls(struct unit *punit)
{
  struct city *pcity;
  
  if((pcity=game_find_city_by_coor(punit->x,punit->y)))
    return city_got_citywalls(pcity);
  
  return 0;
}

/**************************************************************************
...
**************************************************************************/
int unit_on_fortress(struct unit *punit)
{
  return (map_get_special(punit->x, punit->y)&S_FORTRESS);
}
int unit_behind_coastal(struct unit *punit)
{
  struct city *pcity;
  return ((pcity=game_find_city_by_coor(punit->x, punit->y)) && city_got_building(pcity, B_COASTAL));
}

/**************************************************************************
...
**************************************************************************/
void unit_versus_unit(struct unit *attacker, struct unit *defender)
{
  int attackpower=get_attack_power(attacker);
  int defensepower=get_defense_power(defender);
  if (unit_behind_coastal(defender) && is_sailing_unit(attacker))
    defensepower*=4;
  else  if (!unit_ignores_citywalls(attacker) && unit_behind_walls(defender)) 
    defensepower*=3;
  else if (unit_on_fortress(defender) || 
	   map_get_city(defender->x, defender->y)) 
    defensepower*=2;
  else if (defender->activity == ACTIVITY_FORTIFY)
    defensepower*=1.5;
  log(LOG_DEBUG, "attack:%d, defense:%d\n", attackpower, defensepower);
  while (attacker->hp>0 && defender->hp>0) {
    if (lrand48()%(attackpower+defensepower)>defensepower) {
      defender->hp=defender->hp-get_unit_type(attacker->type)->firepower;
    } else
      attacker->hp=attacker->hp-get_unit_type(defender->type)->firepower;
  }
  if (attacker->hp<0) attacker->hp=0;
  if (defender->hp<0) defender->hp=0;

  if (attacker->hp && !attacker->veteran) /* might make surviver veteran */
    attacker->veteran=lrand48()%2;
  if (defender->hp && !defender->veteran) 
    defender->veteran=lrand48()%2;
}

/**************************************************************************
...
**************************************************************************/
void create_unit(struct player *pplayer, int x, int y, enum unit_type_id type,
		 int make_veteran, int homecity_id)
{
  struct unit *punit;
  struct city *pcity;
  punit=(struct unit *)malloc(sizeof(struct unit));
  punit->type=type;
  punit->id=get_next_id_number();
  punit->owner=pplayer->player_no;
  punit->x=x;
  punit->y=y;
  if (is_sailing_unit(punit) || is_air_unit(punit))
    punit->veteran=0;
  else
    punit->veteran=make_veteran;
  punit->homecity=homecity_id;
  punit->attacks_left=get_unit_type(type)->no_attacks;
  punit->moves_left=get_unit_type(type)->move_rate;
  punit->hp=get_unit_type(punit->type)->hp;
  punit->activity=ACTIVITY_IDLE;
  punit->activity_count=0;
  punit->upkeep=0;
  punit->unhappiness=0;
  punit->fuel=get_unit_type(punit->type)->fuel;
  
  unit_list_insert(&pplayer->units, punit);
  unit_list_insert(&map_get_tile(x, y)->units, punit);
  if((pcity=game_find_city_by_id(punit->homecity)))
    unit_list_insert(&pcity->units_supported, punit);
  punit->bribe_cost=unit_bribe_cost(punit);
  send_unit_info(0, punit, 0);
}

/**************************************************************************
...
**************************************************************************/
void handle_unit_change_homecity(struct player *pplayer, 
				 struct packet_unit_request *req)
{
  struct unit *punit;
  
  if((punit=unit_list_find(&pplayer->units, req->unit_id))) {
    struct city *pcity;
    if((pcity=city_list_find_id(&pplayer->cities, req->city_id))) {
      unit_list_insert(&pcity->units_supported, punit);
      
      if((pcity=city_list_find_id(&pplayer->cities, punit->homecity)))
	unit_list_unlink(&pcity->units_supported, punit);
      
      punit->homecity=req->city_id;
      send_unit_info(pplayer, punit, 0);
    }
  }
}

/**************************************************************************
...
**************************************************************************/
void handle_unit_disband(struct player *pplayer, 
			 struct packet_unit_request *req)
{
  struct unit *punit;
  struct city *pcity;
  /* give 1/2 of the worth of the unit, to the currently builded thing 
     have to be on the location of the city_square
   */
  if((punit=unit_list_find(&pplayer->units, req->unit_id))) {
    if ((pcity=map_get_city(punit->x, punit->y))) {
      pcity->shield_stock+=(get_unit_type(punit->type)->build_cost/2);
      send_city_info(pplayer, pcity, 0);
    }
    wipe_unit(pplayer, punit);
  }
}


/**************************************************************************
...
**************************************************************************/
void handle_unit_build_city(struct player *pplayer, 
			    struct packet_unit_request *req)
{
  struct unit *punit;
  char *name;
  struct city *pcity;
  if((punit=unit_list_find(&pplayer->units, req->unit_id))) {
    if (punit->type!=U_SETTLERS) {
      notify_player(pplayer, "Game: You need a settler to build a city.");
      return;
    }  
    
    if ((pcity=map_get_city(punit->x, punit->y))) {
      if (pcity->size>8) {
	notify_player(pplayer, "Game: Your settlers doesn't feel comfortable here.");
	return;
      }
      else {
	pcity->size++;
	  if (!add_adjust_workers(pcity))
	    auto_arrange_workers(pcity);
	send_remove_unit(0, req->unit_id);
	game_remove_unit(req->unit_id);
	send_city_info(pplayer, pcity, 0);
	notify_player(pplayer, "Game: Settlers added to aid %s in growing", pcity->name);
	return;
      }
    }
    
    if(can_unit_build_city(punit)) {
      if(!(name=get_sane_name(req->name))) {
	notify_player(pplayer, 
		      "Game: Let's not build a city with such a stupid name.");
	return;
      }

      send_remove_unit(0, req->unit_id);
      map_set_special(punit->x, punit->y, S_ROAD);
      send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
      create_city(pplayer, punit->x, punit->y, name);
      game_remove_unit(req->unit_id);
    } else
      notify_player(pplayer, "Game: Can't place city here.");
  }
}



/**************************************************************************
...
**************************************************************************/
void send_remove_unit(struct player *pplayer, int unit_id)
{
  int o;
  
  struct packet_generic_integer packet;

  packet.value=unit_id;

  for(o=0; o<game.nplayers; o++)           /* dests */
    if(!pplayer || &game.players[o]==pplayer)
      send_packet_generic_integer(game.players[o].conn, PACKET_REMOVE_UNIT,
				  &packet);
}



/**************************************************************************
...
**************************************************************************/
void update_unit_activities(struct player *pplayer)
{
  struct genlist_iterator myiter;
  genlist_iterator_init(&myiter, &pplayer->units.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    update_unit_activity(pplayer, (struct unit *)ITERATOR_PTR(myiter));
}


/**************************************************************************
...
**************************************************************************/
void set_unit_activity(struct unit *punit, enum unit_activity new_activity)
{
  punit->activity=new_activity;
  punit->activity_count=0;
}



/**************************************************************************
...
**************************************************************************/
void update_unit_activity(struct player *pplayer, struct unit *punit)
{
  punit->activity_count++;

   if(punit->activity==ACTIVITY_PILLAGE && punit->activity_count>=1) {
      if(map_get_special(punit->x, punit->y)&S_IRRIGATION)
	map_clear_special(punit->x, punit->y, S_IRRIGATION);
      else if(map_get_special(punit->x, punit->y)&S_RAILROAD)
	map_clear_special(punit->x, punit->y, S_RAILROAD);
      else 
	map_clear_special(punit->x, punit->y, S_ROAD);
    send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
    set_unit_activity(punit, ACTIVITY_IDLE);
   }

  if(punit->activity==ACTIVITY_POLUTION && punit->activity_count>=3) {
    map_clear_special(punit->x, punit->y, S_POLUTION);
    send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
    set_unit_activity(punit, ACTIVITY_IDLE);
  }

  if(punit->activity==ACTIVITY_FORTRESS && punit->activity_count>=3) {
    map_set_special(punit->x, punit->y, S_FORTRESS);
    send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
    set_unit_activity(punit, ACTIVITY_IDLE);
  }
  
  if(punit->activity==ACTIVITY_IRRIGATE && 
     punit->activity_count>=map_build_irrigation_time(punit->x, punit->y)) {
    map_irrigate_tile(punit->x, punit->y);
    send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
    punit->activity=ACTIVITY_IDLE;
    set_unit_activity(punit, ACTIVITY_IDLE);
  }

  if(punit->activity==ACTIVITY_ROAD && 
     punit->activity_count>map_build_road_time(punit->x, punit->y)) {
    map_set_special(punit->x, punit->y, S_ROAD);
    send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
    punit->activity=ACTIVITY_IDLE;
    set_unit_activity(punit, ACTIVITY_IDLE);
  }

  if(punit->activity==ACTIVITY_RAILROAD && punit->activity_count>=3) {
    map_set_special(punit->x, punit->y, S_RAILROAD);
    send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
    punit->activity=ACTIVITY_IDLE;
    handle_unit_activity_request(pplayer, punit, ACTIVITY_IDLE);
    set_unit_activity(punit, ACTIVITY_IDLE);
  }
  
  if(punit->activity==ACTIVITY_MINE && 
     punit->activity_count>=map_build_mine_time(punit->x, punit->y)) {
    map_mine_tile(punit->x, punit->y);
    send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
    set_unit_activity(punit, ACTIVITY_IDLE);
  }

  if(punit->activity==ACTIVITY_GOTO) {
    do_unit_goto(pplayer, punit);
    return;
  }
  
  if(punit->activity==ACTIVITY_IDLE && 
     map_get_terrain(punit->x, punit->y)==T_OCEAN &&
     is_ground_unit(punit))
    set_unit_activity(punit, ACTIVITY_SENTRY);

  
  
  send_unit_info(0, punit, 0);
}


/**************************************************************************
...
**************************************************************************/
void handle_unit_info(struct player *pplayer, struct packet_unit_info *pinfo)
{
  struct unit *punit;

  punit=unit_list_find(&pplayer->units, pinfo->id);

  if(punit) {
    if(punit->activity!=pinfo->activity)
      handle_unit_activity_request(pplayer, punit, pinfo->activity);
    else if(punit->x!=pinfo->x || punit->y!=pinfo->y)
      handle_unit_move_request(pplayer, punit, pinfo->x, pinfo->y);
  }
}

/**************************************************************************
...
**************************************************************************/
void do_nuke_tile(int x, int y)
{
  struct unit_list *punit_list;
  struct unit *punit;
  struct city *pcity;
  struct genlist_iterator myiter;
  punit_list=&map_get_tile(x, y)->units;
  
  genlist_iterator_init(&myiter, &punit_list->list, 0);
  
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    punit=(struct unit *)ITERATOR_PTR(myiter);
    send_remove_unit(0, punit->id);
    game_remove_unit(punit->id);
  }

  if((pcity=game_find_city_by_coor(x,y))) {
    pcity->size/=2;
    auto_arrange_workers(pcity);
    send_city_info(0, pcity, 0);
  }
  else if ((map_get_terrain(x,y)!=T_OCEAN && map_get_terrain(x,y)<=T_TUNDRA) &&
           (!(map_get_special(x,y)&S_POLUTION)) && lrand48()%2) { 
    map_set_special(x,y, S_POLUTION);
    send_tile_info(0, x, y, TILE_KNOWN);
  }
}

/**************************************************************************
...
**************************************************************************/
void do_nuclear_explosion(int x, int y)
{
  int i,j;
  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      do_nuke_tile(x+i-1,y+j-1);
}

int sdi_defense_close(int x, int y)
{
  struct city *pcity;
  int lx, ly;
  for (lx=x-2;lx<x+3;lx++)
    for (ly=y-2;ly<y+3;ly++) {
      pcity=game_find_city_by_coor(lx,ly);
      if (pcity && city_got_building(pcity, B_SDI))
	return 1;
    }
  return 0;

}

/**************************************************************************
...
**************************************************************************/
void handle_unit_attack_request(struct player *pplayer, struct unit *punit,
				struct unit *pdefender)
{
  int o;
  struct packet_unit_combat combat;
  struct unit *plooser, *pwinner;
  struct city *pcity;
  punit->attacks_left--;
  punit->moves_left-=3;
    
  if(punit->moves_left<0)
    punit->moves_left=0;
  

  if(punit->type==U_NUCLEAR) {
    struct packet_nuke_tile packet;
    
    packet.x=pdefender->x;
    packet.y=pdefender->y;
    if (sdi_defense_close(pdefender->x, pdefender->y)) {
      notify_player(pplayer, "Game: Your Nuclear missile has been shot down by SDI, what a waste.");
      notify_player(&game.players[pcity->owner], "Game: The nuclear attack on %s was avoided by your SDI defense",pcity->name); 
      send_remove_unit(0, punit->id);
      game_remove_unit(punit->id);
      return;
    } 

    for(o=0; o<game.nplayers; o++)
      send_packet_nuke_tile(game.players[o].conn, &packet);
    
    do_nuclear_explosion(pdefender->x, pdefender->y);
    send_remove_unit(0, punit->id);
    game_remove_unit(punit->id);
    return;
  }
  
  unit_versus_unit(punit, pdefender);

  if (punit->hp && (pcity=map_get_city(pdefender->x, pdefender->y)) && pcity->size>1 && !city_got_citywalls(pcity)) {
    pcity->size--;
    city_auto_remove_worker(pcity);
    city_refresh(pcity);
    send_city_info(0, pcity, 0);
  }

  
  pwinner=(punit->hp) ?     punit : pdefender;
  plooser=(pdefender->hp) ? punit : pdefender;
    
  combat.attacker_unit_id=punit->id;
  combat.defender_unit_id=pdefender->id;
  combat.attacker_hp=punit->hp;
  combat.defender_hp=pdefender->hp;
  combat.make_winner_veteran=pwinner->veteran;
  
  for(o=0; o<game.nplayers; o++)
    if(map_get_known(punit->x, punit->y, &game.players[o]) ||
       map_get_known(pdefender->x, pdefender->y, &game.players[o]))
      send_packet_unit_combat(game.players[o].conn, &combat);
  
  if(punit==plooser) {
    notify_player(&game.players[pwinner->owner], "Game: You survived the pathetic attack from the %s.", races[game.players[plooser->owner].race].name);
    wipe_unit(pplayer, plooser);
  }
  else {
    kill_unit(pplayer, plooser);
  }
  /* send winner unit, since it might have changed #attacks_left, mp */
  send_unit_info(0, pwinner, 0);
}

/**************************************************************************
...
**************************************************************************/
int find_a_unit_type(struct city *pcity)
{
  return U_CAVALRY;
}

/**************************************************************************
...
**************************************************************************/
int can_unit_attack_tile(struct unit *punit, int dest_x, int dest_y)
{
  struct unit *pdefender;
  int fromtile=map_get_terrain(punit->x, punit->y);
  int totile=map_get_terrain(dest_x, dest_y);

  if(!is_military_unit(punit))
    return 0;
  
  pdefender=get_defender(&game.players[punit->owner], punit, dest_x, dest_y);
    /*only fighters can attack planes, except for city attacks */
  if (punit->type!=U_FIGHTER && is_air_unit(pdefender) && !map_get_city(dest_x, dest_y)) {
    return 0;
  }
  /* can't attack with ground unit from ocean */
  if(fromtile==T_OCEAN && is_ground_unit(punit)) {
    return 0;
  }

  if(fromtile!=T_OCEAN && totile==T_OCEAN && is_ground_unit(punit)) {
    return 0;
  }
  
  /* Shore bombardement */
  else if (fromtile==T_OCEAN && is_sailing_unit(punit) && totile!=T_OCEAN) {
    switch (punit->type) {
    case U_BATTLESHIP:
    case U_CRUISER:
    case U_IRONCLAD:
    case U_TRIREME:
    case U_FRIGATE:
      break;
    default:
      return 0;
    }
  }
  return 1;
}


/**************************************************************************
...
**************************************************************************/
void handle_unit_enter_hut(struct unit *punit)
{
  struct player *pplayer=&game.players[punit->owner];
  if (is_air_unit(punit))
    return;
  map_get_tile(punit->x, punit->y)->special^=S_HUT;
  
  send_tile_info(0, punit->x, punit->y, TILE_KNOWN);
  
  switch (lrand48()%13) {
  case 0:
    notify_player(pplayer, "Game: You found 25 credits.");
    pplayer->economic.gold+=25;
    break;
  case 1:
  case 2:
  case 3:
    notify_player(pplayer, "Game: You found 50 credits.");
    pplayer->economic.gold+=50;
    break;
  case 4:
    notify_player(pplayer, "Game: You found 100 credits"); 
    pplayer->economic.gold+=100;
    break;
  case 5:
  case 6:
  case 7:
/*this function is hmmm a hack */
    notify_player(pplayer, "Game: You found ancient scrolls of wisdom."); 
    {
      int res=pplayer->research.researched;
      int wasres=pplayer->research.researching;
      choose_random_tech(pplayer);
      update_tech(pplayer, 1000000); /*hum*/
      if (get_invention(pplayer,wasres)==TECH_KNOWN) {
	choose_random_tech(pplayer);
	pplayer->research.researched=res;
      }  else {
	pplayer->research.researched=res;
	pplayer->research.researching=wasres;
      }
    }
    break;
  case 8:
  case 9:
    notify_player(pplayer, "Game: A band of friendly mercenaries joins your cause.");
    create_unit(pplayer, punit->x, punit->y, find_a_unit_type(0), 0, punit->homecity);
    break;
  case 10:
    notify_player(pplayer, "Game: An abandoned village is here.");
    break;
  case 11:
    notify_player(pplayer, "Game: Friendly nomads are impressed by you, and join you");
    create_unit(pplayer, punit->x, punit->y, U_SETTLERS, 0, punit->homecity);
    break;
  case 12:
    notify_player(pplayer, "Game: Your unit has been cowardly slaughtered by a band of barbarians");
    wipe_unit(pplayer, punit);
    break;
  }
  send_player_info(pplayer, pplayer);
}

/**************************************************************************
...
**************************************************************************/
int try_move_unit(struct unit *punit, int dest_x, int dest_y) 
{
  if (lrand48()%(1+map_move_cost(punit, dest_x, dest_y))>punit->moves_left && punit->moves_left<get_unit_type(punit->type)->move_rate) {
    punit->moves_left=0;
    send_unit_info(&game.players[punit->owner], punit, 0);
  }
  return punit->moves_left;
}

/**************************************************************************
...
**************************************************************************/

void handle_unit_move_request(struct player *pplayer, struct unit *punit,
			      int dest_x, int dest_y)
{
  int unit_id;
  struct unit *pdefender;
  struct unit_list cargolist;
  
  unit_id=punit->id;
  pdefender=get_defender(pplayer, punit, dest_x, dest_y);

  if(pdefender && pdefender->owner!=punit->owner) {
    if(can_unit_attack_tile(punit,dest_x , dest_y)) {
      if(punit->attacks_left<=0)
	notify_player(pplayer, "Game: This unit has no attacks left.");
      else if(punit->moves_left<=0)
	notify_player(pplayer, "Game: This unit has no moves left.");
      else
	  handle_unit_attack_request(pplayer, punit, pdefender);
    } else
      notify_player(pplayer, "Game: You can't attack there.");
  }
  else if(can_unit_move_to_tile(punit, dest_x, dest_y) && try_move_unit(punit, dest_x, dest_y)) {
    int src_x, src_y;
    struct city *pcity;
    
    if((pcity=map_get_city(dest_x, dest_y))) {
      if ((pcity->owner!=punit->owner && (is_air_unit(punit) || 
					  !is_military_unit(punit)))) {
	notify_player(pplayer, "Game: Only ground troops can take over a city.");
	return;
      }
    }

    if(!unit_list_find(&pplayer->units, unit_id))
      return; /* diplomat or caravan action killed unit */

    
    /* light the squares the unit is entering */
    light_square(pplayer, dest_x, dest_y, 
		 get_unit_type(punit->type)->vision_range);
    
    /* ok now move the unit */

    src_x=punit->x;
    src_y=punit->y;

    unit_list_unlink(&map_get_tile(src_x, src_y)->units, punit);

    if(get_transporter_capacity(punit)) {
      struct genlist_iterator myiter;
      transporter_cargo_to_unitlist(punit, &cargolist);
      genlist_iterator_init(&myiter, &cargolist.list, 0);
      for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
	struct unit *pcarried=(struct unit *)ITERATOR_PTR(myiter);
	pcarried->x=dest_x;
	pcarried->y=dest_y;
	send_unit_info(0, pcarried, 1);
      }
    }
      
    punit->x=dest_x;
    punit->y=dest_y;
    
    if((punit->moves_left-=map_move_cost(punit, dest_x, dest_y))<0)
      punit->moves_left=0;

    send_unit_info(0, punit, 1);
    
    unit_list_insert(&map_get_tile(dest_x, dest_y)->units, punit);

    if(get_transporter_capacity(punit)) {
      move_unit_list_to_tile(&cargolist, punit->x, punit->y);
      genlist_unlink_all(&cargolist.list); 
    }
      
    /* ok entered new tile */
    
    if(pcity)
      handle_unit_enter_city(pplayer, pcity);

    if((map_get_tile(dest_x, dest_y)->special&S_HUT))
      handle_unit_enter_hut(punit);

  }
}

/**************************************************************************
...
**************************************************************************/
void raze_city(struct city *pcity)
{
  int i;
  pcity->improvements[B_PALACE]=0;
  for (i=0;i<B_LAST;i++) {
    if (city_got_building(pcity, i) && !is_wonder(i) 
	&& lrand48()%2) {
      pcity->improvements[i]=0;
    }
  }
  pcity->shield_stock=0;
  /*  advisor_choose_build(pcity);  we add the civ bug here :)*/
}

/**************************************************************************
...
**************************************************************************/

void get_a_tech(struct player *pplayer, struct player *target)
{
  int tec;
  int i;
  int j=0;
  for (i=0;i<A_LAST;i++) {
    if (get_invention(pplayer, i)!=TECH_KNOWN && get_invention(target, i)== TECH_KNOWN) {
      j++;
    }
  }
  if (!j) 
    return;
  j=(lrand48()%j)+1;
  for (i=0;i<A_LAST;i++) {
    if (get_invention(pplayer, i)!=TECH_KNOWN && 
	get_invention(target, i)== TECH_KNOWN) 
      j--;
    if (!j) break;
  }
  if (i==A_LAST) 
    printf("Bug in get_a_tech\n");
  set_invention(pplayer, i, TECH_KNOWN);
  pplayer->research.researchpoints++;
  notify_player(pplayer, "Game: You acquired %s from %s",
		advances[i].name, target->name); 
  notify_player(target, "Game: %s discovered %s in the city.", pplayer->name, 
		advances[i].name); 
  if (pplayer->research.researching==i) {
    tec=pplayer->research.researched;
    choose_random_tech(pplayer);
    pplayer->research.researched=tec;
  }
}
  
/**************************************************************************
...
**************************************************************************/

void handle_unit_help_build_wonder(struct player *pplayer, 
				   struct packet_unit_request *req)
{
  struct unit *punit;
  
  if((punit=unit_list_find(&pplayer->units, req->unit_id))) {
    struct city *pcity_dest;
    
    pcity_dest=find_city_by_id(req->city_id);
    
    if(punit->type==U_CARAVAN && pcity_dest &&
       unit_can_help_build_wonder(punit, pcity_dest)) {

      send_remove_unit(0, punit->id);
      game_remove_unit(punit->id);
      
      notify_player(pplayer, "Game: Your caravan helps building the %s.",
		  get_improvement_type(pcity_dest->currently_building)->name);

      pcity_dest->shield_stock+=50;
      send_player_info(pplayer, pplayer);
      send_city_info(pplayer, pcity_dest, 0);
    }
  }
}

/**************************************************************************
...
**************************************************************************/

void handle_unit_establish_trade(struct player *pplayer, 
				 struct packet_unit_request *req)
{
  struct unit *punit;
  
  if((punit=unit_list_find(&pplayer->units, req->unit_id))) {
    
    struct city *pcity_homecity, *pcity_dest;
    
    pcity_homecity=city_list_find_id(&pplayer->cities, punit->homecity);
    pcity_dest=find_city_by_id(req->city_id);
    
    if(punit->type==U_CARAVAN && pcity_homecity && pcity_dest && 
       is_tiles_adjacent(punit->x, punit->y, pcity_dest->x, pcity_dest->y) &&
       can_establish_trade_route(pcity_homecity, pcity_dest)) {
      int revenue;

      revenue=establish_trade_route(pcity_homecity, pcity_dest);
      send_remove_unit(0, punit->id);
      game_remove_unit(punit->id);
      notify_player(pplayer, "Game: Your caravan has arrived in %s, and revenues account to %d in gold.", 
		    pcity_dest->name, revenue);
      pplayer->economic.gold+=revenue;
      send_player_info(pplayer, pplayer);
      send_city_info(pplayer, pcity_homecity, 0);
      send_city_info(pplayer, pcity_dest, 0);
    }
  }
}

/**************************************************************************
...
**************************************************************************/
void handle_unit_enter_city(struct player *pplayer, struct city *pcity)
{
  int i;
  int coins;
  struct city *pc2;
  struct player *cplayer;
  if(pplayer->player_no!=pcity->owner) {
    struct city *pnewcity;
    cplayer=&game.players[pcity->owner];
    pcity->size--;
    if (pcity->size<1) {
      notify_player(pplayer, "Game: You destroy %s completely.", pcity->name);
      notify_player(cplayer, 
		    "Game: %s has been destroyed by %s", 
		    pcity->name, pplayer->name);
      remove_city(pcity);
      return;
    }
    city_auto_remove_worker(pcity);
    get_a_tech(pplayer, cplayer);
    coins=cplayer->economic.gold;
    coins=(lrand48()%((coins/20)+1))+(coins*(pcity->size))/200;
    pplayer->economic.gold+=coins;
    cplayer->economic.gold-=coins;
    send_player_info(cplayer, cplayer);
    notify_player(pplayer, 
		  "Game: You conquer %s, your lootings accumulate to %d gold", 
		  pcity->name, coins);
    notify_player(cplayer, 
		  "Game: %s conquered %s and looted %d gold from the city.",
		  pplayer->name, pcity->name, coins);
    pnewcity=(struct city *)malloc(sizeof(struct city));
    *pnewcity=*pcity;
    remove_city(pcity);
    for (i=0;i<4;i++) {
      pc2=find_city_by_id(pnewcity->trade[i]);
      if (can_establish_trade_route(pnewcity, pc2))    
	establish_trade_route(pnewcity, pc2);
    }
    /* now set things up for the new owner */
    
    pnewcity->id=get_next_id_number();
    pnewcity->owner=pplayer->player_no;

    unit_list_init(&pnewcity->units_supported);
    city_list_insert(&pplayer->cities, pnewcity);
    
    map_set_city(pnewcity->x, pnewcity->y, pnewcity);
    raze_city(pnewcity);
    city_refresh(pnewcity);
    send_city_info(0, pnewcity, 0);
    send_player_info(pplayer, pplayer);
  }
}

  
/**************************************************************************
...
**************************************************************************/
void handle_unit_activity_request(struct player *pplayer, struct unit *punit, 
				  enum unit_activity new_activity)
{
  if((punit->moves_left>0 || punit->activity==ACTIVITY_GOTO) && 
     can_unit_do_activity(punit, new_activity)) {
    punit->activity=new_activity;
    punit->activity_count=0;

     send_unit_info(0, punit, 0);
  }
}


/**************************************************************************
this is a highlevel routine
**************************************************************************/
void wipe_unit(struct player *dest, struct unit *punit)
{
  if(get_transporter_capacity(punit) && 
     map_get_terrain(punit->x, punit->y)==T_OCEAN) {
    struct genlist_iterator myiter;
    struct unit_list list;
    
    transporter_cargo_to_unitlist(punit, &list);
    genlist_iterator_init(&myiter, &list.list, 0);
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
      struct unit *punit2=(struct unit *)ITERATOR_PTR(myiter);
	send_remove_unit(0, punit2->id);
	game_remove_unit(punit2->id);
    }
  }
  send_remove_unit(0, punit->id);
  game_remove_unit(punit->id);
}


/**************************************************************************
this is a highlevel routine
the unit has been killed in combat => all other units on the
tile dies unless ...
**************************************************************************/
void kill_unit(struct player *dest, struct unit *punit)
{
  int klaf;
  klaf=unit_list_size(&(map_get_tile(punit->x, punit->y)->units));
  if(!(map_get_city(punit->x, punit->y)) && 
     !(map_get_special(punit->x, punit->y)&S_FORTRESS)) {
    struct genlist_iterator myiter;
    if (klaf>1) {
      notify_player(&game.players[punit->owner], 
		    "Game: You lost %d units under an attack from %s",
		    klaf, dest->name);
    }
    genlist_iterator_init(&myiter, 
			  &map_get_tile(punit->x, punit->y)->units.list, 0);
    
    for(; ITERATOR_PTR(myiter); ) {
      struct unit *punit2=(struct unit *)ITERATOR_PTR(myiter);
      ITERATOR_NEXT(myiter);
      notify_player(&game.players[punit2->owner],
		    "Game: You lost a%s %s under an attack from %s",
		    n_if_vowel(get_unit_type(punit2->type)->name[0]),
		    get_unit_type(punit2->type)->name, dest->name);
      send_remove_unit(0, punit2->id);
      game_remove_unit(punit2->id);
    }
  } else {
    notify_player(&game.players[punit->owner], "Game: You lost %d units under an attack from %s",klaf, dest->name);
    send_remove_unit(0, punit->id);
    game_remove_unit(punit->id);
  }
}

/**************************************************************************
...
**************************************************************************/
void send_unit_info(struct player *dest, struct unit *punit, int dosend)
{
  int o;
  struct packet_unit_info info;

  info.id=punit->id;
  info.owner=punit->owner;
  info.x=punit->x;
  info.y=punit->y;
  info.homecity=punit->homecity;
  info.veteran=punit->veteran;
  info.type=punit->type;
  info.attacksleft=punit->attacks_left;
  info.movesleft=punit->moves_left;
  info.hp=punit->hp;
  info.activity=punit->activity;
  info.activity_count=punit->activity_count;
  info.unhappiness=punit->unhappiness;
  info.upkeep=punit->upkeep;
  info.bribe_cost=punit->bribe_cost;
  
  for(o=0; o<game.nplayers; o++)           /* dests */
    if(!dest || &game.players[o]==dest)
      if(dosend || map_get_known(info.x, info.y, &game.players[o]))
	 send_packet_unit_info(game.players[o].conn, &info);
}
