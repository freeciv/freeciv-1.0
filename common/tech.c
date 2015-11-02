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
#include "tech.h"
#include "player.h"
#include "game.h"

struct advance advances[]= {
  {"None",                  {A_NONE, A_NONE} }, 
  {"Advanced Flight",       { A_FLIGHT, A_ELECTRICITY} },
  {"Alphabet",              { A_NONE, A_NONE} },
  {"Astronomy",             { A_MYSTICISM, A_MATHEMATICS} },
  {"Atomic Theory",         { A_THEORY, A_PHYSICS} },
  {"Automobile",            { A_COMBUSTION, A_STEEL} },
  {"Banking",               { A_TRADE, A_REPUBLIC} },
  {"Bridge Building",       { A_IRON, A_CONSTRUCTION} },
  {"Bronze Working",        { A_NONE, A_NONE} },
  {"Ceremonial Burial",     { A_NONE, A_NONE} },
  {"Chemistry",             { A_UNIVERSITY, A_MEDICINE} },
  {"Chivalry",              { A_FEUDALISM, A_HORSEBACK} },
  {"Code of Laws",          { A_ALPHABET, A_NONE} },
  {"Combustion",            { A_REFINING, A_EXPLOSIVES} },
  {"Communism",             { A_PHILOSOPHY, A_INDUSTRIALIZATION} },
  {"Computers",             { A_MATHEMATICS, A_ELECTRONICS} },
  {"Conscription",          { A_REPUBLIC, A_EXPLOSIVES} },
  {"Construction",          { A_MASONRY, A_CURRENCY} },
  {"Currency",              { A_BRONZE, A_NONE} },
  {"Democracy",             { A_PHILOSOPHY, A_LITERACY} },
  {"Electricity",           { A_METALLURGY, A_MAGNETISM} },
  {"Electronics",           { A_ENGINEERING, A_ELECTRICITY} },
  {"Engineering",           { A_WHEEL, A_CONSTRUCTION} },
  {"Explosives",            { A_GUNPOWDER, A_CHEMISTRY} },
  {"Feudalism",             { A_MASONRY, A_MONARCHY} },
  {"Flight",                { A_COMBUSTION, A_PHYSICS} },
  {"Fusion Power",          { A_POWER, A_SUPERCONDUCTOR} },
  {"Genetic Engineering",   { A_MEDICINE, A_CORPORATION} },
  {"Gunpowder",             { A_INVENTION, A_IRON} },
  {"Horseback Riding",      { A_NONE, A_NONE} },
  {"Industrialization",     { A_RAILROAD, A_BANKING} },
  {"Invention",             { A_ENGINEERING, A_LITERACY} },
  {"Iron Working",          { A_BRONZE, A_NONE} },
  {"Labor Union",           { A_MASS, A_COMMUNISM} },
  {"Literacy",              { A_WRITING, A_CODE} },
  {"Magnetism",             { A_NAVIGATION, A_PHYSICS} },
  {"MapMaking",             { A_ALPHABET, A_NONE} },
  {"Masonry",               { A_NONE, A_NONE} },
  {"Mass Production",       { A_AUTOMOBILE, A_CORPORATION} },
  {"Mathematics",           { A_ALPHABET, A_MASONRY} },
  {"Medicine",              { A_PHILOSOPHY, A_TRADE} },
  {"Metallurgy",            { A_GUNPOWDER, A_UNIVERSITY} },
  {"Monarchy",              { A_CEREMONIAL, A_CODE} },
  {"Mysticism",             { A_CEREMONIAL, A_NONE} },
  {"Navigation",            { A_MAPMAKING, A_ASTRONOMY} },
  {"Nuclear Fission",       { A_MASS, A_ATOMIC} },
  {"Nuclear Power",         { A_FISSION, A_ELECTRONICS} },
  {"Philosophy",            { A_MYSTICISM, A_LITERACY} },
  {"Physics",               { A_MATHEMATICS, A_NAVIGATION} },
  {"Plastics",              { A_REFINING, A_SPACEFLIGHT} },
  {"Pottery",               { A_NONE, A_NONE} },
  {"Railroad",              { A_STEAM, A_BRIDGE} },
  {"Recycling",             { A_MASS, A_DEMOCRACY} },
  {"Refining",              { A_CHEMISTRY, A_CORPORATION} },
  {"Religion",              { A_PHILOSOPHY, A_WRITING} },
  {"Robotics",              { A_PLASTICS, A_COMPUTERS} },
  {"Rocketry",              { A_ADVANCED, A_ELECTRONICS} },
  {"Space Flight",          { A_COMPUTERS, A_ROCKETRY} },
  {"Steam Engine",          { A_PHYSICS, A_INVENTION} },
  {"Steel",                 { A_METALLURGY, A_INDUSTRIALIZATION} },
  {"Superconductors",       { A_PLASTICS, A_MASS} },
  {"The Corporation",       { A_BANKING, A_INDUSTRIALIZATION} },
  {"The Republic",          { A_CODE, A_LITERACY} },
  {"The Wheel",             { A_NONE, A_NONE} },
  {"Theory Of Gravity",     { A_ASTRONOMY, A_UNIVERSITY} },
  {"Trade",                 { A_CURRENCY, A_CODE} },
  {"University",            { A_MATHEMATICS, A_PHILOSOPHY} },
  {"Writing",               { A_ALPHABET, A_NONE }}
};


/**************************************************************************
...
**************************************************************************/
int get_invention(struct player *plr, int tech)
{
  if(tech<0 || tech>=A_LAST)
    return 0;
  return plr->research.inventions[tech];
}

/**************************************************************************
...
**************************************************************************/
void set_invention(struct player *plr, int tech, int value)
{
  if(plr->research.inventions[tech]==value)
    return;

  plr->research.inventions[tech]=value;

  if(value==TECH_KNOWN) {
    game.global_advances[tech]++;
  }
}


/**************************************************************************
...
**************************************************************************/
void update_research(struct player *plr) 
{
  int i;
  for (i=0;i<A_LAST;i++) 
    if(get_invention(plr, i)==0 
       && get_invention(plr, advances[i].req[0])==TECH_KNOWN   
       && get_invention(plr, advances[i].req[1])==TECH_KNOWN)
      plr->research.inventions[i]=TECH_REACHABLE;
/*  
      
      set_invention(plr, i, TECH_REACHABLE); */
}

