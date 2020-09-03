/*
 * MODULE NAME: AMTRL3_TASK.c
 *
 * PURPOSE: The address monitor Task.
 *
 * NOTES:	API List #
 *			-------------------------------------------------------------------
 *		    AMTRL3_TASK_Init()
 *          AMTRL3_TASK_CreateTask()
 *          AMTRL3_TASK_EnterTransitionMode()
 *          AMTRL3_TASK_EnterMasterMode()
 *          AMTRL3_TASK_EnterSlaveMode()
 *
 * HISTORY:
 *		Date		--	Modifier,	Reason
 *		-----------------------------------------------------------------------
 *		04-28-2003	--	hyliao,	     Design change and merge with original IPLRN
 *      01-12-2004      amytu        Redesign
 *
 * COPYRIGHT(C)			Accton Corporation, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "sys_dflt.h"
#include "amtrl3_mgr.h"
#include "amtrl3_task.h"
#include "syslog_type.h"
#include "syslog_mgr.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"



/* NAMING CONSTANT DECLARARTIONS
 */
#define AMTRL3_TASK_MSG_Q_LEN                       256
#define AMTRL3_TASK_MINUTE_TICK_INTERVAL            (60 *  SYS_BLD_TICKS_PER_SECOND) /* timer tick every minute */
#define AMTRL3_TASK_TIME_TICK_INTERVAL              (SYS_BLD_TICKS_PER_SECOND / 2)  /* timer tick every 0.5 seconds */
#define AMTRL3_TASK_COMPENSATE_NETROUTE_INDEX       8   /* Compensate net entry every (8*0.5) seconds */
#define AMTRL3_TASK_SCAN_HITBIT_INTERVAL_INDEX      20   /* Scan host route hit bit every (20*0.5) sec  */
#define AMTRL3_TASK_LAST_INTERVAL_INDEX             40
#define AMTRL3_TASK_MINUTE_INTERVAL_INDEX           120 /* Every minute is 120 ticks */
#define AMTRL3_TASK_ARP_GATEWAY_INTERVAL_INDEX      10   /* Arp gateway entry every 10 minutes   */

/* MACRO DEFINITIONS
 */

/* TYPE DEFINITIONS
 */
#define AMTRL3_TASK_TIMER_EVENT         0x00000001   /* timer event         */
#define AMTRL3_TASK_MINUTE_EVENT        0x00000002   /* minute timer event  */
#define AMTRL3_TASK_MAC_SCAN_EVENT      0x00000004   /* ARP Scan event      */
#define AMTRL3_TASK_ALL_EVENTS          (AMTRL3_TASK_TIMER_EVENT | AMTRL3_TASK_MINUTE_EVENT | AMTRL3_TASK_MAC_SCAN_EVENT)

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void     AMTRL3_TASK_Main (void);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T   amtrl3_task_tid;
//static UI32_T   amtrl3_task_timer_id;
static void*    amtrl3_task_timer_id;
static void*    amtrl3_task_minute_id;
static SYSFUN_MsgQ_T   amtrl3_task_msgQ_id;
//static UI32_T   amtrl3_task_smid;
static UI32_T   amtrl3_task_timer_index;
static UI32_T   amtrl3_task_gateway_timer;
static BOOL_T   amtrl3_task_is_transition_done;
#if (SYS_CPNT_ARP_SUPPORT_TIMEOUT == TRUE)
static BOOL_T   sync_complete_flag;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_Init(void)
{
    /* create message queue. Wait forever if fail
     */
    if (SYSFUN_CreateMsgQ(AMTRL3_TASK_MSG_Q_LEN, SYSFUN_MSG_FIFO, &amtrl3_task_msgQ_id) != SYSFUN_OK)
        while (1);

    amtrl3_task_timer_index = 0;
#if (SYS_CPNT_ARP_SUPPORT_TIMEOUT == TRUE)
    sync_complete_flag      = FALSE;
#endif
    return;

} /* End of AMTRL3_TASK_Init () */


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_CreateTask
 *------------------------------------------------------------------------
 * FUNCTION: This function will create address management task
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_CreateTask(void)
{
    if(SYSFUN_SpawnThread(SYS_BLD_AMTRL3_CSC_THREAD_PRIORITY,
                          SYS_BLD_AMTRL3_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_AMTRL3_TASK,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          AMTRL3_TASK_Main,
                          0,
                          &amtrl3_task_tid) != SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
        return;
    }

    return;
} /* end of AMTRL3_TASK_CreateTask() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will set transition state flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_SetTransitionMode(void)
{
    amtrl3_task_is_transition_done = FALSE;
    return;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_EnterTransitionMode(void)
{
    /*	wait task release all resources	*/
    SYSFUN_TASK_ENTER_TRANSITION_MODE(amtrl3_task_is_transition_done);
    //amtrl3_task_is_transition_done = TRUE;/* djd */
    return;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_EnterMasterMode(void)
{
    return;
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_EnterSlaveMode(void)
{
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_MacDeleteByLifeTimeCallbackHandler
 *------------------------------------------------------------------------
 * FUNCTION: Handler function of MAC address delete by life time callback
 * INPUT   : life_time --- which kind of life time
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_MacDeleteByLifeTimeCallbackHandler(UI32_T life_time)
{
    SYSFUN_SendEvent (amtrl3_task_tid, AMTRL3_TASK_MAC_SCAN_EVENT);
}


/* Local SubRoutines
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_Main
 * -------------------------------------------------------------------------
 * FUNCTION: This function will update ethernet address table database periodically.
 *           AMTRL3 Task monitor
 *                       Timer Event
 *                       Host Learn Event ??
 *                       Edit Addr Event ??
 *                       Delete Host Route By Net Event ??
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
static void AMTRL3_TASK_Main(void)
{

    UI32_T      wait_events, all_events = 0;
    UI32_T      current_state;
    UI32_T      fib_id, member_id;
    BOOL_T      flag;
    BOOL_T      mac_scan = FALSE;
    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetAmtrl3GroupTGHandle();

    /* join the thread group of AMTRL3 Group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_AMTRL3_CSC_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    if((amtrl3_task_timer_id = SYSFUN_PeriodicTimer_Create()) == NULL)
        printf("%s: create timer (0.5 second interval) fail.\n", __FUNCTION__);
    if((amtrl3_task_minute_id = SYSFUN_PeriodicTimer_Create()) == NULL)
        printf("%s: create timer (1 minute interval) fail.\n", __FUNCTION__);

    /* start periodic timer
     */
    /* This timer is base on 0.5 second interval */
    SYSFUN_PeriodicTimer_Start(amtrl3_task_timer_id, AMTRL3_TASK_TIME_TICK_INTERVAL, AMTRL3_TASK_TIMER_EVENT);

    /* This timer sends event every minute */
    SYSFUN_PeriodicTimer_Start(amtrl3_task_minute_id, AMTRL3_TASK_MINUTE_TICK_INTERVAL, AMTRL3_TASK_MINUTE_EVENT);

    /* Task Body
     */
    while (TRUE)
    {
        /* wait event (timer)
         */
        SYSFUN_ReceiveEvent (AMTRL3_TASK_ALL_EVENTS,
                             SYSFUN_EVENT_WAIT_ANY,
                             (all_events) ? SYSFUN_TIMEOUT_NOWAIT : SYSFUN_TIMEOUT_WAIT_FOREVER, /* timeout */
                             &wait_events);


        all_events |= wait_events;

        if(all_events == 0)
        {
            continue;
        }

        current_state = AMTRL3_MGR_GetOperationMode();

        if(current_state == SYS_TYPE_STACKING_TRANSITION_MODE)
            amtrl3_task_is_transition_done = TRUE;

        if (current_state != SYS_TYPE_STACKING_MASTER_MODE)
        {
            all_events = 0;
            continue;
        }

        if (wait_events & AMTRL3_TASK_MINUTE_EVENT)
        {
#if (SYS_CPNT_ARP_SUPPORT_TIMEOUT == TRUE)
            sync_complete_flag = FALSE;
#endif
            amtrl3_task_gateway_timer++;

            /* request thread group execution permission */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
            fib_id = AMTRL3_OM_GET_FIRST_FIB_ID;
            while(AMTRL3_MGR_GetNextFIBID(&fib_id))
            {
                /* for all FIBs, ipv4 and ipv6  */
                AMTRL3_MGR_CompensateResolvedNetRouteTable(fib_id);
            }
            /* release thread group execution permission */
            if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

            /* ARP Request Gateway entry shall have longer duration because default value for
             * gateway expiration timer is 20 minutes.  Hence, ARP request gateway action
             * only need occur every couple minutes.
             */
            if (amtrl3_task_gateway_timer == AMTRL3_TASK_ARP_GATEWAY_INTERVAL_INDEX)
            {
                amtrl3_task_gateway_timer = 0;

                /* request thread group execution permission */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                /* only one fib om, now */
                AMTRL3_MGR_RequestGatewayEntry(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, 0);
#if 0
                while(AMTRL3_MGR_GetNextFIBID(&fib_id))
                {
                    /* for all FIBs, ipv4 and ipv6  */
                    AMTRL3_MGR_RequestGatewayEntry(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id);
                }
#endif
                /* release thread group execution permission */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
            }
            all_events ^= AMTRL3_TASK_MINUTE_EVENT;
        } /* end of if */

        if (wait_events & AMTRL3_TASK_MAC_SCAN_EVENT)
        {
            mac_scan = TRUE;
            all_events ^= AMTRL3_TASK_MAC_SCAN_EVENT;
        }

        /* periodic timer event arrival
         */
        if (wait_events & AMTRL3_TASK_TIMER_EVENT)
        {
            if (amtrl3_task_timer_index < AMTRL3_TASK_LAST_INTERVAL_INDEX)
                amtrl3_task_timer_index++;
            else
                amtrl3_task_timer_index = 1;

            /* Unresolved Host entry has great impact on network stability. Hence, it is very
             * necessary to gather all routing information for each gateway entry in the shortest
             * time.
             */
            /* request thread group execution permission */
            if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

            /* only one fib om, now */
            AMTRL3_MGR_RequestUnresolvedHostEntry(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, 0);
#if 0
            while(AMTRL3_MGR_GetNextFIBID(&fib_id))
            {
                /* for all FIBs, ipv4 and ipv6  */
                AMTRL3_MGR_RequestUnresolvedHostEntry(AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6, fib_id);
            }
#endif

            /* release thread group execution permission */
            if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

            /* ARP scan
             */
            if (mac_scan == TRUE)
            {
                BOOL_T finished = TRUE;

                if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                fib_id = AMTRL3_OM_GET_FIRST_FIB_ID;
                while (AMTRL3_MGR_GetNextFIBID(&fib_id))
                {
                    if (!AMTRL3_MGR_MacScan(fib_id))
                        finished = FALSE;
                }

                if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

                if (finished)
                    mac_scan = FALSE;
            }


            /* The timer to scan host route hit bit shall be trigger by the value defined for
             * AMTRL3_TASK_SCAN_HITBIT_INTERVAL_INDEX.  However, if all host route entry has
             * been scaned, the next scanning cycle shall resume at the next minute tick.  A
             * global variabl "sync_complete_flag" is used for this purpose.  This prevents
             * excessive host route scanning operation.
             */
#if (SYS_CPNT_ARP_SUPPORT_TIMEOUT == TRUE)
            if ((amtrl3_task_timer_index % AMTRL3_TASK_SCAN_HITBIT_INTERVAL_INDEX) == 0)
            {
                if (!sync_complete_flag)
                {
                    sync_complete_flag = TRUE;

                    /* request thread group execution permission */
                    if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                        printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                    fib_id = AMTRL3_OM_GET_FIRST_FIB_ID;
                    while(AMTRL3_MGR_GetNextFIBID(&fib_id))
                    {

                        flag = AMTRL3_MGR_TimerEventHandler(fib_id);
                        if(flag == FALSE)
                            sync_complete_flag = FALSE;
                    }
                    /* release thread group execution permission */
                    if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                        printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
                }
            }
#endif
            if ((amtrl3_task_timer_index % AMTRL3_TASK_COMPENSATE_NETROUTE_INDEX) == 0)
            {
                /* request thread group execution permission */
                if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

                fib_id = AMTRL3_OM_GET_FIRST_FIB_ID;
                while(AMTRL3_MGR_GetNextFIBID(&fib_id))
                {
                    /* for all FIBs, ipv4 and ipv6  */
                    AMTRL3_MGR_CompensateNetRouteEntry(fib_id);
                }
                /* release thread group execution permission */
                if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
            }

            all_events ^= AMTRL3_TASK_TIMER_EVENT;
        } /* end of if */
    }/* end of while(TRUE) */

    return;
} /* end of AMTRL3_Task_Main () */

/* End of amtrl3_task.c */
