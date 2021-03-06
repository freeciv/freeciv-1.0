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

#include "genlist.h"
#include "player.h"

#include "diptreaty.h"


/****************************************************************
...
*****************************************************************/
void init_treaty(struct Treaty *ptreaty, 
		 struct player *plr0, struct player *plr1)
{
  ptreaty->plr0=plr0;
  ptreaty->plr1=plr1;
  ptreaty->accept0=0;
  ptreaty->accept1=0;
  genlist_init(&ptreaty->clauses);
}


/****************************************************************
...
*****************************************************************/
int remove_clause(struct Treaty *ptreaty, struct player *pfrom, 
		  enum clause_type type, int val)
{
  struct genlist_iterator myiter;
  
  genlist_iterator_init(&myiter, &ptreaty->clauses, 0);
  
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct Clause *pclause=(struct Clause *)ITERATOR_PTR(myiter);
    if(pclause->type==type && pclause->from==pfrom &&
       pclause->value==val) {
      genlist_unlink(&ptreaty->clauses, pclause);
      free(pclause);

      ptreaty->accept0=0;
      ptreaty->accept1=0;

      return 1;
    }
  }

  return 0;
}



/****************************************************************
...
*****************************************************************/
int add_clause(struct Treaty *ptreaty, struct player *pfrom, 
		enum clause_type type, int val)
{
  struct Clause *pclause;
  struct genlist_iterator myiter;
  
  genlist_iterator_init(&myiter, &ptreaty->clauses, 0);
  
  for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
    struct Clause *pclause=(struct Clause *)ITERATOR_PTR(myiter);
    if(pclause->type==CLAUSE_GOLD && pclause->from==pfrom) {
      pclause->value=val;
      return 1;
    }
    else if(pclause->type==type && pclause->from==pfrom &&
	    pclause->value==val)
      return 0;
  }
   
  pclause=malloc(sizeof(struct Clause));

  pclause->type=type;
  pclause->from=pfrom;
  pclause->value=val;
  
  genlist_insert(&ptreaty->clauses, pclause, -1);

  ptreaty->accept0=0;
  ptreaty->accept1=0;

  return 1;
}
