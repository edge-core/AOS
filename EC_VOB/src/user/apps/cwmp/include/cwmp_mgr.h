/*-----------------------------------------------------------------------------
 * FILE NAME: CWMP_MGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE : Provide interface functions for any UI components to configure the
 *           parameters of ManagementServer object and for other UI components
 *           to enable/disable CWMP.
 *
 * NOTES   : None
 *
 * HISTORY : Date          Modifier,       Reason
 *           --------------------------------------------------------------
 *           2007-01-10    Timon Chang     create
 *
 * COPYRIGHT(C)         Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef CWMP_MGR_H
#define CWMP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"
#include "cwmp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define CWMP_MGR_IPCMSG_TYPE_SIZE sizeof(union CWMP_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
	CWMP_MGR_IPC_SETURL,
    CWMP_MGR_IPC_SETUSERNAME,
    CWMP_MGR_IPC_SETPASSWORD,
    CWMP_MGR_IPC_SETPERIODICINFORMENABLE,
    CWMP_MGR_IPC_SETPERIODICINFORMINTERVAL,
    CWMP_MGR_IPC_SETCONNECTIONREQUESTUSERNAME,
    CWMP_MGR_IPC_SETCONNECTIONREQUESTPASSWORD,
    CWMP_MGR_IPC_NOTIFYCONNECTIONREQUEST
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in CWMP_MGR_IpcMsg_T.data
 */
#define CWMP_MGR_GET_MSG_SIZE(field_name)                       \
            (CWMP_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((CWMP_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
	union CWMP_MGR_IpcMsg_Type_U
	{
		UI32_T  cmd;
        BOOL_T  ret_bool;
	} type; /* the intended action or return value */

	union
	{
	    BOOL_T  arg_bool;
	    UI32_T  arg_ui32;
	    char    arg_ar257[CWMP_TYPE_STR_LEN_256+1];
	} data; /* the argument(s) for the function corresponding to cmd */
} CWMP_MGR_IpcMsg_T;


/* EXPORTED SUBPROGRAM DECLARATIONS
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetThreadId
 * ------------------------------------------------------------------------
 * PURPOSE : Set the thread id of CWMP CSC thread to CWMP MGR.
 * INPUT   : thread_id - the thread id of CWMP CSC thread
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_MGR_SetThreadId(UI32_T thread_id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_GetOperationMode
 * ------------------------------------------------------------------------
 * PURPOSE : Get the operation mode of CWMP.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : operation mode
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T CWMP_MGR_GetOperationMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE : Set the operation mode to transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_MGR_SetTransitionMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_EnterTransitionMode
 * ------------------------------------------------------------------------
 * PURPOSE : Do all things in the beginning of transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_MGR_EnterTransitionMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_EnterMasterMode
 * ------------------------------------------------------------------------
 * PURPOSE : Do all things in the beginning of master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_MGR_EnterMasterMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_EnterSlaveMode
 * ------------------------------------------------------------------------
 * PURPOSE : Do all things in the beginning of slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_MGR_EnterSlaveMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetBootstrapFlag
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of bootstrap flag to the configuration file.
 * INPUT   : bootstrap -- TRUE: initial boot or the ACS URL is changed
 *                        FALSE: BOOTSTRAP event has been sent successfully
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : Only used by CWMP
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetBootstrapFlag(BOOL_T bootstrap);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetBootFlag
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of boot flag to the configuration file.
 * INPUT   : boot -- TRUE: initial boot or reboot
 *                   FALSE: BOOT event has been sent successfully
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : Only used by CWMP
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetBootFlag(BOOL_T boot);
#if 0
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetCwmpStatus
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of 'CWMP_GLOBAL_STATUS' to the configuration
 *           file and start/stop the CWMP process accordingly.
 * INPUT   : status -- TRUE: enabled; FALSE: disabled
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetCwmpStatus(BOOL_T status);
#endif
/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetUrl
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.URL" to
 *           the configuration file.
 * INPUT   : url -- URL for the device to contact the ACS
 *                  (must have been allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : It may be necessary to change username or password for
 *           connecting to the ACS URL which is set by the function.
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetUrl(char* url);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetUsername
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.Username"
 *           to the configuration file.
 * INPUT   : username -- username for the device to contact the ACS
 *                       (must have been allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetUsername(char* username);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetPassword
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.Password"
 *           to the configuration file.
 * INPUT   : password -- password for the device to contact the ACS
 *                       (must have been allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetPassword(char* password);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetPeriodicInformEnable
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           PeriodicInformEnable" to the configuration file.
 * INPUT   : status -- whether Inform must be sent periodically
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetPeriodicInformEnable(BOOL_T status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetPeriodicInformInterval
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           PeriodicInformInterval" to the configuration file.
 * INPUT   : interval - the duration in seconds between periodic Informs
 *                      (must be greater than 0)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetPeriodicInformInterval(UI32_T interval);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetPeriodicInformTime
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.Password"
 *           to the configuration file.
 * INPUT   : time - the absolute time reference for PeriodicInform
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : 1. Ref. WT-121v5-3.15 and WT-121v5-3.27 for time format
 *           2. Only used by CWMP
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetPeriodicInformTime(char* time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetParamterKey
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           ParameterKey" to the configuration file.
 * INPUT   : param_key - the value of ParamterKey
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : 1. Ref. TR-098 page 8 for detailed parameter definition
 *           2. Only used by CWMP
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetParamterKey(char* param_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetConnectionRequestUsername
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           ConnectionRequestUsername" to the configuration file.
 * INPUT   : cr_username -- username for the ACS to contact the device
 *                          (must be allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetConnectionRequestUsername(char* cr_username);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetConnectionRequestPassword
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           ConnectionRequestPassword" to the configuration file.
 * INPUT   : cr_password -- password for the ACS to contact the device
 *                          (must be allocated 257 bytes)
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetConnectionRequestPassword(char* cr_password);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_SetUpgradesManaged
 * ------------------------------------------------------------------------
 * PURPOSE : Set the value of "InternetGateway.ManagementServer.
 *           UpgradesManaged" to the configuration file.
 * INPUT   : state -- TRUE: SHOULD NOT use means other than the ACS to
 *                          seek out available upgrades
 *                    FALSE: MAY use other means for this purpose
 * OUTPUT  : None
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 * NOTES   : Only used by CWMP
 * ------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_SetUpgradesManaged(BOOL_T state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_NotifyConnectionRequest
 * ------------------------------------------------------------------------
 * PURPOSE : Notify CWMP that connetion request has been received and
 *           authenticated successfully.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_MGR_NotifyConnectionRequest(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_RifActiveCallBack
 * ------------------------------------------------------------------------
 * PURPOSE : Notify CWMP that a RIF is activated.
 * INPUT   : ip_addr -- the ip address of the specified rif.
 *           mask    -- the network mask of the specified rif.
 * OUTPUT  : None
 * RETURN  : None
 * NOTES   : None
 * ------------------------------------------------------------------------
 */
void CWMP_MGR_RifActiveCallBack(UI8_T *ip_addr, UI8_T *mask);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CWMP_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for CWMP MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T CWMP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


#endif /* end of #ifndef CWMP_MGR_H */
