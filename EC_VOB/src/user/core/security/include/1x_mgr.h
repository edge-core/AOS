/* Project Name: New Feature
 * File_Name : 1x_mgr.h
 * Purpose     : DOT1X initiation and DOT1X task creation
 *
 * 2002/05/07    : Kevin Cheng  Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (Mercury)
 */

#ifndef	DOT1X_MGR_H
#define	DOT1X_MGR_H
//#include "1x_eapol.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "1x_types.h"
#include "security_backdoor.h"

#define DOT1X_SUPPORT_ACCTON_BACKDOOR      (TRUE && SECURITY_SUPPORT_ACCTON_BACKDOOR) /* support backdoor functions */


#define DOT1X_MAX_MSGQ_LEN  64
#define MIN_PERIOD_TIME 1
#define MAX_PERIOD_TIME 65535
#define MIN_MAXREQ_TIME 1
#define MAX_MAXREQ_TIME 10
/*system operation mode*/
#define DOT1X_PORT_OPERATION_MODE_ONEPASS	1
#define DOT1X_PORT_OPERATION_MODE_MULTIPASS	2
#define DOT1X_PORT_OPERATION_MODE_MACBASED      3
#define DOT1X_DEFAULT_PORT_OPERATION_MODE	DOT1X_PORT_OPERATION_MODE_ONEPASS
/* max MAC count*/
#define DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT	5
#define DOT1X_DEFAULT_ACTION                VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic

/* ES4649-32-01078
   obsoleted by DOT1X_USERNAME_LENGTH in 1x_types.h
#define DOT1X_USERNAME_LENGTH                   32
*/

#define DOT1X_MGR_DATA_START_OFFSET (UI32_T)(&(((DOT1X_MGR_IPCMsg_T *)0)->data.data_start_offset))

enum
{
    DOT1X_MGR_IPCCMD_SETCONFIGSETTINGTODEFAULT,
    DOT1X_MGR_IPCCMD_SETPORTMAXREQ,
    DOT1X_MGR_IPCCMD_NOMAXREQ,
    DOT1X_MGR_IPCCMD_NOPORTMAXREQ,
    DOT1X_MGR_IPCCMD_SETMULTIHOSTMODE,
    DOT1X_MGR_IPCCMD_NOMULTIHOSTMODE,
    DOT1X_MGR_IPCCMD_SETPORTCONTROLMODE,
    DOT1X_MGR_IPCCMD_NOPORTCONTROLMODE,
    DOT1X_MGR_IPCCMD_SETPORTREAUTHENABLED,
    DOT1X_MGR_IPCCMD_DOREAUTHENTICATE,
    DOT1X_MGR_IPCCMD_SET_REAUTHENTICATIONMODE,
    DOT1X_MGR_IPCCMD_NO_REAUTHENTICATIONMODE,
    DOT1X_MGR_IPCCMD_SET_QUIETPERIOD,
    DOT1X_MGR_IPCCMD_SET_PORTQUIETPERIOD,
    DOT1X_MGR_IPCCMD_NO_QUIETPERIOD,
    DOT1X_MGR_IPCCMD_NO_PORTQUIETPERIOD,
    DOT1X_MGR_IPCCMD_SET_REAUTHPERIOD,
    DOT1X_MGR_IPCCMD_SET_PORTREAUTHPERIOD,
    DOT1X_MGR_IPCCMD_NO_REAUTHPERIOD,
    DOT1X_MGR_IPCCMD_NO_PORTREAUTHPERIOD,
    DOT1X_MGR_IPCCMD_SET_TXPERIOD,
    DOT1X_MGR_IPCCMD_SET_PORTTXPERIOD,
    DOT1X_MGR_IPCCMD_NO_TXPERIOD,
    DOT1X_MGR_IPCCMD_NO_PORTTXPERIOD,
    DOT1X_MGR_IPCCMD_SET_SYSTEMAUTHCONTROL,
    DOT1X_MGR_IPCCMD_SET_PAEPORTINITIALIZE,
    DOT1X_MGR_IPCCMD_SET_PAEPORTREAUTHENTICATE,
    DOT1X_MGR_IPCCMD_SET_ADMINCTRLDIRECTIONSE,
    DOT1X_MGR_IPCCMD_SET_CTRLPORTCONTROL,
    DOT1X_MGR_IPCCMD_SET_AUTHSUPPTIMEOUT,
    DOT1X_MGR_IPCCMD_SET_AUTHSERVERTIMEOUT,
    DOT1X_MGR_IPCCMD_SET_AUTHTXENABLED,
    DOT1X_MGR_IPCCMD_SET_PORTOPERATIONMODE,
    DOT1X_MGR_IPCCMD_SET_PORTMULTIHOSTMACCOUNT,
    DOT1X_MGR_IPCCMD_SESSION_STATS_TABLE,
    DOT1X_MGR_IPCCMD_GET_NEXT_SESSION_STATS_TABLE,
    DOT1X_MGR_IPCCMD_GET_PORTOPERATIONMODE,
    DOT1X_MGR_IPCCMD_GETRUNNING_PORTOPERATIONMODE,
    DOT1X_MGR_IPCCMD_GETNEXT_PORTOPERATIONMODE,
    DOT1X_MGR_IPCCMD_GET_PORTMULTIHOSTMACCOUNT,
    DOT1X_MGR_IPCCMD_GETNEXT_PORTMULTIHOSTMACCOUNT,
    DOT1X_MGR_IPCCMD_GETNEXT_MACAUTHSTATSTABLE,
    DOT1X_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC
};

/*use to the definition of IPC message buffer*/
typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
    }type;

    union
    {
        UI8_T data_start_offset;
        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        UI8_T ip4_v[4];

        struct
        {
            UI32_T lport;
            UI32_T times;
        }port_times;

        struct
        {
            UI32_T lport;
            BOOL_T control;
        }port_control;

        struct
        {
            UI32_T lport;
            UI32_T type;
        }lport_type;

        struct
        {
            UI32_T lport;
            UI32_T seconds;
        }lport_seconds;

        struct
        {
            UI32_T lport;
            UI32_T mode;
        }lport_mode;

        struct
        {
            UI32_T lport;
            UI32_T direction;
        }lport_direction;

        struct
        {
            UI32_T lport;
            UI32_T count;
        }lport_count;

        struct
        {
            UI32_T lport;
            DOT1X_AuthSessionStatsEntry_T AuthSessionStatsEntry;
        }session_stats;

        struct
        {
            UI32_T lport;
            UI32_T mac_cnt;
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
        }lport_reauth_mac;

        struct
        {
            UI32_T              lport;
            UI32_T              mac_idx;
            DOT1X_AuthStatsEntry_T dot1x_sentry;
            UI8_T               sup_mac[SYS_ADPT_MAC_ADDR_LEN];
        }lport_macauth_status;
    } data; /* contains the supplemntal data for the corresponding cmd */
}__attribute__((packed, aligned(1)))DOT1X_MGR_IPCMsg_T;

#define DOT1X_MGR_MSGBUF_TYPE_SIZE    sizeof(((DOT1X_MGR_IPCMsg_T *)0)->type)

BOOL_T DOT1X_MGR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE:  This function initializes all function pointer registration operations.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void DOT1X_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_CurrentOperationMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of 1X 's task
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   1X_operation_mode
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 SYS_TYPE_Stacking_Mode_T DOT1X_MGR_CurrentOperationMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the 1X enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 void DOT1X_MGR_EnterMasterMode();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the 1X enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void DOT1X_MGR_EnterSlaveMode();

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_SetTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Set transition mode
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void DOT1X_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_EnterTransition Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the 1X enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void DOT1X_MGR_EnterTransitionMode();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_MGR_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for DOT1X objects
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTE     :   This function is invoked in DOT1X_TASK_Init.
 *-------------------------------------------------------------------------
 */
BOOL_T   DOT1X_MGR_InitSemaphore(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_PortOperStatusChanged_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: When effective oper status of some user port was changed for
            dot1x the function will be called.
 * INPUT:  pre_status --- The status before change.
 *         current_status --- The status after change.
 *                    Status                                    Precedence
 *                 ----------------------------------------     ----------
 *                 1) VAL_ifOperStatus_up                       0
 *                 2) SWCTRL_PORT_DORMANT_STATUS_TYPE_LACP      1
 *                 3) SWCTRL_PORT_DORMANT_STATUS_TYPE_DOT1X     2
 *                 4) VAL_ifOperStatus_down                     3
 *                 5) VAL_ifOperStatus_lowerLayerDown           3
 *                 6) VAL_ifOperStatus_notPresent
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_PortOperStatusChanged_CallBack(UI32_T unit, UI32_T port, UI32_T pre_status, UI32_T current_status) ;

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_PortLinkUp_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Port link up callback function
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_PortLinkUp_CallBack (UI32_T unit,UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_UPortAdminEnable_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Uport Admin Enable CallBack function
 * INPUT:   unit,port.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_UPortAdminEnable_CallBack(UI32_T unit, UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_UPortAdminDisable_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: Uport Admin Enable CallBack function
 * INPUT:   unit,port
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_UPortAdminDisable_CallBack(UI32_T unit, UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_ProcessEAPMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process EAP packet from supplicant
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_ProcessEAPMsg(UI32_T lport,UI8_T *mac,UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_ProcessRADIUSPMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process EAP packet from RADIUS client
 * INPUT:  port.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_ProcessRADIUSPMsg(UI32_T lport,UI8_T *mac,UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_ProcessTimeoutMsg
 * ---------------------------------------------------------------------
 * PURPOSE: Process Timeout Event from Timer
 * INPUT:  port.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_ProcessTimeoutMsg(UI32_T port);

/*************************************************************************/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set default value of 1X configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_SetConfigSettingToDefault();

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_Authen_do1x()
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:  times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_Authen_do1x(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_Authen_do1x()
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:  times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_Authen_do1x(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetRunning_Authen_Dot1x
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default Authen_Do1x is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT:authen_do1x   =  0   --> DISABLE
          authen_do1x   =  1   --> ENABLE
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_MGR_GetRunning_Authen_Dot1x(UI32_T *authen_do1x);
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_MaxReq
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
BOOL_T DOT1X_MGR_Set_MaxReq(UI32_T times);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port MaxReq.
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_PortMaxReq(UI32_T lport,UI32_T times);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_MaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set MaxReq to default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_MaxReq();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_PortMaxReq
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port MaxReq to default.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_PortMaxReq(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_MaxReq
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
UI32_T DOT1X_MGR_Get_MaxReq();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable the 802.1x port to support multiple-host
 * INPUT:  lport,control.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host
                     FALSE for Disable the 802.1x port to support multiple-host
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_MultiHostMode(UI32_T lport,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the  multiple-host mode to default
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable  the 802.1x port to support multiple-host
                     FALSE for Disable the 802.1x port to support multiple-host
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_MultiHostMode(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the mode of 802.1x port support multiple-host
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: mode.
 * NOTES:  mode =  TRUE for Enable  the 802.1x port to support multiple-host
                     FALSE for Disable the 802.1x port to support multiple-host
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Get_MultiHostMode(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNext_MultiHostMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next port MultiHostMode
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE for get next Success
          FALSE for get next  Failure
 * NOTES:  mode =  TRUE for Enable  the 802.1x port to support multiple-host
                  FALSE for Disable the 802.1x port to support multiple-host
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_GetNext_MultiHostMode(UI32_T *index,BOOL_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNextRunning_MultiHostMode
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
UI32_T DOT1X_MGR_GetNextRunning_MultiHostMode(UI32_T *index,BOOL_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortControlMode
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
BOOL_T DOT1X_MGR_Set_PortControlMode(UI32_T lport,UI32_T type);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set port control mode of 1X configuration to default
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_PortControlMode(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNext_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get next port control mode of 1X configuration
 * INPUT:  index.
 * OUTPUT: type.
 * RETURN: TRUE/FALSE.
 * NOTES:  mode = VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized for ForceUnauthorized
                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized for ForceAuthorized
                  VAL_dot1xAuthAuthControlledPortControl_auto for Auto
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_GetNext_PortControlMode(UI32_T *index,UI32_T *type);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortReAuthEnabled
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
BOOL_T DOT1X_MGR_Set_PortReAuthEnabled(UI32_T lport,UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNext_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get port period re-authentication status of the client
 * INPUT:  index.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  mode =  VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
                   VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_GetNext_PortReAuthEnabled(UI32_T *index,UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Do_ReAuthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Use the command to manually initiate a re-authentication of
            all 802.1x-enabled ports or the specified 802.1x-enabled port
 * INPUT:  lport, mac.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Do_ReAuthenticate(UI32_T lport, UI8_T *mac,UI32_T mac_count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_ReAuthenticationMode
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
BOOL_T DOT1X_MGR_Set_ReAuthenticationMode(UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set global period re-authentication of the client to default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_ReAuthenticationMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_ReAuthenticationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get port global re-authentication status of the client
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: mode.
 * NOTES:  mode =    VAL_dot1xPaePortReauthenticate_true  for Enable re-authentication
                     VAL_dot1xPaePortReauthenticate_false for Disable re-authentication
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_MGR_Get_ReAuthenticationMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_QuietPeriod
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
BOOL_T DOT1X_MGR_Set_QuietPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port QuietPeriod
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_PortQuietPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set QuietPeriod to default.
 * INPUT:  None.
 * OUTPUT: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_QuietPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port QuietPeriod to default.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_PortQuietPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_QuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds that the switch remains in the quiet state
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: QuietPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_MGR_Get_QuietPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNext_PortQuietPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get number of seconds for per-port that the switch remains in the quiet state
 *          following a failed authentication exchange with the client.
 *          The default is 60 seconds.
 * INPUT:  index.
 * OUTPUT: seconds.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_GetNext_PortQuietPeriod(UI32_T *index,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set the number of seconds between re-authentication attempts.
 *          The range is 1 to 4294967295;the default is 3600 seconds.
 * INPUT:  seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_ReAuthPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port ReAuthPeriod
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_PortReAuthPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set ReAuthPeriod to default
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_ReAuthPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port ReAuthPeriod to default
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_PortReAuthPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_ReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: ReAuthPeriod.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_MGR_Get_ReAuthPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNext_PortReAuthPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get the number of seconds between re-authentication attempts for per-port.
 * INPUT:  index.
 * OUTPUT: seconds(ReAuthPeriod).
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_GetNext_PortReAuthPeriod(UI32_T *index,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_TxPeriod
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
BOOL_T DOT1X_MGR_Set_TxPeriod(UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port TxPeriod.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_PortTxPeriod(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_TxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set TxPeriod to default.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_TxPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_No_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Set per-port TxPeriod to default.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_No_PortTxPeriod(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_TxPeriod
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
UI32_T DOT1X_MGR_Get_TxPeriod();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNext_PortTxPeriod
 * ---------------------------------------------------------------------
 * PURPOSE: Get next port TxPeriod.
 * INPUT:  index.
 * OUTPUT: seconds.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_GetNext_PortTxPeriod(UI32_T *index,UI32_T *seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the administrative enable/disable state for Port Access
            Control in a system.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_SystemAuthControl(UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PaePortInitialize
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
BOOL_T DOT1X_MGR_Set_PaePortInitialize(UI32_T lport,UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_PaePortInitialize
 * ---------------------------------------------------------------------
 * PURPOSE: Get this attribute TRUE causes the Port to be initialized.
            the attribute value reverts to FALSE once initialization has completed.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: PortInitialize.
 * NOTES:  PortInitialize  =  0 for Error
                              VAL_dot1xPaePortInitialize_true for Enable Initialize
                              VAL_dot1xPaePortInitialize_false Disable Initialize
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_MGR_Get_PaePortInitialize(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PaePortReauthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Set the reauthentication control for this port.
 * INPUT:  lport,mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  mode =   VAL_dot1xPaePortReauthenticate_true for Enable Reauthenticate
                    VAL_dot1xPaePortReauthenticate_false for Disable Reauthenticate
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_PaePortReauthenticate(UI32_T lport,UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_PaePortReauthenticate
 * ---------------------------------------------------------------------
 * PURPOSE: Get the reauthentication control for this port.
            This attribute always return FALSE when it is read.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: Reauthenticate.
 * NOTES:  Reauthenticate =  VAL_dot1xPaePortReauthenticate_true for Enable Reauthenticate
                             VAL_dot1xPaePortReauthenticate_false for Disable Reauthenticate
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_MGR_Get_PaePortReauthenticate(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_AdminCtrlDirections
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
BOOL_T DOT1X_MGR_Set_AdminCtrlDirections(UI32_T lport,UI32_T direction);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value of the administrative controlled directions
            parameter for the port.
 * INPUT:  port.
 * OUTPUT: None.
 * RETURN: direction.
 * NOTES:  direction =   VAL_dot1xAuthAdminControlledDirections_both for Both
                         VAL_dot1xAuthAdminControlledDirections_in for In
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_MGR_Get_AdminCtrlDirections(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_CtrlPortControl
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
BOOL_T DOT1X_MGR_Set_CtrlPortControl(UI32_T lport,UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_CtrlPortControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value of the controlled port parameter for the port.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: control.
 * NOTES:  control =   VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized for ForceUnauthorized
                       VAL_dot1xAuthAuthControlledPortControl_auto for Auto
                       VAL_dot1xAuthAuthControlledPortControl_forceAuthorized for ForceAuthorized
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_MGR_Get_CtrlPortControl(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the suppTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_AuthSuppTimeout(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_AuthServerTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value ,in seconds ,of the serverTimeout constant currently
            in use by the Backend Authentication state machine.
 * INPUT:  lport,seconds.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_AuthServerTimeout(UI32_T lport,UI32_T seconds);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Set the value of the keyTransmissionEnabled constant currently
            in use by the Authenticator PAE state machine.
 * INPUT:  port,control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  control =  TRUE for Enable AuthTxEnabled
                     FALSE for Disable AuthTxEnabled
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_AuthTxEnabled(UI32_T lport,BOOL_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get the value of the keyTransmissionEnabled constant currently
            in use by the Authenticator PAE state machine.
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: AuthTxEnabled.
 * NOTES:  AuthTxEnabled =  TRUE for Enable AuthTxEnabled
                           FALSE for Disable AuthTxEnabled
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Get_AuthTxEnabled(UI32_T lport);

/**********For MIB ***************/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Session_Stats_Table
 * ---------------------------------------------------------------------
 * PURPOSE: The session statistics information for an Authenticatior PAE.
            This shows the current values being collected for each session
            that is still in progress, or the final values for the last valid
            session on each port where there is no session currently active.
 * INPUT:  Logical port.
 * OUTPUT: AuthSessionStatsEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Session_Stats_Table(UI32_T lport,DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsEntry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_Next_Session_Stats_Table
 * ---------------------------------------------------------------------
 * PURPOSE: Get next session statistics information for an Authenticatior PAE.
            This shows the current values being collected for each session
            that is still in progress, or the final values for the last valid
            session on each port where there is no session currently active.
 * INPUT:  Index(Logical port).
 * OUTPUT: AuthSessionStatsEntry.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Get_Next_Session_Stats_Table(UI32_T *Index,DOT1X_AuthSessionStatsEntry_T *AuthSessionStatsEntry);

/*****************For CLI Show do1x ****************************************/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_MGR_AnnounceRADIUSPacket
 *-------------------------------------------------------------------------
 * PURPOSE  : Whenever RADIUS client received a EAP attribute packet,it calls
 *            this function to handle the packet.
 * INPUT    :
 *      UI32_T   result      -- Authenticated result.
 *      int    shmem_id      -- share memory id.
 *      UI32_T   data_len    -- RADIUS packet data length.
 *      UI32_T   src_port    -- which port's data
 * RETURN   :   none
 * NOTE:
 *-------------------------------------------------------------------------
 */
void DOT1X_MGR_AnnounceRADIUSPacket(
    UI32_T  result,                 int shmem_id,
    UI32_T  data_len,               UI32_T  src_port,
    UI8_T   *src_mac,               UI32_T  src_vid,
    UI8_T   *authorized_vlan_list,  UI8_T   *authorized_qos_list,
    UI32_T   session_timeout,       UI32_T	server_ip);

BOOL_T DOT1X_MGR_Set_Debug_Transparent_Mode(BOOL_T mode);
BOOL_T DOT1X_MGR_Get_Debug_Transparent_Mode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_MGR_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void DOT1X_MGR_SetDebugFlag(UI32_T flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_MGR_GetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : get the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T DOT1X_MGR_GetDebugFlag();

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Set the administrative enable/disable state for Port Access
 *          Control in a system.
 * INPUT:  control.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
 *         VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_SystemAuthControl(UI32_T control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Get_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: Get the administrative enable/disable state for Port Access
            Control in a system.
 * INPUT:  None.
 * OUTPUT: control.
 * RETURN: TRUE/FALSE
 * NOTES:  VAL_dot1xPaeSystemAuthControl_enabled for Enable SystemAuthControl
           VAL_dot1xPaeSystemAuthControl_disabled for Disable SystemAuthControl
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Get_SystemAuthControl(UI32_T *control);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetRunning_SystemAuthControl
 * ---------------------------------------------------------------------
 * PURPOSE: This function return SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
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
UI32_T  DOT1X_MGR_GetRunning_SystemAuthControl(UI32_T *control);
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the authenticator's operation mode
 * INPUT:  lport,mode.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *         DOT1X_PORT_OPERATION_MODE_MACBASED  for MAC-based
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_PortOperationMode(UI32_T lport,UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_Set_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Set the max allowed MAC number in multi-host mode .
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_Set_PortMultiHostMacCount(UI32_T lport,UI32_T count);

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_SetPortSecurityStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_SetPortSecurityStatus (UI32_T ifindex, UI32_T portsec_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_GetNext_Authenticated_MAC
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next authenticated Supplicant
 * INPUT:  lport.
 * OUTPUT: The MAC and vid of authenticated supplicant.
 * RETURN: TRUE/FALSE.
 * NOTES:  if index is 0, then get the first authenticated Supplicant.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_GetNext_Authenticated_MAC(UI32_T lport,UI32_T *index,UI8_T *supplicant_mac,UI32_T *supplicant_vid);

/* FUNCTION NAME - DOT1X_MGR_HandleHotInsertion
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
void DOT1X_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_IsThisMacAuthenticatingMac
 * ---------------------------------------------------------------------
 * PURPOSE: Check if input MAC is the port's authenticating MAC
 * INPUT:  lport.
 *             *authenticating_mac
 * OUTPUT: none.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_IsThisMacAuthenticatingMac(UI32_T lport, UI8_T *authenticating_mac);

#if (SYS_CPNT_NETACCESS == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_SetRadaMode
 * ---------------------------------------------------------------------
 * PURPOSE  : set rada mode
 * INPUT    : lport, rada_mode
 * OUTPUT   : None.
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_SetRadaMode(UI32_T lport,UI32_T rada_mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_SendEapPacket
 * ---------------------------------------------------------------------
 * PURPOSE  : send eap packet
 * INPUT    : lport, dst_mac, eap_identifier, eap_code
 * OUTPUT   : None.
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_SendEapPacket(UI32_T lport, UI8_T *dst_mac, UI8_T eap_identifier, UI8_T eap_code);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_MGR_AsyncAuthCheck
 * ---------------------------------------------------------------------
 * PURPOSE  : 3com 1x authentication
 * INPUT    : lport, src_mac, dst_mac, tag_info, ...
 * OUTPUT   : None.
 * RETURN   : TRUE -- succeeded / FALSE -- failed
 * NOTES    : none
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_MGR_AsyncAuthCheck(
    UI32_T	lport,          UI8_T   *src_mac,
    UI8_T   *dst_mac,       UI16_T  tag_info,
    UI16_T  type,           UI8_T   *eappkt_data,
    UI32_T  eappkt_length,  BOOL_T  eap_start_is_auto_created,
    UI32_T  cookie);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_NotifyPortLinkUp
 * ---------------------------------------------------------------------
 * PURPOSE  : port link up notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_NotifyPortLinkUp(UI32_T unit, UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_NotifyPortLinkDown
 * ---------------------------------------------------------------------
 * PURPOSE  : port link down notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_NotifyPortLinkDown(UI32_T unit, UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_NotifyPortAdminUp
 * ---------------------------------------------------------------------
 * PURPOSE  : port admin up notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_NotifyPortAdminUp(UI32_T unit, UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_MGR_NotifyPortAdminDown
 * ---------------------------------------------------------------------
 * PURPOSE  : port admin down notification
 * INPUT    : unit, port
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : networkaccess is upper than dot1x.
 *            therefore, dot1x can't register callback and
 *            networkaccess just call dot1x directly
 * ---------------------------------------------------------------------
 */
void DOT1X_MGR_NotifyPortAdminDown(UI32_T unit, UI32_T port);

#endif /* SYS_CPNT_NETACCESS == TRUE */

 /*-------------------------------------------------------------------------
  * FUNCTION NAME - DOT1X_MGR_ConvertAuthPaeState2MibValue
  *-------------------------------------------------------------------------
  * PURPOSE  : convert pae_state to mib_value
  * INPUT    : lport, pae_state
  * OUTPUT   : mib_value
  * RETURN   : none
  * NOTE     : none
  *-------------------------------------------------------------------------*/
 void DOT1X_MGR_ConvertAuthPaeState2MibValue(UI32_T lport, AUTH_PAE_STATE pae_state, UI32_T *mib_value);

 /*------------------------------------------------------------------------------
  * ROUTINE NAME : DOT1X_MGR_HandleIPCReqMsg
  *------------------------------------------------------------------------------
  * PURPOSE:
  *    Handle the ipc request message for DOT1X mgr.
  * INPUT:
  *    msg_p         --  the request ipc message buffer
  *    ipcmsgq_p     --  The handle of ipc message queue. The response message
  *                      will be sent through this handle.
  *
  * OUTPUT:
  *    None.
  *
  * RETURN:
  *    TRUE  - There is a response need to send.
  *    FALSE - There is no response to send.
  * NOTES:
  *    None.
  *------------------------------------------------------------------------------
  */
BOOL_T DOT1X_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msg_p);
#endif /*DOT1X_MGR_H*/

