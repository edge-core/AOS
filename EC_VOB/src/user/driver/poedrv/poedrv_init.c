/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_init.c
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file includes functions to initialize system resources and 
 *    create task for PoE driver.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created	
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "poedrv.h"
#include "poedrv_type.h"
#include "poedrv_init.h"
#include "sysrsc_mgr.h"
#include "leaf_sys.h"
#include "phyaddr_access.h"
#include "sys_hwcfg.h"

//#include "stktplg_board.h"

/* NAMING CONSTANT DECLARATIONS
 */
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS 
 */


/* STATIC VARIABLE DECLARATIONS 
 */
//static BOOL_T   poedrv_init_support_poe;

/* MACRO FUNCTIONS DECLARACTION
 */
 

/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME : POEDRV_INIT_InitiateSystemResources
 * PURPOSE: This function initializes all related variables and resources
 *          on PoE driver
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T POEDRV_INIT_InitiateSystemResources(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return TRUE;

    if (POEDRV_InitiateSystemResources()==FALSE)
		return FALSE;

    return TRUE;

} /* End of POEDRV_INIT_InitiateSystemResources() */

/* FUNCTION NAME: POEDRV_INIT_AttachSystemResources
 * PURPOSE: Attach system resource for POEDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void POEDRV_INIT_AttachSystemResources(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

	POEDRV_AttachSystemResources();
}

/* FUNCTION NAME: POEDRV_INIT_GetShMemInfo
 * PURPOSE: Get share memory info
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void POEDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

    *segid_p = SYSRSC_MGR_POEDRV_SHMEM_SEGID;
    *seglen_p = sizeof(POEDRV_TYPE_ShmemData_T);
}


/* FUNCTION NAME : POEDRV_INIT_SetTransitionMode
 * PURPOSE: This function is used to set POEDRV in transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_SetTransitionMode(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

    POEDRV_SetTransitionMode();

} /* End of POEDRV_INIT_SetTransitionMode() */


/* FUNCTION NAME : POEDRV_INIT_EnterTransitionMode
 * PURPOSE: This function is used to force POEDRV to enter transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_EnterTransitionMode(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

    POEDRV_EnterTransitionMode();

} /* End of POEDRV_INIT_EnterTransitionMode() */


/* FUNCTION NAME : POEDRV_INIT_EnterMasterMode
 * PURPOSE: This function is used to force POEDRV to enter master mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_EnterMasterMode(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

    POEDRV_EnterMasterMode();

} /* End of POEDRV_INIT_EnterMasterMode() */


/* FUNCTION NAME : POEDRV_INIT_EnterSlaveMode
 * PURPOSE: This function is used to force POEDRV to enter slave mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_EnterSlaveMode(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

    POEDRV_EnterSlaveMode();  
 
} /* End of POEDRV_INIT_EnterSlaveMode() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_INIT_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the PoE Driver module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POEDRV_INIT_ProvisionComplete(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

    POEDRV_ProvisionComplete();
}

/* FUNCTION NAME : POEDRV_INIT_Create_Tasks
 * PURPOSE: This function is used to create the main task for PoE driver
 *          module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T POEDRV_INIT_CreateTasks(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return TRUE;

    return POEDRV_CreateTasks();

} /* End of POEDRV_INIT_CreateTasks() */

/* FUNCTION NAME : POEDRV_INIT_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
void POEDRV_INIT_Create_InterCSC_Relation(void)
{
    SYS_TYPE_VAddr_T virt_addr=0;
    UI8_T  poe_status=0;

DBG_PRINT();
    /* Read CPLD to check if PoE module exists */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_BOARD_VERSION_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &poe_status);

    if (!(poe_status & SYS_HWCFG_BOARD_VERSION_POE))
		return;

    POEDRV_Create_InterCSC_Relation();
}

void POEDRV_INIT_HandleHotInsertion(void)
{
    POEDRV_HotSwapInsert();
}

void POEDRV_INIT_HandleHotRemoval(void)
{
   POEDRV_HotSwapremove();
}


