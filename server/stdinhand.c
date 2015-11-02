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
#include <ctype.h>
#include <stdlib.h>
#include "game.h"
#include "player.h"
#include "civserver.h"
#include "log.h"
#include "sernet.h"
#include "map.h"
#include "mapgen.h"
#include "registry.h"

void cut_player_connection(char *playername);
void quit_game(void);
void show_help(void);
void show_players(void);

struct proto_settings {
  char *name;
  char *help;
  int *value;
  int min_value, max_value, default_value;
};
struct proto_settings settings[] = {
  { "xsize", "Width of map in squares", 
    &map.xsize,  
    MAP_MIN_WIDTH, MAP_MAX_HEIGHT, MAP_DEFAULT_WIDTH},

  { "ysize", "Height of map in squares", 
    &map.ysize, 
    MAP_MIN_HEIGHT, MAP_MAX_HEIGHT, MAP_DEFAULT_HEIGHT},

  { "seed", "This single number defines the random sequence that generate the map\nSame seed will always produce same map.", 
    &map.seed, 
    MAP_MIN_SEED,MAP_MAX_SEED, MAP_DEFAULT_SEED},
  
  { "landmass", "This number defines the percentage of the map that becomes land.", 
    &map.landpercent, 
    MAP_MIN_LANDMASS, MAP_MAX_LANDMASS, MAP_DEFAULT_LANDMASS},

  { "specials", "This number donates a percentage chance that a square is special.",
    &map.riches,
    MAP_MIN_RICHES, MAP_MAX_RICHES, MAP_DEFAULT_RICHES},

  { "swamps", "How many swamps to create on the map.",
    &map.swampsize,
    MAP_MIN_SWAMPS, MAP_MAX_SWAMPS, MAP_DEFAULT_SWAMPS},

  { "settlers", "How many settlers each player starts with.",
    &game.settlers,
    GAME_MIN_SETTLERS, GAME_MAX_SETTLERS, GAME_DEFAULT_SETTLERS},

  { "deserts", "How many deserts to create on the map.",
    &map.deserts,
    MAP_MIN_DESERTS, MAP_MAX_DESERTS, MAP_DEFAULT_DESERTS},

  { "rivers", "Denotes the total length of the rivers on the map.",
    &map.riverlength,
    MAP_MIN_RIVERS, MAP_MAX_RIVERS, MAP_DEFAULT_RIVERS},

  { "mountains", "How flat/high is the map, higher values give more mountains.", 
    &map.mountains,
    MAP_MIN_MOUNTAINS, MAP_MAX_MOUNTAINS, MAP_DEFAULT_MOUNTAINS},

  { "forests", "How much forest to create, higher values give more forest.", 
    &map.forestsize,
    MAP_MIN_FORESTS, MAP_MAX_FORESTS, MAP_DEFAULT_FORESTS},

  { "huts", "how many 'bonus huts' should be created.",
    &map.huts,
    MAP_MIN_HUTS, MAP_MAX_HUTS, MAP_DEFAULT_HUTS},

  { "gold", "how much gold does each players start with.",
    &game.gold,
    GAME_MIN_GOLD, GAME_MAX_GOLD, GAME_DEFAULT_GOLD},

  { "techlevel", "How many initial advances does each player have.",
    &game.tech,
    GAME_MIN_TECHLEVEL, GAME_MAX_TECHLEVEL, GAME_DEFAULT_TECHLEVEL},

  { "endyear", "In what year is the game over.", 
    &game.end_year,
    GAME_MIN_END_YEAR, GAME_MAX_END_YEAR, GAME_DEFAULT_END_YEAR},

  { "minplayers", "How many players are needed to start the game.",
    &game.min_players,
    GAME_MIN_MIN_PLAYERS, GAME_MAX_MIN_PLAYERS, GAME_DEFAULT_MIN_PLAYERS},
  
  { "maxplayers", "How many players are maximally wanted in game.",
    &game.max_players,
    GAME_MIN_MAX_PLAYERS, GAME_MAX_MAX_PLAYERS, GAME_DEFAULT_MAX_PLAYERS},

  { "researchspeed", "How fast do players gain technology.",
    &game.techlevel,
    GAME_MIN_RESEARCHLEVEL, GAME_MAX_RESEARCHLEVEL, GAME_DEFAULT_RESEARCHLEVEL},

  { "unhappysize", "When do people get angry in a city.",
    &game.unhappysize,
    GAME_MIN_UNHAPPYSIZE, GAME_MAX_UNHAPPYSIZE, GAME_DEFAULT_UNHAPPYSIZE},
  

  { "saveturns", "How many turns between when the game is saved.",
    &game.save_nturns,
    0, 200, 1},

    { "timeout", "How many seconds can a turn max take (0 is no timeout).",
    &game.timeout,
    0, 999, GAME_DEFAULT_TIMEOUT},
    
    { NULL, NULL, NULL, 0,0,0}

};


/**************************************************************************
...
**************************************************************************/
void save_command(char *arg)
{
  struct section_file file;
  
  section_file_init(&file);
  
  game_save(&file);

  if(!section_file_save(&file, arg))
    printf("Failed saving game as %s\n", arg);
  else
    printf("Game saved as %s\n", arg);
}


int lookup_cmd(char *find)
{
  int i;
  for (i=0;settings[i].name!=NULL;i++) 
    if (!strcmp(find, settings[i].name)) return i;
  
  return -1;
}
void help_command(char *str)
{

  char command[512], *cptr_s, *cptr_d;
  int cmd,i;

  for(cptr_s=str; *cptr_s && !isalnum(*cptr_s); cptr_s++);
  for(cptr_d=command; *cptr_s && isalnum(*cptr_s); cptr_s++, cptr_d++)
    *cptr_d=*cptr_s;
  *cptr_d='\0';

  if (*command) {
    cmd=lookup_cmd(command);
    if (cmd==-1) {
      puts("No help on that (yet).");
      return;
    }
    printf("%s: is set to %d\nAffect: %s\nMinimum %d, Maximum %d, Default %d\n"
	   , settings[cmd].name, *settings[cmd].value, settings[cmd].help, 
	   settings[cmd].min_value, settings[cmd].max_value, 
	   settings[cmd].default_value);
  } else {
  puts("------------------------------------------------------------------------------");
      puts("Help are defined on the following variables:");
  puts("------------------------------------------------------------------------------");
    for (i=0;settings[i].name;i++) {
      printf("%-19s%c",settings[i].name, ((i+1)%4) ? ' ' : '\n'); 
    }
    if ((i)%4!=0) puts("");
  puts("------------------------------------------------------------------------------");
  }
}
  
void show_command(char *str)
{
  int i;
  puts("------------------------------------------------------------------------------");
  printf("%-20svalue  (min , max)\n", "Variable");
  puts("------------------------------------------------------------------------------");
  for (i=0;settings[i].name;i++) {
    printf("%-20s%c%-6d (%d,%d)\n", settings[i].name,(*settings[i].value==settings[i].default_value) ? '*' : ' ',  *settings[i].value, settings[i].min_value, settings[i].max_value);
  }
  puts("------------------------------------------------------------------------------");
  puts("* means that it's the default for the variable");
  puts("------------------------------------------------------------------------------");
}

void set_command(char *str) 
{
  char command[512], arg[512], *cptr_s, *cptr_d;
  int val, cmd;
    
  for(cptr_s=str; *cptr_s && !isalnum(*cptr_s); cptr_s++);
  
  for(cptr_d=command; *cptr_s && isalnum(*cptr_s); cptr_s++, cptr_d++)
    *cptr_d=*cptr_s;
  *cptr_d='\0';

  for(; *cptr_s && !isalnum(*cptr_s); cptr_s++);

  for(cptr_d=arg; *cptr_s && isalnum(*cptr_s); cptr_s++ , cptr_d++)
    *cptr_d=*cptr_s;
  *cptr_d='\0';

  cmd=lookup_cmd(command);
  if (cmd==-1) {
    puts("Undefined argument. Usage: set <variable> <value>.");
    return;
  }
  val=atoi(arg);
    
  if (val>=settings[cmd].min_value && val<=settings[cmd].max_value) {
    *(settings[cmd].value)=val;
  }
  else
    puts("Value out of range. Usage: set <variable> <value>.");
}

/**************************************************************************
...
**************************************************************************/
void handle_stdin_input(char *str)
{
  char command[512], arg[512], *cptr_s, *cptr_d;
  char arg2[512];
  for(cptr_s=str; *cptr_s && !isalnum(*cptr_s); cptr_s++);

  for(cptr_d=command; *cptr_s && isalnum(*cptr_s); cptr_s++, cptr_d++)
    *cptr_d=*cptr_s;
  *cptr_d='\0';

  for(; *cptr_s && !isalnum(*cptr_s); cptr_s++);
  strcpy(arg2, cptr_s);
  for(cptr_d=arg; *cptr_s && isalnum(*cptr_s); cptr_s++, cptr_d++)
    *cptr_d=*cptr_s;
  *cptr_d='\0';

  if(!strcmp("save", command))
    save_command(arg);
  else if(!strcmp("h", command))
    show_help();
  else if(!strcmp("l", command))
    show_players();
  else if(!strcmp("q", command))
    quit_game();
  else if(!strcmp("c", command))
    cut_player_connection(arg);
  else if (!strcmp("show",command)) 
    show_command(arg);
  else if (!strcmp("help",command)) 
    help_command(arg);
  else if (!strcmp("set", command)) 
    set_command(arg2);
  else if(server_state==PRE_GAME_STATE) {
    if(!strcmp("s", command))
      start_game();
    else
      printf("Unknown Command, try 'h' for help.\n");
  }
  else
    printf("Unknown Command, try 'h' for help.\n");  
  
  printf(">");
  fflush(stdout);
}

/**************************************************************************
...
**************************************************************************/
void cut_player_connection(char *playername)
{
  int i;
  for (i=0;playername[i] && isalnum(playername[i]);i++);
  playername[i]=0;
  for(i=0; i<game.nplayers; i++)
    if(game.players[i].conn && !strcmp(game.players[i].name, playername)) {
      log(LOG_NORMAL, "cutting connection to %s", playername);
      close_connection(game.players[i].conn);
      game.players[i].conn=NULL;
      return;
    }
  puts("uh no such player connected");
}

/**************************************************************************
...
**************************************************************************/
void quit_game(void)
{
  int i;
  struct packet_generic_message gen_packet;
  gen_packet.message[0]='\0';
  
  for(i=0; i<game.nplayers; i++)
    send_packet_generic_message(game.players[i].conn, PACKET_SERVER_SHUTDOWN,
				&gen_packet);
  close_connections_and_socket();
  
  exit(1);
}

/**************************************************************************
...
**************************************************************************/
void show_help(void)
{
  puts("Available commands");
  puts("-------------------------------------");
  puts("c P  - cutconn player P");
  puts("h    - this help");
  puts("l    - list players");
  puts("q    - quit game and shutdown server");
  puts("show - list server options");
  puts("help - help on server options");
  if(server_state==PRE_GAME_STATE) {
    puts("set  - set options");
    puts("s    - start game");
  }
  else {
    
  }
}

/**************************************************************************
...
**************************************************************************/
void show_players(void)
{
  int i;

  puts("List of players:");
  puts("-------------------------------------");

  for(i=0; i<game.nplayers; i++) {
    printf("%s ", game.players[i].name);
    if(game.players[i].conn)
      printf("is connected from %s\n", game.players[i].addr); 
    else
      printf("is not connected\n");
  }
}
