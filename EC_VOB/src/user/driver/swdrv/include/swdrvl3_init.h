#ifndef _SWDRVL3_INIT_H_
#define _SWDRVL3_INIT_H_
/* Purpose: 
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
#include "sysrsc_mgr.h"


/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRVL3_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for SWDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SWDRVL3_INIT_AttachSystemResources(void) ;

/* FUNCTION NAME: SWDRVL3_INIT_GetShMemInfo
 *----------------------------------------------------------------------------------
 * PURPOSE: Get share memory info
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL3_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p) ;

/* FUNCTION NAME: SWDRVL3_INIT_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init SWDRV driver and LAN driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL3_INIT_InitiateSystemResources(void) ;

/* FUNCTION NAME: SWDRVL3_INIT_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL3_INIT_Create_InterCSC_Relation(void) ;

/* FUNCTION NAME: SWDRVL3_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create SWDRV task 
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL3_INIT_Create_Tasks(void) ;

/* FUNCTION NAME: SWDRVL3_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Master Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL3_INIT_EnterMasterMode(void) ;

/* FUNCTION NAME: SWDRVL3_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Slave Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL3_INIT_EnterSlaveMode(void) ;

/* FUNCTION NAME: SWDRVL3_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Enter Transition Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL3_INIT_EnterTransitionMode(void);

/* FUNCTION NAME: SWDRVL3_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: SWDRV Module Set Transition Mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void SWDRVL3_INIT_SetTransitionMode(void) ;
#endif
