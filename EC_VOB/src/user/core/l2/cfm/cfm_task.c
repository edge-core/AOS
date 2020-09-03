/*-----------------------------------------------------------------------------
 * Module Name: cfm_task.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementations for the CFM task
 *-----------------------------------------------------------------------------
 * NOTES:
 * This file won't be used at linux plateform, it only will be used at VxWorks
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    15/12/2006 - macauley_cheng, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syslog_mgr.h"

#include "cfm_task.h"
#include "cfm_mgr.h"
#include "backdoor_mgr.h"
#include "cfm_backdoor.h"
#include "cfm_engine.h"
#include "swctrl.h"
#include "l2mux_mgr.h"
#include "xstp_mgr.h"
#include "vlan_mgr.h"
#if (SYS_CPNT_CFM == TRUE)
static  UI32_T  CFM_TASK_TaskId;
static  UI32_T  CFM_TASK_TimerId;
static  UI32_T  CFM_TASK_MsgqId;
static  BOOL_T  CFM_TASK_IsTransitionDone=FALSE;
static  BOOL_T  CFM_TASK_IsProvisionComplete = FALSE;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_AnnounceCFMDUPacket
 * ------------------------------------------------------------------------
 * FUNCTION : Callback function for l2_mux when receive CFMDU
 * INPUT    : mem_ref, dst_mac, src_mac, type, lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function has the usage of OM sempaphore, should it be a must?
 * ------------------------------------------------------------------------
 */
static void CFM_TASK_AnnounceCFMDU(
    L_MREF_T    *mem_ref,

    UI8_T       *dst_mac,
    UI8_T       *src_mac,
    UI16_T      tag_info,
    UI16_T      type,
    UI32_T      packet_length,
    UI32_T      lport
)
{
    CFM_TYPE_PaceketHeader_T *packet_header=NULL;
    CFM_TYPE_Msg_T     msg;
    UI32_T                     task_id, msgq_id;
    UI32_T                     current_mode;

    current_mode = CFM_MGR_GetOperationMode();

    /* Not in Master Mode : release this packet buffer and reject this request */
    if (current_mode != SYS_TYPE_STACKING_MASTER_MODE)
    {
        L_MREF_ReleaseReference(&mem_ref);
        return;
    }

    if(NULL == (packet_header=(CFM_TYPE_PaceketHeader_T *)malloc(sizeof(CFM_TYPE_PaceketHeader_T))))
    {
        return;
    }

    memcpy(packet_header->dstMac,dst_mac,SYS_ADPT_MAC_ADDR_LEN);
    memcpy(packet_header->srcMac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    packet_header->lport = lport;
    packet_header->tagInfo= tag_info;

    msg.packet_header_p = packet_header;
    msg.mem_ref_p= mem_ref;

    task_id = CFM_TASK_TaskId;
    msgq_id = CFM_TASK_MsgqId;

    if(SYSFUN_SendMsgQ(msgq_id, (UI32_T *)&msg, FALSE, SYSFUN_TIMEOUT_NOWAIT) == SYSFUN_OK)
    {
        /* send event to task */
        SYSFUN_SendEvent(task_id, CFM_TYPE_EVENT_CFMDURCVD);
    }
    else
    {
        L_MREF_ReleaseReference(&mem_ref);
    }

    return ;
}



/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_Init
 *-------------------------------------------------------------------------
 * FUNCTION: Init the CFM task
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : This function is called by Bridge_initiate_system_resource
 *-------------------------------------------------------------------------
 */
void CFM_TASK_Init()
{
    UI32_T  msgq_id;

    /* Create CFM message queue
     * CFM MsgQ: 64 messages of 16 bytes which contain the pointer to the packet buffer
     */
    SYSFUN_CreateMsgQ (CFM_TASK_MAX_MSGQ_LEN, SYSFUN_MSG_FIFO, &msgq_id);
    CFM_TASK_MsgqId = msgq_id;

    CFM_MGR_Init();
    CFM_OM_Init();


    CFM_TASK_Create_InterCSC_Relation();

    CFM_TASK_IsTransitionDone = FALSE;

    /*regist to get the packet  from driver*/
    L2MUX_MGR_Register_CFM_Handler(CFM_TASK_AnnounceCFMDU);
    return;
}/* End of CFM_TASK_Init() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_Initiate_System_Resources
 *-------------------------------------------------------------------------
 * FUNCTION: Init the CFM System_Resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_Initiate_System_Resources()
{
    CFM_TASK_Init();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_Create_InterCSC_Relation()
{
    /* Register callback function */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("CFM", CFM_BACKDOOR_Main);

    L2MUX_MGR_Register_CFM_Handler(&CFM_TASK_AnnounceCFMDU);

    SWCTRL_Register_TrunkMemberAdd_CallBack(CFM_MGR_ProcessTrunkAddMember_Callback);
    SWCTRL_Register_TrunkMemberAdd1st_CallBack(CFM_MGR_ProcessTrunkAdd1stMember_Callback);
    SWCTRL_Register_TrunkMemberDeleteLst_CallBack(CFM_MGR_ProcessTrunkDeleteLastMember_Callback);
    SWCTRL_Register_TrunkMemberDelete_CallBack(CFM_MGR_ProcessTrunkMemberDelete_Callback);

    SWCTRL_Register_LPortNotOperUp_CallBack(CFM_MGR_ProcessInterfaceStatusChange_Callback);
    SWCTRL_Register_LPortOperUp_CallBack(CFM_MGR_ProcessInterfaceStatusChange_Callback);
    SWCTRL_Register_LPortAdminDisableBefore_CallBack(CFM_MGR_ProcessPortAdminDisable_Callback);
    VLAN_MGR_RegisterVlanCreated_CallBack(CFM_MGR_ProcessVlanCreate_Callback);

}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_StartTimerEvent
 * ------------------------------------------------------------------------
 * FUNCTION : Service routine to start the periodic timer event for the
 *            spanning tree.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : The periodic timer event is sent to the task which creates
 *            the timer. Hence we have to set the timer by the CFM
 *            task itself.
 * ------------------------------------------------------------------------
 */
static void CFM_TASK_StartTimerEvent()
{
    UI32_T  timer_id;
    SYSFUN_StartPeriodicTimerEvent(CFM_TYPE_TIMER_TICKS2SEC,CFM_TYPE_EVENT_TIMER,0, &timer_id);

    CFM_TASK_TimerId = timer_id;
    return;

}/* End of CFM_TASK_StartTimerEvent() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_Main
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize CFM function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void CFM_TASK_Main()
{
    UI32_T                          events, all_events;
    UI32_T                          msgq_id;
    UI32_T                          timeout;
    UI32_T                          current_mode = 0;
    CFM_TYPE_Msg_T                 msg;

    CFM_TASK_StartTimerEvent();

    all_events = CFM_TYPE_EVENT_NONE;
    while(1)
    {
        if(all_events)
            timeout = (UI32_T)SYSFUN_TIMEOUT_NOWAIT;
        else
            timeout = (UI32_T)SYSFUN_TIMEOUT_WAIT_FOREVER;

        /* this will wait event */
        SYSFUN_ReceiveEvent(CFM_TYPE_EVENT_ALL,
                            SYSFUN_EVENT_WAIT_ANY, timeout, &events);
        all_events |= events;
        all_events &= (CFM_TYPE_EVENT_TIMER|CFM_TYPE_EVENT_CFMDURCVD|CFM_TYPE_EVENT_ENTER_TRANSITION);

        current_mode = CFM_MGR_GetOperationMode();

        /* In stacking transition mode */
        if(current_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            CFM_TASK_IsProvisionComplete = FALSE;

            /* When in transition mode, the packet received by CFM engine should be invalid.
             * So we must clear the message queue.
             */
            while(SYSFUN_ReceiveMsgQ(CFM_TASK_MsgqId, (UI32_T *)&msg, SYSFUN_TIMEOUT_NOWAIT) == SYSFUN_OK)
            {
                L_MREF_ReleaseReference (&msg.mem_ref_p);
            }

            /* if receive enter transition event*/
            if(all_events & CFM_TYPE_EVENT_ENTER_TRANSITION)
            {
                CFM_TASK_IsTransitionDone = TRUE;
            }

            all_events = CFM_TYPE_EVENT_NONE;

            /* trap notify may be needed */
            SYSFUN_Sleep(200);

            continue;
        }

        /* slave mode */
        else if(current_mode == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            all_events = CFM_TYPE_EVENT_NONE;
            SYSFUN_Sleep(200);

            continue;
        }

        else /* master mode */
        {
            if(CFM_TASK_IsProvisionComplete)
            {/* Timer events */
                /* recv packet event*/
                if((all_events & CFM_TYPE_EVENT_CFMDURCVD))
                {
                    msgq_id = CFM_TASK_MsgqId;

                    if(SYSFUN_ReceiveMsgQ((UI32_T)msgq_id, (UI32_T*)&msg, SYSFUN_TIMEOUT_NOWAIT) == SYSFUN_OK)
                    {
                        CFM_TYPE_CfmStatus_T global_status,port_status;

                        if(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW))
                        {
                            printf("\nsrc mac=%x-%x-%x-%x-%x-%x, dst mac=%x-%x-%x-%x-%x-%x, lport= %ld, vid=%d",
                            msg.packet_header_p->srcMac[0],msg.packet_header_p->srcMac[1],msg.packet_header_p->srcMac[2],msg.packet_header_p->srcMac[3],msg.packet_header_p->srcMac[4],
                            msg.packet_header_p->srcMac[5],msg.packet_header_p->dstMac[0],msg.packet_header_p->dstMac[1],msg.packet_header_p->dstMac[2],msg.packet_header_p->dstMac[3],
                            msg.packet_header_p->dstMac[4],msg.packet_header_p->dstMac[5],msg.packet_header_p->lport,msg.packet_header_p->tagInfo&0x0fff);
                        }

                        CFM_OM_GetCFMGlobalStatus(&global_status);
                        CFM_OM_GetCFMPortStatus(msg.packet_header_p->lport, &port_status);

                        if((CFM_TYPE_CFM_STATUS_ENABLE == global_status)&&(CFM_TYPE_CFM_STATUS_ENABLE==port_status))
                        {
                            CFM_MGR_ProcessRcvdPDU(&msg);
                        }

                        L_MREF_ReleaseReference(&(msg.mem_ref_p));
                        free(msg.packet_header_p);

                    }
                    else
                    {
                        all_events &= ~CFM_TYPE_EVENT_CFMDURCVD;
                    }
                }

                if(all_events & CFM_TYPE_EVENT_TIMER)
                {
                    CFM_MGR_ProcessTimerEvent();
                    all_events &= ~CFM_TYPE_EVENT_TIMER;
                }

            }
            else
                all_events = CFM_TYPE_EVENT_NONE;
        }
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_EnterMasterMode()
{
    CFM_OM_EnterMasterMode();
    CFM_MGR_EnterMasterMode();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_EnterSlaveMode()
{
    CFM_OM_EnterSlaveMode();
    CFM_MGR_EnterSlaveMode();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_SetTransitionMode()
{
    CFM_MGR_SetTransitionMode();
    CFM_TASK_IsTransitionDone = FALSE;
    SYSFUN_SendEvent (CFM_TASK_TaskId, CFM_TYPE_EVENT_ENTER_TRANSITION);
}/* End of CFM_TASK_SetTransitionMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_EnterTransitionMode()
{
    CFM_ENGINE_EnterTransitionMode();
    CFM_OM_EnterTransitionMode();
    CFM_MGR_EnterTransitionMode();
    SYSFUN_TASK_ENTER_TRANSITION_MODE(CFM_TASK_IsTransitionDone);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_CreateTasks
 * ------------------------------------------------------------------------
 * FUNCTION : Create CFM main task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void CFM_TASK_CreateTask()
{
    UI32_T      task_id;
    SYSLOG_OM_RecordOwnerInfo_T     owner_info;

    if ( SYSFUN_SpawnTask(  SYS_BLD_CFM_TASK,                  /* Need to define in Sys_bld.h */
                            SYS_BLD_CFM_TASK_PRIORITY,         /* Need to define in Sys_bld.h */
                            SYS_BLD_TASK_LARGE_STACK_SIZE,     /* Need to define in Sys_bld.h */
                            0,
                            CFM_TASK_Main,
                            0,
                            &task_id) != SYSFUN_OK)
    {
        owner_info.level        = SYSLOG_LEVEL_NOTICE;
        owner_info.module_no    = SYS_MODULE_CFM;              /* Need to define in Sys_module.h */
        owner_info.function_no  = CFM_TYPE_LOG_FUN_CFM_TASK_CREATE_TASK;
        owner_info.error_no     = CFM_TYPE_LOG_ERR_CFM_TASK_CREATE_TASK;
        SYSLOG_MGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, "CFM_TASK", 0, 0);
    }

    CFM_TASK_TaskId        = task_id;
    return ;
}/* End of CFM_TASK_CreateTasks */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CFM_TASK_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell CFM that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void CFM_TASK_ProvisionComplete()
{
    CFM_TASK_IsProvisionComplete = TRUE;
}
#endif/*#if (SYS_CPNT_CFM == TRUE)*/

