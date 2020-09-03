/* Module Name: PSEC_TASK.C
 * Purpose: Initialize the resources and create task for the port security module.
 *
 * Notes:
 *
 * History:
 *    09/02/02       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2002
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_threadgrp.h"
#include "psec_mgr.h"
#include "l2mux_mgr.h"
#include "leaf_es3626a.h"
#include "amtr_mgr.h"
#include "l2_l4_proc_comm.h"
#include "swctrl.h"
#include "sys_time.h"

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_pmgr.h"
#endif /* SYS_CPNT_LLDP */

#include "netaccess_backdoor.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define PSEC_TASK_EVENT_0_5_SEC                 0x0001L
#define PSEC_TASK_MSG_EVENT                     0x0002L
#define PSEC_TASK_EVENT_ENTER_TRANSITION_MODE   0x0004L

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
#define LOG(fmt, args...) \
    {                                       \
        if(NETACCESS_BACKDOOR_IsOn(psec_task_backdoor_reg_no)) {printf(fmt, ##args);printf("%s","\n");}  \
    }
#else
#define LOG(fmt, args...)
#endif

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

static void PSEC_TASK_Main(void);
/* static void PSEC_TASK_IntrusionMac_CallBack (UI32_T port); */

//static void PSEC_TASK_DispatchMsg (UI8_T *rcv_msg);
static void PSEC_TASK_DispatchMsg (SYSFUN_Msg_T *msg_p);

static BOOL_T PSEC_TASK_IsIntrusionPacket(const UI8_T *da, UI16_T ether_type);

#endif

/* STATIC VARIABLE DECLARATIONS
 */
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

static  UI32_T  psec_task_id;
#if 0 /* There is no need in refined psec schema */
static  void*   tmid_psec;
#endif
static  SYSFUN_MsgQ_T  psec_queue_id;  /* message queue id     */

static  BOOL_T                      psec_task_is_transition_done;

#endif

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
static UI32_T psec_task_backdoor_reg_no;
#endif


/* MACRO FUNCTIONS DECLARACTION
 */
#define PSEC_TASK_GetTGHandle()  L2_L4_PROC_COMM_GetNetaccessGroupTGHandle()


/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME: PSEC_TASK_InitiateProcessResources
 * PURPOSE: This function is used to initialize the port security module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:   Initialize the port security module.
 *
 */
BOOL_T PSEC_TASK_InitiateProcessResources(void)
{
#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
    NETACCESS_BACKDOOR_Register("psec_task", &psec_task_backdoor_reg_no);
#endif
    return(TRUE);
} /* PSEC_TASK_InitiateProcessResources() */

/* FUNCTION NAME: PSEC_TASK_CreateTaskMsgQ
 * PURPOSE: create task msq queue
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE.
 *          FALSE.
 * NOTES:
 *
 */
BOOL_T PSEC_TASK_CreateTaskMsgQ(void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    /* Create Queue */
    if (SYSFUN_CreateMsgQ(SYS_BLD_CSC_PSEC_TASK_MSGQ_KEY, SYSFUN_MSGQ_UNIDIRECTIONAL, &psec_queue_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf(" PSEC_TASK: create message queue fail ! \n");
        while(1);
    }

#endif
    return TRUE;
}

/* FUNCTION NAME: PSEC_TASK_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *
 */
void PSEC_TASK_Create_InterCSC_Relation(void)
{
// TODO: remove; replaced by sys_callback
// SYS_CALLBACK_MGR_CALLBACKEVENTID_INTRUSION_MAC
// SYS_CALLBACK_MGR_CALLBACKEVENTID_SECURITY_PORT_MOVE
#if 0
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
#if 0
    /* Register callback function to lan.c for intrusion MAC address */
    /* LAN_Register_Intrusion_Handler(PSEC_TASK_IntrusionMac_CallBack); */
    L2MUX_MGR_Register_Intrusion_Handler(PSEC_TASK_IntrusionMac_CallBack);
#endif
    AMTR_MGR_Register_IntrusionMac_CallBack(PSEC_TASK_IntrusionMac_CallBack);
    AMTR_MGR_Register_SecurityPortMove_CallBack(PSEC_TASK_PortMove_CallBack);
    /* Register callback function to amtr_mgr for intrusion MAC address */
    /* For AMTR refinement*/
    /*AMTR_MGR_Register_IntrusionMac_CallBack(PSEC_TASK_NA_IntrusionMac_CallBack);*/
#endif
#endif
}

/* FUNCTION NAME: PSEC_TASK_Create_Tasks
 * PURPOSE: Create and start PSEC task.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T PSEC_TASK_Create_Tasks(void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    if (SYSFUN_SpawnThread(SYS_BLD_PORT_SECURITY_THREAD_PRIORITY,
                           SYS_BLD_PORT_SECURITY_THREAD_SCHED_POLICY,
                           SYS_BLD_PORT_SECURITY_THREAD_NAME,
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           PSEC_TASK_Main,
                           NULL,
                           &psec_task_id) != SYSFUN_OK)
    {
        perror("PSEC Task Spawn error");
        return FALSE;
    }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_NETACCESS_PSEC, psec_task_id, SYS_ADPT_NETACCESS_PSEC_SW_WATCHDOG_TIMER );
#endif
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE) */

    return TRUE;

}/* End of PSEC_TASK_Create_Tasks */



/* FUNCTION NAME: PSEC_TASK_SetTransitionMode
 * PURPOSE: set the transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the
 *             PSEC_TASK_EnterMasterMode function.
 */
void  PSEC_TASK_SetTransitionMode (void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    psec_task_is_transition_done=FALSE;
    SYSFUN_SendEvent (psec_task_id, PSEC_TASK_EVENT_ENTER_TRANSITION_MODE);

    /* Need to set to FALSE when re-stacking */

#endif
} /* End of PSEC_TASK_EnterTransitionMode */


/* FUNCTION NAME: PSEC_TASK_EnterTransitionMode
 * PURPOSE: The function enter transition mode and free all PSEC resources
 *          and reset database to factory default.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the
 *             PSEC_TASK_EnterMasterMode function.
 */
void  PSEC_TASK_EnterTransitionMode (void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    SYSFUN_TASK_ENTER_TRANSITION_MODE(psec_task_is_transition_done);
    /* Need to set to FALSE when re-stacking */

#endif
} /* End of PSEC_TASK_EnterTransitionMode */


/* FUNCTION NAME: PSEC_TASK_EnterMasterMode
 * PURPOSE: The function enter master mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
void  PSEC_TASK_EnterMasterMode(void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
    ;

#endif
} /* End of PSEC_TASK_EnterMasterMode */


/* FUNCTION NAME: PSEC_TASK_EnterSlaveMode
 * PURPOSE: The function enter slave mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
void PSEC_TASK_EnterSlaveMode(void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    ;

#endif
} /* End of PSEC_TASK_EnterSlaveMode */

#if (SYS_CPNT_PORT_SECURITY == TRUE) && (SYS_CPNT_NETWORK_ACCESS == FALSE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_TASK_IntrusionMac_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Notification intrusion mac address
 * INPUT    : src_lport, vid, ...
 * OUTPUT   : None.
 * RETURN   : TRUE -- intrusion packet, drop packet / FALSE -- not intrusion, go ahead
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T
PSEC_TASK_IntrusionMac_CallBack(
    UI32_T src_lport,
    UI16_T vid,
    UI8_T *src_mac,
    UI8_T *dst_mac,
    UI16_T ether_type,
    void *cookie)
{
    UI32_T          port_security_enabled_by_who;
    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(PSEC_TYPE_MSG_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;
    PSEC_TYPE_MSG_T *pse_msg_p = (PSEC_TYPE_MSG_T *)msg_p->msg_buf;

    if ((SYS_TYPE_STACKING_MASTER_MODE != PSEC_MGR_GetOperationMode()) ||
        (NULL == dst_mac) || (NULL == src_mac) ||
        (FALSE == PSEC_TASK_IsIntrusionPacket(dst_mac, ether_type)))
    {
        return FALSE;
    }

    LOG("PSEC_TASK_IntrusionMac_CallBack, Port(%lu), MAC(%02X-%02X-%02X-%02X-%02X-%02X)",
        src_lport,
        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]
        );

    /* if packets come from non-secured port, MUST NOT BE intrusion packets
     * or if this port is secured port but not enabled by PSEC, PSEC ingore this packet
     */
    if ((FALSE == SWCTRL_IsSecurityPort(src_lport, &port_security_enabled_by_who)) ||
        (SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC != port_security_enabled_by_who))
    {
        LOG("This is not PSec enabled port");
        return FALSE;
    }

    memset(msg_buf, 0, sizeof(msg_buf));
    msg_p->msg_size = sizeof(PSEC_TYPE_MSG_T);
    msg_p->msg_type = PSEC_TYPE_MSG_INTRUSION_MAC;
    pse_msg_p->port = src_lport;
    pse_msg_p->vid = vid;
    memcpy(pse_msg_p->mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
    pse_msg_p->cookie = cookie;
    pse_msg_p->info = PSEC_TYPE_MSG_INTRUSION_MAC;

    /* enqueue first */
    if (SYSFUN_SendRequestMsg(psec_queue_id, msg_p, SYSFUN_TIMEOUT_NOWAIT, 0, 0, NULL) == SYSFUN_OK)
    {
        /* send event again to push the PSEC to clear up the queue
           in case it lost last event flag  */
        SYSFUN_SendEvent(psec_task_id, PSEC_TASK_MSG_EVENT);

        /* mask for AMTR SW Learning; water_huang
         * L_MM_Mref_Release(&mem_ref); */
        return TRUE;
    }

    /* second, send event to task */
    if (SYSFUN_SendEvent(psec_task_id, PSEC_TASK_MSG_EVENT) != SYSFUN_OK)
    {
        /* what action should take here ? */
    }

    /* mask for AMTR SW Learning; water_huang
     * L_MM_Mref_Release(&mem_ref); */
    return TRUE;
} /* end of PSEC_TASK_IntrusionMac_CallBack */

/* Callback functions
 */
/* FUNCTION NAME: PSEC_TASK_NA_IntrusionMac_CallBack
 * PURPOSE: Notification intrusion mac address
 * INPUT:   l_port, mac, reason
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   intrusion come from amtr
 */
void PSEC_TASK_NA_IntrusionMac_CallBack (UI32_T l_port, UI32_T vid, UI8_T mac[6])
{
    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(PSEC_TYPE_MSG_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;
    PSEC_TYPE_MSG_T *pse_msg_p = (PSEC_TYPE_MSG_T *)msg_p->msg_buf;

    memset(msg_buf, 0, sizeof(msg_buf));
    msg_p->msg_size = sizeof(PSEC_TYPE_MSG_T);
    msg_p->msg_type = PSEC_TYPE_MSG_INTRUSION_MAC;

    if (PSEC_MGR_GetOperationMode()!=SYS_TYPE_STACKING_MASTER_MODE)
        return;

    /* Need to transfer port-mapping or not? Uport ==> lport or otherwise */

    pse_msg_p->port = l_port;

    pse_msg_p->mac[0] = mac[0];
    pse_msg_p->mac[1] = mac[1];
    pse_msg_p->mac[2] = mac[2];
    pse_msg_p->mac[3] = mac[3];
    pse_msg_p->mac[4] = mac[4];
    pse_msg_p->mac[5] = mac[5];

    pse_msg_p->info = PSEC_TYPE_MSG_INTRUSION_MAC;

    if (SYSFUN_SendRequestMsg(psec_queue_id, msg_p, SYSFUN_TIMEOUT_NOWAIT, 0, 0, NULL) == SYSFUN_OK)
    {
        /* Set Event */
        SYSFUN_SendEvent(psec_task_id, PSEC_TASK_MSG_EVENT);
    }

    return;
} /* end of PSEC_TASK_NA_IntrusionMac_CallBack */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_TASK_PortMove_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE  : Notification port move
 * INPUT    : src_lport, vid, ...
 * OUTPUT   : None.
 * RETURN   : TRUE / FALSE
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_TASK_PortMove_CallBack(
                            UI32_T src_lport,
                            UI16_T vid,
                            UI8_T *mac,
                            UI32_T original_ifindex)
{
    UI32_T          port_security_enabled_by_who;
    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(PSEC_TYPE_MSG_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;
    PSEC_TYPE_MSG_T *pse_msg_p = (PSEC_TYPE_MSG_T *)msg_p->msg_buf;

    if ((SYS_TYPE_STACKING_MASTER_MODE != PSEC_MGR_GetOperationMode()) ||
        (NULL == mac))
    {
        return FALSE;
    }

    /* if packets come from non-secured port, MUST NOT BE intrusion packets
     * or if this port is secured port but not enabled by PSEC, PSEC ingore this packet
     */
    if ((FALSE == SWCTRL_IsSecurityPort(src_lport, &port_security_enabled_by_who)) ||
        (SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC != port_security_enabled_by_who))
    {
        return FALSE;
    }

    memset(msg_buf, 0, sizeof(msg_buf));
    msg_p->msg_size = sizeof(PSEC_TYPE_MSG_T);
    msg_p->msg_type = PSEC_TYPE_MSG_STATION_MOVE;
    pse_msg_p->port = src_lport;
    memcpy(pse_msg_p->mac, mac, SYS_ADPT_MAC_ADDR_LEN);
    pse_msg_p->info = PSEC_TYPE_MSG_STATION_MOVE;

    /* enqueue first */
    if (SYSFUN_SendRequestMsg(psec_queue_id, msg_p, SYSFUN_TIMEOUT_NOWAIT, 0, 0, NULL) == SYSFUN_OK)
    {
        /* send event again to push the PSEC to clear up the queue
           in case it lost last event flag  */
        SYSFUN_SendEvent(psec_task_id, PSEC_TASK_MSG_EVENT);

        return TRUE;
    }

    /* second, send event to task */
    if (SYSFUN_SendEvent(psec_task_id, PSEC_TASK_MSG_EVENT) != SYSFUN_OK)
    {
        /* what action should take here ? */
    }

    return TRUE;
}/* end of PSEC_TASK_PortMove_CallBack */

#endif


/* LOCAL SUBPROGRAM BODIES
 */

#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

/* FUNCTION NAME: PSEC_TASK_Main
 * PURPOSE: PSEC task body.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. Set the periodic timer, interval value is 1 seconds.
 *
 */
static void PSEC_TASK_Main(void)
{
    /* LOCAL CONSTANT DECLARATIONS */
#define PSEC_WAIT_MODE_CHANGE_TICKS  200  /* 2 seconds */

    /* LOCAL VARIABLES DEFINITION */
    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(PSEC_TYPE_MSG_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;

    UI32_T  ui_event_var;      /* keep track events not be handled. */
    UI32_T  ui_rcv_event;
    UI32_T  timeout;
    UI32_T  ui_return_value;
    UI32_T  current_mode;
    UI32_T  member_id;
    L_THREADGRP_Handle_T tg_handle;

    PSEC_TASK_CreateTaskMsgQ();

    /* Join the thread group
     */
    tg_handle = PSEC_TASK_GetTGHandle();
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_PORT_SECURITY_THREAD_PRIORITY, &member_id) != TRUE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.", __FUNCTION__);
        return;
    }

    ui_event_var = 0;   /* clear tracking event variable */

#if 0 /* There is no need in refined psec schema */
    tmid_psec = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(tmid_psec, SYS_BLD_PSEC_LOOKUP_INTERVAL_TICKS, PSEC_TASK_EVENT_0_5_SEC);
#endif

    memset(msg_buf, 0, sizeof(msg_buf));

    while(1)
    {
        if (ui_event_var )
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        else
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;

        ui_return_value=SYSFUN_ReceiveEvent (
            /* PSEC_TASK_EVENT_0_5_SEC |*/ PSEC_TASK_MSG_EVENT | PSEC_TASK_EVENT_ENTER_TRANSITION_MODE
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            | SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
#endif
            ,SYSFUN_EVENT_WAIT_ANY,
            timeout,
            &ui_rcv_event);

        ui_event_var |= ui_rcv_event;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (ui_event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_NETACCESS_PSEC);
            ui_event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        current_mode=PSEC_MGR_GetOperationMode();
        if ( current_mode==SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            while (SYSFUN_ReceiveMsg(psec_queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(PSEC_TYPE_MSG_T), msg_p) == SYSFUN_OK)
                ;

            if (ui_event_var &PSEC_TASK_EVENT_ENTER_TRANSITION_MODE )
                psec_task_is_transition_done = TRUE;

            ui_event_var=0;
            continue;

        }
        else if (current_mode ==SYS_TYPE_STACKING_SLAVE_MODE)
        {
            ui_event_var=0;
            continue;
        }

        if ( ui_event_var == 0)
            continue;

        /* request thread group execution permission
         */
        if (L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
        {
            SYSFUN_Debug_Printf("\r\n%s: L_THREADGRP_Execution_Request fail.", __FUNCTION__);
        }

#if 0 /* There is no need in refined psec schema */
        /*  Check timer event occurs ?  */
        if (ui_event_var & PSEC_TASK_EVENT_0_5_SEC)
        {
            PSEC_MGR_TimerExpiry();
            ui_event_var ^= PSEC_TASK_EVENT_0_5_SEC;
        }   /*  End of timer event handling */
#endif

        /*  Check message arriaved ?    */
        if (ui_event_var & PSEC_TASK_MSG_EVENT)
        {
            while (SYSFUN_ReceiveMsg(psec_queue_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(PSEC_TYPE_MSG_T), msg_p) != SYSFUN_RESULT_NO_MESSAGE)
            {
                PSEC_TASK_DispatchMsg(msg_p);
            }
            ui_event_var ^= PSEC_TASK_MSG_EVENT;

        }   /*  End of message hanlding */

        /* release thread group execution permission
         */
        if (L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
        {
            SYSFUN_Debug_Printf("\r\n%s: L_THREADGRP_Execution_Release fail.", __FUNCTION__);
        }

    }   /*  End of while(1) */

}/* End of PSEC_TASK_Main */

/* FUNCTION NAME: PSEC_TASK_DispatchMsg
 * PURPOSE: PSEC task body.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. Set the periodic timer, interval value is 1 seconds.
 *
 */
static void PSEC_TASK_DispatchMsg (SYSFUN_Msg_T *msg_p)
{
    PSEC_TYPE_MSG_T *pse_msg_p = (PSEC_TYPE_MSG_T *)msg_p->msg_buf;

    if (msg_p == 0)
        return;

    //logMsg("\n\r PSEC_TASK_DispatchMsg %d %d %ld \n\r", msg_p->msg_type, pse_msg_p->port, pse_msg_p->info);
    //logMsg("\n\r MAC:%02x:%02x:%02x:%02x:%02x:%02x \n\r", pse_msg_p->mac[0], pse_msg_p->mac[1], pse_msg_p->mac[2], pse_msg_p->mac[3], pse_msg_p->mac[4], pse_msg_p->mac[5]);

    switch (msg_p->msg_type)
    {
        case PSEC_TYPE_MSG_INTRUSION_MAC:
            PSEC_MGR_ProcessIntrusionMac(pse_msg_p);
            break;

        case PSEC_TYPE_MSG_STATION_MOVE:
            PSEC_MGR_ProcessStationMove(pse_msg_p);
            break;

        default:
            break;
    }; /* end of switch (msg->msg_type) */

    return;
} /* end of PSEC_TASK_DispatchMsg() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_TASK_IsIntrusionPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : do intrusion packet checking
 * INPUT    : da, ether_type
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTE     : None
 *-------------------------------------------------------------------------*/
static BOOL_T PSEC_TASK_IsIntrusionPacket(const UI8_T *da, UI16_T ether_type)
{
    if (NULL == da)
        return FALSE;

    /* For AMTR refinement, DA={0,0,0,0,0,0} && ether_type = 0 is meaning NA intruction*/
    if ((0 == da[0]) &&
        (0 == da[1]) &&
        (0 == da[2]) &&
        (0 == da[3]) &&
        (0 == da[4]) &&
        (0 == da[5]) &&
        (0 == ether_type))
    {
        return TRUE;
    }

    /* the following packet should not be treat as an intrusion packet */

    /* BPDU, doesn's have ether type */
    #define PSEC_ADDR_BPDU1     0x01
    #define PSEC_ADDR_BPDU2     0x80
    #define PSEC_ADDR_BPDU3     0xC2
    #define PSEC_ADDR_BPDU4     0x00
    #define PSEC_ADDR_BPDU5     0x00
    #define PSEC_ADDR_BPDU6     0x00

    if ((PSEC_ADDR_BPDU1 == da[0]) &&
        (PSEC_ADDR_BPDU2 == da[1]) &&
        (PSEC_ADDR_BPDU3 == da[2]) &&
        (PSEC_ADDR_BPDU4 == da[3]) &&
        (PSEC_ADDR_BPDU5 == da[4]) &&
        (PSEC_ADDR_BPDU6 == da[5]))
    {
        return FALSE;
    }

    /* EAP
       RADA modes replace 802.1x multicast MAC with unicast one
       so do not check da here */
    #define PSEC_EAP_FRAME_TYPE    0x888e

    if (PSEC_EAP_FRAME_TYPE == ether_type)
    {
        return FALSE;
    }

    return TRUE;
}
#endif
