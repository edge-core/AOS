/* MODULE NAME:  sshd_mgr.h
* PURPOSE: 
*   Initialize the resource and provide some functions for the sshd module.
*   
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-05-27      -- Isiah , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef SSHD_MGR_H

#define SSHD_MGR_H



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

/* FUNCTION NAME:  SSHD_MGR_Init
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
 *          This function is invoked in SSHD_INIT_Initiate_System_Resources.
 */
BOOL_T SSHD_MGR_Init(void);



/* FUNCTION NAME:  SSHD_MGR_EnterMasterMode
 * PURPOSE: 
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the SSHD subsystem will enter the
 *          Master Operation mode.                                                            
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
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *				switch will be initiated to the factory default value.
 *			2. SSHD will handle network requests only when this subsystem
 *				is in the Master Operation mode 
 *			3. This function is invoked in SSHD_INIT_EnterMasterMode.
 */
BOOL_T SSHD_MGR_EnterMasterMode(void);



/* FUNCTION NAME:  SSHD_MGR_EnterTransitionMode
 * PURPOSE: 
 *          This function forces this subsystem enter the Transition Operation mode.
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
 *			.
 */
BOOL_T SSHD_MGR_EnterTransitionMode(void);



/* FUNCTION NAME:  SSHD_MGR_EnterSlaveMode
 * PURPOSE: 
 *          This function forces this subsystem enter the Slave Operation mode.
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
 *          In Slave Operation mode, any network requests 
 *          will be ignored.
 */
void SSHD_MGR_EnterSlaveMode(void);



/* FUNCTION	NAME : SSHD_MGR_SetTransitionMode
 * PURPOSE:
 *		Set transition mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void SSHD_MGR_SetTransitionMode(void);



/* FUNCTION NAME:  SSHD_MGR_SetOpMode
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
void SSHD_MGR_SetOpMode (SYS_TYPE_Stacking_Mode_T opmode);



/* FUNCTION NAME:  SSHD_MGR_GetOpMode
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
SYS_TYPE_Stacking_Mode_T SSHD_MGR_GetOpMode(void);



/* FUNCTION	NAME : SSHD_MGR_GetOperationMode
 * PURPOSE:
 *		Get current sshd operation mode (master / slave / transition).
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *          SYS_TYPE_Stacking_Mode_T - opmode.
 *
 * NOTES:
 *		None.
 */
SYS_TYPE_Stacking_Mode_T SSHD_MGR_GetOperationMode(void);



/* FUNCTION NAME:  SSHD_MGR_SetSshdStatus
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
 *          This function maybe invoked in CLI command 'ip ssh server'.
 */
BOOL_T SSHD_MGR_SetSshdStatus (SSHD_State_T state);



/* FUNCTION NAME:  SSHD_MGR_GetSshdStatus
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
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
SSHD_State_T SSHD_MGR_GetSshdStatus(void);



/* FUNCTION NAME:  SSHD_MGR_SetSshdPort
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
 *          none.
 * NOTES:
 *          This function maybe invoked in CLI command 'ip ssh port'.
 */
BOOL_T SSHD_MGR_SetSshdPort (UI32_T port);



/* FUNCTION NAME:  SSHD_MGR_GetSshdPort
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
UI32_T SSHD_MGR_GetSshdPort(void);



/* FUNCTION NAME:  SSHD_MGR_SetAuthenticationRetries
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
 *          This function maybe invoked in CLI command 'ip ssh authentication-retries'.
 */
BOOL_T SSHD_MGR_SetAuthenticationRetries(UI32_T retries);



/* FUNCTION NAME:  SSHD_MGR_GetAuthenticationRetries
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
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
UI32_T SSHD_MGR_GetAuthenticationRetries(void);



/* FUNCTION NAME:  SSHD_MGR_SetNegotiationTimeout
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
 *          This function maybe invoked in CLI command 'ip ssh timeout'.
 */
BOOL_T SSHD_MGR_SetNegotiationTimeout(UI32_T timeout);



/* FUNCTION NAME:  SSHD_MGR_GetNegotiationTimeout
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
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
UI32_T SSHD_MGR_GetNegotiationTimeout(void);



/* FUNCTION NAME:  SSHD_MGR_GetSshServerVersion()
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
 *          This function maybe invoked in CLI command 'show ip ssh'.
 */
BOOL_T SSHD_MGR_GetSshServerVersion(UI32_T *major, UI32_T *minor);



/* FUNCTION NAME : SSHD_MGR_GetSessionPair
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
BOOL_T SSHD_MGR_GetSessionPair(UI32_T tnsh_port, UI32_T *user_ip, UI32_T *user_port);



/* FUNCTION NAME : SSHD_MGR_SetConnectionIDAndTnshID
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
 *      This function invoked in TNSHD_ChildTask().
 */
BOOL_T SSHD_MGR_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid);



/* FUNCTION NAME : SSHD_MGR_GetNextSshConnectionEntry
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
BOOL_T SSHD_MGR_GetNextSshConnectionEntry(I32_T *cid, SSHD_ConnectionInfo_T *info);



/* FUNCTION NAME : SSHD_MGR_GetSshConnectionEntry
 * PURPOSE:
 *      Get Specify active connection entry.
 *
 * INPUT:
 *      UI32_T   -- Specify active connection id.
 *
 * OUTPUT:
 *      SSHD_ConnectionInfo_T * -- Specify active connection information.
 *
 * RETURN:
 *      TRUE  - The output value is Specify active connection info.
 *      FALSE - The output value is invalid.
 *
 * NOTES:
 *      This function invoked in SNMP .
 */
BOOL_T SSHD_MGR_GetSshConnectionEntry(UI32_T cid, SSHD_ConnectionInfo_T *info);



/* FUNCTION NAME : SSHD_MGR_CheckSshConnection
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
 *      This function invoked in CLI command "disconnect ssh".
 */
BOOL_T SSHD_MGR_CheckSshConnection(UI32_T cid);



/* FUNCTION NAME:  SSHD_MGR_GetRunningNegotiationTimeout
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific SSH negotiation timeout with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - Negotiation Timeout.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningNegotiationTimeout(UI32_T *timeout);



/* FUNCTION NAME:  SSHD_MGR_GetRunningAuthenticationRetries
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific SSH authentication retries times with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - Authentication Retries.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningAuthenticationRetries(UI32_T *retries);



/* FUNCTION NAME:  SSHD_MGR_GetRunningSshdStatus
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific Sshd Status with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - Sshd Status.
 *               
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 */
SYS_TYPE_Get_Running_Cfg_T  SSHD_MGR_GetRunningSshdStatus(UI32_T *state);



/* FUNCTION NAME - SSHD_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void SSHD_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - SSHD_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void SSHD_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);








#endif /* #ifndef SSHD_MGR_H */



