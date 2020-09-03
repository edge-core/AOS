/* Module Name: k_phyaddr_address.c (Kernel space)
 * Purpose: 
 *         the implementation to access physical address in linux kernel mode
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

/* INCLUDE FILE DECLARATIONS
 */

#include <asm/io.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "k_sysfun.h"
#include "l_cvrt.h"
#include "k_phyaddr_map.h"
#include "phyaddr_access_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
 
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T PHYADDR_ACCESS_Read(SYS_TYPE_VAddr_T virtual_address, UI32_T element_size ,UI32_T element_number ,UI8_T *data_buffer );
static BOOL_T PHYADDR_ACCESS_Write(SYS_TYPE_VAddr_T virtual_address, UI32_T element_size, UI32_T element_number ,UI8_T *data_buffer );
static UI32_T PHYADDR_ACCESS_Operation(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4);

static BOOL_T PHYSICAL_ADDR_ACCESS_Read(SYS_TYPE_PAddr_T physical_address, UI32_T element_size ,UI32_T element_number ,UI8_T *data_buffer );
static BOOL_T PHYSICAL_ADDR_ACCESS_Write(SYS_TYPE_PAddr_T physical_address, UI32_T element_size, UI32_T element_number ,UI8_T *data_buffer );
static UI32_T PHYSICAL_ADDR_ACCESS_Operation(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4);


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: PHYADDR_ACCESS_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for PHYADDR_ACCESS
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  : successful
 *            FALSE : failure
 * NOTES:   
 */
BOOL_T PHYADDR_ACCESS_Init(void)
{
    PHYADDR_MAP_MapPhysicalAddress();
    return TRUE;
}

/* FUNCTION NAME: PHYADDR_ACCESS_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
void PHYADDR_ACCESS_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_PHYADDR_ACCESS, PHYADDR_ACCESS_Operation);
}

/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */
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
static BOOL_T PHYADDR_ACCESS_Read(SYS_TYPE_VAddr_T virtual_address, UI32_T element_size ,UI32_T element_number ,UI8_T *data_buffer )
{
    int offset;
    void *addr;
    UI8_T tempbuffer[ element_size * element_number  ];

    if (element_size != 1 && element_size != 2 && element_size != 4)
    {
        return FALSE;
    }

    for(offset = 0 ; offset < element_size * element_number  ; offset += element_size)
    {
        addr = L_CVRT_UINT_TO_PTR(virtual_address + offset);
        switch (element_size)
        {
            case 1 :
                *(UI8_T*)(tempbuffer + offset)=(UI8_T) ioread8(addr);
                break;
            case 2 :
                *(UI16_T*)(tempbuffer + offset)=(UI16_T) ioread16(addr);
                break;
            case 4 :
                *(UI32_T*)(tempbuffer + offset)=(UI32_T) ioread32(addr);
                break;
        }
    }
    SYSFUN_CopyToUser( data_buffer  ,tempbuffer , element_size * element_number * sizeof (UI8_T));
    return TRUE;
}

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
static BOOL_T PHYADDR_ACCESS_Write(SYS_TYPE_VAddr_T virtual_address, UI32_T element_size, UI32_T element_number ,UI8_T *data_buffer )
{
    int offset;
    void *addr;
    UI8_T tempbuffer[ element_size * element_number  ];
 
    if (element_size != 1 && element_size != 2 && element_size != 4)
    {
        return FALSE;
    }

    SYSFUN_CopyFromUser(tempbuffer,data_buffer,element_size * element_number * sizeof (UI8_T));
    for(offset = 0 ; offset < element_size * element_number  ; offset += element_size)
    {
        addr = L_CVRT_UINT_TO_PTR(virtual_address + offset);
        switch (element_size)
        {
            case 1 :
                iowrite8(*(UI8_T*)(tempbuffer + offset) , addr);
                break;
            case 2 :
                iowrite16(*(UI16_T*)(tempbuffer + offset) , addr);
                break;
            case 4 :
                iowrite32(*(UI32_T*)(tempbuffer + offset) , addr);                
                break;
        }
    }
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME: PHYADDR_ACCESS_Operation
 * PURPOSE:  
 * INPUT:    cmd  -- command
 *              arg1 - arg4 -- different meanings for different cmd case 
 * OUTPUT: arg1 - arg4 -- different meanings for different cmd case 
 * RETURN : 0 --Fail 
 *              1 --Success
 * NOTE:     None.
 */ 
static UI32_T PHYADDR_ACCESS_Operation(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    int cmd = L_CVRT_PTR_TO_UINT(arg0);

    switch(cmd)
    {
        case PHYADDR_ACCESS_CMD_GET_VIRTUAL_ADDR:
        {
            /* INPUT   : arg1
             * OUTPUT : arg2
             * arg1: physical address
             * arg2: virtual address
             * arg3: NULL
             * arg4: NULL
             */
            SYS_TYPE_VAddr_T virt_addr = 0;
            BOOL_T ret = PHYADDR_MAP_GetVirtualAddr(L_CVRT_PTR_TO_UINT(arg1),&virt_addr);
            SYSFUN_CopyToUser(arg2, &virt_addr, sizeof(virt_addr));
            return (UI32_T)ret;
        }
            break;

        case PHYADDR_ACCESS_CMD_READ:
        {
            /* INPUT   : arg1 arg2 arg3 
             * OUTPUT : arg4             
             * arg1: virtual_address
             * arg2: element_size
             * arg3: element_number
             * arg4: data_buffer
             */
            if (arg1 == 0x0)
            {
                return 0;
            }
            return (UI32_T)PHYADDR_ACCESS_Read(L_CVRT_PTR_TO_UINT(arg1),L_CVRT_PTR_TO_UINT(arg2),L_CVRT_PTR_TO_UINT(arg3),arg4);
        }
            break;

        case PHYADDR_ACCESS_CMD_WRITE:
        {
            /* INPUT   : arg1 arg2 arg3 arg4
             * OUTPUT : NONE  
             * arg1: virtual_address
             * arg2: element_size
             * arg3: element_number
             * arg4: data_buffer
             */
            if (arg1 == 0x0)
            {
                return 0;
            }
            return (UI32_T)PHYADDR_ACCESS_Write(L_CVRT_PTR_TO_UINT(arg1),L_CVRT_PTR_TO_UINT(arg2),L_CVRT_PTR_TO_UINT(arg3),arg4);
        }
            break;
        default:
            printk("<0>%s:Unknown cmd(%d).\n", __FUNCTION__, (int)cmd);
    }

    return 0;
}

/* anzhen.zheng, 2/19/2007 */
/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_Init
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for PHYSICAL_ADDR_ACCESS
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  : successful
 *            FALSE : failure
 * NOTES:   
 */
BOOL_T PHYSICAL_ADDR_ACCESS_Init(void)
{
    PHYSICAL_ADDR_MAP_MapPhysicalAddress();
    return TRUE;
}

/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES:
 */
void PHYSICAL_ADDR_ACCESS_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_PHYSICAL_ADDR_ACCESS, PHYSICAL_ADDR_ACCESS_Operation);
}

/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */
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
static BOOL_T PHYSICAL_ADDR_ACCESS_Read(SYS_TYPE_PAddr_T physical_address, UI32_T element_size ,UI32_T element_number ,UI8_T *data_buffer )
{
    int offset;
    void *addr;
    UI8_T tempbuffer[ element_size * element_number  ];
    SYS_TYPE_VAddr_T virtual_address;
    BOOL_T ret = FALSE;
	
    if (element_size != 1 && element_size != 2 && element_size != 4)
    {
        return FALSE;
    }

    if((ret = PHYSICAL_ADDR_MAP_GetVirtualAddr(physical_address, &virtual_address)) == FALSE)
        return FALSE;

    for(offset = 0 ; offset < element_size * element_number  ; offset += element_size)
    {
        addr = L_CVRT_UINT_TO_PTR(virtual_address + offset);
        switch (element_size)
        {
            case 1 :
                *(UI8_T*)(tempbuffer + offset)=(UI8_T) ioread8(addr);
                break;
            case 2 :
                *(UI16_T*)(tempbuffer + offset)=(UI16_T) ioread16(addr);
                break;
            case 4 :
                *(UI32_T*)(tempbuffer + offset)=(UI32_T) ioread32(addr);
                break;
        }
    }
    SYSFUN_CopyToUser( data_buffer  ,tempbuffer , element_size * element_number * sizeof (UI8_T));
    return TRUE;
}

/* FUNCTION NAME: PHYSICAL_ADDR_ACCESS_Write
 * PURPOSE  : This function is for user mode program to write data to a physical address 
 *            
 * INPUT    : physical _address  -- physical address (is used to get virtual address )
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
static BOOL_T PHYSICAL_ADDR_ACCESS_Write(SYS_TYPE_PAddr_T physical_address, UI32_T element_size, UI32_T element_number ,UI8_T *data_buffer )
{
    int offset;
    void *addr;
    UI8_T tempbuffer[ element_size * element_number  ];
    SYS_TYPE_VAddr_T virtual_address;
    BOOL_T ret = FALSE;
	
    if (element_size != 1 && element_size != 2 && element_size != 4)
    {
        return FALSE;
    }

    if((ret = PHYSICAL_ADDR_MAP_GetVirtualAddr(physical_address, &virtual_address)) == FALSE)
        return FALSE;
 
    SYSFUN_CopyFromUser(tempbuffer,data_buffer,element_size * element_number * sizeof (UI8_T));
    for(offset = 0 ; offset < element_size * element_number  ; offset += element_size)
    {
        addr = L_CVRT_UINT_TO_PTR(virtual_address + offset);
        switch (element_size)
        {
            case 1 :
                iowrite8(*(UI8_T*)(tempbuffer + offset) , addr);
                break;
            case 2 :
                iowrite16(*(UI16_T*)(tempbuffer + offset) , addr);
                break;
            case 4 :
                iowrite32(*(UI32_T*)(tempbuffer + offset) , addr);                
                break;
        }
    }
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME: PHYSICAL_ADDR_ACCESS_Operation
 * PURPOSE:  
 * INPUT:    cmd  -- command
 *              arg1 - arg4 -- different meanings for different cmd case 
 * OUTPUT: arg1 - arg4 -- different meanings for different cmd case 
 * RETURN : 0 --Fail 
 *              1 --Success
 * NOTE:     None.
 */ 
static UI32_T PHYSICAL_ADDR_ACCESS_Operation(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    int cmd = L_CVRT_PTR_TO_UINT(arg0);

    switch(cmd)
    {
        case PHYSICAL_ADDR_ACCESS_CMD_READ:
        {
            /* INPUT   : arg1 arg2 arg3 
             * OUTPUT : arg4             
             * arg1: physical_address
             * arg2: element_size
             * arg3: element_number
             * arg4: data_buffer
             */
            return (UI32_T)PHYSICAL_ADDR_ACCESS_Read(L_CVRT_PTR_TO_UINT(arg1),L_CVRT_PTR_TO_UINT(arg2),L_CVRT_PTR_TO_UINT(arg3),arg4);
        }
            break;

        case PHYSICAL_ADDR_ACCESS_CMD_WRITE:
        {
            /* INPUT   : arg1 arg2 arg3 arg4
             * OUTPUT : NONE  
             * arg1: physical_address
             * arg2: element_size
             * arg3: element_number
             * arg4: data_buffer
             */
            return (UI32_T)PHYSICAL_ADDR_ACCESS_Write(L_CVRT_PTR_TO_UINT(arg1),L_CVRT_PTR_TO_UINT(arg2),L_CVRT_PTR_TO_UINT(arg3),arg4);
        }
            break;
        default:
            printk("<0>%s:Unknown cmd(%d).\n", __FUNCTION__, (int)cmd);
    }

    return 0;
}
