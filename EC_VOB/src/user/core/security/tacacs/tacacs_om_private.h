
/* Project Name: New Feature
 * File_Name : tacacs_om_private.h
 * Purpose     : TACACS initiation and TACACS task creation
 *
 * 2002/05/07    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
#ifndef TACACS_OM_PRIVATE_H
#define TACACS_OM_PRIVATE_H

//#include "sys_dflt.h"
#include <string.h>
//#include "leaf_es3626a.h"
#include "tacacs_type.h"
/*------------------------------------------------------------------------
 * LOCAL VARIABLES
 *-----------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 * DEFAULT VARIABLES DECLARTIONS
 *-----------------------------------------------------------------------*/
#if 0
#define TACACS_DEFAULT_TIMEOUT          4
#define TACACS_DEFAULT_SERVER_PORT      SYS_DFLT_TACACS_AUTH_CLIENT_SERVER_PORT_NUMBER
#define TACACS_DEFAULT_SERVER_IP        SYS_DFLT_TACACS_AUTH_SERVER_ADDRESS
#define TACACS_DEFAULT_SERVER_KEY       ""
#define TACACS_NOT_IN_MASTER_MODE          -3
#define TACACS_MAX_RETRANSMIT_TIMES        30
#define TACACS_MIN_RETRANSMIT_TIMES         1
#define TACACS_MAX_SERVER_PORT          65535 
#define TACACS_MIN_SERVER_PORT              1
#define TACACS_MAX_REQUEST_TIMEOUT      65535
#define TACACS_MIN_REQUEST_TIMEOUT          1 
#define TACSCA_MAX_SECRET_LENGTH        MAXSIZE_tacacsServerKey  
#endif 
 
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
BOOL_T TACACS_OM_CreatSem(void);

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
BOOL_T TACACS_OM_SetConfigSettingToDefault();

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
BOOL_T TACACS_OM_Set_Server_Port(UI32_T serverport);
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
BOOL_T TACACS_OM_Set_Server_Secret(UI8_T *serversecret);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote TACACS server
 * INPUT:    TACACS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_Set_Server_IP(UI32_T serverip);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Set_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will copy server_host setting into server entry
 * INPUT    : by_server_index,
 *            [server_index], server_ip, server_port, timeout, retransmit, secret, acct_port
 * OUTPUT   : None.
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS)
 *            if server_port, timeout, retransmit, acct_port == 0 then use global value
 *            if strlen(secret) == 0 then use global value
 *            if specified entry doesn't exist, then create it. or modify it
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_Set_Server_Host(BOOL_T by_server_index, TACACS_Server_Host_T *server_host);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_Destroy_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will destroy server entry
 * INPUT:    server_index
 * OUTPUT:   None.
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE:     index RANGE (1..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS)
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_OM_Destroy_Server_Host(UI32_T server_index);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetServerHostIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : get the ip address by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : ip_address
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetServerHostIpAddress(UI32_T server_index, UI32_T *ip_address);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_OM_GetServerHostTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : get the retransmission_timeout by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : retransmission_timeout
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetServerHostTimeout(UI32_T server_index, UI32_T *retransmission_timeout);

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
BOOL_T TACACS_OM_AccInitialize();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_AccFinalize
 *-------------------------------------------------------------------------
 * PURPOSE  : clean accounting om resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void TACACS_OM_AccFinalize();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : port number
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T TACACS_OM_GetServerAcctPort();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetServerAcctPort
 *-------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetServerAcctPort(UI32_T acct_port);

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
BOOL_T TACACS_OM_GetAccUserEntryQty(UI32_T *qty);
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
BOOL_T TACACS_OM_GetNextAccUserEntryInterface(TPACC_UserInfoInterface_T *entry);

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
BOOL_T TACACS_OM_GetNextAccUserEntryFilterByName(TPACC_UserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetNextAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : copy next accounting user to user_entry
 * INPUT    : user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetNextAccUserEntry(TACACS_AccUserInfo_T *entry);

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
BOOL_T TACACS_OM_GetAccUserEntry(TACACS_AccUserInfo_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryByKey
 *-------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryByKey(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type, TACACS_AccUserInfo_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntryActiveServerIdx
 *-------------------------------------------------------------------------
 * PURPOSE  : get active server index by user index
 * INPUT    : user_index
 * OUTPUT   : active_server_index
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntryActiveServerIdx(UI16_T user_index, UI32_T *active_server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_DoesAccUserExist
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified accounting user exist or not
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not exist
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_DoesAccUserExist(UI16_T user_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_MoveAccUserToTail
 *-------------------------------------------------------------------------
 * PURPOSE  : move user to the tail of queue
 * INPUT    : user_index (1-based)
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void TACACS_OM_MoveAccUserToTail(UI16_T user_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_CreateAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : create an accounting user for request
 * INPUT    : request, sys_time
 * OUTPUT   : none
 * RETURN   : user_index - succeeded, 0 - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI16_T TACACS_OM_CreateAccUserEntry(const AAA_AccRequest_T *request, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_FreeAccUser
 *-------------------------------------------------------------------------
 * PURPOSE: recycle specific user entry from user list
 * INPUT:   user_index (1-based)
 * OUTPUT:  none.
 * RETURN:  none
 * NOTES:   none.
 *-------------------------------------------------------------------------*/
void TACACS_OM_FreeAccUser(UI16_T user_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntrySessionStartTime
 *-------------------------------------------------------------------------
 * PURPOSE  : set the session start time by specific user index
 * INPUT    : user_index, start_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntrySessionStartTime(UI16_T user_index, UI32_T start_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryLastUpdateTime
 *-------------------------------------------------------------------------
 * PURPOSE  : set the last update time by specific user index
 * INPUT    : user_index, update_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryLastUpdateTime(UI16_T user_index, UI32_T update_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryAAATacacsInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : set aaa tacacs info by user index
 * INPUT    : user_index, entry
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryAAATacacsInfo(UI16_T user_index, TACACS_AAATacacsEntryInfo_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryStartSent
 *-------------------------------------------------------------------------
 * PURPOSE  : set the start package send flag by specific user index
 * INPUT    : user_index, sent_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryStartSent(UI16_T user_index, BOOL_T sent_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryStopSent
 *-------------------------------------------------------------------------
 * PURPOSE  : set the stop package send flag by specific user index
 * INPUT    : user_index, sent_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryStopSent(UI16_T user_index, BOOL_T sent_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryStartPacketWait
 *-------------------------------------------------------------------------
 * PURPOSE  : set the start package wait flag by specific user index
 * INPUT    : user_index, wait_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryStartPacketWait(UI16_T user_index, BOOL_T wait_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_ResetAccUserEntryCallbackInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : reset the call_back_func, identifier by specific user index
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_ResetAccUserEntryCallbackInfo(UI16_T user_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_GetAccUserEntrySessionId
 *-------------------------------------------------------------------------
 * PURPOSE  : get specified user's session id
 * INPUT    : user_index, buffer_size
 * OUTPUT   : id_buffer
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_GetAccUserEntrySessionId(UI16_T user_index, UI8_T *id_buffer, UI16_T buffer_size);

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
BOOL_T TACACS_OM_SetAccUserEntryTaskId(UI16_T user_index, UI32_T task_id);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_OM_SetAccUserEntryConnectStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : setup connection status by specific user index
 * INPUT    : user_index, connect_status
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_OM_SetAccUserEntryConnectStatus(UI16_T user_index, AAA_AccConnectStatus_T connect_status);
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */
#endif /* End of TACACS_OM_H */

