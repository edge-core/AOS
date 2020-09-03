/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    The definitions of initialization for PoE driver.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created	
 *    07/01/2009 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */


#ifndef POEDRV_INIT_H
#define POEDRV_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sysrsc_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
BOOL_T POEDRV_INIT_InitiateSystemResources(void);

/* FUNCTION NAME: POEDRV_INIT_AttachSystemResources
 * PURPOSE: Attach system resource for POEDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void POEDRV_INIT_AttachSystemResources(void);

/* FUNCTION NAME: POEDRV_INIT_GetShMemInfo
 * PURPOSE: Get share memory info
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void POEDRV_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/* FUNCTION NAME : POEDRV_INIT_SetTransitionMode
 * PURPOSE: This function is used to set POEDRV in transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_SetTransitionMode(void);


/* FUNCTION NAME : POEDRV_INIT_EnterTransitionMode
 * PURPOSE: This function is used to force POEDRV to enter transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_EnterTransitionMode(void);


/* FUNCTION NAME : POEDRV_INIT_EnterMasterMode
 * PURPOSE: This function is used to force POEDRV to enter master mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_EnterMasterMode(void);


/* FUNCTION NAME : POEDRV_INIT_EnterSlaveMode
 * PURPOSE: This function is used to force POEDRV to enter slave mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_INIT_EnterSlaveMode(void);

/* ROUTINE NAME - POEDRV_INIT_ProvisionComplete
 * FUNCTION: This function will tell the PoE Driver module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void POEDRV_INIT_ProvisionComplete(void);

/* FUNCTION NAME : POEDRV_INIT_CreateTasks
 * PURPOSE: This function is used to create the main task for PoE driver
 *          module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T POEDRV_INIT_CreateTasks(void);

/* FUNCTION NAME : POEDRV_INIT_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
void POEDRV_INIT_Create_InterCSC_Relation(void);

void POEDRV_INIT_HandleHotInsertion(void);

void POEDRV_INIT_HandleHotRemoval(void);

#endif /* POEDRV_INIT_H */

