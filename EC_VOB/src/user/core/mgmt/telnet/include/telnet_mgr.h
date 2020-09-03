/* MODULE NAME: telnet_mgr.h
* PURPOSE: 
*   Initialize the resource and provide some functions for the telnet module.
*
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-02         -- jason wang , created.
*  06/05/2007         -- Rich Lee,    Transform to Linux Platform
* Copyright(C)      Accton Corporation, 2002
*/

#ifndef _TELNET_MGR_H_
#define _TELNET_MGR_H_


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "l_inet.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define TELNET_MGR_MSGBUF_TYPE_SIZE sizeof(union TELNET_MGR_IPCMSG_TYPE_U)

#define TELNET_MGR_GET_MSGBUFSIZE(field_name)                       \
            (TELNET_MGR_MSGBUF_TYPE_SIZE +                        \
            sizeof(((TELNET_MGR_IPCMsg_T*)0)->data.field_name))



/* DATA TYPE DECLARATIONS
 */
/*isiah. patch for enabled/disabled telnetd*/
typedef enum TELNET_State_E
{
    TELNET_STATE_ENABLED = 1L, /* VAL_ipSshdState_enabled  */
    TELNET_STATE_DISABLED = 2L /* VAL_ipSshdState_disabled */
} TELNET_State_T;

/* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
typedef struct TELNET_Options_S
{
	UI32_T remote_port;
	char myopts[256];
	char hisopts[256];
} TELNET_Options_T;

/* structure for the request/response ipc message in telnet pmgr and mgr
 */
typedef struct
{
    union TELNET_MGR_IPCMSG_TYPE_U
    {
        UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result; /* for response */
    } type;

    union
    {
        /* the structure which is used when 
         * cmd==TELNET_MGR_IPC_GET_TNPD_PORT / TELNET_MGR_IPC_SET_TNPD_PORT
         */
        struct TELNET_MGR_IPCMSG_PORT_DATA_S
        {
            UI32_T port;            
        } port_data; 

        /* the structure which is used when
         * cmd==TELNET_MGR_IPC_GET_TNPD_STATUS / TELNET_MGR_IPC_SET_TNPD_STATUS
         */
        struct TELNET_MGR_IPCMSG_STATUS_DATA_S
        {
            TELNET_State_T state;
        } status_data; 

        /* the structure which is used when
         * cmd==TELNET_MGR_IPC_GET_TNPD_MAX_SESSION / TELNET_MGR_IPC_SET_TNPD_MAX_SESSION
         */
        struct TELNET_MGR_IPCMSG_MAX_SESSION_DATA_S
        {
            TELNET_State_T maxSession;
        } max_session_data; 

        struct TELNET_MGR_IPCMSG_CLUSTER_RELAY_DATA_S
        {
            UI32_T  task_id;
            BOOL_T  bRelaying;
            UI8_T   memberId;
        } cluster_relay_data;

        struct TELNET_MGR_IPCMSG_CLUSTER_member_DATA_S
        {
            UI8_T   memberId;
        } cluster_member_data;

        /* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
        TELNET_Options_T options_data;
    } data; /* contains the supplemntal data for the corresponding cmd */
} TELNET_MGR_IPCMsg_T;

/* definitions of command in CSCA which will be used in ipc message
 */
enum
{
    TELNET_MGR_IPC_GET_TNPD_PORT,
    TELNET_MGR_IPC_SET_TNPD_PORT,
    TELNET_MGR_IPC_GET_TNPD_STATUS,
    TELNET_MGR_IPC_SET_TNPD_STATUS,
    TELNET_MGR_IPC_GET_TNPD_MAX_SESSION,
    TELNET_MGR_IPC_SET_TNPD_MAX_SESSION,
    /* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
    TELNET_MGR_IPC_GET_TNPD_OPTIONS,
    TELNET_MGR_IPC_SET_TELNET_RELAYING,
    TELNET_MGR_IPC_CLUSTER_TO_MEMBER_FROM_UART
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for telnet mgr.
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
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T TELNET_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TELNET_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TELNET_MGR_Create_InterCSC_Relation(void);

/* FUNCTION NAME:  TELNET_MGR_EnterMasterMode
 * PURPOSE: 
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the TELNET subsystem will enter the
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
 *          2. TELNET will handle network requests only when this subsystem
 *				is in the Master Operation mode 
 *          3. This function is invoked in TELNET_INIT_EnterMasterMode.
 */
BOOL_T TELNET_MGR_EnterMasterMode(void);

/* FUNCTION NAME:  TELNET_MGR_EnterTransitionMode
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
BOOL_T TELNET_MGR_EnterTransitionMode(void);

/* FUNCTION NAME:  TELNET_MGREnterSlaveMode
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
void TELNET_MGR_EnterSlaveMode(void);

/* FUNCTION NAME : TELNET_MGR_SetTransitionMode
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
void TELNET_MGR_SetTransitionMode(void);

/* FUNCTION NAME : TELNET_MGR_GetOperationMode
 * PURPOSE:
 *      Get current telnet operation mode (master / slave / transition).
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
SYS_TYPE_Stacking_Mode_T TELNET_MGR_GetOperationMode(void);

/* FUNCTION NAME:  TELNET_MGR_GetTnpdStatus
 * PURPOSE: 
 *          This function get TELNET state.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TELNET_State_T - TELNET status.
 * NOTES:
 *          .
 */
TELNET_State_T TELNET_MGR_GetTnpdStatus(void);

/* FUNCTION NAME:  TELNET_MGR_SetTnpdMaxSession
 * PURPOSE: 
 *          This function set TELNET max session number.
 *
 * INPUT:   
 *          max session number.
 *
 * OUTPUT:  
 *          none.
 *
 * RETURN:  
 *          TRUE  -- successful
 *          FALSE -- failed
 *
 * NOTES:
 *
 */
BOOL_T TELNET_MGR_SetTnpdMaxSession(UI32_T maxSession);


/* FUNCTION NAME:  TELNET_MGR_GetTnpdMaxSession
 * PURPOSE: 
 *          This function get TELNET max session number.
 *
 * INPUT:   
 *          none.
 *
 * OUTPUT:  
 *          none.
 *
 * RETURN:  
 *          max session number.
 *
 * NOTES:
 *
 */
UI32_T TELNET_MGR_GetTnpdMaxSession();

/* FUNCTION NAME:  TELNET_MGR_GetRunningTnpdMaxSession
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific TELNET max sessions number with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *
 * OUTPUT:  
 *          UI32_T *pMaxSession -- TELNET max session number.
 *
 * RETURN:  
 *          SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL                                           
 *
 * NOTES:   
 *          1. This function shall only be invoked by CLI to save the 
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this 
 *             function shall return non-default structure for each field for the device.
 *
 */
SYS_TYPE_Get_Running_Cfg_T TELNET_MGR_GetRunningTnpdMaxSession(UI32_T *pMaxSession);

/* FUNCTION NAME:  TELNET_MGR_GetTnpdStatus
 * PURPOSE: 
 *          This function set TELNET state.
 *
 * INPUT:   
 *          TELNET_State_T - TELNET status.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 *          .
 */
BOOL_T TELNET_MGR_SetTnpdStatus (TELNET_State_T state);

/* FUNCTION NAME:  TELNET_MGR_GetRunningTnpdStatus
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific TELNET Status with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T * - TELNET Status.
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
SYS_TYPE_Get_Running_Cfg_T  TELNET_MGR_GetRunningTnpdStatus(UI32_T *state);

/* FUNCTION NAME:  TELNET_MGR_GetTnpdPort
 * PURPOSE: 
 *          This function get TELNET port number.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          UI32_T  *port   --  TELNET port number.
 *                                   
 * RETURN:  
 *          TRUE.
 * NOTES:
 *          .
 */
BOOL_T TELNET_MGR_GetTnpdPort(UI32_T *port);

/* FUNCTION NAME:  TELNET_MGR_SetTnpdPort
 * PURPOSE: 
 *          This function set TELNET port number.
 *
 * INPUT:   
 *          UI32_T  port    --  TELNET port number.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          TRUE    --  success.  
 *          FALSE   --  failure.
 * NOTES:
 *          .
 */
BOOL_T TELNET_MGR_SetTnpdPort(UI32_T port);

/* FUNCTION NAME:  TELNET_MGR_GetRunningTnpdPort
 * PURPOSE: 
 *			This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the 
 *          specific TELNET port number with non-default values 
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL 
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 *
 * INPUT:   
 *          none. 
 *                                   
 * OUTPUT:  
 *          UI32_T *port -- TELNET port number.
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
SYS_TYPE_Get_Running_Cfg_T  TELNET_MGR_GetRunningTnpdPort(UI32_T *port);

/* FUNCTION NAME - TELNET_MGR_HandleHotInsertion
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
void TELNET_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* FUNCTION NAME - TELNET_MGR_HandleHotRemoval
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
void TELNET_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#if (SYS_CPNT_CLUSTER == TRUE)

/* FUNCTION NAME : TELNET_MGR_SetTelnetRelaying
  *PURPOSE:
 *        This function used for commander connect to member's telnet service from telnet
 *INPUT:
 *        UI32_T task_id, BOOL_T bRelaying, UI8_T memberId
 *OUTPUT:
 *        none.
 *RETURN:
 *       TRUE    --  success.
 *       FALSE   --  failure.
 */
BOOL_T TELNET_MGR_SetTelnetRelaying(UI32_T task_id, BOOL_T bRelaying, UI8_T memberId);

/*FUNCTION NAME: TELNET_MGR_ClusterToMemberFromUART
  *PURPOSE:
 *        This function used for commander connect to member's telnet service from console
 *INPUT:
 *        UI8_T member_id --member id.
 *OUTPUT:  
 *        none.
 *RETURN:
 *       TRUE    --  success.
 *       FALSE   --  failure. 
  */
BOOL_T TELNET_MGR_ClusterToMemberFromUART(UI8_T member_id);

#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

#if (SYS_CPNT_TELNET == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_MGR_RifDestroyedCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback to Telnet if rip destroy (IP address is changed)
 *
 * INPUT:
 *    ip_addr_p  -- the ip address which is destroyed
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
void TELNET_MGR_RifDestroyedCallbackHandler(L_INET_AddrIp_T *ip_addr_p);
#endif /* #if (SYS_CPNT_TELNET == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : TELNET_MGR_MgmtIPFltChangedCallbackHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Callback to Telnet if the database of mgmt IP filter was changed
 *
 * INPUT:
 *    mode  --  which mode of mgmt IP filter was changed
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
void TELNET_MGR_MgmtIPFltChangedCallbackHandler(UI32_T mode);
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

#endif /* #ifndef _TELNET_MGR_H_ */

