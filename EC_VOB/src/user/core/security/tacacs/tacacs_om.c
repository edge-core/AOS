#include "l_stdlib.h"
#include "sysfun.h"
#include "tacacs_om_private.h"
#include "tacacs_om.h"
#include "libtacacs.h"
#include "ip_lib.h"
#include <stdio.h>
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "l_inet.h"
#include "sys_bld.h"

/* NAMING CONSTANT DECLARATIONS
 */
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
#define TACACS_ACC_MAX_NBR_OF_REQUEST       FD_SETSIZE
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */


static UI32_T TACACS_Auth_Server_Port;
static UI32_T TACACS_Auth_Server_Retransmit;
static UI32_T TACACS_Auth_Server_Timeout;
static UI8_T  TACACS_Auth_Server_IP[16];
static UI8_T  TACACS_Auth_Key[MAXSIZE_tacacsServerKey+1];

static TACACS_Server_Host_T TACACS_Server_Host[TACACS_MAX_NBR_OF_SERVERS];

static UI32_T tacacs_om_sem_id;
/*use for remember original priority of semaphore id
 */
static UI32_T orig_priority;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)

static BOOL_T   tacacs_acc_intialized = FALSE;

static UI32_T   tacacs_acct_port;

static UI32_T   tacacs_acc_invalid_server_address_counter;

static TACACS_AccUserInfo_T        acc_user_info[SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS];
static TACACS_AccUserInfo_T        *head_of_acc_user;
static TACACS_AccUserInfo_T        *tail_of_acc_user;
#endif

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
#define TACACS_OM_MAX_NBR_OF_ACC_CMD_USER   (SYS_ADPT_MAX_TELNET_NUM+1)
                                            /* 1 for console session,
                                             * refer to CLI_TASK_MAX_CONSOLE_SESSION_NUM
                                             */

static TACACS_AccCmdUserInfo_T      acc_cmd_user_info[TACACS_OM_MAX_NBR_OF_ACC_CMD_USER];
#endif


/* MACRO FUNCTIONS DECLARACTION
 */
#define TACACS_OM_LOCK()       orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(tacacs_om_sem_id);
#define TACACS_OM_UNLOCK()     SYSFUN_OM_LEAVE_CRITICAL_SECTION(tacacs_om_sem_id,orig_priority);

static void TACACS_OM_CopyServerHost(TACACS_Server_Host_T *det, TACACS_Server_Host_T *src);
static TACACS_Server_Host_T *TACACS_OM_GetServerHostByIpAddress(UI32_T ip_address);
static TACACS_Server_Host_T *TACACS_OM_Get_Server_Host_Entry(UI32_T server_index);
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
static TACACS_AccUserInfo_T *TACACS_OM_GetAccFirstUser();
static TACACS_AccUserInfo_T *TACACS_OM_GetAccUser(UI16_T user_index);
static TACACS_AccUserInfo_T *TACACS_OM_GetNextAccUser(UI16_T user_index);
static TACACS_AccUserInfo_T *TACACS_OM_QueryAccUser(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type);
static TACACS_AccUserInfo_T *TACACS_OM_AllocAccUser();
static void TACACS_OM_CopyAccUserEntryInterface(TPACC_UserInfoInterface_T *det, const TACACS_AccUserInfo_T *src);
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */



/*-------------------------------------------------------------------------
 * FUNCTION NAME - TACACS_OM_CreatSem
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for TACACS objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_CreatSem(void)
{
    /* create semaphore */
    if(SYSFUN_OK!=SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_TACACS_OM, &tacacs_om_sem_id))
    {
        return FALSE;
    }
    return TRUE;
} /* End of TACACS_OM_CreatSem */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_OM_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set default value of TACACS configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T TACACS_OM_SetConfigSettingToDefault()
{
    UI32_T  i, network_order_default_ip;

    TACACS_OM_LOCK();
    TACACS_Auth_Server_Port       = SYS_DFLT_TACACS_AUTH_CLIENT_SERVER_PORT_NUMBER;
    TACACS_Auth_Server_Retransmit = SYS_DFLT_TACACS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS;
    TACACS_Auth_Server_Timeout    = SYS_DFLT_TACACS_AUTH_CLIENT_TIMEOUTS;
    /* ES3550MO-PoE-FLF-AA-00077
     * sys_dflt.h define IP address in host order
     * so need to convert it to network order
     */
    network_order_default_ip = L_STDLIB_Hton32(TACACS_DEFAULT_SERVER_IP);

    /*TACACS_Auth_Timeout = TACACS_DEFAULT_TIMEOUT ;*/
    memset(TACACS_Auth_Key,'\0',MAXSIZE_tacacsServerKey+1);

    /*sprintf(TACACS_Auth_Server_IP,"%d.%d.%d.%d",((int) (TACACS_DEFAULT_SERVER_IP>>24)&0xff),(int) ((TACACS_DEFAULT_SERVER_IP>>16)&0xff),(int) ((TACACS_DEFAULT_SERVER_IP>>8)&0xff),(int) (TACACS_DEFAULT_SERVER_IP & 0xff));*/
    L_INET_Ntoa(network_order_default_ip, TACACS_Auth_Server_IP);

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    tacacs_acct_port = SYS_DFLT_TACACS_ACC_CLIENT_SERVER_PORT_NUMBER;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

    /* for multi-server */
    for(i=0;i<TACACS_MAX_NBR_OF_SERVERS;i++)
    {
        TACACS_Server_Host[i].used_flag   = FALSE;
        TACACS_Server_Host[i].server_ip   = network_order_default_ip;
        TACACS_Server_Host[i].server_port = SYS_DFLT_TACACS_AUTH_CLIENT_SERVER_PORT_NUMBER;
        TACACS_Server_Host[i].retransmit  = SYS_DFLT_TACACS_AUTH_CLIENT_TIMEOUTS;
        TACACS_Server_Host[i].timeout     = SYS_DFLT_TACACS_AUTH_CLIENT_TIMEOUTS;
        memset(TACACS_Server_Host[i].secret,'\0',TACACS_AUTH_KEY_MAX_LEN);
        TACACS_Server_Host[i].server_index = i + 1;
    }

    TACACS_OM_UNLOCK();
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    TACACS_OM_AccInitialize();
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
    return TRUE;
}
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_OM_GetRunningServerPort
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T TACACS_OM_GetRunningServerPort(UI32_T *serverport)
{
    UI32_T result;

    TACACS_OM_LOCK();
    *serverport = TACACS_Auth_Server_Port;
    TACACS_OM_UNLOCK();

    if (TACACS_DEFAULT_SERVER_PORT != *serverport)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_OM_Get_Server_Port(void)
{
    UI32_T serverport;

    TACACS_OM_LOCK();
    serverport = TACACS_Auth_Server_Port;
    TACACS_OM_UNLOCK();
    return serverport;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote TACACS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_Set_Server_Port(UI32_T serverport)
{
    /*UI8_T *arg_p = "Server port";*/

    /* if(Radius_operation_mode != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;*/

    if (serverport < TACACS_MIN_SERVER_PORT|| serverport > TACACS_MAX_SERVER_PORT)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_TACACS, TACACS_OM_SET_SERVER_PORT_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO,"Server port"/*arg_p*/);/*Mercury_V2-00030*/
         return FALSE;
        }

    TACACS_OM_LOCK();
        TACACS_Auth_Server_Port = serverport;

#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == FALSE)
      TACACS_Server_Host[0].server_port = serverport;
#endif

    TACACS_OM_UNLOCK();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_OM_GetRunningServerSecret
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server secret is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serversecret[MAX_SECRET_LENGTH + 1]
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  TACACS_OM_GetRunningServerSecret(UI8_T serverkey[])
{
    UI32_T result;

    TACACS_OM_LOCK();
    memcpy(serverkey,TACACS_Auth_Key,MAXSIZE_tacacsServerKey);
    TACACS_OM_UNLOCK();
    if ( strcmp((char *)serverkey,TACACS_DEFAULT_SERVER_KEY) !=0)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_Get_Server_Secret(UI8_T* secret)
{
    TACACS_OM_LOCK();
    memcpy(secret,TACACS_Auth_Key,sizeof(TACACS_Auth_Key));
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_Set_Server_Secret(UI8_T *serversecret)
{
    /*UI8_T *arg_p = "Server secret";*/
    UI32_T  index;

    if (strlen((char *)serversecret) > MAXSIZE_tacacsServerKey) /*length can't greater than 48 */ /* validation check */
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TACACS, TACACS_OM_SET_SERVER_SECRET_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO,"Server secret"/*arg_p*/);/*Mercury_V2-00030*/
        return FALSE;
    }

    TACACS_OM_LOCK();
    strncpy((char *)TACACS_Auth_Key,(char *)serversecret,MAXSIZE_tacacsServerKey);
    for(index = 0; TACACS_MAX_NBR_OF_SERVERS > index; ++index)
        strncpy((char *)TACACS_Server_Host[index].secret, (char *)serversecret, TACACS_AUTH_KEY_MAX_LEN);
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetRunningServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default TACACS retransmit is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:    None.
 * OUTPUT:   retransmit.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
             SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
             SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_OM_GetRunningServerRetransmit(UI32_T *retransmit)
{
    UI32_T result;

    if (NULL == retransmit)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    TACACS_OM_LOCK();
    *retransmit = TACACS_Auth_Server_Retransmit;
    TACACS_OM_UNLOCK();

    if (SYS_DFLT_TACACS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS != *retransmit)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the global retransmit of the remote TACACS server
 * INPUT  :  None.
 * OUTPUT :  None.
 * RETURN :  Retransmit
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_OM_GetServerRetransmit(void)
{
    UI32_T retransmit;

    TACACS_OM_LOCK();
    retransmit = TACACS_Auth_Server_Retransmit;
    TACACS_OM_UNLOCK();
    return retransmit;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_SetServerRetransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global retransmit of the remote TACACS server
 * INPUT  :  Retransmit
 * OUTPUT :  None.
 * RETURN :  TRUE/FALSE
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_SetServerRetransmit(UI32_T retransmit)
{
    if (retransmit < SYS_ADPT_TACACS_MIN_RETRANSMIT || retransmit > SYS_ADPT_TACACS_MAX_RETRANSMIT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TACACS, LIBTACACS_OM_SET_SERVER_PORT_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO,"Server retransmit");
        return FALSE;
    }

    TACACS_OM_LOCK();
    TACACS_Auth_Server_Retransmit = retransmit;

#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == FALSE)
      TACACS_Server_Host[0].retransmit = retransmit;
#endif

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetRunningServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default TACACS timeout is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:    None.
 * OUTPUT:   timeout.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
             SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
             SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_OM_GetRunningServerTimeout(UI32_T *timeout)
{
    UI32_T result;

    if (NULL == timeout)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    TACACS_OM_LOCK();
    *timeout = TACACS_Auth_Server_Timeout;
    TACACS_OM_UNLOCK();

    if (SYS_DFLT_TACACS_AUTH_CLIENT_TIMEOUTS != *timeout)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the global timeout of the remote TACACS server
 * INPUT  :  None.
 * OUTPUT :  None.
 * RETURN :  Timeout
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_OM_GetServerTimeout(void)
{
    UI32_T timeout;

    TACACS_OM_LOCK();
    timeout = TACACS_Auth_Server_Timeout;
    TACACS_OM_UNLOCK();
    return timeout;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_SetServerTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the global timeout of the remote TACACS server
 * INPUT  :  Prot number
 * OUTPUT :  None.
 * RETURN :  TRUE/FALSE
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_SetServerTimeout(UI32_T timeout)
{
    if (timeout < SYS_ADPT_TACACS_MIN_TIMEOUT || timeout > SYS_ADPT_TACACS_MAX_TIMEOUT)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_TACACS, LIBTACACS_OM_SET_SERVER_PORT_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO,"Server timeout");
        return FALSE;
    }

    TACACS_OM_LOCK();
    TACACS_Auth_Server_Timeout = timeout;

#if (SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER == FALSE)
      TACACS_Server_Host[0].timeout = timeout;
#endif

    TACACS_OM_UNLOCK();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_OM_GetRunningServerIP
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T TACACS_OM_GetRunningServerIP(UI32_T *serverip)
{
    UI32_T result, network_order_default_ip;

    TACACS_OM_LOCK();
    /* obsoleted my_inet_addr() with L_INET_Aton
     */
    /* *serverip=(UI32_T ) my_inet_addr(TACACS_Auth_Server_IP); */
    if (FALSE == L_INET_Aton(TACACS_Auth_Server_IP, serverip))
        *serverip = 0;

    TACACS_OM_UNLOCK();

    /* ES3550MO-PoE-FLF-AA-00077
     * sys_dflt.h define IP address in host order
     * so need to convert it to network order
     */
    network_order_default_ip = L_STDLIB_Hton32(TACACS_DEFAULT_SERVER_IP);

    if (network_order_default_ip != *serverip)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TACACS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  TACACS_OM_Get_Server_IP(void)
{
 UI32_T serverip;

 TACACS_OM_LOCK();
    /* ES3550MO-PoE-FLF-AA-00077
     * TACACS+ use network order internally
     */
    /*serverip = (UI32_T ) my_inet_addr(TACACS_Auth_Server_IP);*/
    if (FALSE == L_INET_Aton(TACACS_Auth_Server_IP, &serverip))
        serverip = 0;

 TACACS_OM_UNLOCK();
 return serverip;
}

BOOL_T TACACS_OM_Set_Server_IP(UI32_T serverip)
{
    TACACS_OM_LOCK();
    sprintf((char *)TACACS_Auth_Server_IP,"%d.%d.%d.%d",((int) (serverip>>24)&0xff),(int) ((serverip>>16)&0xff),(int) ((serverip>>8)&0xff),(int) (serverip & 0xff));

    /* ES3550MO-PoE-FLF-AA-00077
     * TACACS+ use network order internally
     */
    /*sprintf(TACACS_Auth_Server_IP,"%d.%d.%d.%d",((int) (serverip>>24)&0xff),(int) ((serverip>>16)&0xff),(int) ((serverip>>8)&0xff),(int) (serverip & 0xff));*/
    L_INET_Ntoa(serverip, TACACS_Auth_Server_IP);

    TACACS_OM_UNLOCK();
    return TRUE;
}
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_CopyServerHost
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will copy one entry to another
 * INPUT:    src
 * OUTPUT:   det
 * RETURN:   none
 * NOTE:     none
 *---------------------------------------------------------------------------
 */
static void TACACS_OM_CopyServerHost(TACACS_Server_Host_T *det, TACACS_Server_Host_T *src)
{
    memcpy(det, src, sizeof(TACACS_Server_Host_T)); /* maybe better than one-by-one copy */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetServerHostByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : get server host by ip address
 * INPUT    : ip_address
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static TACACS_Server_Host_T *TACACS_OM_GetServerHostByIpAddress(UI32_T ip_address)
{
    UI32_T  index;

    for (index = 0; TACACS_MAX_NBR_OF_SERVERS > index; ++index)
    {
        if (FALSE == TACACS_Server_Host[index].used_flag)
            continue;

        if (ip_address == TACACS_Server_Host[index].server_ip)
            return &TACACS_Server_Host[index];
    }

    return NULL;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will copy server_host setting into server entry
 * INPUT    : by_server_index,
 *            [server_index], server_ip, server_port, timeout, retransmit, secret, acct_port
 * OUTPUT   : None.
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..TACACS_MAX_NBR_OF_SERVERS)
 *            if server_port, timeout, retransmit, acct_port == 0 then use global value
 *            if strlen(secret) == 0 then use global value
 *            if specified entry doesn't exist, then create it. or modify it
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_Set_Server_Host(BOOL_T by_server_index, TACACS_Server_Host_T *server_host)
{
    TACACS_Server_Host_T    *om_entry;

    UI32_T  index;
    UI32_T  secret_len;

    TACACS_OM_LOCK();

    if (NULL == server_host)
        {
        TACACS_OM_UNLOCK();
        return FALSE;
        }

    om_entry = NULL;

    /* search by server ip */
    for (index = 0; TACACS_MAX_NBR_OF_SERVERS > index; ++index)
    {
        /*maggie liu, ES4827G-FLF-ZZ-00405*/
        /*if (FALSE == TACACS_Server_Host[index].used_flag)
            continue;*/

        if (server_host->server_ip != TACACS_Server_Host[index].server_ip)
            continue;

        om_entry = &TACACS_Server_Host[index];
        break;
    }

    /* prepare entry */
    if (TRUE == by_server_index)
    {
        if ((NULL != om_entry) && (server_host->server_index != om_entry->server_index))
        {
            TACACS_OM_UNLOCK();
            return FALSE;
        }

        if (( 0 >= server_host->server_index) || (TACACS_MAX_NBR_OF_SERVERS < server_host->server_index))
        {
            TACACS_OM_UNLOCK();
            return FALSE;
        }

        om_entry = &TACACS_Server_Host[server_host->server_index - 1]; /* to zero-based */
    }
    else
    {
        if (NULL == om_entry)
        {
            /* not exist, find an empty entry and create it */
            for (index = 0; TACACS_MAX_NBR_OF_SERVERS > index; ++index)
            {
               /*maggie liu, ES4827G-FLF-ZZ-00405*/
               /* if (TRUE == TACACS_Server_Host[index].used_flag)
                    continue;*/

                om_entry = &TACACS_Server_Host[index];
                break;
            }

            if (NULL == om_entry)
            {
                TACACS_OM_UNLOCK();
                return FALSE;
            }
        }
    }

    /* fill data members one by one and validate them
     */

    om_entry->server_ip = server_host->server_ip; /* ip can't be omitted */

    if (server_host->server_port == 0)
    {
        om_entry->server_port = TACACS_Auth_Server_Port;
    }
    else
    {
        if (server_host->server_port < TACACS_MIN_SERVER_PORT ||
            server_host->server_port > TACACS_MAX_SERVER_PORT)
        {
            TACACS_OM_UNLOCK();
            return FALSE;
        }

        om_entry->server_port = server_host->server_port;
    }

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    if (server_host->acct_port == 0)
    {
        om_entry->acct_port = tacacs_acct_port;
    }
    else
    {
        if (server_host->acct_port < TACACS_MIN_SERVER_PORT ||
            server_host->acct_port > TACACS_MAX_SERVER_PORT)
        {
            TACACS_OM_UNLOCK();
            return FALSE;
        }

        om_entry->acct_port = server_host->acct_port;
    }
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
    if (server_host->retransmit == 0)
    {
        om_entry->retransmit = TACACS_Auth_Server_Retransmit;
    }
    else
    {
        if (server_host->retransmit < SYS_ADPT_TACACS_MIN_RETRANSMIT||
            server_host->retransmit > SYS_ADPT_TACACS_MAX_RETRANSMIT)
        {
            TACACS_OM_UNLOCK();
            return FALSE;
        }

        om_entry->retransmit = server_host->retransmit;
    }

    if (server_host->timeout == 0)
    {
        om_entry->timeout = TACACS_Auth_Server_Timeout;
    }
    else
    {
        if (server_host->timeout < TACACS_MIN_REQUEST_TIMEOUT ||
            server_host->timeout > TACACS_MAX_REQUEST_TIMEOUT)
        {
            TACACS_OM_UNLOCK();
            return FALSE;
        }

        om_entry->timeout = server_host->timeout;
    }
    secret_len = strlen((char *)server_host->secret);
    if (secret_len == 0)
    {
        strncpy((char *)om_entry->secret, (char *)TACACS_Auth_Key, TACACS_AUTH_KEY_MAX_LEN);
    }
    else
    {
        if (secret_len > TACACS_AUTH_KEY_MAX_LEN)
        {
            TACACS_OM_UNLOCK();
            return FALSE;
        }

        strncpy((char *)om_entry->secret, (char *)server_host->secret, TACACS_AUTH_KEY_MAX_LEN);
    }

    om_entry->used_flag = TRUE;

    TACACS_OM_UNLOCK();
    return TRUE;
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Destroy_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy server entry
 * INPUT:    server_index
 * OUTPUT:   None.
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE:     index RANGE (1..TACACS_MAX_NBR_OF_SERVERS)
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_Destroy_Server_Host(UI32_T server_index)
{
    TACACS_OM_LOCK();
    if(server_index <= 0 || server_index > TACACS_MAX_NBR_OF_SERVERS)
    {
        TACACS_OM_UNLOCK();
        return FALSE;
    }

    server_index --;/*database start at 0*/

    TACACS_Server_Host[server_index].used_flag   = FALSE;
    TACACS_Server_Host[server_index].server_ip   = TACACS_DEFAULT_SERVER_IP;
    TACACS_Server_Host[server_index].server_port = TACACS_DEFAULT_SERVER_PORT;
    TACACS_Server_Host[server_index].timeout     = SYS_DFLT_TACACS_AUTH_CLIENT_TIMEOUTS;
    memset(TACACS_Server_Host[server_index].secret,'\0',TACACS_AUTH_KEY_MAX_LEN);
   TACACS_OM_UNLOCK();
   return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetNext_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will copy server_host setting from server entry
 * INPUT:    server_index
 * OUTPUT:   next server_index (current index + 1), server_host
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE:     index RANGE (0..TACACS_MAX_NBR_OF_SERVERS - 1)
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_GetNext_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host)
{
    UI32_T i=0;

 TACACS_OM_LOCK();

    if(*index > TACACS_MAX_NBR_OF_SERVERS)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    if (server_host == NULL)
       {
         TACACS_OM_UNLOCK();
         return FALSE;
        }
    for(i=*index;i < TACACS_MAX_NBR_OF_SERVERS;i++)
    {
        if (TACACS_Server_Host[i].used_flag==FALSE)
            continue;

        TACACS_OM_CopyServerHost(server_host, &TACACS_Server_Host[i]);

        *index = ++i;

        TACACS_OM_UNLOCK();
                return TRUE;
    }
    TACACS_OM_UNLOCK();
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_OM_GetNextRunning_Server_Host
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TACACS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T TACACS_OM_GetNextRunning_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host)
{
    UI32_T result;

    if(*index > TACACS_MAX_NBR_OF_SERVERS)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (TACACS_OM_GetNext_Server_Host(index,server_host))
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Get_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will copy server_host setting from server entry
 * INPUT    : server_index (1-based)
 * OUTPUT   : server_host
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..TACACS_MAX_NBR_OF_SERVERS)
 *            fail (1). if out of range (2). used_flag == false
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_Get_Server_Host(UI32_T server_index, TACACS_Server_Host_T *server_host)
{
    TACACS_Server_Host_T    *entry;

   TACACS_OM_LOCK();
    if (NULL == server_host)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    entry = TACACS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == entry) || (FALSE == entry->used_flag))
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }
    TACACS_OM_CopyServerHost(server_host, entry);
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_Get_Server_Host_Entry
 *-------------------------------------------------------------------------
 * PURPOSE  : get server host entry by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - not found
 * NOTES    : call by rc_send_server
 *-------------------------------------------------------------------------*/
static TACACS_Server_Host_T *TACACS_OM_Get_Server_Host_Entry(UI32_T server_index)
{
    if ((0 >= server_index) || (TACACS_MAX_NBR_OF_SERVERS < server_index))
        return NULL;

    return &(TACACS_Server_Host[server_index - 1]);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetServerHostIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : get the ip address by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : ip_address
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetServerHostIpAddress(UI32_T server_index, UI32_T *ip_address)
{
    TACACS_Server_Host_T    *server_host;

    TACACS_OM_LOCK();
    if (NULL == ip_address)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    server_host = TACACS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    *ip_address = server_host->server_ip;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetServerHostTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : get the retransmission_timeout by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : retransmission_timeout
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetServerHostTimeout(UI32_T server_index, UI32_T *retransmission_timeout)
{
    TACACS_Server_Host_T    *server_host;

    TACACS_OM_LOCK();

    if (NULL == retransmission_timeout)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    server_host = TACACS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    *retransmission_timeout = server_host->timeout;
    TACACS_OM_UNLOCK();
    return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetServerHostMaxRetransmissionTimeout
 *-------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout)
{
    UI32_T  server_index;

    TACACS_Server_Host_T    *server_host;

    TACACS_OM_LOCK();
    if (NULL == max_retransmission_timeout)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    *max_retransmission_timeout = 0;
    for (server_index = 1; TACACS_MAX_NBR_OF_SERVERS >= server_index; ++server_index)
    {
        server_host = TACACS_OM_Get_Server_Host_Entry(server_index);
        if (NULL == server_host)
            break;

        if (FALSE == server_host->used_flag)
            continue;

        if (server_host->timeout > *max_retransmission_timeout)
            *max_retransmission_timeout = server_host->timeout;
    }

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_IsServerHostValid
 *-------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_IsServerHostValid(UI32_T server_index)
{
    TACACS_Server_Host_T    *server_host;

    server_host = TACACS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host)
        return FALSE;

    if (FALSE == server_host->used_flag)
    {
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_LookupServerIndexByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index)
{
    TACACS_Server_Host_T    *server_host;

    TACACS_OM_LOCK();

    if (NULL == server_index)
       {
         TACACS_OM_UNLOCK();
         return FALSE;
       }

    server_host = TACACS_OM_GetServerHostByIpAddress(ip_address);
    if (NULL == server_host)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
       }
    *server_index = server_host->server_index;
    TACACS_OM_UNLOCK();
    return TRUE;
}

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_AccInitialize
 *-------------------------------------------------------------------------
 * PURPOSE  : (re-)initialize accounting om
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : FALSE - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_AccInitialize()
{
    UI16_T  index;
    TACACS_AccUserInfo_T       *user_info;

    TACACS_OM_AccFinalize();

    tacacs_acc_invalid_server_address_counter = 0;

    for (index = 0; SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS > index; ++index)
    {
        user_info = &acc_user_info[index];

        memset(user_info, 0, sizeof(TACACS_AccUserInfo_T));

        user_info->user_index = index + 1;
        user_info->entry_status = TPACC_ENTRY_DESTROYED;
    }

    head_of_acc_user = NULL;
    tail_of_acc_user = NULL;
    tacacs_acc_intialized = TRUE;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_AccFinalize
 *-------------------------------------------------------------------------
 * PURPOSE  : clean accounting om resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void TACACS_OM_AccFinalize()
{
    if (TRUE != tacacs_acc_intialized)
    {
        /* om doesn't initialize yet */
        return;
    }

    tacacs_acc_intialized = FALSE;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : port number
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T TACACS_OM_GetServerAcctPort()
{
   return tacacs_acct_port;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetServerAcctPort(UI32_T acct_port)
{
    TACACS_OM_LOCK();
    if ((TACACS_MIN_SERVER_PORT > acct_port) || (TACACS_MAX_SERVER_PORT < acct_port))
    {
        TACACS_OM_UNLOCK();
        return FALSE;
    }

    tacacs_acct_port = acct_port;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryQty(UI32_T *qty)
{
    TACACS_AccUserInfo_T       *user_info;

    TACACS_OM_LOCK();
     if (NULL == qty)
        {
        TACACS_OM_UNLOCK();
         return FALSE;
        }

    for (*qty = 0, user_info = TACACS_OM_GetAccFirstUser(); NULL != user_info;
        user_info = user_info->next_user)
    {
        switch (user_info->connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }

        ++(*qty);
    }

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryQtyFilterByName
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name
 * INPUT    : name
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryQtyFilterByName(char *name, UI32_T *qty)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == qty)
        {
          TACACS_OM_UNLOCK();
          return FALSE;
         }

    *qty = 0;
    user_info = TACACS_OM_GetAccFirstUser();
    for ( ; NULL != user_info; user_info = user_info->next_user)
    {
        switch (user_info->connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }
        if (strcmp((char *)name, (char *)user_info->user_name) == 0)
            ++(*qty);
    }

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == qty)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    *qty = 0;
    user_info = TACACS_OM_GetAccFirstUser();
    for ( ; NULL != user_info; user_info = user_info->next_user)
    {
        switch (user_info->connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }
        if (client_type == user_info->client_type)
            ++(*qty);
    }

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty)
{
    TACACS_AccUserInfo_T   *user_info;

    if (NULL == qty)
        return FALSE;

    *qty = 0;
    user_info = TACACS_OM_GetAccFirstUser();
    for ( ; NULL != user_info; user_info = user_info->next_user)
    {
        switch (user_info->connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }

        if ((client_type == user_info->client_type) &&
            (strcmp((char *)name, (char *)user_info->user_name) == 0))
            ++(*qty);
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetNextAccUserEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user entry by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetNextAccUserEntryInterface(TPACC_UserInfoInterface_T *entry)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == entry)
        {
        TACACS_OM_UNLOCK();
        return FALSE;
        }

    user_info = TACACS_OM_GetNextAccUser(entry->user_index);
    for ( ; NULL != user_info; user_info = user_info->next_user)
    {
        switch (user_info->connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }

        TACACS_OM_CopyAccUserEntryInterface(entry, user_info);
        TACACS_OM_UNLOCK();
        return TRUE;
    }
   TACACS_OM_UNLOCK();
   return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetNextAccUserEntryFilterByName
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name by index.
 * INPUT    : entry->user_index, entry->user_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetNextAccUserEntryFilterByName(TPACC_UserInfoInterface_T *entry)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == entry)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    user_info = TACACS_OM_GetNextAccUser(entry->user_index);
    for ( ; NULL != user_info; user_info = user_info->next_user)
    {
        if (strcmp((char *)entry->user_name, (char *)user_info->user_name) != 0)
            continue;
        switch (user_info->connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }

        TACACS_OM_CopyAccUserEntryInterface(entry, user_info);
        TACACS_OM_UNLOCK();
        return TRUE;
    }

   TACACS_OM_UNLOCK();
   return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetNextAccUserEntryFilterByType(TPACC_UserInfoInterface_T *entry)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == entry)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
       }

    user_info = TACACS_OM_GetNextAccUser(entry->user_index);
    for ( ; NULL != user_info; user_info = user_info->next_user)
    {
        if (entry->client_type != user_info->client_type)
            continue;
        switch (user_info->connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }

        TACACS_OM_CopyAccUserEntryInterface(entry, user_info);
        TACACS_OM_UNLOCK();
        return FALSE;
    }

     TACACS_OM_UNLOCK();
     return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetNextAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : copy next accounting user to user_entry
 * INPUT    : user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetNextAccUserEntry(TACACS_AccUserInfo_T *entry)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == entry)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
         }
    user_info = TACACS_OM_GetNextAccUser(entry->user_index);
    if (NULL == user_info)
        {
          TACACS_OM_UNLOCK();
          return FALSE;
        }

    memcpy(entry, user_info, sizeof(TACACS_AccUserInfo_T));
    entry->next_user = NULL;
    entry->prev_user = NULL;

   TACACS_OM_UNLOCK();
   return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : copy accounting user to user_entry
 * INPUT    : user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *            fail (1). if out of range (2). entry_tatus == DESTROYED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntry(TACACS_AccUserInfo_T *entry)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == entry)
        {
          TACACS_OM_UNLOCK();
          return FALSE;
        }
    user_info = TACACS_OM_GetAccUser(entry->user_index);
    if (NULL == user_info)
     {
      TACACS_OM_UNLOCK();
      return FALSE;
     }

    memcpy(entry, user_info, sizeof(TACACS_AccUserInfo_T));
    entry->next_user = NULL;
    entry->prev_user = NULL;

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryByKey
 *-------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryByKey(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type, TACACS_AccUserInfo_T *entry)
{
    TACACS_AccUserInfo_T  *user_info;

    TACACS_OM_LOCK();
    if (NULL == entry)
      {
        TACACS_OM_UNLOCK();
        return FALSE;
      }

    user_info = TACACS_OM_QueryAccUser(ifindex, user_name, client_type);
    if (NULL == user_info)
        {
         TACACS_OM_UNLOCK();
          return FALSE;
        }

    memcpy(entry, user_info, sizeof(TACACS_AccUserInfo_T));
    entry->next_user = NULL;
    entry->prev_user = NULL;

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryActiveServerIdx
 *-------------------------------------------------------------------------
 * PURPOSE  : get active server index by user index
 * INPUT    : user_index
 * OUTPUT   : active_server_index
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryActiveServerIdx(UI16_T user_index, UI32_T *active_server_index)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == active_server_index)
        {
        TACACS_OM_UNLOCK();
        return FALSE;
        }

    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
        }

    *active_server_index = user_info->tacacs_entry_info.active_server_index;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_DoesAccUserExist
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified accounting user exist or not
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not exist
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_DoesAccUserExist(UI16_T user_index)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        {
         TACACS_OM_UNLOCK();
          return FALSE;
        }

    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_MoveAccUserToTail
 *-------------------------------------------------------------------------
 * PURPOSE  : move user to the tail of queue
 * INPUT    : user_index (1-based)
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void TACACS_OM_MoveAccUserToTail(UI16_T user_index)
{
    TACACS_AccUserInfo_T  *entry;

    TACACS_OM_LOCK();
    if ((0 >= user_index) || (SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS < user_index))
        {
         TACACS_OM_UNLOCK();
         return;
        }

    entry = &acc_user_info[user_index - 1];

    if (tail_of_acc_user == entry) /* this entry already be the last one */
         {
         TACACS_OM_UNLOCK();
         return;
        }

    if (head_of_acc_user == entry) /* this entry is the first one */
        head_of_acc_user = entry->next_user;
    else
        entry->prev_user->next_user = entry->next_user;

    entry->next_user->prev_user = entry->prev_user;

    tail_of_acc_user->next_user = entry;
    entry->prev_user = tail_of_acc_user;
    entry->next_user = NULL;
    tail_of_acc_user = entry;
    TACACS_OM_UNLOCK();
    return;
 }

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_CreateAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : create an accounting user for request
 * INPUT    : request, sys_time
 * OUTPUT   : none
 * RETURN   : user_index - succeeded, 0 - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI16_T TACACS_OM_CreateAccUserEntry(const AAA_AccRequest_T *request, UI32_T sys_time)
{
    TACACS_AccUserInfo_T  *user_info;

    TACACS_OM_LOCK();
    if (NULL == request)
    {
       TACACS_OM_UNLOCK();
       return 0;
    }

    user_info = TACACS_OM_AllocAccUser();
    if (NULL == user_info)
    {
        TACACS_OM_UNLOCK();
        return 0;
    }

    user_info->ifindex = request->ifindex;
    user_info->rem_ip_addr = request->rem_ip_addr;

    strncpy((char *)user_info->user_name, (char *)request->user_name, SYS_ADPT_MAX_USER_NAME_LEN);
    user_info->user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0'; /* force to end a string */

    user_info->accounting_start_time = sys_time;
    user_info->session_start_time = sys_time;
    user_info->last_update_time = 0;
    memset(&user_info->tacacs_entry_info, 0, sizeof(TACACS_AAATacacsEntryInfo_T));
    memset(&user_info->ctrl_bitmap, 0, sizeof(TACACS_AccUserCtrlBitmap_T));

    user_info->connect_status = AAA_ACC_CNET_IDLE;

    user_info->identifier = request->identifier;
    user_info->call_back_func = request->call_back_func;

    user_info->auth_by_whom = request->auth_by_whom;
    user_info->client_type = request->client_type;
    user_info->serial_number = request->serial_number;

    TACACS_OM_UNLOCK();
    return user_info->user_index;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_FreeAccUser
 *-------------------------------------------------------------------------
 * PURPOSE: recycle specific user entry from user list
 * INPUT:   user_index (1-based)
 * OUTPUT:  none.
 * RETURN:  none
 * NOTES:   none.
 *-------------------------------------------------------------------------*/
void TACACS_OM_FreeAccUser(UI16_T user_index)
{
    TACACS_AccUserInfo_T   *entry;

    TACACS_OM_LOCK();
    if ((0 >= user_index) || (SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS < user_index))
    {
        TACACS_OM_UNLOCK();
        return;
    }

    entry = &acc_user_info[user_index - 1];
    if (TPACC_ENTRY_DESTROYED == entry->entry_status)
    {
        TACACS_OM_UNLOCK();
        return;
    }

    if (entry == head_of_acc_user)
    {
        head_of_acc_user = entry->next_user;
    }

    if (entry == tail_of_acc_user)
    {
        tail_of_acc_user = entry->prev_user;
    }

    if (NULL != entry->prev_user)
        entry->prev_user->next_user = entry->next_user;

    if (NULL != entry->next_user)
        entry->next_user->prev_user = entry->prev_user;

    entry->prev_user = NULL;
    entry->next_user = NULL;
    entry->entry_status = TPACC_ENTRY_DESTROYED;
    TACACS_OM_UNLOCK();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntrySessionStartTime
 *-------------------------------------------------------------------------
 * PURPOSE  : set the session start time by specific user index
 * INPUT    : user_index, start_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntrySessionStartTime(UI16_T user_index, UI32_T start_time)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
      {
        TACACS_OM_UNLOCK();
        return FALSE;
      }

    user_info->session_start_time = start_time;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryLastUpdateTime
 *-------------------------------------------------------------------------
 * PURPOSE  : set the last update time by specific user index
 * INPUT    : user_index, update_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryLastUpdateTime(UI16_T user_index, UI32_T update_time)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
       {
       TACACS_OM_UNLOCK();
       return FALSE;
      }

    user_info->last_update_time = update_time;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryAAATacacsInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : set aaa tacacs info by user index
 * INPUT    : user_index, entry
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryAAATacacsInfo(UI16_T user_index, TACACS_AAATacacsEntryInfo_T *entry)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    if (NULL == entry)
      {
         TACACS_OM_UNLOCK();
         return FALSE;
      }
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
       }

    memcpy(&user_info->tacacs_entry_info, entry, sizeof(TACACS_AAATacacsEntryInfo_T));
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryStartSent
 *-------------------------------------------------------------------------
 * PURPOSE  : set the start package send flag by specific user index
 * INPUT    : user_index, sent_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryStartSent(UI16_T user_index, BOOL_T sent_flag)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
      {
       TACACS_OM_UNLOCK();
       return FALSE;
      }

    user_info->ctrl_bitmap.start_packet_sent = sent_flag ? 1 : 0;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryStopSent
 *-------------------------------------------------------------------------
 * PURPOSE  : set the stop package send flag by specific user index
 * INPUT    : user_index, sent_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryStopSent(UI16_T user_index, BOOL_T sent_flag)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
       {
        TACACS_OM_UNLOCK();
        return FALSE;
       }

    user_info->ctrl_bitmap.stop_packet_sent = sent_flag ? 1 : 0;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryStartPacketWait
 *-------------------------------------------------------------------------
 * PURPOSE  : set the start package wait flag by specific user index
 * INPUT    : user_index, wait_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryStartPacketWait(UI16_T user_index, BOOL_T wait_flag)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
      {
       TACACS_OM_UNLOCK();
       return FALSE;
      }

    user_info->ctrl_bitmap.start_packet_wait = wait_flag ? 1 : 0;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_ResetAccUserEntryCallbackInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : reset the call_back_func, identifier by specific user index
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_ResetAccUserEntryCallbackInfo(UI16_T user_index)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
      {
        TACACS_OM_UNLOCK();
         return FALSE;
       }

    user_info->call_back_func = NULL;
    user_info->identifier = 0;
   TACACS_OM_UNLOCK();
   return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntrySessionId
 *-------------------------------------------------------------------------
 * PURPOSE  : get specified user's session id
 * INPUT    : user_index, buffer_size
 * OUTPUT   : id_buffer
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntrySessionId(UI16_T user_index, UI8_T *id_buffer, UI16_T buffer_size)
{
    /* didn't check whether user_index is valid or not here */
    UI16_T  digit_cnt, div_num;

    for (digit_cnt = 1, div_num = user_index; 0 < div_num; ++digit_cnt)
    {
        div_num /= 10;
    }

    if (digit_cnt >= buffer_size) /* buffer size must be reserved 1-char for '\0' */
    {
        return FALSE;
    }

    sprintf((char *)id_buffer, "%d", user_index);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryConnectStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : setup connection status by specific user index
 * INPUT    : user_index, connect_status
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryConnectStatus(UI16_T user_index, AAA_AccConnectStatus_T connect_status)
{
    TACACS_AccUserInfo_T   *user_info;

   TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
     {
      TACACS_OM_UNLOCK();
      return FALSE;
    }
    user_info->connect_status = connect_status;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - TACACS_OM_GetAccFirstUser
 *-------------------------------------------------------------------------
 * PURPOSE  : get the first entry
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTE     : none.
 *-------------------------------------------------------------------------*/
static TACACS_AccUserInfo_T *TACACS_OM_GetAccFirstUser()
{
    return head_of_acc_user;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : get specific user entry from user list
 * INPUT    : user_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static TACACS_AccUserInfo_T *TACACS_OM_GetAccUser(UI16_T user_index)
{
    TACACS_AccUserInfo_T   *user_info;

    if ((0 >= user_index) || (SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS < user_index))
        return NULL;

    user_info = &acc_user_info[user_index - 1]; /* to zero-based */
    if (TPACC_ENTRY_DESTROYED == user_info->entry_status)
        return NULL;

    return user_info;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetNextAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : get next accounting user
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static TACACS_AccUserInfo_T *TACACS_OM_GetNextAccUser(UI16_T user_index)
{
    TACACS_AccUserInfo_T   *user_info;

    if (0 == user_index)
    {
        user_info = TACACS_OM_GetAccFirstUser();
    }
    else
    {
        user_info = TACACS_OM_GetAccUser(user_index);
        if (NULL != user_info)
            user_info = user_info->next_user;
    }

    return user_info;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_QueryAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static TACACS_AccUserInfo_T *TACACS_OM_QueryAccUser(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type)
{
    UI16_T      index;

    TACACS_AccUserInfo_T  *entry;

    if (NULL == user_name)
        return FALSE;

    for (index = 0; SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS > index; ++index)
    {
        entry = &acc_user_info[index];

        if (TPACC_ENTRY_DESTROYED == entry->entry_status)
            continue;

        if (ifindex != entry->ifindex)
            continue;

        if (client_type != entry->client_type)
            continue;

        if (strcmp((char *)user_name, (char *)entry->user_name) != 0)
            continue;

        return entry;
    }

    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_AllocAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty user info
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static TACACS_AccUserInfo_T *TACACS_OM_AllocAccUser()
{
    UI16_T      index;

    TACACS_AccUserInfo_T  *entry;

    for (index = 0; SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS > index; ++index)
    {
        if (TPACC_ENTRY_DESTROYED != acc_user_info[index].entry_status)
            continue;

        entry = &acc_user_info[index];
        entry->entry_status = TPACC_ENTRY_READY;

        /* let this new user be the first one
           because its last_update_time is smallest
         */
        entry->prev_user = NULL;
        entry->next_user = head_of_acc_user;

        if (NULL == head_of_acc_user)
        {
            /* first entry */
            tail_of_acc_user = entry;
        }
        else
        {
            head_of_acc_user->prev_user = entry;
        }

        head_of_acc_user = entry;

        return entry;
    }

    return NULL;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_OM_CopyAccUserEntryInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void TACACS_OM_CopyAccUserEntryInterface(TPACC_UserInfoInterface_T *det, const TACACS_AccUserInfo_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        return;
    }

    det->user_index = src->user_index;
    det->ifindex = src->ifindex;

    strncpy((char *)det->user_name, (char *)src->user_name, SYS_ADPT_MAX_USER_NAME_LEN);
    det->user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0'; /* force to end a string */

    det->accounting_start_time = src->accounting_start_time;
    det->client_type = src->client_type;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryTaskId
 *-------------------------------------------------------------------------
 * PURPOSE  : set task id of acc user
 * INPUT    : user_index  --  user index
 *            task_id     --  task id
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryTaskId(UI16_T user_index, UI32_T task_id)
{
    TACACS_AccUserInfo_T   *user_info;

    TACACS_OM_LOCK();
    user_info = TACACS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        {
         TACACS_OM_UNLOCK();
         return FALSE;
         }

    user_info->task_id = task_id;
    TACACS_OM_UNLOCK();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetRunningServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *            the non-default TACACS accounting port is successfully retrieved.
 *            Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT    : none
 * OUTPUT   : acct_port
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default value.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T TACACS_OM_GetRunningServerAcctPort(UI32_T *acct_port)
{
    UI32_T  port;

    if (NULL == acct_port)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    TACACS_OM_LOCK();
    port = TACACS_OM_GetServerAcctPort();
    TACACS_OM_UNLOCK();

    if (SYS_DFLT_TACACS_ACC_CLIENT_SERVER_PORT_NUMBER == port)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    *acct_port = port;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserRunningInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : get running_info by ifindex, user name, client type
 * INPUT    : ifindex, name, client_type
 * OUTPUT   : running_info
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : suppose (ifindex + name + client_type) is unique
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserRunningInfo(UI32_T ifindex, const char *name, AAA_ClientType_T client_type, AAA_AccUserRunningInfo_T *running_info)
{
    BOOL_T  ret;

    TACACS_AccUserInfo_T   entry;

    if (NULL == running_info)
    {
        return FALSE;
    }

    ret = TACACS_OM_GetAccUserEntryByKey(ifindex, name, client_type, &entry);
    if (TRUE == ret)
    {
        running_info->session_start_time = entry.session_start_time;
        running_info->connect_status = entry.connect_status;

        if (FALSE == TACACS_OM_GetServerHostIpAddress(entry.tacacs_entry_info.active_server_index, &running_info->in_service_ip))
        {
            running_info->in_service_ip = 0;
        }
    }

     return ret;
}

#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)*/

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccCmdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : To get the info for accounting commands
 * INPUT    : ifindex     - ifindex to get
 *            user_name_p - pointer to user name to get
 *            cmd_buf_p   - pointer to command buffer to get
 *            cmd_pri     - command priviledge to get
 * OUTPUT   : ser_no      - pointer to output serial number
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccCmdInfo(
    UI32_T      ifindex,
    const char  *user_name_p,
    const char  *cmd_buf_p,
    UI16_T      cmd_pri,
    UI32_T      *ser_no_p)
{
    TACACS_AccCmdUserInfo_T *tmp_cu_info_p;
    BOOL_T                  ret = FALSE;

    TACACS_OM_LOCK();
    if (ifindex <= TACACS_OM_MAX_NBR_OF_ACC_CMD_USER)
    {
        tmp_cu_info_p = &acc_cmd_user_info[ifindex];
        if (  (cmd_pri == tmp_cu_info_p->cmd_pri)
            &&(0 == strcmp(user_name_p, tmp_cu_info_p->user_name))
            &&(0 == strcmp(cmd_buf_p, tmp_cu_info_p->cmd))
           )
        {
            *ser_no_p = tmp_cu_info_p->serial_number;
            ret = TRUE;
        }
    }
    TACACS_OM_UNLOCK();

    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccCmdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : To set the info for accounting commands
 * INPUT    : ifindex     - ifindex to set
 *            user_name_p - pointer to user name to set
 *            cmd_buf_p   - pointer to command buffer to set
 *            cmd_pri     - command priviledge to set
 *            ser_no      - serial number to set
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccCmdInfo(
    UI32_T      ifindex,
    const char  *user_name_p,
    const char  *cmd_buf_p,
    UI16_T      cmd_pri,
    UI32_T      ser_no)
{
    TACACS_AccCmdUserInfo_T *tmp_cu_info_p;
    BOOL_T                  ret = FALSE;

    TACACS_OM_LOCK();
    if (ifindex <= TACACS_OM_MAX_NBR_OF_ACC_CMD_USER)
    {
        tmp_cu_info_p = &acc_cmd_user_info[ifindex];
        tmp_cu_info_p->serial_number = ser_no;
        tmp_cu_info_p->cmd_pri       = cmd_pri;
        strcpy (tmp_cu_info_p->user_name, user_name_p);
        strcpy (tmp_cu_info_p->cmd, cmd_buf_p);
        ret = TRUE;
    }
    TACACS_OM_UNLOCK();

    return ret;
}
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)*/

/*------------------------------------------------------------------------------
 * ROUTINE NAME : TACACS_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T  TACACS_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    TACACS_OM_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if(ipcmsg_p==NULL)
        return FALSE;

    msg_data_p= (TACACS_OM_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;

    switch(cmd)
    {

       case TACACS_OM_IPCCMD_GET_RUNNING_SERVER_PORT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_GetRunningServerPort(&msg_data_p->data.ui32_v);
            break;


       case TACACS_OM_IPCCMD_GET_SERVER_PORT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32=TACACS_OM_Get_Server_Port();
            break;

       case TACACS_OM_IPCCMD_GET_RUNNING_SERVER_SECRET:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.serversecret);
            msg_data_p->type.result_ui32=TACACS_OM_GetRunningServerSecret(
                msg_data_p->data.serversecret);
            break;

       case TACACS_OM_IPCCMD_GET_SERVER_SECRET:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.serversecret);
            msg_data_p->type.result_bool=TACACS_OM_Get_Server_Secret(
                msg_data_p->data.serversecret);

            break;

       case TACACS_OM_IPCCMD_GET_RUNNING_SERVER_IP:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_GetRunningServerIP(&msg_data_p->data.ui32_v);
            break;

       case TACACS_OM_IPCCMD_GET_SERVER_IP:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_Get_Server_IP();
            break;

       case TACACS_OM_IPCCMD_GET_RUNNING_SERVER_RETRANSMIT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_GetRunningServerRetransmit(
                &msg_data_p->data.ui32_v);
            break;

       case TACACS_OM_IPCCMD_GET_RUNNING_SERVER_TIMEOUT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_GetRunningServerTimeout(
                &msg_data_p->data.ui32_v);
            break;

       case TACACS_OM_IPCCMD_GET_SERVER_RETRANSMIT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_GetServerRetransmit();
            break;

       case TACACS_OM_IPCCMD_GET_SERVER_TIMEOUT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_GetServerTimeout();
            break;

       case TACACS_OM_IPCCMD_GET_NEXT_SERVER_HOST:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.index_serverhost);
            msg_data_p->type.result_bool=TACACS_OM_GetNext_Server_Host(
                &msg_data_p->data.index_serverhost.index,
                &msg_data_p->data.index_serverhost.server_host);
            break;

       case TACACS_OM_IPCCMD_GET_NEXT_RUNNING_SERVER_HOST:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.index_serverhost);
            msg_data_p->type.result_ui32=TACACS_OM_GetNextRunning_Server_Host(
                &msg_data_p->data.index_serverhost.index,
                &msg_data_p->data.index_serverhost.server_host);
            break;

       case TACACS_OM_IPCCMD_GET_SERVER_HOST:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.index_serverhost);
            msg_data_p->type.result_bool=TACACS_OM_Get_Server_Host(
                msg_data_p->data.index_serverhost.index,
                &msg_data_p->data.index_serverhost.server_host);
            break;

       case TACACS_OM_IPCCMD_GETSERVERHOSTMAXRETRANSMISSIONTIMEOUT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_bool=TACACS_OM_GetServerHostMaxRetransmissionTimeout(
                &msg_data_p->data.ui32_v);
            break;

       case TACACS_OM_IPCCMD_ISSERVERHOSTVALID:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_bool=TACACS_OM_IsServerHostValid(msg_data_p->data.ui32_v);/**/
            break;

       case TACACS_OM_IPCCMD_LOOKUPSERVERINDEXBYIPADDRESS:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ip_serverindex);
            msg_data_p->type.result_bool=TACACS_OM_LookupServerIndexByIpAddress(
                msg_data_p->data.ip_serverindex.ip_address,
                &msg_data_p->data.ip_serverindex.server_index);
            break;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
       case TACACS_OM_IPCCMD_GETRUNNINGSERVERACCTPORT:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_ui32=TACACS_OM_GetRunningServerAcctPort(
                &msg_data_p->data.ui32_v);
            break;

       case TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAME:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.name_qty);
            msg_data_p->type.result_bool=TACACS_OM_GetAccUserEntryQtyFilterByName(
                msg_data_p->data.name_qty.name,
                &msg_data_p->data.name_qty.qty);
            break;

       case TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYTYPE:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.type_qty);
            msg_data_p->type.result_bool=TACACS_OM_GetAccUserEntryQtyFilterByType(
                msg_data_p->data.type_qty.client_type,
                &msg_data_p->data.type_qty.qty);
            break;

       case TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAMEANDTYPE:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.name_type_qty);
            msg_data_p->type.result_bool=TACACS_OM_GetAccUserEntryQtyFilterByNameAndType(
                msg_data_p->data.name_type_qty.name,
                msg_data_p->data.name_type_qty.client_type,
                &msg_data_p->data.name_type_qty.qty);
            break;

       case TACACS_OM_IPCCMD_GETNEXTACCUSERENTRYFILTERBYTYPE:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.userinfointerface);
            msg_data_p->type.result_bool=TACACS_OM_GetNextAccUserEntryFilterByType(
                &msg_data_p->data.userinfointerface);
            break;

       case TACACS_OM_IPCCMD_GETACCUSERRUNNINGINFO:
            ipcmsg_p->msg_size=TACACS_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.accuserrunninginfo);
            msg_data_p->type.result_bool=TACACS_OM_GetAccUserRunningInfo(
                msg_data_p->data.accuserrunninginfo.ifindex,
                msg_data_p->data.accuserrunninginfo.name,
                msg_data_p->data.accuserrunninginfo.client_type,
                &msg_data_p->data.accuserrunninginfo.running_info);
            break;
#endif /* #if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE) */

        default:
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
     */
    if(cmd<TACACS_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
