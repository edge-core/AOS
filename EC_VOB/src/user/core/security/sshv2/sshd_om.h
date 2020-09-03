/* MODULE NAME:  sshd_om.h
* PURPOSE:
*   Initialize the database resource and provide some get/set functions for accessing the
*   sshd database.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2002-05-27      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef SSHD_OM_H

#define SSHD_OM_H



/* INCLUDE FILE DECLARATIONS
 */
#include "includes.h"
#include "sys_type.h"
#include "sshd_type.h"

#include "servconf.h"
#include "bufaux.h"
#include "kex.h"
#include "ssh.h"
#include "channels.h"
#include "dispatch.h"
#include "auth.h"
#include "session.h"

#include "l_inet.h"
#include "userauth.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/*
 * Any really sensitive data in the application is contained in this
 * structure. The idea is that this structure could be locked into memory so
 * that the pages do not get written into swap.  However, there are some
 * problems. The private key contains BIGNUMs, and we do not (in principle)
 * have access to the internals of them, and locking just the structure is
 * not very useful.  Currently, memory locking is not implemented.
 */
typedef struct SSHD_SensitiveData_S
{
	Key	*server_key;		/* ephemeral server key */
	Key	*ssh1_host_key;		/* ssh1 host key */
	Key	*host_keys[3];		/* all private host keys */

	int	have_ssh1_key;
	int	have_ssh2_key;
	u_char	ssh1_cookie[SSH_SESSION_KEY_LENGTH];
}SSHD_SensitiveData_T;


typedef void chan_fn(Channel *c, fd_set * readset, fd_set * writeset);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  SSHD_OM_Init
 * PURPOSE:
 *          Initiate the semaphore for SSHD objects
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
 *          This function is invoked in SSHD_MGR_Init.
 */
BOOL_T SSHD_OM_Init(void);



#if 0
/* FUNCTION NAME:  SSHD_OM_EnterCriticalSection
 * PURPOSE:
 *          Enter critical section before a task invokes the sshd objects.
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
 *          .
 */
BOOL_T SSHD_OM_EnterCriticalSection(void);



/* FUNCTION NAME:  SSHD_OM_LeaveCriticalSection
 * PURPOSE:
 *          Leave critical section after a task invokes the sshd objects.
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
 *          .
 */
BOOL_T SSHD_OM_LeaveCriticalSection(void);
#endif



/* FUNCTION NAME:  SSHD_OM_SetSshdStatus
 * PURPOSE:
 *          This function set sshd state.
 *
 * INPUT:
 *          SSHD_State_T - SSHD status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetSshdStatus (SSHD_State_T state);



/* FUNCTION NAME:  SSHD_OM_GetSshdStatus
 * PURPOSE:
 *          This function get sshd state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSHD_State_T - SSHD status.
 * NOTES:
 *          .
 */
SSHD_State_T SSHD_OM_GetSshdStatus(void);



/* FUNCTION NAME:  SSHD_OM_SetSshdPort
 * PURPOSE:
 *          This function set sshd port number.
 *
 * INPUT:
 *          UI32_T - SSHD port number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshdPort (UI32_T port);



/* FUNCTION NAME:  SSHD_OM_GetSshdPort
 * PURPOSE:
 *			This function get sshd port number.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - SSHD port value.
 * NOTES:
 *          default is tcp/22.
 */
UI32_T SSHD_OM_GetSshdPort(void);



#if 0
/* FUNCTION NAME:  SSHD_OM_ResetSshdSessionRecord
 * PURPOSE:
 *          This function reset sshd session record.
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
 *          .
 */
void SSHD_OM_ResetSshdSessionRecord(void);
#endif



/* FUNCTION NAME:  SSHD_OM_SetAuthenticationRetries
 * PURPOSE:
 *          This function set number of retries for authentication user.
 *
 * INPUT:
 *          UI32_T -- number of retries for authentication user.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetAuthenticationRetries(UI32_T retries);



/* FUNCTION NAME:  SSHD_OM_GetAuthenticationRetries
 * PURPOSE:
 *          This function get number of retries for authentication user.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T --  number of retries for authentication user.
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetAuthenticationRetries(void);



/* FUNCTION NAME:  SSHD_OM_SetNegotiationTimeout
 * PURPOSE:
 *          This function set number of negotiation timeout .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetNegotiationTimeout(UI32_T timeout);



/* FUNCTION NAME:  SSHD_OM_GetNegotiationTimeout
 * PURPOSE:
 *          This function get number of negotiation timeout .
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T --  number of negotiation timeout .
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetNegotiationTimeout(void);



#if 0
/* FUNCTION NAME:  SSHD_OM_GetSshdSessionRecord
 * PURPOSE:
 *			This function get sshd session record pointer.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSHD_Session_Record_T - SSHD session record pointer.
 * NOTES:
 *          .
 */
SSHD_Session_Record_T *SSHD_OM_GetSshdSessionRecord(void);
#endif



/* FUNCTION NAME:  SSHD_OM_GetCreatedSessionNumber
 * PURPOSE:
 *			This function get number of  ssh connection.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T -- number of  ssh connection.
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetCreatedSessionNumber(void);



/* FUNCTION NAME:  SSHD_OM_SetCreatedSessionNumber
 * PURPOSE:
 *			This function set number of  ssh connection.
 * INPUT:
 *          UI32_T -- number of  ssh connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetCreatedSessionNumber(UI32_T number);



#if 0
/* FUNCTION NAME:  SSHD_OM_SetTaskID
 * PURPOSE:
 *			This function set task id to session record.
 * INPUT:
 *          UI32_T -- index of session record.
 *          UI32_T -- task id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetTaskID(UI32_T index, UI32_T tid);



/* FUNCTION NAME:  SSHD_OM_GetTaskID
 * PURPOSE:
 *			This function get task id from session record.
 * INPUT:
 *          UI32_T -- index of session record.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T -- task id.
 * NOTES:
 *          .
 */
UI32_T SSHD_OM_GetTaskID(UI32_T index);
#endif



#if 0
/* FUNCTION NAME:  SSHD_OM_GetContextAddress
 * PURPOSE:
 *			This function get ssh_context pointer.
 * INPUT:
 *          UI32_T -- index of session record.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          SSHD_Context_T * -- ssh_context pointer.
 * NOTES:
 *          .
 */
SSHD_Context_T *SSHD_OM_GetContextAddress(UI32_T index);



/* FUNCTION NAME:  SSHD_OM_ResetTaskID
 * PURPOSE:
 *			This function reset task id to session record.
 * INPUT:
 *          UI32_T -- index of session record.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_ResetTaskID(UI32_T index);
#endif



/* FUNCTION NAME:  SSHD_OM_SetSshServerVersion()
 * PURPOSE:
 *          This function set version of ssh server.
 *
 * INPUT:
 *          UI32_T - number of major version.
 *          UI32_T - number of minor version.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshServerVersion(UI32_T major, UI32_T minor);



/* FUNCTION NAME:  SSHD_OM_GetSshServerVersion()
 * PURPOSE:
 *          This function get version of ssh server.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T * - number of major version.
 *          UI32_T * - number of minor version.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshServerVersion(UI32_T *major, UI32_T *minor);



/* FUNCTION NAME : SSHD_OM_GetNextSshConnectionEntry
 * PURPOSE:
 *      Get next active connection entry.
 *
 * INPUT:
 *      UI32_T * -- previous active connection id.
 *
 * OUTPUT:
 *      UI32_T * -- current active connection id.
 *      SSHD_ConnectionInfo_T * -- current active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is current active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in CLI command "show ssh".
 *      Initial input value is -1.
 */
BOOL_T SSHD_OM_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info);



/* FUNCTION NAME : SSHD_OM_GetSshConnectionEntry
 * PURPOSE:
 *      Get specify active connection entry.
 *
 * INPUT:
 *      UI32_T   -- specify active connection id.
 *
 * OUTPUT:
 *      SSHD_ConnectionInfo_T * -- specify active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is specify active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in SNMP.
 */
BOOL_T SSHD_OM_GetSshConnectionEntry(UI32_T cid, SSHD_ConnectionInfo_T *info);



/* FUNCTION NAME:  SSHD_OM_SetServerKeySize
 * PURPOSE:
 *          This function set number of bits for server key.
 *
 * INPUT:
 *          UI32_T key_size --  number of bits for server key. The number of key size range is
 *                              512 to 896 bits.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetServerKeySize(UI32_T key_size);



/* FUNCTION NAME:  SSHD_OM_GetServerKeySize
 * PURPOSE:
 *          This function get number of bits for server key.
 *
 * INPUT:
 *          UI32_T *key_size    --  number of bits for server key.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 * NOTES:
 *
 */
BOOL_T SSHD_OM_GetServerKeySize(UI32_T *key_size);



/* FUNCTION NAME:  SSHD_OM_GetSshServerOptions
 * PURPOSE:
 *			This function get ssh server options pointer.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          ServerOptions   **options   -- SSH server options pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshServerOptions(UI32_T my_tid, ServerOptions **options);



/* FUNCTION NAME:  SSHD_OM_SetSshServerOptions
 * PURPOSE:
 *			This function alloc ssh server options pointer and set owner tid.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshServerOptions(UI32_T my_tid);



/* FUNCTION NAME:  SSHD_OM_SetSshServerVersionString
 * PURPOSE:
 *			This function set ssh server version string.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I8_T    *version_string --  server version string.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshServerVersionString(UI32_T my_tid, I8_T *version_string);



/* FUNCTION NAME:  SSHD_OM_SetSshClientVersionString
 * PURPOSE:
 *			This function set ssh client version string.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I8_T    *version_string --  client version string.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshClientVersionString(UI32_T my_tid, I8_T *version_string);



/* FUNCTION NAME:  SSHD_OM_SetSshSessionId
 * PURPOSE:
 *			This function set ssh session id for sshv1.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI8_T   *session_id --  session id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshSessionId(UI32_T my_tid, UI8_T *session_id);



/* FUNCTION NAME:  SSHD_OM_GetSshSessionId
 * PURPOSE:
 *			This function get ssh session id for sshv1.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          UI8_T    *session_id --  session id.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSessionId(UI32_T my_tid, UI8_T *session_id);



/* FUNCTION NAME:  SSHD_OM_GetSshServerVersionString
 * PURPOSE:
 *			This function get ssh server version string pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I8_T    **version_string    --  server version string pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshServerVersionString(UI32_T my_tid, I8_T **version_string);



/* FUNCTION NAME:  SSHD_OM_GetSshClientVersionString
 * PURPOSE:
 *			This function get ssh client version string pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I8_T    **version_string    --  client version string pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshClientVersionString(UI32_T my_tid, I8_T **version_string);



/* FUNCTION NAME:  SSHD_OM_SetSshKex
 * PURPOSE:
 *			This function set ssh kex pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          Kex     *xxx_kex    --  kex pointer.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshKex(UI32_T my_tid, Kex *xxx_kex);



/* FUNCTION NAME:  SSHD_OM_GetSshKex
 * PURPOSE:
 *			This function get ssh kex pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          Kex **xxx_kex   --  kex pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshKex(UI32_T my_tid, Kex **xxx_kex);



/* FUNCTION NAME:  SSHD_OM_SetSshSessionId2
 * PURPOSE:
 *			This function set ssh session id and id length for sshv2.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          UI8_T   *session_id2    --  session id.
 *          I32_T   session_id2_len --  id length.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshSessionId2(UI32_T my_tid, UI8_T *session_id2, I32_T session_id2_len);



/* FUNCTION NAME:  SSHD_OM_GetSshSessionId2
 * PURPOSE:
 *			This function get ssh session id and id length for sshv2.
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **session_id2       --  session id.
 *          I32_T   *session_id2_len    --  id length.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSessionId2(UI32_T my_tid, UI8_T **session_id2, I32_T *session_id2_len);



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionSockId
 * PURPOSE:
 *          This function set socket id for communicating with
 *          the other side.  connection_in is used for reading;
 *          connection_out for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   connection_in   --  connection_in is used for reading.
 *          I32_T   connection_out  --  connection_in is used for writing.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionSockId(UI32_T my_tid, I32_T connection_in, I32_T connection_out);



/* FUNCTION NAME:  SSHD_OM_GetSshConnectionSockId
 * PURPOSE:
 *          This function get socket id for communicating with
 *          the other side.  connection_in is used for reading;
 *          connection_out for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *connection_in  --  connection_in is used for reading.
 *          I32_T   *connection_out --  connection_in is used for writing.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshConnectionSockId(UI32_T my_tid, I32_T *connection_in, I32_T *connection_out);



/* FUNCTION NAME:  SSHD_OM_GetSshSendContext
 * PURPOSE:
 *			This function get ssh connection cipher context for sending.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          CipherContext   **send_context   --  cipher context for sending.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSendContext(UI32_T my_tid, CipherContext **send_context);



/* FUNCTION NAME:  SSHD_OM_GetSshReceiveContext
 * PURPOSE:
 *			This function get ssh connection cipher context for receiving.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          CipherContext   **Receive_context   --  cipher context for receiving.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshReceiveContext(UI32_T my_tid, CipherContext **receive_context);



/* FUNCTION NAME:  SSHD_OM_GetSshCurrentKeys
 * PURPOSE:
 *          This function get pointer of new Session key information for Encryption and MAC.
 *          MODE_IN is used for reading, MODE_OUT is used for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Newkeys ***current_keys --  pointer of Session key information for Encryption and MAC.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshCurrentKeys(UI32_T my_tid, Newkeys ***current_keys);



/* FUNCTION NAME:  SSHD_OM_GetSshNewKey
 * PURPOSE:
 *          This function get pointer of Session key information for Encryption and MAC.
 *          MODE_IN is used for reading, MODE_OUT is used for writing.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Newkeys ***newkeys  --  pointer of Session key information for Encryption and MAC.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshNewKey(UI32_T my_tid, Newkeys ***newkeys);



/* FUNCTION NAME:  SSHD_OM_GetSshInputBuffer
 * PURPOSE:
 *          This function get buffer pointer for raw input data from the socket.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **input --  buffer pointer for raw input data from the socket.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshInputBuffer(UI32_T my_tid, Buffer **input);



/* FUNCTION NAME:  SSHD_OM_GetSshOutputBuffer
 * PURPOSE:
 *          This function get buffer pointer for raw output data going to the socket.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **output    --  buffer pointer for raw output data going to the socket.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshOutputBuffer(UI32_T my_tid, Buffer **output);



/* FUNCTION NAME:  SSHD_OM_GetSshOutgoingPacketBuffer
 * PURPOSE:
 *          This function get buffer pointer for the partial outgoing packet being constructed.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **outgoing_packet   --  buffer pointer for the partial outgoing packet being constructed.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshOutgoingPacketBuffer(UI32_T my_tid, Buffer **outgoing_packet);



/* FUNCTION NAME:  SSHD_OM_GetSshIncomingPacketBuffer
 * PURPOSE:
 *          This function get buffer pointer for the incoming packet currently being processed.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **incoming_packet   --  buffer pointer for the incoming packet currently being processed.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshIncomingPacketBuffer(UI32_T my_tid, Buffer **incoming_packet);



/* FUNCTION NAME:  SSHD_OM_SetSshDataFellows
 * PURPOSE:
 *          This function set datafellows bug compatibility.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   datafellows --  datafellows bug compatibility.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshDataFellows(UI32_T my_tid, I32_T datafellows);



/* FUNCTION NAME:  SSHD_OM_GetSshDataFellows
 * PURPOSE:
 *          This function get datafellows bug compatibility.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *datafellows    --  datafellows bug compatibility.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshDataFellows(UI32_T my_tid, I32_T *datafellows);



/* FUNCTION NAME:  SSHD_OM_SetSshCompat20
 * PURPOSE:
 *          This function enabling compatibility mode for protocol 2.0 .
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshCompat20(UI32_T my_tid);



/* FUNCTION NAME:  SSHD_OM_GetSshCompat20
 * PURPOSE:
 *          This function get compatibility mode for protocol 2.0 .
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *compat20   --  compatibility mode for protocol 2.0 .
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshCompat20(UI32_T my_tid, I32_T *compat20);



/* FUNCTION NAME:  SSHD_OM_SetSshCompat13
 * PURPOSE:
 *          This function enabling compatibility mode for protocol 1.3 .
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshCompat13(UI32_T my_tid);



/* FUNCTION NAME:  SSHD_OM_GetSshCompat13
 * PURPOSE:
 *          This function get compatibility mode for protocol 1.3 .
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *compat13   --  compatibility mode for protocol 1.3 .
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshCompat13(UI32_T my_tid, I32_T *compat13);



/* FUNCTION NAME:  SSHD_OM_GetSshSensitiveData
 * PURPOSE:
 *			This function get ssh sensitive_data pointer.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          SSHD_SensitiveData_T    **sensitive_data    -- SSH sensitive_data pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshSensitiveData(UI32_T my_tid, SSHD_SensitiveData_T **sensitive_data);



/* FUNCTION NAME:  SSHD_OM_GetSshChannelsAllocNumber
 * PURPOSE:
 *			This function get ssh size of the channel array.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *channels_alloc -- size of the channel array.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshChannelsAllocNumber(UI32_T my_tid, I32_T *channels_alloc);



/* FUNCTION NAME:  SSHD_OM_SetSshChannelsAllocNumber
 * PURPOSE:
 *			This function set ssh size of the channel array.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   channels_alloc  -- size of the channel array.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshChannelsAllocNumber(UI32_T my_tid, I32_T channels_alloc);



/* FUNCTION NAME:  SSHD_OM_GetSshChannels
 * PURPOSE:
 *			This function get pointer of an array containing all allocated channels.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Channel ***channels -- Pointer to an array containing all allocated channels.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshChannels(UI32_T my_tid, Channel ***channels);



/* FUNCTION NAME:  SSHD_OM_SetSshChannels
 * PURPOSE:
 *			This function set pointer of an array containing all allocated channels.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          Channel **channels  -- Pointer to an array containing all allocated channels.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshChannels(UI32_T my_tid, Channel **channels);



/* FUNCTION NAME:  SSHD_OM_SetSshChannelMaxSock
 * PURPOSE:
 *			This function set maximum socket id value used in any of the channels.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   channel_max_fd  --  maximum socket id value used in any of the channels.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshChannelMaxSock(UI32_T my_tid, I32_T channel_max_fd);



/* FUNCTION NAME:  SSHD_OM_GetSshChannelMaxSock
 * PURPOSE:
 *			This function get maximum socket id value used in any of the channels.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *channel_max_fd --  maximum socket id value used in any of the channels.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshChannelMaxSock(UI32_T my_tid, I32_T *channel_max_fd);



/* FUNCTION NAME:  SSHD_OM_GetSshDispatch
 * PURPOSE:
 *			This function get pointer of an array containing all dispatch functions.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          dispatch_fn ***dispatch --  pointer to an array containing all dispatch functions.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshDispatch(UI32_T my_tid, dispatch_fn ***dispatch);



/* FUNCTION NAME:  SSHD_OM_SetSshRemoteProtocolFlags
 * PURPOSE:
 *			This function set protocol flags for the remote side for SSHv1.
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *          UI32_T  remote_protocol_flags   --  protocol flags for the remote side.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshRemoteProtocolFlags(UI32_T my_tid, UI32_T remote_protocol_flags);



/* FUNCTION NAME:  SSHD_OM_GetSshRemoteProtocolFlags
 * PURPOSE:
 *			This function get protocol flags for the remote side for SSHv1.
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *remote_protocol_flags  --  protocol flags for the remote side.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshRemoteProtocolFlags(UI32_T my_tid, UI32_T *remote_protocol_flags);



#if 0
/* FUNCTION NAME:  SSHD_OM_SetSshv1SessionKey
 * PURPOSE:
 *			This function set session key for protocol v1.
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI8_T   *ssh1_key   --  session key for SSHv1.
 *          UI32_T  ssh1_keylen --  session key len for SSHv1.
 *
 * OUTPUT:
 *          UI32_T  *remote_protocol_flags  --  protocol flags for the remote side.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshv1SessionKey(UI32_T my_tid, UI8_T *ssh1_key, UI32_T ssh1_keylen);
#endif



/* FUNCTION NAME:  SSHD_OM_SetKeeperTaskId
 * PURPOSE:
 *			Set task id which keep public key buffer.
 *
 * INPUT:
 *          UI32_T  tid --  task id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *			.
 */
BOOL_T SSHD_OM_SetKeeperTaskId(UI32_T tid);



/* FUNCTION NAME:  SSHD_OM_GetKeeperTaskId
 * PURPOSE:
 *          Get task id which keep public key buffer.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *tid    --  task id.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetKeeperTaskId(UI32_T *tid);



/* FUNCTION NAME : SSHD_OM_GetSshInteractiveMode
 * PURPOSE:
 *          This function get ssh interactive mode status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *interactive_mode   --  interactive mode.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshInteractiveMode(UI32_T my_tid, I32_T *interactive_mode);



/* FUNCTION NAME : SSHD_OM_SetSshInteractiveMode
 * PURPOSE:
 *          This function set ssh interactive mode status.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *          I32_T   interactive_mode    --  interactive mode.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshInteractiveMode(UI32_T my_tid, I32_T interactive_mode);



/* FUNCTION NAME : SSHD_OM_GetSshMaxPacketSize
 * PURPOSE:
 *          This function get ssh maximum packet size.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *max_packet_size    --  maximum packet size.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshMaxPacketSize(UI32_T my_tid, I32_T *max_packet_size);



/* FUNCTION NAME : SSHD_OM_SetSshMaxPacketSize
 * PURPOSE:
 *          This function set ssh maximum packet size.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          I32_T   max_packet_size --  maximum packet size.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshMaxPacketSize(UI32_T my_tid, I32_T max_packet_size);



/* FUNCTION NAME : SSHD_OM_GetSshChannelPre
 * PURPOSE:
 *          This function get array of channel_pre.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          chan_fn ***channel_pre  --  called just before select() to add any bits relevant to
 *                                      channels in the select bitmasks.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshChannelPre(UI32_T my_tid, chan_fn ***channel_pre);



/* FUNCTION NAME : SSHD_OM_GetSshChannelPost
 * PURPOSE:
 *          This function get array of channel_post.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          chan_fn ***channel_post --  perform any appropriate operations for channels which
 *                                      have events pending.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshChannelPost(UI32_T my_tid, chan_fn ***channel_post);



/* FUNCTION NAME : SSHD_OM_GetSshChannelHandlerInit
 * PURPOSE:
 *          This function get channel handler init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *did_init   --  have do channel handler init.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshChannelHandlerInit(UI32_T my_tid, I32_T *did_init);



/* FUNCTION NAME : SSHD_OM_SetSshChannelHandlerInit
 * PURPOSE:
 *          This function set channel handler init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   did_init    --  have do channel handler init.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshChannelHandlerInit(UI32_T my_tid, I32_T did_init);



/* FUNCTION NAME : SSHD_OM_GetSshHadChannel
 * PURPOSE:
 *          This function get channel have exist.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *had_channel    --  have channel exist.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshHadChannel(UI32_T my_tid, I32_T *had_channel);



/* FUNCTION NAME : SSHD_OM_SetSshHadChannel
 * PURPOSE:
 *          This function set channel have exist.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   had_channel --  have channel exist.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshHadChannel(UI32_T my_tid, I32_T had_channel);



/* FUNCTION NAME : SSHD_OM_GetSshStdioBuffer
 * PURPOSE:
 *          This function get buffer pointer of stdin, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          Buffer  **stdin_buffer  --  Buffer for stdin data.
 *          Buffer  **stdout_buffer --  Buffer for stdout data.
 *          Buffer  **stderr_buffer --  Buffer for stderr data.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshStdioBuffer(UI32_T my_tid, Buffer **stdin_buffer, Buffer **stdout_buffer, Buffer **stderr_buffer);



/* FUNCTION NAME : SSHD_OM_GetSshStdioDescriptor
 * PURPOSE:
 *          This function get descriptor for stdin, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *fdin   --  Descriptor for stdin (for writing)
 *          I32_T   *fdout  --  Descriptor for stdout (for reading)
 *          I32_T   *fderr  --  Descriptor for stderr.  May be -1.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshStdioDescriptor(UI32_T my_tid, I32_T *fdin, I32_T *fdout, I32_T *fderr);



/* FUNCTION NAME : SSHD_OM_SetSshStdioDescriptor
 * PURPOSE:
 *          This function get descriptor for stdin, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          I32_T   fdin    --  Descriptor for stdin (for writing)
 *          I32_T   fdout   --  Descriptor for stdout (for reading)
 *          I32_T   fderr   --  Descriptor for stderr.  May be -1.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshStdioDescriptor(UI32_T my_tid, I32_T fdin, I32_T fdout, I32_T fderr);



/* FUNCTION NAME : SSHD_OM_GetSshEofStatus
 * PURPOSE:
 *          This function get EOF status from client, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *stdin_eof  --  EOF message received from client.
 *          I32_T   *fdout_eof  --  EOF encountered reading from fdout.
 *          I32_T   *fderr_eof  --  EOF encountered readung from fderr.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshEofStatus(UI32_T my_tid, I32_T *stdin_eof, I32_T *fdout_eof, I32_T *fderr_eof);



/* FUNCTION NAME : SSHD_OM_SetSshEofStatus
 * PURPOSE:
 *          This function set EOF status from client, stdout, stderr.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   stdin_eof   --  EOF message received from client.
 *          I32_T   fdout_eof   --  EOF encountered reading from fdout.
 *          I32_T   fderr_eof   --  EOF encountered readung from fderr.
 *
 * OUTPUT:
 *          noen.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshEofStatus(UI32_T my_tid, I32_T stdin_eof, I32_T fdout_eof, I32_T fderr_eof);



/* FUNCTION NAME : SSHD_OM_SetSshConnectionClosed
 * PURPOSE:
 *          This function set ssh connection closed.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *          I32_T   connection_closed   --  connection close status.
 *
 * OUTPUT:
 *          noen.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshConnectionClosed(UI32_T my_tid, I32_T connection_closed);



/* FUNCTION NAME : SSHD_OM_GetSshConnectionClosed
 * PURPOSE:
 *          This function get ssh connection closed status.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *connection_closed  --  connection close status.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshConnectionClosed(UI32_T my_tid, I32_T *connection_closed);



/* FUNCTION NAME : SSHD_OM_GetSshBufferHigh
 * PURPOSE:
 *          This function get "Soft" max buffer size.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *buffer_high    --  "Soft" max buffer size.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshBufferHigh(UI32_T my_tid, UI32_T *buffer_high);



/* FUNCTION NAME : SSHD_OM_SetSshBufferHigh
 * PURPOSE:
 *          This function set "Soft" max buffer size.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI32_T  buffer_high --  "Soft" max buffer size.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshBufferHigh(UI32_T my_tid, UI32_T buffer_high);



/* FUNCTION NAME : SSHD_OM_GetSshClientAliveTimeouts
 * PURPOSE:
 *          This function get client alive timeouts.
 *
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *client_alive_timeouts  --  client alive timeouts.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshClientAliveTimeouts(UI32_T my_tid, I32_T *client_alive_timeouts);



/* FUNCTION NAME : SSHD_OM_SetSshClientAliveTimeouts
 * PURPOSE:
 *          This function set client alive timeouts.
 *
 * INPUT:
 *          UI32_T  my_tid                  --  caller tid.
 *          I32_T   client_alive_timeouts   --  client alive timeouts.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshClientAliveTimeouts(UI32_T my_tid, I32_T client_alive_timeouts);



/* FUNCTION NAME : SSHD_OM_SetSshAuthctxt
 * PURPOSE:
 *          This function set authenaction context.
 *
 * INPUT:
 *          UI32_T      my_tid      --  caller tid.
 *          Authctxt    *authctxt   --  pointer authenaction context.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshAuthctxt(UI32_T my_tid, Authctxt *authctxt);



/* FUNCTION NAME : SSHD_OM_GetSshAuthctxt
 * PURPOSE:
 *          This function get authenaction context.
 *
 * INPUT:
 *          UI32_T      my_tid      --  caller tid.
 *
 * OUTPUT:
 *          Authctxt    **authctxt  --  pointer authenaction context.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshAuthctxt(UI32_T my_tid, Authctxt **authctxt);



/* FUNCTION NAME : SSHD_OM_GetSshSessions
 * PURPOSE:
 *          This function get array of sessions.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          Session ***sessions --  array of sessions.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshSessions(UI32_T my_tid, Session **sessions);

/* FUNCTION NAME : SSHD_OM_CheckSshSessions
 * PURPOSE:
 *          This function check all ssh sessions to confirm
 *          whether the session with task id "my_tid" has been assigned tnsh_tid value.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          none
 *
 * RETURN:
 *          TRUE  - tnsh_tid value has been assigned.
 *          FALSE - tnsh_tid value has not been assigned.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_CheckSshSessions(UI32_T my_tid);



/* FUNCTION NAME : SSHD_OM_GetSshSessionNewInit
 * PURPOSE:
 *          This function get session array init init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          I32_T   *did_init   --  have session array init..
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshSessionNewInit(UI32_T my_tid, I32_T *did_init);



/* FUNCTION NAME : SSHD_OM_SetSshSessionNewInit
 * PURPOSE:
 *          This function set session array init status.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          I32_T   did_init    --  have do init session array.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshSessionNewInit(UI32_T my_tid, I32_T did_init);



/* FUNCTION NAME : SSHD_OM_GetSshServerKeyIndex
 * PURPOSE:
 *          This function get index of server key.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *index  --  index of server key.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshServerKeyIndex(UI32_T my_tid, UI32_T *index);



/* FUNCTION NAME : SSHD_OM_GetSshFatalCleanups
 * PURPOSE:
 *          This function get fatal cleanup function link list.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          struct fatal_cleanup    **fatal_cleanups    --  fatal cleanup function link list.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshFatalCleanups(UI32_T my_tid, struct fatal_cleanup **fatal_cleanups);



/* FUNCTION NAME : SSHD_OM_SetSshFatalCleanups
 * PURPOSE:
 *          This function set fatal cleanup function link list.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          struct fatal_cleanup    *fatal_cleanups --  fatal cleanup function link list.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshFatalCleanups(UI32_T my_tid, struct fatal_cleanup *fatal_cleanups);



/* FUNCTION NAME:  SSHD_OM_ClearSshServerOptions
 * PURPOSE:
 *			This function release ssh server options resource.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_ClearSshServerOptions(UI32_T my_tid);



/* FUNCTION NAME : SSHD_OM_GetSshReadSequenceNumber
 * PURPOSE:
 *          This function get read sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *read_seqnr --  read sequence number.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshReadSequenceNumber(UI32_T my_tid, UI32_T *read_seqnr);



/* FUNCTION NAME : SSHD_OM_SetSshReadSequenceNumber
 * PURPOSE:
 *          This function set read sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI32_T  read_seqnr  --  read sequence number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshReadSequenceNumber(UI32_T my_tid, UI32_T read_seqnr);



/* FUNCTION NAME : SSHD_OM_GetSshSendSequenceNumber
 * PURPOSE:
 *          This function get send sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *send_seqnr --  send sequence number.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetSshSendSequenceNumber(UI32_T my_tid, UI32_T *send_seqnr);



/* FUNCTION NAME : SSHD_OM_SetSshSendSequenceNumber
 * PURPOSE:
 *          This function set send sequence number.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI32_T  send_seqnr  --  send sequence number.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetSshSendSequenceNumber(UI32_T my_tid, UI32_T send_seqnr);



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionStatus()
 * PURPOSE:
 *          This function set status of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          SSHD_ConnectionState_T  --  current state of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionStatus(UI32_T my_tid, SSHD_ConnectionState_T state);



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionVersion
 * PURPOSE:
 *          This function set version of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          UI32_T  major   --  number of major version of connection.
 *          UI32_T  minor   --  number of minor version of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionVersion(UI32_T my_tid, UI32_T major, UI32_T minor);



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionUsername()
 * PURPOSE:
 *          This function set username of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid      --  caller tid.
 *          UI8_T   *username   --  username
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionUsername(UI32_T my_tid, UI8_T *username);



/* FUNCTION NAME : SSHD_OM_SetSessionPair
 * PURPOSE:
 *      Add a session pair to session record.
 *
 * INPUT:
 *          UI32_T  my_tid              --  caller tid.
 *          UI32_T  remote_tnsh_port    --  remote site port of TNSH (pty) session.
 *          UI32_T  tnsh_port           --  the local side port connect to TNSHD.
 *          UI32_T  user_local_port     --  local site port of SSHD (net) session.
 *          UI32_T  user_ip             --  the ip of remote site in socket.
 *          UI32_T  user_port           --  the port of remote site in socket.(net)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
BOOL_T SSHD_OM_SetSessionPair(UI32_T my_tid, UI32_T remote_tnsh_port, UI32_T tnsh_port,
                              UI32_T user_local_port,
                              const L_INET_AddrIp_T *user_addr, UI32_T user_port);



/* FUNCTION NAME : SSHD_OM_SetConnectionIDAndTnshID
 * PURPOSE:
 *      Set tnsh id and ssh connection id to session record.
 *
 * INPUT:
 *      UI32_T  tnsh_port   --  the port connect to TNSHD.
 *      UI32_T  tid         --  TNSH id.
 *      UI32_T  cid         --  ssh connection id.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - tnsh_port found and ssh server enabled, or don't found tnsh_port.
 *      FALSE - tnsh_port found and ssh server disabled.
 *
 * NOTES:
 *      This function invoked in SSHD_MGR_SetConnectionIDAndTnshID().
 */
BOOL_T SSHD_OM_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid);



/* FUNCTION NAME : SSHD_OM_GetSshConnectionId
 * PURPOSE:
 *      Get SSH pty connection id.
 *
 * INPUT:
 *      UI32_T  task_id     --  SSH task id.
 *
 * OUTPUT:
 *      UI32_T  *conn_id_p  --  SSH connection id.
 *
 * RETURN:
 *      TRUE  - Succeeded.
 *      FALSE - Failed.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_GetSshConnectionId(UI32_T task_id, UI32_T *conn_id_p);



/* FUNCTION NAME : SSHD_OM_CheckSshConnection
 * PURPOSE:
 *      Check connection is ssh or not.
 *
 * INPUT:
 *      UI32_T  cid --  connection id.
 *
 * OUTPUT:
 *
 * RETURN:
 *      TRUE  - This connection is ssh connection.
 *      FALSE - This connection is not ssh connection.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_CheckSshConnection(UI32_T cid);



/* FUNCTION NAME : SSHD_OM_SetConnectingTnshFd
 * PURPOSE:
 *      Set connecting tnsh socket ID to OM.
 *
 * INPUT:
 *      UI32_T  task_id --  SSH task id
 *      int  sock_id    --  socket id that connects
 *                          to tnsh
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_SetConnectingTnshFd(UI32_T task_id, int sock_id);



/* FUNCTION NAME : SSHD_OM_GetConnectingTnshFd
 * PURPOSE:
 *      Gets connecting tnsh socket ID from OM.
 *
 * INPUT:
 *      UI32_T  task_id --  SSH task id.
 *
 * OUTPUT:
 *      int  *tnsh_fd_p --  socket id that connects
 *                           to tnsh
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_GetConnectingTnshFd(UI32_T task_id, int *sock_id_p);



/* FUNCTION NAME : SSHD_OM_GetSshConnectionUsername
 * PURPOSE:
 *      Get username of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI8_T   *username   --  username.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_GetSshUsername
 */
BOOL_T SSHD_OM_GetSshConnectionUsername(UI32_T cid, UI8_T *username);



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionPassword
 * PURPOSE:
 *          This function set password of ssh connection.
 *
 * INPUT:
 *          UI32_T       my_tid      --  caller tid.
 *          const char   *password   --  password
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionPassword(
    UI32_T my_tid,
    const char *password
);



/* FUNCTION NAME : SSHD_OM_GetSshConnectionPassword
 * PURPOSE:
 *      Get password of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI8_T   *password   --  Buffer for username. The size of buffer MUST
 *                              larger than SYS_ADPT_MAX_PASSWORD_LEN+1.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_GetSshUsername
 */
BOOL_T SSHD_OM_GetSshConnectionPassword(
    UI32_T cid,
    char *password,
    UI32_T password_size);



/* FUNCTION NAME : SSHD_OM_SetSshConnectionPrivilege
 * PURPOSE:
 *      Set username privilege of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  my_tid      --  caller tid.
 *      UI32_T  privilege   --  privilege.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_VM_SetSshConnectionPrivilege
 */
BOOL_T SSHD_OM_SetSshConnectionPrivilege(UI32_T my_tid, UI32_T privilege);



/* FUNCTION NAME : SSHD_OM_GetSshConnectionPrivilege
 * PURPOSE:
 *      Get username privilege of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      UI32_T  *privilege  --  privilege.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      This API is invoked in SSHD_MGR_GetSshConnectionPrivilege
 */
BOOL_T SSHD_OM_GetSshConnectionPrivilege(UI32_T cid, UI32_T *privilege);



/* FUNCTION NAME : SSHD_OM_SetSshConnectionAuthResult
 * PURPOSE:
 *      Set authenticated result of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  my_tid      --  caller tid.
 *      USERAUTH_AuthResult_T auth_result_p   --  authenticated result.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      none.
 */
BOOL_T SSHD_OM_SetSshConnectionAuthResult(
    UI32_T my_tid,
    USERAUTH_AuthResult_T *auth_result_p
);



/* FUNCTION NAME : SSHD_OM_GetSshConnectionAuthResult
 * PURPOSE:
 *      Get authenticated result of ssh connection. Index  is connection id.
 *
 * INPUT:
 *      UI32_T  cid         --  connection_id.
 *
 * OUTPUT:
 *      USERAUTH_AuthResult_T  *auth_result_p  --  authenticated result.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_OM_GetSshConnectionAuthResult(
    UI32_T cid,
    USERAUTH_AuthResult_T *auth_result_p
);



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionCipher
 * PURPOSE:
 *          This function set cipher of ssh connection.
 *
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *          UI32_T  *cipher --  cipher of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionCipher(UI32_T my_tid, UI8_T *cipher);



/* FUNCTION NAME:  SSHD_OM_GetSshPacketLength
 * PURPOSE:
 *			This function get packet length.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI32_T  *packet_length --  packet length.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshPacketLength(UI32_T my_tid, UI32_T *packet_length);



/* FUNCTION NAME:  SSHD_OM_SetSshPacketLength
 * PURPOSE:
 *			This function set send sequence number.
 * INPUT:
 *          UI32_T  my_tid          --  caller tid.
 *          UI32_T  packet_length   --  packet_length.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshPacketLength(UI32_T my_tid, UI32_T packet_length);



/* FUNCTION NAME:  SSHD_OM_GetSshMacBuffer
 * PURPOSE:
 *          This function get mac buffer for mac_compute.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **mac_buffer    --  mac buffer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshMacBuffer(UI32_T my_tid, UI8_T **mac_buffer);



/* FUNCTION NAME:  SSHD_OM_GetSshKexgexHashDigest
 * PURPOSE:
 *          This function get digest of kexgex_hash.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **digest    --  digest of kexgex_hash.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshKexgexHashDigest(UI32_T my_tid, UI8_T **digest);



/* FUNCTION NAME:  SSHD_OM_GetSshkexDhHashDigest
 * PURPOSE:
 *          This function get digest of kex_dh_hash.
 * INPUT:
 *          UI32_T  my_tid  --  caller tid.
 *
 * OUTPUT:
 *          UI8_T   **digest    --  digest of kex_dh_hash.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_GetSshkexDhHashDigest(UI32_T my_tid, UI8_T **digest);



/* FUNCTION NAME : SSHD_OM_GetSessionPair
 * PURPOSE:
 *      Retrieve a session pair from session record.
 *
 * INPUT:
 *      UI32_T  -- the port connect to TNSHD.
 *
 * OUTPUT:
 *      UI32_T * -- the ip of remote site in socket.
 *      UI32_T * -- the port of remote site in socket.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *      This function invoked in CLI_TASK_SetSessionContext().
 */
BOOL_T SSHD_OM_GetSessionPair(UI32_T tnsh_port, L_INET_AddrIp_T *user_addr, UI32_T *user_port);



/* FUNCTION NAME : SSHD_OM_GetGenerateHostKeyAction
 * PURPOSE:
 *      Get tpye of host key generation.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of host key generation.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetGenerateHostKeyAction(UI32_T *action_type);



/* FUNCTION NAME : SSHD_OM_SetGenerateHostKeyAction
 * PURPOSE:
 *      Set tpye of host key generation.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of host key generation.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetGenerateHostKeyAction(UI32_T action_type);



/* FUNCTION NAME : SSHD_OM_GetGenerateHostKeyStatus
 * PURPOSE:
 *      Get result of host key generation.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of host key generation.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetGenerateHostKeyStatus(UI32_T *action_result);



/* FUNCTION NAME : SSHD_OM_SetGenerateHostKeyStatus
 * PURPOSE:
 *      Set result of host key generation.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of host key generation.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetGenerateHostKeyStatus(UI32_T action_result);



/* FUNCTION NAME : SSHD_OM_GetDeleteUserPublicKeyAction
 * PURPOSE:
 *      Get tpye of user key delete.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of user key delete.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetDeleteUserPublicKeyAction(UI32_T *action_type);



/* FUNCTION NAME : SSHD_OM_SetDeleteUserPublicKeyAction
 * PURPOSE:
 *      Set tpye of user key delete.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of user key delete.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetDeleteUserPublicKeyAction(UI32_T action_type);



/* FUNCTION NAME : SSHD_OM_GetDeleteUserPublicKeyStatus
 * PURPOSE:
 *      Get result of user key delete.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of user key delete.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetDeleteUserPublicKeyStatus(UI32_T *action_result);



/* FUNCTION NAME : SSHD_OM_SetDeleteUserPublicKeyStatus
 * PURPOSE:
 *      Set result of user key delete.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of user key delete.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetDeleteUserPublicKeyStatus(UI32_T action_result);



/* FUNCTION NAME : SSHD_OM_GetWriteHostKey2FlashAction
 * PURPOSE:
 *      Get tpye of host key writing.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_type    --  tpye of host key writing.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetWriteHostKey2FlashAction(UI32_T *action_type);



/* FUNCTION NAME : SSHD_OM_SetWriteHostKey2FlashAction
 * PURPOSE:
 *      Set tpye of host key writing.
 *
 * INPUT:
 *      UI32_T  action_type --  tpye of host key writing.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetWriteHostKey2FlashAction(UI32_T action_type);



/* FUNCTION NAME : SSHD_OM_GetWriteHostKey2FlashStatus
 * PURPOSE:
 *      Get result of host key writing.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      UI32_T  *action_result  --  result of host key writing.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_GetWriteHostKey2FlashStatus(UI32_T *action_result);



/* FUNCTION NAME : SSHD_OM_SetWriteHostKey2FlashStatus
 * PURPOSE:
 *      Set result of host key writing.
 *
 * INPUT:
 *      UI32_T  action_result   --  result of host key writing.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetWriteHostKey2FlashStatus(UI32_T action_result);



/* FUNCTION NAME:  SSHD_OM_SetSshPasswordAuthenticationStatus
 * PURPOSE:
 *          This function set password authentication state.
 *
 * INPUT:
 *          SSHD_PasswordAuthenticationStatus_T state   --  Password Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_SetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T state);



/* FUNCTION NAME:  SSHD_OM_GetSshPasswordAuthenticationStatus
 * PURPOSE:
 *          This function get password authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_PasswordAuthenticationStatus_T *state  --  Password Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_GetSshPasswordAuthenticationStatus(SSHD_PasswordAuthenticationStatus_T *state);



/* FUNCTION NAME:  SSHD_OM_SetSshPubkeyAuthenticationStatus
 * PURPOSE:
 *          This function set password authentication state.
 *
 * INPUT:
 *          SSHD_PubkeyAuthenticationStatus_T state   --  Pubkey Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_SetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T state);



/* FUNCTION NAME:  SSHD_OM_GetSshPubkeyAuthenticationStatus
 * PURPOSE:
 *          This function get pubkey authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_PubkeyAuthenticationStatus_T *state  --  Pubkey Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_GetSshPubkeyAuthenticationStatus(SSHD_PubkeyAuthenticationStatus_T *state);



/* FUNCTION NAME:  SSHD_OM_SetSshRsaAuthenticationStatus
 * PURPOSE:
 *          This function set rsa authentication state.
 *
 * INPUT:
 *          SSHD_RsaAuthenticationStatus_T state   --  Rsa Authentication Status.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_SetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T state);



/* FUNCTION NAME:  SSHD_OM_GetSshRsaAuthenticationStatus
 * PURPOSE:
 *          This function get rsa authentication state.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_RsaAuthenticationStatus_T *state  --  Rsa Authentication Status.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command.
 */
BOOL_T SSHD_OM_GetSshRsaAuthenticationStatus(SSHD_RsaAuthenticationStatus_T *state);







#endif /* #ifndef SSHD_OM_H */



