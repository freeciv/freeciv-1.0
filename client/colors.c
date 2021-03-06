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
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>


#include "log.h"
#include "colors.h"

Colormap cmap;
extern int screen_number;
extern Display *display;
extern Widget toplevel;
extern Widget map_canvas;



/* This is just so we can print the visual class intelligibly */
/*static char *visual_class[] = {
   "StaticGray",
   "GrayScale",
   "StaticColor",
   "PseudoColor",
   "TrueColor",
   "DirectColor"
};
*/
struct rgbtriple {
  int r, g, b;
} colors_standard_rgb[COLOR_STD_LAST] = {
  {  0,   0,   0},
  {255, 255, 255},
  {255,   0,   0}, 
  {100, 100,   0},
  {  0, 255, 200},
  {  0, 200,   0},
  {  0,   0, 200},
  {128,   0,   0},  /* races */
  {128, 255, 255},  /* races */
  {255,   0,   0},  /* races */
  {255,   0, 128},  /* races */
  {  0,   0, 128},  /* races */
  {255,   0, 255},  /* races */
  {255, 128,   0},  /* races */
  {255, 255, 128},  /* races */
  {255, 128, 128},  /* races */
  {  0,   0, 255},  /* races */
  {  0, 255,   0},  /* races */
  {  0, 128, 128},  /* races */
  {128,  64,  64},  /* races */
  {192, 192, 192},  /* races */
};

unsigned long colors_standard[COLOR_STD_LAST];


/*************************************************************
...
*************************************************************/
void color_error(void)
{
  static int using_private_cmap;
  
  if(using_private_cmap) {
    log(LOG_FATAL, "Private colormap ran out of entries -  exiting");
    exit(1);
  }
  else {
    log(LOG_NORMAL, "Ran out of colors -  trying with a private colormap"); 
    cmap=XCopyColormapAndFree(display, cmap);
    XtVaSetValues(toplevel, XtNcolormap, cmap, NULL);
    using_private_cmap=1;
  }
  
}


/*************************************************************
...
*************************************************************/
void alloc_standard_colors(void)
{
  int i;

  for(i=0; i<COLOR_STD_LAST; i++) {
    XColor mycolor;

    mycolor.red=colors_standard_rgb[i].r<<8;
    mycolor.green=colors_standard_rgb[i].g<<8;
    mycolor.blue=colors_standard_rgb[i].b<<8;
    
    if(XAllocColor(display, cmap, &mycolor))
      colors_standard[i]=mycolor.pixel;
    else
      color_error();
  
  }
  
}

/*************************************************************
...
*************************************************************/
enum Display_color_type get_visual(void)
{
  int i, default_depth;
  Visual *default_visual;
  XVisualInfo visual_info; 

  /* Try to allocate colors for PseudoColor, TrueColor,
   * DirectColor, and StaticColor; use black and white
   * for StaticGray and GrayScale */
  default_depth = DefaultDepth(display, screen_number);
  default_visual = DefaultVisual(display, screen_number);

  if (default_depth == 1) { 
    /* Must be StaticGray, use black and white */
    log(LOG_DEBUG, "found B/W display.");
    return BW_DISPLAY;
  }

  i=5;
  
  while(!XMatchVisualInfo(display, screen_number, 
			  default_depth, i--, &visual_info));
  

/*
  log(LOG_DEBUG, "Found a %s class visual at default depth.",
      visual_class[++i]); */
   
  if(i < StaticColor) { /* Color visual classes are 2 to 5 */
    /* No color visual available at default depth;
     * some applications might call XMatchVisualInfo
     * here to try for a GrayScale visual if they
     * can use gray to advantage, before giving up
     * and using black and white */
    log(LOG_DEBUG, "found grayscale(?) display.");
    return GRAYSCALE_DISPLAY;
  }

   /* Otherwise, got a color visual at default depth */
   /* The visual we found is not necessarily the default
    * visual, and therefore it is not necessarily the one
    * we used to create our window; however, we now know
    * for sure that color is supported, so the following
    * code will work (or fail in a controlled way) */
   /* Let's check just out of curiosity: */
  if (visual_info.visual != default_visual) {
/*    log(LOG_DEBUG, "Found: %s class visual at default depth",
	visual_class[i]); */
  }

  log(LOG_DEBUG, "color system booted ok.");

  return COLOR_DISPLAY;
}



/*************************************************************
...
*************************************************************/
void init_color_system(void)
{
  cmap=DefaultColormap(display, screen_number);

  alloc_standard_colors();
}

/*************************************************************
...
*************************************************************/
void free_colors(unsigned long *pixels, int ncols)
{
  XFreeColors(display, cmap, pixels, ncols, 0);
}


