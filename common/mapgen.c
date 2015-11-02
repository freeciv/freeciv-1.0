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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "map.h"


void make_huts(int number);
void init_map();
void spread(int spr);
void smooth_map();
void add_height(int x,int y, int radius);
void adjust_map(int minval);
int *height_map;

int landpercent=30;
int swampsize=10;
int maxval=0;
int forests=0;

int full_map(int x, int y)
{
  return height_map[y*map.xsize+x];
}

int is_at_coast(int x, int y)
{
  if (map_get_terrain(x-1,y)==T_OCEAN)   return 1;
  if (map_get_terrain(x,y-1)==T_OCEAN)   return 1;
  if (map_get_terrain(x,y+1)==T_OCEAN)   return 1;
  if (map_get_terrain(x+1,y)==T_OCEAN)   return 1;
  return 0;
  
}

int is_coastline(int x,int y) 
{
  if (map_get_terrain(x-1,y)!=T_OCEAN)   return 1;
  if (map_get_terrain(x-1,y-1)!=T_OCEAN) return 1;
  if (map_get_terrain(x-1,y+1)!=T_OCEAN) return 1;
  if (map_get_terrain(x,y-1)!=T_OCEAN)   return 1;
  if (map_get_terrain(x,y+1)!=T_OCEAN)   return 1;
  if (map_get_terrain(x+1,y-1)!=T_OCEAN) return 1;
  if (map_get_terrain(x+1,y)!=T_OCEAN)   return 1;
  if (map_get_terrain(x+1,y+1)!=T_OCEAN) return 1;
  return 0;
}

int terrain_is_clean(int x, int y)
{
  int x1,y1;
  for (x1=x-3;x1<x+3;x1++)
    for (y1=y-3;y1<y+3;y1++) 
      if (map_get_terrain(x1,y1)!=T_GRASSLAND && map_get_terrain(x1,y1)!=T_PLAINS) return 0;
  return 1;
}

void make_mountains(int thill)
{
  int x,y;
  int mount;
  int j;
  for (j=0;j<10;j++) {
    mount=0;
    for (y=0;y<map.ysize;y++) 
      for (x=0;x<map.xsize;x++) 
	if (full_map(x, y)>thill) 
	    mount++;
    if (mount<((map.xsize*map.ysize)*map.mountains)/1000) 
      thill*=0.95;
    else 
      thill*=1.05;
  }
  
  for (y=0;y<map.ysize;y++) 
    for (x=0;x<map.xsize;x++) 
      if (full_map(x, y)>thill &&map_get_terrain(x,y)!=T_OCEAN) { 
	if (lrand48()%100>75) 
	  map_set_terrain(x, y, T_MOUNTAINS);
	else if (lrand48()%100>25) 
	  map_set_terrain(x, y, T_HILLS);
      }
}

void make_polar()
{
  int y,x;

  for (y=0;y<5;y++) {
    for (x=0;x<map.xsize;x++) {
      if ((full_map(x, y)+(map.ysize*0.1-y*25)>lrand48()%maxval && map_get_terrain(x,y)==T_GRASSLAND) || y==0) { 
	if (y<2)
	  map_set_terrain(x, y, T_ARCTIC);
	else
	  map_set_terrain(x, y, T_TUNDRA);
	  
      } 
    }
  }
  for (y=map.ysize*0.9;y<map.ysize;y++) {
    for (x=0;x<map.xsize;x++) {
      if ((full_map(x, y)+(map.ysize*0.1-(map.ysize-y)*25)>lrand48()%maxval && map_get_terrain(x, y)==T_GRASSLAND) || y==map.ysize-1) {
	if (y>map.ysize-3)
	  map_set_terrain(x, y, T_ARCTIC);
	else
	  map_set_terrain(x, y, T_TUNDRA);
      }
    }
  }
}

void make_desert(int x, int y, int height, int diff) 
{
  if (abs(full_map(x, y)-height)<diff && map_get_terrain(x, y)==T_GRASSLAND) {
    map_set_terrain(x, y, T_DESERT);
    make_desert(x-1,y, height, diff-1);
    make_desert(x,y-1, height, diff-3);
    make_desert(x+1,y, height, diff-1);
    make_desert(x,y+1, height, diff-3);
  }
}

void make_forest(int x, int y, int height, int diff)
{
  if (x==0 || x==map.xsize-1 ||y==0 || y==map.ysize-1)
    return;

  if (map_get_terrain(x, y)==T_GRASSLAND) {
    if (y>map.ysize*0.42 && y<map.ysize*0.58 && lrand48()%100>50)
      map_set_terrain(x, y, T_JUNGLE);
    else 
      map_set_terrain(x, y, T_FOREST);
      if (abs(full_map(x, y)-height)<diff) {
	if (lrand48()%10>5) make_forest(x-1,y, height, diff-5);
	if (lrand48()%10>5) make_forest(x,y-1, height, diff-5);
	if (lrand48()%10>5) make_forest(x+1,y, height, diff-5);
	if (lrand48()%10>5) make_forest(x,y+1, height, diff-5);
      }
    forests++;
  }
}

void make_forests()
{
  int x,y;
  int forestsize=25;
  forestsize=(map.xsize*map.ysize*map.forestsize)/1000;
   do {
    x=lrand48()%map.xsize;
    y=lrand48()%map.ysize;
    if (map_get_terrain(x, y)==T_GRASSLAND) {
      make_forest(x,y, full_map(x, y), 25);
    }
    if (lrand48()%100>75) {
      y=(lrand48()%(int)(map.ysize*0.2))+map.ysize*0.4;
      x=lrand48()%map.xsize;
      if (map_get_terrain(x, y)==T_GRASSLAND) {
	make_forest(x,y, full_map(x, y), 25);
      }
    }
  } while (forests<forestsize);
}

void make_swamps()
{
  int x,y,i;
  int forever=0;
  for (i=0;i<swampsize;) {
    forever++;
    if (forever>1000) return;
    y=lrand48()%map.ysize;
    x=lrand48()%map.xsize;
    if (map_get_terrain(x, y)==T_GRASSLAND && full_map(x, y)<(maxval*60)/100) {
      map_set_terrain(x, y, T_SWAMP);
      if (lrand48()%10>5 && map_get_terrain(x-1, y)!=T_OCEAN) 
	map_set_terrain(x-1,y, T_SWAMP);
      if (lrand48()%10>5 && map_get_terrain(x+1, y)!=T_OCEAN) 
	map_set_terrain(x+1,y, T_SWAMP);
      if (lrand48()%10>5 && map_get_terrain(x, y-1)!=T_OCEAN) 
	map_set_terrain(x,y-1, T_SWAMP);
      if (lrand48()%10>5 && map_get_terrain(x, y+1)!=T_OCEAN) 
	map_set_terrain(x, y+1, T_SWAMP);
      i++;
    }
  }
}

void make_deserts()
{
  int x,y,i,j;
  i=map.deserts;
  j=0;
  while (i && j<1000) {
    j++;
    y=lrand48()%(int)(map.ysize*0.1)+map.ysize*0.45;
    x=lrand48()%map.xsize;
    if (map_get_terrain(x, y)==T_GRASSLAND) {
      make_desert(x,y, full_map(x, y), 50);
      i--;
    }
  }
}

int make_river(int x,int y) 
{
  int mini=10000;
  int mp;
  int res=0;
  mp=-1;
  if (x==0 || x==map.xsize-1 ||y==0 || y==map.ysize-1)
    return 0;
  if (map_get_terrain(x, y)==T_OCEAN)
    return 1;
  if (is_at_coast(x, y)) {
    map_set_terrain(x, y, T_RIVER);
    return 1;
  }

  if (map_get_terrain(x, y)==T_RIVER )
    return 0;
  map_set_terrain(x, y, map_get_terrain(x,y)+16);
  if (full_map(x, y-1)<mini+lrand48()%10 && map_get_terrain(x, y)<16) {
    mini=full_map(x, y-1);
    mp=0;
  }
  
  if (full_map(x, y+1)<mini+lrand48()%11 && map_get_terrain(x, y+1)<16) {
    mini=full_map(x, y+1);
    mp=1;
  }
  if (full_map(x+1, y)<mini+lrand48()%12 && map_get_terrain(x+1, y)<16) {
    mini=full_map(x+1, y);
    mp=2;
  }
  if (full_map(x-1, y)<mini+lrand48()%13 && map_get_terrain(x-1, y)<16) {
    mp=3;
  }
  if (mp==-1) {
    map_set_terrain(x, y, map_get_terrain(x, y)-16);
    return 0;
  }
  switch(mp) {
   case 0:
    res=make_river(x,y-1);
    break;
   case 1:
    res=make_river(x,y+1);
    break;
   case 2:
    res=make_river(x+1,y);
    break;
   case 3:
    res=make_river(x-1,y);
    break;
  }
  
  if (res) {
    map_set_terrain(x, y, T_RIVER);
  }
  else
    map_set_terrain(x, y, map_get_terrain(x ,y) - 16);
  return res;
}

void make_rivers()
{
  int x,y,i;
  i=0;

  while (i<map.riverlength) {
    y=lrand48()%map.ysize;
    x=lrand48()%map.xsize;
    if (map_get_terrain(x, y)==T_OCEAN ||map_get_terrain(x, y)==T_RIVER)
      continue;
    i+=make_river(x,y);
    i+=1;
  }
}

void make_plains()
{
  int x,y;
  for (y=0;y<map.ysize;y++)
    for (x=0;x<map.xsize;x++)
      if (map_get_terrain(x, y)==T_GRASSLAND && (lrand48()>>8)%100>50)
	map_set_terrain(x, y, T_PLAINS);
}
/* we want the map to be sailable east-west atleast at north and south pole */
/* make it a bit jagged aswell */
void make_passable()
{
  int x;
  
  for (x=0;x<map.xsize;x++) {
    map_set_terrain(x, 2, T_OCEAN);
    if (lrand48()%100>50) map_set_terrain(x,1,T_OCEAN);
    if (lrand48()%100>50) map_set_terrain(x,3,T_OCEAN);
    map_set_terrain(x, map.ysize-3, T_OCEAN);
    if (lrand48()%100>50) map_set_terrain(x,map.ysize-2,T_OCEAN);
    if (lrand48()%100>50) map_set_terrain(x,map.ysize-4,T_OCEAN);
  } 
  
}

/* we don't want huge areas of grass/plains, 
 * so we put in a hill here and there 
 * where it gets too 'clean' 
 */

void make_fair()
{
  int x,y;
  for (y=2;y<map.ysize-3;y++) 
    for (x=0;x<map.xsize;x++) {
      if (terrain_is_clean(x,y)) {
	map_set_terrain(x,y, T_HILLS);
	if (lrand48()%100>66 && map_get_terrain(x-1, y)!=T_OCEAN)
	  map_set_terrain(x-1,y, T_HILLS);
	if (lrand48()%100>66 && map_get_terrain(x+1, y)!=T_OCEAN)
	  map_set_terrain(x+1,y, T_HILLS);
	if (lrand48()%100>66 && map_get_terrain(x, y-1)!=T_OCEAN) 
	  map_set_terrain(x,y-1, T_HILLS);
	if (lrand48()%100>66 && map_get_terrain(x, y+1)!=T_OCEAN) 
	  map_set_terrain(x,y+1, T_HILLS);
      }
    }
}

void make_land()
{
  int x, y;
  int tres=(maxval*landpercent)/100;
  int count=0;
  int total=(map.xsize*map.ysize*landpercent)/100;
  int forever=0;
  do {
    forever++;
    if (forever>50) break; /* loop elimination */
    count=0;
    for (y=0;y<map.ysize;y++)
      for (x=0;x<map.xsize;x++) {
	if (full_map(x, y)<tres)
	  map_set_terrain(x, y, T_OCEAN);
	else {
	  map_set_terrain(x, y, T_GRASSLAND);
	  count++;
	}
      }
    if (count>total)
      tres=tres*1.1;
    else
      tres=tres*0.9;
  } while (abs(total-count)> maxval/40);
  make_passable();
  make_mountains(maxval*0.8);
  make_forests();
  make_swamps();
  make_deserts();
  make_rivers(); 
  make_plains();
  make_polar();
  make_fair();
}

int tiny_island(int x, int y) 
{
  if (map_get_terrain(x,y)==T_OCEAN) return 0;
  if (map_get_terrain(x-1,y)!=T_OCEAN) return 0;
  if (map_get_terrain(x+1,y)!=T_OCEAN) return 0;
  if (map_get_terrain(x,y-1)!=T_OCEAN) return 0;
  if (map_get_terrain(x,y+1)!=T_OCEAN) return 0;
  return 1;
}

void filter_land()
{
  int x,y;
  
  for (y=0;y<map.ysize;y++)
    for (x=0;x<map.xsize;x++) {
      /* Remove Islands that is only 1x1 */
      if (tiny_island(x,y)) {
	map_set_terrain(x,y, T_OCEAN);
      }
    }
}

int is_special_close(int x, int y)
{
  int x1,y1;
  for (x1=x-1;x1<x+2;x1++)
    for (y1=y-1;y1<=y;y1++) 
      if(map_get_tile(x1,y1)->special)
	return 1;
  return 0;
}

void add_specials(int prob)
{
  int x,y;
  for (y=1;y<map.ysize-1;y++)
    for (x=0;x<map.xsize; x++) {
      if ((map_get_terrain(x, y)==T_OCEAN && is_coastline(x,y)) 
	  || (map_get_terrain(x,y)!=T_OCEAN)) {
	if (lrand48()%1000<prob) {
	  if (!is_special_close(x,y))
	    map_get_tile(x,y)->special|=S_SPECIAL;
	}
      }
    }
}

struct isledata {
  int x,y;                        /* upper left corner of the islands */
  int goodies;
  int starters;
} islands[100];

int is_good_tile(int x, int y)
{
  int c;
  c=map_get_terrain(x,y);
  switch (c) {

  case T_FOREST:
  case T_GRASSLAND:
  case T_PLAINS:
  case T_RIVER:
    if (map_get_tile(x, y)->special) return 2;
    return 1;
  default:
  case T_HILLS:
    if (map_get_tile(x,y)->special) return 1;
    return 0;
  }
  return 0;
}

void flood_fill(int x, int y, int nr)
{
  if (x==-1) x=map.xsize-1;
  if (x==map.xsize) x=0;
  if (y==-1) return;
  if (y==map.ysize) return;

  if (full_map(x,y)) return;
  if (map_get_terrain(x, y)==T_OCEAN) return;
  islands[nr].goodies+=is_good_tile(x, y);
  height_map[y*map.xsize+x]=nr;
  flood_fill(x-1,y,nr);
  flood_fill(x-1,y+1,nr);
  flood_fill(x-1,y-1,nr);
  flood_fill(x,y-1,nr);
  flood_fill(x,y+1,nr);
  flood_fill(x+1,y-1,nr);
  flood_fill(x+1,y,nr);
  flood_fill(x+1,y+1,nr);
}

int same_island(int x1, int y1, int x2, int y2)
{
  return (full_map(x1,y1)==full_map(x2,y2));
}

int is_starter_close(int x, int y, int nr, int dist) 
{
  int i;
  if (map_get_terrain(x, y)!=T_PLAINS && map_get_terrain(x, y)!=T_GRASSLAND)
    return 1;
  if (full_map(x, y)<=2) return 1; /*don't want them starting
						    on the poles */
  for (i=0;i<nr;i++) 
    if ( same_island(x,y, map.start_positions[i].x, map.start_positions[i].y)
	 && (map.start_positions[i].x-x)*(map.start_positions[i].x-x)+
	 (map.start_positions[i].y-y)*(map.start_positions[i].y-y)<dist)
      return 1;
  return 0;
}


void flood_it()
{
  int x,y;
  int nr,good;
  int isles=3;
  int j=0;
  int dist=40;
  for (y=0;y<map.ysize;y++)
    for (x=0;x<map.xsize;x++)
      height_map[y*map.xsize+x]=0;
  flood_fill(0,0,1);                   /* want to know where the rim is */
  flood_fill(0,map.ysize-1,2);         /* ... */
  for (y=0;y<map.ysize;y++)
    for (x=0;x<map.xsize;x++) 
      if (!full_map(x, y) && map_get_terrain(x, y)!=T_OCEAN) {
	islands[isles].goodies=0;
	flood_fill(x,y,isles);
	islands[isles].starters=0;
	islands[isles].x=x;
	islands[isles].y=y;
	isles++;
      }
  good=0;
  for (x=0;x<isles;x++) {
    if (islands[x].goodies>20) 
      good+=islands[x].goodies;
  }
  for (x=0;x<isles;x++) {
    if (islands[x].goodies>20) {
      islands[x].starters=1+islands[x].goodies/(good/MAX_PLAYERS);
    } 
  }
  nr=0;
  while (nr<MAX_PLAYERS) {
    x=lrand48()%map.xsize;
    y=lrand48()%map.ysize;
    if (islands[full_map(x, y)].starters) {
      j++;
      if (!is_starter_close(x, y, nr, dist)) {
	islands[full_map(x, y)].starters--;
	map.start_positions[nr].x=x;
	map.start_positions[nr].y=y;
	nr++;
      }
      if (j>500) {
	  dist--;
	} 
	j=0;
    }
  } 
}


void map_fractal_generate(void)
{
  int x,y;
  int i;
  int minval=5000000;
  height_map=(int *) malloc (sizeof(int)*map.xsize*map.ysize);
  if(!(map.tiles=(struct tile*)malloc(map.xsize*map.ysize*
				      sizeof(struct tile)))) {
    fprintf(stderr, "malloc failed in mapgen\n");
    exit(1);
  }

  for(y=0;y<map.ysize;y++)
     for(x=0;x<map.xsize;x++)
      tile_init(map_get_tile(x, y));


  init_map();
  if (map.seed==0)
    srand48(time(NULL));
  else 
    srand48(map.seed);
  for (i=0;i<1500;i++) {
    height_map[lrand48()%(map.ysize*map.xsize)]+=lrand48()%5000;
    if (!(i%100)) {
      smooth_map(); 
    }
  }
  smooth_map(); 
  smooth_map(); 
  smooth_map(); 

  for (y=0;y<map.ysize;y++)
    for (x=0;x<map.xsize;x++) {
      if (full_map(x, y)>maxval) 
	maxval=full_map(x, y);
      if (full_map(x, y)<minval)
	minval=full_map(x, y);
    }
/*  printf("min %d max %d\n", minval, maxval);*/
  maxval-=minval;
  adjust_map(minval);
  make_land();
  filter_land();
  add_specials(map.riches); /* hvor mange promiller specials oensker vi*/

  
/*  for (y=0;y<map.ysize;y++) {
    for (x=0;x<map.xsize;x++) {
      putchar(terrain_chars[map_get_terrain(x, y)]);
    }
    puts("");
  }*/
  flood_it();
  make_huts(map.huts);
  free(height_map);
}

void adjust_map(int minval)
{
  int x,y;
  for (y=0;y<map.ysize;y++) {
    for (x=0;x<map.xsize;x++) {
      height_map[y*map.xsize+x]-=minval;
    }
  }
}

void init_map()
{
  int x,y;
  for (y=0;y<map.ysize;y++) {
    for (x=0;x<map.xsize;x++) {
      height_map[y*map.xsize+x]=lrand48()%40+(abs(500-y)/10);
    }
  }
}
	
void smooth_map()
{
  int x,y;
  int mx,my,px,py;
  int a;
  
  for (y=0;y<map.ysize;y++) {
    my=map_adjust_y(y-1);
    py=map_adjust_y(y+1);
    for (x=0;x<map.xsize;x++) {
      mx=map_adjust_x(x-1);
      px=map_adjust_x(x+1);
      a=full_map(x, y)*2;
      
      a+=full_map(px, my);
      a+=full_map(mx, my);
      a+=full_map(mx, py);
      a+=full_map(px, py);

      a+=full_map(x, my);
      a+=full_map(mx, y);
      
      a+=full_map(x, py);
      a+=full_map(px, y);

      a+=(lrand48()>>2)%60;
      a-=30;
      if (a<0) a=0;
      height_map[y*map.xsize +x]=a/10;
    }
  }
}

int is_water_adjacent(int x, int y)
{
  int x1,y1;
  for (y1=y-1;y1<y+2;y1++) 
    for (x1=x-1;x1<x+2;x1++) {
      if (map_get_terrain(x1,y1)==T_OCEAN || map_get_terrain(x1, y1)==T_RIVER)
	return 1;
    } 
  return 0;
}

int is_hut_close(int x, int y)
{
  int x1,y1;
  for (y1=y-3;y1<y+4;y1++) 
    for (x1=x-3;x1<x+4;x1++) {
      if (map_get_tile(x1,y1)->special&S_HUT)
	return 1;
    } 
  return 0;
}


void make_huts(int number)
{
  int x,y;
  int count=0;
  while ((number*map.xsize*map.ysize)/2000 && count++<map.xsize*map.ysize*2) {
    x=lrand48()%map.xsize;
    y=lrand48()%map.ysize;
    if (map_get_terrain(x, y)!=T_OCEAN) {
      if (!is_hut_close(x,y)) {
	number--;
	map_get_tile(x,y)->special|=S_HUT;
      }
    }
  }
}


