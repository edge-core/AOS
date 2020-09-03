/*-----------------------------------------------------------------------------
 * Module   : sshd_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access SSHD control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 09/28/2007 - Squid Ro, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef	SSHD_PMGR_H
#define	SSHD_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sshd_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - SSHD_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T SSHD_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_DeleteUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : delete user's public key.
 * INPUT    : username - name of user of public key will be delete.
 *            key_type - type of host key.
 *              (KEY_TYPE_RSA, KEY_TYPE_DSA and KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "delete public-key".
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_DeleteUserPublicKey(UI8_T *username, UI32_T key_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetAuthenticationRetries
 * ------------------------------------------------------------------------|
 * FUNCTION : get number of retries for authentication user.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : number of retries for authentication user.
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
UI32_T SSHD_PMGR_GetAuthenticationRetries(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetNegotiationTimeout
 * ------------------------------------------------------------------------|
 * FUNCTION : get number of negotiation timeout.
 * INPUT    : None
 * OUTPUT	: None
 * RETURN   : number of negotiation timeout.
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
UI32_T SSHD_PMGR_GetNegotiationTimeout(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetNextSshConnectionEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : get next active connection entry.
 * INPUT    : cid  - previous active connection id.
 * OUTPUT	: cid  - current active connection id.
 *            info - current active connection information.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show ssh".
 *            initial input value is -1.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetNextUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : get user's public key.
 * INPUT    : username - username
 * OUTPUT	: username - next user name
 *            rsa_key  - pointer of buffer to storage rsa public key.
 *            dsa_key  - pointer of buffer to storage dsa public key.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show public-key".
 *            initiation username is empty string.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetNextUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetServerKeySize
 * ------------------------------------------------------------------------|
 * FUNCTION : get version of ssh server.
 * INPUT    : key_size - number of bits for server key.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetServerKeySize(UI32_T *key_size);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshConnectionPrivilege
 * ------------------------------------------------------------------------|
 * FUNCTION : get user privilege of ssh connection, index is connection id.
 * INPUT    : cid       - connection_id.
 * OUTPUT	: privilege - privilege.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshConnectionPrivilege(UI32_T cid, UI32_T *privilege);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshConnectionUsername
 * ------------------------------------------------------------------------|
 * FUNCTION : get username of ssh connection, index is connection id.
 * INPUT    : cid      - connection_id.
 * OUTPUT	: username - username.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshConnectionUsername(UI32_T cid, UI8_T *username);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshdStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : get sshd state.
 * INPUT    : None
 * OUTPUT	: sshd state.
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
UI32_T SSHD_PMGR_GetSshdStatus(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshServerVersion
 * ------------------------------------------------------------------------|
 * FUNCTION : get version of ssh server.
 * INPUT    : major - number of major version.
 *            minor - number of minor version.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshServerVersion(UI32_T *major, UI32_T *minor);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : get user's public key.
 * INPUT    : username - username
 * OUTPUT	: rsa_key  - pointer of buffer to storage rsa public key.
 *            dsa_key  - pointer of buffer to storage dsa public key.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show public-key".
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetUserPublicKey(UI8_T *username, UI8_T *rsa_key, UI8_T *dsa_key);

/* FUNCTION NAME:  SSHD_PMGR_CopyUserPublicKey
 * PURPOSE:
 *          This function to copy public key from xfer tftp buffer to sshd buffer
 *
 * INPUT:
 *          UI8_T   *key_buf   --  xfer tftp public key buffer address
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in CLI command "copy tftp public-key".
 *          This function have be called before SSHD_PMGR_SetUserPublicKey().
 */
BOOL_T SSHD_PMGR_CopyUserPublicKey(UI8_T *key_buf);

/* FUNCTION NAME : SSHD_PMGR_GetUserPublicKeyType
 * PURPOSE:
 *      Get user's public key type.
 *
 * INPUT:
 *      UI8_T   *username   -- username.
 *
 * OUTPUT:
 *      UI32_T  *key_type   --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 *
 * RETURN:
 *      TRUE  - User has public key.
 *      FALSE - User has not public key.
 *
 * NOTES:
 *      This API is invoked in CLI command "show users".
 */
BOOL_T SSHD_PMGR_GetUserPublicKeyType(UI8_T *username, UI32_T *key_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetAuthenticationRetries
 * ------------------------------------------------------------------------|
 * FUNCTION : set number of retries for authentication user.
 * INPUT    : retries - number of retries for authentication user.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh authentication-retries'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetAuthenticationRetries(UI32_T retries);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetConnectionIDAndTnshID
 * ------------------------------------------------------------------------|
 * FUNCTION : set tnsh id and ssh connection id to session record.
 * INPUT    : tnsh_port - the port connect to TNSHD.
 *            tid       - TNSH id.
 *            cid       - ssh connection id.
 * OUTPUT	: None
 * RETURN   : TRUE  - tnsh_port found and ssh server enabled, or don't found tnsh_port.
 *            FALSE - tnsh_port found and ssh server disabled.
 * NOTE		: nvoked in TNSHD_ChildTask().
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetNegotiationTimeout
 * ------------------------------------------------------------------------|
 * FUNCTION : set number of negotiation timeout.
 * INPUT    : timeout - negotiation timeout
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh timeout'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetNegotiationTimeout(UI32_T timeout);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetServerKeySize
 * ------------------------------------------------------------------------|
 * FUNCTION : set number of bits for server key.
 * INPUT    : key_size - number of bits for server key, range is
 *                       from 512 to 896 bits.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh server-key size'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetServerKeySize(UI32_T key_size);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_SetSshdStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : set sshd state.
 * INPUT    : state - SSHD status.
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh server'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_SetSshdStatus (UI32_T state);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetGenerateHostKeyStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : get generate host key status of ssh server.
 * INPUT    : none
 * OUTPUT	: action_result - result of host key generation.
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetGenerateHostKeyStatus(UI32_T *action_result);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetGenerateHostKeyAction
 * ------------------------------------------------------------------------|
 * FUNCTION : Get tpye of host key generation.
 * INPUT    : none
 * OUTPUT	: UI32_T  *action_type    --  tpye of host key generation.
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetGenerateHostKeyAction(UI32_T *action_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetWriteHostKey2FlashAction
 * ------------------------------------------------------------------------|
 * FUNCTION : Get tpye of host key writing.
 * INPUT    : none
 * OUTPUT	: action_type - tpye of host key writing
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'show ip ssh'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetWriteHostKey2FlashAction(UI32_T *action_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_AsyncWriteHostKey2Flash
 * ------------------------------------------------------------------------|
 * FUNCTION : This function write host key from memory to flash.
 * INPUT    : none
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_AsyncWriteHostKey2Flash ();

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetWriteHostKey2FlashStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : Get result of host key writing.
 * INPUT    : none
 * OUTPUT	: *action_result  --  result of host key writing.
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetWriteHostKey2FlashStatus(UI32_T *action_result);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_AsyncGenerateHostKeyPair
 * ------------------------------------------------------------------------|
 * FUNCTION : Generate RSA/DSA public and private key pair of host key.
 * INPUT    : UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: maybe invoked in CLI command 'ip ssh server-key size'.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_AsyncGenerateHostKeyPair(UI32_T key_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetSshConnectionEntry
 * ------------------------------------------------------------------------|
 * FUNCTION : get active connection entry.
 * INPUT    : cid  - previous active connection id.
 * OUTPUT	: info - current active connection information.
 * RETURN   : TRUE/FALSE
 * NOTE		: invoked in CLI command "show ssh".
 *            initial input value is -1.
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetSshConnectionEntry(I32_T cid, SSHD_ConnectionInfo_T *info);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_AsyncDeleteUserPublicKey
 * ------------------------------------------------------------------------|
 * FUNCTION : Delete RSA/DSA public key of user key.
 * INPUT    : username - name of user of public key will be delete.
 *            key_type - Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA, KEY_TYPE_BOTH_RSA_AND_DSA)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: This API is invoked in SNMP sshUserKeyDelAction().
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_AsyncDeleteUserPublicKey(UI8_T *username, UI32_T key_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetRunningServerKeySize
 * ------------------------------------------------------------------------|
 * FUNCTION : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *            specific server key size non-default values
 *            can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *            or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT    : None
 * OUTPUT	: key_size - number of bits for server key.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE		: 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T  SSHD_PMGR_GetRunningServerKeySize(UI32_T *key_size);

/* FUNCTION NAME:  SSHD_PMGR_SetUserPublicKey
 * PURPOSE:
 *          This function to save user's public key to flash.
 *
 * INPUT:
 *          UI8_T   *username   --  Username of public key.
 *          UI32_T  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA)
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          One of SSHD_SetUserPublicKeyResult_T to indicate that the result.
 * NOTES:
 *          This function is invoked in CLI command "copy tftp public-key".
 *          This function have be called after SSHD_MGR_GetUserPublicKeyFromXfer().
 */
SSHD_SetUserPublicKeyResult_T SSHD_PMGR_SetUserPublicKey(UI8_T *username, UI32_T key_type);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - SSHD_PMGR_GetDeleteUserPublicKeyAction
 * ------------------------------------------------------------------------|
 * FUNCTION : Get tpye of user key delete.
 * INPUT    :
 * OUTPUT	: action_type --  tpye of user key delete.
 * RETURN   : TRUE/FALSE
 * NOTE		:
 * ------------------------------------------------------------------------*/
BOOL_T SSHD_PMGR_GetDeleteUserPublicKeyAction(UI32_T *action_type);

#endif /* SSHD_PMGR_H */
