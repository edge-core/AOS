/* Module Name: SWDRV_INIT.C
 * Purpose: 
 *  This module is in first task of Switch Driver, bring up whole switch
 *  relative driver which will include SwDrv, AmtrDrv, NmtrDrv and chip
 *  driver which is supported by chip vendor or by us.
 *
 * Notes: 
 *
 * History:                                                               
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2001.10.22  -- Jason Hsue,      Creation
 *        2002.09.19  -- Jeff Kao         Add Stacking Mode
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h> 
#include <swdrv_init.h>
#include "sysrsc_mgr.h"
#include "swdrv_type.h"
#include "swdrv_om.h"


/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for SWDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SWDRV_INIT_AttachSystemResources(void)
{
    SWDRV_OM_AttachSystemResources();
}

/* FUNCTION NAME: SWDRV_INIT_GetShMemInfo
 *----------------------------------------------------------------------------------
 * PURPOSE: Get share memory info
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_SWDRV_SHMEM_SEGID;
    *seglen_p = sizeof(SWDRV_TYPE_ShmemData_T);
}

/* FUNCTION NAME: SWDRV_INIT_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init SWDRV driver and LAN driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRV_INIT_InitiateSystemResources(void)
{
#if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_Init();
#endif
}

/* FUNCTION NAME: SWDRV_INIT_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRV_INIT_Create_InterCSC_Relation(void)
{

#if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_Create_InterCSC_Relation();
#endif
}

/* FUNCTION NAME: SWDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create SWDRV task 
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRV_INIT_Create_Tasks(void)
{

}

/* FUNCTION NAME: SWDRV_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Master Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRV_INIT_EnterMasterMode(void)
{

#if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_EnterMasterMode();
#endif

}

/* FUNCTION NAME: SWDRV_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Slave Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRV_INIT_EnterSlaveMode(void)
{
#if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_EnterSlaveMode();
#endif

}

/* FUNCTION NAME: SWDRV_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Transition Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRV_INIT_EnterTransitionMode(void)
{

#if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_EnterTransitionMode();
#endif

}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
void SWDRV_INIT_HandleHotInsertion(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    #if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_HotSwapInsert(unit_id, starting_port_ifindex, number_of_port);
    #endif
}

void SWDRV_INIT_HandleHotRemoval(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    #if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_HotSwapremove(unit_id, starting_port_ifindex, number_of_port);
    #endif
}
#endif

/* FUNCTION NAME: SWDRV_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Set Transition Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRV_INIT_SetTransitionMode(void)
{

#if (SYS_CPNT_SWDRV == TRUE)
    /*SWDRV*/
    SWDRV_SetTransitionMode();
#endif
    
}
