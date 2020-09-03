/* MODULE NAME:  sshd_vm.h
* PURPOSE: 
*   Initialize the resource and provide some functions for the sshd module.
*   
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-09-19      -- Isiah , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef SSHD_VM_H

#define SSHD_VM_H



/* INCLUDE FILE DECLARATIONS
 */
#include <sys_type.h>
#include "sshd_type.h"



/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:  SSHD_VM_GetSshdSessionRecord
 * PURPOSE: 
 *			This function get sshd session record pointer.
 * INPUT:   
 *          none.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          void * - SSHD session record pointer.
 * NOTES:
 *          .
 */ 
void *SSHD_VM_GetSshdSessionRecord(void);



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



/* FUNCTION NAME:  SSHD_VM_SetTaskID
 * PURPOSE: 
 *			This function set tas id to session record.
 * INPUT:   
 *          UI32_T -- task id.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          TRUE to indicate successful and FALSE to indicate failure.
 */ 
BOOL_T SSHD_VM_SetTaskID(UI32_T tid);



/* FUNCTION NAME:  SSHD_VM_GetTaskID
 * PURPOSE: 
 *			This function get tas id from session record.
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
UI32_T SSHD_VM_GetTaskID(UI32_T index);



/* FUNCTION NAME:  SSHD_VM_GetContextAddress
 * PURPOSE: 
 *			This function get ssh_context pointer.
 * INPUT:   
 *          none.
 *                                   
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          void * -- ssh_context pointer.
 * NOTES:
 *          .
 */ 
void *SSHD_VM_GetContextAddress(void);



/* FUNCTION NAME:  SSHD_VM_ResetTaskID
 * PURPOSE: 
 *			This function reset task id to session record.
 * INPUT:   
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
void SSHD_VM_ResetTaskID(UI32_T tid);



/* FUNCTION NAME:  SSHD_VM_GetHostkey
 * PURPOSE: 
 *          This function get hostkey of sshd.
 *
 * INPUT:   
 *          void * -- pointer of ssh context.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetHostkey(void *ssh_context);



/* FUNCTION NAME:  SSHD_VM_GetServerkey
 * PURPOSE: 
 *          This function get serverkey of sshd.
 *
 * INPUT:   
 *          void * -- pointer of ssh context.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T SSHD_VM_GetServerkey(void *ssh_context);



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
BOOL_T SSHD_VM_SetSshConnectionStatus(SSHD_ConnectionState_T state);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionVersion()
 * PURPOSE: 
 *          This function set version of ssh connection.
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
BOOL_T SSHD_VM_SetSshConnectionVersion(UI32_T major, UI32_T minor);



/* FUNCTION NAME:  SSHD_VM_SetSshServerVersion()
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
BOOL_T SSHD_VM_SetSshServerVersion(UI32_T major, UI32_T minor);



/* FUNCTION NAME:  SSHD_VM_SetSshConnectionUsername()
 * PURPOSE: 
 *          This function set username of ssh connection.
 *
 * INPUT:   
 *          UI8_T * - username
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
BOOL_T SSHD_VM_GetRemoteSessionName(UI32_T sock_id, UI32_T *ip, UI32_T *port);



/* FUNCTION NAME : SSHD_VM_SetSessionPair
 * PURPOSE:
 *      Add a session pair to session record.
 *
 * INPUT:
 *      UI32_T  -- remote site port of TNSH (pty) session.
 *      UI32_T  -- the local side port connect to TNSHD.
 *      UI32_T  -- local site port of SSHD (net) session.
 *      UI32_T  -- the ip of remote site in socket.
 *      UI32_T  -- the port of remote site in socket.(net)
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
BOOL_T SSHD_VM_SetSessionPair(UI32_T remote_tnsh_port, UI32_T tnsh_port,
                               UI32_T user_local_port,
                               UI32_T user_ip, UI32_T user_port);



/* FUNCTION NAME:  SSHD_VM_GetSshdStatus
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
SSHD_State_T SSHD_VM_GetSshdStatus(void);



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



/* FUNCTION NAME:  SSHD_VM_GetAuthenticationRetries
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
UI32_T SSHD_VM_GetAuthenticationRetries(void);










#endif /* #ifndef SSHD_VM_H */
