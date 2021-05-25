/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *  Copyright (C) 2009          Andre Heider "dhewg" <dhewg@wiibrew.org>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "common/types.h"
#include "ppc.h"
#include "common/utils.h"
#include "video/gfx.h"
#include <stdio.h>
#include <sys/errno.h>
#include "elf.h"
#include "memory.h"
#include <string.h>

#define PHDR_MAX 10

static int _check_physaddr(u32 addr) {
    if((addr >= 0xFFE00000) && (addr <= 0xFFF1FFFF))
        return 0;

    if((addr <= 0x01FFFFFF))
        return 1;

    if((addr >= 0x14000000) && (addr <= 0x1CFFFFFF))
        return 2;

    if((addr >= 0x28000000) && (addr <= 0xCFFFFFFF))
        return 3;

    return -1;
}

static u32 _translate_physaddr(u32 addr) {
    if((addr >= 0xFFE00000) && (addr <= 0xFFF1FFFF))
        return (addr - 0xFFE00000) + 0x08000000;

    return addr;
}

static int _check_physrange(u32 addr, u32 len) {
    switch (_check_physaddr(addr)) {
        case 0:
            if ((addr + len) <= 0xFFF1FFFF)
                return 0;
            break;
        case 1:
            if ((addr + len) <= 0x01FFFFFF)
                return 1;
            break;
        case 2:
            if ((addr + len) <= 0x1CFFFFFF)
                return 2;
            break;
        case 3:
            if ((addr + len) <= 0xCFFFFFFF)
                return 3;
            break;
    }

    return -1;
}

static Elf32_Ehdr elfhdr;
static Elf32_Phdr phdrs[PHDR_MAX];

int ppc_load_file(const char *path, u32* entry)
{
    int res = 0, read = 0;

    FILE* file = fopen(path, "rb");
    if(!file) return -errno;

    read = fread(&elfhdr, sizeof(elfhdr), 1, file);
    if(read != 1)
        return -100;

    if (memcmp("\x7F" "ELF\x01\x02\x01\x00\x00", elfhdr.e_ident, 9)) {
        printf("ELF: invalid ELF header! 0x%02x 0x%02x 0x%02x 0x%02x\n",
                elfhdr.e_ident[0], elfhdr.e_ident[1],
                        elfhdr.e_ident[2], elfhdr.e_ident[3]);
        return -101;
    }

    if (_check_physaddr(elfhdr.e_entry) < 0) {
        printf("ELF: invalid entry point! 0x%08lX\n", elfhdr.e_entry);
        return -102;
    }

    if (elfhdr.e_phoff == 0 || elfhdr.e_phnum == 0) {
        printf("ELF: no program headers!\n");
        return -103;
    }

    if (elfhdr.e_phnum > PHDR_MAX) {
        printf("ELF: too many (%d) program headers!\n", elfhdr.e_phnum);
        return -104;
    }

    res = fseek(file, elfhdr.e_phoff, SEEK_SET);
    if (res) return -res;

    read = fread(phdrs, sizeof(phdrs[0]), elfhdr.e_phnum, file);
    if(read != elfhdr.e_phnum)
        return -errno;

    Elf32_Phdr *phdr = phdrs;
    u16 count = read;

    ppc_prepare();

    while (count--) {
        if (phdr->p_type != PT_LOAD) {
            printf("ELF: skipping PHDR of type %ld\n", phdr->p_type);
        } else {
            if (_check_physrange(phdr->p_paddr, phdr->p_memsz) < 0) {
                printf("ELF: PHDR out of bounds [0x%08lX...0x%08lX]\n",
                                phdr->p_paddr, phdr->p_paddr + phdr->p_memsz);
                return -106;
            }

            void *dst = (void *) _translate_physaddr(phdr->p_paddr);

            printf("ELF: LOAD 0x%lX @0x%08lX [0x%lX]\n", phdr->p_offset, phdr->p_paddr, phdr->p_filesz);
            if(phdr->p_filesz != 0) {
                res = fseek(file, phdr->p_offset, SEEK_SET);
                if (res) return -res;
                count = fread(dst, phdr->p_filesz, 1, file);
                if(count != 1) return -errno;
            }
        }
        phdr++;
    }

    dc_flushall();

    printf("ELF: load done.\n");
    *entry = elfhdr.e_entry;

    return 0;
}

int ppc_load_mem(const u8 *addr, u32 len, u32* entry)
{
    if (len < sizeof(Elf32_Ehdr))
        return -100;

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) addr;

    if (memcmp("\x7F" "ELF\x01\x02\x01\x00\x00", ehdr->e_ident, 9)) {
        printf("ELF: invalid ELF header! 0x%02x 0x%02x 0x%02x 0x%02x\n",
                        ehdr->e_ident[0], ehdr->e_ident[1],
                        ehdr->e_ident[2], ehdr->e_ident[3]);
        return -101;
    }

    if (_check_physaddr(ehdr->e_entry) < 0) {
        printf("ELF: invalid entry point! 0x%08lX\n", ehdr->e_entry);
        return -102;
    }

    if (ehdr->e_phoff == 0 || ehdr->e_phnum == 0) {
        printf("ELF: no program headers!\n");
        return -103;
    }

    if (ehdr->e_phnum > PHDR_MAX) {
        printf("ELF: too many (%d) program headers!\n",
                        ehdr->e_phnum);
        return -104;
    }

    u16 count = ehdr->e_phnum;
    if (len < ehdr->e_phoff + count * sizeof(Elf32_Phdr))
        return -105;

    Elf32_Phdr *phdr = (Elf32_Phdr *) &addr[ehdr->e_phoff];

    // TODO: add more checks here
    // - loaded ELF overwrites itself?

    ppc_prepare();

    while (count--) {
        if (phdr->p_type != PT_LOAD) {
            printf("ELF: skipping PHDR of type %ld\n", phdr->p_type);
        } else {
            if (_check_physrange(phdr->p_paddr, phdr->p_memsz) < 0) {
                printf("ELF: PHDR out of bounds [0x%08lX...0x%08lX]\n",
                                phdr->p_paddr, phdr->p_paddr + phdr->p_memsz);
                return -106;
            }

            printf("ELF: LOAD 0x%lX @0x%08lX [0x%lX]\n", phdr->p_offset, phdr->p_paddr, phdr->p_filesz);

            void *dst = (void *) _translate_physaddr(phdr->p_paddr);
            memcpy(dst, &addr[phdr->p_offset], phdr->p_filesz);
        }
        phdr++;
    }

    dc_flushall();

    printf("ELF: load done.\n");
    *entry = ehdr->e_entry;

    return 0;
}
