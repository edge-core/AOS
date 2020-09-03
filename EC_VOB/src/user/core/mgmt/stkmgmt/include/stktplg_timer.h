/* Module Name: STKTPLG_TIMER.H
 *
 * Purpose: 
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005
 */

#ifndef  STKTPLG_TIMER_H
#define  STKTPLG_TIMER_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"


/* EXPORTED SUBPROGRAM DECLARATIONS
 */


/* FUNCTION NAME : STKTPLG_TIMER_StartTimer
 * PURPOSE: This function starts a specific timer.
 * INPUT:   timer -- what timer that we want to start
 *                   STKTPLG_TIMER_HBT0
 *                   STKTPLG_TIMER_HBT1
 *                   STKTPLG_TIMER_RECEIVE_PKTS
 *                   STKTPLG_TIMER_GET_TPLG_INFO
 *          timeout -- timeout value for this timer 
 * OUTPUT:  None
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_TIMER_StartTimer(UI8_T timer, UI32_T timeout);



/* FUNCTION NAME : STKTPLG_TIMER_StopTimer
 * PURPOSE: This function stops a specific timer.
 * INPUT:   timer -- what timer that we want to stop
 *                   STKTPLG_TIMER_HBT0
 *                   STKTPLG_TIMER_HBT1
 *                   STKTPLG_TIMER_RECEIVE_PKTS
 *                   STKTPLG_TIMER_GET_TPLG_INFO
 * OUTPUT:  None
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_TIMER_StopTimer(UI8_T timer);


/* FUNCTION NAME : STKTPLG_TIMER_TimerStatus
 * PURPOSE: This function queries the specific timer status and ticks to be expired.
 * INPUT:   timer -- what timer that we want to start
 *                   STKTPLG_TIMER_HBT0
 *                   STKTPLG_TIMER_HBT1
 *                   STKTPLG_TIMER_RECEIVE_PKTS
 *                   STKTPLG_TIMER_GET_TPLG_INFO
 *          timeout -- timeout value for this timer 
 * OUTPUT:  *pTime_to_timeout -- ticks to timer expire
 * RETUEN:  TRUE         -- Active.
 *          FALSE        -- Inactive.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_TIMER_TimerStatus(UI8_T timer, UI32_T *pTime_to_timeout);


/* FUNCTION NAME : STKTPLG_TIMER_CheckTimeOut
 * PURPOSE: This function checks if a specific timer is timeout or not.
 * INPUT:   timer -- what timer that we want to check
 *                   STKTPLG_TIMER_HBT0
 *                   STKTPLG_TIMER_HBT1
 *                   STKTPLG_TIMER_RECEIVE_PKTS
 *                   STKTPLG_TIMER_GET_TPLG_INFO
 * OUTPUT:  None
 * RETUEN:  TRUE         -- timeout.
 *          FALSE        -- not yet timeout.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_TIMER_CheckTimeOut(UI8_T timer);


#endif   /* STKTPLG_TIMER_H */
