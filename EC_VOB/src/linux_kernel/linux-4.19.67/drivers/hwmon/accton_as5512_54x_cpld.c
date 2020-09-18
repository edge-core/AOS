/*
 * A hwmon driver for the accton lpc cpld
 *
 * Copyright (C) 2015 Accton Technology Corporation.
 * Brandon Chuang <brandon_chuang@accton.com.tw>
 *
 * Based on ad7414.c
 * Copyright 2006 Stefan Roese <sr at denx.de>, DENX Software Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <asm/io.h>

#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/sysfs.h>

#define NUM_OF_CPLD				3

#define CPLD_LPC_BASE_ADDRESS   0xFED50000
#define LPC_CPLD_RANGE          0x400

#define GPIO2_LPC_PIN_NUMBER    22 /* start from 0 */
#define GPIO2_LPC_PIN_MASK      (1 << GPIO2_LPC_PIN_NUMBER)
#define GPIO2_LPC_BASE_ADDR     0x580
#define GPIO2_LPC_USE_SEL       (GPIO2_LPC_BASE_ADDR + 0x0) /* 1 is GPIO , 0 is nactive */
#define GPIO2_LPC_IO_SEL        (GPIO2_LPC_BASE_ADDR + 0x4) /* 1:input, 0:output */ 
#define GPIO2_LPC_IO_LVL        (GPIO2_LPC_BASE_ADDR + 0x8)

#define VALIDATE_CPLD_ADDR(addr)    \
do {\
    if (addr != 0x60 && addr != 0x61 && addr != 0x62){\
        return -ENXIO;\
    }\
} while (0);

static struct device *hwmon_dev = NULL;
static u8 __iomem *cpld_lpc_base_addr = NULL;

static int lpc_access_begin(void)
{
    unsigned int val;

    /* Set the gpio mode = 1 to enable LPC 
     */
    val  = inl(GPIO2_LPC_USE_SEL);
    val |= GPIO2_LPC_PIN_MASK;
    outl(val, GPIO2_LPC_USE_SEL); 

    /* Set gpio direction as output 
     */
    val  = inl(GPIO2_LPC_IO_SEL);
    val &= ~GPIO2_LPC_PIN_MASK;
    outl(val, GPIO2_LPC_IO_SEL);

    /* 2. Set GPIO level as high to activate LPC
     */
    val  = inl(GPIO2_LPC_IO_LVL);
    val |= GPIO2_LPC_PIN_MASK;
    outl(val, GPIO2_LPC_IO_LVL);   
    
    return 0;
}

static int lpc_access_end(void)
{
    return 0;
}

static u8 __iomem *cpld_offset(unsigned short cpld_addr, u8 reg)
{
    unsigned int offset;

    switch(cpld_addr) {
    case 0x60:
        offset = 0x100 + reg;
        break;
    case 0x61:
        offset = 0x200 + reg;
        break;
    case 0x62:
        offset = 0x300 + reg;
        break;
    default:
        return 0;
    }
    
    return (u8 __iomem *)(cpld_lpc_base_addr + offset);
}

int as5512_54x_cpld_read(unsigned short cpld_addr, u8 reg)
{
    int ret = 0;
    VALIDATE_CPLD_ADDR(cpld_addr);

    lpc_access_begin();
    ret = (int)ioread8(cpld_offset(cpld_addr, reg));
    lpc_access_end();

    return ret;
}
EXPORT_SYMBOL(as5512_54x_cpld_read);

int as5512_54x_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
    VALIDATE_CPLD_ADDR(cpld_addr);

    lpc_access_begin();
    iowrite8(value, cpld_offset(cpld_addr, reg));
    lpc_access_end();

    return 0;
}
EXPORT_SYMBOL(as5512_54x_cpld_write);

static ssize_t show_cpld_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i = 0;
    int cpld_version[NUM_OF_CPLD] = {0};
    
    for (i = 0; i < ARRAY_SIZE(cpld_version); i++) {
        cpld_version[i] = as5512_54x_cpld_read(0x60 + i, 0x1);
    }

    return sprintf(buf, "%x.%x.%x", cpld_version[0], cpld_version[1], cpld_version[2]);
}

static struct device_attribute ver = __ATTR(version, S_IRUGO, show_cpld_version, NULL);

static int __init as5512_54x_cpld_init(void)
{
    int status = 0;

    cpld_lpc_base_addr = ioremap(CPLD_LPC_BASE_ADDRESS, LPC_CPLD_RANGE);
    
    if (!cpld_lpc_base_addr) {
        printk(KERN_ERR "Unable to map LPC-CPLD I/O register\n");
        status = -EIO;
        goto exit;
    }

    hwmon_dev = hwmon_device_register(NULL);

    if (IS_ERR(hwmon_dev)) {
        status = PTR_ERR(hwmon_dev);
        printk("Failed to register hwmon device for CPLD\r\n");
        goto exit_ioremap;
    }   
    
    status = sysfs_create_file(&hwmon_dev->kobj, &ver.attr);
    if (status) {
        printk("Failed to create sysfs attribute for CPLD\r\n");
        goto exit_hwmon;
    }

    return status;

exit_hwmon:
    hwmon_device_unregister(hwmon_dev);
    
exit_ioremap:
    iounmap(cpld_lpc_base_addr);    
    
exit:   
    return status;
}

static void __exit as5512_54x_cpld_exit(void)
{
    iounmap(cpld_lpc_base_addr);
    sysfs_remove_file(&hwmon_dev->kobj, &ver.attr);
    hwmon_device_unregister(hwmon_dev);
}

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("accton cpld driver");
MODULE_LICENSE("GPL");

module_init(as5512_54x_cpld_init);
module_exit(as5512_54x_cpld_exit);

