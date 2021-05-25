/*
    elm.h
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ELM_H
#define _ELM_H

#include <stdint.h>
#include <sys/iosupport.h>
#include <sys/types.h>
#include <sys/syslimits.h>

#include "ff.h"
#include "diskio.h"

#ifdef __cplusplus
extern "C" {
#endif

#if _MAX_SS == 512    /* Single sector size */
#define ELM_SS(fs)  512U
#elif _MAX_SS == 1024 || _MAX_SS == 2048 || _MAX_SS == 4096 /* Multiple sector size */
#define ELM_SS(fs)  ((fs)->ssize)
#else
#error Wrong sector size.
#endif

#define ELM_VALID_DISK(disk) (disk < _VOLUMES)

typedef struct _DIR_EX_
{
  FDIR dir;
  TCHAR name[_MAX_LFN+1];
  size_t namesize;
} DIR_EX;

int ELM_Mount(void);
void ELM_Unmount(void);
int ELM_ClusterSizeFromHandle(int fildes, uint32_t* size);
int ELM_SectorsPerClusterFromHandle(int fildes, uint32_t* per);
int ELM_ClusterSizeFromDisk(int disk, uint32_t* size);
int ELM_ClustersFromDisk(int disk, uint32_t* clusters);
int ELM_FreeClustersFromDisk(int disk, uint32_t* clusters);
int ELM_SectorsFromDisk(int disk, uint32_t* sectors);
uint32_t ELM_GetFAT(int fildes, uint32_t cluster, uint32_t* sector);
int ELM_DirEntry(int fildes, uint64_t* entry);
uint32_t ELM_GetSectorCount(unsigned char drive);

int dirnext(DIR_ITER *dirState, char *filename, struct stat *filestat);

#ifdef __cplusplus
}
#endif

#define MAX_FILENAME_LENGTH 768 // 256 UCS-2 characters encoded into UTF-8 can use up to 768 UTF-8 chars

#endif
