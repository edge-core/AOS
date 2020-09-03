/* ------------------------------------------------------------------------
 *  FILE NAME  -  SNMP_TASK.C
 * ------------------------------------------------------------------------
 * Note: None
 * ------------------------------------------------------------------------
 *  Copyright(C) Accton Technology Corporation, 2000
 * ------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */
//#include <envoy/h/buffer.h>
//#include <envoy/h/snmpdefs.h>

#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"

#include "snmp_mgr.h"
//#include "User_exits.h"
#include "snmp_task.h"
//eli #include "l_mem.h"
#include "netsnmp_port.h"
#include "syslog_type.h"
#include "syslog_mgr.h"

#if (SYS_CPNT_EH == TRUE)
#include "eh_type.h"
#include "eh_mgr.h"
#include "sys_module.h"
#endif/* end of #if (SYS_CPNT_EH == TRUE)*/

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif


#include "snmp_proc_comm.h"
#if 0
/* for socket */
#include "skt_vx.h"
#include "socket.h"
#endif

/* for net-snmp-5.0.6 */

#include <net-snmp/net-snmp-config.h>

//#include <stdio.h>
//#include <errno.h>
#if HAVE_STRING_H
//#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
//#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
//#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
//#  include <time.h>
# endif
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if HAVE_NET_IF_H
#include <net/if.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#ifdef HAVE_FCNTL_H
//#include <fcntl.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
//#include <signal.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_LIMITS_H
//#include <limits.h>
#endif
#if HAVE_PWD_H
//#include <pwd.h>
#endif
#if HAVE_GRP_H
//#include <grp.h>
#endif

#ifndef PATH_MAX
# ifdef _POSIX_PATH_MAX
#  define PATH_MAX _POSIX_PATH_MAX
# else
#  define PATH_MAX 255
# endif
#endif

#ifndef FD_SET
typedef long    fd_mask;
#define NFDBITS (sizeof(fd_mask) * NBBY)        /* bits per mask */
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((p), 0, sizeof(*(p)))
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "m2m.h"
#include <net-snmp/agent/mib_module_config.h>

#include "snmpd.h"
#include "mibgroup/struct.h"
#include <net-snmp/agent/mib_modules.h>

#include "mibgroup/util_funcs.h"

#include <net-snmp/agent/agent_trap.h>

#include <net-snmp/agent/table.h>
#include <net-snmp/agent/table_iterator.h>
#include "mib_module_includes.h"

#if(SYS_CPNT_CLUSTER==TRUE)
#include "snmp_cluster.h"
#endif /*end of #if(SYS_CPNT_CLUSTER==TRUE)*/
#include <unistd.h>

/* XXX steven.jiang for warnings */
BOOL_T SYSLOG_PMGR_AddFormatMsgEntry(SYSLOG_OM_RecordOwnerInfo_T *owner_info,
                                     UI8_T   message_index,
                                     void *  arg_0,
                                     void *  arg_1,
                                     void *  arg_2);

/*
 * Globals.
 */

static BOOL_T provision_complete;



#if defined(FTTH_OKI)
  #include "mbs.h"
#endif
extern void perror();

/* NAMEING CONSTANT
 */
#define AGENT_UDP_PORT      161

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static BOOL_T SNMP_TASK_RMON_Init(void);

/* Local Variable Declarations
 */


/* UDP_PACKET_BUFFER > SNMP_MAX_PACKET_SIZE in envoy.h */
static struct sockaddr_in  dest;

typedef struct  SNMP_TASK_LCB_S
{
    BOOL_T      init_flag;                  /* TRUE: SNMP_TASK initialized */
    UI32_T      snmp_task_id;                 /* SNMP_TASK ID               */
    I32_T       socket_id;
}   SNMP_TASK_LCB_T, *SNMP_TASK_LCB_P;

typedef struct {
  UI32_T socket_id;
  BOOL_T used;
} SNMP_TASK_InformRequestSocketInfo_T;

static  SNMP_TASK_LCB_P     snmp_task_lcb;
static SNMP_TASK_InformRequestSocketInfo_T   inform_socket_info[SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER];

static L_THREADGRP_Handle_T snmp_task_tg_handle;
static UI32_T snmp_task_member_id;

 int	                snmp_trap_socket= -1;

//static BOOL_T socket_success_init = FALSE;

UI32_T  snmp_taskId;

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Get_Socket_ID
 * ------------------------------------------------------------------------
 *  FUNCTION : Get the SNMP socket ID.
 *  INPUT    : socket_id.
 *  OUTPUT   : socket_id.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
BOOL_T  SNMP_TASK_Get_Socket_ID(I32_T *socket_id)
{
   *socket_id = snmp_task_lcb->socket_id;
   if (*socket_id < 0)
   {
      return FALSE;
   }
   else
   {
      return TRUE;
   }
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Set_Socket_ID
 * ------------------------------------------------------------------------
 *  FUNCTION : Set the SNMP socket ID variable.
 *  INPUT    : s_id
 *  OUTPUT   : None.
 *  RETURN   : True/False.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
BOOL_T SNMP_TASK_Set_Socket_ID(I32_T s_id)
{
	snmp_task_lcb->socket_id = s_id;
    return TRUE;
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME - SNMP_TASK_GetTaskId                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : Get the SNMP task ID.       
 *  INPUT    : None.                                                   
 *  OUTPUT   : id_p.                                                    
 *  RETURN   : TRUE/FALSE
 *  NOTE     : None.                                                    
 * ------------------------------------------------------------------------
 */
BOOL_T SNMP_TASK_GetTaskId(UI32_T *id_p)
{
    if (NULL == id_p)
    {
        return FALSE;
    }

    *id_p = snmp_taskId;
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_TASK_EnterTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SNMP_TASK_EnterTransitionMode()
{
    if (snmp_task_lcb->socket_id >= 0)
    {
        close(snmp_task_lcb->socket_id);
        snmp_task_lcb->socket_id= -1;
    }
        
/*    SYSFUN_TASK_ENTER_TRANSITION_MODE(snmp_task_lcb->is_transition_done); */
    return;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_TASK_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SNMP_TASK_EnterMasterMode()
{
   return;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_TASK_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SNMP_TASK_EnterSlaveMode()
{
   return;
}


/* FUNCTION NAME : SNMP_TASK_SetTransitionMode
 * PURPOSE:
 *		Sending enter transition event to task calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void SNMP_TASK_SetTransitionMode()
{
	provision_complete = FALSE;


    //snmp_task_lcb->is_transition_done = FALSE;
    return;
}






/*---------------------------------------------------------------------------+
 * Routine Name : SNMP_TASK_Init()                                           +
 *---------------------------------------------------------------------------+
 * Function : Initialize Snmp 's Task .	                                     +
 * Input    : None                                                           +
 * Output   :                                                                +
 * Return   : never returns                                                  +
 * Note     :                                                                +
 *---------------------------------------------------------------------------*/
void SNMP_TASK_Init(void)
{

    UI32_T i;

    if (snmp_task_lcb == 0)
    {
        if ((snmp_task_lcb = (SNMP_TASK_LCB_P) malloc (sizeof(SNMP_TASK_LCB_T)))==NULL)
            return;
        else
        {
            memset ((char*)snmp_task_lcb, 0, sizeof(SNMP_TASK_LCB_T));
        }
    }

    snmp_task_lcb->socket_id = -1;

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        inform_socket_info[i].used = FALSE;
        inform_socket_info[i].socket_id= -1;
    }

    (void) memset(&dest, 0, sizeof (dest));


    /*
     * start library
     */

     snmp_task_lcb->init_flag = TRUE;

}

/*---------------------------------------------------------------------------+
 * Routine Name : SNMP_TASK_CreateSnmpTask()                                 +
 *---------------------------------------------------------------------------+
 * Function : Create and start SNMP task	                                 +
 * Input    : None						                                     +
 * Output   :                                                                +
 * Return   : never returns                                                  +
 * Note     :                                                                +
 *---------------------------------------------------------------------------*/
void SNMP_TASK_CreateSnmpTask(void)
{
    SYSLOG_OM_RecordOwnerInfo_T   owner_info;
    UI32_T thread_id;

    /* create a thread for CSCA task only when it needs to take care of timer
     * event. Thread group handle will be passed to CSCA_Task_Main as an
     * argument. CSCA_Task_Main must call L_THREADGRP_Join() to join the thread
     * group at the beginning of CSCA_Task_Main.
     */
    if(SYSFUN_SpawnThread(SYS_BLD_SNMP_CSC_THREAD_PRIORITY,
                          SYS_BLD_SNMP_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_SNMP_CSC_THREAD_NAME,
#ifdef ES3526MA_POE_7LF_LN
                          (SYS_BLD_TASK_COMMON_STACK_SIZE * 6),
#else
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
#endif
                          SYSFUN_TASK_FP,
                          SNMP_TASK_Body,
                          NULL,
                          &thread_id)!=SYSFUN_OK)
    {
        owner_info.level        = SYSLOG_LEVEL_CRIT;
        owner_info.module_no    = SYSLOG_MODULE_APP_SNMP;
        owner_info.function_no  = SNMP_TASK_LOG_FUN_CREATE_TASK;
        owner_info.error_no     = SNMP_TASK_LOG_ERR_CREATE_TASK;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, "SNMP_TASK", 0, 0);
        SYSFUN_Debug_Printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread ( SW_WATCHDOG_SNMP, thread_id, SYS_ADPT_SNMP_SW_WATCHDOG_TIMER );
#endif

    snmp_taskId = thread_id;
} /* End of SNMP_TASK_CreateSnmpTask() */
#if 0
void SNMP_TASK_CreateSnmpTask(void)
{
    #if (SYS_CPNT_EH == TRUE)
        UI32_T module_id;
        UI32_T function_no;
        UI32_T msg_flag;
        UI8_T  ehmsg[256];
    #endif//end of #if (SYS_CPNT_EH == TRUE)

    SYSLOG_OM_RecordOwnerInfo_T   owner_info;

    if (SYSFUN_SpawnTask (SYS_BLD_SNMP_TASK,
                        SYS_BLD_SNMP_TASK_PRIORITY,
                        SYS_BLD_SNMP_V3_TASK_STACK_SIZE,
                        SYSFUN_TASK_NO_FP,
                        SNMP_TASK_Body,
                        0,
                        &snmp_task_lcb->snmp_task_id) != SYSFUN_OK )

    {
        #if (SYS_CPNT_EH == TRUE)
           EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
           SYSFUN_Debug_Printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        #endif//end of #if (SYS_CPNT_EH == TRUE)

        owner_info.level        = SYSLOG_LEVEL_CRIT;
        owner_info.module_no    = SYSLOG_MODULE_APP_SNMP;
        owner_info.function_no  = SNMP_TASK_LOG_FUN_CREATE_TASK;
        owner_info.error_no     = SNMP_TASK_LOG_ERR_CREATE_TASK;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, "SNMP_TASK", 0, 0);

    }

    snmp_taskId = snmp_task_lcb->snmp_task_id;
} /* End of SNMP_TASK_CreateSnmpTask() */
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_TASK_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: This function is for SNMP provision complete.
 *
 * INPUT:  none
 * OUTPUT: none
 * RETURN: none;
 * NOTES:1.none
 *
 * ---------------------------------------------------------------------
 */
void SNMP_TASK_ProvisionComplete()
{
#if 0
    /*
    EPR_ID:ES3628BT-FLF-ZZ-00184,ES4827G-FLF-ZZ-00476
    Problem: RMON:Slave unit ethernet history counter not work.
    Root Cause: Statistics,history,alarm and event didn't update their table,When the hot insertion and removal.
    Solution: Update their data table when provision complete.
    Modified files:
	    src\user\apps\snmpmgmt\v3\snmp_task.c
    Approved by: Tiger Liu
    */
    STATISTICS_DeleteAllRow();
    STATISTICS_CreateDefaultEntry();
    HISTORY_DeleteAllRow();
    HISTORY_CreateDefaultEntry();
    ALARM_DeleteAllRow();
    ALARM_CreateDefaultEntry();
    EVENT_DeleteAllRow();
    EVENT_CreateDefaultEntry();
#endif
    provision_complete = TRUE;
}


/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Socket_Init
 * ------------------------------------------------------------------------
 *  FUNCTION : initialize SNMP agent socket (create and bind socket ...)
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
BOOL_T SNMP_TASK_Socket_Init(void)
{
    UI32_T max_msg_size;
    I32_T rc;
    struct sockaddr_in srvr;
    fd_set ready;
    struct timeval timeout;
    #if (SYS_CPNT_EH == TRUE)
        UI32_T module_id;
        UI32_T function_no;
        UI32_T msg_flag;
        UI8_T  ehmsg[256];
    #endif//end of #if (SYS_CPNT_EH == TRUE)

    /*eli,we should check socket_id for avoiding to access the invalid memory*/
    if(snmp_task_lcb->socket_id != -1)
    {
        FD_ZERO(&ready);
    	FD_SET(snmp_task_lcb->socket_id, &ready);
        timeout.tv_sec = 1;
    	timeout.tv_usec = 0;
    	rc=select(snmp_task_lcb->socket_id+1, &ready, &ready, NULL, &timeout);
        if (rc >= 1)
        {
        	return TRUE;//socket already open, don't need to open anymore
    	}
        close(snmp_task_lcb->socket_id);
    }

    if ((snmp_task_lcb->socket_id = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        #if (SYS_CPNT_EH == TRUE)
           EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
           SYSFUN_Debug_Printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        #endif//end of #if (SYS_CPNT_EH == TRUE)
        return FALSE;
    }

    /* allow snmp to send udp packets to 8KB max.
     */
    max_msg_size = 8 * 1024;
    setsockopt(snmp_task_lcb->socket_id, SOL_SOCKET, SO_SNDLOWAT, &max_msg_size, sizeof(max_msg_size));

    srvr.sin_family = PF_INET;
    srvr.sin_port = htons(AGENT_UDP_PORT);
    srvr.sin_addr.s_addr = 0L;

    if (bind(snmp_task_lcb->socket_id,(struct sockaddr*)&srvr, sizeof (srvr)) == -1)
    {
        #if (SYS_CPNT_EH == TRUE)
           EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
           SYSFUN_Debug_Printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        #endif//end of #if (SYS_CPNT_EH == TRUE)
            perror("Can't bind server socket");
            return  FALSE;
    }

    return TRUE;
}

BOOL_T SNMP_TASK_Marked_Inform_Request_Socket( UI32_T inform_sock_num, BOOL_T flag)
{
    int i;
    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if (inform_socket_info[i].socket_id == inform_sock_num)
        {
            inform_socket_info[i].used = flag;
            return TRUE;
        }
    }
    return FALSE;
}


I32_T SNMP_TASK_GetAvailableInformSocket()
{

     int i;

     for (i = 0; i < SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
     {
        if (!inform_socket_info[i].used)
        {
            inform_socket_info[i].used = TRUE;
            return inform_socket_info[i].socket_id;
        }
     }
     return -1;
}


/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Inform_Request_Socket_Init
 * ------------------------------------------------------------------------
 *  FUNCTION : initialize SNMP trap socket (create and bind socket ...)
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ------------------------------------------------------------------------*/
BOOL_T SNMP_TASK_Inform_Request_Socket_Init(void)
{
    struct sockaddr_in srvr;
    fd_set ready;
    I32_T rc;
    struct timeval timeout;
    int i = 0;

    FD_ZERO(&ready);

    if(inform_socket_info[i].socket_id!=-1)
    {
    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
	    FD_SET(inform_socket_info[i].socket_id, &ready);
	}

    timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	for (i = 0; i < SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
	{
	    rc=select(inform_socket_info[i].socket_id+1, &ready, &ready, NULL, &timeout);
        if (rc >= 1)
        {
            /* suppose one socket init mean all socket all init*/
    	    return TRUE;//socket already open, don't need to open anymore
	    }
	}
    }

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if (inform_socket_info[i].socket_id >= 0)
            close(inform_socket_info[i].socket_id);
    
        inform_socket_info[i].used = FALSE;
        if ((inform_socket_info[i].socket_id= socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        {
            return FALSE;
        }
    }
    /*SYSFUN_Debug_Printf("Inform Request Socket_init success\n");*/

    /* binding the socket*/
    for ( i = 0; i < SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        srvr.sin_family = PF_INET;
        srvr.sin_port = htons(SNMP_MGR_INFORM_PORT+i);
        srvr.sin_addr.s_addr = 0L;
        if (bind(inform_socket_info[i].socket_id, (struct sockaddr*)&srvr, sizeof (srvr)) == -1)
        {
            perror("Can't bind server socket");
            return  FALSE;
        }
    }
    
    return TRUE;
}
/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_RMON_Init
 * ------------------------------------------------------------------------
 *  FUNCTION : SNMP_TASK_RMON_Init
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
static BOOL_T SNMP_TASK_RMON_Init(void)
{
    BOOL_T flag;
    SNMP_MGR_GetRmonInitFlag(&flag);
    if (flag == TRUE)
    {
        return TRUE;
    }
    else
    {
        STATISTICS_DeleteAllRow();
        STATISTICS_CreateDefaultEntry();

        HISTORY_DeleteAllRow();
        HISTORY_CreateDefaultEntry();

#if (SYS_CPNT_RMON_ALARM_DEFAULT == TRUE)
        ALARM_DeleteAllRow();
        ALARM_CreateDefaultEntry();
#endif /* #if (SYS_CPNT_RMON_ALARM_DEFAULT == TRUE) */

        EVENT_DeleteAllRow();
        EVENT_CreateDefaultEntry();

        SNMP_MGR_SetRmonInitFlag(TRUE);
        return TRUE;
    }
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Body
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
void SNMP_TASK_Body()
{
    int ret;
    BOOL_T initMasterAgentFlag=FALSE;
    SYS_TYPE_Stacking_Mode_T current_mode=SYS_TYPE_STACKING_TRANSITION_MODE;
    BOOL_T status;

    snmp_task_tg_handle = SNMP_PROC_COMM_GetSnmpGroupTGHandle();


    /* join the thread group
     */
    if(L_THREADGRP_Join(snmp_task_tg_handle, SYS_BLD_SNMP_CSC_THREAD_PRIORITY, &snmp_task_member_id)==FALSE)
    {
        SYSFUN_Debug_Printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while(snmp_task_lcb->init_flag == FALSE) /* snmp_task not initial */
    {
        SYSFUN_Debug_Printf(" snmp task not init\n");
        SYSFUN_Sleep(100);
    }


    /*  Your main loop here...  */
    while (1)
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        SNMP_MGR_CheckSoftwareWatchdogEvent();
#endif

        /* if you use select(), see snmp_api(3) */
        /*     --- OR ---  */

        /* request thread group execution permission
         */
        SNMP_TASK_ThreadgrpExecutionRequest();

#if(SYS_CPNT_CLUSTER==TRUE)
        SNMP_CLUSTER_CheckRoleChange();
#endif
        current_mode = SNMP_MGR_GetOperationMode();

		if ((current_mode) == SYS_TYPE_STACKING_SLAVE_MODE)
		{
			SNMP_TASK_ThreadgrpExecutionRelease();
			SYSFUN_Sleep(100);
			continue;
		}

        if (current_mode == SYS_TYPE_STACKING_MASTER_MODE)
        {
            if (!initMasterAgentFlag)
            {
                if ((ret = init_master_agent()) != 0)
                {
                    SNMP_TASK_ThreadgrpExecutionRelease();
                    /*
                     * Some error opening one of the specified agent transports.
                     */
                    SYSFUN_Sleep(200);
                    continue;
                }
                initMasterAgentFlag=TRUE;
            }

            SNMP_MGR_Get_AgentStatus(&status);
            if (!status)
            {
                /* The logs should be logged into syslog even agent is disable,
                 * because SNMP is the entry for both SNMP trap and syslog.
                 */            
                SNMP_MGR_CheckTrapEvent();
                SNMP_TASK_ThreadgrpExecutionRelease();
      	        SYSFUN_Sleep(200);
                continue;
            }

            /* enter transition mode will close v4 socket,
             * need to bind v4 socket in master mode
             */
            if (!SNMP_TASK_Socket_Init())
            {
                SNMP_TASK_ThreadgrpExecutionRelease();
	            SYSFUN_Sleep(200);
                continue;
            }

            if (!SNMP_TASK_Inform_Request_Socket_Init())
            {
                SNMP_TASK_ThreadgrpExecutionRelease();
                SYSFUN_Sleep(200);
                continue;
            }

            SNMP_TASK_RMON_Init();

            agent_check_and_process(0);
        }/* end of if (current_mode == SYS_TYPE_STACKING_MASTER_MODE)*/

        SNMP_MGR_CheckTrapEvent();

        SNMP_TASK_ThreadgrpExecutionRelease();

        if (current_mode == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            SYSFUN_Sleep(200);
        }
    }/* end of while(1)*/
} /* End of SNMP_TASK_Body() */


#if 0
/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_CreateTrapSocket
 * ---------------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT	 : None.
 *  OUTPUT	 : None.
 *  RETURN	 : None.
 *  NOTE	 : None.
 * ---------------------------------------------------------------------------
 */
static void SNMP_TASK_CreateTrapSocket(void)
{

    struct sockaddr_in   srvr;


    /* BODY */

    if ((snmp_trap_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
     #if (SYS_CPNT_EH == FALSE)
        SYSLOG_OM_RecordOwnerInfo_T       owner_info;
        owner_info.level = SYSLOG_LEVEL_ERR;
        owner_info.module_no = SYSLOG_MODULE_APP_SNMP;
        owner_info.function_no = 1;
        owner_info.error_no = 1;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, FUNCTION_RETURN_FAIL_INDEX, "SNMP_TASK_CreateTrapSocket", 0, 0);
        return;
     #else
     /* should implement EH handler in the furture, kinghong*/
     #endif

    } /* End of if */


    srvr.sin_family = AF_INET;
    srvr.sin_port = htons(162);
    srvr.sin_addr = 0L;
    /* Remove by amytu 3-29-2002 */

    if (bind(snmp_trap_socket, (struct sockaddr*)&srvr, sizeof (srvr)) == -1)
    {
    #if (SYS_CPNT_EH == FALSE)
        SYSLOG_OM_RecordOwnerInfo_T       owner_info;
        owner_info.level = SYSLOG_LEVEL_ERR;
        owner_info.module_no = SYSLOG_MODULE_APP_SNMP;
        owner_info.function_no = 1;
        owner_info.error_no = 1;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, FUNCTION_RETURN_FAIL_INDEX, "Can't bind trap socket", 0, 0);
        return;
     #else
     /* should implement EH handler in the furture, kinghong*/
     #endif
    } /* End of if */


    return;

} /* end of SNMP_TASK_CreateTrapSocket() */
#endif

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_ThreadgrpExecutionRequest
 * ------------------------------------------------------------------------
 *  FUNCTION : THREADGRP Execution Request for snmp task
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
void SNMP_TASK_ThreadgrpExecutionRequest()
{
    if(L_THREADGRP_Execution_Request(snmp_task_tg_handle, snmp_task_member_id)!=TRUE)
    {
        SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
    }
}

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  -  THREADGRP Execution Release for snmp task
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
void SNMP_TASK_ThreadgrpExecutionRelease()
{
    if(L_THREADGRP_Execution_Release(snmp_task_tg_handle, snmp_task_member_id)!=TRUE)
    {
        SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
    }
}


