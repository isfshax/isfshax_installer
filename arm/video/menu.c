/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *  Copyright (C) 2016          SALT
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "menu.h"

#include "common/types.h"
#include "common/utils.h"
#include <stdio.h>

#include "system/smc.h"

#define MENU_REDRAW     (0)
#define MENU_VISIBLE    (1)
#define MENU_QUIT       (-1)

void menu_draw(menu_t *menu)
{
    int i = 0;

    if(menu->state == MENU_REDRAW)
    {
        if (menu->title) {
            puts(menu->title);
            puts("");
        }

        for (i = 0; i < menu->subtitles; i++)
            puts(menu->subtitle[i]);
        puts("");

        menu->state = MENU_VISIBLE;
    }
    else 
        printf("\e[%dA", menu->entries);

    for(i = 0; i < menu->entries; i++) {
        if(!menu->option[i].active) {
            puts("");
            continue;
        }

        if(i == menu->selected)
            printf("> \e[7m%s\e[0m\n", menu->option[i].text);
        else
            printf("  %s\n", menu->option[i].text);
    }
}

void menu_next_selection(menu_t *menu)
{
    int i, j = menu->selected;
    for(i = 1; i < menu->entries; i++) {
        if(++j >= menu->entries) j -= menu->entries;
        if(menu->option[j].active) break;
    }
    menu->selected = j;
}

void menu_select(menu_t *menu)
{
    if(menu->option[menu->selected].callback)
    {
        menu->state = MENU_REDRAW;
        menu->option[menu->selected].callback(menu);
    }
}

void menu_init(menu_t *menu)
{
    menu->state = MENU_REDRAW;
    while (menu->state != MENU_QUIT)
    {
        menu_draw(menu);

        u8 input = smc_get_events();

        if(input & SMC_EJECT_BUTTON) menu_select(menu);
        if(input & SMC_POWER_BUTTON) menu_next_selection(menu);
    }
}

void menu_close(menu_t *menu)
{
    menu->state = MENU_QUIT;
}
