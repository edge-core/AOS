/* MODULE NAME:  phyaddr_access_hardware.c
 * PURPOSE:
 *      the implementation which hardware info to PHYADDR_ACCESS.
 * NOTES:
 *      None.
 * HISTORY
 *    2007/07/30 - Echo Chen, Created
 *    2008/02/19 - Anzhen Zheng, Modified
 *
 * Copyright(C)      Accton Corporation, 2007
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <asm/io.h>
#include "k_sysfun.h"
#include "sys_hwcfg.h"
#include "l_cvrt.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PHYADDR_MAP_READY 0 /* CPLD on AOS5700-54X is accessed through I2C bus */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#if PHYADDR_MAP_READY
typedef struct PHYADDR_MAP_S
{
    SYS_TYPE_PAddr_T physical_address;
    UI32_T size;    
    SYS_TYPE_VAddr_T virtual_address;
} PHYADDR_MAP_T;

typedef struct PHYSICAL_ADDR_MAP_S
{
    SYS_TYPE_PAddr_T physical_address;
    UI32_T size;    
    SYS_TYPE_VAddr_T virtual_address;
} PHYSICAL_ADDR_MAP_T;
static PHYADDR_MAP_T phyaddr_map[] = {
    { SYS_HWCFG_CPLD_BASE , 0x20 , 0 }          /*EPLD*/
};
#define TOTAL_MAP_NUM (sizeof(phyaddr_map)/sizeof(PHYADDR_MAP_T))

static PHYSICAL_ADDR_MAP_T physical_addr_map[] = {
    { SYS_HWCFG_CPLD_BASE , 0x20, 0 }          /*EPLD*/
};
#define MAP_COUNT (sizeof(physical_addr_map)/sizeof(PHYSICAL_ADDR_MAP_T))
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
 /* FUNCTION NAME: PHYADDR_MAP_MapPhysicalAddress
 * PURPOSE  : This function is for the different hardware to get different setting  
 *             
 * INPUT    : NONE 
 * OUTPUT   : NONE 
 * RETURN   : NONE 
 * NOTES    : 
 */
void PHYADDR_MAP_MapPhysicalAddress(void)
{
#if PHYADDR_MAP_READY
    int num;
    for(num =0 ;num < TOTAL_MAP_NUM ;num ++)
    {
       if ( !( phyaddr_map[num].virtual_address =  L_CVRT_PTR_TO_UINT(ioremap ( phyaddr_map[num].physical_address ,  phyaddr_map[num].size ))) )
            printk("<0>\r\n%s:ioremp error (%d).\n", __FUNCTION__, (int)phyaddr_map[num].physical_address);
    }
#endif
    return;
}

/* FUNCTION NAME: PHYADDR_ACCESS_GetVirtualAddr
 * PURPOSE  : This function will get virtual address 
 * INPUT    : physical_address  -- the physcial address of Hardware for I/O  
 * OUTPUT   : virtaul_address   -- the virtual address that linux kernel ioremap physical address
 * RETURN   : TURE/FALSE        -- success / fail
 * NOTES    : 1. user mode program had better keep it for better performance 
 *            2. user mode program must use virtual address in PHYADDR_ACCESS_Read() and PHYADDR_ACCESS_Write() APIs
 */
BOOL_T PHYADDR_MAP_GetVirtualAddr(SYS_TYPE_PAddr_T physical_address, SYS_TYPE_VAddr_T *virtual_address)
{
#if PHYADDR_MAP_READY
    int num;
    int offset;
    for(num =0 ;num < TOTAL_MAP_NUM ;num ++)
    {
        if ( (physical_address >= phyaddr_map[num].physical_address) && 
             (physical_address < phyaddr_map[num].physical_address + phyaddr_map[num].size ) )
        {
            offset = physical_address - phyaddr_map[num].physical_address  ;
            *virtual_address = phyaddr_map[num].virtual_address + offset ;
            return TRUE;
        }
        
    }
#endif

    return FALSE;
}


 /* FUNCTION NAME: PHYSICAL_ADDR_MAP_MapPhysicalAddress
 * PURPOSE  : This function is for the different hardware to get different setting  
 *             
 * INPUT    : NONE 
 * OUTPUT   : NONE 
 * RETURN   : NONE 
 * NOTES    : 
 */
void PHYSICAL_ADDR_MAP_MapPhysicalAddress(void)
{
#if PHYADDR_MAP_READY
    int num;
    for(num =0 ;num < MAP_COUNT ;num ++)
    {
       if ( !( physical_addr_map[num].virtual_address =  L_CVRT_PTR_TO_UINT(ioremap ( physical_addr_map[num].physical_address ,  physical_addr_map[num].size ))) )
            printk("<0>\r\n%s:ioremp error (%d).\n", __FUNCTION__, (int)physical_addr_map[num].physical_address);
    }
#endif
    return;
}

/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_GetVirtualAddr
 * PURPOSE  : This function will get virtual address 
 * INPUT    : physical_address  -- the physcial address of Hardware for I/O  
 * OUTPUT   : virtaul_address   -- the virtual address that linux kernel ioremap physical address
 * RETURN   : TURE/FALSE        -- success / fail
 * NOTES    : 1. virtual address is not  available in user mode, it is only seen in kernel mode
 *            2. user mode program use physical address in PHYSICAL_ADDR_ACCESS_Read() and PHYSICAL_ADDR_ACCESS_Write() APIs
 */
BOOL_T PHYSICAL_ADDR_MAP_GetVirtualAddr(SYS_TYPE_PAddr_T physical_address, SYS_TYPE_VAddr_T *virtual_address)
{
#if PHYADDR_MAP_READY
    int num;
    int offset;
    for(num =0 ;num < MAP_COUNT ;num ++)
    {
        if ( (physical_address >= physical_addr_map[num].physical_address) && 
             (physical_address < physical_addr_map[num].physical_address + physical_addr_map[num].size ) )
        {
            offset = physical_address - physical_addr_map[num].physical_address  ;
            *virtual_address = physical_addr_map[num].virtual_address + offset ;
            return TRUE;
        }
        
    }
#endif
    return FALSE;
}

