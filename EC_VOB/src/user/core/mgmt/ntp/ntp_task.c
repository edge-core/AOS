
/* static char SccsId[] = "+-<>?!NTP_TASK.C  22.1  5/05/02  08:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _NTP_TASK.C
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *    This module handles all events and signal handling functions
 *
 *  Notes:
 *    None.
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *   HardSun, 2005 02 17 10:59     Created    Created
 * ------------------------------------------------------------------------
 *  Copyright(C)        Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */



/* INCLUDE FILE DECLARATIONS
 */
#include <assert.h>
#include "sys_type.h"
#include "sys_bld.h"
/* For Exceptional Handler */
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
/* end For Exceptional Handler */
#include "sysfun.h"
#include "l_stdlib.h"
#include "sys_time.h"
#include "ntp_task.h"
#include "ntp_mgr.h"
#include "ntp_type.h"
#include "ntp_dbg.h"
#include "ntp_recvbuff.h"



/* NAME CONSTANT DECLARATIONS
 */
#define NTP_SLEEP_TICK                      100     /* tickTime ntp slept */
#define NTP_TASK_ONE_SEC                    SYS_BLD_TICKS_PER_SECOND
#define NTP_TASK_ALARM_TWENTY_TICK          SYS_BLD_TICKS_PER_SECOND/5
#define NTP_TASK_EVENT_TIMER_1_SEC          BIT_2
#define NTP_TASK_EVENT_ENTER_TRANSITION     BIT_4
#define NTP_TASK_DFLT_RECEIVE_LOCAL_PORT    1024
#define NTP_TASK_INVALID_SOCKET (-1)

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/* For Exceptional Handler */
enum NTP_TASK_FUN_NO_E
{
    NTP_TASK_TASK_INIT_FUNC_NO = 1,
    NTP_TASK_TASK_CREATE_FUNC_NO
};

typedef struct NTP_TASK_LCB_S
{
    UI32_T      task_id;            /*  NTP task id */
    I32_T       socket_id;          /* socket id*/
    BOOL_T      task_created_flag;  /*  TRUE-task created, FALSE-no */
    BOOL_T      is_transition_done; /* flag for finishing clear all msg or allocation */
} NTP_TASK_LCB_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void NTP_TASK_TaskMain(void);
static BOOL_T NTP_TASK_IsProvisionComplete(void);

/* STATIC VARIABLE DECLARATIONS
 */
NTP_TASK_LCB_T  ntp_task_lcb;
static  BOOL_T  is_provision_complete = FALSE;
static  fd_set ntprecvFD;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : SInit system resource required by NTP_TASK; including message queue,
 *        memory.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void  NTP_TASK_Init (void)
{
    ntp_task_lcb.is_transition_done = FALSE;
    ntp_task_lcb.socket_id = NTP_TASK_INVALID_SOCKET;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_CreateTask
 *------------------------------------------------------------------------------
 * PURPOSE  : Create task in NTP
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. All tasks created, then call MGR, perform proper function.
 *------------------------------------------------------------------------------*/
void  NTP_TASK_CreateTask (void)
{
    /* BODY */
    /*  If NTP task is never created,
     *    Create the task and set task_created flag.
     *  else
     *    keep log in system : NTP_TASK is created twice.
     */


    if (ntp_task_lcb.task_created_flag)
    {
        /*  log message to system : NTP_TASK create twice */
        return;
    }

    if(SYSFUN_SpawnThread(SYS_BLD_NTP_CSC_THREAD_PRIORITY,
                          SYS_BLD_NTP_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_NTP_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          NTP_TASK_TaskMain,
                          NULL,
                          &(ntp_task_lcb.task_id))!=SYSFUN_OK)
    {
        ntp_task_lcb.task_id = 0;
        ntp_task_lcb.task_created_flag = FALSE;
        EH_MGR_Handle_Exception (SYS_MODULE_NTP, NTP_TASK_TASK_CREATE_FUNC_NO, EH_TYPE_MSG_TASK_CREATE, SYSLOG_LEVEL_CRIT);
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }
    else
    {
        /* Set flag in LCB, the IP_TASK resource is created */
        ntp_task_lcb.task_created_flag = TRUE;
    }
} /*  end of NTP_TASK_CreateTask  */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will tell the SSHD module to start.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This function is invoked in NTP_INIT_ProvisionComplete().
 *------------------------------------------------------------------------------*/
void NTP_TASK_ProvisionComplete(void)
{
    /* BODY */
    is_provision_complete = TRUE;
}


/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_TaskMain
 *------------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------
 */
static void NTP_TASK_TaskMain(void)
{
    UI32_T event_var;
    UI32_T wait_events;
    UI32_T rcv_events;
    UI32_T timeout;
    UI32_T ret_value;
    void   *one_sec_timer_id;

    /* Prepare waiting event and init. event var.
     */
    wait_events = NTP_TASK_EVENT_TIMER_1_SEC | NTP_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;

    /* Start one sec timer
    */
    one_sec_timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(one_sec_timer_id, NTP_TASK_ONE_SEC, NTP_TASK_EVENT_TIMER_1_SEC);

    while(1)
    {
        /* Check timer event and message event
         */
        timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        if ((ret_value=SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                                 timeout, &rcv_events))!=SYSFUN_OK)
        {
            if (ret_value != SYSFUN_RESULT_NO_EVENT_SET)
            {
                /* Log to system : unexpect return value
                 */
                ;
            }
        }
        event_var |= rcv_events;

        if (event_var == 0)
        {
            /* Log to system : ERR--Receive Event Failure
             */
            continue;
        }

        /* From 100 -> 5 Mofified by- QingfengZhang, 16 March, 2005 5:53:58
         */
        SYSFUN_Sleep(5);

        switch (NTP_MGR_GetOperationMode())
        {
            case NTP_TYPE_SYSTEM_STATE_TRANSITION :
                if (event_var & NTP_TASK_EVENT_ENTER_TRANSITION)
                {
                    ntp_task_lcb.is_transition_done = TRUE;
                }
                event_var = 0;
                is_provision_complete = FALSE;
                break;

            case NTP_TYPE_SYSTEM_STATE_MASTER :
                if (NTP_TASK_IsProvisionComplete() == FALSE)
                {
                    SYSFUN_Sleep(10);
                    break;
                }
                if (ntp_task_lcb.is_transition_done != FALSE)
                {
                    NTP_MGR_InTimeServiceMode();
                }
                break;

            case NTP_TYPE_SYSTEM_STATE_SLAVE :
                event_var = 0;
                is_provision_complete = FALSE;
                SYSFUN_Sleep(10);
                break;

            default:
                break;
        }
    }
} /* end of NTP_TASK_TaskMain */



/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/

void NTP_TASK_EnterMasterMode(void)
{

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/

void NTP_TASK_EnterSlaveMode(void)
{

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/

void NTP_TASK_EnterTransitionMode(void)
{
    /*  want task release all resources */
  SYSFUN_TASK_ENTER_TRANSITION_MODE(ntp_task_lcb.is_transition_done);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
void NTP_TASK_SetTransitionMode(void)
{
    ntp_task_lcb.is_transition_done = TRUE;
#if 0    /*  - QingfengZhang, 15 March, 2005 1:11:27 */
    ntp_task_lcb.is_transition_done = FALSE;
    SYSFUN_SendEvent (ntp_task_lcb.task_id, NTP_TASK_EVENT_ENTER_TRANSITION);
#endif
} /* end of NTP_TASK_SetTransitionMode */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_TASK_IsProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will check the NTP module can start.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 *------------------------------------------------------------------------------*/
static BOOL_T NTP_TASK_IsProvisionComplete(void)
{
    return( is_provision_complete );
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_CreateSocket
 * ------------------------------------------------------------------------
 *  FUNCTION : create ntp socket (create and bind socket)
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : TRUE is successful and FALSE is failure.
 *  NOTE     : None.
 * ------------------------------------------------------------------------
 */
BOOL_T NTP_TASK_CreateSocket(void)
{
    struct sockaddr_in srvr;

    if (ntp_task_lcb.socket_id != NTP_TASK_INVALID_SOCKET)
    {
        /* socket is already created so return immedately
         */
        return TRUE;
    }

    if ((ntp_task_lcb.socket_id = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ntp_task_lcb.socket_id = 0;
        EH_MGR_Handle_Exception(SYS_MODULE_NTP,
                                0,
                                EH_TYPE_MSG_SOCKET_OP_FAILED,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#if 0
        perror("Can't get NTP socket");
        SYSFUN_LogMsg(" NTP_TASK : Can't create socket(err=%d)\n",snmp_task_lcb->socket_id,0,0,0,0,0);
#endif
        return FALSE;
    }

    srvr.sin_family = AF_INET;
    srvr.sin_port = L_STDLIB_Hton16(NTP_TASK_DFLT_RECEIVE_LOCAL_PORT);
    srvr.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_ANY);

    if (bind(ntp_task_lcb.socket_id,(struct sockaddr*)&srvr, sizeof (srvr)) == -1)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_NTP,
                                0,
                                EH_TYPE_MSG_SOCKET_OP_FAILED,
                                (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));

        perror("Can't bind server socket");
        return  FALSE;
    }

    return TRUE;
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_CloseSocket
 * ------------------------------------------------------------------------
 *  FUNCTION : close the ntp agent socket
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------
 */
void NTP_TASK_CloseSocket(void)
{
    close(ntp_task_lcb.socket_id);
    ntp_task_lcb.socket_id = NTP_TASK_INVALID_SOCKET;
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_Get_SocketId
 * ------------------------------------------------------------------------
 *  FUNCTION : get socket id for dbg use
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
BOOL_T NTP_TASK_Get_SocketId(I32_T *sockid)
{
    if(sockid <= 0)
        return FALSE;
    *sockid = ntp_task_lcb.socket_id;
    return TRUE;
}


/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_Get_TaskId
 * ------------------------------------------------------------------------
 *  FUNCTION : get task id for dbg use
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
UI32_T NTP_TASK_Get_TaskId(void)
{
    return ntp_task_lcb.task_id;
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NTP_TASK_AddPkts_ToRecvbuff
 * ------------------------------------------------------------------------
 *  FUNCTION : add incoming packets to the recvbuff.
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------
 */
void NTP_TASK_AddPkts_ToRecvbuff(void)
{
    struct timeval tvzero;
    struct recvbuf *rb;
    int    count;
    socklen_t fromlen;

    /* Add the incoming packets to the recvbuff
     */

    FD_ZERO(&ntprecvFD);
    FD_SET(ntp_task_lcb.socket_id, &ntprecvFD);

    /* set the timeout value to 0.1sec
     */
    tvzero.tv_sec = (long) 0;
    tvzero.tv_usec =(long) 100;

    /* listen if a NTP packet is coming in
    */
    while ((count = select(ntp_task_lcb.socket_id+1, &ntprecvFD, NULL, NULL, &tvzero)) > 0)
    {
        /* A NTP packet is coming in
         */
       if (FD_ISSET(ntp_task_lcb.socket_id, &ntprecvFD))
       {
            rb = NTP_RECVBUFF_GetFreeBuffer();
            fromlen = sizeof(rb->recv_srcadr);
            rb->recv_length = recvfrom(ntp_task_lcb.socket_id,
                                       (char *)&rb->recv_pkt,
                                       sizeof(NTP_PACKET),
                                       0,
                                       (struct sockaddr *)&(rb->recv_srcadr),
                                       &fromlen);

            {
                UI32_T time=0;

                SYS_TIME_GetUTC(&time);
                rb->recv_time.Ul_i.Xl_ui = time + NTP_1900_TO_1970_SECS;
                rb->recv_time.Ul_f.Xl_uf = (time%SYS_BLD_TICKS_PER_SECOND)* 100000000;
            }

            NTP_RECVBUFF_AddRecvBuffer(rb);
        }

         /* set the timeout value to 0.1sec
         */
        tvzero.tv_sec = (long) 0;
        tvzero.tv_usec =(long) 100;
    }
}
