#ifndef	DOT1X_PMGR_H
#define	DOT1X_PMGR_H

#include "sys_type.h"
#include "1x_mgr.h"

/*------------------------------------------------------------------------------
 * ROUTINE NAME : CLUSTER_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CLUSTER_PMGR.
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
void DOT1X_PMGR_Init(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set default value of 1X configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_SetConfigSettingToDefault();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port MaxReq.
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortMaxReq(UI32_T lport,UI32_T times);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set MaxReq to default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_MaxReq();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port MaxReq to default.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_PortMaxReq(UI32_T lport);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable the 802.1x port to support multiple-host
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host
                     FALSE for Disable the 802.1x port to support multiple-host
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_MultiHostMode(UI32_T lport,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the  multiple-host mode to default
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host
                     FALSE for Disable the 802.1x port to support multiple-host
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_MultiHostMode(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set port control mode of 1X configuration
 * INPUT:  lport,type.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  Type = VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized for ForceUnauthorized
                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized for ForceAuthorized
                  VAL_dot1xAuthAuthControlledPortControl_auto for Auto
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortControlMode(UI32_T lport,UI32_T type);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set port control mode of 1X configuration to default
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_PortControlMode(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable port period re-authentication of the client ,which
 *          is disabled by default.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
                      VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortReAuthEnabled(UI32_T lport ,UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Do_ReAuthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Use the command to manually initiate a re-authentication of
 *          all 802.1x-enabled ports or the specified 802.1x-enabled port
 * INPUT:  lport, mac, mac_count
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Do_ReAuthenticate(UI32_T lport, UI8_T *mac,UI32_T mac_count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable global period re-authentication of the client ,which
 *          is disabled by default.
 * INPUT:  control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
                      VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_ReAuthenticationMode(UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set global period re-authentication of the client to default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_ReAuthenticationMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_QuietPeriod
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
BOOL_T DOT1X_PMGR_Set_QuietPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port QuietPeriod
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortQuietPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set QuietPeriod to default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_QuietPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port QuietPeriod to default.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_PortQuietPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set the number of seconds between re-authentication attempts.
 *          The range is 1 to 65535;the default is 3600 seconds.
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_ReAuthPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port ReAuthPeriod
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortReAuthPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set ReAuthPeriod to default
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_ReAuthPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port ReAuthPeriod to default
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_PortReAuthPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set number of seconds that the switch should wait for a respond
 *          to an EAP request/identity frame from the client before
 *          retransmitting the request.
 *          The range is 1 to 65535 seconds;the default is 30 seconds.
 * INPUT:  seconds(TxPeriod).
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_TxPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port TxPeriod.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortTxPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set TxPeriod to default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_TxPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_No_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port TxPeriod to default.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_No_PortTxPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the administrative enable/disable state for Port Access
            Control in a system.
 * INPUT:  control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_SystemAuthControl(UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PaePortInitialize
 * ---------------------------------------------------------------------
 * PURPOSE: Set this attribute TRUE causes the Port to be initialized.
            the attribute value reverts to FALSE once initialization has completed.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =   VAL_dot1xPaePortInitialize_true for Enable Initialize
                       VAL_dot1xPaePortInitialize_false Disable Initialize
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PaePortInitialize(UI32_T lport ,UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PaePortReauthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauthentication control for this port.
 * INPUT:  lport,mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  mode =   VAL_dot1xPaePortReauthenticate_true for Enable Reauthenticate
                    VAL_dot1xPaePortReauthenticate_false for Disable Reauthenticate
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PaePortReauthenticate(UI32_T lport ,UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value of the administrative controlled directions
            parameter for the port.
 * INPUT:  lport,direction.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  direction =   VAL_dot1xAuthAdminControlledDirections_both for Both
                         VAL_dot1xAuthAdminControlledDirections_in for In
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_AdminCtrlDirections(UI32_T lport,UI32_T direction);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_CtrlPortControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value of the controlled port parameter for the port.
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =   VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized for ForceUnauthorized
                       VAL_dot1xAuthAuthControlledPortControl_auto for Auto
                       VAL_dot1xAuthAuthControlledPortControl_forceAuthorized for ForceAuthorized
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_CtrlPortControl(UI32_T lport,UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_AuthSuppTimeout(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_AuthServerTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_AuthServerTimeout(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_AuthTxEnabled
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
BOOL_T DOT1X_PMGR_Set_AuthTxEnabled(UI32_T lport,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the authenticator's operation mode
 * INPUT:  lport,mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortOperationMode(UI32_T lport,UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Set_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Set the max allowed MAC number in multi-host mode .
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Set_PortMultiHostMacCount(UI32_T lport,UI32_T count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Session_Stats_Table
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
BOOL_T DOT1X_PMGR_Session_Stats_Table(UI32_T lport,DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsEntry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Get_Next_Session_Stats_Table
 * ---------------------------------------------------------------------
 * PURPOSE: Get next session statistics information for an Authenticatior PAE.
            This shows the current values being collected for each session
            that is still in progress, or the final values for the last valid
            session on each port where there is no session currently active.
 * INPUT:  Index.
 * OUTPUT: AuthSessionStatsEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Get_Next_Session_Stats_Table(UI32_T *Index,DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsEntry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Get_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authenticator's operation mode
 * INPUT:  lport.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Get_PortOperationMode(UI32_T lport,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Get_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authenticator's operation mode
 * INPUT:  lport.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_PMGR_GetRunning_PortOperationMode(UI32_T lport,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_GetNext_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get next port operation mode of 1X configuration
 * INPUT:  index.
 * OUTPUT: type.
 * RETURN: TRUE/FALSE.
 * NOTES: DOT1X_SYSTEM_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *        DOT1X_SYSTEM_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_GetNext_PortOperationMode(UI32_T *index,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_Get_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number in multi-host mode .
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_Get_PortMultiHostMacCount(UI32_T lport,UI32_T *count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_GetNext_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next max allowed MAC number in multi-host mode
 * INPUT:  None.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_GetNext_PortMultiHostMacCount(UI32_T *index,UI32_T *count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_PMGR_GetNext_MAC_Auth_Stats_Table
 * ---------------------------------------------------------------------
 * PURPOSE: A table that contains the statistics objects for the Authenticator
 *          PAE associated with each port.
 *          An entry appears in this table for each supplicant on the port
 *          that may authenticate access to itself.
 * INPUT:  lport.
 * OUTPUT: AuthStateEntry,supplicant_mac.
 * RETURN: TRUE/FALSE
 * NOTES:  index = 0xFFFF --- get the first entry on the port.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_PMGR_GetNext_MAC_Auth_Stats_Table(
    UI32_T              lport,              UI32_T  *mac_index,
    DOT1X_AuthStatsEntry_T *AuthStateEntry,    UI8_T   *supplicant_mac);

#endif /*#ifndef	DOT1X_PMGR_H*/

