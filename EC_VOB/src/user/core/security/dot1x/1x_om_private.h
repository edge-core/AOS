
/* Project Name: New Feature
 * File_Name : 1x_om.h
 * Purpose     : 1X initiation and 1X task creation
 *
 * 2002/06/25    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */
 
#ifndef	DOT1X_OM_PRIVATE_H
#define	DOT1X_OM_PRIVATE_H  
 
#include "1x_auth_pae.h"
/*------------------------------------------------------------------------
 * DEFAULT VARIABLES DECLARTIONS
 *-----------------------------------------------------------------------*/
/* Need to sync. sys_dflt.h */

 
/* MACRO FUNCTIONS DECLARACTION
 */

/* GLOBAL CONFIGURATION VALUE */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for DOT1X objects
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function is invoked in DOT1X_TASK_Init.
 *-------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_InitSemaphore(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_EnterCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical region before a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function make the DOT1X task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
UI32_T DOT1X_OM_EnterCriticalRegion(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_LeaveCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical region after a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   none
 * NOTE     :   This function make the DOT1X task and the other tasks
 *              keep mutual exclusive.
 *-------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_LeaveCriticalRegion(UI32_T orig_priority);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetStateMachineWorkingArea
 * ---------------------------------------------------------------------
 * PURPOSE: Get DOT1X State Machine Working Area
 * INPUT:  lport.
 * OUTPUT: pae_ptr.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
Global_Params * DOT1X_OM_GetStateMachineWorkingArea(UI32_T port);
#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_RunStateMachine
 * ---------------------------------------------------------------------
 * PURPOSE: This function will run DOT1X State Machine
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_RunStateMachine(UI32_T port,UI32_T event_type);
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set default value of 1X configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetConfigSettingToDefault();
#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_Authen_Dot1x
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default Authen_dot1x is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_Authen_Dot1x(UI32_T *authen_do1x);
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable the 802.1x port to support multiple-host 
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host 
                     FALSE for Disable the 802.1x port to support multiple-host 
 * ---------------------------------------------------------------------
 */

BOOL_T DOT1X_OM_Set_MultiHostMode(UI32_T port,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_No_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the  multiple-host mode to default
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host 
                     FALSE for Disable the 802.1x port to support multiple-host 
 * ---------------------------------------------------------------------
 */

BOOL_T DOT1X_OM_No_MultiHostMode(UI32_T port);
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the mode of 802.1x port support multiple-host 
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host 
                     FALSE for Disable the 802.1x port to support multiple-host 
 * ---------------------------------------------------------------------
 */

BOOL_T DOT1X_OM_Get_MultiHostMode(UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MultiHostMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MultiHostMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_MultiHostMode(UI32_T port,BOOL_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the mode of 802.1x port support multiple-host 
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host 
                     FALSE for Disable the 802.1x port to support multiple-host 
 * ---------------------------------------------------------------------
 */

BOOL_T DOT1X_OM_GetNext_MultiHostMode(UI32_T *index,BOOL_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNextRunning_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the mode of 802.1x port support multiple-host 
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */

UI32_T DOT1X_OM_GetNextRunning_MultiHostMode(UI32_T *index,BOOL_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set port control mode of 1X configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  mode = 0 for ForceUnauthorized
                  1 for ForceAuthorized
                  2 for Auto
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortControlMode(UI32_T port,UI32_T mode);
BOOL_T DOT1X_OM_Init_PortControlMode(UI32_T lport);
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get port control mode of 1X configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: mode.
 * NOTES:  mode = 0 for ForceUnauthorized
                  1 for ForceAuthorized
                  2 for Auto
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortControlMode(UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: PortControlMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortControlMode(UI32_T port,UI32_T *type);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get port control mode of 1X configuration
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  Type = 0 for ForceUnauthorized
                  1 for ForceAuthorized
                  2 for Auto
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortControlMode(UI32_T *index,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable port period re-authentication of the client ,which 
 *          is disabled by default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable re-authentication
                     FALSE for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortReAuthEnabled(UI32_T port ,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get port period re-authentication status of the client
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  control =  TRUE for Enable re-authentication
                     FALSE for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_PortReAuthEnabled(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthEnabled
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortReAuthEnabled(UI32_T port,BOOL_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get port period re-authentication status of the client
 * INPUT:  index.
 * OUTPUT: control.
 * RETURN: TRUE/FASLE.
 * NOTES:  control =  TRUE for Enable re-authentication
 *                    FALSE for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortReAuthEnabled(UI32_T *index,BOOL_T *control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNextRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get port period re-authentication status of the client
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetNextRunning_PortReAuthEnabled(UI32_T *index,BOOL_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable global period re-authentication of the client ,which 
 *          is disabled by default.
 * INPUT:  control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable re-authentication
                     FALSE for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_ReAuthenticationMode(BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get port global re-authentication status of the client
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: mode.
 * NOTES:  mode =  TRUE for Enable re-authentication
 *                 FALSE for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_ReAuthenticationMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthenticationMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_ReAuthenticationMode(UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set the number of seconds between re-authentication attempts.
 *          The range is 1 to 65535;the default is 3600 seconds. 
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_ReAuthPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set the number of seconds between re-authentication attempts for per-port.
 *          The range is 1 to 65535;the default is 3600 seconds. 
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortReAuthPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_No_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set the number of seconds between re-authentication attempts to default for per-port.
 *          The range is 1 to 65535;the default is 3600 seconds. 
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_No_PortReAuthPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: ReAuthPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_ReAuthPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts for per-port.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: PortReAuthPeriod
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortReAuthPeriod(UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_ReAuthPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: ReAuthPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortReAuthPeriod(UI32_T port,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts for per-port.
 * INPUT:  index.
 * OUTPUT: times.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortReAuthPeriod(UI32_T *index,UI32_T *times);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds that the switch remains in the quiet state 
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_QuietPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds for per-port that the switch remains in the quiet state 
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortQuietPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch remains in the quiet state 
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: QuietPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_QuietPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds for per-port that the switch remains in the quiet state 
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.The range is from 1 to 65535.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortQuietPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortQuietPeriod(UI32_T lport);
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: QuietPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_QuietPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: QuietPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortQuietPeriod(UI32_T port,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds for per-port that the switch remains in the quiet state 
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.
 * INPUT:  index.
 * OUTPUT: times.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortQuietPeriod(UI32_T *index,UI32_T *times);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before 
 *          retransmitting the request.
 *          The range is 1 to 65535 seconds;the default is 30 seconds.
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_TxPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before 
 *          retransmitting the request for per-port.
 *          The range is 1 to 65535 seconds;the default is 30 seconds.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortTxPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before 
 *          retransmitting the request.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TxPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_TxPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before 
 *          retransmitting the request for per-port.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortTxPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortTxPeriod(UI32_T lport);
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: TxPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_TxPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: TxPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortTxPeriod(UI32_T port,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before 
 *          retransmitting the request for per-port.
 * INPUT:  index.
 * OUTPUT: seconds.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortTxPeriod(UI32_T *index,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of times that the switch will send an 
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 *          The range is 1 to 10;the default is 2 .
 * INPUT:  times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_MaxReq(UI32_T times);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of times for per-port that the switch will send an 
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 *          The range is 1 to 10;the default is 2 .
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortMaxReq(UI32_T lport,UI32_T times);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_No_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of times for per-port that the switch will send an 
 *          EAP-request/identity frame before restarting the authentication
 *          process to default.
 *          The range is 1 to 10;the default is 2 .
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_No_PortMaxReq(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of times that the switch will send an 
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: MaxReq.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_MaxReq();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of times for per-port that the switch will send an 
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortMaxReq.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortMaxReq(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_MaxReq(UI32_T *max_req);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortMaxReq(UI32_T port,UI32_T *max_req);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of times for per-port that the switch will send an 
 *          EAP-request/identity frame before restarting the authentication
 *          process.
 * INPUT:  index.
 * OUTPUT: max_req.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortMaxReq(UI32_T *index,UI32_T *max_req);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNextRunning_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetNextRunning_PortMaxReq(UI32_T *index,UI32_T *max_req);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the administrative enable/disable state for Port Access 
            Control in a system.
 * INPUT:  control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_SystemAuthControl(UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access 
            Control in a system.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_SystemAuthControl();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: SystemAuthControl
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_SystemAuthControl(UI32_T *control);
#if 0 /*eli move to mgr to do*/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PaePortInitialize
 * ---------------------------------------------------------------------
 * PURPOSE: Set this attribute TRUE causes the Port to be initialized.
            the attribute value reverts to FALSE once initialization has completed.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =   TRUE for Enable Initialize
                      FALSE for Disable Initialize
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PaePortInitialize(UI32_T port ,BOOL_T control);
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PaePortInitialize
 * ---------------------------------------------------------------------
 * PURPOSE: Get this attribute TRUE causes the Port to be initialized.
            the attribute value reverts to FALSE once initialization has completed.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PaePortInitialize.
 * NOTES:  control =   TRUE for Enable Initialize
                      FALSE for Disable Initialize
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_PaePortInitialize(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PaePortReauthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauthentication control for this port.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =   TRUE for Enable Initialize
                      FALSE for Disable Initialize
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PaePortReauthenticate(UI32_T lport ,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PaePortReauthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauthentication control for this port.
            This attribute always return FALSE when it is read.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PaePortReauthenticate.
 * NOTES:  control =   TRUE for Enable Initialize
                      FALSE for Disable Initialize
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_PaePortReauthenticate(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value of the administrative controlled directions 
            parameter for the port.
 * INPUT:  lport,direction.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  direction =   0 for Both
                         1 for In
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AdminCtrlDirections(UI32_T lport,UI32_T direction);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value of the administrative controlled directions 
            parameter for the port.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: direction.
 * NOTES:  direction =   0 for Both
                         1 for In
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AdminCtrlDirections(UI32_T lport);
#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_CtrlPortControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value of the controlled port parameter for the port.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =   0 for ForceUnauthorized
                       1 for ForceAuthorized
                       2 for Auto
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_CtrlPortControl(UI32_T lport,UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_CtrlPortControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value of the controlled port parameter for the port.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  control =   0 for ForceUnauthorized
                       1 for ForceAuthorized
                       2 for Auto
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_CtrlPortControl(UI32_T lport);
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AuthSuppTimeout(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the suppTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: AuthSuppTimeout.
 * NOTES:  
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AuthSuppTimeout(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AuthServerTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AuthServerTimeout(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_AuthServerTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value ,in seconds ,of the serverTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: AuthServerTimeout.
 * NOTES:  
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AuthServerTimeout(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value of the keyTransmissionEnabled constant currently
            in use by the Authenticator PAE state machine.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable AuthTxEnabled
                     FALSE for Disable AuthTxEnabled
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AuthTxEnabled(UI32_T port,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value of the keyTransmissionEnabled constant currently
            in use by the Authenticator PAE state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  control =  TRUE for Enable AuthTxEnabled
                     FALSE for Disable AuthTxEnabled
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_AuthTxEnabled(UI32_T lport);

/**********For MIB ***************/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Pae_Port_Table
 * ---------------------------------------------------------------------
 * PURPOSE: A table of system level information for each port supported 
            by the Port Access Entity. An entry appears in this table for 
            each port of this system.
 * INPUT:  lport.
 * OUTPUT: PaePortEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
//BOOL_T DOT1X_OM_Pae_Port_Table(UI32_T port,Dot1xPaePortEntry *PaePortEntry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Auth_Config_Table
 * ---------------------------------------------------------------------
 * PURPOSE: A table that contains the configuration objects for the Authenticator
            PAE associated with each port.
            An entry appears in this table for each port that may authenticate 
            access to itself.
 * INPUT:  lport.
 * OUTPUT: AuthConfigEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
//BOOL_T DOT1X_OM_Auth_Config_Table(UI32_T port,Dot1xAuthConfigEntry *AuthConfigEntry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Auth_Stats_Table
 * ---------------------------------------------------------------------
 * PURPOSE: A table that contains the statistics objects for the Authenticator
            PAE associated with each port.
            An entry appears in this table for each port that may authenticate 
            access to itself.
 * INPUT:  lport.
 * OUTPUT: AuthStateEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
//BOOL_T DOT1X_OM_Auth_Stats_Table(UI32_T port,Dot1xAuthStateEntry *AuthStateEntry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Auth_Diag_Table
 * ---------------------------------------------------------------------
 * PURPOSE: A table that contains the diagnostics objects for the Authenticator
            PAE associated with each port.
            An entry appears in this table for each port that may authenticate 
            access to itself.
 * INPUT:  lport.
 * OUTPUT: AuthDiagEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
//BOOL_T DOT1X_OM_Auth_Diag_Table(UI32_T port,Dot1xAuthDiagEntry *AuthDiagEntry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Session_Stats_Table
 * ---------------------------------------------------------------------
 * PURPOSE: The session statistics information for an Authenticatior PAE.
            This shows the current values being collected for each session 
            that is still in progress, or the final values for the last valid
            session on each port where there is no session currently active.
 * INPUT:  lport.
 * OUTPUT: AuthSessionStatsEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
//BOOL_T DOT1X_OM_Session_Stats_Table(UI32_T port,Dot1xAuthSessionStatsEntry *AuthSessionStatsEntry);

BOOL_T DOT1X_OM_Set_Svr_Port(UI32_T port_number);
UI32_T DOT1X_OM_Get_Svr_Port(void);
BOOL_T DOT1X_OM_Set_RADIUSId(UI32_T id);
UI32_T DOT1X_OM_Get_RADIUSId(void);
#if 0
BOOL_T DOT1X_OM_Set_TotalLPort(UI32_T port_number);
UI32_T DOT1X_OM_Get_TotalLPort(void);
#endif
BOOL_T DOT1X_OM_Set_GlobalQuietPeriod(int period);
int DOT1X_OM_Get_GlobalQuietPeriod(void);
BOOL_T DOT1X_OM_Set_GlobalReauthPeriod(int period);
int DOT1X_OM_Get_GlobalReauthPeriod(void);
BOOL_T DOT1X_OM_Set_GlobalReauthEnabled(BOOL_T mode);
BOOL_T DOT1X_OM_Get_GlobalReauthEnabled(void);
BOOL_T DOT1X_OM_Set_GlobalMaxReq(UI32_T times);
UI32_T DOT1X_OM_Get_GlobalMaxReq(void);
BOOL_T DOT1X_OM_Set_GlobalTxPeriod(int period);
int DOT1X_OM_Get_GlobalTxPeriod(void);
BOOL_T DOT1X_OM_Set_TaskId(UI32_T taskid);
UI32_T DOT1X_OM_Get_TaskId(void);
void DOT1X_OM_Set_SessionId(Auth_Pae * auth_pae);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the authenticator's operation mode 
 * INPUT:  lport,mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortOperationMode(UI32_T lport,UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function return SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default SystemOperationMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: SystemOperationMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortOperationMode(UI32_T lport,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get port operation mode of 1X configuration
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:   VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *          VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortOperationMode(UI32_T *index,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Set the max allowed MAC number in multi-host mode .
 * INPUT:  lport,count.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortMultiHostMacCount(UI32_T lport,UI32_T count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortMultiHostMacCount is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: PortMultiHostMacCount
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortMultiHostMacCount(UI32_T lport,UI32_T *count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next max allowed MAC number in multi-host mode 
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortMultiHostMacCount(UI32_T *index,UI32_T *count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE: Get the port's re-auth max
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortReAuthMax.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortReAuthMax(UI32_T lport);

UI32_T DOT1X_OM_Get_RadiusMsgQid();
BOOL_T DOT1X_OM_Set_RadiusMsgQid(UI32_T msg_id);

BOOL_T DOT1X_OM_SetPortRadaMode(UI32_T lport,UI32_T rada_mode);
BOOL_T DOT1X_OM_GetPortRadaMode(UI32_T lport,UI32_T *rada_mode);
BOOL_T DOT1X_OM_SetDot1xMsgQId(UI32_T dot1x_msgq_id);
BOOL_T DOT1X_OM_GetDot1xMsgQId(UI32_T *dot1x_msgq_id);
BOOL_T DOT1X_OM_SetTaskServiceFunPtr(DOT1X_AuthorizedResultCookie_T cookie, UI8_T service_type);
BOOL_T DOT1X_OM_GetTaskServiceFunPtr(DOT1X_AuthorizedResultCookie_T *cookie, UI8_T service_type);
#endif /*DOT1X_OM_PRIVATE_H*/
