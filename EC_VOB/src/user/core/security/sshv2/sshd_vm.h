/* MODULE NAME:  sshd_vm.h
* PURPOSE:
*   Initialize the resource and provide some functions for the sshd module.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-04-11      -- Isiah , created.
*
* Copyright(C)      Accton Corporation, 2003
*/

#ifndef SSHD_VM_H

#define SSHD_VM_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_inet.h"
#include "sshd_type.h"
#include "sshd_om.h"



/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  SSHD_VM_SshExit
 * PURPOSE:
 *			This function release resource and kill task.
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
BOOL_T SSHD_VM_SshExit(void);



/* FUNCTION NAME:  SSHD_VM_GetSshServerOptions
 * PURPOSE:
 *			This function get ssh server options pointer.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          ServerOptions   **options   -- SSH server options pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshServerOptions(ServerOptions **options);



/* FUNCTION NAME:  SSHD_VM_SetSshServerOptions
 * PURPOSE:
 *			This function alloc ssh server options pointer and set owner tid.
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
BOOL_T SSHD_VM_SetSshServerOptions(void);



/* FUNCTION NAME:  SSHD_VM_SetSshServerVersionString
 * PURPOSE:
 *			This function set ssh server version string.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshServerVersionString(I8_T *version_string);



/* FUNCTION NAME:  SSHD_VM_SetSshClientVersionString
 * PURPOSE:
 *			This function set ssh client version string.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshClientVersionString(I8_T *version_string);



/* FUNCTION NAME:  SSHD_VM_SetSshSessionId
 * PURPOSE:
 *			This function set ssh session id for sshv1.
 * INPUT:
 *          UI8_T    *session_id --  session id.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshSessionId(UI8_T *session_id);



/* FUNCTION NAME:  SSHD_VM_GetSshSessionId
 * PURPOSE:
 *			This function get ssh session id for sshv1.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T    *session_id --  session id.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshSessionId(UI8_T *session_id);



/* FUNCTION NAME:  SSHD_VM_GetSshServerVersionString
 * PURPOSE:
 *			This function get ssh server version string pointer.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I8_T    **version_string    --  server version string pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshServerVersionString(I8_T **version_string);



/* FUNCTION NAME:  SSHD_VM_GetSshClientVersionString
 * PURPOSE:
 *			This function get ssh client version string pointer.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I8_T    **version_string    --  client version string pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshClientVersionString(I8_T **version_string);



/* FUNCTION NAME:  SSHD_VM_SetSshKex
 * PURPOSE:
 *			This function set ssh kex pointer.
 * INPUT:
 *          Kex *xxx_kex    --  kex pointer.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshKex(Kex *xxx_kex);



/* FUNCTION NAME:  SSHD_VM_GetSshKex
 * PURPOSE:
 *			This function get ssh kex pointer.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          Kex **xxx_kex   --  kex pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshKex(Kex **xxx_kex);



/* FUNCTION NAME:  SSHD_VM_SetSshSessionId2
 * PURPOSE:
 *			This function set ssh session id and id length for sshv2.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshSessionId2(UI8_T *session_id2, I32_T session_id2_len);



/* FUNCTION NAME:  SSHD_VM_GetSshSessionId2
 * PURPOSE:
 *			This function get ssh session id and id length for sshv2.
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshSessionId2(UI8_T **session_id2, I32_T *session_id2_len);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionSockId
 * PURPOSE:
 *          This function set socket id for communicating with
 *          the other side.  connection_in is used for reading;
 *          connection_out for writing.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshConnectionSockId(I32_T connection_in, I32_T connection_out);



/* FUNCTION NAME:  SSHD_VM_GetSshConnectionSockId
 * PURPOSE:
 *          This function get socket id for communicating with
 *          the other side.  connection_in is used for reading;
 *          connection_out for writing.
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshConnectionSockId(I32_T *connection_in, I32_T *connection_out);



/* FUNCTION NAME:  SSHD_VM_GetSshSendContext
 * PURPOSE:
 *			This function get ssh connection cipher context for sending.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          CipherContext   **send_context   --  cipher context for sending.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshSendContext(CipherContext **send_context);



/* FUNCTION NAME:  SSHD_VM_GetSshReceiveContext
 * PURPOSE:
 *			This function get ssh connection cipher context for receiving.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          CipherContext   **Receive_context   --  cipher context for receiving.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshReceiveContext(CipherContext **receive_context);



/* FUNCTION NAME:  SSHD_VM_GetSshCurrentKeys
 * PURPOSE:
 *          This function get pointer of new Session key information for Encryption and MAC.
 *          MODE_IN is used for reading, MODE_OUT is used for writing.
 * INPUT:
 *
 * OUTPUT:
 *          Newkeys ***current_keys --  pointer of Session key information for Encryption and MAC.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshCurrentKeys(Newkeys ***current_keys);



/* FUNCTION NAME:  SSHD_VM_GetSshNewKey
 * PURPOSE:
 *          This function set pointer of Session key information for Encryption and MAC.
 *          MODE_IN is used for reading, MODE_OUT is used for writing.
 * INPUT:
 *
 * OUTPUT:
 *          Newkeys ***newkeys  --  pointer of Session key information for Encryption and MAC.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshNewKey(Newkeys ***newkeys);



/* FUNCTION NAME:  SSHD_VM_GetSshInputBuffer
 * PURPOSE:
 *          This function get buffer pointer for raw input data from the socket.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          Buffer  **input --  buffer pointer for raw input data from the socket.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshInputBuffer(Buffer **input);



/* FUNCTION NAME:  SSHD_VM_GetSshOutputBuffer
 * PURPOSE:
 *          This function get buffer pointer for raw output data going to the socket.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          Buffer  **output    --  buffer pointer for raw output data going to the socket.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshOutputBuffer(Buffer **output);



/* FUNCTION NAME:  SSHD_VM_GetSshOutgoingPacketBuffer
 * PURPOSE:
 *          This function get buffer pointer for the partial outgoing packet being constructed.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          Buffer  **outgoing_packet   --  buffer pointer for the partial outgoing packet being constructed.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshOutgoingPacketBuffer(Buffer **outgoing_packet);



/* FUNCTION NAME:  SSHD_VM_GetSshIncomingPacketBuffer
 * PURPOSE:
 *          This function get buffer pointer for the incoming packet currently being processed.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          Buffer  **incoming_packet   --  buffer pointer for the incoming packet currently being processed.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshIncomingPacketBuffer(Buffer **incoming_packet);



/* FUNCTION NAME:  SSHD_VM_SetSshDataFellows
 * PURPOSE:
 *          This function set datafellows bug compatibility.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshDataFellows(I32_T datafellows);



/* FUNCTION NAME:  SSHD_VM_GetSshDataFellows
 * PURPOSE:
 *          This function get datafellows bug compatibility.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I32_T   *datafellows    --  datafellows bug compatibility.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshDataFellows(I32_T *datafellows);



/* FUNCTION NAME:  SSHD_VM_SetSshCompat20
 * PURPOSE:
 *          This function enabling compatibility mode for protocol 2.0 .
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
BOOL_T SSHD_VM_SetSshCompat20(void);



/* FUNCTION NAME:  SSHD_VM_GetSshCompat20
 * PURPOSE:
 *          This function get compatibility mode for protocol 2.0 .
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I32_T   *compat20   --  compatibility mode for protocol 2.0 .
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshCompat20(I32_T *compat20);



/* FUNCTION NAME:  SSHD_VM_SetSshCompat13
 * PURPOSE:
 *          This function enabling compatibility mode for protocol 1.3 .
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
BOOL_T SSHD_VM_SetSshCompat13(void);



/* FUNCTION NAME:  SSHD_VM_GetSshCompat13
 * PURPOSE:
 *          This function get compatibility mode for protocol 1.3 .
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I32_T   *compat13   --  compatibility mode for protocol 1.3 .
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshCompat13(I32_T *compat13);



/* FUNCTION NAME:  SSHD_VM_GetSshSensitiveData
 * PURPOSE:
 *			This function get ssh sensitive_data pointer.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          SSHD_SensitiveData_T    **sensitive_data    -- SSH sensitive_data pointer.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshSensitiveData(SSHD_SensitiveData_T **sensitive_data);



/* FUNCTION NAME:  SSHD_VM_GetSshChannelsAllocNumber
 * PURPOSE:
 *			This function get ssh size of the channel array.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I32_T   *channels_alloc -- size of the channel array.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshChannelsAllocNumber(I32_T *channels_alloc);



/* FUNCTION NAME:  SSHD_VM_SetSshChannelsAllocNumber
 * PURPOSE:
 *			This function set ssh size of the channel array.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshChannelsAllocNumber(I32_T channels_alloc);



/* FUNCTION NAME:  SSHD_VM_GetSshChannels
 * PURPOSE:
 *			This function get pointer of an array containing all allocated channels.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          Channel ***channels -- Pointer to an array containing all allocated channels.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshChannels(Channel ***channels);



/* FUNCTION NAME:  SSHD_VM_SetSshChannels
 * PURPOSE:
 *			This function set pointer of an array containing all allocated channels.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshChannels(Channel **channels);



/* FUNCTION NAME:  SSHD_VM_SetSshChannelMaxSock
 * PURPOSE:
 *			This function set maximum socket id value used in any of the channels.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshChannelMaxSock(I32_T channel_max_fd);



/* FUNCTION NAME:  SSHD_VM_GetSshChannelMaxSock
 * PURPOSE:
 *			This function get maximum socket id value used in any of the channels.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I32_T   *channel_max_fd --  maximum socket id value used in any of the channels.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshChannelMaxSock(I32_T *channel_max_fd);



/* FUNCTION NAME:  SSHD_VM_GetSshDispatch
 * PURPOSE:
 *			This function get pointer of an array containing all dispatch functions.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          dispatch_fn ***dispatch --  pointer to an array containing all dispatch functions.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshDispatch(dispatch_fn ***dispatch);



/* FUNCTION NAME:  SSHD_VM_SetSshRemoteProtocolFlags
 * PURPOSE:
 *			This function set protocol flags for the remote side for SSHv1.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshRemoteProtocolFlags(UI32_T remote_protocol_flags);



/* FUNCTION NAME:  SSHD_VM_GetSshRemoteProtocolFlags
 * PURPOSE:
 *			This function get protocol flags for the remote side for SSHv1.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *remote_protocol_flags  --  protocol flags for the remote side.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetSshRemoteProtocolFlags(UI32_T *remote_protocol_flags);

/* FUNCTION NAME:  SSHD_VM_AddRemoteAddrToBlockCache
 * PURPOSE:
 *          Add remote address to block cache.
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
BOOL_T SSHD_VM_AddRemoteAddrToBlockCache();

#if 0
/* FUNCTION NAME:  SSHD_VM_SetSshv1SessionKey
 * PURPOSE:
 *			This function set session key for protocol v1.
 * INPUT:
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
BOOL_T SSHD_VM_SetSshv1SessionKey(UI8_T *ssh1_key, UI32_T ssh1_keylen);
#endif



/* FUNCTION NAME:  SSHD_VM_GetAuthenticationRetries
 * PURPOSE:
 *			This function get number of authentication retries.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *retries    --  number of authentication retries.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetAuthenticationRetries(UI32_T *retries);



/* FUNCTION NAME : SSHD_VM_GetUserPublicKey
 * PURPOSE:
 *      Get user's public key.
 *
 * INPUT:
 *      UI_8_T  *username   --  username.
 *      UI32_t  key_type    --  Type of host key.(KEY_TYPE_RSA, KEY_TYPE_DSA).
 *
 * OUTPUT:
 *      UI8_T   *file       --  pointer of buffer to storage public key.
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetUserPublicKey(UI8_T *username, UI32_T key_type, UI8_T *file);



/* FUNCTION NAME : SSHD_VM_GetSshInteractiveMode
 * PURPOSE:
 *          This function get ssh interactive mode status.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          I32_T   *interactive_mode   --  interactive mode.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetSshInteractiveMode(I32_T *interactive_mode);



/* FUNCTION NAME : SSHD_VM_SetSshInteractiveMode
 * PURPOSE:
 *          This function set ssh interactive mode status.
 *
 * INPUT:
 *          I32_T   interactive_mode    --  interactive mode.
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
BOOL_T SSHD_VM_SetSshInteractiveMode(I32_T interactive_mode);



/* FUNCTION NAME : SSHD_VM_GetSshMaxPacketSize
 * PURPOSE:
 *          This function get ssh maximum packet size.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshMaxPacketSize(I32_T *max_packet_size);



/* FUNCTION NAME : SSHD_VM_SetSshMaxPacketSize
 * PURPOSE:
 *          This function set ssh maximum packet size.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshMaxPacketSize(I32_T max_packet_size);



/* FUNCTION NAME : SSHD_VM_GetSshChannelPre
 * PURPOSE:
 *          This function get array of channel_pre.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshChannelPre(chan_fn ***channel_pre);



/* FUNCTION NAME : SSHD_VM_GetSshChannelPost
 * PURPOSE:
 *          This function get array of channel_post.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshChannelPost(chan_fn ***channel_post);



/* FUNCTION NAME : SSHD_VM_GetSshChannelHandlerInit
 * PURPOSE:
 *          This function get channel handler init status.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshChannelHandlerInit(I32_T *did_init);



/* FUNCTION NAME : SSHD_VM_SetSshChannelHandlerInit
 * PURPOSE:
 *          This function set channel handler init status.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshChannelHandlerInit(I32_T did_init);



/* FUNCTION NAME : SSHD_VM_GetSshHadChannel
 * PURPOSE:
 *          This function get channel have exist.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshHadChannel(I32_T *had_channel);



/* FUNCTION NAME : SSHD_VM_SetSshHadChannel
 * PURPOSE:
 *          This function set channel have exist.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshHadChannel(I32_T had_channel);



/* FUNCTION NAME : SSHD_VM_GetSshStdioBuffer
 * PURPOSE:
 *          This function get buffer pointer of stdin, stdout, stderr.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshStdioBuffer(Buffer **stdin_buffer, Buffer **stdout_buffer, Buffer **stderr_buffer);



/* FUNCTION NAME : SSHD_VM_GetSshStdioDescriptor
 * PURPOSE:
 *          This function get descriptor for stdin, stdout, stderr.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshStdioDescriptor(I32_T *fdin, I32_T *fdout, I32_T *fderr);



/* FUNCTION NAME : SSHD_VM_SetSshStdioDescriptor
 * PURPOSE:
 *          This function get descriptor for stdin, stdout, stderr.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshStdioDescriptor(I32_T fdin, I32_T fdout, I32_T fderr);



/* FUNCTION NAME : SSHD_VM_GetSshEofStatus
 * PURPOSE:
 *          This function get EOF status from client, stdout, stderr.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshEofStatus(I32_T *stdin_eof, I32_T *fdout_eof, I32_T *fderr_eof);



/* FUNCTION NAME : SSHD_VM_SetSshEofStatus
 * PURPOSE:
 *          This function set EOF status from client, stdout, stderr.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshEofStatus(I32_T stdin_eof, I32_T fdout_eof, I32_T fderr_eof);



/* FUNCTION NAME : SSHD_VM_SetSshConnectionClosed
 * PURPOSE:
 *          This function set ssh connection closed.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshConnectionClosed(I32_T connection_closed);



/* FUNCTION NAME : SSHD_VM_GetSshConnectionClosed
 * PURPOSE:
 *          This function get ssh connection closed status.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshConnectionClosed(I32_T *connection_closed);



/* FUNCTION NAME : SSHD_VM_GetSshBufferHigh
 * PURPOSE:
 *          This function get "Soft" max buffer size.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshBufferHigh(UI32_T *buffer_high);



/* FUNCTION NAME : SSHD_VM_SetSshBufferHigh
 * PURPOSE:
 *          This function set "Soft" max buffer size.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshBufferHigh(UI32_T buffer_high);



/* FUNCTION NAME : SSHD_VM_GetSshClientAliveTimeouts
 * PURPOSE:
 *          This function get client alive timeouts.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshClientAliveTimeouts(I32_T *client_alive_timeouts);



/* FUNCTION NAME : SSHD_VM_SetSshClientAliveTimeouts
 * PURPOSE:
 *          This function set client alive timeouts.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshClientAliveTimeouts(I32_T client_alive_timeouts);



/* FUNCTION NAME : SSHD_VM_SetSshAuthctxt
 * PURPOSE:
 *          This function set authenaction context.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshAuthctxt(Authctxt *authctxt);



/* FUNCTION NAME : SSHD_VM_GetSshAuthctxt
 * PURPOSE:
 *          This function get authenaction context.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshAuthctxt(Authctxt **authctxt);



/* FUNCTION NAME : SSHD_VM_GetSshSessions
 * PURPOSE:
 *          This function get array of sessions.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshSessions(Session **sessions);



/* FUNCTION NAME : SSHD_VM_GetSshSessionNewInit
 * PURPOSE:
 *          This function get session array init init status.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshSessionNewInit(I32_T *did_init);



/* FUNCTION NAME : SSHD_VM_SetSshSessionNewInit
 * PURPOSE:
 *          This function set session array init status.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshSessionNewInit(I32_T did_init);



/* FUNCTION NAME : SSHD_VM_GetSshServerKeyIndex
 * PURPOSE:
 *          This function get index of server key.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshServerKeyIndex(UI32_T *index);



/* FUNCTION NAME : SSHD_VM_GetSshFatalCleanups
 * PURPOSE:
 *          This function get fatal cleanup function link list.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshFatalCleanups(struct fatal_cleanup **fatal_cleanups);



/* FUNCTION NAME : SSHD_VM_SetSshFatalCleanups
 * PURPOSE:
 *          This function set fatal cleanup function link list.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshFatalCleanups(struct fatal_cleanup *fatal_cleanups);



/* FUNCTION NAME:  SSHD_VM_ClearSshServerOptions
 * PURPOSE:
 *			This function release ssh server options resource.
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
BOOL_T SSHD_VM_ClearSshServerOptions(void);



/* FUNCTION NAME : SSHD_VM_GetSshReadSequenceNumber
 * PURPOSE:
 *          This function get read sequence number.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshReadSequenceNumber(UI32_T *read_seqnr);



/* FUNCTION NAME : SSHD_VM_SetSshReadSequenceNumber
 * PURPOSE:
 *          This function set read sequence number.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshReadSequenceNumber(UI32_T read_seqnr);



/* FUNCTION NAME : SSHD_VM_GetSshSendSequenceNumber
 * PURPOSE:
 *          This function get send sequence number.
 *
 * INPUT:
 *          none.
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
BOOL_T SSHD_VM_GetSshSendSequenceNumber(UI32_T *send_seqnr);



/* FUNCTION NAME : SSHD_VM_SetSshSendSequenceNumber
 * PURPOSE:
 *          This function set send sequence number.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshSendSequenceNumber(UI32_T send_seqnr);



/* FUNCTION NAME:  SSHD_VM_GetNegotiationTimeout
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
UI32_T SSHD_VM_GetNegotiationTimeout(void);



/* FUNCTION NAME:  SSHD_VM_GetCreatedSessionNumber
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
UI32_T SSHD_VM_GetCreatedSessionNumber(void);



/* FUNCTION NAME:  SSHD_VM_SetCreatedSessionNumber
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
void SSHD_VM_SetCreatedSessionNumber(UI32_T number);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionStatus()
 * PURPOSE:
 *          This function set status of ssh connection.
 *
 * INPUT:
 *          SSHD_ConnectionState_T  state   --  current state of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshConnectionStatus(SSHD_ConnectionState_T state);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionVersion()
 * PURPOSE:
 *          This function set version of ssh connection.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshConnectionVersion(UI32_T major, UI32_T minor);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionUsername()
 * PURPOSE:
 *          This function set username of ssh connection.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshConnectionUsername(UI8_T *username);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionPassword
 * PURPOSE:
 *          This function set password of ssh connection.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshConnectionPassword(const char *password);



/* FUNCTION NAME : SSHD_VM_SetSshConnectionPrivilege
 * PURPOSE:
 *      Set username privilege of ssh connection. Index  is connection id.
 *
 * INPUT:
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
 *      This API is invoked in SSHD
 */
BOOL_T SSHD_VM_SetSshConnectionAuthResult(USERAUTH_AuthResult_T *auth_result_p);



/* FUNCTION NAME:  SSHD_VM_GetLocalSessionName()
 * PURPOSE:
 *          This function get local site session (ip address ,port) according socket-id.
 *
 * INPUT:
 *          UI32_T - socket id.
 *
 * OUTPUT:
 *          UI32_T * - ip address.
 *          UI32_T * - number of port.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetLocalSessionName(UI32_T sock_id, UI32_T *ip, UI32_T *port);



/* FUNCTION NAME:  SSHD_VM_GetRemoteSessionName()
 * PURPOSE:
 *          This function get remote site session (ip address ,port) according socket-id.
 *
 * INPUT:
 *          UI32_T - socket id.
 *
 * OUTPUT:
 *          UI32_T * - ip address.
 *          UI32_T * - number of port.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetRemoteSessionName(UI32_T sock_id, L_INET_AddrIp_T *ip, UI32_T *port);



/* FUNCTION NAME : SSHD_VM_GetSockPort
 * PURPOSE:
 *      Get port number from sockaddr.
 *
 * INPUT:
 *      sa - sockaddr.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      port number.
 *
 * NOTES:
 *      None.
 */
UI32_T SSHD_VM_GetSockPort(struct sockaddr *sa);



/* FUNCTION NAME : SSHD_VM_SetSessionPair
 * PURPOSE:
 *      Add a session pair to session record.
 *
 * INPUT:
 *      net  -- socket id for SSH client.
 *      pty  -- socket id for tnsh.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE to indicate successful and FALSE to indicate failure.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_VM_SetSessionPair(int net, int pty);



/* FUNCTION NAME : SSHD_VM_GetSshConnectionId
 * PURPOSE:
 *      Get SSH connection id.
 *
 * INPUT:
 *      None.
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
BOOL_T SSHD_VM_GetSshConnectionId(UI32_T *conn_id_p);



/* FUNCTION NAME : SSHD_VM_CheckSshConnection
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
BOOL_T SSHD_VM_CheckSshConnection(UI32_T cid);



/* FUNCTION NAME : SSHD_VM_CleanUpConnectingTnshFd
 * PURPOSE:
 *      Callback function for clean up the socket that
 *      connects to tnsh task.
 *
 * INPUT:
 *      context_p  --  No use
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void SSHD_VM_CleanUpConnectingTnshFd(void *context_p);



/* FUNCTION NAME : SSHD_VM_SetConnectingTnshFd
 * PURPOSE:
 *      Set connecting tnsh socket ID to OM.
 *
 * INPUT:
 *      int  tnsh_fd --  socket id that connects
 *                       to tnsh
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
BOOL_T SSHD_VM_SetConnectingTnshFd(int tnsh_fd);



/* FUNCTION NAME : SSHD_VM_GetConnectingTnshFd
 * PURPOSE:
 *      Gets connecting tnsh socket ID from OM.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      int  *tnsh_fd_p --  socket id that connects
 *                          to tnsh
 *
 * RETURN:
 *      TRUE  - Success.
 *      FALSE - Fault.
 *
 * NOTES:
 *      None.
 */
BOOL_T SSHD_VM_GetConnectingTnshFd(int *tnsh_fd_p);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionCipher
 * PURPOSE:
 *          This function set cipher of ssh connection.
 *
 * INPUT:
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
BOOL_T SSHD_VM_SetSshConnectionCipher(char *cipher);



/* FUNCTION NAME : SSHD_VM_GetSshPacketLength
 * PURPOSE:
 *          This function get packet length.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI32_T  *packet_length --  packet length.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetSshPacketLength(UI32_T *packet_length);



/* FUNCTION NAME : SSHD_VM_SetSshPacketLength
 * PURPOSE:
 *          This function set send sequence number.
 *
 * INPUT:
 *          UI32_T  packet_length  --  packet_length.
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
BOOL_T SSHD_VM_SetSshPacketLength(UI32_T packet_length);



/* FUNCTION NAME : SSHD_VM_GetSshMacBuffer
 * PURPOSE:
 *          This function get mac buffer for mac_compute.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   **mac_buffer    --  mac buffer.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetSshMacBuffer(UI8_T **mac_buffer);



/* FUNCTION NAME : SSHD_VM_GetSshKexgexHashDigest
 * PURPOSE:
 *          This function get digest of kexgex_hash.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   **digest    --  digest of kexgex_hash.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetSshKexgexHashDigest(UI8_T **digest);



/* FUNCTION NAME : SSHD_VM_GetSshkexDhHashDigest
 * PURPOSE:
 *          This function get digest of kex_dh_hash.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          UI8_T   **digest    --  digest of kex_dh_hash.
 *
 * RETURN:
 *          TRUE  - Success.
 *          FALSE - failure.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetSshkexDhHashDigest(UI8_T **digest);



/* FUNCTION NAME:  SSHD_VM_do_convert_from_ssh2
 * PURPOSE:
 *          This function convert user public key from ssh2.
 *
 * INPUT:
 *          Key     *key    --  user public key from client.
 *          cahr    *file   --  user public key from database.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_do_convert_from_ssh2(Key *key, char *file);















#endif /* #ifndef SSHD_VM_H */
