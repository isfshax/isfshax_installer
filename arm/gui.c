#include <stdlib.h>
#include <malloc.h>
#include "video/gfx.h"
#include "system/exception.h"
#include "system/memory.h"
#include "system/irq.h"
#include "storage/sd/sdcard.h"
#include "storage/sd/fatfs/elm.h"
#include "storage/nand/nand.h"
#include "crypto/crypto.h"
#include "system/smc.h"
#include "common/utils.h"
#include "gui.h"
#include "installer.h"
#include "video/menu.h"
#include <stdio.h>

static void main_install(menu_t *menu);
static void main_uninstall(menu_t *menu);
static void main_credits(menu_t *menu);

static int ask_confirmation(void);
static void wait_continue(void);

static menu_t m_main = {
    .title = "\e[2;0H\e[0J\e[33mMain menu\e[0m",
    .option = {
        {"Install isfshax", &main_install, 0},
        {"Uninstall isfshax", &main_uninstall, 0},
        {},
        {"Power off", &menu_close, 1},
        {},
        {"Credits", &main_credits, 1},
    },
    .entries = 6,
};

void gui_main() {
    int status = 0;
    puts("\e[H\e[J\e[36misfshax\e[0m installer");
    puts("\b\e[19D(c) 2021 rw-r-r-0644\n");

    /* disclaimer */
    puts(
        "THIS SOFTWARE COMES WITH ABSOLUTELY NO WARRANTY! YOU ARE\n"
        "CHOOSING TO INSTALL THIS SOFTWARE, AT YOUR OWN RISK.\n"
        "THE AUTHOR(S) OF THIS SOFTWARE WILL NOT BE HELD LIABLE\n"
        "FOR ANY DAMAGE IT MIGHT CAUSE.\n\n"
        "THIS SOFTWARE IS AVAILABLE FOR FREE UNDER THE TERMS OF THE\n"
        "GNU GPLv2 LICENSE. IF YOU'VE PAID FOR THIS SOFTWARE, YOU\n"
        "HAVE BEEN SCAMMED AND SHOULD ASK FOR YOUR MONEY BACK.\n"
    );
    wait_continue();

    /* check isfshax compatibility */
    puts("\e[2;0H\e[0J\e[33mCompatibility check\e[0m");
    status = installer_check_compatibility();
    wait_continue();

    /* update main menu accordingly */
    m_main.option[0].active = (status & ISFSHAX_INSTALL_POSSIBLE) != 0;
    m_main.option[1].active = (status & ISFSHAX_REMOVAL_POSSIBLE) != 0;

    /* enter main menu */
    menu_init(&m_main);
}

static void main_install(menu_t *menu) {
    int rc;

    puts("\e[2;0H\e[0JInstall isfshax now?");
    if (!ask_confirmation()) return;

    puts("\e[2;0H\e[0JInstalling isfshax...");
    rc = install_isfshax();

    if (rc >= 0) {
        m_main.option[1].active = 1;
        m_main.selected = 3;
    }

    wait_continue();
}

static void main_uninstall(menu_t *menu) {
    int rc;

    puts("\e[2;0H\e[0J" "WANRING: Before Uninstalling ISFShax make sure the console boots correctly using\n"
        "the 'Patch ISFShax and boot IOS (slc)' option in minute\n"
        "If your console can't boot coorectly, uninstalling ISFShax will BRICK the console!!!\n\n"
        "Uninstall isfshax now?");
    if (!ask_confirmation()) return;

    puts("\e[2;0H\e[0J" "Uninstalling isfshax...");
    rc = uninstall_isfshax();

    if (rc >= 0) {
        m_main.option[1].active = 0;
        m_main.selected = 3;
    }

    wait_continue();
}

static void main_credits(menu_t *menu) {
    puts(
        "\e[2;0H\e[0JThanks to:\n\n"
        "rw-r-r-0644,\t\tMaschell,\t\tQuarkTheAwesome,\n"
        "GaryOderNichts,\t\texjam,\t\t\tvgmoose,\n"
        "CompuCat,\t\t\thexkyz,\t\t\tderrek,\n"
        "plutoo,\t\t\t\tnaehrwert,\t\thykem,\n"
        "yellows8,\t\t\tsmealum,\t\tSalt team,\n"
        "fail0verflow,\t\tForTheUsers,\tlibwiiu team,\n"
        "and many others!\n"
    );

    wait_continue();
}


static int ask_confirmation(void) {
    static menu_t m_confirm = {
        .option = {
            {"Yes, proceed", &menu_close, 1},
            {"No, go back", &menu_close, 1},
        },
        .entries = 2,
    };
    m_confirm.selected = 1;
    menu_init(&m_confirm);
    return m_confirm.selected == 0;
}


static void wait_continue(void) {
    static menu_t m_continue = {
        .option = {
            {"Continue", &menu_close, 1}
        },
        .entries = 1,
    };
    menu_init(&m_continue);
}
