/* Module Name: SWDRV_INIT.H
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
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

#ifndef SWDRV_INIT_H
#define SWDRV_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include <sys_adpt.h>
#include "sysfun.h"
#include <sys_cpnt.h>
#include <swdrv.h>
#include <swdrvl4.h>

#if (SYS_CPNT_SWDRV_CACHE == TRUE)
#include <swdrv_cache_init.h>
#endif

#if (SYS_CPNT_SWDRVL3 == TRUE)
#include <swdrvl3.h>
#endif

#if (SYS_CPNT_AMTRDRV == TRUE)
#include <amtrdrv_mgr.h>
#endif
#include <nmtrdrv.h>

#include "sysrsc_mgr.h"

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_INIT_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init SWDRV driver and LAN driver
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *----------------------------------------------------------------------------------*/
void SWDRV_INIT_InitiateSystemResources(void);

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
void SWDRV_INIT_Create_InterCSC_Relation(void);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create SWDRV task 
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *----------------------------------------------------------------------------------*/
void SWDRV_INIT_Create_Tasks(void);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Master Mode
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *----------------------------------------------------------------------------------*/
void SWDRV_INIT_EnterMasterMode(void);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Slave Mode
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *----------------------------------------------------------------------------------*/
void SWDRV_INIT_EnterSlaveMode(void);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Transition Mode
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *----------------------------------------------------------------------------------*/
void SWDRV_INIT_EnterTransitionMode(void);

/*----------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Set Transition Mode
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 *----------------------------------------------------------------------------------*/
void SWDRV_INIT_SetTransitionMode(void);

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
void SWDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

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
void SWDRV_INIT_AttachSystemResources(void);


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

void SWDRV_INIT_HandleHotInsertion(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port);

void SWDRV_INIT_HandleHotRemoval(UI8_T unit_id, UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif


#endif
