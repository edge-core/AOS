#ifndef TACACS_POM_H
#define TACACS_POM_H


#include "tacacs_type.h"

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLUSTER_POM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
 * INPUT:
 *    None.
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
void TACACS_POM_Init(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetRunningServerPort
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
UI32_T TACACS_POM_GetRunningServerPort(UI32_T *serverport);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_POM_Get_Server_Port(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetRunningServerSecret
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
UI32_T  TACACS_POM_GetRunningServerSecret(UI8_T serversecret[]);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the TACACS client and server.
 * INPUT:    secret.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_POM_Get_Server_Secret(UI8_T* secret);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetRunningServerIP
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
UI32_T TACACS_POM_GetRunningServerIP(UI32_T *serverip);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TACACS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  TACACS_POM_Get_Server_IP(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetRunningServerRetransmit
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

UI32_T TACACS_POM_GetRunningServerRetransmit(UI32_T *retransmit);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Retransmit
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   retransmit.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_POM_GetServerRetransmit(void);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetRunningServerTimeout
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

UI32_T TACACS_POM_GetRunningServerTimeout(UI32_T *timeout);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote TACACS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   timeout.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_POM_GetServerTimeout(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_GetNext_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will copy server_host setting from server entry
 * INPUT:    server_index
 * OUTPUT:   next server_index (current index + 1), server_host
 * RETURN:   TRUE -- succeeded, FALSE -- Failed
 * NOTE:     index RANGE (0..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS - 1)
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_POM_GetNext_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TACACS_POM_GetNextRunning_Server_Host
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
UI32_T TACACS_POM_GetNextRunning_Server_Host(UI32_T *index,TACACS_Server_Host_T *server_host);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_POM_Get_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE  : This function will copy server_host setting from server entry
 * INPUT    : server_index (1-based)
 * OUTPUT   : server_host
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS)
 *            fail (1). if out of range (2). used_flag == false
 *---------------------------------------------------------------------------*/
BOOL_T TACACS_POM_Get_Server_Host(UI32_T server_index, TACACS_Server_Host_T *server_host);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetServerHostMaxRetransmissionTimeout
 *-------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_IsServerHostValid
 *-------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_IsServerHostValid(UI32_T server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_LookupServerIndexByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index);

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)/*maggie liu, 2009-03-09*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetRunningServerAcctPort
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
SYS_TYPE_Get_Running_Cfg_T TACACS_POM_GetRunningServerAcctPort(UI32_T *acct_port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserEntryQtyFilterByName
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name
 * INPUT    : name
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserEntryQtyFilterByName(UI8_T *name, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserEntryQtyFilterByNameAndType(UI8_T *name, AAA_ClientType_T client_type, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetNextAccUserEntryFilterByType(TPACC_UserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  TACACS_POM_GetAccUserRunningInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : get running_info by ifindex, user name, client type
 * INPUT    : ifindex, name, client_type
 * OUTPUT   : running_info
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : suppose (ifindex + name + client_type) is unique
 *-------------------------------------------------------------------------*/
BOOL_T TACACS_POM_GetAccUserRunningInfo(UI32_T ifindex, const UI8_T *name, AAA_ClientType_T client_type, AAA_AccUserRunningInfo_T *running_info);
#endif

#endif /*#ifndef TACACS_POM_H*/

