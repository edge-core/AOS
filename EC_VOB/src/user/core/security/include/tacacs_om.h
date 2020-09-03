
/* Project Name: New Feature
 * File_Name : tacacs_om.h
 * Purpose     : TACACS initiation and TACACS task creation
 *
 * 2007/05/31    : Eli Lin  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
#ifndef TACACS_OM_H
#define TACACS_OM_H
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "tacacs_type.h"
enum
{
    TACACS_OM_IPCCMD_GET_RUNNING_SERVER_PORT,
    TACACS_OM_IPCCMD_GET_SERVER_PORT,
    TACACS_OM_IPCCMD_GET_RUNNING_SERVER_SECRET,
    TACACS_OM_IPCCMD_GET_SERVER_SECRET,
    TACACS_OM_IPCCMD_GET_RUNNING_SERVER_IP,
    TACACS_OM_IPCCMD_GET_SERVER_IP,
    TACACS_OM_IPCCMD_GET_RUNNING_SERVER_RETRANSMIT,
    TACACS_OM_IPCCMD_GET_RUNNING_SERVER_TIMEOUT,
    TACACS_OM_IPCCMD_GET_SERVER_RETRANSMIT,
    TACACS_OM_IPCCMD_GET_SERVER_TIMEOUT,
    TACACS_OM_IPCCMD_GET_NEXT_SERVER_HOST,
    TACACS_OM_IPCCMD_GET_NEXT_RUNNING_SERVER_HOST,
    TACACS_OM_IPCCMD_GET_SERVER_HOST,
    TACACS_OM_IPCCMD_GETSERVERHOSTMAXRETRANSMISSIONTIMEOUT,
    TACACS_OM_IPCCMD_ISSERVERHOSTVALID,
    TACACS_OM_IPCCMD_LOOKUPSERVERINDEXBYIPADDRESS,
    TACACS_OM_IPCCMD_GETRUNNINGSERVERACCTPORT,
    TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAME,
    TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYTYPE,
    TACACS_OM_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAMEANDTYPE,
    TACACS_OM_IPCCMD_GETNEXTACCUSERENTRYFILTERBYTYPE,
    TACACS_OM_IPCCMD_GETACCUSERRUNNINGINFO,
    TACACS_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC
} ;

/*use to the definition of IPC message buffer*/
typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
    }type;

    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];


        UI8_T serversecret[MAXSIZE_tacacsServerKey+1];

        struct
        {
            UI32_T index;
            TACACS_Server_Host_T server_host;
        }index_serverhost;

        struct
        {
            UI32_T ip_address;
            UI32_T server_index;
        }ip_serverindex;

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
        struct
        {
            char   name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
            UI32_T qty;
        }name_qty;

        struct
        {
            AAA_ClientType_T client_type;
            UI32_T qty;
        }type_qty;

        struct
        {
            char name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
            AAA_ClientType_T client_type;
            UI32_T qty;
        }name_type_qty;

        TPACC_UserInfoInterface_T userinfointerface;

        struct
        {
            UI32_T ifindex;
            char name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
            AAA_ClientType_T client_type;
            AAA_AccUserRunningInfo_T running_info;
        }accuserrunninginfo;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING */
    } data; /* contains the supplemntal data for the corresponding cmd */
}TACACS_OM_IPCMsg_T;

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
UI32_T TACACS_OM_GetRunningServerPort(UI32_T *serverport);
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
UI32_T TACACS_OM_Get_Server_Port(void);
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
UI32_T  TACACS_OM_GetRunningServerSecret(UI8_T serversecret[]);
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
BOOL_T TACACS_OM_Get_Server_Secret(UI8_T* secret);
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
UI32_T TACACS_OM_GetRunningServerIP(UI32_T *serverip);
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
UI32_T  TACACS_OM_Get_Server_IP(void);

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
UI32_T TACACS_OM_GetRunningServerRetransmit(UI32_T *retransmit);

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
UI32_T TACACS_OM_GetServerRetransmit(void);

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
BOOL_T TACACS_OM_SetServerRetransmit(UI32_T retransmit);

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
UI32_T TACACS_OM_GetRunningServerTimeout(UI32_T *timeout);

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
UI32_T TACACS_OM_GetServerTimeout(void);

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
BOOL_T TACACS_OM_SetServerTimeout(UI32_T timeout);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetNext_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will copy server_host setting from server entry
 * INPUT:    server_index
 * OUTPUT:   next server_index (current index + 1), server_host
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE:     index RANGE (0..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS - 1)
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_GetNext_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host);
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
UI32_T TACACS_OM_GetNextRunning_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Get_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will copy server_host setting from server entry
 * INPUT    : server_index (1-based)
 * OUTPUT   : server_host
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS)
 *            fail (1). if out of range (2). used_flag == false
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_Get_Server_Host(UI32_T server_index, TACACS_Server_Host_T *server_host);
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetServerHostMaxRetransmissionTimeout
 *-------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout);
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_IsServerHostValid
 *-------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_IsServerHostValid(UI32_T server_index);
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_LookupServerIndexByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index);
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
SYS_TYPE_Get_Running_Cfg_T TACACS_OM_GetRunningServerAcctPort(UI32_T *acct_port);
#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
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
BOOL_T TACACS_OM_GetAccUserEntryQtyFilterByName(char *name, UI32_T *qty);
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
BOOL_T TACACS_OM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty);
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
BOOL_T TACACS_OM_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty);

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
BOOL_T TACACS_OM_GetNextAccUserEntryFilterByType(TPACC_UserInfoInterface_T *entry);
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserRunningInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : get running_info by ifindex, user name, client type
 * INPUT    : ifindex, name, client_type
 * OUTPUT   : running_info
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : suppose (ifindex + name + client_type) is unique
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserRunningInfo(UI32_T ifindex, const char *name, AAA_ClientType_T client_type, AAA_AccUserRunningInfo_T *running_info);
#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)*/

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
BOOL_T  TACACS_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccCmdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccCmdInfo(
    UI32_T      ifindex,
    const char  *user_name_p,
    const char  *cmd_buf_p,
    UI16_T      cmd_pri,
    UI32_T      *ser_no_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccCmdInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccCmdInfo(
    UI32_T      ifindex,
    const char  *user_name_p,
    const char  *cmd_buf_p,
    UI16_T      cmd_pri,
    UI32_T      ser_no);

#endif /*#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND == TRUE)*/

#endif /* End of TACACS_OM_H */

