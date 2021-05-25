/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "video/gfx.h"
#include "common/utils.h"
#include <stdio.h>

extern const u8 console_font_8x16[];

gfx_screen_t
gfx_tv =
{
    .fb = (u32*)(0x14000000 + 0x3500000),
    .width = TV_WIDTH,
    .height = TV_HEIGHT,
    .stride = 1280,
    .bpp = 4,
},
gfx_drc =
{
    .fb = (u32*)(0x14000000 + 0x38C0000),
    .width = DRC_WIDTH,
    .height = DRC_HEIGHT,
    .stride = 896,
    .bpp = 4,
};

void gfx_draw_hline(gfx_screen_t* screen, u32 x, u32 y, u32 len, u32 color)
{
    u32* cur = screen->fb + y * screen->stride + x;
    memset32(cur, color, len * screen->bpp);
}

void gfx_draw_fill_rect(gfx_screen_t* screen, u32 x, u32 y, u32 w, u32 h, u32 color)
{
    u32* cur = screen->fb + y * screen->stride + x;
    for (u32 i = 0; i < h; i++) {
        memset32(cur, color, w * screen->bpp);
        cur += screen->stride;
    }
}

void gfx_draw_plot(gfx_screen_t* screen, int x, int y, u32 color)
{
    screen->fb[x + y * screen->stride] = color;
}

void gfx_clear(gfx_screen_t* screen, u32 color)
{
    memset32(screen->fb, color, screen->stride * screen->height * screen->bpp);
}

void gfx_draw_char(gfx_screen_t* screen, char c, int x, int y, u32 fg_color, u32 bg_color)
{
    if(c < 32) return;
    c -= 32;

    const u8* charData = &console_font_8x16[(CHAR_SIZE_X * CHAR_SIZE_Y * c) / 8];
    u32* fb = &screen->fb[x + y * screen->stride];

    for(int i = 0; i < CHAR_SIZE_Y; ++i)
    {
        u8 v = *(charData++);

        for(int j = 0; j < CHAR_SIZE_X; ++j)
        {
            if(v & (128 >> j)) *fb = fg_color;
            else *fb = bg_color;
            fb++;
        }

        fb += screen->stride - CHAR_SIZE_X;
    }
}

void gfx_init(void)
{
    gfx_clear(GFX_TV, BLACK);
    gfx_clear(GFX_DRC, BLACK);
}
