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
#ifndef __MAPGEN_H
#define __MAPGEN_H

void map_fractal_generate(void);
int is_coastline(int x,int y);

#define MAP_DEFAULT_MOUNTAINS    40
#define MAP_MIN_MOUNTAINS        10
#define MAP_MAX_MOUNTAINS        100

#define MAP_DEFAULT_HUTS         30
#define MAP_MIN_HUTS             0
#define MAP_MAX_HUTS             500

#define MAP_DEFAULT_WIDTH        80
#define MAP_MIN_WIDTH            40
#define MAP_MAX_WIDTH            200

#define MAP_DEFAULT_HEIGHT       50
#define MAP_MIN_HEIGHT           25
#define MAP_MAX_HEIGHT           100

#define MAP_DEFAULT_SEED         0
#define MAP_MIN_SEED             0
#define MAP_MAX_SEED             50000

#define MAP_DEFAULT_LANDMASS     30
#define MAP_MIN_LANDMASS         20
#define MAP_MAX_LANDMASS         80

#define MAP_DEFAULT_RICHES       250
#define MAP_MIN_RICHES           0
#define MAP_MAX_RICHES           1000

#define MAP_DEFAULT_SWAMPS       10
#define MAP_MIN_SWAMPS           0
#define MAP_MAX_SWAMPS           100

#define MAP_DEFAULT_DESERTS      5
#define MAP_MIN_DESERTS          0
#define MAP_MAX_DESERTS          100

#define MAP_DEFAULT_RIVERS       50
#define MAP_MIN_RIVERS           0
#define MAP_MAX_RIVERS           1000

#define MAP_DEFAULT_FORESTS      25
#define MAP_MIN_FORESTS          0
#define MAP_MAX_FORESTS          100


#define GAME_DEFAULT_GOLD        50
#define GAME_MIN_GOLD            0
#define GAME_MAX_GOLD            5000

#define GAME_DEFAULT_SETTLERS    2
#define GAME_MIN_SETTLERS        1
#define GAME_MAX_SETTLERS        4

#define GAME_DEFAULT_TECHLEVEL   3
#define GAME_MIN_TECHLEVEL       0
#define GAME_MAX_TECHLEVEL       50

#define GAME_DEFAULT_UNHAPPYSIZE 4
#define GAME_MIN_UNHAPPYSIZE 1
#define GAME_MAX_UNHAPPYSIZE 6

#define GAME_DEFAULT_END_YEAR    2000
#define GAME_MIN_END_YEAR        -2000
#define GAME_MAX_END_YEAR        5000

#define GAME_DEFAULT_MIN_PLAYERS 1
#define GAME_MIN_MIN_PLAYERS     1
#define GAME_MAX_MIN_PLAYERS     MAX_PLAYERS

#define GAME_DEFAULT_MAX_PLAYERS 14
#define GAME_MIN_MAX_PLAYERS     1
#define GAME_MAX_MAX_PLAYERS     MAX_PLAYERS

#define GAME_DEFAULT_RESEARCHLEVEL   10
#define GAME_MIN_RESEARCHLEVEL       6
#define GAME_MAX_RESEARCHLEVEL       14

#endif
