/* MODULE NAME:  phyaddr_access_hardware.h
 * PURPOSE:
 *      Define definitions which hardware info to PHYADDR_ACCESS.
 * NOTES:
 *      None.
 * HISTORY
 *    2007/07/30 - Echo Chen, Created
 *    2008/02/19 - Anzhen Zheng, Modified
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef PHYADDR_MAP_H
#define PHYADDR_MAP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "k_sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

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
void PHYADDR_MAP_MapPhysicalAddress(void);

/* FUNCTION NAME: PHYADDR_ACCESS_GetVirtualAddr
 * PURPOSE  : This function will get virtual address 
 * INPUT    : physical_address  -- the physcial address of Hardware for I/O  
 * OUTPUT   : virtaul_address   -- the virtual address that linux kernel ioremap physical address
 * RETURN   : TURE/FALSE        -- success / fail
 * NOTES    : 1. user mode program had better keep it for better performance 
 *            2. user mode program must use virtual address in PHYADDR_ACCESS_Read() and PHYADDR_ACCESS_Write() APIs
 */
BOOL_T PHYADDR_MAP_GetVirtualAddr(SYS_TYPE_PAddr_T physical_address, SYS_TYPE_VAddr_T *virtual_address);

/* anzhen.zheng, 2/19/2007  */
 /* FUNCTION NAME: PHYSICAL_ADDR_MAP_MapPhysicalAddress
 * PURPOSE  : This function is for the different hardware to get different setting  
 *             
 * INPUT    : NONE 
 * OUTPUT   : NONE 
 * RETURN   : NONE 
 * NOTES    : 
 */
void PHYSICAL_ADDR_MAP_MapPhysicalAddress(void);

/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_GetVirtualAddr
 * PURPOSE  : This function will get virtual address 
 * INPUT    : physical_address  -- the physcial address of Hardware for I/O  
 * OUTPUT   : virtaul_address   -- the virtual address that linux kernel ioremap physical address
 * RETURN   : TURE/FALSE        -- success / fail
 * NOTES    : 1. virtual address is not  available in user mode, it is only seen in kernel mode
 *            2. user mode program use physical address in PHYSICAL_ADDR_ACCESS_Read() and PHYSICAL_ADDR_ACCESS_Write() APIs
 */
BOOL_T PHYSICAL_ADDR_MAP_GetVirtualAddr(SYS_TYPE_PAddr_T physical_address, SYS_TYPE_VAddr_T *virtual_address);

#endif    /* PHYADDR_MAP_H */

