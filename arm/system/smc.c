/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "common/types.h"
#include "common/utils.h"
#include "i2c.h"
#include "latte.h"

int smc_read_register(u8 offset, u8* data)
{
    int res = 0;

    // Clock is 10000 in C2W, but 5000 in IOS...
    i2c_init(5000, 1);

    res = i2c_write(I2C_SLAVE_SMC, &offset, 1);
    if(res) return res;

    res = i2c_read(I2C_SLAVE_SMC, data, 1);
    return res;
}

int smc_write_register(u8 offset, u8 data)
{
    // Clock is 10000 in C2W, but 5000 in IOS...
    i2c_init(5000, 1);

    u8 cmd[2] = {offset, data};
    return i2c_write(I2C_SLAVE_SMC, cmd, 2);
}

int smc_write_raw(u8 data)
{
    // Clock is 10000 in C2W, but 5000 in IOS...
    i2c_init(5000, 1);

    return i2c_write(I2C_SLAVE_SMC, &data, 1);
}

int smc_set_odd_power(bool enable)
{
    u8 cmd = 0x01;
    if(enable) cmd = 0x00;

    return smc_write_raw(cmd);
}

int smc_set_cc_indicator(int state)
{
    u8 cmd = 0x14;

    switch(state) {
        case 1:
            cmd = 0x15;
            break;

        case 2:
            cmd = 0x16;
            break;

        default: break;
    }

    return smc_write_raw(cmd);
}

int smc_set_on_indicator(int state)
{
    u8 cmd = 0x12;

    switch(state) {
        case 1:
            cmd = 0x10;
            break;

        case 2:
            cmd = 0x13;
            break;

        default: break;
    }

    return smc_write_raw(cmd);
}

u8 smc_get_events(void)
{
    u8 data = 0;
    smc_read_register(0x41, &data);
    return data;
}

u8 smc_wait_events(u8 mask)
{
    smc_get_events();

    while(true) {
        u8 data = smc_get_events();
        if(data & mask) return data & mask;
    }
}

void smc_get_panic_reason(char* buffer)
{
    u32* buf32 = (u32*)buffer;

    write32(EXI0_CSR, 0x108);
    write32(EXI0_DATA, 0x20000100);
    write32(EXI0_CR, 0x35);
    while(!(read32(EXI0_CSR) & 8));

    for(int i = 0; i < 64 / sizeof(u32); i++)
    {
        write32(EXI0_CSR, 0x108);
        write32(EXI0_CR, 0x31);
        while(!(read32(EXI0_CSR) & 8));

        buf32[i] = read32(EXI0_DATA);
    }

    write32(EXI0_CSR, 0);
}

void smc_set_panic_reason(const char* buffer)
{
    const u32* buf32 = (const u32*)buffer;

    write32(EXI0_CSR, 0x108);
    write32(EXI0_DATA, 0xA0000100);
    write32(EXI0_CR, 0x35);
    while(!(read32(EXI0_CSR) & 8));

    for(int i = 0; i < 64 / sizeof(u32); i++)
    {
        write32(EXI0_CSR, 0x108);
        write32(EXI0_DATA, buf32[i]);
        write32(EXI0_CR, 0x35);
        while(!(read32(EXI0_CSR) & 8));
    }

    write32(EXI0_CSR, 0);
}

void SRAM_TEXT __attribute__((__noreturn__)) smc_shutdown(bool reset)
{
    write16(MEM_FLUSH_MASK, 0b1111);
    while(read16(MEM_FLUSH_MASK) & 0b1111);

    if(read32(LT_RESETS) & 4) {
        write32(LT_ABIF_CPLTL_OFFSET, 0xC0008020);
        write32(LT_ABIF_CPLTL_DATA, 0xFFFFFFFF);
        write32(LT_ABIF_CPLTL_OFFSET, 0xC0000E60);
        write32(LT_ABIF_CPLTL_DATA, 0xFFFFFFDB);
    }

    write32(LT_RESETS_AHB, 0xFFFFCE71);
    write32(LT_RESETS_AHMN, 0xFFFFCD70);
    write32(LT_RESETS_COMPAT, 0xFF8FCDEF);

    write16(MEM_REFRESH_FLAG, 0);

    write16(MEM_SEQ_REG_ADDR, 0x18);
    write16(MEM_SEQ_REG_VAL, 1);
    write16(MEM_SEQ_REG_ADDR, 0x19);
    write16(MEM_SEQ_REG_VAL, 0);
    write16(MEM_SEQ_REG_ADDR, 0x1A);
    write16(MEM_SEQ_REG_VAL, 1);

    write16(MEM_SEQ0_REG_ADDR, 0x18);
    write16(MEM_SEQ0_REG_VAL, 1);
    write16(MEM_SEQ0_REG_ADDR, 0x19);
    write16(MEM_SEQ0_REG_VAL, 0);
    write16(MEM_SEQ0_REG_ADDR, 0x1A);
    write16(MEM_SEQ0_REG_VAL, 1);

    if(reset) {
        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000D00);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0x501);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }

        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000100);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }

        clear32(LT_RESETS, 1);
    } else {
        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000100);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }

        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000D00);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0x101);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }

        {
            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0xA1000D00);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0x108);
            write32(EXI0_DATA, 0x10101);
            write32(EXI0_CR, 0x35);
            while(!(read32(EXI0_CSR) & 8));

            write32(EXI0_CSR, 0);
        }
    }

    while(true);
}

void __attribute__((__noreturn__)) smc_power_off(void)
{
    smc_shutdown(false);
}

void __attribute__((__noreturn__)) smc_reset(void)
{
    smc_shutdown(true);
}
