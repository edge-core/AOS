/* Project Name: New Feature
 * File_Name : 1x_pom.h
 * Purpose     :
 *
 * 2007/06/13    : Eli Lin Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for linux platform
 */

#ifndef	DOT1X_POM_H
#define	DOT1X_POM_H
#include "1x_types.h"
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
void DOT1X_POM_Init(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Get pe-port MaxReq.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: MaxReq.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortMaxReq(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_MaxReq
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
UI32_T  DOT1X_POM_GetRunning_MaxReq(UI32_T *max_req);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default ReAuth-MAX is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortReAuthMax(UI32_T lport,UI32_T *max_req);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default MaxReq is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: MaxReq
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_PortMaxReq(UI32_T lport,UI32_T *max_req);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get port control mode of 1X configuration
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: type.
 * NOTES:  Type = VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized for ForceUnauthorized
                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized for ForceAuthorized
                  VAL_dot1xAuthAuthControlledPortControl_auto for Auto
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortControlMode(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortControlMode is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: PortControlMode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_PortControlMode(UI32_T lport,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get port period re-authentication status of the client
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  control =  VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
                      VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortReAuthEnabled(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default PortReAuthEnabled is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: mode
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_POM_GetRunning_PortReAuthEnabled(UI32_T lport,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port QuietPeriod
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: QuietPeriod.
 * NOTES:  0 -- Get error.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortQuietPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default QuietPeriod is successfully retrieved.
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
UI32_T DOT1X_POM_GetRunning_QuietPeriod(UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default QuietPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: QuietPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortQuietPeriod(UI32_T lport,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port ReAuthPeriod
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: ReAuthPeriod.
 * NOTES:  0 -- Get error.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortReAuthPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default ReAuthPeriod is successfully retrieved.
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
UI32_T DOT1X_POM_GetRunning_ReAuthPeriod(UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default ReAuthPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: ReAuthPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortReAuthPeriod(UI32_T lport,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get per-port TxPeriod.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TxPeriod.
 * NOTES:  0 -- Get error.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortTxPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TxPeriod is successfully retrieved.
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
UI32_T DOT1X_POM_GetRunning_TxPeriod(UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default TxPeriod is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: TxPeriod
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortTxPeriod(UI32_T lport,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default supp-timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: lport.
 * OUTPUT: AuthSuppTimeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_AuthSuppTimeout(UI32_T lport, UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
            Control in a system.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: SystemAuthControl
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_SystemAuthControl();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default SystemAuthControl is successfully retrieved.
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
UI32_T DOT1X_POM_GetRunning_SystemAuthControl(UI32_T *control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetPaePortEntry(UI32_T lport, DOT1X_PaePortEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextPaePortEntry(UI32_T *lport_p, DOT1X_PaePortEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetAuthConfigEntry(UI32_T lport, DOT1X_AuthConfigEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextAuthConfigEntry(UI32_T *lport_p, DOT1X_AuthConfigEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetAuthStatsEntry(UI32_T lport, DOT1X_AuthStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextAuthStatsEntry(UI32_T *lport_p, DOT1X_AuthStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetAuthDiagEntry(UI32_T lport, DOT1X_AuthDiagEntry_T*entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetNextAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextAuthDiagEntry(UI32_T *lport_p, DOT1X_AuthDiagEntry_T*entry_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_Get_Global_Parameters
 *---------------------------------------------------------------------------
 * PURPOSE:  Get dot1x global parameters.
 * INPUT:    global_parameters pointer.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void DOT1X_POM_Get_Global_Parameters(DOT1X_Global_Parameters_T * global_parameters);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_POM_GetPortAuthorized
 * --------------------------------------------------------------------------
 * PURPOSE : Get the current authorization state of the port
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_AUTHORIZED,
 *           DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_UNAUTHORIZED or
 *           DOT1X_TYPE_AUTH_CONTROLLED_PORT_STATUS_ERR
 * NOTE    : None
 * --------------------------------------------------------------------------
 */
DOT1X_TYPE_AuthControlledPortStatus_T DOT1X_POM_GetPortAuthorized(UI32_T lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_POM_Get_Port_Details
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_Get_Port_Details(UI32_T lport, DOT1X_PortDetails_T *port_details_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_POM_GetNextPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_POM_GetNextPortDetails(UI32_T lport, UI32_T *index_p, DOT1X_PortDetails_T *port_details_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortMultiHostMacCount
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
UI32_T  DOT1X_POM_GetRunning_PortMultiHostMacCount(UI32_T lport,UI32_T *count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_Get_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE: Get the port's re-auth max
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortReAuthMax.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_Get_PortReAuthMax(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_POM_GetRunning_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authenticator's operation mode
 * INPUT:  lport.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_POM_GetRunning_PortOperationMode(UI32_T lport,UI32_T *mode);

#endif /*#ifndef	DOT1X_POM_H*/

