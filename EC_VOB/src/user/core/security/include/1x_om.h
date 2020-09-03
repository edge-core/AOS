
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

#ifndef	DOT1X_OM_H
#define	DOT1X_OM_H

#include "1x_auth_pae.h"
#include "1x_common.h"
#include "dot1x_sm_auth.h"

/*------------------------------------------------------------------------
 * DEFAULT VARIABLES DECLARTIONS
 *-----------------------------------------------------------------------*/
/* Need to sync. sys_dflt.h */
#define		DOT1XMAC_TOTAL_MAC_TABLE                 200
#define		DOT1XMAC_MAX_MAC_TABLE_PER_PORT           10
#define		DOT1XMAC_MIN_MAC_TABLE_PER_PORT            1
#define		DOT1XMAC_MAC_LENGTH			       6

#if (SYS_CPNT_DOT1X_MACBASED_AUTH == TRUE)
#define     DOT1X_OM_MAX_SUPP_NBR_PER_PORT      5
#else
#define     DOT1X_OM_MAX_SUPP_NBR_PER_PORT      1
#endif

#define DOT1X_DEFAULT_EAPOL_PASS_THRU_STATUS    DOT1X_OM_EAPOL_PASS_THRU_DISABLED

/* MACRO FUNCTIONS DECLARACTION
 */
#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
typedef enum
{
    DOT1X_OM_EAPOL_PASS_THRU_ENABLED  = VAL_dot1xEapolPassThrough_enabled,
    DOT1X_OM_EAPOL_PASS_THRU_DISABLED = VAL_dot1xEapolPassThrough_disabled
}DOT1X_OM_EapolPassThru_T;
#else
typedef enum
{
    DOT1X_OM_EAPOL_PASS_THRU_ENABLED  = 1L,
    DOT1X_OM_EAPOL_PASS_THRU_DISABLED = 2L
}DOT1X_OM_EapolPassThru_T;
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

typedef enum DOT1X_OM_TimerType_E
{
    DOT1X_OM_TIMER_AWHILE        =   1,
    DOT1X_OM_TIMER_QUIETWHILE,
    DOT1X_OM_TIMER_REAUTHWHEN,
    DOT1X_OM_TIMER_TXWHEN,
    DOT1X_OM_TIMER_PROCESSWHILE

}DOT1X_OM_TimerType;

/*Need be defined in Sys_adpt.h ?*/
#define		DOT1XMAC_TOTAL_MAC_TABLE                 200
#define		DOT1XMAC_MAX_MAC_TABLE_PER_PORT           10
#define		DOT1XMAC_MIN_MAC_TABLE_PER_PORT            1
#define		DOT1XMAC_MAC_LENGTH			       6
/* DATA TYPE DECLARATIONS
 */

enum
{
    DOT1XMAC_STATE_INIT = 1,
    DOT1XMAC_STATE_CONNECTING,
    DOT1XMAC_STATE_RECONNECT,
    DOT1XMAC_STATE_DISCONNECT,
    DOT1XMAC_STATE_CLOSE,
    DOT1XMAC_STATE_TIMEOUT,
    DOT1XMAC_STATE_AUTH_RESP,
    DOT1XMAC_STATE_REQUEST,
    DOT1XMAC_STATE_HELD,
    DOT1XMAC_STATE_AUTHENTICATED,
};

typedef struct
{
    BOOL_T  used_flag;      			/* If the entry is uesd */
    UI32_T  lport;          			/* lport number */
    UI8_T   macaddr[DOT1XMAC_MAC_LENGTH]; /* mac address of supplicant */
    UI16_T  vlanid;         			/* vlan id of EAPOL packet */
    UI8_T   timertype;      			/* timer type */
    UI32_T  timeractive;    			/* timer active of not */
    BOOL_T  authflag;       			/* authenticated or not */
    UI8_T   currentid;      			/* current ID for EAPOL packet */
    UI8_T   *pkt_p;         			/* pointer point to packet buffer */
    UI8_T   authcount;      			/* times of authentication */
    UI8_T   reqcount;       			/* times of EAPOL-Request */
    UI8_T   current_state;          	/* current state of state machine */
    Global_Params *mac_entry_gp;		/* mac entry pointer */
    UI16_T  pre_entry_idx;  			/* previous mac_table_entry index */
    UI16_T  current_entry_idx;  		/* current mac_table_entry index */
    UI16_T  next_entry_idx; 			/* next mac_table_entry index */
} DOT1X_MacEntry_T;

#define DOT1X_OM_DATA_START_OFFSET (uintptr_t)(&(((DOT1X_OM_IPCMsg_T *)0)->data.data_start_offset))

enum
{
    DOT1X_OM_IPCCMD_GET_PORTMAXREQ,
    DOT1X_OM_IPCCMD_GET_GETRUNNING_MAXREQ,
    DOT1X_OM_IPCCMD_GET_RUNNING_PORT_REAUTH_MAX,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTMAXREQ,
    DOT1X_OM_IPCCMD_GET_PORTCONTROLMODE,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTCONTROLMODE,
    DOT1X_OM_IPCCMD_GET_PORTREAUTHENABLED,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTREAUTHENABLED,
    DOT1X_OM_IPCCMD_GET_PORTQUIETPERIOD,
    DOT1X_OM_IPCCMD_GET_GETRUNNING_QUIETPERIOD,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTQUIETPERIOD,
    DOT1X_OM_IPCCMD_GET_PORTREAUTHPERIOD,
    DOT1X_OM_IPCCMD_GETRUNNING_REAUTHPERIOD,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTREAUTHPERIOD,
    DOT1X_OM_IPCCMD_GET_PORTTXPERIODD,
    DOT1X_OM_IPCCMD_GETRUNNING_TXPERIOD,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTTXPERIOD,
    DOT1X_OM_IPCCMD_GET_SYSTEMAUTHCONTROL,
    DOT1X_OM_IPCCMD_GETRUNNING_SYSTEMAUTHCONTROL,
    DOT1X_OM_IPCCMD_GET_PAE_PORT_ENTRY,
    DOT1X_OM_IPCCMD_GET_NEXT_PAE_PORT_ENTRY,
    DOT1X_OM_IPCCMD_GET_AUTH_CONFIG_ENTRY,
    DOT1X_OM_IPCCMD_GET_NEXT_AUTH_CONFIG_ENTRY,
    DOT1X_OM_IPCCMD_GET_AUTH_STATS_ENTRY,
    DOT1X_OM_IPCCMD_GET_NEXT_AUTH_STATS_ENTRY,
    DOT1X_OM_IPCCMD_GET_AUTH_DIAG_ENTRY,
    DOT1X_OM_IPCCMD_GET_NEXT_AUTH_DIAG_ENTRY,
    DOT1X_OM_IPCCMD_GET_GLOBAL_PARAMETERS,
    DOT1X_OM_IPCCMD_GET_PORT_AUTHORIZED,
    DOT1X_OM_IPCCMD_GET_PORT_DETAILS,
    DOT1X_OM_IPCCMD_GET_NEXT_PORT_DETAILS,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTMULTIHOSTMACCOUNT,
    DOT1X_OM_IPCCMD_GET_PORTREAUTHMAX,
    DOT1X_OM_IPCCMD_GETRUNNING_PORTOPERATIONMODE,
    DOT1X_OM_IPCCMD_GETNEXT_MAC_AUTH_STATS_TABLE,
    DOT1X_OM_IPCCMD_GETRUNNING_AUTH_SUPP_TIMEOUT,
    DOT1X_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC
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
            UI32_T ui32arg;
        }lport_ui32arg;

        struct
        {
            UI32_T lport;
            BOOL_T boolarg;
        }lport_boolarg;

        struct
        {
            UI32_T lport;
            DOT1X_PaePortEntry_T paeportentry;
        }lport_paeportentry;

        struct
        {
            UI32_T lport;
            DOT1X_AuthConfigEntry_T authconfigentry;
        }lport_authconfigentry;


        struct
        {
            UI32_T lport;
            DOT1X_AuthStatsEntry_T authstateentry;
        }lport_authstateentry;

        struct
        {
            UI32_T lport;
            DOT1X_AuthDiagEntry_T authdiagentry;
        }lport_authdiagentry;

        DOT1X_Global_Parameters_T global_parameters;

        struct
        {
            UI32_T lport;
            DOT1X_PortDetails_T port_details;
        }lport_portdetails;

        struct
        {
            UI32_T lport;
            UI32_T mac_index;
            DOT1X_PortDetails_T port_details;
        }lport_macidx_portdetails;

        struct
        {
            UI32_T lport;
            UI32_T mode;
        }lport_mode;

        struct
        {
            UI32_T lport;
            UI32_T mac_index;
            DOT1X_AuthStatsEntry_T AuthStateEntry;
            UI8_T supplicant_mac[DOT1XMAC_MAC_LENGTH];
        }lport_mac_authstate_mac;
    } data; /* contains the supplemntal data for the corresponding cmd */
}__attribute__((packed, aligned(1)))DOT1X_OM_IPCMsg_T;

/* GLOBAL CONFIGURATION VALUE */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_OM_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : set the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void DOT1X_OM_SetDebugFlag(UI32_T flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_OM_GetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : get the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
UI32_T DOT1X_OM_GetDebugFlag();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DOT1X_OM_GetDebugPrompt
 *-------------------------------------------------------------------------
 * PURPOSE  : get the debug prompt
 * INPUT    : flag -- debug flag
 * OUTPUT   : none
 * RETURN   : string form of debug flag
 * NOTES    : none
 *-------------------------------------------------------------------------*/
const char* DOT1X_OM_GetDebugPrompt(UI32_T flag);

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
BOOL_T DOT1X_OM_EnterCriticalRegion(void);

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
BOOL_T DOT1X_OM_LeaveCriticalRegion(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetSMObjByPortMac
 * ---------------------------------------------------------------------
 * PURPOSE : Get state machine object
 * INPUT   : lport, mac
 * OUTPUT  : sm_p
 * RETURN  : The pointer of state machine object
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T *DOT1X_OM_GetSMObjByPortMac(UI32_T lport, UI8_T *mac);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_NewSMObj
 * ---------------------------------------------------------------------
 * PURPOSE : Allocate a new state machine object
 * INPUT   : lport      - logic port number
 *           mac        - source MAC of the supplicant
 *           eapol      - If TRUE, new state machine object for receiving
 *                        an EAPOL packet. It have higher priority than
 *                        state machine occupied by an non-EAPOL packet.
 * OUTPUT  : None
 * RETURN  : The pointer of the new state machine object
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T *DOT1X_OM_NewSMObj(UI32_T lport, UI8_T *mac, BOOL_T eapol);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextWorkingSMObj
 * ---------------------------------------------------------------------
 * PURPOSE : Get next working state machine object
 * INPUT   : lport, idx_p
 * OUTPUT  : None
 * RETURN  : The pointer of the new state machine object
 * NOTES   : Use *idx_p = 0 to get first
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T* DOT1X_OM_GetNextWorkingSMObj(UI32_T lport, UI32_T *idx_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetStateMachineObj
 * ---------------------------------------------------------------------
 * PURPOSE : Get state machine object
 * INPUT   : lport
 * OUTPUT  : sm_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T *DOT1X_OM_GetStateMachineObj(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetSMObj
 * ---------------------------------------------------------------------
 * PURPOSE : Get state machine object
 * INPUT   : lport, mac_index
 * OUTPUT  : sm_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_SM_AUTH_Obj_T *DOT1X_OM_GetSMObj(UI32_T lport, UI32_T mac_index);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthStats
 * ---------------------------------------------------------------------
 * PURPOSE : Get dot1xAuthStatsTable entry pointer
 * INPUT   : lport, mac_index
 * OUTPUT  : None
 * RETURN  : The pointer of dot1xAuthStatsTable entry
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
DOT1X_AuthStatsEntry_T* DOT1X_OM_GetAuthStats(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE : Init OM database
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Initialize();

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE : Set config setting to default
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
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
 * ROUTINE NAME - DOT1X_OM_Set_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE : This function will set port control mode of 1X configuration
 * INPUT   : lport
 *           mode - VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized,
 *                  VAL_dot1xAuthAuthControlledPortControl_forceAuthorized or
 *                  VAL_dot1xAuthAuthControlledPortControl_auto
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortControlMode(UI32_T lport, UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE : This function will get port control mode of 1X configuration
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized,
 *           VAL_dot1xAuthAuthControlledPortControl_forceAuthorized or
 *           VAL_dot1xAuthAuthControlledPortControl_auto
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortControlMode(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetRunning_PortControlMode
 * ---------------------------------------------------------------------
 * PURPOSE : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default PortControlMode is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : None
 * OUTPUT  : value_p - VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized,
 *                     VAL_dot1xAuthAuthControlledPortControl_forceAuthorized or
 *                     VAL_dot1xAuthAuthControlledPortControl_auto
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE or
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default system name.
 *           3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_PortControlMode(UI32_T lport, UI32_T *value_p);

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

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPortAuthorized
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
DOT1X_TYPE_AuthControlledPortStatus_T DOT1X_OM_GetPortAuthorized(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Set_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Set period re-authentication status of the specified port
 * INPUT   : lport - port number
 *           value - VAL_dot1xPaePortReauthenticate_true
 *                   VAL_dot1xPaePortReauthenticate_false
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortReAuthEnabled(UI32_T lport, UI32_T value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get period re-authentication status of the specified port
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : VAL_dot1xPaePortReauthenticate_true or
 *           VAL_dot1xPaePortReauthenticate_false
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_PortReAuthEnabled(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *           the non-default PortControlMode is successfully retrieved.
 *           Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT   : lport
 * OUTPUT  : value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE or
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *           3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T  DOT1X_OM_GetRunning_PortReAuthEnabled(UI32_T lport, UI32_T *value_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNext_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get period re-authentication status of the next port
 * INPUT   : lport_p
 * OUTPUT  : lport_p - the next port number
 *           value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_PortReAuthEnabled(UI32_T *lport_p, UI32_T *value_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextRunning_PortReAuthEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get period re-authentication status of the next port
 * INPUT   : lport_p
 * OUTPUT  : lport_p - the next port number
 *           value_p - VAL_dot1xPaePortReauthenticate_true or
 *                     VAL_dot1xPaePortReauthenticate_false
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE or
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default system name.
 *           3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetNextRunning_PortReAuthEnabled(UI32_T *lport_p, UI32_T *value_p);

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
 * ROUTINE NAME  - DOT1X_OM_GetRunning_PortReAuthMax
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
UI32_T  DOT1X_OM_GetRunning_PortReAuthMax(UI32_T lport,UI32_T *max_req);

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

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetPortEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Set the value of port enabled
 * INPUT   : lport -- port number
 *           value -- TRUE: port is link up
 *                    FALSE: port is link down
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetPortEnabled(UI32_T lport, BOOL_T value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPortEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get the value of port enabled
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : TRUE: port is link up
 *           FALSE: port is link down
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetPortEnabled(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE : Set the value of dot1xAuthAdminControlledDirections
 * INPUT   : lport - port number
 *           value - VAL_dot1xAuthAdminControlledDirections_both or
 *                   VAL_dot1xAuthAdminControlledDirections_in
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AdminCtrlDirections(UI32_T lport, UI32_T value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_AdminCtrlDirections
 * ---------------------------------------------------------------------
 * PURPOSE : Get the value of dot1xAuthAdminControlledDirections
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : VAL_dot1xAuthAdminControlledDirections_both or
 *           VAL_dot1xAuthAdminControlledDirections_in
 * NOTES   : None
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
 * ROUTINE NAME  - DOT1X_OM_GetRunning_AuthSuppTimeout
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default supp-timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: AuthSuppTimeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetRunning_AuthSuppTimeout(UI32_T lport,UI32_T *seconds);

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
 * ROUTINE NAME - DOT1X_OM_Set_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Set the value of the keyTransmissionEnabled constant currently
             in use by the Authenticator PAE state machine.
 * INPUT   : lport
 *           value
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_AuthTxEnabled(UI32_T lport, UI32_T value);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_AuthTxEnabled
 * ---------------------------------------------------------------------
 * PURPOSE : Get the value of the keyTransmissionEnabled constant currently
             in use by the Authenticator PAE state machine.
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : VAL_dot1xAuthKeyTxEnabled_true or
 *           VAL_dot1xAuthKeyTxEnabled_false
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
UI32_T DOT1X_OM_Get_AuthTxEnabled(UI32_T lport);

/* for 802.1x MIB (IEEE8021-PAE-MIB)
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetPaePortEntry(UI32_T lport, DOT1X_PaePortEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextPaePortEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xPaePortTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextPaePortEntry(UI32_T *lport_p, DOT1X_PaePortEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthConfigEntry(UI32_T lport, DOT1X_AuthConfigEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthConfigEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthConfigTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthConfigEntry(UI32_T *lport_p, DOT1X_AuthConfigEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthStatsEntry(UI32_T lport, DOT1X_AuthStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthStatsTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthStatsEntry(UI32_T *lport_p, DOT1X_AuthStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthDiagEntry(UI32_T lport, DOT1X_AuthDiagEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthDiagEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthDiagTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthDiagEntry(UI32_T *lport_p, DOT1X_AuthDiagEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetAuthSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the specified entry of dot1xAuthSessionStatsTable
 * INPUT   : lport
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetAuthSessionStatsEntry(UI32_T lport, DOT1X_AuthSessionStatsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetNextAuthSessionStatsEntry
 * ---------------------------------------------------------------------
 * PURPOSE : Get the next entry of dot1xAuthSessionStatsTable
 * INPUT   : lport
 * OUTPUT  : lport
 *           entry_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextAuthSessionStatsEntry(UI32_T *lport_p, DOT1X_AuthSessionStatsEntry_T *entry_p);

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

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortOperationMode
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
BOOL_T DOT1X_OM_Set_PortOperationMode(UI32_T lport,UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_PortOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the authenticator's operation mode
 * INPUT:  lport.
 * OUTPUT: mode.
 * RETURN: TRUE/FALSE.
 * NOTES:  DOT1X_PORT_OPERATION_MODE_ONEPASS   for OnePass (Single-Host)
 *         DOT1X_PORT_OPERATION_MODE_MULTIPASS for MultiPass (Multi-Host)
 *         DOT1X_PORT_OPERATION_MODE_MACBASED  for MAC-based
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_PortOperationMode(UI32_T lport,UI32_T *mode);

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
 * ROUTINE NAME - DOT1X_OM_IsPortBasedMode
 * ---------------------------------------------------------------------
 * PURPOSE : Is the port running on port based mode or not
 * INPUT   : lport
 * OUTPUT  : None
 * RETURN  : TRUE -- port based mode
 *           FALSE -- non port based mode (may be MAC-based)
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_IsPortBasedMode(UI32_T lport);

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
 * ROUTINE NAME  - DOT1X_OM_Get_PortMultiHostMacCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get the max allowed MAC number in multi-host mode .
 * INPUT:  lport,times.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_PortMultiHostMacCount(UI32_T lport,UI32_T *count);

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
 * ROUTINE NAME - DOT1X_OM_Set_PortReAuthMax
 * ---------------------------------------------------------------------
 * PURPOSE : Set the port's re-auth max
 * INPUT   : lport, value
 * OUTPUT  : None
 * RETURN  : reauth_max
 * NOTES   : None
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Set_PortReAuthMax(UI32_T lport, UI32_T reauth_max);

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

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Set_PortTimer
 * ---------------------------------------------------------------------
 * PURPOSE: Set the port's timer
 * INPUT:  lport,timer,type.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
 BOOL_T DOT1X_OM_Set_PortTimer(UI32_T lport,int port_timer,DOT1X_OM_TimerType type);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Init_PortTimer
 * ---------------------------------------------------------------------
 * PURPOSE: Initialize the port's timer
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Init_PortTimer(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Add_Authenticated_MAC
 * ---------------------------------------------------------------------
 * PURPOSE: Add a authenticated MAC
 * INPUT:  lport,mac,vid.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Add_Authenticated_MAC(UI32_T lport,UI8_T *mac, UI32_T vid,UI32_T src_vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Delete_Authenticated_MAC
 * ---------------------------------------------------------------------
 * PURPOSE: Remove a authenticated MAC
 * INPUT:  lport,mac,vid.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Delete_Authenticated_MAC(UI32_T lport,UI8_T *mac, UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Delete_Port_MAC
 * ---------------------------------------------------------------------
 * PURPOSE: Remove all the authenticated MACs on the port
 * INPUT:  lport.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Delete_Port_MAC(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Is_Authenticated_MAC
 * ---------------------------------------------------------------------
 * PURPOSE: Check if the MAC is a authenticated MAC
 * INPUT:  lport,mac.
 * OUTPUT: None.
 * RETURN: TRUE/FALSE.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Is_Authenticated_MAC(UI32_T lport,UI8_T *mac,UI32_T *src_vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_Authenticated_MAC
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next authenticated MAC's State Machine entry
 * INPUT:  lport,mac_index.
 * OUTPUT: sm_entry.
 * RETURN: TRUE/FALSE.
 * NOTES:  if mac_index is 0, then get the first authenticated MAC's State Machine entry.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNext_Authenticated_MAC(UI32_T lport,UI32_T *mac_index,UI32_T *src_vid,UI8_T *src_mac);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1XMAC_OM_InitMacEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Initial the mac entry database
 * INPUT:   mac_entry_ptr     -- mac entry structure pointer.
 *	    current_entry_idx -- entry index.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void DOT1XMAC_OM_InitMacEntry(DOT1X_MacEntry_T * mac_entry_ptr, UI32_T current_entry_idx);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Get_MacEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Get the mac entry database
 * INPUT:   entry index.
 * OUTPUT:  None.
 * RETURN:  mac entry structure pointer.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
DOT1X_MacEntry_T * DOT1X_OM_Get_MacEntry(UI32_T mac_entry_index);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_FindMACEntryPtr
 * ---------------------------------------------------------------------
 * PURPOSE: Find the mac's working area .If the mac's working area don't exist,
 *          return NULL.
 * INPUT:   lport -- lport number
 *          mac   -- mac address
 *          vid   -- vlan id
 * OUTPUT:  None.
 * RETURN:  MAC entry pointer.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
DOT1X_MacEntry_T * DOT1X_OM_FindMACEntryPtr(UI32_T lport,UI8_T *mac,UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetNext_MACEntryPtr
 * ---------------------------------------------------------------------
 * PURPOSE: Get the next mac's working area .If the mac's working area don't exist,
 *          return NULL.
 * INPUT:   lport -- lport number
 * OUTPUT:  mac table index.
 * RETURN:  MAC entry pointer.
 * NOTES:   index = 0xFFFF --- get the first entry on the port.
 * ---------------------------------------------------------------------
 */
DOT1X_MacEntry_T * DOT1X_OM_GetNext_MACEntryPtr(UI32_T lport,UI32_T *mac_index);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Free_MacEntry_Working_Area
 * ---------------------------------------------------------------------
 * PURPOSE: Free a mac entry database to mac entry pool and delete a record from HISAM
 * INPUT:   lport -- lport number
 *          mac   -- mac address
 *          vid   -- vlan id
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Free_MacEntry_Working_Area(UI32_T lport,UI8_T *mac,UI32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_Port_Free_All_MacEntry_Working_Area
 * ---------------------------------------------------------------------
 * PURPOSE: Free all the mac entry database linked to the port
 * INPUT:   lport -- lport number
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Port_Free_All_MacEntry_Working_Area(UI32_T lport);
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DOT1X_OM_GetMACStateMachineWorkingArea
 * ---------------------------------------------------------------------
 * PURPOSE: Get DOT1XMAC State Machine Working Area
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
DOT1X_MacEntry_T * DOT1X_OM_GetMACStateMachineWorkingArea(UI32_T lport,UI8_T *mac,UI32_T vid);

UI32_T DOT1X_OM_Get_RadiusMsgQid();
BOOL_T DOT1X_OM_Set_RadiusMsgQid(UI32_T msg_id);
Global_Params* DOT1X_OM_Get_MACgp(UI32_T entry_index);
Auth_Pae* DOT1X_OM_Get_MACAuthPae(UI32_T entry_index);

BOOL_T DOT1X_OM_SetDot1xMsgQId(UI32_T dot1x_msgq_id);
BOOL_T DOT1X_OM_GetDot1xMsgQId(UI32_T *dot1x_msgq_id);
BOOL_T DOT1X_OM_SetTaskServiceFunPtr(UI32_T cookie, UI8_T service_type);
BOOL_T DOT1X_OM_GetTaskServiceFunPtr(UI32_T *cookie, UI8_T service_type);

/* -----------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetPortIntrusionActionStatus
 * -----------------------------------------------------------------------
 * FUNCTION : Set per-port intrusion action
 * INPUT    : lport - port number
 *            value - VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic or
 *                    VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * -----------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetPortIntrusionActionStatus(UI32_T lport, UI32_T value);

/* -----------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetPortIntrusionActionStatus
 * -----------------------------------------------------------------------
 * FUNCTION : Get per-port intrusion action
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic or
 *            VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
 * NOTE     : None
 * -----------------------------------------------------------------------
 */
UI32_T DOT1X_OM_GetPortIntrusionActionStatus(UI32_T lport);

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_SetEapolPassThrough
 *------------------------------------------------------------------------
 * FUNCTION: Set status of EAPOL frames pass-through
 * INPUT   : status - DOT1X_OM_EAPOL_PASS_THRU_ENABLED(1)
 *                    DOT1X_OM_EAPOL_PASS_THRU_DISABLED(2)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_SetEapolPassThrough(DOT1X_OM_EapolPassThru_T status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_GetEapolPassThrough
 *------------------------------------------------------------------------
 * FUNCTION: Get the status of EAPOL frames pass-through
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : status - DOT1X_OM_EAPOL_PASS_THRU_ENABLED(1)
 *                    DOT1X_OM_EAPOL_PASS_THRU_DISABLED(2)
 * NOTE    :
 *------------------------------------------------------------------------
 */
DOT1X_OM_EapolPassThru_T DOT1X_OM_GetEapolPassThrough();

/* ---------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_ConvertAuthStateToPaeState
 * ---------------------------------------------------------------------
 * PURPOSE : Convert state of state machine to backend state
 * INPUT   : state
 * OUTPUT  : pae_value_p
 * RETURN  : None
 * NOTE    : None
 * ---------------------------------------------------------------------
 */
void DOT1X_OM_ConvertAuthStateToPaeState(DOT1X_SM_AUTH_State_T state, UI32_T *pae_value_p);

/* ---------------------------------------------------------------------
 * FUNCTION NAME - DOT1X_OM_ConvertAuthStateToBackendState
 * ---------------------------------------------------------------------
 * PURPOSE : Convert state of state machine to backend state
 * INPUT   : state
 * OUTPUT  : backend_state_p
 * RETURN  : None
 * NOTE    : None
 * ---------------------------------------------------------------------
 */
void DOT1X_OM_ConvertAuthStateToBackendState(DOT1X_SM_AUTH_State_T state, UI32_T *backend_state_p);

/*****************For CLI Show do1x ****************************************/
/*--------------------------------------------------------------------------
 * ROUTINE NAME - DOT1X_OM_Get_Global_Parameters
 *---------------------------------------------------------------------------
 * PURPOSE:  Get dot1x global parameters.
 * INPUT:    global_parameters pointer.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
 void DOT1X_OM_Get_Global_Parameters(DOT1X_Global_Parameters_T * global_parameters);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_OM_Get_Port_Details
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_Get_Port_Details(UI32_T lport, DOT1X_PortDetails_T *port_details_p);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: DOT1X_OM_GetNextPortDetails
 * ------------------------------------------------------------------------
 * PURPOSE : Get dot1x port detail information
 * INPUT   : lport
 * OUTPUT  : port_details_p
 * RETURN  : TRUE/FALSE
 * NOTE    : Use empty supplicant MAC address to get first
 * ------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_GetNextPortDetails(UI32_T lport, UI32_T *index_p, DOT1X_PortDetails_T *details_p);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DOT1X_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *------------------------------------------------------------------------------
 */
BOOL_T DOT1X_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif /*DOT1X_OM_H*/

