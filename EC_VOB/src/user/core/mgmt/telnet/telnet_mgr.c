/* MODULE NAME:  TELNET_mgr.c
* PURPOSE: 
*   Initialize the resource and provide some functions for the TELNET module.
*
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-11         -- jason wang created.
*   
* Copyright(C)      Accton Corporation, 2002
*/


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include <sys_type.h>
#include <fcntl.h>
#include "sysfun.h"
#include "telnet_mgr.h"
#include "telnet_om.h"
//rich#include "telnet_om_private.h"
#include "telnetd.h"
#include "l_stdlib.h"
#include "ip_lib.h"
#include "netcfg_pmgr_main.h"

#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_pom.h"
#include "sys_bld.h"
#include <string.h>
#include "cli_pom.h"
#endif

#if 0 /* rich */
#include "iproute.h"
/*  for socket.h include.   */
#include "skt_port.h"
#include "queue.h"          /*  for TAILQ_HEAD                  */
#include "socket.h"
#include "sockvar.h"        /*  for STK_MSG                     */

/*  2001.10.28, William, includes following files for porting to VxWorks and
 *              try to clear up the dependence.
 */
#include "ip_cmn.h"         /*  for IP_CMN_GetMode() replace P2INIT_P2MG_Task_Start()    */
#include "netcfg.h"
#include "ip_task.h"
#include "skt_vx.h"
#include "sock_port.h"
#endif

#if ((SYS_CPNT_TELNET == TRUE) || (SYS_CPNT_MGMT_IP_FLT == TRUE))
#include "cli_pmgr.h"
#endif /* #if ((SYS_CPNT_TELNET == TRUE) || (SYS_CPNT_MGMT_IP_FLT == TRUE)) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

#define TELNET_MGR_CLUSTER_MAX_RETRY_TIME    3
#define TELNET_MGR_CLUSTER_MAX_IDLE_TIME    180 /* in second */
#define TELNET_MGR_CLUSTER_MAX_TRY_COUNT   2

/* DATA TYPE DECLARATIONS
 */

/* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
BOOL_T TELNET_MGR_GetTnpdOptions (TELNET_Options_T *options);

/* LOCAL SUBPROGRAM DECLARATIONS 
 */

SYSFUN_DECLARE_CSC


/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for telnet mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    TELNET_MGR_IPCMsg_T *mgr_msg_p;
    //UI32_T  telnetPort;

    if(ipcmsg_p==NULL)
        return FALSE;

    mgr_msg_p = (TELNET_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        mgr_msg_p->type.result = FALSE;
        ipcmsg_p->msg_size = TELNET_MGR_MSGBUF_TYPE_SIZE;
        return TRUE;
    }

    switch(mgr_msg_p->type.cmd)
    {
        case TELNET_MGR_IPC_SET_TNPD_PORT:
            mgr_msg_p->type.result = TELNET_MGR_SetTnpdPort(mgr_msg_p->data.port_data.port);
            ipcmsg_p->msg_size=TELNET_MGR_MSGBUF_TYPE_SIZE;
            break;

        case TELNET_MGR_IPC_GET_TNPD_PORT:
            mgr_msg_p->type.result = TELNET_MGR_GetTnpdPort(&(mgr_msg_p->data.port_data.port));
            ipcmsg_p->msg_size = TELNET_MGR_GET_MSGBUFSIZE(port_data);
            break;

        case TELNET_MGR_IPC_SET_TNPD_STATUS:
            mgr_msg_p->type.result = TELNET_MGR_SetTnpdStatus(mgr_msg_p->data.status_data.state);
            ipcmsg_p->msg_size=TELNET_MGR_MSGBUF_TYPE_SIZE;
            break;

        case TELNET_MGR_IPC_GET_TNPD_STATUS:
            mgr_msg_p->data.status_data.state = TELNET_MGR_GetTnpdStatus();
            mgr_msg_p->type.result = TRUE;
            ipcmsg_p->msg_size = TELNET_MGR_GET_MSGBUFSIZE(status_data);
            break;

        case TELNET_MGR_IPC_SET_TNPD_MAX_SESSION:
            mgr_msg_p->type.result = TELNET_MGR_SetTnpdMaxSession(mgr_msg_p->data.max_session_data.maxSession);
            ipcmsg_p->msg_size=TELNET_MGR_MSGBUF_TYPE_SIZE;
            break;

        case TELNET_MGR_IPC_GET_TNPD_MAX_SESSION:
            mgr_msg_p->data.max_session_data.maxSession = TELNET_MGR_GetTnpdMaxSession();
            mgr_msg_p->type.result = TRUE;
            ipcmsg_p->msg_size = TELNET_MGR_GET_MSGBUFSIZE(max_session_data);
            break;

        /* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
        case TELNET_MGR_IPC_GET_TNPD_OPTIONS:
            mgr_msg_p->type.result = TELNET_MGR_GetTnpdOptions(&mgr_msg_p->data.options_data);
            ipcmsg_p->msg_size=TELNET_MGR_MSGBUF_TYPE_SIZE;
            break;

#if (SYS_CPNT_CLUSTER == TRUE)
        case TELNET_MGR_IPC_SET_TELNET_RELAYING:
            mgr_msg_p->type.result = TELNET_MGR_SetTelnetRelaying(
                mgr_msg_p->data.cluster_relay_data.task_id,
                mgr_msg_p->data.cluster_relay_data.bRelaying,
                mgr_msg_p->data.cluster_relay_data.memberId);
            ipcmsg_p->msg_size=TELNET_MGR_MSGBUF_TYPE_SIZE;
            break;

        case TELNET_MGR_IPC_CLUSTER_TO_MEMBER_FROM_UART:
            mgr_msg_p->type.result = TELNET_MGR_ClusterToMemberFromUART(
                mgr_msg_p->data.cluster_member_data.memberId);
            ipcmsg_p->msg_size=TELNET_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif  /* #if (SYS_CPNT_CLUSTER == TRUE) */

        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            mgr_msg_p->type.result = FALSE;
            ipcmsg_p->msg_size = TELNET_MGR_MSGBUF_TYPE_SIZE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TELNET_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TELNET_MGR_Create_InterCSC_Relation(void)
{
    return;
} /* end of TELNET_MGR_Create_InterCSC_Relation */

/* FUNCTION NAME:  TELNET_MGR_Enter_Master_Mode
 * PURPOSE: 
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the TELNET subsystem will enter the
 *          Master Operation mode.                                                            
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *				switch will be initiated to the factory default value.
 *          2. TELNET will handle network requests only when this subsystem
 *				is in the Master Operation mode 
 *          3. This function is invoked in TELNET_INIT_EnterMasterMode.
 */
BOOL_T TELNET_MGR_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    
    /* BODY */
	/* init database */

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();
	return TRUE;
}

/* FUNCTION NAME:  TELNET_MGR_Enter_Transition_Mode
 * PURPOSE: 
 *          This function forces this subsystem enter the Transition Operation mode.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *			.
 */
BOOL_T TELNET_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    /* set mgr in transition mode */
    SYSFUN_ENTER_TRANSITION_MODE();
	return TRUE;
}

/* FUNCTION NAME:  TELNET_MGR_Enter_Slave_Mode
 * PURPOSE: 
 *          This function forces this subsystem enter the Slave Operation mode.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          In Slave Operation mode, any network requests 
 *          will be ignored.
 */
void TELNET_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SYSFUN_ENTER_SLAVE_MODE();

	return;	
} 

/* FUNCTION NAME : TELNET_MGR_SetTransitionMode
 * PURPOSE:
 *		Set transition mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void TELNET_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
     
    /* LOCAL VARIABLES DEFINITION
     */
     
    /* BODY */
    
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}

/* FUNCTION NAME : TELNET_MGR_GetOperationMode
 * PURPOSE:
 *      Get current TELNET operation mode (master / slave / transition).
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *		None.
 */
SYS_TYPE_Stacking_Mode_T TELNET_MGR_GetOperationMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    
    /* LOCAL VARIABLES DECLARATIONS 
     */

    /* BODY */
    
    return ( SYSFUN_GET_CSC_OPERATING_MODE() );
}

/* FUNCTION NAME:  TELNET_MGR_GetTnpdStatus
 * PURPOSE: 
 *          This function get TELNET state.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TELNET_State_T - TELNET status.
 * NOTES:
 *          .
 */
TELNET_State_T TELNET_MGR_GetTnpdStatus(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    TELNET_State_T status;

    /* BODY */
    TELNET_OM_GetTnpdStatus(&status);
    return (status);
}

/* FUNCTION NAME:  TELNET_MGR_SetTnpdMaxSession
 * PURPOSE: 
 *          This function set TELNET max session number.
 *
 * INPUT:   
 *          max session number.
 *
 * OUTPUT:  
 *          none.
 *
 * RETURN:  
 *          TRUE  -- successful
 *          FALSE -- failed
 *
 * NOTES:
 *
 */
BOOL_T TELNET_MGR_SetTnpdMaxSession(UI32_T maxSession)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    return TELNET_OM_SetTnpdMaxSession(maxSession);
}

/* FUNCTION NAME:  TELNET_MGR_GetTnpdMaxSession
 * PURPOSE: 
 *          This function get TELNET max session number.
 *
 * INPUT:   
 *          none.
 *
 * OUTPUT:  
 *          none.
 *
 * RETURN:  
 *          max session number.
 *
 * NOTES:
 *
 */
UI32_T TELNET_MGR_GetTnpdMaxSession()
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    return TELNET_OM_GetTnpdMaxSession();
}

/* FUNCTION NAME:  TELNET_MGR_GetRunningTnpdMaxSession
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific TELNET max sessions number with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *
 * OUTPUT:  
 *          UI32_T *pMaxSession -- TELNET max session number.
 *
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 *
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 *
 */
SYS_TYPE_Get_Running_Cfg_T TELNET_MGR_GetRunningTnpdMaxSession(UI32_T *pMaxSession)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
    *pMaxSession = TELNET_MGR_GetTnpdMaxSession();
	if (*pMaxSession != SYS_DFLT_TELNET_DEFAULT_MAX_SESSION)
	{
		return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
	}

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* FUNCTION NAME:  TELNET_MGR_SetTnpdStatus
 * PURPOSE: 
 *          This function set TELNET state.
 *
 * INPUT:   
 *          TELNET_State_T - TELNET status.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 *          .
 */
BOOL_T TELNET_MGR_SetTnpdStatus (TELNET_State_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    
    /* BODY */
    return TELNET_OM_SetTnpdStatus(state);

}

/* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
BOOL_T TELNET_MGR_GetTnpdOptions (TELNET_Options_T *options)
{
    return TNPD_GetOptions(options->remote_port, options->myopts, options->hisopts);
}

/* FUNCTION NAME:  TELNET_MGR_GetRunningTnpdStatus
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific TELNET Status with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - TELNET Status.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  TELNET_MGR_GetRunningTnpdStatus(UI32_T *state)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
    *state = TELNET_MGR_GetTnpdStatus();
	if ( *state != SYS_DFLT_TELNET_DEFAULT_STATE )
	{
		return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
	}
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* FUNCTION NAME:  TELNET_MGR_GetTnpdPort
 * PURPOSE: 
 *          This function get TELNET port number.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          UI32_T  *port   --  TELNET port number.
 *                                   
 * RETURN:  
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T TELNET_MGR_GetTnpdPort(UI32_T *port_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */    
    
    /* LOCAL VARIABLES DECLARATIONS 
    */
    
    /* BODY */
    return TELNET_OM_GetTnpdPort(port_p);
}

/* FUNCTION NAME:  TELNET_MGR_SetTnpdPort
 * PURPOSE: 
 *          This function set TELNET port number.
 *
 * INPUT:   
 *          UI32_T  port    --  TELNET port number.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE    --  success.  
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T TELNET_MGR_SetTnpdPort(UI32_T port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    UI32_T           old_port;

    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    if( port<1 || port>65535 )
    {
        return FALSE;
    }

    if((TELNET_OM_GetTnpdPort(&old_port) == TRUE) &&
       ( old_port != port ))
    {
        if (TRUE != IP_LIB_IsValidSocketPortForServerListen((UI16_T)port, IP_LIB_SKTTYPE_TCP, IP_LIB_CHKAPP_TELNET))
        {
            return FALSE;
        }

        /* chao 2004/10/19 
           ES4549-08-00296 - to avoid user setting a port 
           which is used by other components.     
        */
        if(NETCFG_PMGR_MAIN_IsEmbeddedTcpPort(port))        
        {
            return FALSE;        
        }
        if(TELNET_OM_SetTnpdPort(port) == TRUE)        
            TNPD_PortChange();
    }
    
	return TRUE;
}

/* FUNCTION NAME:  TELNET_MGR_GetRunningTnpdPort
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific TELNET port number with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T *port -- TELNET port number.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  TELNET_MGR_GetRunningTnpdPort(UI32_T *port)
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS 
	*/

	/* BODY */
    TELNET_MGR_GetTnpdPort(port);
	if ( *port != SYS_DFLT_TELNET_SOCKET_PORT )
	{
		return SYS_TYPE_GET_RUNNING_CFG_SUCCESS ;
	}
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/* FUNCTION NAME - TELNET_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void TELNET_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    return;
}

/* FUNCTION NAME - TELNET_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void TELNET_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    return;
}

#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME : TELNET_MGR_SetTelnetRelaying
  *PURPOSE:
 *        This function used for commander connect to member's telnet service from telnet
 *INPUT:
 *        UI32_T task_id, BOOL_T bRelaying, UI8_T memberId
 *OUTPUT:
 *        none.
 *RETURN:
 *       TRUE    --  success.
 *       FALSE   --  failure.
 */
BOOL_T TELNET_MGR_SetTelnetRelaying(UI32_T task_id, BOOL_T bRelaying, UI8_T memberId)
{
    return TNPD_SetTelnetRelaying(task_id, bRelaying, memberId);
}

/*FUNCTION NAME: TELNET_MGR_ClusterToMemberFromUART
  *PURPOSE:
 *        This function used for commander connect to member's telnet service from console
 *INPUT:
 *        UI8_T member_id --member id.
 *OUTPUT:  
 *        none.
 *RETURN:
 *       TRUE    --  success.
 *       FALSE   --  failure. 
  */
BOOL_T TELNET_MGR_ClusterToMemberFromUART(UI8_T member_id)
{
    struct sockaddr_in   sin;
    int                  sockfd, on = 1;
    int                  try_count = 0;
    int                  ret = 0;
    int                  conn_retry_count;
    int                  ch_count;
    char                 ch;
    UI32_T               start_ticks;
    UI8_T                ch_str_ar[2]={0};
    UI8_T                member_ip_ar[4];
    UI32_T               i_member_ip;
    char                 p_buf_ar[2048];
    UI32_T               uart_handler;
    int                  f_status;

    memset(&member_ip_ar, 0, sizeof(member_ip_ar));
    /* Get member IP Address
     */
    if (FALSE == CLUSTER_POM_MemberIdToIp(member_id,member_ip_ar))
    {
        return FALSE;
    }

    memcpy(&i_member_ip,member_ip_ar,sizeof(UI32_T));
    sockfd = socket(PF_INET , SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        TELNETD_ErrorPrintf("\naborted telnet_relay_action on socket(),sockfd:[%d]\n",sockfd);
        s_close(sockfd);
        return FALSE;
    }

    memset(&sin, 0, sizeof (sin));
    sin.sin_addr.s_addr = L_STDLIB_Hton32(i_member_ip);/*htonl(member_ip);*/
    sin.sin_family = AF_INET;
    sin.sin_port   = L_STDLIB_Hton16(SYS_DFLT_TELNET_SOCKET_PORT); /* should get from slave */

    for(conn_retry_count = 0; conn_retry_count < TELNET_MGR_CLUSTER_MAX_RETRY_TIME; conn_retry_count++)
    {
        if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
        {
            break;
        }
        SYSFUN_Sleep(1<<6);
    }

    if(conn_retry_count ==TELNET_MGR_CLUSTER_MAX_RETRY_TIME)
    {
        s_close(sockfd);
        return FALSE;
    }
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
    /* set the non-blocking mode of socket connect to slave.
     */
    f_status = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL,f_status | O_NONBLOCK);

    /* keep the time for auto logout
     */
    start_ticks = SYSFUN_GetSysTick();
    CLI_POM_GetUartHandler(&uart_handler);

    while(1)
    {
        memset(p_buf_ar,0,sizeof(p_buf_ar));
        ret = recv(sockfd,p_buf_ar,sizeof(p_buf_ar),0);

        /* output any character which receive from member
         */
        if(ret >0)
        {
            for(ch_count =0;ch_count<ret;ch_count++)
                printf("%c",p_buf_ar[ch_count]);
            fflush(stdout);
            SYSFUN_Sleep(1);
            start_ticks = SYSFUN_GetSysTick();
            continue;
        }

        if(ret <= 0 && errno!= EWOULDBLOCK)
        {
            s_close(sockfd);
            break;
        }

        if(ret != 0 &&  errno == EWOULDBLOCK)
        {
            /* auto logout after TELNET_MGR_CLUSTER_MAX_IDLE_TIME seconds if not key in any word
             */
            if(SYSFUN_GetSysTick() - start_ticks > TELNET_MGR_CLUSTER_MAX_IDLE_TIME * SYS_BLD_TICKS_PER_SECOND)
            {
                s_close(sockfd);
                break;
            }

            /* all data maybe not receive by one time,so receive more time
             */
            else if( try_count < TELNET_MGR_CLUSTER_MAX_TRY_COUNT)
            {
                SYSFUN_Sleep(1);
                try_count += 1;
                continue;
            }
        }

        try_count = 0;

        SYSFUN_Sleep(1);

         /* user input
          */
        if(0 < SYSFUN_UARTPollRxBuff(uart_handler, 1, &ch))
        {
            /* refer cli code to filter (-1) and (0xff)*/
            if( ch!=(0xff) )
            {
                ch_str_ar[0]=ch;
                ret = send(sockfd, ch_str_ar,1,0);
                SYSFUN_Sleep(1);
            }
        }
    } /* end while */

    return TRUE;
}
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

#if (SYS_CPNT_TELNET == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_MGR_RifDestroyedCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback to Telnet if rip destroy (IP address is changed)
 *
 * INPUT:
 *    ip_addr_p  -- the ip address which is destroyed
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void TELNET_MGR_RifDestroyedCallbackHandler(L_INET_AddrIp_T *ip_addr_p)
{
    TNPD_SendSignalToSessions(ip_addr_p);
}
#endif /* #if (SYS_CPNT_TELNET == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_MGR_MgmtIPFltChangedCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback to Telnet if the database of mgmt IP filter was changed
 *
 * INPUT:
 *    mode  --  which mode of mgmt IP filter was changed
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void TELNET_MGR_MgmtIPFltChangedCallbackHandler(UI32_T mode)
{
    if (MGMT_IP_FLT_TELNET == mode)
    {
        CLI_PMGR_HandleChangedIpMgmtFilter();
    }
}
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

