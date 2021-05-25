/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _ANCAST_H
#define _ANCAST_H

#include "common/types.h"

u32 ancast_iop_load(const char* path);
u32 ancast_ppc_load(const char* path);

#endif
