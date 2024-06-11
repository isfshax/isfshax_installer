/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644 <rwrr0644@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <string.h>
#include <stdlib.h>
#include "common/types.h"
#include "common/utils.h"
#include "system/smc.h"
#include "system/latte.h"
#include "storage/sd/fatfs/ff.h"
#include "storage/nand/nand.h"
#include "storage/nand/isfs/isfs.h"
#include "storage/nand/isfs/super.h"
#include "storage/nand/isfs/volume.h"
#include "storage/nand/isfs/isfshax.h"
#include "crypto/crypto.h"
#include "crypto/sha.h"
#include "video/console.h"
#include "installer.h"
#include "boot1.h"

static int _load_isfshax_superblock(isfshax_super *s_isfshax);
static int _load_file_to_mem(const char *path, void *buf, u32 size);


void pr_error(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    fputs(CONSOLE_RED "ERROR: ", stdout);
    vprintf(fmt, va);
    fputs(CONSOLE_RESET, stdout);
    va_end(va);
}

int installer_check_compatibility(void)
{
    isfs_ctx *slc = isfs_get_volume(ISFSVOL_SLC);
    int status = ISFSHAX_INSTALL_POSSIBLE | ISFSHAX_REMOVAL_POSSIBLE;

    /* ensure this is a normal, retail console */
    fputs("\nConsole Type:        ", stdout);
    u32 asicrev = read32(LT_ASICREV_CCR);
    if ((seeprom.bc.board_type == 0x4346) ||
        (seeprom.bc.console_type == 1) ||
        ((asicrev >> 16) == 0xcafe) ||
        ((asicrev & 0xff) >= 0x30)) {
        puts(CONSOLE_GREEN "Retail" CONSOLE_RESET);
    } else {
        status = 0;
        puts(CONSOLE_RED "Unsupported" CONSOLE_RESET);
    }

    /* ensure boot1 version is v8377 */
    fputs("\nboot1 version:       ", stdout);
    int boot1ver = boot1_get_version();
    if (boot1ver == 8377) {
        puts(CONSOLE_GREEN "8377" CONSOLE_RESET);
    } else {
        status &= ~ISFSHAX_INSTALL_POSSIBLE;
        printf(CONSOLE_RED "Unsupported (%d)\n" CONSOLE_RESET, boot1ver);
    }

    /* check if isfshax is already installed to allow removal */
    fputs("\nisfshax:             ", stdout);
    if ((isfs_load_super(slc, ISFSHAX_GENERATION_FIRST, 0xffffffff) >= 0) &&
        (read32((u32)slc->super + ISFSHAX_INFO_OFFSET) == ISFSHAX_MAGIC)) {
        puts(CONSOLE_GREEN "Is installed" CONSOLE_RESET);
    } else {
        status &= ~ISFSHAX_REMOVAL_POSSIBLE;
        if (status & ISFSHAX_INSTALL_POSSIBLE)
            puts(CONSOLE_GREEN "Can be installed" CONSOLE_RESET);
        else
            puts(CONSOLE_RED "Cannot be installed" CONSOLE_RESET);
    }

    return status;
}

int install_isfshax(void)
{
    static isfshax_super s_isfshax = {0};
    isfs_ctx *slc = isfs_get_volume(ISFSVOL_SLC);
    int good_slots, needed_slots = ISFSHAX_REDUNDANCY, isfshax_okay = 0;
    isfshax_info isfshax;
    int i, index, rc;

    puts("Loading and verifying crafted isfshax superblock");

    rc = _load_isfshax_superblock(&s_isfshax);
    if (rc) {
        pr_error("Failed to load crafted isfshax superblock! (%d)\n", rc);
        return -1;
    }    

    isfshax.magic = ISFSHAX_MAGIC;
    isfshax.generationbase = ISFSHAX_GENERATION_FIRST;

    fputs("Looking for previous isfshax installs... ", stdout);
    if (isfs_load_super(slc, ISFSHAX_GENERATION_FIRST, 0xffffffff) >= 0) {
        puts("Found");

        /* import current isfshax info (allocated slots, generation base, ...) */
        memcpy(&isfshax, slc->super + ISFSHAX_INFO_OFFSET, sizeof(isfshax));

        /* keep good slots, discard no longer usable bad slots */
        for (i = (ISFSHAX_REDUNDANCY - 1); i >= 0; i--)
            if (!(isfshax.slots[i].bad))
                isfshax.slots[--needed_slots] = isfshax.slots[i];
    }
    else puts("Not found");

    printf("%d new slots need to be allocated for isfshax\n", needed_slots);

    rc = isfs_load_super(slc, 0, ISFSHAX_GENERATION_FIRST);
    if (rc < 0) {
        pr_error("Failed to find an unpatched isfs superblock (%d)\n", rc);
        return -2;
    }

    /* allocate the slots needed for isfshax */
    good_slots = 0;
    for (index = (slc->super_count - 1); index >= 0; index--) {
        if (isfs_super_check_slot(slc, index) < 0) continue;

        if (needed_slots > 0) {
            isfs_super_mark_bad_slot(slc, index);
            isfshax.slots[--needed_slots].slot = index;
            printf("Allocated slot %d for isfshax\n", index);
        }
        else good_slots++;
    }
    if (good_slots < 16) {
        pr_error("The nand contains too many bad superblock slots, cannot safely proceed\n");
        return -3;
    }

    /* write two copies of the updated ISFS superblock, just to be sure */
    puts("Writing updated isfs superblock");
    for (i = 0; i < 2; i++) {
        rc = isfs_commit_super(slc);
        if (rc < 0) {
            pr_error("Failed to commit updated superblock (%d)\n", rc);
            return -4;
        }
    }

    /* write the crafted superblock slots to the allocated slots */
    puts("Writing crafted isfshax superblocks");
    isfshax.generation = isfshax.generationbase;
    for (i = 0; i < ISFSHAX_REDUNDANCY; i++) {
        s_isfshax.generation = isfshax.generation;
        isfshax.index = i;
        memcpy(&s_isfshax.isfshax, &isfshax, sizeof(isfshax));

        printf("Writing isfshax superblock to slot %d... ", (int)isfshax.slots[i].slot);
        rc = isfs_write_super(slc, &s_isfshax, isfshax.slots[i].slot);
        if (rc >= 0) {
            puts("OK");
            isfshax_okay++;
        } else {
            puts("Fail");
        }
        isfshax.generation++;
    }
    if (!isfshax_okay) {
        pr_error("Couldn't write to any isfshax slot!\n");
        return -5;
    }

    puts(CONSOLE_GREEN "\nSUCCESS." CONSOLE_RESET);
    return 0;
}

int uninstall_isfshax(void)
{
    isfshax_info isfshax;
    isfs_ctx *slc = isfs_get_volume(ISFSVOL_SLC);
    u16* fat;
    int i, rc;

    /* load isfshax slot allocation from the newest isfshax superblock */
    puts("Loading latest isfshax superblock...\n");
    rc = isfs_load_super(slc, ISFSHAX_GENERATION_FIRST, 0xffffffff);
    if (rc < 0) {
        pr_error("Failed to find isfshax superblock (%d)\n", rc);
        return -1;
    }

    memcpy(&isfshax, slc->super + ISFSHAX_INFO_OFFSET, sizeof(isfshax));
    if (isfshax.magic != ISFSHAX_MAGIC) {
        pr_error("Bad isfshax data magic %08X\n", isfshax.magic);
        return -2;
    }

    /* load normal isfs superblock to free up good isfshas slots */
    rc = isfs_load_super(slc, 0, ISFSHAX_GENERATION_FIRST);
    if (rc < 0) {
        pr_error("Failed to find an unpatched isfs superblock (%d)\n", rc);
        return -3;
    }
    fat = isfs_get_fat(slc);

    for (i = 0; i < ISFSHAX_REDUNDANCY; i++) {
        u32 slot = isfshax.slots[i].slot;
        u32 block = BLOCK_COUNT - (slc->super_count - slot) * ISFSSUPER_BLOCKS;
        u32 cluster = block * BLOCK_CLUSTERS;
        u32 offs = 0;

        /* (attempt to) erase isfshax superblocks */
        printf("Erasing isfshax slot %d (isfs slot %lu, block %lu-%lu)\n", i, slot, block+0, block+1);
        nand_erase_block(block+0);
        nand_erase_block(block+1);

        if (isfshax.slots[i].bad)
            continue;

        /* remove bad slot mark from good slots */
        for (offs = 0; offs < ISFSSUPER_CLUSTERS; offs++)
            fat[cluster + offs] = FAT_CLUSTER_RESERVED;
    }

    /* write two copies of the updated ISFS superblock, just to be sure */
    puts("Writing updated isfs superblock");
    for (i = 0; i < 2; i++) {
        rc = isfs_commit_super(slc);
        if (rc < 0) {
            pr_error("Failed to commit updated superblock (%d)\n", rc);
            return -4;
        }
    }

    puts(CONSOLE_GREEN "\nSUCCESS." CONSOLE_RESET);
    return 0;
}

static int _load_isfshax_superblock(isfshax_super *s_isfshax)
{
    u8 savedhash[SHA_HASH_SIZE], computedhash[SHA_HASH_SIZE];

    puts("Loading superblock.img");
    if (_load_file_to_mem("superblock.img", s_isfshax, sizeof(*s_isfshax))) {
        pr_error("Failed to load superblock.img\n");
        return -1;
    }

    puts("Loading superblock.img.sha");
    if (_load_file_to_mem("superblock.img.sha", savedhash, sizeof(savedhash))) {
        pr_error("Failed to load superblock.img.sha\n");
        return -2;
    }

    puts("Verifying superblock.img checksum");
    sha_hash(s_isfshax, computedhash, sizeof(*s_isfshax));
    if (memcmp(savedhash, computedhash, SHA_HASH_SIZE)) {
        pr_error("Checksum verification failed!\n");
        return -3;
    }

    return 0;
}

static int _load_file_to_mem(const char *path, void *buf, u32 size)
{
    UINT br = 0;
    FIL fil;

    if (f_open(&fil, path, FA_READ))
        return -1;
    if (f_size(&fil) == size)
        f_read(&fil, buf, size, &br);
    f_close(&fil);

    return (size != br) ? -2 : 0;
}
