/* MODULE NAME:  NETACCESS_TASK.C
 * PURPOSE:
 *  NETACCESS initiation and NETACCESS task creation
 *
 *REASON:
 *      Description:
 *      CREATOR:      Ricky Lin
 *      Date         2006/01/27
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "sys_adpt.h"
#include "netaccess_type.h"
#include "netaccess_mgr.h"
#include "netaccess_om.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "l_mm.h"
#include "l2_l4_proc_comm.h"
#include "l_threadgrp.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETACCESS_TIMER_TICKS1SEC   100         /* every 1 sec send task a timer event */

/* MACRO FUNCTION DECLARATIONS
 */
#define NETACCESS_TASK_GetTGHandle()  L2_L4_PROC_COMM_GetNetaccessGroupTGHandle()

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void NETACCESS_TASK_Body();
static void NETACCESS_TASK_LocalCleanNewMacMsg(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_ptr);

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_transition_done;

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_TASK_CreateTask()
 *---------------------------------------------------------------------------
 * Function : Create and start NETACCESS task
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_TASK_CreateTask(void)
{
    UI32_T task_id;

    if(SYSFUN_SpawnThread(SYS_BLD_NETACCESS_CSC_THREAD_PRIORITY,
                          SYS_BLD_NETACCESS_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_NETACCESS_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          NETACCESS_TASK_Body,
                          NULL,
                          &task_id)!= SYSFUN_OK)
    {
        task_id = 0;
    }

    NETACCESS_OM_SetTaskId(task_id);

} /* End of NETACCESS_TASK_CreateTask() */

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_TASK_InitiateSystemResources
 *---------------------------------------------------------------------------
 * Function : Initialize NETACCESS's Task .
 * Input    : None
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *---------------------------------------------------------------------------
 */
BOOL_T NETACCESS_TASK_InitiateSystemResources(void)
{
    UI32_T msgq_id;

    /* Create Queue for new MAC msg
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CSC_NETACCESS_TASK_NEW_MAC_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &msgq_id) == SYSFUN_OK)
    {
        NETACCESS_OM_SetNewMacMsgQId(msgq_id);
    }
    else
    {
        return FALSE;
    }

    /* Create Queue for dot1x msg
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CSC_NETACCESS_TASK_DOT1X_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &msgq_id) == SYSFUN_OK)
    {
        NETACCESS_OM_SetDot1xMsgQId(msgq_id);
    }
    else
    {
        return FALSE;
    }

    /* Create Queue for RADIUS MAC msg
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CSC_NETACCESS_TASK_RADIUS_MAC_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &msgq_id) == SYSFUN_OK)
    {
        NETACCESS_OM_SetRadiusMsgQId(msgq_id);
    }
    else
    {
        return FALSE;
    }

    /* Create Queue for link state change msg
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CSC_NETACCESS_TASK_LINK_STCNG_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &msgq_id) == SYSFUN_OK)
    {
        NETACCESS_OM_SetLinkStateChangeMsgQId(msgq_id);
    }
    else
    {
        return FALSE;
    }

    /* Create Queue for vlan change msg
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CSC_NETACCESS_TASK_VLAN_CNG_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &msgq_id) == SYSFUN_OK)
    {
        NETACCESS_OM_SetVlanModifiedMsgQId(msgq_id);
    }
    else
    {
        return FALSE;
    }

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC) || (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    /* Create Queue for MAC age out msg
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CSC_NETACCESS_TASK_MAC_AGE_OUT_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &msgq_id) == SYSFUN_OK)
    {
        NETACCESS_OM_SetMacAgeOutMsgQId(msgq_id);
    }
    else
    {
        return FALSE;
    }
#endif /* SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC */

    is_transition_done = FALSE;
    return TRUE;
} /* End of NETACCESS_TASK_InitiateSystemResources() */

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_TASK_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * Function : This function initializes all function pointer registration operations.
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_TASK_Create_InterCSC_Relation(void)
{
    return;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void NETACCESS_TASK_SetTransitionMode()
{
    UI32_T task_id;

    is_transition_done = FALSE;
    NETACCESS_OM_GetTaskId(&task_id);
    SYSFUN_SendEvent (task_id, NETACCESS_EVENT_ENTER_TRANSITION);
} /* End of NETACCESS_TASK_SetTransitionMode() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void    NETACCESS_TASK_EnterTransitionMode()
{
    /*  want task release all resources */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
} /* End of NETACCESS_TASK_EnterTransitionMode() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Master mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void NETACCESS_TASK_EnterMasterMode()
{
    return;
} /* End of NETACCESS_TASK_EnterMasterMode() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Slave mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void NETACCESS_TASK_EnterSlaveMode()
{
    return;
} /* End of NETACCESS_TASK_EnterSlaveMode() */

/* LOCAL SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NETACCESS_TASK_Body
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
static void NETACCESS_TASK_Body()
{
    UI8_T                       msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(NETACCESS_MSGQ_T))];
    SYSFUN_Msg_T                *msg_p = (SYSFUN_Msg_T *)msgbuf;
    NETACCESS_MSGQ_T            *na_msg_p = (NETACCESS_MSGQ_T *)msg_p->msg_buf;
    void                        *tm_id;
    L_THREADGRP_Handle_T        tg_handle;
    UI32_T                      timeout, queue_id, events, all_events, member_id;
    SYS_TYPE_Stacking_Mode_T    current_mode;

    /* Join the thread group
     */
    tg_handle = NETACCESS_TASK_GetTGHandle();
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_NETACCESS_GROUP_MGR_THREAD_PRIORITY, &member_id) != TRUE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    all_events = NETACCESS_EVENT_NONE;

    tm_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(tm_id, NETACCESS_TIMER_TICKS1SEC, NETACCESS_EVENT_TIMER);

    while (1)
    {
        if (all_events)
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        SYSFUN_ReceiveEvent(NETACCESS_EVENT_ALL,SYSFUN_EVENT_WAIT_ANY,timeout,&events);
        all_events |= events;
        if (all_events == 0)
        {
            /*  Log to system : ERR--Receive Event Failure */
            continue;
        }

         /* Get the system operation mode from MGR */
        current_mode =  NETACCESS_MGR_GetCurrentOperationMode();
        if (current_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            /* task in transition mode, should clear resource (msgQ) in task */
            NETACCESS_OM_GetNewMacMsgQId(&queue_id);

            while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
            {
                NETACCESS_TASK_LocalCleanNewMacMsg(&na_msg_p->new_mac_msg); /* free buffer saved in message */
            }   /*  end of while */

            NETACCESS_OM_GetDot1xMsgQId(&queue_id);

            while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
            {
                if (NULL != na_msg_p->dot1x_msg.m_dot1x_data)
                {
                    L_MM_Free(na_msg_p->dot1x_msg.m_dot1x_data);
                }
            }/* End of while()*/

            NETACCESS_OM_GetRadiusMsgQId(&queue_id);
            while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
            {
                if (NULL != na_msg_p->radius_msg.m_radius_data)
                {
                    L_MM_Free(na_msg_p->radius_msg.m_radius_data);
                }
            }/* End of while()*/

            /* Set flag to Transition done, make STKCTRL task turn-back */
            if (all_events & NETACCESS_EVENT_ENTER_TRANSITION )
                is_transition_done = TRUE;
            all_events = 0;
            continue;
        }
        else if (current_mode == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            all_events = 0;
            continue;
        }

        all_events &= (NETACCESS_EVENT_TIMER | NETACCESS_EVENT_NEWMAC |
                        NETACCESS_EVENT_RADIUS_RESULT | NETACCESS_EVENT_DOT1X_RESULT |
                        NETACCESS_EVENT_LINK_STATE_CHANGE | NETACCESS_EVENT_VLAN_MODIFIED|
                        NETACCESS_EVENT_MAC_AGE_OUT);

        /* request thread group execution permission
         */
        if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
        }

        if (all_events)
        {
            if (all_events & NETACCESS_EVENT_MAC_AGE_OUT)
            {
                NETACCESS_OM_GetMacAgeOutMsgQId(&queue_id);
                while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
                {
                    NETACCESS_MGR_ProcessMacAgeOutMsg(&na_msg_p->mac_ageout_msg);
                    if (NULL != na_msg_p->mac_ageout_msg.m_macageout_data)
                    {
                        L_MM_Free(na_msg_p->mac_ageout_msg.m_macageout_data);
                    }
                }/* End of while()*/

                all_events &= ~NETACCESS_EVENT_MAC_AGE_OUT;
            }/* End of if (all_events & NETACCESS_EVENT_MAC_AGE_OUT )*/

            if (all_events & NETACCESS_EVENT_TIMER)
            {
                NETACCESS_MGR_ProcessTimerEvent();
                all_events &= ~NETACCESS_EVENT_TIMER;
            }/* End of if (all_events & NETACCESS_EVENT_TIMER )*/

            if (all_events & NETACCESS_EVENT_RADIUS_RESULT)/*receive event from RADIUS client */
            {
                NETACCESS_OM_GetRadiusMsgQId(&queue_id);
                while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
                {
                    NETACCESS_MGR_ProcessRadiusMsg(&na_msg_p->radius_msg);
                    if (NULL != na_msg_p->radius_msg.m_radius_data)
                    {
                        L_MM_Free(na_msg_p->radius_msg.m_radius_data);
                    }
                }/* End of while()*/
                all_events &= ~NETACCESS_EVENT_RADIUS_RESULT;
            }/* End of if (all_events & NETACCESS_EVENT_RADIUS_RESULT )*/

            if (all_events & NETACCESS_EVENT_DOT1X_RESULT)/*receive event from dot1x */
            {
                NETACCESS_OM_GetDot1xMsgQId(&queue_id);
                while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
                {
                    NETACCESS_MGR_ProcessDot1xMsg(&na_msg_p->dot1x_msg);
                    if (NULL != na_msg_p->dot1x_msg.m_dot1x_data)
                    {
                        L_MM_Free(na_msg_p->dot1x_msg.m_dot1x_data);
                    }
                }/* End of while()*/
                all_events &= ~NETACCESS_EVENT_DOT1X_RESULT;
            }/* End of if (all_events & NETACCESS_EVENT_DOT1X_RESULT )*/

            if (all_events & NETACCESS_EVENT_NEWMAC )/* receive packet from Supplicant*/
            {
                NETACCESS_OM_GetNewMacMsgQId(&queue_id);
                while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
                {
                    NETACCESS_MGR_ProcessNewMacMsg(&na_msg_p->new_mac_msg);
                    NETACCESS_TASK_LocalCleanNewMacMsg(&na_msg_p->new_mac_msg);
                }/* End of while()*/
                all_events &= ~NETACCESS_EVENT_NEWMAC;
            }/* End of if (all_events & NETACCESS_EVENT_NEWMAC )*/

            if (all_events & NETACCESS_EVENT_LINK_STATE_CHANGE)/* NETACCESS_LinkStateChange_T */
            {
                NETACCESS_OM_GetLinkStateChangeMsgQId(&queue_id);
                while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
                {
                    NETACCESS_MGR_ProcessLinkStateChangeMsg(&na_msg_p->link_change_msg);
                }/* End of while()*/
                all_events &= ~NETACCESS_EVENT_LINK_STATE_CHANGE;
            }/* End of if (all_events & NETACCESS_EVENT_LINK_STATE_CHANGE )*/

            if (all_events & NETACCESS_EVENT_VLAN_MODIFIED)
            {
                NETACCESS_OM_GetVlanModifiedMsgQId(&queue_id);
                while (SYSFUN_ReceiveMsg(queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(msgbuf), msg_p) == SYSFUN_OK)
                {
#if (SYS_CPNT_REFINE_IPC_MSG == TRUE)
                    NETACCESS_MGR_ProcessVlanListModifiedMsg(&na_msg_p->vlan_modified_msg);
#else
                    NETACCESS_MGR_ProcessVlanModifiedMsg(&na_msg_p->vlan_modified_msg);
#endif
                }/* End of while()*/
                all_events &= ~NETACCESS_EVENT_VLAN_MODIFIED;
            }/* End of if (all_events & NETACCESS_EVENT_VLAN_MODIFIED )*/
        }/*End of if (all_events)*/

        /* release thread group execution permission
         */
        if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
        }
    } /* End of while() */
} /* End of NETACCESS_TASK_Body() */


/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NETACCESS_TASK_LocalCleanNewMacMsg
 * ------------------------------------------------------------------------
 *  FUNCTION : free memory
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
static void NETACCESS_TASK_LocalCleanNewMacMsg(NETACCESS_NEWMAC_MSGQ_T *new_mac_msg_ptr)
{
    if (NULL == new_mac_msg_ptr)
        return;

    if (NULL != new_mac_msg_ptr->m_newmac_data)
    {
        L_MM_Free(new_mac_msg_ptr->m_newmac_data);
    }
    if (NULL != new_mac_msg_ptr->m_eap_data)
    {
        L_MM_Free(new_mac_msg_ptr->m_eap_data->pkt_data);
        L_MM_Free(new_mac_msg_ptr->m_eap_data);
    }
}

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
