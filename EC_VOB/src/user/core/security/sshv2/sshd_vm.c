/* MODULE NAME:  sshd_vm.c
* PURPOSE:
*   Initialize the resource and provide some functions for the sshd module.
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-04-11      -- Isiah , created.
*     2007-06-25      -- Rich Lee, Porting to Linux Platform
* Copyright(C)      Accton Corporation, 2003
*/



/* INCLUDE FILE DECLARATIONS
 */
#include <unistd.h>
#define s_close(fd) close(fd)

#include "sys_type.h"
//#include "skt_vx.h"
#include "l_stdlib.h"
//#include "socket.h"
//#include "sshd_mgr.h"
#include "sshd_om.h"
#include "sysfun.h"
#include "sys_mgr.h"     /* for telnet silent-time */
#include "sys_pmgr.h"    /* for telnet silent-time */
#include "mgmt_ip_flt.h" /* for telnet silent-time */
//#include "rsa.h"
//#include "sshd.h"
#include "log.h"
#include "keygen_type.h"
#include "sshd_vm.h"
//#include "sshd_record.h"
#include "keygen_mgr.h"
#include "uuencode.h"



/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_VM_SshExit
 * PURPOSE:
 *          This function release resource and kill task.
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
BOOL_T SSHD_VM_SshExit(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
/* this function will not support in Linux
 */
   /* SYSFUN_DeleteTask(0);*/
    return TRUE;
}



/* FUNCTION NAME:  SSHD_VM_GetSshServerOptions
 * PURPOSE:
 *          This function get ssh server options pointer.
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
BOOL_T SSHD_VM_GetSshServerOptions(ServerOptions **options)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshServerOptions(my_tid, options);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshServerOptions
 * PURPOSE:
 *          This function alloc ssh server options pointer and set owner tid.
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
BOOL_T SSHD_VM_SetSshServerOptions(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshServerOptions(my_tid);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshServerVersionString
 * PURPOSE:
 *          This function set ssh server version string.
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
BOOL_T SSHD_VM_SetSshServerVersionString(I8_T *version_string)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshServerVersionString(my_tid, version_string);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshClientVersionString
 * PURPOSE:
 *          This function set ssh client version string.
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
BOOL_T SSHD_VM_SetSshClientVersionString(I8_T *version_string)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshClientVersionString(my_tid, version_string);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshSessionId
 * PURPOSE:
 *          This function set ssh session id for sshv1.
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
BOOL_T SSHD_VM_SetSshSessionId(UI8_T *session_id)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshSessionId(my_tid, session_id);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshSessionId
 * PURPOSE:
 *          This function get ssh session id for sshv1.
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
BOOL_T SSHD_VM_GetSshSessionId(UI8_T *session_id)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshSessionId(my_tid, session_id);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshServerVersionString
 * PURPOSE:
 *          This function get ssh server version string pointer.
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
BOOL_T SSHD_VM_GetSshServerVersionString(I8_T **version_string)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshServerVersionString(my_tid, version_string);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshClientVersionString
 * PURPOSE:
 *          This function get ssh client version string pointer.
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
BOOL_T SSHD_VM_GetSshClientVersionString(I8_T **version_string)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshClientVersionString(my_tid, version_string);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshKex
 * PURPOSE:
 *          This function set ssh kex pointer.
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
BOOL_T SSHD_VM_SetSshKex(Kex *xxx_kex)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshKex(my_tid, xxx_kex);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshKex
 * PURPOSE:
 *          This function get ssh kex pointer.
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
BOOL_T SSHD_VM_GetSshKex(Kex **xxx_kex)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshKex(my_tid, xxx_kex);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshSessionId2
 * PURPOSE:
 *          This function set ssh session id and id length for sshv2.
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
BOOL_T SSHD_VM_SetSshSessionId2(UI8_T *session_id2, I32_T session_id2_len)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshSessionId2(my_tid, session_id2, session_id2_len);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshSessionId2
 * PURPOSE:
 *          This function get ssh session id and id length for sshv2.
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
BOOL_T SSHD_VM_GetSshSessionId2(UI8_T **session_id2, I32_T *session_id2_len)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshSessionId2(my_tid, session_id2, session_id2_len);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshConnectionSockId(I32_T connection_in, I32_T connection_out)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionSockId(my_tid, connection_in, connection_out);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshConnectionSockId(I32_T *connection_in, I32_T *connection_out)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshConnectionSockId(my_tid, connection_in, connection_out);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshSendContext
 * PURPOSE:
 *          This function get ssh connection cipher context for sending.
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
BOOL_T SSHD_VM_GetSshSendContext(CipherContext **send_context)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshSendContext(my_tid, send_context);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshReceiveContext
 * PURPOSE:
 *          This function get ssh connection cipher context for receiving.
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
BOOL_T SSHD_VM_GetSshReceiveContext(CipherContext **receive_context)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshReceiveContext(my_tid, receive_context);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshCurrentKeys(Newkeys ***current_keys)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshCurrentKeys(my_tid, current_keys);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshNewKey
 * PURPOSE:
 *          This function get pointer of Session key information for Encryption and MAC.
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
BOOL_T SSHD_VM_GetSshNewKey(Newkeys ***newkeys)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshNewKey(my_tid, newkeys);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshInputBuffer(Buffer **input)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshInputBuffer(my_tid, input);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshOutputBuffer(Buffer **output)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshOutputBuffer(my_tid, output);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshOutgoingPacketBuffer(Buffer **outgoing_packet)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshOutgoingPacketBuffer(my_tid, outgoing_packet);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshIncomingPacketBuffer(Buffer **incoming_packet)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshIncomingPacketBuffer(my_tid, incoming_packet);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshDataFellows(I32_T datafellows)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshDataFellows(my_tid, datafellows);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshDataFellows(I32_T *datafellows)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshDataFellows(my_tid, datafellows);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshCompat20(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshCompat20(my_tid);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshCompat20(I32_T *compat20)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshCompat20(my_tid, compat20);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshCompat13(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshCompat13(my_tid);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshCompat13(I32_T *compat13)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshCompat13(my_tid, compat13);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshSensitiveData
 * PURPOSE:
 *          This function get ssh sensitive_data pointer.
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
BOOL_T SSHD_VM_GetSshSensitiveData(SSHD_SensitiveData_T **sensitive_data)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshSensitiveData(my_tid, sensitive_data);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshChannelsAllocNumber
 * PURPOSE:
 *          This function get ssh size of the channel array.
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
BOOL_T SSHD_VM_GetSshChannelsAllocNumber(I32_T *channels_alloc)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshChannelsAllocNumber(my_tid, channels_alloc);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshChannelsAllocNumber
 * PURPOSE:
 *          This function set ssh size of the channel array.
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
BOOL_T SSHD_VM_SetSshChannelsAllocNumber(I32_T channels_alloc)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshChannelsAllocNumber(my_tid, channels_alloc);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshChannels
 * PURPOSE:
 *          This function get pointer of an array containing all allocated channels.
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
BOOL_T SSHD_VM_GetSshChannels(Channel ***channels)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshChannels(my_tid, channels);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshChannels
 * PURPOSE:
 *          This function set pointer of an array containing all allocated channels.
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
BOOL_T SSHD_VM_SetSshChannels(Channel **channels)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshChannels(my_tid, channels);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshChannelMaxSock
 * PURPOSE:
 *          This function set maximum socket id value used in any of the channels.
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
BOOL_T SSHD_VM_SetSshChannelMaxSock(I32_T channel_max_fd)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshChannelMaxSock(my_tid, channel_max_fd);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshChannelMaxSock
 * PURPOSE:
 *          This function get maximum socket id value used in any of the channels.
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
BOOL_T SSHD_VM_GetSshChannelMaxSock(I32_T *channel_max_fd)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshChannelMaxSock(my_tid, channel_max_fd);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshDispatch
 * PURPOSE:
 *          This function get pointer of an array containing all dispatch functions.
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
BOOL_T SSHD_VM_GetSshDispatch(dispatch_fn ***dispatch)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshDispatch(my_tid, dispatch);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_SetSshRemoteProtocolFlags
 * PURPOSE:
 *          This function set protocol flags for the remote side for SSHv1.
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
BOOL_T SSHD_VM_SetSshRemoteProtocolFlags(UI32_T remote_protocol_flags)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshRemoteProtocolFlags(my_tid, remote_protocol_flags);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_GetSshRemoteProtocolFlags
 * PURPOSE:
 *          This function get protocol flags for the remote side for SSHv1.
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
BOOL_T SSHD_VM_GetSshRemoteProtocolFlags(UI32_T *remote_protocol_flags)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshRemoteProtocolFlags(my_tid, remote_protocol_flags);

    return (ret);
}

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
BOOL_T SSHD_VM_AddRemoteAddrToBlockCache()
{
    SYS_MGR_Telnet_T    telnet_cfg;
    I32_T               in;
    I32_T               out;
    struct sockaddr_in6 sock_addr;
    socklen_t           sock_addr_size = (socklen_t)sizeof(sock_addr);
    L_INET_AddrIp_T     inet_addr;

    memset(&telnet_cfg, 0, sizeof(SYS_MGR_Telnet_T));

    SYS_PMGR_GetTelnetCfg(&telnet_cfg);

    /* telnet silent time disable ?
     */
    if (0 == telnet_cfg.silent_time)
        return TRUE;

    if (FALSE == SSHD_VM_GetSshConnectionSockId(&in, &out))
        return FALSE;

    if (0 != getpeername(in, (struct sockaddr *)&sock_addr, &sock_addr_size))
        return FALSE;

    if (FALSE == L_INET_SockaddrToInaddr((struct sockaddr *)&sock_addr, &inet_addr))
        return FALSE;

    MGMT_IP_FLT_AddBlockCache(MGMT_IP_FLT_TELNET, &inet_addr, telnet_cfg.silent_time);

    return TRUE;
}

#if 0
/* FUNCTION NAME:  SSHD_VM_SetSshv1SessionKey
 * PURPOSE:
 *          This function set session key for protocol v1.
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
BOOL_T SSHD_VM_SetSshv1SessionKey(UI8_T *ssh1_key, UI32_T ssh1_keylen)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshv1SessionKey(my_tid, ssh1_key, ssh1_keylen);

    return (ret);
}
#endif



/* FUNCTION NAME:  SSHD_VM_GetAuthenticationRetries
 * PURPOSE:
 *          This function get number of authentication retries.
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
BOOL_T SSHD_VM_GetAuthenticationRetries(UI32_T *retries)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    *retries = SSHD_OM_GetAuthenticationRetries();

    return TRUE;
}



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
 *      FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetUserPublicKey(UI8_T *username, UI32_T key_type, UI8_T *file)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret = FALSE;
    UI8_T   *temp_file;

    /* BODY */
    temp_file = (UI8_T *)malloc(1024);
    if( key_type == KEY_TYPE_RSA )
    {
        ret = KEYGEN_MGR_GetUserPublicKey(username, file, temp_file);
    }
    else if( key_type == KEY_TYPE_DSA )
    {
        ret = KEYGEN_MGR_GetUserPublicKey(username, temp_file, file);
    }
    free(temp_file);

    return (ret);
}



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
 *          FALSE - Fault.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_VM_GetSshInteractiveMode(I32_T *interactive_mode)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshInteractiveMode(my_tid, interactive_mode);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshInteractiveMode(I32_T interactive_mode)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshInteractiveMode(my_tid, interactive_mode);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshMaxPacketSize(I32_T *max_packet_size)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshMaxPacketSize(my_tid, max_packet_size);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshMaxPacketSize(I32_T max_packet_size)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshMaxPacketSize(my_tid, max_packet_size);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshChannelPre(chan_fn ***channel_pre)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshChannelPre(my_tid, channel_pre);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshChannelPost(chan_fn ***channel_post)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshChannelPost(my_tid, channel_post);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshChannelHandlerInit(I32_T *did_init)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshChannelHandlerInit(my_tid, did_init);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshChannelHandlerInit(I32_T did_init)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshChannelHandlerInit(my_tid, did_init);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshHadChannel(I32_T *had_channel)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshHadChannel(my_tid, had_channel);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshHadChannel(I32_T had_channel)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshHadChannel(my_tid, had_channel);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshStdioBuffer(Buffer **stdin_buffer, Buffer **stdout_buffer, Buffer **stderr_buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshStdioBuffer(my_tid, stdin_buffer, stdout_buffer, stderr_buffer);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshStdioDescriptor(I32_T *fdin, I32_T *fdout, I32_T *fderr)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshStdioDescriptor(my_tid, fdin, fdout, fderr);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshStdioDescriptor(I32_T fdin, I32_T fdout, I32_T fderr)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshStdioDescriptor(my_tid, fdin, fdout, fderr);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshEofStatus(I32_T *stdin_eof, I32_T *fdout_eof, I32_T *fderr_eof)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshEofStatus(my_tid, stdin_eof, fdout_eof, fderr_eof);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshEofStatus(I32_T stdin_eof, I32_T fdout_eof, I32_T fderr_eof)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshEofStatus(my_tid, stdin_eof, fdout_eof, fderr_eof);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshConnectionClosed(I32_T connection_closed)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionClosed(my_tid, connection_closed);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshConnectionClosed(I32_T *connection_closed)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshConnectionClosed(my_tid, connection_closed);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshBufferHigh(UI32_T *buffer_high)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshBufferHigh(my_tid, buffer_high);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshBufferHigh(UI32_T buffer_high)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshBufferHigh(my_tid, buffer_high);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshClientAliveTimeouts(I32_T *client_alive_timeouts)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshClientAliveTimeouts(my_tid, client_alive_timeouts);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshClientAliveTimeouts(I32_T client_alive_timeouts)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshClientAliveTimeouts(my_tid, client_alive_timeouts);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshAuthctxt(Authctxt *authctxt)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshAuthctxt(my_tid, authctxt);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshAuthctxt(Authctxt **authctxt)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshAuthctxt(my_tid, authctxt);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshSessions(Session **sessions)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshSessions(my_tid, sessions);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshSessionNewInit(I32_T *did_init)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshSessionNewInit(my_tid, did_init);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshSessionNewInit(I32_T did_init)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshSessionNewInit(my_tid, did_init);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshServerKeyIndex(UI32_T *index)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshServerKeyIndex(my_tid, index);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshFatalCleanups(struct fatal_cleanup **fatal_cleanups)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshFatalCleanups(my_tid, fatal_cleanups);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshFatalCleanups(struct fatal_cleanup *fatal_cleanups)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshFatalCleanups(my_tid, fatal_cleanups);

    return (ret);
}



/* FUNCTION NAME:  SSHD_VM_ClearSshServerOptions
 * PURPOSE:
 *          This function release ssh server options resource.
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
BOOL_T SSHD_VM_ClearSshServerOptions(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_ClearSshServerOptions(my_tid);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshReadSequenceNumber(UI32_T *read_seqnr)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshReadSequenceNumber(my_tid, read_seqnr);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshReadSequenceNumber(UI32_T read_seqnr)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshReadSequenceNumber(my_tid, read_seqnr);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshSendSequenceNumber(UI32_T *send_seqnr)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshSendSequenceNumber(my_tid, send_seqnr);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshSendSequenceNumber(UI32_T send_seqnr)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshSendSequenceNumber(my_tid, send_seqnr);

    return (ret);
}



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
UI32_T SSHD_VM_GetNegotiationTimeout(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  timeout;

    /* BODY */
    timeout = SSHD_OM_GetNegotiationTimeout();

    return timeout;
}



/* FUNCTION NAME:  SSHD_VM_GetCreatedSessionNumber
 * PURPOSE:
 *          This function get number of  ssh connection.
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
UI32_T SSHD_VM_GetCreatedSessionNumber(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T  created_session_number;

    /* BODY */
    created_session_number = SSHD_OM_GetCreatedSessionNumber();

    return created_session_number;
}



/* FUNCTION NAME:  SSHD_VM_SetCreatedSessionNumber
 * PURPOSE:
 *          This function set number of  ssh connection.
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
void SSHD_VM_SetCreatedSessionNumber(UI32_T number)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    SSHD_OM_SetCreatedSessionNumber(number);

    return ;
}



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionStatus()
 * PURPOSE:
 *          This function set status of ssh connection.
 *
 * INPUT:
 *          SSHD_ConnectionState_T - current state of connection.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_SetSshConnectionStatus(SSHD_ConnectionState_T state)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionStatus(my_tid, state);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshConnectionVersion(UI32_T major, UI32_T minor)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionVersion(my_tid, major, minor);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshConnectionUsername(UI8_T *username)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionUsername(my_tid, username);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshConnectionPassword(const char *password)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionPassword(my_tid, password);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshConnectionAuthResult(USERAUTH_AuthResult_T *auth_result_p)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionAuthResult(my_tid, auth_result_p);

    return (ret);
}



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
BOOL_T SSHD_VM_GetLocalSessionName(UI32_T sock_id, UI32_T *ip, UI32_T *port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    struct  sockaddr    sock_addr;
    struct  sockaddr_in *sock_in;
    socklen_t sock_addr_size=(socklen_t)sizeof(sock_addr);
    UI32_T     res;

    /* BODY */
    *ip = 0;
    *port = 0;

    res = getsockname(sock_id, &sock_addr,&sock_addr_size);

    if (res==0)
    {
        sock_in = (struct sockaddr_in*) &sock_addr;
        *port = L_STDLIB_Ntoh16(sock_in->sin_port);
        *ip   = L_STDLIB_Ntoh32(sock_in->sin_addr.s_addr);
        return TRUE;
    }

    return FALSE;
}



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
BOOL_T SSHD_VM_GetRemoteSessionName(UI32_T sock_id, L_INET_AddrIp_T *ip, UI32_T *port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    struct sockaddr_storage sock_addr;
    socklen_t  sock_addr_size=(socklen_t)sizeof(sock_addr);
    UI32_T  res;

    /* BODY */
    memset(ip, 0, sizeof(*ip));
    *port = 0;

    res = getpeername(sock_id, (struct sockaddr *)&sock_addr, &sock_addr_size);

    if (res==0)
    {
        L_INET_SockaddrToInaddr((struct sockaddr *)&sock_addr, ip);
        *port = SSHD_VM_GetSockPort((struct sockaddr *)&sock_addr);
        return TRUE;
    }

    return FALSE;
}



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
UI32_T SSHD_VM_GetSockPort(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        struct sockaddr_in *sin4 = (struct sockaddr_in*)sa;

        return (UI32_T) L_STDLIB_Ntoh16( sin4->sin_port );
    }
    else if (sa->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;

        return (UI32_T) L_STDLIB_Ntoh16( sin6->sin6_port );
    }

    /* should not go here
     */
    return 0;
}



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
BOOL_T SSHD_VM_SetSessionPair(int net, int tnsh)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;
    struct  sockaddr_storage sin;
    socklen_t sinlen;

    UI32_T remote_tnsh_port;
    UI32_T tnsh_port;
    UI32_T user_local_port;
    UI32_T user_port;

    L_INET_AddrIp_T user_addr;

    /* gets user IP address and port no
     */
    sinlen = sizeof(sin);
    if ( getpeername(net, (struct sockaddr*)&sin, &sinlen) != 0)
    {
        return FALSE;
    }

    L_INET_SockaddrToInaddr((struct sockaddr *) &sin, &user_addr);

    user_port = SSHD_VM_GetSockPort((struct sockaddr*)&sin);

    /* gets local port for user
     */
    sinlen = sizeof(sin);
    if ( getsockname(net, (struct sockaddr*)&sin, &sinlen) != 0)
    {
        return FALSE;
    }

    user_local_port = SSHD_VM_GetSockPort((struct sockaddr*)&sin);

    /* gets remote port for tnsh
     */
    sinlen = sizeof(sin);
    if ( getpeername(tnsh, (struct sockaddr*)&sin, &sinlen) != 0)
    {
        return FALSE;
    }

    remote_tnsh_port = SSHD_VM_GetSockPort((struct sockaddr*)&sin);

    /* gets local port for tnsh
     */
    sinlen = sizeof(sin);
    if ( getsockname(tnsh, (struct sockaddr*)&sin, &sinlen) != 0)
    {
        return FALSE;
    }

    tnsh_port = SSHD_VM_GetSockPort((struct sockaddr*)&sin);

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSessionPair(my_tid, remote_tnsh_port
                                       , tnsh_port
                                       , user_local_port
                                       , &user_addr
                                       , user_port);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshConnectionId(UI32_T *conn_id_p)
{
    BOOL_T  ret;
    UI32_T  my_tid;

    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshConnectionId(my_tid, conn_id_p);

    return (ret);
}



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
BOOL_T SSHD_VM_CheckSshConnection(UI32_T cid)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;

    /* BODY */
    ret = SSHD_OM_CheckSshConnection(cid);

    return (ret);
}



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
void SSHD_VM_CleanUpConnectingTnshFd(void *context_p)
{
    UI32_T  my_tid;
    int     tnsh_fd;
    BOOL_T  ret;

    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetConnectingTnshFd(my_tid, &tnsh_fd);

    if (FALSE == ret || tnsh_fd < 0)
    {
        return;
    }

    SSHD_OM_SetConnectingTnshFd(my_tid, -1);

    s_close(tnsh_fd);
}



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
BOOL_T SSHD_VM_SetConnectingTnshFd(int tnsh_fd)
{
    BOOL_T  ret;
    UI32_T  my_tid;

    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetConnectingTnshFd(my_tid, tnsh_fd);

    if (TRUE == ret && tnsh_fd >= 0)
    {
        fatal_add_cleanup(SSHD_VM_CleanUpConnectingTnshFd, 0);
    }

    return (ret);
}



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
BOOL_T SSHD_VM_GetConnectingTnshFd(int *tnsh_fd_p)
{
    BOOL_T  ret;
    UI32_T  my_tid;

    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetConnectingTnshFd(my_tid, tnsh_fd_p);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshConnectionCipher(char *cipher)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshConnectionCipher(my_tid, (UI8_T *)cipher);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshPacketLength(UI32_T *packet_length)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshPacketLength(my_tid, packet_length);

    return (ret);
}



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
BOOL_T SSHD_VM_SetSshPacketLength(UI32_T packet_length)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_SetSshPacketLength(my_tid, packet_length);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshMacBuffer(UI8_T **mac_buffer)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshMacBuffer(my_tid, mac_buffer);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshKexgexHashDigest(UI8_T **digest)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshKexgexHashDigest(my_tid, digest);

    return (ret);
}



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
BOOL_T SSHD_VM_GetSshkexDhHashDigest(UI8_T **digest)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */
    BOOL_T  ret;
    UI32_T  my_tid;

    /* BODY */
    my_tid = SYSFUN_TaskIdSelf();
    ret = SSHD_OM_GetSshkexDhHashDigest(my_tid, digest);

    return (ret);
}



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
BOOL_T SSHD_VM_do_convert_from_ssh2(Key *key, char *file)
{
    Key *k;
    int blen;
    u_int len;
    char line[1024], *p;
/*  u_char blob[8096];
    char encoded[8096];*/
    u_char *blob;
    char *encoded;
    char *end;
//  struct stat st;
    int escaped = 0; /*, private = 0;*//*, ok;*/
//  FILE *fp;
    int found_key = 0;
    BIO *bio_p = NULL;

    /* empty public key
     */
    if(strlen(file) == 0)
        return FALSE;

    /* BODY */
    blob = (u_char *)malloc(8096);
    if (blob == NULL)
    {
        return FALSE;
    }
    encoded = (char *)malloc(8096);
    if (encoded == NULL)
    {
        free(blob);
        return FALSE;
    }
    end = encoded + 8095;
    encoded[0] = '\0';

    bio_p = BIO_new_mem_buf(file, -1);

    while(BIO_gets(bio_p, line, sizeof(line)))
    {
        if (!(p = (char *)strchr(line, '\n')))
        {
//          fprintf(stderr, "input line too long.\n");
            BIO_free(bio_p);
            free(blob);
            free(encoded);
            return FALSE;
        }
        if (p > line && p[-2] == '\\')
            escaped++;
        if (strncmp(line, "----", 4) == 0 ||
            strstr(line, ": ") != NULL)
        {
            /*
            if (strstr(line, SSH_COM_PRIVATE_BEGIN) != NULL)
                private = 1;
            */
            if (strstr(line, " END ") != NULL)
            {
                break;
            }
            /* fprintf(stderr, "ignore: %s", line); */
            continue;
        }
        if (escaped)
        {
            escaped--;
            /* fprintf(stderr, "escaped: %s", line); */
            continue;
        }
        *p = '\0';
//      strlcat(encoded, line, sizeof(encoded));
        strncat(encoded, line, end - encoded);
    }
    *end = '\0';
    len = strlen(encoded);
    if(len < 4)
    {
        BIO_free(bio_p);
        free(blob);
        free(encoded);
        return FALSE;
    }
    if (((len % 4) == 3) &&
        (encoded[len-1] == '=') &&
        (encoded[len-2] == '=') &&
        (encoded[len-3] == '='))
        encoded[len-3] = '\0';
//  blen = uudecode(encoded, blob, sizeof(blob));
    blen = uudecode(encoded, blob, 8096);
    if (blen < 0)
    {
//      fprintf(stderr, "uudecode failed.\n");
        BIO_free(bio_p);
        free(blob);
        free(encoded);
        return FALSE;
    }
/*  k = private ?
        do_convert_private_ssh2_from_blob(blob, blen) :
        key_from_blob(blob, blen);*/
    k = key_from_blob(blob, blen);

    if (k == NULL)
    {
//      fprintf(stderr, "decode blob failed.\n");
        BIO_free(bio_p);
        free(blob);
        free(encoded);
        return FALSE;
    }
/*  ok = private ?
        (k->type == KEY_DSA ?
         PEM_write_DSAPrivateKey(stdout, k->dsa, NULL, NULL, 0, NULL, NULL) :
         PEM_write_RSAPrivateKey(stdout, k->rsa, NULL, NULL, 0, NULL, NULL)) :
        key_write(k, stdout);
    if (!ok) {
        fprintf(stderr, "key write failed");
        exit(1);
    }*/

    if (key_equal(k, key) /*&&
        auth_parse_options(pw, options, file, linenum) == 1*//*isiah*/)
    {
        found_key = 1;
    }

    key_free(k);

    BIO_free(bio_p);
    free(blob);
    free(encoded);
    if( found_key == 1 )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}













/* LOCAL SUBPROGRAM BODIES
 */



