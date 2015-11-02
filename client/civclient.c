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

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>

#include "civclient.h"
#include "clinet.h"
#include "xmain.h"
#include "log.h"
#include "packets.h"
#include "map.h"
#include "dialogs.h"
#include "chatline.h"
#include "game.h"
#include "mapgen.h"
#include "plrdlg.h"
#include "mapctrl.h"
#include "mapview.h"
#include "citydlg.h"
#include "diplodlg.h"
#include "repodlgs.h"

char usage[] = 
"Usage: %s [-bhlpsv] [--bgcol] [--cmap] [--help] [--log] [--name]\n\t[--port] [--server] [--version]\n";

char server_name[512]={"localhost"};
char name[512];
int port;

enum client_states client_state;

int use_solid_color_behind_units;
int sound_bell_at_new_turn;

int seconds_to_turndone;

int did_advance_tech_this_turn;

void handle_move_unit(struct packet_move_unit *packet);
void handle_new_year(struct packet_new_year *ppacket);
void handle_city_info(struct packet_city_info *packet);
void handle_remove_unit(struct packet_generic_integer *packet);
void handle_remove_city(struct packet_generic_integer *packet);
void handle_unit_combat(struct packet_unit_combat *packet);
void handle_game_state(struct packet_generic_integer *packet);
void handle_nuke_tile(struct packet_nuke_tile *packet);
void handle_page_msg(struct packet_generic_message *packet);

/**************************************************************/
int main(int argc, char *argv[])
{
  int i;
  int h=0, v=0;
  char *log_filename=NULL;
  char *arg;
  struct passwd *password;

  password=getpwuid(getuid());
  strcpy(name, password->pw_name);
  strcpy(server_name, "localhost");
  port=DEFAULT_SOCK_PORT;

  /* no  we don't use GNU's getopt or even the "standard" getopt */
  /* yes we do have reasons ;)                                   */
  i=1;
  while(i<argc) {
    
    if(!strcmp("-b", argv[i]) || !strcmp("--bgcol", argv[i])) { 
      use_solid_color_behind_units=1;
      i++;
    }
    else if(!strcmp("-h", argv[i]) || !strcmp("--help", argv[i])) { 
      h=1;
      break;
    }
    else if((arg=get_option_text(argv, &i, argc, 'l', "log"))) { 
      if(*arg) 
	log_filename=arg;
      else {
	fprintf(stderr, "Error: filename not specified.\n");
	h=1;
	break;
      }
    }
    else if((arg=get_option_text(argv, &i, argc, 'n', "name"))) { 
      if(*arg) {
	strncpy(name, arg, MAX_LENGTH_NAME);
	name[MAX_LENGTH_NAME-1]='\0';
      }
      else {
	fprintf(stderr, "Error: playername not specified.\n");
	h=1;
	break;
      }
    }
    else if((arg=get_option_text(argv, &i, argc, 'p', "port"))) { 
      if(*arg) 
	port=atoi(arg);
      else {
	fprintf(stderr, "Error: port not specified.\n");
	h=1;
	break;
      }
    }
    else if((arg=get_option_text(argv, &i, argc, 's', "server"))) { 
      if(*arg) 
	strcpy(server_name, arg);
      else {
	fprintf(stderr, "Error: server not specified.\n");
	h=1;
	break;
      }
    }
    else if(!strcmp("-v", argv[i]) || !strcmp("--version", argv[i])) { 
      v=1;
      break;
    }
    else {
      fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
      h=1;
      break;
    }
  }

  if(h) {
    fprintf(stderr, "This is the Freeciv client\n");
    fprintf(stderr, usage, argv[0]);
    fprintf(stderr, "  -b, --bgcol\t\t\tUse solid colors behind units\n");
    fprintf(stderr, "  -h, --help\t\t\tPrint a summary of the options\n");
    fprintf(stderr, "  -l, --log F\t\t\tUse F as logfile\n");
    fprintf(stderr, "  -n, --name N\t\t\tUse N as name\n");
    fprintf(stderr, "  -p, --port N\t\t\tconnect to port N\n");
    fprintf(stderr, "  -s, --server S\t\tConnect to the server at S\n");
    fprintf(stderr, "  -v, --version\t\t\tPrint the version number\n");
    exit(0);
  }

  if(v) {
    fprintf(stderr, "%s\n", FREECIV_NAME_VERSION);
    exit(0);
  }

  log_init(log_filename);
  log_set_level(LOG_NORMAL);

  game_init();

  if(connect_to_server(server_name, port)==-1) {
    log(LOG_FATAL, "failed to connect to %s %d", server_name, port);
    exit(1);
  }
  
  { 
    struct packet_req_join_game request;
    strcpy(request.name, name);
    send_packet_req_join_game(&aconnection, &request);
  }

  if(!client_state) /*kill*/
     client_state=CLIENT_PRE_GAME_STATE;

  x_main(argc, argv);

  return 0;
}




/**************************************************************************
...
**************************************************************************/
void handle_packet_input(char *packet, int type)
{
  switch(type) {
  case PACKET_REPLY_JOIN_GAME_ACCEPT:
    log(LOG_DEBUG, "join game accept:%s", ((struct packet_generic_message *)
	packet)->message);
    break;

  case PACKET_REPLY_JOIN_GAME_REJECT:
    append_output_window("You were rejected from the game:");
    append_output_window(((struct packet_generic_message *)packet)->message);
    break;

  case PACKET_SERVER_SHUTDOWN:
    log(LOG_DEBUG, "server shutdown");
    break;

  case PACKET_NEW_YEAR:
    handle_new_year((struct packet_new_year *)packet);
    break;

  case PACKET_UNIT_INFO:
    handle_unit_info((struct packet_unit_info *)packet);
    break;

   case PACKET_MOVE_UNIT:
    handle_move_unit((struct packet_move_unit *)packet);
    break;
    
  case PACKET_TILE_INFO:
    handle_tile_info((struct packet_tile_info *)packet);
    break;

  case PACKET_SELECT_RACE:
    handle_select_race((struct packet_generic_integer *)packet);
    break;

  case PACKET_PLAYER_INFO:
    handle_player_info((struct packet_player_info *)packet);
    break;
    
  case PACKET_GAME_INFO:
    handle_game_info((struct packet_game_info *)packet);
    break;

  case PACKET_MAP_INFO:
    handle_map_info((struct packet_map_info *)packet);
    break;
    
  case PACKET_CHAT_MSG:
    handle_chat_msg((struct packet_generic_message *)packet);
  break;

  case PACKET_PAGE_MSG:
    handle_page_msg((struct packet_generic_message *)packet);
  break;
    
  case PACKET_CITY_INFO:
    handle_city_info((struct packet_city_info *)packet);
  break;

  case PACKET_REMOVE_UNIT:
    handle_remove_unit((struct packet_generic_integer *)packet);
  break;

  case PACKET_REMOVE_CITY:
    handle_remove_city((struct packet_generic_integer *)packet);
  break;
    
  case PACKET_UNIT_COMBAT:
    handle_unit_combat((struct packet_unit_combat *)packet);
  break;

  case PACKET_GAME_STATE:
    handle_game_state(((struct packet_generic_integer *)packet));
  break;

  case PACKET_NUKE_TILE:
    handle_nuke_tile(((struct packet_nuke_tile *)packet));
  break;

  case PACKET_DIPLOMACY_INIT_MEETING:
    handle_diplomacy_init_meeting((struct packet_diplomacy_info *)packet);  
    break;

  case PACKET_DIPLOMACY_CANCEL_MEETING:
    handle_diplomacy_cancel_meeting((struct packet_diplomacy_info *)packet);  
    break;

  case PACKET_DIPLOMACY_CREATE_CLAUSE:
    handle_diplomacy_create_clause((struct packet_diplomacy_info *)packet);  
    break;

  case PACKET_DIPLOMACY_REMOVE_CLAUSE:
    handle_diplomacy_remove_clause((struct packet_diplomacy_info *)packet);  
    break;

  case PACKET_DIPLOMACY_ACCEPT_TREATY:
    handle_diplomacy_accept_treaty((struct packet_diplomacy_info *)packet);  
    break;
    
  default:
    log(LOG_FATAL, "Received unknown packet from server!");
    exit(1);
    break;
  }

  free(packet);
}


/**************************************************************************
...
**************************************************************************/
void handle_nuke_tile(struct packet_nuke_tile *packet)
{
  put_nuke_mushroom_pixmaps(packet->x, packet->y);
}




/**************************************************************************
this piece of code below is about as bad as it can get
**************************************************************************/
void handle_unit_combat(struct packet_unit_combat *packet)
{
  struct unit *punit0, *punit1;
  
  if((punit0=game_find_unit_by_id(packet->attacker_unit_id)));
    /*punit0->hp=packet->attacker_hp;*/

  if((punit1=game_find_unit_by_id(packet->defender_unit_id)));
    /*punit1->hp=packet->defender_hp;*/

  if(punit0 && tile_visible_mapcanvas(punit0->x, punit0->y) &&
     punit1 && tile_visible_mapcanvas(punit1->x, punit1->y)) {
    struct unit *pfocus;
    pfocus=get_unit_in_focus();
  
    decrease_unit_hp_smooth(punit0, packet->attacker_hp,
			    punit1, packet->defender_hp);

    set_unit_focus(pfocus);
  }

}




/**************************************************************************
...
**************************************************************************/
void handle_game_state(struct packet_generic_integer *packet)
{
  if(client_state==CLIENT_SELECT_RACE_STATE && 
     packet->value==CLIENT_GAME_RUNNING_STATE) {
    popdown_races_dialog();
  }
  client_state=packet->value;

  if(client_state==CLIENT_GAME_RUNNING_STATE) {
    refresh_overview_canvas();
    refresh_overview_viewrect();
    player_set_unit_focus_status(game.player_ptr);

    update_unit_focus();
    update_unit_info_label(get_unit_in_focus());

    if(get_unit_in_focus())
      center_tile_mapcanvas(get_unit_in_focus()->x, get_unit_in_focus()->y);
    
  }
}




/**************************************************************************
...
**************************************************************************/
void handle_remove_city(struct packet_generic_integer *packet)
{
  struct city *pcity;
  
  if((pcity=game_find_city_by_id(packet->value))) {
    int x, y;
    
    x=pcity->x;
    y=pcity->y;

    game_remove_city(packet->value);
    refresh_tile_mapcanvas(x, y, 1);
  
    popdown_city_dialog(pcity);
  }
}


/**************************************************************************
...
**************************************************************************/
void handle_remove_unit(struct packet_generic_integer *packet)
{
  struct unit *punit;
  struct city *pcity;
    
  if((punit=game_find_unit_by_id(packet->value))) {
    int x, y, hc;
    
    x=punit->x;
    y=punit->y;
    hc=punit->homecity;

    if(punit==get_unit_in_focus()) {
      set_unit_focus_no_center(0);
      game_remove_unit(packet->value);
      advance_unit_focus();
    }
    else
      game_remove_unit(packet->value);

    if((pcity=map_get_city(x, y)))
      refresh_city_dialog(pcity);
    if((pcity=city_list_find_id(&game.player_ptr->cities, hc)))
      refresh_city_dialog(pcity);

    refresh_tile_mapcanvas(x, y, 1);
  }
}



/**************************************************************************
...
**************************************************************************/
void handle_city_info(struct packet_city_info *packet)
{
  int i, x, y, city_is_new;
  struct city *pcity;

  pcity=city_list_find_id(&game.players[packet->owner].cities, packet->id);

  if(!pcity) {
    city_is_new=1;
    pcity=(struct city *)malloc(sizeof(struct city));
  }
  else
    city_is_new=0;
  
  pcity->id=packet->id;
  pcity->owner=packet->owner;
  pcity->x=packet->x;
  pcity->y=packet->y;
  strcpy(pcity->name, packet->name);
  
  pcity->size=packet->size;
  pcity->ppl_happy[4]=packet->ppl_happy;
  pcity->ppl_content[4]=packet->ppl_content;
  pcity->ppl_unhappy[4]=packet->ppl_unhappy;
  pcity->ppl_elvis=packet->ppl_elvis;
  pcity->ppl_scientist=packet->ppl_scientist;
  pcity->ppl_taxman=packet->ppl_taxman;
  for (i=0;i<4;i++)
    pcity->trade[i]=packet->trade[i];
  
  pcity->food_prod=packet->food_prod;
  pcity->food_surplus=packet->food_surplus;
  pcity->shield_prod=packet->shield_prod;
  pcity->shield_surplus=packet->shield_surplus;
  pcity->trade_prod=packet->trade_prod;
  pcity->corruption=packet->corruption;
  
  pcity->luxury_total=packet->luxury_total;
  pcity->tax_total=packet->tax_total;
  pcity->science_total=packet->science_total;
  
  pcity->food_stock=packet->food_stock;
  pcity->shield_stock=packet->shield_stock;
  pcity->polution=packet->polution;
  pcity->incite_revolt_cost=packet->incite_revolt_cost;
    
  pcity->is_building_unit=packet->is_building_unit;
  pcity->currently_building=packet->currently_building;
  pcity->did_buy=packet->did_buy;
  pcity->did_sell=packet->did_sell;
    
  i=0;
  for(y=0; y<CITY_MAP_SIZE; y++)
    for(x=0; x<CITY_MAP_SIZE; x++)
      pcity->city_map[x][y]=packet->city_map[i++]-'0';
    
  for(i=0; i<B_LAST; i++)
    pcity->improvements[i]=(packet->improvements[i]=='1') ? 1 : 0;

  
  if(city_is_new) {
    unit_list_init(&pcity->units_supported);
    city_list_insert(&game.players[packet->owner].cities, pcity);
    map_set_city(pcity->x, pcity->y, pcity);
    if(pcity->owner==game.player_idx)
      city_report_dialog_update();

    for(i=0; i<game.nplayers; i++) {
      struct genlist_iterator myiter;
      genlist_iterator_init(&myiter, &game.players[i].units.list, 0);
     
      for(;ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
	struct unit *punit=(struct unit *)ITERATOR_PTR(myiter);
	if(punit->homecity==pcity->id)
	  unit_list_insert(&pcity->units_supported, punit);
      }
    }
  }
    
  refresh_tile_mapcanvas(pcity->x, pcity->y, 1);
  
  if(city_is_new && client_state==CLIENT_GAME_RUNNING_STATE && 
     pcity->owner==game.player_idx)
    popup_city_dialog(pcity, 0);

  if(!city_is_new)
    refresh_city_dialog(pcity);
}




/**************************************************************************
...
**************************************************************************/
void handle_new_year(struct packet_new_year *ppacket)
{
  update_turn_done_button(1);
  enable_turn_done_button();
  
  game.year=ppacket->year;
  update_info_label();

  player_set_unit_focus_status(game.player_ptr);
  
  update_unit_focus();
    
  update_unit_info_label(get_unit_in_focus());

  seconds_to_turndone=game.timeout;

  update_report_dialogs();
}


/**************************************************************************
...
**************************************************************************/
void handle_chat_msg(struct packet_generic_message *packet)
{
  append_output_window(packet->message);
}

/**************************************************************************
...
**************************************************************************/
void handle_page_msg(struct packet_generic_message *packet)
{
  int i;
  char title[512];
  
  for(i=0; packet->message[i]!='\n'; i++)
    title[i]=packet->message[i];
  title[i]='\0';
  
  popup_notify_dialog(title, packet->message+i+1);
}



/**************************************************************************
...
**************************************************************************/
void handle_move_unit(struct packet_move_unit *packet)
{
}




/**************************************************************************
...
**************************************************************************/
void handle_unit_info(struct packet_unit_info *packet)
{
  struct city *pcity;
  struct unit *punit;
  int repaint_unit;
  int repaint_city;
  
  repaint_unit=0;
  repaint_city=0;
  punit=unit_list_find(&game.players[packet->owner].units, packet->id);
  
  if(punit) {

    if(punit->activity!=packet->activity) { /* change activity */
      punit->activity=packet->activity;
  
      repaint_unit=1;
      
      /*      refresh_tile_mapcanvas(punit->x, punit->y, 1);
      update_unit_pix_label(punit);
      refresh_unit_city_dialogs(punit);
      update_unit_focus(); */
    }
    
    if(punit->homecity!=packet->homecity) { /* change homecity */
      struct city *pcity;
      if((pcity=game_find_city_by_id(punit->homecity))) {
	unit_list_unlink(&pcity->units_supported, punit);
	refresh_city_dialog(pcity);
      }
      
      punit->homecity=packet->homecity;
      if((pcity=game_find_city_by_id(punit->homecity))) {
	unit_list_insert(&pcity->units_supported, punit);
	refresh_city_dialog(pcity);
      }
    }

    if(punit->hp!=packet->hp) {                      /* hp changed */
      punit->hp=packet->hp;
      repaint_unit=1;
    }

    if(punit->attacks_left!=packet->attacksleft) {   /* #attacks changed */
      punit->attacks_left=packet->attacksleft;
      repaint_unit=1;
    }
    
    if(punit->x!=packet->x || punit->y!=packet->y) { /* change position */
      struct city *pcity;
      pcity=map_get_city(punit->x, punit->y);
      
      if(tile_is_known(packet->x, packet->y)) {
	do_move_unit(punit, packet);
	update_unit_focus();
      }
      else {
	unit_list_unlink(&game.players[packet->owner].units, punit);
	unit_list_unlink(&map_get_tile(punit->x, punit->y)->units, punit);
	refresh_tile_mapcanvas(punit->x, punit->y, 1);
	free(punit);
      }
      if(pcity)
	refresh_city_dialog(pcity);
      
      if((pcity=map_get_city(punit->x, punit->y)))
	refresh_city_dialog(pcity);
      
      repaint_unit=0;
    }
    if (punit->unhappiness!=packet->unhappiness) {
      punit->unhappiness=packet->unhappiness;
      repaint_city=1;
    }
    if (punit->upkeep!=packet->upkeep) {
      punit->upkeep=packet->upkeep;
      repaint_city=1;
    }
    if (repaint_city) {
      if((pcity=game_find_city_by_id(punit->homecity))) {
	refresh_city_dialog(pcity);
      }
    }

    punit->moves_left=packet->movesleft;
    punit->bribe_cost=packet->bribe_cost;
  }
  
  else {      /* create new unit */
    punit=(struct unit *)malloc(sizeof(struct unit));
    
    punit->id=packet->id;
    punit->owner=packet->owner;
    punit->x=packet->x;
    punit->y=packet->y;
    punit->veteran=packet->veteran;
    punit->homecity=packet->homecity;
    punit->type=packet->type;
    punit->attacks_left=packet->attacksleft;
    punit->moves_left=packet->movesleft;
    punit->hp=packet->hp;
    punit->unhappiness=0;
    punit->activity=packet->activity;
    punit->upkeep=0;
    punit->hp=packet->hp;
    punit->bribe_cost=packet->bribe_cost;
    
    unit_list_insert(&game.players[packet->owner].units, punit);
    unit_list_insert(&map_get_tile(punit->x, punit->y)->units, punit);
    
    if((pcity=game_find_city_by_id(punit->homecity)))
      unit_list_insert(&pcity->units_supported, punit);
    
    /* this is ugly - prevent unit from being drawn if it's moved into
     * screen by a transporter - only works for ground_units.. yak */
    if(!is_ground_unit(punit) || map_get_terrain(punit->x, punit->y)!=T_OCEAN)
      repaint_unit=1;
    else
      repaint_unit=0;
  }

  if(punit && punit==get_unit_in_focus())
    update_unit_info_label(punit);

  if(repaint_unit)
    refresh_tile_mapcanvas(punit->x, punit->y, 1);

  update_unit_focus(); 
}


/**************************************************************************
...
**************************************************************************/
void handle_map_info(struct packet_map_info *pinfo)
{
  int x, y;

  map.xsize=pinfo->xsize;
  map.ysize=pinfo->ysize;
  map.is_earth=pinfo->is_earth;

  if(!(map.tiles=(struct tile*)malloc(map.xsize*map.ysize*
				      sizeof(struct tile)))) {
    log(LOG_FATAL, "malloc failed in handle_map_info");
    exit(1);
  }
  for(y=0; y<map.ysize; y++)
    for(x=0; x<map.xsize; x++)
      tile_init(map_get_tile(x, y));

  set_overview_dimensions(map.xsize, map.ysize);
}


/**************************************************************************
...
**************************************************************************/
void handle_game_info(struct packet_game_info *pinfo)
{
  int i;
  
  game.gold=pinfo->gold;
  game.tech=pinfo->tech;
  game.techlevel=pinfo->techlevel;
  game.skill_level=pinfo->skill_level;
  game.timeout=pinfo->timeout;

  game.end_year=pinfo->end_year;
  game.year=pinfo->year;
  game.min_players=pinfo->min_players;
  game.max_players=pinfo->max_players;
  game.nplayers=pinfo->nplayers;
  game.globalwarming=pinfo->globalwarming;
  game.heating=pinfo->heating;
  if(client_state!=CLIENT_GAME_RUNNING_STATE) {
    game.player_idx=pinfo->player_idx;
    game.player_ptr=&game.players[game.player_idx];
  }
  for(i=0; i<A_LAST; i++)
    game.global_advances[i]=pinfo->global_advances[i];
  for(i=0; i<B_LAST; i++)
    game.global_wonders[i]=pinfo->global_wonders[i];
  
  if(client_state!=CLIENT_GAME_RUNNING_STATE) {
    if(client_state==CLIENT_SELECT_RACE_STATE)
      popdown_races_dialog();

  }
  update_unit_focus();

}

/**************************************************************************
...
**************************************************************************/
void handle_player_info(struct packet_player_info *pinfo)
{
  int i;
  
  struct player *pplayer=&game.players[pinfo->playerno];
  
  strcpy(pplayer->name, pinfo->name);
  pplayer->race=pinfo->race;
  
  pplayer->economic.gold=pinfo->gold;
  pplayer->economic.tax=pinfo->tax;
  pplayer->economic.science=pinfo->science;
  pplayer->economic.luxury=pinfo->luxury;
  pplayer->goverment=pinfo->goverment;
  pplayer->embassy=pinfo->embassy;
  
  
  for(i=0; i<A_LAST; i++)
    pplayer->research.inventions[i]=pinfo->inventions[i]-'0';

  if(pplayer->research.researching!=pinfo->researching && 
     client_state==CLIENT_GAME_RUNNING_STATE && pplayer==game.player_ptr) {
    popup_science_dialog(1);
    did_advance_tech_this_turn=game.year;
  }
  
  pplayer->research.researched=pinfo->researched;
  pplayer->research.researchpoints=pinfo->researchpoints;
  pplayer->research.researching=pinfo->researching;
  
  pplayer->turn_done=pinfo->turn_done;
  pplayer->nturns_idle=pinfo->nturns_idle;
  pplayer->is_alive=pinfo->is_alive;
  
  pplayer->is_connected=pinfo->is_connected;
  strcpy(pplayer->addr, pinfo->addr);

  pplayer->revolution=pinfo->revolution;
  
  if(pplayer==game.player_ptr && pplayer->revolution==0 && 
     pplayer->goverment==G_ANARCHY) 
    popup_goverment_dialog();
  
  update_players_dialog();

  if(pplayer==game.player_ptr) {
    update_info_label();
  }
}


/**************************************************************************
...
**************************************************************************/
void handle_tile_info(struct packet_tile_info *packet)
{
  int old_known, old_special;

  struct tile *ptile=map_get_tile(packet->x, packet->y);

  old_known=ptile->known;
  old_special=ptile->special;

  ptile->terrain=packet->type;
  ptile->special=packet->special;

  ptile->known=packet->known;
  
  if(client_state==CLIENT_GAME_RUNNING_STATE && packet->known>=TILE_KNOWN) {
    refresh_tile_mapcanvas(packet->x, packet->y, 1); 
    if(old_known<TILE_KNOWN) { 
      int known;
      
      known=tile_is_known(packet->x-1, packet->y);
      if(known>=TILE_KNOWN && known!=ptile->known)
	refresh_tile_mapcanvas(packet->x-1, packet->y, 1); 

      known=tile_is_known(packet->x+1, packet->y);
      if(known>=TILE_KNOWN && known!=ptile->known)
	refresh_tile_mapcanvas(packet->x+1, packet->y, 1); 

      known=tile_is_known(packet->x, packet->y-1);
      if(known>=TILE_KNOWN && known!=ptile->known)
	refresh_tile_mapcanvas(packet->x, packet->y-1, 1); 

      known=tile_is_known(packet->x, packet->y+1);
      if(known>=TILE_KNOWN && known!=ptile->known)
	refresh_tile_mapcanvas(packet->x, packet->y+1, 1); 
    }
    else { /* happens so seldom(new roads etc) so refresh'em all */
      refresh_tile_mapcanvas(packet->x-1, packet->y, 1); 
      refresh_tile_mapcanvas(packet->x+1, packet->y, 1); 
      refresh_tile_mapcanvas(packet->x, packet->y-1, 1); 
      refresh_tile_mapcanvas(packet->x, packet->y+1, 1); 
    }
  }
}

/**************************************************************************
...
**************************************************************************/
void handle_select_race(struct packet_generic_integer *packet)
{
  if(client_state==CLIENT_SELECT_RACE_STATE) {
    if(packet->value==-1) {
      client_state=CLIENT_WAITING_FOR_GAME_START_STATE;
      popdown_races_dialog();
    }
    else
      races_toggles_set_sensitive(packet->value);
  }
  else if(client_state==CLIENT_PRE_GAME_STATE) {
    client_state=CLIENT_SELECT_RACE_STATE;
    popup_races_dialog();
    races_toggles_set_sensitive(packet->value);
  }
  else
    log(LOG_DEBUG, "got a select race packet in an incompatible state");
}

/**************************************************************************
...
**************************************************************************/

void user_ended_turn(void)
{
  struct packet_generic_message gen_packet;
  gen_packet.message[0]='\0';

  send_packet_generic_message(&aconnection, PACKET_TURN_DONE, &gen_packet);
}

/**************************************************************************
...
**************************************************************************/
void send_unit_info(struct unit *punit)
{
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
  info.activity=punit->activity;

  send_packet_unit_info(&aconnection, &info);
}

/**************************************************************************
...
**************************************************************************/
void send_report_request(enum report_type type)
{
 struct packet_generic_integer pa;
  
  pa.value=type;
  send_packet_generic_integer(&aconnection, PACKET_REPORT_REQUEST, &pa);
}
