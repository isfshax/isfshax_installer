/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2023          Max Thomas <mtinc2@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */
#include "common/types.h"
#include "gpio.h"
#include "latte.h"
#include "common/utils.h"
#include <string.h>

void gpio_set(u16 gpio_id, u8 val)
{
    clear32(LT_GPIO_OWNER, BIT(gpio_id));

    mask32(LT_GPIO_OUT, BIT(gpio_id), (val ? BIT(gpio_id) : 0));
    mask32(LT_GPIOE_OUT, BIT(gpio_id), (val ? BIT(gpio_id) : 0));
}

void gpio_enable(u16 gpio_id, u8 val)
{
    clear32(LT_GPIO_OWNER, BIT(gpio_id));

    clear32(LT_GPIO_INTLVL, BIT(gpio_id));
    clear32(LT_GPIOE_INTLVL, BIT(gpio_id));
    clear32(LT_GPIO_INTMASK, BIT(gpio_id));
    clear32(LT_GPIOE_INTMASK, BIT(gpio_id));
    mask32(LT_GPIO_ENABLE, BIT(gpio_id), (val ? BIT(gpio_id) : 0));
}

void gpio_set_dir(u16 gpio_id, u8 dir)
{
    clear32(LT_GPIO_OWNER, BIT(gpio_id));

    set32(LT_GPIOE_DIR, dir ? BIT(gpio_id) : 0);
    set32(LT_GPIO_DIR, dir ? BIT(gpio_id) : 0);
}

void gpio2_set(u16 gpio_id, u8 val)
{
    clear32(LT_GPIO2_OWNER, BIT(gpio_id));

    mask32(LT_GPIO2_OUT, BIT(gpio_id), (val ? BIT(gpio_id) : 0));
    mask32(LT_GPIOE2_OUT, BIT(gpio_id), (val ? BIT(gpio_id) : 0));
}

void gpio2_enable(u16 gpio_id, u8 val)
{
    clear32(LT_GPIO2_OWNER, BIT(gpio_id));

    clear32(LT_GPIO2_INTLVL, BIT(gpio_id));
    clear32(LT_GPIOE2_INTLVL, BIT(gpio_id));
    clear32(LT_GPIO2_INTMASK, BIT(gpio_id));
    clear32(LT_GPIOE2_INTMASK, BIT(gpio_id));
    mask32(LT_GPIO2_ENABLE, BIT(gpio_id), (val ? BIT(gpio_id) : 0));
}

void gpio2_set_dir(u16 gpio_id, u8 dir)
{
    clear32(LT_GPIO2_OWNER, BIT(gpio_id));

    set32(LT_GPIOE2_DIR, dir ? BIT(gpio_id) : 0);
    set32(LT_GPIO2_DIR, dir ? BIT(gpio_id) : 0);
}

void gpio_ave_i2c_init()
{
    gpio_enable(GP_AVE_SCL, 1);
    gpio_enable(GP_AVE_SDA, 1);
    gpio_enable(GP_AV1_I2C_CLK, 1);
    gpio_enable(GP_AV1_I2C_DAT, 1);
    gpio2_basic_set(GP2_AVRESET, 1);
    gpio2_basic_set(GP2_AVRESET, 0);
    gpio2_basic_set(GP2_AVRESET, 1);
}

void gpio_basic_set(u16 gpio_id, u8 val)
{
    gpio_set_dir(gpio_id, GPIO_DIR_OUT);
    gpio_set(gpio_id, val);
    gpio_enable(gpio_id, 1);
}

void gpio2_basic_set(u16 gpio_id, u8 val)
{
    gpio2_set_dir(gpio_id, GPIO_DIR_OUT);
    gpio2_set(gpio_id, val);
    gpio2_enable(gpio_id, 1);
}
