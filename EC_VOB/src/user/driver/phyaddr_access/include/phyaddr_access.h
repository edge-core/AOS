/* Module Name: phyaddr_address.h
 * Purpose: 
 *         the interface to access physical address (for I/O) in linux user mode
 *         
 *
 * Notes:  
 *        
 * History:                                                               
 *    07/30/2007 - Echo Chen, Created	
 *    02/19/2007 - Anzhen Zheng, Modified
 *
 * Copyright(C)      Accton Corporation, 2007   				
 */

#ifndef	PHYADDR_ACCESS_H
#define	PHYADDR_ACCESS_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
 
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: PHYADDR_ACCESS_GetVirtualAddr
 * PURPOSE  : This function will get virtual address 
 * INPUT    : physical_address  -- the physcial address of Hardware for I/O  
 * OUTPUT   : virtaul_address   -- the virtual address that linux kernel ioremap physical address
 * RETURN   : TURE/FALSE        -- success / fail
 * NOTES    : 1. user mode program had better keep it for better performance 
 *            2. user mode program must use virtual address in PHYADDR_ACCESS_Read() and PHYADDR_ACCESS_Write() APIs
 */
BOOL_T PHYADDR_ACCESS_GetVirtualAddr(SYS_TYPE_PAddr_T physical_address, SYS_TYPE_VAddr_T *virtual_address);

/* FUNCTION NAME: PHYADDR_ACCESS_Read
 * PURPOSE  : This function is for user mode program to read data from virtual address 
 *            which is got from H/W physical address 
 * INPUT    : virtaul _address  -- virtual address (got from PHYADDR_GetVirtualAddr )
 *            element_size      -- the size of an element. the valid values are:
 *                                 1: one byte 
 *                                 2: two bytes
 *                                 4: four bytes
 *            element_number    -- the numbers of elements the caller will access 
 * OUTPUT   : data_buffer       -- the data buffer to store the read data 
 * RETURN   : TURE/FALSE        -- success / fail 
 * NOTES    : the size of data_buffer = element_size * element_number
 */
BOOL_T PHYADDR_ACCESS_Read(SYS_TYPE_VAddr_T virtual_address, UI32_T element_size ,UI32_T element_number ,UI8_T *data_buffer );


/* FUNCTION NAME: PHYADDR_ACCESS_Write
 * PURPOSE  : This function is for user mode program to write data to a virtual address 
 *            which is got from H/W physical address 
 * INPUT    : virtaul _address  -- virtual address (got from PHYADDR_GetVirtualAddr )
 *            element_size      -- the size of an element. the valid values are:
 *                                 1: one byte 
 *                                 2: two bytes
 *                                 4: four bytes
 *            element_number    -- the numbers of element you will access 
 *            data_buffer       -- the data buffer to be written to H/W IO address 
 * OUTPUT   : None 
 * RETURN   : TURE/FALSE        -- success / fail 
 * NOTES:   : the size of data_buffer = element_size * element_number
 */
BOOL_T PHYADDR_ACCESS_Write(SYS_TYPE_VAddr_T virtual_address, UI32_T element_size, UI32_T element_number ,UI8_T *data_buffer );

/* anzhen.zheng, 2/19/2008 */
/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_Read
 * PURPOSE  : This function is for user mode program to read data from physical address 
 *        
 * INPUT    : physical _address  -- physical address 
 *            element_size      -- the size of an element. the valid values are:
 *                                 1: one byte 
 *                                 2: two bytes
 *                                 4: four bytes
 *            element_number    -- the numbers of elements the caller will access 
 * OUTPUT   : data_buffer       -- the data buffer to store the read data 
 * RETURN   : TURE/FALSE        -- success / fail 
 * NOTES    : the size of data_buffer = element_size * element_number
 */
BOOL_T PHYSICAL_ADDR_ACCESS_Read(SYS_TYPE_PAddr_T physical_address, UI32_T element_size ,UI32_T element_number ,UI8_T *data_buffer );


/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_Write
 * PURPOSE  : This function is for user mode program to write data to a physical address 
 *            
 * INPUT    : physical _address  -- physical address 
 *            element_size      -- the size of an element. the valid values are:
 *                                 1: one byte 
 *                                 2: two bytes
 *                                 4: four bytes
 *            element_number    -- the numbers of element you will access 
 *            data_buffer       -- the data buffer to be written to H/W IO address 
 * OUTPUT   : None 
 * RETURN   : TURE/FALSE        -- success / fail 
 * NOTES:   : the size of data_buffer = element_size * element_number
 */
BOOL_T PHYSICAL_ADDR_ACCESS_Write(SYS_TYPE_PAddr_T physical_address, UI32_T element_size, UI32_T element_number ,UI8_T *data_buffer );

#endif  /* PHYADDR_ACCESS_H */

