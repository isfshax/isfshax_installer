/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644 <rwrr0644@gmail.com>
 *  Copyright (C) 2017          Ash Logan <quarktheawesome@gmail.com>
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Haxx Enterprises <bushing@gmail.com>
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *  Copyright (C) 2009          Andre Heider "dhewg" <dhewg@wiibrew.org>
 *  Copyright (C) 2009          John Kelley <wiidev@kelley.ca>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <video/gpu.h>
#include "video/gfx.h"
#include "video/console.h"
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

void NORETURN _main(void* base) {
    gpu_display_init();
    gfx_init();
    console_set_area(CONSOLE_DRC, 16, 16, DRC_WIDTH - 16, DRC_HEIGHT - 16);
    console_set_area(CONSOLE_TV, 16, 16, TV_WIDTH - 16, TV_HEIGHT - 16);
    console_init(CONSOLE_DRC);
    console_init(CONSOLE_TV);
    exception_initialize();
    mem_initialize();
    irq_initialize();
    crypto_initialize();
    nand_initialize();
    sdcard_init();
    int res = ELM_Mount();
    if (res) {
        printf("SD Card mount error: %d\n", res);
        panic(0);
    }
    smc_get_events();
    smc_set_odd_power(false);

    gui_main();

    ELM_Unmount();
    sdcard_exit();
    irq_disable(IRQ_SD0);
    nand_deinitialize();
    irq_shutdown();
    mem_shutdown();
    smc_shutdown(false);
}


