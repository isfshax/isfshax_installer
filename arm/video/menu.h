/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *  Copyright (C) 2016          SALT
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _MENU_H
#define _MENU_H

#include "common/types.h"
#include "video/console.h"

#define MAX_LINES 128

typedef struct menu_s menu_t;
typedef struct menu_item_s menu_item_t;

struct menu_item_s
{
    char* text;
    void(* callback)(menu_t *menu);
    int active;
};

struct menu_s
{
    char* title;
    char* subtitle[MAX_LINES - 1];
    int subtitles;
    menu_item_t option[MAX_LINES - 1];
    int entries;
    int selected;
    int state;
};

void menu_draw(menu_t *menu);
void menu_next_selection(menu_t *menu);
void menu_select(menu_t *menu);
void menu_init(menu_t *menu);
void menu_close(menu_t *menu);

#endif
