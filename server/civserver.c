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
#include <ctype.h>
#include "game.h"
#include "civserver.h"
#include "map.h"
#include "shared.h"
#include "city.h"
#include "tech.h"
#include "sernet.h"
#include "log.h"
#include "packets.h"
#include "unithand.h"
#include "cityhand.h"
#include "handchat.h"
#include "maphand.h"
#include "player.h"
#include "plrhand.h"
#include "mapgen.h"
#include "diplhand.h"

void show_ending();
void end_game();
int end_turn();
void start_new_game(void);
void init_new_game(void);

void send_game_info_small(struct player *dest);
void send_game_state(struct player *dest, int state);
int find_highest_used_id(void);
void send_all_info(struct player *dest);

extern struct connection connections[];

enum server_states server_state;

/* this global is checked deep down the netcode. 
   packets handling functions can set it to none-zero, to
   force end-of-tick asap
*/
int force_end_of_sniff;


/* this counter creates all the id numbers used */
/* use get_next_id_number()                     */
int global_id_counter=100;


char usage[] = 
"Usage: %s [-fhlpv] [--file] [--help] [--log] [--port]\n\t[--version]\n";

int port=DEFAULT_SOCK_PORT;

/**************************************************************************
...
**************************************************************************/
int main(int argc, char *argv[])
{
  int h=0, v=0;
  char *log_filename=NULL;
  char *load_filename=NULL;
  int i, is_new_game=1;
  int save_counter;


  /* no  we don't use GNU's getopt or even the "standard" getopt */
  /* yes we do have reasons ;)                                   */
  i=1;
  while(i<argc) {
    if(!strcmp("-f", argv[i]) || !strcmp("--file", argv[i])) { 
      if(++i<argc) 
	load_filename=argv[i];
      else {
	fprintf(stderr, "Error: filename not specified.\n");
	h=1;
	break;
      }
    }
    else if(!strcmp("-h", argv[i]) || !strcmp("--help", argv[i])) { 
      h=1;
      break;
    }
    else if(!strcmp("-l", argv[i]) || !strcmp("--log", argv[i])) { 
      if(++i<argc) 
	log_filename=argv[i];
      else {
	fprintf(stderr, "Error: filename not specified.\n");
	h=1;
	break;
      }
    }
    else if(!strcmp("-p", argv[i]) || !strcmp("--port", argv[i])) { 
      if(++i<argc) 
	port=atoi(argv[i]);
      else {
	fprintf(stderr, "Error: port not specified.\n");
	h=1;
	break;
      }
    }
    else if(!strcmp("-v", argv[i]) || !strcmp("--version", argv[i])) { 
      v=1;
    }
    else {
      fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
      h=1;
      break;
    }
    i++;
  }
    
  if(h) {
    fprintf(stderr, "This is the Freeciv server\n");
    fprintf(stderr, "  -f, --file F\t\t\tLoad saved game F\n");
    fprintf(stderr, "  -h, --help\t\t\tPrint a summary of the options\n");
    fprintf(stderr, "  -l, --log F\t\t\tUse F as logfile\n");
    fprintf(stderr, "  -p, --port N\t\t\tconnect to port N\n");
    fprintf(stderr, "  -v, --version\t\t\tPrint the version number\n");
    exit(0);
  }

  if(v) {
    fprintf(stderr, FREECIV_NAME_VERSION "\n");
    exit(0);
  }

  log_init(log_filename);
  log_set_level(LOG_NORMAL);
  
  printf(FREECIV_NAME_VERSION " server\n> ");
  fflush(stdout);

  game_init();

  /* load a saved game */
  if(load_filename) {
    struct section_file file;
    printf("loading game: %s\n", load_filename);
    if(!section_file_load(&file, load_filename)) { 
      log(LOG_FATAL, "couldn't load savefile: %s", load_filename);
      exit(1);
    }
    
    game_load(&file);
    section_file_free(&file);
    if(game.nplayers)
      is_new_game=0;
    global_id_counter=find_highest_used_id()+1;
  }
  
  /* init network */  
  init_connections(); 
  server_open_socket();

  /* accept new players, wait for serverop to start..*/
  server_state=PRE_GAME_STATE;
  while(server_state==PRE_GAME_STATE)
    sniff_packets();

  /* allow players to select a race(case new game} */
  for(i=0; i<game.nplayers && game.players[i].race!=R_LAST; i++);
    
  /* at least one player is missing a race */
  if(i<game.nplayers) { 
    send_select_race(&game.players[i]);
    for(i=0; i<game.nplayers; i++)
      send_select_race(&game.players[i]);
    while(server_state==SELECT_RACES_STATE)
      sniff_packets();
  }

  if(map_is_empty())
    map_fractal_generate();
  
  /* start the game */
  server_state=RUN_GAME_STATE;

  if(is_new_game) {
    int i;
    for(i=0; i<game.nplayers; i++) {
      init_tech(&game.players[i], game.tech); 
      game.players[i].economic.gold=game.gold;
    }
    game.max_players=game.nplayers;
  }
  
  send_all_info(0);

  if(is_new_game) 
    init_new_game();
    
  send_game_state(0, CLIENT_GAME_RUNNING_STATE);
  
  save_counter=game.save_nturns;
  
  while(server_state==RUN_GAME_STATE) {
/*    log(LOG_DEBUG, "new year is now: %s", textyear(game.year));*/
    force_end_of_sniff=0;
    sniff_packets();
    end_turn();
    game_next_year();
    send_player_info(0, 0);
    send_game_info(0);
    send_year_to_clients(game.year);
    
    if(--save_counter==0) {
      save_counter=game.save_nturns;
      save_game();
    }
    if (game.year>game.end_year) 
      server_state=GAME_OVER_STATE;
  }
  

  show_ending();
  end_game();
  return 0;
}


/**************************************************************************
dest can be NULL meaning all players
**************************************************************************/
void send_all_info(struct player *dest)
{
  send_game_info(dest);
  send_map_info(dest);
  send_player_info(0, dest);
  send_all_known_tiles(dest);
}

/**************************************************************************
...
**************************************************************************/
int find_highest_used_id(void)
{
  int i, max_id;
  struct genlist_iterator myiter;

  max_id=0;
  for(i=0; i<game.nplayers; i++) {
    genlist_iterator_init(&myiter, &game.players[i].cities.list, 0);
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
      max_id=MAX(max_id, ((struct city *)ITERATOR_PTR(myiter))->id);
    genlist_iterator_init(&myiter, &game.players[i].units.list, 0);
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
      max_id=MAX(max_id, ((struct unit *)ITERATOR_PTR(myiter))->id);
  }

  return max_id;
}
/**************************************************************************
...
**************************************************************************/
void do_apollo_program()
{
  int cityid;
  int i;
  struct genlist_iterator myiter;
  struct player *pplayer;
  struct city *pcity;
  if ((cityid=game.global_wonders[B_APOLLO]) && 
      (pcity=find_city_by_id(cityid))) {
    pplayer=city_owner(pcity);
    
    for(i=0; i<game.nplayers; i++) {
      genlist_iterator_init(&myiter, &game.players[i].cities.list, 0);
      for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
	pcity=((struct city *)ITERATOR_PTR(myiter));
	light_square(pplayer, pcity->x, pcity->y, 0);
      }
    }
  }
}

/***************************************************************
...
***************************************************************/
void game_load(struct section_file *file)
{
  int i;
  struct genlist_iterator myiter;

  game.gold=secfile_lookup_int(file, "game.gold");
  game.tech=secfile_lookup_int(file, "game.tech");
  game.skill_level=secfile_lookup_int(file, "game.skill_level");
  game.timeout=secfile_lookup_int(file, "game.timeout");
  game.end_year=secfile_lookup_int(file, "game.end_year");
  game.techlevel=secfile_lookup_int(file, "game.techlevel");
  game.year=secfile_lookup_int(file, "game.year");
  game.min_players=secfile_lookup_int(file, "game.min_players");
  game.max_players=secfile_lookup_int(file, "game.max_players");
  game.nplayers=secfile_lookup_int(file, "game.nplayers");
  game.globalwarming=secfile_lookup_int(file, "game.globalwarming");
  game.warminglevel=secfile_lookup_int(file, "game.warminglevel");
  game.unhappysize=secfile_lookup_int(file, "game.unhappysize");
  game.heating=0;
  map_load(file);

  for(i=0; i<game.nplayers; i++) {
    player_load(&game.players[i], i, file); 
      genlist_iterator_init(&myiter, &game.players[i].cities.list, 0);
      for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
	city_refresh((struct city *)ITERATOR_PTR(myiter));
      }
  }
  game.player_idx=0;
  game.player_ptr=&game.players[0];  
  initialize_globals();
}

/***************************************************************
...
***************************************************************/
void game_save(struct section_file *file)
{
  int i;

  secfile_insert_int(file, game.gold, "game.gold");
  secfile_insert_int(file, game.tech, "game.tech");
  secfile_insert_int(file, game.skill_level, "game.skill_level");
  secfile_insert_int(file, game.timeout, "game.timeout");
  secfile_insert_int(file, game.end_year, "game.end_year");
  secfile_insert_int(file, game.year, "game.year");
  secfile_insert_int(file, game.techlevel, "game.techlevel");
  secfile_insert_int(file, game.min_players, "game.min_players");
  secfile_insert_int(file, game.max_players, "game.max_players");
  secfile_insert_int(file, game.nplayers, "game.nplayers");
  secfile_insert_int(file, game.globalwarming, "game.globalwarming");
  secfile_insert_int(file, game.warminglevel, "game.warminglevel");
  secfile_insert_int(file, game.unhappysize, "game.unhappysize");
  for(i=0; i<game.nplayers; i++)
    player_save(&game.players[i], i, file);

  map_save(file);

}

/**************************************************************************
...
**************************************************************************/

int end_turn()
{
  int i;
  
  for(i=0; i<game.nplayers; i++) {
    update_player_activities(&game.players[i]);
    game.players[i].turn_done=0;
  }
  make_history_report();
  update_polution();
  do_apollo_program();
  return 1;
}


/**************************************************************************
...
**************************************************************************/

void end_game()
{
}

/**************************************************************************
...
**************************************************************************/

void save_game(void)
{
  char filename[512];
  struct section_file file;
  
  sprintf(filename, "%s%d.sav", game.save_name, game.year);
  
  section_file_init(&file);
  
  game_save(&file);
  
  if(!section_file_save(&file, filename))
    printf("Failed saving game as %s\n", filename);
  else
    printf("Game saved as %s\n", filename);

  section_file_free(&file);
}

/**************************************************************************
...
**************************************************************************/

void start_game(void)
{
  if(server_state!=PRE_GAME_STATE) {
    puts("the game is already running.");
    return;
  }

  puts("starting game.");

  server_state=SELECT_RACES_STATE; /* loaded ??? */
  force_end_of_sniff=1;
}


/**************************************************************************
...
**************************************************************************/
void handle_report_request(struct player *pplayer, enum report_type type)
{
  switch(type) {
   case REPORT_WONDERS_OF_THE_WORLD:
    wonders_of_the_world(pplayer);
    break;
   case REPORT_TOP_5_CITIES:
    top_five_cities(pplayer);
    break;
   case REPORT_DEMOGRAPHIC:
    demographics_report(pplayer);
    break;
  }
  
}


/**************************************************************************
...
**************************************************************************/
void handle_packet_input(struct connection *pconn, char *packet, int type)
{
  int i;
  struct player *pplayer;
  

  switch(type) {
  case PACKET_REQUEST_JOIN_GAME:
    handle_request_join_game(pconn, (struct packet_req_join_game *)packet);
    free(packet);
    return;
    break;
  }

  for(i=0; i<game.nplayers; i++)
    if(game.players[i].conn==pconn) {
      pplayer=&game.players[i];
      break;
    }

  if(i==game.nplayers) {
    log(LOG_DEBUG, "got game packet from unaccepted connection");
    free(packet);
    return;
  }

  pplayer->nturns_idle=0;

  if(!pplayer->is_alive && type!=PACKET_CHAT_MSG)
    return;
  
  switch(type) {
    
  case PACKET_TURN_DONE:
    handle_turn_done(i);
    break;

  case PACKET_ALLOC_RACE:
    handle_alloc_race(i, (struct packet_alloc_race *)packet);
    break;

  case PACKET_UNIT_INFO:
    handle_unit_info(pplayer, (struct packet_unit_info *)packet);
    break;
 
  case PACKET_CHAT_MSG:
    handle_chat_msg(pplayer, (struct packet_generic_message *)packet);
    break;

   case PACKET_CITY_SELL:
    handle_city_sell(pplayer, (struct packet_city_request *)packet);
    break;

   case PACKET_CITY_BUY:
    handle_city_buy(pplayer, (struct packet_city_request *)packet);
    break;
   
   case PACKET_CITY_CHANGE:
    handle_city_change(pplayer, (struct packet_city_request *)packet);
    break;

   case PACKET_CITY_MAKE_SPECIALIST:
    handle_city_make_specialist(pplayer, (struct packet_city_request *)packet);
    break;

   case PACKET_CITY_MAKE_WORKER:
    handle_city_make_worker(pplayer, (struct packet_city_request *)packet);
    break;

   case PACKET_CITY_CHANGE_SPECIALIST:
    handle_city_change_specialist(pplayer, (struct packet_city_request *)packet);
    break;

   case PACKET_CITY_RENAME:
    handle_city_rename(pplayer, (struct packet_city_request *)packet);
    break;

   case PACKET_PLAYER_RATES:
    handle_player_rates(pplayer, (struct packet_player_request *)packet);
    break;

   case PACKET_PLAYER_REVOLUTION:
    handle_player_revolution(pplayer);
    break;

   case PACKET_PLAYER_GOVERMENT:
    handle_player_goverment(pplayer, (struct packet_player_request *)packet);
    break;

   case PACKET_PLAYER_RESEARCH:
    handle_player_research(pplayer, (struct packet_player_request *)packet);
    break;

   case PACKET_UNIT_BUILD_CITY:
    handle_unit_build_city(pplayer, (struct packet_unit_request *)packet);
    break;

   case PACKET_UNIT_DISBAND:
    handle_unit_disband(pplayer, (struct packet_unit_request *)packet);
    break;

   case PACKET_UNIT_CHANGE_HOMECITY:
    handle_unit_change_homecity(pplayer, (struct packet_unit_request *)packet);
    break;

   case PACKET_UNIT_ESTABLISH_TRADE:
    handle_unit_establish_trade(pplayer, (struct packet_unit_request *)packet);
    break;

   case PACKET_UNIT_HELP_BUILD_WONDER:
    handle_unit_help_build_wonder(pplayer, (struct packet_unit_request *)packet);
    break;

   case PACKET_UNIT_GOTO_TILE:
    handle_unit_goto_tile(pplayer, (struct packet_unit_request *)packet);
    break;
    
   case PACKET_DIPLOMAT_ACTION:
    handle_diplomat_action(pplayer, (struct packet_diplomat_action *)packet);
    break;

   case PACKET_REPORT_REQUEST:
    handle_report_request(pplayer, 
     ((struct packet_generic_integer *)packet)->value);
    break;
   case PACKET_DIPLOMACY_INIT_MEETING:
    handle_diplomacy_init(pplayer, (struct packet_diplomacy_info *)packet);
    break;
  case PACKET_DIPLOMACY_CANCEL_MEETING:
    handle_diplomacy_cancel_meeting(pplayer, (struct packet_diplomacy_info *)packet);  
    break;
  case PACKET_DIPLOMACY_CREATE_CLAUSE:
    handle_diplomacy_create_clause(pplayer, (struct packet_diplomacy_info *)packet);  
    break;
  case PACKET_DIPLOMACY_REMOVE_CLAUSE:
    handle_diplomacy_remove_clause(pplayer, (struct packet_diplomacy_info *)packet);  
    break;
  case PACKET_DIPLOMACY_ACCEPT_TREATY:
    handle_diplomacy_accept_treaty(pplayer, (struct packet_diplomacy_info *)packet);  
    break;
   default:
    log(LOG_NORMAL, "uh got an unknown packet from %s", game.players[i].name);
  }

  free(packet);
}

/**************************************************************************
...
**************************************************************************/

int get_next_id_number(void)
{
  return ++global_id_counter;
}

/**************************************************************************
...
**************************************************************************/

void handle_turn_done(int player_no)
{
  int i;

  game.players[player_no].turn_done=1;

  for(i=0; i<game.nplayers; i++)
    if(game.players[i].conn && game.players[i].is_alive && 
       !game.players[i].turn_done) {
      send_player_info(&game.players[player_no], 0);
      return;
    }

  force_end_of_sniff=1;
}

/**************************************************************************
...
**************************************************************************/

void handle_alloc_race(int player_no, struct packet_alloc_race *packet)
{
  int i;
  struct packet_generic_integer retpacket;

  log(LOG_NORMAL, "%s is the %s ruler %s", game.players[player_no].name, 
      get_race_name(packet->race_no), packet->name);

  for(i=0; i<game.nplayers; i++)
    if(game.players[i].race==packet->race_no) { /* also check name one day */
      send_select_race(&game.players[player_no]); /* it failed */
      return;
    }

  /* inform player his choice was ok */
  retpacket.value=-1;
  send_packet_generic_integer(game.players[player_no].conn,
			      PACKET_SELECT_RACE, &retpacket);

  game.players[player_no].race=packet->race_no;
  strcpy(game.players[player_no].name, packet->name);

  /* tell the other players, that the race is now unavailable */
  for(i=0; i<game.nplayers; i++)
    if(game.players[i].race==R_LAST)
      send_select_race(&game.players[i]);

  for(i=0; i<game.nplayers; i++)
    if(game.players[i].race==R_LAST)
      return;

  /* everybody got a race => new state */
  server_state=RUN_GAME_STATE;
}


/**************************************************************************
...
**************************************************************************/

void send_select_race(struct player *pplayer)
{
  int i;
  struct packet_generic_integer genint;

  genint.value=0;

  for(i=0; i<game.nplayers; i++)
    if(game.players[i].race!=R_LAST)
      genint.value|=1<<game.players[i].race;

  send_packet_generic_integer(pplayer->conn, PACKET_SELECT_RACE, &genint);
}

/**************************************************************************
...
**************************************************************************/

void accept_new_player(char *name, struct connection *pconn)
{
  struct packet_generic_message gen_packet;

  strcpy(game.players[game.nplayers].name, name);
  game.players[game.nplayers].conn=pconn;
  game.players[game.nplayers].is_connected=1;
  strcpy(game.players[game.nplayers].addr, pconn->addr); 
  sprintf(gen_packet.message, "Welcome %s.", name);
  send_packet_generic_message(pconn, PACKET_REPLY_JOIN_GAME_ACCEPT, 
			      &gen_packet);
  log(LOG_NORMAL, "%s has joined the game.", name);
  
  game.nplayers++;
}

/**************************************************************************
...
**************************************************************************/

void handle_request_join_game(struct connection *pconn, 
			      struct packet_req_join_game *request)
{
  struct packet_generic_message gen_packet;
  struct player *pplayer;

  if((pplayer=find_player_by_name(request->name))!=0) {
    if(!pplayer->conn) {
      sprintf(gen_packet.message, "Welcome back %s.", pplayer->name);
      pplayer->conn=pconn;
      pplayer->is_connected=1;
      strcpy(pplayer->addr, pconn->addr); 
      send_packet_generic_message(pconn, PACKET_REPLY_JOIN_GAME_ACCEPT, 
				  &gen_packet);
      log(LOG_NORMAL, "%s has reconnected.", pplayer->name);
      if(server_state==RUN_GAME_STATE) {
	send_all_info(pplayer);
        send_game_state(pplayer, CLIENT_GAME_RUNNING_STATE);
      }
    }
    else if(server_state==PRE_GAME_STATE) {
      accept_new_player(request->name, pconn);
    }
    else {
      sprintf(gen_packet.message, 
	      "You can't join the game. %s is already connected.", 
	      pplayer->name);
      send_packet_generic_message(pconn, PACKET_REPLY_JOIN_GAME_REJECT,
				  &gen_packet);
      log(LOG_NORMAL, "%s was rejected.", pplayer->name);
      close_connection(pconn);
    }
  }
  else {  /* unknown name */
    if(server_state!=PRE_GAME_STATE) {
      strcpy(gen_packet.message, 
	     "Sorry you can't join. The game is already running.");
      send_packet_generic_message(pconn, PACKET_REPLY_JOIN_GAME_REJECT, 
				  &gen_packet);
      log(LOG_NORMAL, "game running - %s was rejected.", request->name);
      lost_connection_to_player(pconn);
      close_connection(pconn);
    }
    else if(game.nplayers==game.max_players) {
      strcpy(gen_packet.message, "Sorry you can't join. The game is full.");
      send_packet_generic_message(pconn, PACKET_REPLY_JOIN_GAME_REJECT, 
				  &gen_packet);
      log(LOG_NORMAL, "game full - %s was rejected.", request->name);    
      close_connection(pconn);
    }
    else {
      accept_new_player(request->name, pconn);    
    }
  }
}

/**************************************************************************
...
**************************************************************************/

void lost_connection_to_player(struct connection *pconn)
{
  int i;

  for(i=0; i<game.nplayers; i++)
    if(game.players[i].conn==pconn) {
      game.players[i].conn=NULL;
      game.players[i].is_connected=0;
      strcpy(game.players[i].addr, "---.---.---.---");
      log(LOG_NORMAL, "lost connection to %s", game.players[i].name);
      send_player_info(&game.players[i], 0);
      return;
    }

  log(LOG_FATAL, "lost connection to unknown");
}
