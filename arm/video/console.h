/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */
#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "common/types.h"
#include "video/gfx.h"

typedef struct {
    gfx_screen_t* screen;
    int enable;
    u32 x_offset;
    u32 y_offset;
    u32 row_count;
    u32 col_count;
    u32 row;
    u32 col;
    u32 row_sv;
    u32 col_sv;
#define DEFAULT_FG_COLOR    (WHITE)
    u32 fg_color;
#define DEFAULT_BG_COLOR    (BLACK)
    u32 bg_color;
#define E_CHKBRACKET        (1)
#define E_INCODE            (2)
#define E_NUMPARSING        (4)
    int e_state;
    int e_argc;
#define E_NMAX              (5)
    u32 e_argv[E_NMAX];
} console_t;

extern console_t console_drc, console_tv;
#define CONSOLE_DRC (&console_drc)
#define CONSOLE_TV  (&console_tv)


#define CONSOLE_CLEAR   "\e[2J\e[H"

#define CONSOLE_RESET   "\e[0m"
#define CONSOLE_BLACK   "\e[30m"
#define CONSOLE_RED     "\e[31m"
#define CONSOLE_GREEN   "\e[32m"
#define CONSOLE_YELLOW  "\e[33m"
#define CONSOLE_BLUE    "\e[34m"
#define CONSOLE_MAGENTA "\e[35m"
#define CONSOLE_CYAN    "\e[36m"
#define CONSOLE_WHITE   "\e[37m"

void console_print(console_t *con, const char *s);
void console_set_area(console_t *con, u32 x0, u32 y0, u32 x1, u32 y1);
void console_init(console_t *con);

#endif /* _CONSOLE_H */
