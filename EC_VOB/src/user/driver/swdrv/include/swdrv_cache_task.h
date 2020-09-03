/* Module Name: SWDRV_CACHE_TASK.H
 * Purpose:
 *
 *
 * Notes:
 *      None.
 *
 * History:
 *     2002/8/29    : Rhapsody Create
 *
 * Copyright(C)      Accton Corporation, 2002 , 2003
 */
#ifndef SWDRV_CACHE_TASK_H
#define SWDRV_CACHE_TASK_H

#if (SYS_CPNT_STACKING == TRUE)
#include "l_mm.h"
#include "isc.h"
#endif /* end of SYS_CPNT_STACKING == TRUE */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_TASK_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            GATEWAY database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SWDRV_CACHE_TASK_Init();

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_TASK_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration
 *            operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SWDRV_CACHE_TASK_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - SWDRV_CACHE_TASK_CreateTask
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will create gateway task.  This will be call by root.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void SWDRV_CACHE_TASK_CreateTask();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_TASK_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the SWDRV_CACHE_TASK  enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_CACHE_TASK_EnterMasterMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_TASK_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the SWDRV_CACHE_TASK enter the slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_CACHE_TASK_EnterSlaveMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_TASK_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SWDRV_CACHE_TASK into the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_CACHE_TASK_SetTransitionMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_TASK_EnterTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the SWDRV_CACHE_TASK enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_CACHE_TASK_EnterTransitionMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_CACHE_TASK_TimerHandler
 *---------------------------------------------------------------------------
 * PURPOSE:  Set to BCM driver.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SWDRV_CACHE_TASK_TimerHandler();

#if (SYS_CPNT_STACKING == TRUE)
/* -------------------------------------------------------------------------
 * Function : SWDRV_CACHE_ISC_Handler
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulte all of SWDRV_CACHE via ISC
 * INPUT    : *key            -- key of ISC
 *            *mref_handle_p  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by ISC_AGENT
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_CACHE_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);

#endif /* end of SYS_CPNT_STACKING == TRUE */

#endif /*end of SWDRV_CACHE_TASK_H */
