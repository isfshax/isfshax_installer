/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _GFX_H
#define _GFX_H

#include "common/types.h"

#define TV_WIDTH    (1280)
#define TV_HEIGHT   (720)

#define DRC_WIDTH   (854)
#define DRC_HEIGHT  (480)

#define CHAR_SIZE_X (8)
#define CHAR_SIZE_Y (16)

#define BLACK   (0x000000ff)
#define RED     (0xff0000ff)
#define GREEN   (0x00ff00ff)
#define YELLOW  (0xffff00ff)
#define BLUE    (0x0000ffff)
#define MAGENTA (0xff00ffff) 
#define CYAN    (0x00ffffff) 
#define WHITE   (0xffffffff)

typedef struct {
	u32* fb;
	int width;
	int height;
    int stride;
	size_t bpp;
} gfx_screen_t;

extern gfx_screen_t gfx_tv, gfx_drc;
#define GFX_TV  (&gfx_tv)
#define GFX_DRC (&gfx_drc)

void gfx_draw_hline(gfx_screen_t* screen, u32 x, u32 y, u32 len, u32 color);
void gfx_draw_plot(gfx_screen_t* screen, int x, int y, u32 color);
void gfx_draw_fill_rect(gfx_screen_t* screen, u32 x, u32 y, u32 w, u32 h, u32 color);
void gfx_clear(gfx_screen_t* screen, u32 color);
void gfx_draw_char(gfx_screen_t* screen, char c, int x, int y, u32 fg_color, u32 bg_color);
void gfx_draw_string(gfx_screen_t* screen, char* str, int x, int y, u32 color);
void gfx_init(void);

#endif
