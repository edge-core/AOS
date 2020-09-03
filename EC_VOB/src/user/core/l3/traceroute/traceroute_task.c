/* FILE NAME  -  TraceRoute_task.c
 *
 *  ABSTRACT :  This packet provides basic TraceRoute functionality.
 *                                                                         
 *  NOTES: This package shall be a reusable component of BNBU L2/L3/L4 switch product lines. 
 *
 *
 * Modification History:                                        
 *   By            Date      Ver.    Modification Description                
 * ------------ ----------   -----   --------------------------------------- 
 *   Amytu       2003-07-01          Modify
 * ------------------------------------------------------------------------
 * Copyright(C)                   ACCTON Technology Corp. 2003      
 * ------------------------------------------------------------------------ 
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include "sys_type.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "l_threadgrp.h"
#include "traceroute_task.h"
#include "traceroute_mgr.h"
#include "app_protocol_proc_comm.h"

#define TRACEROUTE_TASK_EVENT_ENTER_TRANSITION      0x0001L
#define TRACEROUTE_TASK_TIMER_EVENT                 0x0002L   
#define TRACEROUTE_TASK_START_TIMER_EVENT           0x0004L
#define TRACEROUTE_TASK_PROVISION_EVENT             0x0008L

/* DATA TYPE DECLARATIONS
 */

typedef struct  TRACEROUTE_TASK_Lcb_S
{
    BOOL_T      init_flag;                  /* TRUE: TRACEROUTE_TASK initialized */
    UI32_T      traceroute_task_id;         /* TRACEROUTE_TASK ID               */
}  TRACEROUTE_TASK_Lcb_T, *TRACEROUTE_TASK_Lcb_P_T;

/* LOCAL SUBPROGRAM DECLARATION 
 */
static void TRACEROUTE_TASK_TaskMain (void);        
      


/* Static variable Declration 
*/
static  TRACEROUTE_TASK_Lcb_P_T     traceroute_task_lcb;
static  BOOL_T                      traceroute_task_is_transition_done = FALSE;


/* EXPORTED SUBPROGRAM IMPLEMENTATION
 */

/* FUNCTION NAME - TRACEROUTE_TASK_Initiate_System_Resources                                  
 * PURPOSE  : This function allocates and initiates the system resource for
 *            TRACEROUTE database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none 
 * NOTES    : none 
 */
void TRACEROUTE_TASK_Initiate_System_Resources(void)
{
    traceroute_task_lcb = (TRACEROUTE_TASK_Lcb_P_T) malloc (sizeof(TRACEROUTE_TASK_Lcb_T));
    
    if (!traceroute_task_lcb)
    {
        return;
    }    

    traceroute_task_lcb->init_flag   = TRUE;

    traceroute_task_is_transition_done = FALSE;

    return;        
} /* end of TRACEROUTE_TASK_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRACEROUTE_TASK_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TRACEROUTE_TASK_Create_InterCSC_Relation(void)
{
    return;
} /* end of TRACEROUTE_TASK_Create_InterCSC_Relation */

/* FUNCTION NAME - TRACEROUTE_TASK_EnterMasterMode 
 * PURPOSE  : This function will configured TRACEROUTE to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_TASK_EnterMasterMode(void)
{
    
    return;
} /* end of TRACEROUTE_TASK_EnterMasterMode() */


/* FUNCTION NAME - TRACEROUTE_TASK_EnterTransitionMode
 * PURPOSE  : This function will configured TRACEROUTE to enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_EnterTransitionMode(void)
{
    SYSFUN_TASK_ENTER_TRANSITION_MODE(traceroute_task_is_transition_done);
     return;
} /* end of TRACEROUTE_TASK_EnterTransitionMode() */


/* FUNCTION NAME - TRACEROUTE_TASK_EnterSlaveMode
 * PURPOSE  : This function will configured TRACEROUTE to enter master mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none 
 * NOTES    : none 
 */
void TRACEROUTE_TASK_EnterSlaveMode(void)
{
    return;
} /* end of TRACEROUTE_TASK_EnterSlaveMode() */


/* FUNCTION NAME - TRACEROUTE_TASK_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  TRACEROUTE_TASK_SetTransitionMode(void)
{
    traceroute_task_is_transition_done = FALSE;
    SYSFUN_SendEvent (traceroute_task_lcb->traceroute_task_id, TRACEROUTE_TASK_EVENT_ENTER_TRANSITION);

} /* end of TRACEROUTE_TASK_SetTransitionMode() */


/* FUNCTION NAME - TRACEROUTE_TASK_CreateTask                                  
 * PURPOSE  : This function will create TRACEROUTE task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_TASK_CreateTask(void)
{
    if (SYSFUN_SpawnThread(SYS_BLD_TRACEROUTE_TASK_PRIORITY,
                           SYS_BLD_APP_PROTOCOL_GROUP_MGR_SCHED_POLICY,
                           SYS_BLD_TRACEROUTE_TASK,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           TRACEROUTE_TASK_TaskMain,
                           NULL,
                           &traceroute_task_lcb->traceroute_task_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("SYSFUN_SpawnThread fail in %s\n",__FUNCTION__);
    }
    
    return;
}

/* FUNCTION NAME - TRACEROUTE_TASK_ProvisionComplete                                  
 * PURPOSE  : This function will create socket after provision complete
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 */
void TRACEROUTE_TASK_ProvisionComplete(void)
{
    SYSFUN_SendEvent (traceroute_task_lcb->traceroute_task_id, TRACEROUTE_TASK_PROVISION_EVENT);

} /* end of TRACEROUTE_TASK_ProvisionComplete() */

/* FUNCTION NAME - TRACEROUTE_TASK_PeriodicTimerStart_Callback
 * PURPOSE  : This function will send event to tracert task in order to 
 *            start periodic timer
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_TASK_PeriodicTimerStart_Callback(void)
{
    SYSFUN_SendEvent (traceroute_task_lcb->traceroute_task_id, TRACEROUTE_TASK_START_TIMER_EVENT);
}

/* LOCAL SUBPROGRAM BODIES
 */ 
/* FUNCTION NAME - TRACEROUTE_TASK_TaskMain                                  
 * PURPOSE  : Traceroute task body
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */ 
static void TRACEROUTE_TASK_TaskMain (void)
{
    L_THREADGRP_Handle_T tg_handle = APP_PROTOCOL_PROC_COMM_GetAppProtocolGroupHandle();
    UI32_T  member_id, wait_event,event, all_event = 0;
    void    *timer_id;
    I32_T   count_down = 0;
    BOOL_T  is_timer_start = FALSE;

    if (!traceroute_task_lcb->init_flag)
        return;

    if(L_THREADGRP_Join(tg_handle, SYS_BLD_APP_PROTOCOL_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    timer_id = SYSFUN_PeriodicTimer_Create();
    
    wait_event = TRACEROUTE_TASK_EVENT_ENTER_TRANSITION | 
                 TRACEROUTE_TASK_TIMER_EVENT | 
                 TRACEROUTE_TASK_START_TIMER_EVENT |
                 TRACEROUTE_TASK_PROVISION_EVENT;

    while(1)
    {
        SYSFUN_ReceiveEvent (wait_event, SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_WAIT_FOREVER, &event);
        all_event |= event;

        if (all_event & TRACEROUTE_TASK_EVENT_ENTER_TRANSITION)
        {      
            all_event ^= TRACEROUTE_TASK_EVENT_ENTER_TRANSITION;  
            traceroute_task_is_transition_done = TRUE;
            continue;
        }      

        if (all_event & TRACEROUTE_TASK_PROVISION_EVENT) 
        {   
            all_event ^= TRACEROUTE_TASK_PROVISION_EVENT;            
            SYSFUN_Sleep(30 *SYS_BLD_TICKS_PER_SECOND);
        }
          
        if (all_event & TRACEROUTE_TASK_TIMER_EVENT) 
        {
            all_event ^= TRACEROUTE_TASK_TIMER_EVENT;   

            if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\r\n", __FUNCTION__);
         
            if (TRACEROUTE_MGR_TriggerTraceRoute() != TRACEROUTE_TYPE_NO_MORE_PROBE_TO_SEND)
                count_down = 2*SYS_DFLT_TRACEROUTE_CTL_TIME_OUT/SYS_BLD_TICKS_PER_SECOND;

            if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                printf("\r\n%s: L_THREADGRP_Execution_Release fail.\r\n", __FUNCTION__);
            
            if (--count_down <= 0)
            {
                SYSFUN_PeriodicTimer_Stop(timer_id);
                is_timer_start = FALSE;
            }
        }
        
        if (all_event & TRACEROUTE_TASK_START_TIMER_EVENT)
        {
            all_event ^= TRACEROUTE_TASK_START_TIMER_EVENT;
            if (!is_timer_start)
            {
                if (SYSFUN_PeriodicTimer_Start(timer_id, SYS_BLD_TICKS_PER_SECOND, TRACEROUTE_TASK_TIMER_EVENT))
                    is_timer_start = TRUE;
            }            
        }
    }

    SYSFUN_PeriodicTimer_Destroy(timer_id);
    L_THREADGRP_Leave(tg_handle, member_id);

    return;
}

