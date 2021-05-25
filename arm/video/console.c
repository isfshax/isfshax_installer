/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */
#include "video/console.h"
#include "video/gfx.h"
#include "common/utils.h"
#include "common/types.h"
#include <string.h>
#include <sys/iosupport.h>

console_t
console_drc = {
    .screen = GFX_DRC,
    .row_count = DRC_HEIGHT / CHAR_SIZE_Y,
    .col_count = DRC_WIDTH / CHAR_SIZE_X,
    .bg_color = DEFAULT_BG_COLOR,
    .fg_color = DEFAULT_FG_COLOR,
},
console_tv = {
    .screen = GFX_TV,
    .row_count = TV_HEIGHT / CHAR_SIZE_Y,
    .col_count = TV_WIDTH / CHAR_SIZE_X,
    .bg_color = DEFAULT_BG_COLOR,
    .fg_color = DEFAULT_FG_COLOR,
};

static const u32 console_palette[8] = {
    BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
};

static void console_write_char(console_t *con, int c) {
    if ((con->row >= con->row_count) || (con->col >= con->col_count)) return;
    u32 y = con->y_offset + con->row * CHAR_SIZE_Y;
    u32 x = con->x_offset + con->col * CHAR_SIZE_X;
    gfx_draw_char(con->screen, c, x, y, con->fg_color, con->bg_color);
}

static void console_clear_rect(console_t *con, u32 s_row, u32 s_col, u32 e_row, u32 e_col) {
    if ((e_col < s_col) || (e_col > con->col_count) ||
        (e_row < s_row) || (e_row > con->row_count))
        return;
    u32 y = con->y_offset + s_row * CHAR_SIZE_Y;
    u32 x = con->x_offset + s_col * CHAR_SIZE_X;
    u32 h = (e_row - s_row) * CHAR_SIZE_Y;
    u32 w = (e_col - s_col) * CHAR_SIZE_X;
    gfx_draw_fill_rect(con->screen, x, y, w, h, con->bg_color);
}

static void console_process_escape(console_t *con, int c)
{
    int i;

    if (con->e_state == E_CHKBRACKET) {
        con->e_state = (c == '[') ? E_INCODE : 0;
        return;
    }

    if (c >= '0' && c <= '9') {
        if (con->e_argc >= E_NMAX) return;
        con->e_state |= E_NUMPARSING;
        con->e_argv[con->e_argc] *= 10;
        con->e_argv[con->e_argc] += c - '0';
        return;
    } else if (con->e_state & E_NUMPARSING) {
        con->e_state &= ~E_NUMPARSING;
        con->e_argc++;
    }

    switch (c) {
        case ';':
            return;
        case 'H':
        case 'f':
            if (con->e_argv[0] < con->row_count)
                con->row = con->e_argv[0];
            if (con->e_argv[1] < con->col_count)
                con->col = con->e_argv[1];
            break;
        case 'F':
            con->col = 0;
            /* fallthrough */
        case 'A':
            if (con->row > con->e_argv[0])
                con->row -= con->e_argv[0];
            else
                con->row = 0;
            break;
        case 'E':
            con->col = 0;
            /* fallthrough */
        case 'B':
            con->row += con->e_argv[0];
            if (con->row >= con->row_count)
                con->row = con->row_count - 1;
            break;
        case 'C': 
            con->col += con->e_argv[0];
            if (con->col >= con->col_count)
                con->col = con->col_count - 1;
            break;
        case 'D':
            if (con->col > con->e_argv[0])
                con->col -= con->e_argv[0];
            else
                con->col = 0;
            break;
        case 'G':
            if (con->e_argv[0] < con->col_count)
                con->col = con->e_argv[0];
        case 'J':
            if ((con->e_argc == 0) || (con->e_argv[0] == 2)) {
                console_clear_rect(con, 0, 0, con->row_count, con->col_count);
            } else if (con->e_argv[0] == 1) {
                console_clear_rect(con, 0, 0, con->row, con->col_count);
                console_clear_rect(con, con->row, 0, con->row+1, con->col);
            } else if (con->e_argv[0] == 0) {
                console_clear_rect(con, con->row, con->col, con->row+1, con->col_count);
                console_clear_rect(con, con->row+1, 0, con->row_count, con->col_count);
            }
            break;
        case 'K':
            if ((con->e_argc == 0) || (con->e_argv[0] == 2)) {
                console_clear_rect(con, con->row, 0, con->row+1, con->col_count);
            } else if (con->e_argv[0] == 1) {
                console_clear_rect(con, con->row, 0, con->row+1, con->col);
            } else if (con->e_argv[0] == 0) {
                console_clear_rect(con, con->row, con->col, con->row+1, con->col_count);
            }
            break;
        case 'm':
            for (i = 0; i < con->e_argc; i++) {
                u32 mode = con->e_argv[i];
                if (mode == 0) {
                    con->bg_color = DEFAULT_BG_COLOR;
                    con->fg_color = DEFAULT_FG_COLOR;
                } else if (mode == 7) {
                    u32 t = con->bg_color;
                    con->bg_color = con->fg_color;
                    con->fg_color = t;
                } else if (mode == 8) {
                    con->fg_color = con->bg_color;
                } else if ((mode >= 30) && (mode <= 37)) {
                    con->fg_color = console_palette[mode - 30];
                } else if ((mode >= 40) && (mode <= 47)) {
                    con->bg_color = console_palette[mode - 40];
                }
            }
            break;
        case 's':
            con->row_sv = con->row;
            con->col_sv = con->col;
            break;
        case 'u':
            con->row = con->row_sv;
            con->col = con->col_sv;
            break;
        default:
            break;
    }

    con->e_state = 0;
    con->e_argc = 0;
    memset32(con->e_argv, 0, sizeof(con->e_argv));
}

static void console_print_char(console_t *con, int c)
{
    if (con->e_state) {
        console_process_escape(con, c);
        return;
    }

    switch(c) {
        case '\e':
            con->e_state = 1;
            break;
        case '\n':
            con->row++;
            /* fallthrough */
        case '\r':
            con->col = 0;
            break;
        case '\b':
            if (con->col > 0) {
                con->col--;
            } else if (con->row > 0) {
                con->row--;
                con->col = con->col_count - 1;
            }
            console_write_char(con, ' ');
            break;
        case '\t':
            con->col = (con->col & ~3) + 4;
            break;
        default:
            console_write_char(con, c);
            con->col++;
            break;
            
    }

    if (con->col >= con->col_count) {
        con->col = 0;
        con->row++;
    }
    if (con->row >= con->row_count) {
        con->row = 0;
        console_clear_rect(con, 0, 0, con->row_count, con->col_count);
    }
}

void console_print(console_t *con, const char *s)
{
    size_t i;
    for(i = 0; s[i]; i++)
        console_print_char(con, s[i]);
}

static ssize_t console_write(struct _reent *r, void *fd, const char *ptr, size_t len)
{
    size_t i;
    for(i = 0; i < len; i++) {
        if (console_tv.enable)
            console_print_char(&console_tv, ptr[i]);
        if (console_drc.enable)
            console_print_char(&console_drc, ptr[i]);
    }
    return i;
}

static devoptab_t console_dotab =
{
    .name = "console",
    .write_r = &console_write,
};

void console_set_area(console_t *con, u32 x0, u32 y0, u32 x1, u32 y1)
{
    if ((x1 < x0) || (y1 < y0)) return;
    con->x_offset = x0;
    con->y_offset = y0;
    con->col_count = (x1 - x0) / CHAR_SIZE_X;
    con->row_count = (y1 - y0) / CHAR_SIZE_Y;
}

void console_init(console_t *con)
{
    console_clear_rect(con, 0, 0, con->row_count, 0);
    devoptab_list[STD_OUT] = &console_dotab;
    devoptab_list[STD_ERR] = &console_dotab;
    con->enable = 1;
    con->row = 0;
    con->col = 0;
    con->bg_color = DEFAULT_BG_COLOR;
    con->fg_color = DEFAULT_FG_COLOR;
}
