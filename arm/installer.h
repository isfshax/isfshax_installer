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
int installer_check_compatibility(void);

int install_isfshax(void);
int uninstall_isfshax(void);

#endif 
