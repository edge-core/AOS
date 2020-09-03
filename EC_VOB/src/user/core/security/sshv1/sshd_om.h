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
#include <sys_type.h>
#include "sshd_type.h"
#include "sshd.h"
#include "sshd_record.h"



/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

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



/* FUNCTION NAME:  SSHD_OM_SetOpMode
 * PURPOSE: 
 *          This function set sshd operation mode.
 *
 * INPUT:   
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          .
 */
void SSHD_OM_SetOpMode (SYS_TYPE_Stacking_Mode_T opmode);



/* FUNCTION NAME:  SSHD_OM_GetOpMode
 * PURPOSE: 
 *          This function get sshd operation mode.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 * NOTES:
 *          .
 */
SYS_TYPE_Stacking_Mode_T SSHD_OM_GetOpMode(void);



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



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionStatus()
 * PURPOSE: 
 *          This function set status of ssh connection.
 *
 * INPUT:   
 *          UI32_T                  - index of connection recond.
 *          SSHD_ConnectionState_T - Current connection status.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T SSHD_OM_SetSshConnectionStatus(UI32_T index ,SSHD_ConnectionState_T state);



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionVersion()
 * PURPOSE: 
 *          This function set version of ssh connection.
 *
 * INPUT:   
 *          UI32_T - index of connection recond.
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
BOOL_T SSHD_OM_SetSshConnectionVersion(UI32_T index, UI32_T major, UI32_T minor);



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



/* FUNCTION NAME:  SSHD_OM_SetSshConnectionUsername()
 * PURPOSE: 
 *          This function set username of ssh connection.
 *
 * INPUT:   
 *          UI32_T  - number of connection range is 0-3.
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
BOOL_T SSHD_OM_SetSshConnectionUsername(UI32_T index, UI8_T *username);



/* FUNCTION NAME : SSHD_OM_SetSessionPair
 * PURPOSE:
 *      Add a session pair to session record.
 *
 * INPUT:
 *      UI32_T  -- number of connection range is 0-3.
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
BOOL_T SSHD_OM_SetSessionPair(UI32_T index, UI32_T remote_tnsh_port, 
                                 UI32_T tnsh_port, UI32_T user_local_port,
                                 UI32_T user_ip, UI32_T user_port);



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
 *          (  Something must be known to use this function. )
 */
BOOL_T SSHD_OM_GetSessionPair(UI32_T tnsh_port, UI32_T *user_ip, UI32_T *user_port);



/* FUNCTION NAME : SSHD_OM_SetConnectionIDAndTnshID
 * PURPOSE:
 *      Set tnsh id and ssh connection id to session record.
 *
 * INPUT:
 *      UI32_T  -- the port connect to TNSHD.
 *      UI32_T  -- TNSH id.
 *      UI32_T  -- ssh connection id.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      TRUE  - tnsh_port found and ssh server enabled, or don't found tnsh_port.
 *      FALSE - tnsh_port found and ssh server disabled.
 *
 * NOTES:
 *      .
 */
BOOL_T SSHD_OM_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid);



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



/* FUNCTION NAME : SSHD_OM_CheckSshConnection
 * PURPOSE:
 *      Check connection is ssh or not.
 *
 * INPUT:
 *      UI32_T -- connection id.
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







#endif /* #ifndef SSHD_OM_H */



