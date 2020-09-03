/* Module Name: PSEC_MGR.H
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the hardware independent interface of port security
 *         control functions to applications.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes port security manipulation.
 *        ( 3.  The domain would not be handled by this module. )
 *         None.
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *      2002/5/30    Arthur Wu   Create this file
 *      2004/10/22   Hendra Lin  Added Port Security Learning Status
 *
 *
 * Copyright(C)      Accton Corporation, 2002
 */
/* NOTES:
 * 1. VS2524 MIB contains the following group:
 *    a) PortSecurityMgt
 */
#ifndef PSEC_MGR_H
#define PSEC_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"

/* NAMING CONSTANT
 */
#define SYS_DFLT_PORT_SECURITY_STATUS                   VAL_portSecPortStatus_disabled
#define SYS_DFLT_PORT_SECURITY_ACTION_STATUS            VAL_portSecAction_none

#define PSEC_MGR_INTERFACE_INDEX_FOR_ALL                0

/* The key to get igmpsnp mgr msgq.
 */
#define PSEC_MGR_IPCMSG_KEY    SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY

/* These value will be use by mgr handler to set msg.type.result
 *   PSEC_MGR_IPC_RESULT_OK   - only use when API has no return value
 *                              and mgr deal this request.
 *   PSEC_MGR_IPC_RESULT_FAIL - it denote that mgr handler can't deal
 *                              the request. (ex. in transition mode)
 */
#define PSEC_MGR_IPC_RESULT_OK    (0)
#define PSEC_MGR_IPC_RESULT_FAIL  (-1)

/* The commands for IPC message.
 */
enum {
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_STATUS,
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ACTION_ACTIVE,
    PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_ACTION_ACTIVE,
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LAST_INTRUSION_MAC,
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LAST_INTRUSION_TIME,
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ACTION_STATUS,
    PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_STATUS,
    PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_ACTION_STATUS,
    PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_MAC_COUNT,
    PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_MAC_COUNT_OPERATION,
    PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_MAC_COUNT,
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ENTRY,
    PSEC_MGR_IPC_CMD_GET_NEXT_PORT_SECURITY_ENTRY,
    PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_STATUS,
    PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_STATUS_OPERATION,
    PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_ACTION_STATUS,
    PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_LEARNING_STATUS,
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LEARNING_STATUS,
    PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_MAC_COUNT,
    PSEC_MGR_IPC_CMD_GET_MIN_NBR_OF_MAX_MAC_COUNT,
    PSEC_MGR_IPC_CMD_CONVERT_SECURED_ADDRESS_INTO_MANUAL,
};


/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - PSEC_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of PSEC message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of PSEC message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define PSEC_MGR_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)&((PSEC_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of PSEC message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of PSEC message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(PSEC_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - PSEC_MGR_MSG_CMD
 *              PSEC_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the PSEC command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The PSEC command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define PSEC_MGR_MSG_CMD(msg_p)    (((PSEC_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define PSEC_MGR_MSG_RETVAL(msg_p) (((PSEC_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - PSEC_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define PSEC_MGR_MSG_DATA(msg_p)   ((void *)&((PSEC_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)


/* TYPE DECLARATIONS
 */
typedef struct
{
    /* key */
    UI32_T  portsec_port_index;

    UI32_T  portsec_port_status;

    UI32_T  portsec_port_mac_count;

	UI32_T  portsec_learning_status;

}PSEC_MGR_PortSecurityEntry_T;

/*  for SNMP use
 */
typedef struct
{
    /* key */
    UI32_T port_sec_addr_fdb_id;
    UI8_T  port_sec_addr_address[6];
    UI32_T port_sec_addr_port;
}PSEC_MGR_PortSecAddrEntry_T;

typedef struct
{
    UI16_T      msg_type;       /* 2 bytes */
    UI16_T      vid;            /* 2 bytes */
    UI32_T      port;           /* 4 bytes */
    void        *cookie;
    UI32_T      info;           /* info: reason, max-mac-count */
    UI8_T       mac[6];
} PSEC_TYPE_MSG_T;

enum PSEC_TYPE_MSG_TYPE_E
{
    PSEC_TYPE_MSG_INTRUSION_MAC = 1,
	PSEC_TYPE_MSG_STATION_MOVE,
	PSEC_TYPE_MSG_ENABLED_PSEC,
	PSEC_TYPE_MSG_DISABLED_PSEC,
	PSEC_TYPE_MSG_SET_MAX_COUNT,
};

/* Message declarations for IPC.
 */
typedef union
{
    UI32_T ifindex;
    UI32_T status;
} PSEC_MGR_IPCMsg_GetStatus_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T status;
} PSEC_MGR_IPCMsg_SetStatus_T;

typedef union
{
    UI32_T ifindex;
    UI32_T mac_count;
} PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T;

typedef struct
{
    UI32_T ifindex;
    UI32_T mac_count;
} PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T;

typedef struct
{
    PSEC_MGR_PortSecurityEntry_T portsec_entry;
} PSEC_MGR_IPCMsg_PortSecurityEntry_T;

typedef struct
{
    UI32_T min_number;
} PSEC_MGR_IPCMsg_GetMinNbrOfMaxMacCount_T;

typedef union
{
    UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T result; /* for response */
} PSEC_MGR_IPCMsg_Type_T;

typedef struct
{
    UI32_T ifindex;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
} PSEC_MGR_IPCMsg_GetMac_T;

typedef struct
{
    UI32_T ifindex;
} PSEC_MGR_IPCMsg_Interface_T;

typedef union
{
    PSEC_MGR_IPCMsg_GetStatus_T               get_status_data;
    PSEC_MGR_IPCMsg_SetStatus_T               set_status_data;
    PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T get_port_security_mac_count_data;
    PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T set_port_security_mac_count_data;
    PSEC_MGR_IPCMsg_PortSecurityEntry_T       port_security_entry_data;
    PSEC_MGR_IPCMsg_GetMinNbrOfMaxMacCount_T  get_min_nbr_of_max_mac_count_data;
    PSEC_MGR_IPCMsg_GetMac_T                  get_mac;
    PSEC_MGR_IPCMsg_Interface_T               interface_data;
} PSEC_MGR_IPCMsg_Data_T;

typedef struct
{
    PSEC_MGR_IPCMsg_Type_T type;
    PSEC_MGR_IPCMsg_Data_T data;
} PSEC_MGR_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void PSEC_MGR_Init (void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_EnterTransitionMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_EnterMasterMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_EnterSlaveMode(void);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status
 * INPUT    : ifindex : the logical port
 * OUTPUT	: port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE		: None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityStatus( UI32_T ifindex, UI32_T  *port_security_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityActionActive
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action active state
 * INPUT    : ifindex : the logical port
 * OUTPUT   : action_active
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityActionActive( UI32_T ifindex, UI32_T  *action_active_p);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityActionActive
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action active state
 * INPUT    : ifindex   : the logical port
 *            action_active: TRUE/FALSE
 * OUTPUT   :
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityActionActive( UI32_T ifindex, UI32_T  action_active);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityLastIntrusionMac
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the last intrusion mac
 * INPUT    : ifindex : the logical port
 * OUTPUT   : mac address
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityLastIntrusionMac( UI32_T ifindex, UI8_T  *mac);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the action status of port security
 * INPUT    : ifindex : the logical port
 * OUTPUT	: action_status
 * RETURN   : TRUE/FALSE
 * NOTE		: none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityActionStatus( UI32_T ifindex, UI32_T  *action_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetRunningPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT	: port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE		: None
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_MGR_GetRunningPortSecurityStatus( UI32_T ifindex,
                                                                  UI32_T  *port_security_status);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetRunningPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action status
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT	: action_status
 * RETURN   : TRUE/FALSE
 * NOTE		: none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_MGR_GetRunningPortSecurityActionStatus( UI32_T ifindex,
                                                                        UI32_T  *action_status);
/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityMacCount
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityMacCount(UI32_T ifindex, UI32_T mac_count);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityMacCountOperation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityMacCountOperation(UI32_T ifindex, UI32_T mac_count);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetRunningPortSecurityMacCount
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security max mac count
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT	: mac_count
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_MGR_GetRunningPortSecurityMacCount( UI32_T ifindex,
                                                                    UI32_T  *mac_count);

/*---------------------------------------------------------------------- */
/* ( PortSecurityMgt 1 )--VS2524 */
/*
 *      INDEX       { portSecPortIndex }
 *      portSecPortEntry ::= SEQUENCE
 *      {
 *          portSecPortIndex      INTEGER,
            portSecPortStatus     INTEGER
 *      }
 */
 /*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetNextPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetNextPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityStatus (UI32_T ifindex, UI32_T portsec_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SetPortSecurityStatusOperation
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
BOOL_T PSEC_MGR_SetPortSecurityStatusOperation(UI32_T ifindex, UI32_T portsec_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityStatusOperation_Callback
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
BOOL_T PSEC_MGR_SetPortSecurityStatusOperation_Callback(UI32_T ifindex, UI32_T portsec_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityActionStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security action status
 * INPUT   : ifindex                - interface index
 *           action_status - VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityActionStatus (UI32_T ifindex, UI32_T action_status);

/*---------------------------------------------------------------------- */
/* portSecAddrTable */
/*
 *       INDEX       { portSecAddrFdbId, portSecAddrAddress}
 *       ::= { portSecAddrTable 1 }
 *
 *       PortSecAddrEntry ::= SEQUENCE
 *       {
 *             portSecAddrFdbId       Integer32,
 *             portSecAddrAddress     MacAddress,
 *             portSecAddrPort        INTEGER
 *       }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_GetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->port_sec_addr_fdb_id   -- vid
 *              port_sec_addr_entry->port_sec_addr_address  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   portSecAddrTable
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_GetPortSecAddrEntry (PSEC_MGR_PortSecAddrEntry_T *port_sec_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_GetNextPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified next addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->port_sec_addr_fdb_id   -- vid
 *              port_sec_addr_entry->port_sec_addr_address  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   portSecAddrTable
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_GetNextPortSecAddrEntry (PSEC_MGR_PortSecAddrEntry_T *port_sec_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_SetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to create/update/remove port scure address
 *              entry.
 * INPUT    :   port_sec_addr_entry
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   portSecAddrTable
 *              port_sec_addr_port = 0 => remove the entry
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_SetPortSecAddrEntry (PSEC_MGR_PortSecAddrEntry_T *port_sec_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_TimerExpiry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to check the timer expiry.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
void PSEC_MGR_TimerExpiry(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_TakeIntrusionAction
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to take action for intrusion mac
 * INPUT    :   lport = port no.
 *              mac_p = SA of intrusion mac
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_TakeIntrusionAction(UI32_T lport, UI8_T *mac_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_ProcessIntrusionMac
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to process intrusion mac
 * INPUT    :   msg_type =  PSEC_TYPE_MSG_INTRUSION_MAC
 *              port = port no.
 *              mac[6] = SA of intrusion mac
 *              reason = L2MUX_MGR_RECV_REASON_INTRUDER
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_ProcessIntrusionMac(PSEC_TYPE_MSG_T *msg);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_ProcessStationMove
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to process station move
 * INPUT    :   msg_type =  PSEC_TYPE_MSG_STATION_MOVE
 *              port = port no.
 *              mac[6] = SA of intrusion mac
 *              reason = L2MUX_MGR_RECV_REASON_STATION_MOVE
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_ProcessStationMove(PSEC_TYPE_MSG_T *msg);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetOperationMode
 *------------------------------------------------------------------------
 * FUNCTION: to get the operation mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : SYS_TYPE_STACKING_TRANSITION_MODE | SYS_TYPE_STACKING_MASTER_MODE |
 *	         SYS_TYPE_SYSTEM_STATE_SLAVE
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T PSEC_MGR_GetOperationMode(void) ;

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_SetTransitionMode (void);



/* FUNCTION NAME - PSEC_MGR_HandleHotInsertion
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
void PSEC_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);



/* FUNCTION NAME - PSEC_MGR_HandleHotRemoval
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
void PSEC_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

/* FUNCTION NAME: PSEC_MGR_NoShutdown_CallBack
 * PURPOSE: Notification port admin up with a port argument.
 * INPUT:   port
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void PSEC_MGR_NoShutdown_CallBack (UI32_T ifindex);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security learning status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityLearningStatus (UI32_T ifindex, UI32_T learning_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Get the port security learning status
 * INPUT   : ifindex                - interface index
 * OUTPUT  : learning_status        - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityLearningStatus (UI32_T ifindex, UI32_T *learning_status);

/* Function - PSEC_MGR_GetPortSecurityMacCount
 * Purpose  - This function will get port auto learning mac counts
 * Input    - ifindex        -- which port to
 * Output   - mac_count      -- mac learning count
 * Return  : TRUE: Successfully, FALSE: If not available
 * Note     -
 */
BOOL_T PSEC_MGR_GetPortSecurityMacCount( UI32_T ifindex, UI32_T * mac_count );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_MGR_GetMinNbrOfMaxMacCount
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that max-mac-count can be set to.
 * INPUT    : none
 * OUTPUT   : min_number
 * RETURN   : none
 * NOTES    : for 3com CLI & WEB
 * ---------------------------------------------------------------------
 */
void PSEC_MGR_GetMinNbrOfMaxMacCount(UI32_T *min_number);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_ConvertSecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
PSEC_MGR_ConvertSecuredAddressIntoManual(
    UI32_T ifindex
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for psec_mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);


#endif /* PSEC_MGR_H */
