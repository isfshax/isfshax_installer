/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _INSTALLER_H_
#define _INSTALLER_H_

#define ISFSHAX_INSTALL_POSSIBLE    (1 << 0)
#define ISFSHAX_REMOVAL_POSSIBLE    (1 << 1)
typedef enum {
    SUPERBLOCK_NOT_CHECKED,
    SUPERBLOCK_FROM_SD,
    SUPERBLOCK_FROM_SLC,
    SUPERBLOCK_NOT_FOUND,
    SUPERBLOCK_INVALID_SIZE,
    SUPERBLOCK_INVALID_CHECKSUM
} superblock_state;

extern superblock_state s_superblock_state;

void installer_set_source(int source);
int installer_check_compatibility(void);

int install_isfshax(void);
int uninstall_isfshax(void);

#endif 
