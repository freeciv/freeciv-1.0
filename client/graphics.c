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
#include <string.h>
#include <ctype.h>
#include <X11/Intrinsic.h>
#include <xpm.h>

#include "log.h"
#include "graphics.h"
#include "colors.h"

extern int display_depth;
extern Widget map_canvas;
extern Display *display;
extern XColor colors[MAX_COLORS];
extern GC civ_gc; 
extern Colormap cmap;
extern Widget toplevel;
extern Window root_window;

struct Sprite **tile_sprites;
struct Sprite *intro_gfx_sprite;
struct Sprite *radar_gfx_sprite;

/* current always 30x30, but with scaling.... */
int tile_sprite_width, tile_sprite_height;

/***************************************************************************
...
***************************************************************************/
struct Sprite *get_tile_sprite(int tileno)
{
  return tile_sprites[tileno];
}

/***************************************************************************
...
***************************************************************************/
char* datafilename(char *filename) {
  static char* datadir=0;
  static char  realfile[512];
  if(!datadir) {
    if((datadir = getenv("FREECIV_DATADIR"))) {
      int i;
      for(i=strlen(datadir)-1; i>=0 && isspace(datadir[i]); i--)
	datadir[i] = '\0';
      if(datadir[i] == '/')
	datadir[i] = '\0';
    } else {
      datadir = "data";		/* correct if not 'data' is the default */
    };
  };
  sprintf(realfile,"%s/%s",datadir,filename);
  return(realfile);
}
		

/***************************************************************************
...
***************************************************************************/
void load_intro_gfx(void)
{
  intro_gfx_sprite=load_xpmfile(datafilename("intro.xpm"));
  radar_gfx_sprite=load_xpmfile(datafilename("radar.xpm"));
}


/***************************************************************************
...
***************************************************************************/
void load_tile_gfx(void)
{
  int i, x, y, ntiles, a;
  struct Sprite *big_sprite;
  big_sprite=load_xpmfile(datafilename("tiles.xpm"));
    
  ntiles=2*(big_sprite->width/30)*(big_sprite->height/30);

  if(!(tile_sprites=malloc(ntiles*sizeof(struct Sprite *)))) {
    log(LOG_FATAL, "couldn't malloc tile_sprites array");
    exit(1);
  }
  
  i=0;
  for(y=0, a=0; a<22 && y<big_sprite->height; a++, y+=30)
    for(x=0; x<big_sprite->width; x+=30) {
      GC plane_gc;
      Pixmap mypixmap, mask;
      
      mypixmap=XCreatePixmap(display, root_window, 30, 30, 
			     display_depth);
      XCopyArea(display, big_sprite->pixmap, mypixmap, civ_gc, 
		x, y, 30, 30, 0 ,0);

      mask=XCreatePixmap(display, root_window, 30, 30, 1);

      plane_gc = XCreateGC(display, mask, 0, NULL);

      XCopyArea(display, big_sprite->mask, mask, plane_gc, 
		x, y, 30, 30, 0 ,0);

      tile_sprites[i++]=ctor_sprite_mask(mypixmap, mask, 30, 30);

      XFreeGC(display, plane_gc);
    }

  for(x=0; x<big_sprite->width; x+=15) {
    Pixmap mypixmap;
    
    mypixmap=XCreatePixmap(display, root_window, 15, 20, 
			   display_depth);
    XCopyArea(display, big_sprite->pixmap, mypixmap, civ_gc, 
	      x, y, 15, 20, 0 ,0);
    
    tile_sprites[i++]=ctor_sprite(mypixmap, 15, 20);
 }
  

  tile_sprite_width=30;
  tile_sprite_height=30;

}




/***************************************************************************
...
***************************************************************************/
struct Sprite *ctor_sprite(Pixmap mypixmap, int width, int height)
{
  struct Sprite *mysprite=malloc(sizeof(struct Sprite));
  mysprite->pixmap=mypixmap;
  mysprite->width=width;
  mysprite->height=height;
  mysprite->has_mask=0;
  return mysprite;
}

/***************************************************************************
...
***************************************************************************/
struct Sprite *ctor_sprite_mask(Pixmap mypixmap, Pixmap mask, 
				int width, int height)
{
  struct Sprite *mysprite=malloc(sizeof(struct Sprite));
  mysprite->pixmap=mypixmap;
  mysprite->mask=mask;

  mysprite->width=width;
  mysprite->height=height;
  mysprite->has_mask=1;
  return mysprite;
}




/***************************************************************************
...
***************************************************************************/
void dtor_sprite(struct Sprite *mysprite)
{
  XFreePixmap(display, mysprite->pixmap);
  if(mysprite->has_mask)
    XFreePixmap(display, mysprite->mask);
  free_colors(mysprite->pcolorarray, mysprite->ncols);
  free(mysprite->pcolorarray);
  free(mysprite);

}



/***************************************************************************
...
***************************************************************************/
struct Sprite *load_xpmfile(char *filename)
{
  struct Sprite *mysprite;
  Pixmap mypixmap, mask_bitmap;
  int err;
  XpmAttributes attributes;
  
  attributes.extensions = NULL;
  attributes.valuemask = XpmCloseness|XpmColormap;
  attributes.colormap = cmap;
  attributes.closeness = 40000;

again:
  
  if((err=XpmReadFileToPixmap(display, root_window, filename, &mypixmap, 
			      &mask_bitmap, &attributes)!=XpmSuccess)) {


    printf("%d\n", err);
    if(err==XpmColorError || err==XpmColorFailed) {
      color_error();
      goto again;
    }
    
    log(LOG_FATAL, "failed reading XPM file: %s", filename);
    exit(1);
  }

  if(!(mysprite=(struct Sprite *)malloc(sizeof(struct Sprite)))) {
    log(LOG_FATAL, "failed mallocing sprite struct for %s", filename);
    exit(1);
  }
  
  mysprite->pixmap=mypixmap;
  mysprite->mask=mask_bitmap;
  mysprite->has_mask=(mask_bitmap!=0);
  mysprite->width=attributes.width;
  mysprite->height=attributes.height;

  return mysprite;
}




