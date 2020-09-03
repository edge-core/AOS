/* Module Name: STKTPLG_TIMER.C
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


/* INCLUDE FILE DECLARATIONS
 */
#if 0 
#include <vxWorks.h>
#include <semLib.h>
#include <taskLib.h>
#endif
#include "stdio.h"
#include "string.h"

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "stkmgmt_type.h"
#include "stktplg_type.h"
#include "stktplg_om.h"
#include "stktplg_om_private.h"
#include "sysfun.h"
#include "sys_time.h"



/* NAMING CONSTANT DECLARATIONS
 */



/* DATA TYPE DECLARATIONS
 */



/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static BOOL_T  STKTPLG_TIMER_IsTimeOut(UI16_T cur_ticks, UI16_T timeout_ticks);


/* STATIC VARIABLE DECLARATIONS 
 */



/* EXPORTED SUBPROGRAM BODIES
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
BOOL_T STKTPLG_TIMER_StartTimer(UI8_T timer, UI32_T timeout)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    
    /* check if this is valid timer
     */
    if (timer >= STKTPLG_TIMER_MAX)
    {
        return (FALSE);    	
    }
    	         
    /* get control information
     */    	         
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif    
    /* check if we are under valid state
     */
//    if ((ctrl_info_p->state <= STKTPLG_STATE_INIT) ||
    if ((ctrl_info_p->state < STKTPLG_STATE_INIT) ||
        (ctrl_info_p->state > STKTPLG_STATE_SLAVE))
    {
        return (FALSE);    	
    }
    
    /* setting and starting this timer
     */
    ctrl_info_p->timer[timer].ticks_for_timeout = SYS_TIME_GetSystemTicksBy10ms() + timeout;
    ctrl_info_p->timer[timer].active = TRUE;
    
    return (TRUE);      	
    
}    



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
BOOL_T STKTPLG_TIMER_StopTimer(UI8_T timer)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    
    /* check if this is valid timer
     */
    if (timer >= STKTPLG_TIMER_MAX)
    {
        return (FALSE);    	
    }
    	         
    /* get control information
     */    	         
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif    
    /* check if we are under valid state
     */
//    if ((ctrl_info_p->state <= STKTPLG_STATE_INIT) ||
    if ((ctrl_info_p->state < STKTPLG_STATE_INIT) ||
        (ctrl_info_p->state > STKTPLG_STATE_SLAVE))
    {
        return (FALSE);    	
    }
    
    /* stop this timer
     */
    ctrl_info_p->timer[timer].active = FALSE;
    
    return (TRUE);      
    	
}    

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
BOOL_T STKTPLG_TIMER_TimerStatus(UI8_T timer, UI32_T *pTime_to_timeout)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    
    /* check if this is valid timer
     */
    if (timer >= STKTPLG_TIMER_MAX)
    {
        return (FALSE);    	
    }
    	         
    /* get control information
     */    	         
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif    
    /* check if we are under valid state
     */
//    if ((ctrl_info_p->state <= STKTPLG_STATE_INIT) ||
    if ((ctrl_info_p->state < STKTPLG_STATE_INIT) ||
        (ctrl_info_p->state > STKTPLG_STATE_SLAVE))
    {
        return (FALSE);    	
    }
    
    /* timer's status
     */
    *pTime_to_timeout = ctrl_info_p->timer[timer].ticks_for_timeout - SYS_TIME_GetSystemTicksBy10ms();    
    
    return (ctrl_info_p->timer[timer].active);
    
}  


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
BOOL_T STKTPLG_TIMER_CheckTimeOut(UI8_T timer)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI32_T                  cur_ticks, timeout_ticks;
    
    /* check if this is valid timer
     */
    if (timer >= STKTPLG_TIMER_MAX)
    {
        return (FALSE);    	
    }
    	         
    /* get control information
     */    	         
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif    
    /* check if we are under valid state
     */
//    if ((ctrl_info_p->state <= STKTPLG_STATE_INIT) ||
    if ((ctrl_info_p->state < STKTPLG_STATE_INIT) ||
        (ctrl_info_p->state > STKTPLG_STATE_SLAVE))
    {
        return (FALSE);    	
    }
    
    /* check this timer
     */
    if (!ctrl_info_p->timer[timer].active)
    {
        if (timer == STKTPLG_TIMER_HBT0_UP || timer == STKTPLG_TIMER_HBT0_DOWN)
        	return TRUE;
        else
        	return (FALSE);    	
    } 
    
    /* get current ticks
     */
    cur_ticks = SYS_TIME_GetSystemTicksBy10ms();
    timeout_ticks = ctrl_info_p->timer[timer].ticks_for_timeout;
    
    //return (L_MATH_TimeOut((UI16_T) cur_ticks, (UI16_T) timeout_ticks));      
    return (STKTPLG_TIMER_IsTimeOut((UI16_T) cur_ticks, (UI16_T) timeout_ticks));      
    	
}

/* LOCAL SUBPROGRAM BODY
 */
static BOOL_T  STKTPLG_TIMER_IsTimeOut(UI16_T cur_ticks, UI16_T timeout_ticks)
{

#define HALF_DISTANCE_FOR_UI16_T  0x7FFF

   UI16_T delta_ticks;

   delta_ticks = timeout_ticks - cur_ticks;

   return (delta_ticks > HALF_DISTANCE_FOR_UI16_T);

} /* End of STKTPLG_TIMER_IsTimeOut() */
 
