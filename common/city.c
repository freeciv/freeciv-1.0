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
#include "game.h"
#include "map.h"
#include "city.h"
#include "tech.h"

/****************************************************************
all the city improvements
use get_improvement_type(id) to access the array
*****************************************************************/
struct improvement_type improvement_types[]={
  {"Apollo Program", 1, A_SPACEFLIGHT, 600, 0, A_NONE},
  {"Aqueduct", 0, A_CONSTRUCTION, 120, 2, A_NONE},
  {"Bank", 0, A_BANKING, 120, 3, A_NONE},
  {"Barracks",0, A_NONE,  40, 0, A_GUNPOWDER},
  {"Barracks II",0, A_GUNPOWDER,  40, 1, A_COMBUSTION},
  {"Barracks III",0, A_COMBUSTION,  40, 2, A_NONE},
  {"Capitalization", 0, A_BANKING, 999, 0, A_NONE},
  {"Cathedral",0, A_RELIGION, 160, 3, A_NONE},
  {"City Walls",0, A_MASONRY, 120, 1, A_NONE},
  {"Coastal Defense",0, A_CONSTRUCTION, 120, 1, A_NONE},
  {"Colosseum",0, A_CONSTRUCTION, 100, 4, A_NONE},
  {"Colossus", 1, A_BRONZE, 200, 0, A_ELECTRICITY}, 
  {"Copernicus' Observatory", 1, A_ASTRONOMY, 300, 0, A_AUTOMOBILE},
  {"Courthouse", 0, A_CODE, 80, 1, A_NONE},
  {"Cure For Cancer", 1, A_GENETIC, 600, 0, A_NONE},
  {"Darwin's Voyage", 1, A_RAILROAD, 300, 0, A_NONE},
  {"Factory", 0, A_INDUSTRIALIZATION, 200, 4, A_NONE},
  {"Granary", 0, A_POTTERY, 60, 1, A_NONE},
  {"Great Library", 1, A_LITERACY, 300, 0, A_UNIVERSITY},
  {"Great Wall", 1, A_MASONRY, 300, 0, A_GUNPOWDER},
  {"Hanging Gardens", 1, A_POTTERY, 300, 0, A_INVENTION},
  {"Harbour", 0, A_MAGNETISM, 120, 4, A_NONE},
  {"Hoover Dam", 1, A_ELECTRONICS, 600, 0, A_NONE},
  {"Hydro Plant", 0, A_ELECTRONICS, 240, 4, A_NONE},
  {"Isaac Newton's College", 1, A_THEORY, 400, 0, A_FISSION},
  {"J.S.Bach's Cathedral", 1, A_RELIGION, 400, 0, A_NONE},
  {"Library", 0, A_WRITING, 80, 1, A_NONE},
  {"Lighthouse", 1, A_MAPMAKING, 300, 0, A_MAGNETISM},
  {"Magellan's Expedition", 1, A_NAVIGATION, 400, 0, A_NONE},
  {"Manhattan Project", 1, A_FISSION, 600, 0, A_NONE},
  {"Marketplace", 0, A_CURRENCY, 80, 1, A_NONE},
  {"Mass Transit", 0, A_MASS, 160, 4, A_NONE},
  {"MFG. Plant", 0, A_ROBOTICS, 320, 6, A_NONE},
  {"Michelangelo's Chapel", 1, A_RELIGION, 300, 0, A_COMMUNISM},
  {"Nuclear Plant", 0, A_POWER, 160, 2, A_NONE},
  {"Oracle", 1, A_MYSTICISM, 300, 0, A_RELIGION},
  {"Palace", 0, A_MASONRY, 200, 0, A_NONE},
  {"Power Plant",0, A_REFINING, 160, 4, A_NONE},
  {"Pyramids",1, A_MASONRY, 300, 0, A_COMMUNISM},
  {"Recycling Center", 0, A_RECYCLING, 200, 2, A_NONE},
  {"SDI Defense", 0, A_SUPERCONDUCTOR, 200, 4, A_NONE},
  {"SETI Program", 1, A_COMPUTERS, 600, 0, A_NONE},
  {"Shakespeare's Theatre", 1, A_MEDICINE, 400,0, A_ELECTRONICS},
  {"Temple", 0, A_CEREMONIAL, 40, 1, A_NONE},
  {"United Nations", 1, A_COMMUNISM, 600, 0, A_NONE},
  {"University", 0, A_UNIVERSITY, 160, 3, A_NONE},
  {"Women's Suffrage", 1, A_INDUSTRIALIZATION, 600, 0, A_NONE}
};


char *default_roman_city_names[] = {
  "Rome", "Caesarea", "Carthage", "Nicopolis",
  "Byzantium", "Brundisium", "Syracuse", "Antioch",
  "Palmyra", "Cyrene", "Gordion", "Tyrus", "Jerusalem",
  "Seleucia", "Ravenna", "Artaxata", (char *)0
};

char *default_babylonian_city_names[] = {
  "Babylon", "Sumer", "Uruk", "Nineveh", "Ashur",
  "Ellipi", "Akkad", "Eridu", "Kish", "Nippur",
  "Shuruppak", "Zariqum", "Izibia", "Nimrud", "Arbela",
  "Zamua", (char *)0
};

char *default_german_city_names[] = {
  "Berlin", "Leipzig", "Hamburg", "Bremen", "Frankfurt",
  "Bonn", "Nuremberg", "Cologne", "Hannover", "Munich",
  "Stuttgart", "Heidelburg", "Salzburg", "Konigsberg",
  "Dortmund", "Brandenburg", (char *)0
};

char *default_egyptian_city_names[] = {
  "Thebes", "Memphis", "Oryx", "Heliopolis", "Gaza",
  "Alexandria", "Byblos", "Cairo", "Coptos", "Edfu", "Pithom",
  "Busiris", "Athribis", "Mendes", "Tanis", "Abydos", (char *)0
};

char *default_american_city_names[] = {
  "Washington", "New York", "Boston", "Philadelphia",
  "Atlanta", "Chicago", "Buffalo", "St Louis", "Detroit",
  "New Orleans", "Baltimore", "Denver", "Cincinnati",
  "Dallas", "Los Angeles", "Las Vegas", (char *)0
};

char *default_greek_city_names[] = {
  "Athens", "Sparta", "Corinth", "Delphi", "Eretria",
  "Pharsalos", "Argos", "Mycenae", "Herakleia", "Antioch",
  "Ephesos", "Rhodes", "Knossos", "Troy", "Pergamon",
  "Miletos", (char *)0
};

char *default_indian_city_names[] = {
  "Delhi", "Bombay", "Madras", "Bangalore", "Calcutta",
  "Lahore", "Karachi", "Kolhapur", "Jaipur", "Hyderabad",
  "Bengal", "Chittagong", "Punjab", "Dacca", "Indus",
  "Ganges", (char *)0
};

char *default_russian_city_names[] = {
  "Moscow", "Leningrad", "Kiev", "Minsk", "Smolensk",
  "Odessa", "Sevastopol", "Tblisi", "Sverdlovsk", "Yakutsk",
  "Vladivostok", "Novograd", "Krasnoyarsk", "Riga", "Rostov",
  "Astrakhan", (char *)0
};

char *default_zulu_city_names[] = {
  "Zimbabwe", "Ulundi", "Bapedi", "Hlobane", "Isandhlwana",
  "Intombe", "Mpondo", "Ngome", "Swazi", "Tugela", "Umtata",
  "Umfolozi", "Ibabanago", "Isipezi", "Amatikulu",
  "Zunguin", (char *)0
};

char *default_french_city_names[] = {
  "Paris", "Orleans", "Lyons", "Tours", "Chartres",
  "Bordeaux", "Rouen", "Avignon", "Marseilles", "Grenoble",
  "Dijon", "Amiens", "Cherbourg", "Poitiers", "Toulouse",
  "Bayonne", (char *)0
};


char *default_aztec_city_names[] = {
  "Tenochtitlan", "Chiauhtia", "Chapultepec", "Coatepec",
  "Ayotzinco", "Itzapalapa", "Iztapam", "Mitxcoac",
  "Tacubaya", "Tecamac", "Tepezinco", "Ticoman", "Tlaxcala",
  "Xaltocan", "Xicalango", "Zumpanco", (char *)0
};

char *default_chinese_city_names[] = {
  "Peking", "Shanghai", "Canton", "Nanking", "Tsingtao",
  "Hangchow", "Tientsin", "Tatung", "Macao", "Anyang",
  "Shantung", "Chinan", "Kaifeng", "Ningpo", "Paoting",
  "Yangchow", (char *)0
};


char *default_english_city_names[] = {
  "London", "Coventry", "Birmingham", "Dover", "Nottingham",
  "York", "Liverpool", "Brighton", "Oxford", "Reading",
  "Exeter", "Cambridge", "Hastings", "Canterbury",
  "Banbury", "Newcastle", (char *)0
};


char *default_mongol_city_names[] = {
  "Samarkand", "Bokhara", "Nishapur", "Karakorum",
  "Kashgar", "Tabriz", "Aleppo", "Kabul", "Ormuz", "Basra",
  "Khanbalyk", "Khorasan", "Shangtu", "Kazan", "Quinsay",
  "Kerman", (char *)0
};

char **race_default_city_names[R_LAST]={
  default_roman_city_names,
  default_babylonian_city_names,
  default_german_city_names,
  default_egyptian_city_names,
  default_american_city_names,
  default_greek_city_names,
  default_indian_city_names,
  default_russian_city_names,
  default_zulu_city_names,
  default_french_city_names,
  default_aztec_city_names,
  default_chinese_city_names,
  default_english_city_names,
  default_mongol_city_names
};

extern struct unit_type unit_types[];
void citizen_happy_units(struct city *pcity, int unhap);

/****************************************************************
...
*****************************************************************/
char *get_improvement_name(enum improvement_type_id id)
{
  static char buffer[256];
  int state='W';
  if (!is_wonder(id)) 
    return get_improvement_type(id)->name;
  if (game.global_wonders[id]) state='B';
  if (wonder_is_obsolete(id)) state='O';
  sprintf(buffer, "%s(%c)", get_improvement_type(id)->name, state); 
  
  return buffer;
}

char *get_city_name_suggestion(struct player *pplayer)
{
  char **nptr;
  
  for(nptr=race_default_city_names[pplayer->race]; *nptr; nptr++) {
    struct city *pcity;
    if(!(pcity=game_find_city_by_name(*nptr)))
      return *nptr;
  }

  return "";
}

/****************************************************************
...
*****************************************************************/

int player_owns_city(struct player *pplayer, struct city *pcity)
{
  return (pcity->owner==pplayer->player_no);
}

int city_got_barracks(struct city *pcity)
{
  return (city_got_building(pcity, B_BARRACKS) || 
	  city_got_building(pcity, B_BARRACKS2) ||
	  city_got_building(pcity, B_BARRACKS3));
}

/**************************************************************************
...
**************************************************************************/

void set_worker_city(struct city *pcity, int x, int y, 
		     enum city_tile_type type) 
{
  pcity->city_map[x][y]=type;
}

/**************************************************************************
...
**************************************************************************/

int can_sell_building(struct city *pcity, int id)
{

  return (city_got_building(pcity, id) ? (!is_wonder(id)) : 0);
}

/**************************************************************************
...
**************************************************************************/

int building_value(enum improvement_type_id id)
{
  return (improvement_types[id].build_cost);
}

/**************************************************************************
...
**************************************************************************/

char *building_name(enum improvement_type_id id)
{
  return improvement_types[id].name;
}

/**************************************************************************
...
**************************************************************************/

int is_wonder(enum improvement_type_id id)
{
  return (improvement_types[id].is_wonder);
}

/**************************************************************************
...
**************************************************************************/

int building_obsolete(struct player *pplayer, enum improvement_type_id id) 
{
  if (improvement_types[id].obsolete_by==A_NONE) 
    return 0;
  return (get_invention(pplayer, improvement_types[id].obsolete_by)
	  ==TECH_KNOWN);
}

/**************************************************************************
...
**************************************************************************/
  
int build_cost(struct city *pcity)
{
  int total,cost;
  int build=pcity->shield_stock;
  if (pcity->is_building_unit) {
    total=unit_value(pcity->currently_building);
    if (build>=total)
      return 0;
    cost=(total-build)*2+(total-build)*(total-build)/20; 
    if (!build)
      cost*=2;
  } else {
    total=building_value(pcity->currently_building);
    if (build>=total)
      return 0;
    cost=(total-build)*2;
    if(!build)
      cost*=2;
    if(is_wonder(pcity->currently_building))
      cost*=2;
  }
  return cost;
}

/**************************************************************************
...
**************************************************************************/

enum city_tile_type get_worker_city(struct city *pcity, int x, int y)
{
  if (x==0 && y==0) return C_TILE_UNAVAILABLE;
  if (x==4 && y==0) return C_TILE_UNAVAILABLE;
  if (x==0 && y==4) return C_TILE_UNAVAILABLE;
  if (x==4 && y==4) return C_TILE_UNAVAILABLE;
  return(pcity->city_map[x][y]);
}

/**************************************************************************
...
**************************************************************************/

int wonder_is_obsolete(enum improvement_type_id id)
{ 
  if (improvement_types[id].obsolete_by==A_NONE)
    return 0;
  return (game.global_advances[improvement_types[id].obsolete_by]);
}

/**************************************************************************
...
**************************************************************************/

struct city *find_city_wonder(enum improvement_type_id id)
{
  return (find_city_by_id(game.global_wonders[id]));
}

/**************************************************************************
...
**************************************************************************/

struct improvement_type *get_improvement_type(enum improvement_type_id id)
{
  return &improvement_types[id];
}

/**************************************************************************
...
**************************************************************************/

struct player *city_owner(struct city *pcity)
{
  return (&game.players[pcity->owner]);
}

/****************************************************************
...
*****************************************************************/

int can_build_improvement(struct city *pcity, enum improvement_type_id id)
{
  struct player *p=city_owner(pcity);
  if (id<0 || id>B_LAST)
    return 0;
  if (city_got_building(pcity, id) || 
      get_invention(p,improvement_types[id].tech_requirement)!=TECH_KNOWN) 
    return 0;
  if ((city_got_building(pcity, B_HYDRO)|| city_got_building(pcity, B_POWER) ||
      city_got_building(pcity, B_NUCLEAR)) && (id==B_POWER || id==B_HYDRO || id==B_NUCLEAR))
    return 0;
  if (id==B_UNIVERSITY && !city_got_building(pcity, B_LIBRARY))
    return 0;
  if (id==B_BANK && !city_got_building(pcity, B_MARKETPLACE))
    return 0;
  if (id==B_MFG && !city_got_building(pcity, B_FACTORY))
    return 0;
  if ((id==B_HARBOUR || id==B_COASTAL) && !is_terrain_near_tile(pcity->x, pcity->y, T_OCEAN))
    return 0;
  if(building_obsolete(p, id)) return 0;
  if (is_wonder(id) && game.global_wonders[id])
    return 0;
  return 1;
}

/****************************************************************
...
*****************************************************************/

int can_build_unit(struct city *pcity, enum unit_type_id id)
{  
  struct player *p=city_owner(pcity);
  if (id<0 || id>=U_LAST) return 0;
  if (id==U_NUCLEAR && !game.global_wonders[B_MANHATTEN])
    return 0;
  if (get_invention(p,unit_types[id].tech_requirement)!=TECH_KNOWN)
    return 0;
  if (!is_terrain_near_tile(pcity->x, pcity->y, T_OCEAN) && is_water_unit(id))
    return 0;
  if  (get_invention(p, unit_types[id].obsoleted_by)==TECH_KNOWN)
    return 0;
  return 1;
}

/**************************************************************************
...
**************************************************************************/

int city_population(struct city *pcity)
{
  int i;
  int res=0;
  for (i=1;i<=pcity->size;i++) res+=i;
  return res*10000;
}

/**************************************************************************
...
**************************************************************************/

int city_got_building(struct city *pcity,  enum improvement_type_id id) 
{
  if (id<0 || id>B_LAST)
    return 0;
  else 
    return (pcity->improvements[id]);
}

/**************************************************************************
...
**************************************************************************/

int buildings_upkeep(struct city *pcity)
{
  int i;
  int cost=0;

  for (i=0;i<B_LAST;i++) 
    if (city_got_building(pcity, i)) 
      cost+=improvement_types[i].shield_upkeep;

  return cost;
}

/**************************************************************************
...
**************************************************************************/

int get_shields_tile(int x, int y, struct city *pcity)
{
  int s=0;
  enum tile_special_type spec_t=map_get_special(pcity->x+x-2, pcity->y+y-2);
  enum tile_terrain_type tile_t=map_get_terrain(pcity->x+x-2, pcity->y+y-2);

  int gov=get_goverment(pcity->owner);
  if (is_city_celebrating(pcity))
    gov+=2;
  
  if (spec_t & S_SPECIAL) 
    s=get_tile_type(tile_t)->shield_special;
  else
    s=get_tile_type(tile_t)->shield;
  
  if (spec_t & S_MINE) {
    s++;
    if (tile_t == T_HILLS) 
      s+=2;
  }
  if (spec_t & S_RAILROAD)
    s*=1.5;
  if (s>2 && gov <=G_DESPOTISM) 
    s--;
  if (spec_t & S_POLUTION) return s=(s+1)/2; /* The shields here is icky */
  return s;
}

/**************************************************************************
...
**************************************************************************/

int get_trade_tile(int x, int y, struct city *pcity)
{
  enum tile_special_type spec_t=map_get_special(pcity->x+x-2, pcity->y+y-2);
  enum tile_terrain_type tile_t=map_get_terrain(pcity->x+x-2, pcity->y+y-2);
  int t;
  int gov=get_goverment(pcity->owner);
  if (is_city_celebrating(pcity)) 
    gov+=2;
 
  if (spec_t & S_SPECIAL) 
    t=get_tile_type(tile_t)->trade_special;
  else
    t=get_tile_type(tile_t)->trade;
    
  if (spec_t & S_ROAD) {
    switch (tile_t) {
    case T_DESERT:
    case T_GRASSLAND:
    case T_PLAINS:
      t++;
    default:
      break;
    }
  }
  if (gov >=G_REPUBLIC && t)
    t++; 
  if (spec_t & S_RAILROAD)
    t*=1.5;
  
  if (t>2 && gov <=G_DESPOTISM) 
    t--;
  if (spec_t & S_POLUTION) t=(t+1)/2; /* The trade here is dirty */
  return t;
}

/***************************************************************
Here the exact food production should be calculated. That is
including ALL modifiers. 
Center tile acts as irrigated...
***************************************************************/

int get_food_tile(int x, int y, struct city *pcity)
{
  int f;
  enum tile_special_type spec_t=map_get_special(pcity->x+x-2, pcity->y+y-2);
  enum tile_terrain_type tile_t=map_get_terrain(pcity->x+x-2, pcity->y+y-2);
  int gov=get_goverment(pcity->owner);
  if (is_city_celebrating(pcity))
    gov+=2;

  if (spec_t & S_SPECIAL) 
    f=get_tile_type(tile_t)->food_special;
  else
    f=get_tile_type(tile_t)->food;

  if (spec_t & S_IRRIGATION || (x==2 && y==2 && f))
    f++;
  if (city_got_building(pcity, B_HARBOUR) && tile_t==T_OCEAN)
    f++;

  if (spec_t & S_RAILROAD)
    f*=1.5;

  if (f>2 && gov <=G_DESPOTISM) 
    f--;

  if (spec_t & S_POLUTION)
    f=(f+1)/2;

  return f;
}

/**************************************************************************
Locate the city where the players palace is located, (NULL Otherwise) 
**************************************************************************/

struct city *find_palace(struct player *pplayer)
{
  struct genlist_iterator myiter;
  struct city *pcity;
  genlist_iterator_init(&myiter, &pplayer->cities.list, 0);
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    pcity=(struct city *)ITERATOR_PTR(myiter);
    if (city_got_building(pcity, B_PALACE)) 
      return pcity;
  }
  return NULL;
}

/**************************************************************************
...
**************************************************************************/

int can_establish_trade_route(struct city *pc1, struct city *pc2)
{
  int i;
  int r=1;
  if(!pc1 || !pc2 || pc1==pc2 ||
     (pc1->owner==pc2->owner && 
      map_distance(pc1->x, pc1->y, pc2->x, pc2->y)<=8))
    return 0;
  
  for(i=0;i<4;i++) {
    r*=pc1->trade[i];
    if (pc1->trade[i]==pc2->id) return 0;
  }
  if (r) return 0;
  for (i=0;i<4;i++) 
    r*=pc2->trade[i];
  return (!r);
}

/**************************************************************************
Establish a trade route, notice that there has to be space for them, 
So use can_establish_Trade_route first.
returns the revenue aswell.
**************************************************************************/

int establish_trade_route(struct city *pc1, struct city *pc2)
{
  int i;
  int tb;
  for (i=0;i<4;i++) {
    if (!pc1->trade[i]) {
      pc1->trade[i]=pc2->id;
      break;
    }
  }
  for (i=0;i<4;i++) {
    if (!pc2->trade[i]) {
      pc2->trade[i]=pc1->id;
      break;
    }
  }

  tb=(map_distance(pc1->x, pc1->y, pc2->x, pc2->y)+10);
  tb=(tb*(pc1->trade_prod+pc2->trade_prod))/24;
  if (map_distance(pc1->x, pc1->y, pc2->x, pc2->y)<(map.xsize+map.ysize)/6)
    tb*=0.5;
  if (pc1->owner==pc2->owner)
    tb*=0.5;
  if (get_invention(city_owner(pc1), A_RAILROAD)==TECH_KNOWN)
    tb*=0.66;
  if (get_invention(city_owner(pc1), A_FLIGHT)==TECH_KNOWN)
    tb*=0.66;
  return tb;
}

/**************************************************************************
...
**************************************************************************/

int trade_between_cities(struct city *pc1, struct city *pc2)
{
  int bonus=0;

  if (pc2 && pc1) {
    bonus=(pc1->trade_prod+pc2->trade_prod+4)/8;
    if (map_distance(pc1->x, pc1->y, pc2->x, pc2->y)<(map.xsize+map.ysize)/6)
      bonus/=2;
	
    if (pc1->owner==pc2->owner)
      bonus/=2;
  }
  return bonus;
}

/**************************************************************************
corruption, corruption is halfed during love the XXX days.
**************************************************************************/
void set_trade_prod(struct city *pcity)
{
  struct city *capital;
  int i;
  int dist;
  int corruption[]= { 12,8,20,24,20,0}; /* original {12,8,16,20,24,0} */
  for (i=0;i<4;i++) {
    pcity->trade_prod+=trade_between_cities(pcity, find_city_by_id(pcity->trade[i]));
  }
  if (get_goverment(pcity->owner)==G_DEMOCRACY) {
    pcity->corruption=0;
    return;
  }
  if (get_goverment(pcity->owner)==G_COMMUNISM) {
    dist=10;
  } else { 
    capital=find_palace(city_owner(pcity));
    if (!capital)  
      dist=32;
    else 
      dist=min(32,map_distance(capital->x, capital->y, pcity->x, pcity->y)); 
  }
    pcity->corruption=(pcity->trade_prod*dist*3)/(corruption[get_goverment(pcity->owner)]*10);
  
  if (city_got_building(pcity, B_COURTHOUSE) || 
      city_got_building(pcity, B_PALACE)) 
    pcity->corruption/=2;
  pcity->trade_prod-=pcity->corruption;
}

/*************************************************************************
Calculate amount of gold remaining in city after paying for buildings
*************************************************************************/

int calc_gold_remains(struct city *pcity)
{
  int cost=0;
  int i;
  for (i=0;i<B_LAST;i++) 
    if (city_got_building(pcity, i)) 
      cost+=improvement_types[i].shield_upkeep;
  return pcity->tax_total-cost;
}

/**************************************************************************
calculate the incomes according to the taxrates and # of specialists.
**************************************************************************/

void set_tax_income(struct city *pcity)
{
  pcity->luxury_total=(pcity->trade_prod*
	    game.players[pcity->owner].economic.luxury)/100;
  pcity->science_total=(pcity->trade_prod*
			game.players[pcity->owner].economic.science)/100;
  pcity->tax_total=pcity->trade_prod-
    (pcity->luxury_total+pcity->science_total);
 
  pcity->luxury_total+=(pcity->ppl_elvis*2); 
  pcity->science_total+=(pcity->ppl_scientist*3);
  pcity->tax_total+=(pcity->ppl_taxman*3);          
}
/**************************************************************************
Modify the incomes according to various buildings. 
**************************************************************************/

void add_buildings_effect(struct city *pcity)
{
  int tmp=0;
  int tax_bonus=100;
  int shield_bonus=100;
  int science_bonus=100;

  if (city_got_building(pcity, B_MARKETPLACE))  
    tax_bonus+=50;
  if (city_got_building(pcity, B_BANK)) 
    tax_bonus+=50;
  
  if (city_got_building(pcity, B_UNIVERSITY)) 
    science_bonus+=50;
  if (city_got_building(pcity, B_LIBRARY)) 
    science_bonus+=50;
  if (city_affected_by_wonder(pcity, B_COPERNICUS)) 
    science_bonus*=1.5;
  if (city_affected_by_wonder(pcity, B_ISAAC))
    science_bonus*=1.5;
  if (city_affected_by_wonder(pcity, B_SETI))
    science_bonus*=1.5;

  if (city_got_building(pcity, B_MFG)) 
    tmp=pcity->shield_prod;
  else if (city_got_building(pcity, B_FACTORY)) 
    tmp=pcity->shield_prod/2;
  if (city_affected_by_wonder(pcity, B_HOOVER) || 
      city_got_building(pcity, B_POWER) ||
      city_got_building(pcity, B_HYDRO) ||
      city_got_building(pcity,B_NUCLEAR)) 
    shield_bonus+=50;
  
  pcity->shield_prod+=((tmp*shield_bonus)/100);
  pcity->luxury_total=(pcity->luxury_total*tax_bonus)/100;
  pcity->tax_total=(pcity->tax_total*tax_bonus)/100;
  pcity->science_total=(pcity->science_total*science_bonus)/100;
  pcity->shield_surplus=pcity->shield_prod;
}

/**************************************************************************
...
**************************************************************************/
int city_got_citywalls(struct city *pcity)
{
  if (city_got_building(pcity, B_CITY))
    return 1;
  return (city_affected_by_wonder(pcity, B_WALL));
}

/**************************************************************************
...
**************************************************************************/

int nr_specialists(struct city *pcity)
{
  return (pcity->ppl_elvis+pcity->ppl_scientist+pcity->ppl_taxman);
}
/**************************************************************************
...
**************************************************************************/
int is_worker_here(struct city *pcity, int x, int y) 
{
  if (get_worker_city(pcity,x,y)!=C_TILE_WORKER) 
    return 0;
  if (x==0 && y==0) return 0;
  if (x==0 && y==4) return 0;
  if (x==4 && y==0) return 0;
  if (x==4 && y==4) return 0; 
  return 1;
}
/**************************************************************************
...
**************************************************************************/

void citizen_happy_size(struct city *pcity)
{
  pcity->ppl_unhappy[0]=max(0, (pcity->size-(nr_specialists(pcity)+game.unhappysize)));
  pcity->ppl_content[0]=pcity->size-(nr_specialists(pcity)+pcity->ppl_unhappy[0]);
  pcity->ppl_happy[0]=0;
}
/**************************************************************************
...
**************************************************************************/
void happy_copy(struct city *pcity, int i)
{  
  pcity->ppl_unhappy[i+1]=pcity->ppl_unhappy[i];
  pcity->ppl_content[i+1]=pcity->ppl_content[i];
  pcity->ppl_happy[i+1]=pcity->ppl_happy[i];
}

/**************************************************************************
...
**************************************************************************/
void citizen_happy_luxury(struct city *pcity)
{
  int x=pcity->luxury_total;
  happy_copy(pcity, 0);
  /* make people happy, content are made happy first, then unhappy content etc.   each conversions costs 2 luxuries. */
  while (x>=2 && pcity->ppl_content[1]+pcity->ppl_unhappy[1]) {
    if (pcity->ppl_content[1]) {
      pcity->ppl_content[1]--;
      pcity->ppl_happy[1]++;
      x-=2;
    }
    if (x>=2 && pcity->ppl_unhappy[1]) {
      pcity->ppl_unhappy[1]--;
      pcity->ppl_content[1]++;
      x-=2;
    }
  }
}

/**************************************************************************
...
**************************************************************************/
void citizen_happy_units(struct city *pcity, int unhap)
{
  if (city_affected_by_wonder(pcity, B_WOMENS)) 
    if (get_goverment(pcity->owner)==G_DEMOCRACY)
      unhap-=2;
    else
      unhap--;

    while (pcity->ppl_content[3] && unhap>0) {
      pcity->ppl_content[3]--;
      pcity->ppl_unhappy[3]++;
      unhap--;
    }
    while (unhap>0 && pcity->ppl_happy[3]) {
      pcity->ppl_happy[3]--;
      pcity->ppl_content[3]++;
      unhap--;
    } 
    /* MAKE VERY UNHAPPY DUDES WITH THE REST */

}

/**************************************************************************
...
**************************************************************************/
int city_affected_by_wonder(struct city *pcity, enum improvement_type_id id) /*FIX*/
{
  struct city *tmp;
  if (id<0 || id>=B_LAST || !is_wonder(id) || wonder_is_obsolete(id))
    return 0;
  tmp=find_city_by_id(game.global_wonders[id]);
  if (!tmp)
    return 0;
  if (id==B_MANHATTEN) 
    return 1;
  if (tmp->owner!=pcity->owner)
    return 0;
  if (city_got_building(pcity, id))
    return 1;
  switch (id) {
  case B_APOLLO:
  case B_CURE:
  case B_GREAT:
  case B_WALL:
  case B_HANGING:
  case B_ORACLE:
  case B_UNITED:
  case B_WOMENS:
  case B_DARWIN:
  case B_MAGELLAN:
  case B_MICHELANGELO:
  case B_SETI:
  case B_PYRAMIDS:
    return 1;
  case B_ISAAC:
  case B_COPERNICUS:
  case B_SHAKESPEARE:
  case B_COLLOSSUS:
    return 0;
  case B_HOOVER:
  case B_BACH:
    return (map_distance(tmp->x, tmp->y, tmp->x,pcity->y)<25);  
  default:
    return 0;
  }
} 
/**************************************************************************
...
**************************************************************************/

int get_temple_power(struct city *pcity)
{
  struct player *p=&game.players[pcity->owner];
  int power=1;
  if (get_invention(p, A_MYSTICISM)==TECH_KNOWN)
    power=2;
  if (city_affected_by_wonder(pcity, B_ORACLE)) 
    power*=2;
  return power;
}

/**************************************************************************
...
**************************************************************************/
int get_cathedral_power(struct city *pcity)
{
  int power=4;
  if (city_affected_by_wonder(pcity, B_MICHELANGELO)) 
    power+=2;
  return power;
}

/**************************************************************************
...
**************************************************************************/
void citizen_happy_buildings(struct city *pcity)
{
  int faces=0;
  happy_copy(pcity, 1);
  
  if (city_got_building(pcity,B_TEMPLE)) { 
    faces+=get_temple_power(pcity);
  }
  if (city_got_building(pcity,B_COURTHOUSE) &&
      get_goverment(pcity->owner) == G_DEMOCRACY ) {
    faces++;
  }

   if (city_got_building(pcity, B_COLOSSEUM)) 
    faces+=3;
  if (city_got_building(pcity, B_CATHEDRAL)) 
    faces+=get_cathedral_power(pcity);
  while (faces && pcity->ppl_unhappy[2]) {
    pcity->ppl_unhappy[2]--;
    pcity->ppl_content[2]++;
    faces--;
  }
  while (faces && pcity->ppl_content[2]) { /*maybe remove this if*/
    pcity->ppl_content[2]--;
    pcity->ppl_happy[2]++;
    faces--;
  }
}

/**************************************************************************
...
**************************************************************************/
void citizen_happy_wonders(struct city *pcity)
{
  int bonus=0;
  happy_copy(pcity, 3);
  
  if (city_affected_by_wonder(pcity, B_HANGING)) {
    if (pcity->ppl_unhappy[4]) {
      pcity->ppl_unhappy[4]--;
      pcity->ppl_happy[4]++;
    } else if (pcity->ppl_content[4]) {
      pcity->ppl_content[4]--;
      pcity->ppl_happy[4]++;
    }
  }
  if (city_affected_by_wonder(pcity, B_BACH)) 
    bonus+=2;
  if (city_affected_by_wonder(pcity, B_CURE))
    bonus+=1;
  while (bonus && pcity->ppl_unhappy[4]) {
    pcity->ppl_unhappy[4]--;
    pcity->ppl_content[4]++;
    bonus--;
  } 
  while (bonus && pcity->ppl_content[4]) {
    pcity->ppl_content[4]--;
    pcity->ppl_happy[4]++;
    bonus--;
  }
  if (city_affected_by_wonder(pcity, B_SHAKESPEARE)) {
    pcity->ppl_content[4]+=pcity->ppl_unhappy[4];
    pcity->ppl_unhappy[4]=0;
  }
}

/***************************************************************
...
***************************************************************/

int is_city_happy(struct city *pcity)
{
  return (pcity->ppl_happy[4]>=(pcity->size+1)/2 && pcity->ppl_unhappy[4]==0);
}

/**************************************************************************
...
**************************************************************************/
int is_city_unhappy(struct city *pcity)
{
  return (pcity->ppl_happy[4]<pcity->ppl_unhappy[4]);
}
/**************************************************************************
...
**************************************************************************/

void unhappy_city_check(struct city *pcity)
{
  if (is_city_unhappy(pcity)) {
    pcity->food_surplus=min(0, pcity->food_surplus);
    pcity->tax_total=0;
    pcity->science_total=0;
    pcity->shield_surplus=min(0, pcity->shield_surplus);
  }  
}
/**************************************************************************
...
**************************************************************************/

void set_polution(struct city *pcity)
{
  int mod=0;
  int poppul=0;
  struct player *pplayer=&game.players[pcity->owner];
  pcity->polution=pcity->shield_prod;
  if (city_got_building(pcity, B_RECYCLING))
    pcity->polution/=3;
  else  if (city_got_building(pcity, B_HYDRO) ||
	   city_affected_by_wonder(pcity, B_HOOVER) ||
	    city_got_building(pcity, B_NUCLEAR))
    pcity->polution/=2;
  
  if (!city_got_building(pcity, B_MASS)) {
    if (get_invention(pplayer, A_INDUSTRIALIZATION)==TECH_KNOWN)  mod=1;
    if (get_invention(pplayer, A_AUTOMOBILE)==TECH_KNOWN) mod=2;
    if (get_invention(pplayer, A_MASS)==TECH_KNOWN) mod=3;
    if (get_invention(pplayer, A_PLASTICS)==TECH_KNOWN) mod=4;
    poppul=(pcity->size*mod)/4;
    pcity->polution+=poppul;
  }

  pcity->polution=max(0, pcity->polution-20);  
}

int is_city_celebrating(struct city *pcity)
{
  return (pcity->size>=5 && is_city_happy(pcity));
}

/***************************************************************
...
***************************************************************/
struct city *find_city_by_id(int id)
{
  int i;

  for(i=0; i<game.nplayers; i++) {
    struct city *pcity;
    
    if((pcity=city_list_find_id(&game.players[i].cities, id)))
      return pcity;
  }
  return 0;
}


/**************************************************************************
...
**************************************************************************/
void city_list_init(struct city_list *this)
{
  genlist_init(&this->list);
}

/**************************************************************************
...
**************************************************************************/
struct city *city_list_get(struct city_list *this, int index)
{
  return (struct city *)genlist_get(&this->list, index);
}


/**************************************************************************
...
**************************************************************************/
struct city *city_list_find_id(struct city_list *this, int id)
{
  if(id) {
    struct genlist_iterator myiter;

    genlist_iterator_init(&myiter, &this->list, 0);
    
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    if(((struct city *)ITERATOR_PTR(myiter))->id==id)
	return ITERATOR_PTR(myiter);
  }
  return 0;
}

/**************************************************************************
...
**************************************************************************/
struct city *city_list_find_name(struct city_list *this, char *name)
{
  struct genlist_iterator myiter;

  genlist_iterator_init(&myiter, &this->list, 0);

  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter))
    if(!mystrcasecmp(name, ((struct city *)ITERATOR_PTR(myiter))->name))
      return ITERATOR_PTR(myiter);

  return 0;
}


/**************************************************************************
...
**************************************************************************/
struct city *city_list_find_coor(struct city_list *this, int x, int y)
{
  struct genlist_iterator myiter;

  genlist_iterator_init(&myiter, &this->list, 0);

  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct city *pcity=((struct city *)ITERATOR_PTR(myiter));
    if(pcity->x==x && pcity->y==y)
      return pcity;
  }

  return 0;
}


/**************************************************************************
...
**************************************************************************/
void city_list_insert(struct city_list *this, struct city *pcity)
{
  genlist_insert(&this->list, pcity, 0);
}


/**************************************************************************
...
**************************************************************************/
int city_list_size(struct city_list *this)
{
  return genlist_size(&this->list);
}

/**************************************************************************
...
**************************************************************************/
void city_list_unlink(struct city_list *this, struct city *pcity)
{
  genlist_unlink(&this->list, pcity);
}









