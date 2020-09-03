/* Module Name: netcfg_mgr_main.h
 * Purpose:
 *      NETCFG_MGR_IP provides NETCFG misc. configuration management access-point for
 *      upper layer.
 *
 * Notes:
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       01/29/2008 --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */
 
#ifndef NETCFG_MGR_MAIN_H
#define NETCFG_MGR_MAIN_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "netcfg_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETCFG_MGR_MAIN_MSGBUF_TYPE_SIZE sizeof(union NETCFG_MGR_MAIN_IPCMsg_Type_U)

/* Macro function for calculation of ipc msg_buf size based on structure name
 */
#define NETCFG_MGR_MAIN_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_MAIN_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_MAIN_IPCMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */
/* definitions of command in CSCA which will be used in ipc message
 */
enum
{
    NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTINSERTION,
    NETCFG_MGR_MAIN_IPCCMD_HANDLEHOTREMOVAL,
    NETCFG_MGR_MAIN_IPCCMD_ISEMBEDDEDUDPPORT,
    NETCFG_MGR_MAIN_IPCCMD_ISEMBEDDEDTCPPORT
};

/* structure for the request/response ipc message in csca pmgr and mgr
 */
typedef struct
{
    union NETCFG_MGR_MAIN_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/ 
        UI32_T result_i32;  /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type;

    union
    {
        UI32_T  ui32_v;
        
        struct 
        {
            UI32_T starting_port_ifindex;
            UI32_T number_of_port;
            BOOL_T use_default;
        } hot_inerstion_handle; /* the structure which is used when cmd==CSCA_MGR_IPC_CMD1 */

        struct
        {    
            UI32_T u32_a1;
            UI32_T u32_a2;
        } u32a1_u32a2; 
        
    } data; /* contains the supplemntal data for the corresponding cmd */
} NETCFG_MGR_MAIN_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_MGR_MAIN_SetTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_MAIN_SetTransitionMode(void);


/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_MAIN_EnterTransitionMode (void);


/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterMasterMode
 * PURPOSE:
 *      Enter master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void NETCFG_MGR_MAIN_EnterMasterMode (void);


/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterSlaveMode
 * PURPOSE:
 *      Enter slave mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_MAIN_EnterSlaveMode (void);


/* FUNCTION NAME : NETCFG_MGR_MAIN_EnterMasterMode
 * PURPOSE:
 *      Let default gateway CFGDB into route when provision complete.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void NETCFG_MGR_MAIN_ProvisionComplete(void);


/* FUNCTION NAME : NETCFG_MGR_MAIN_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void NETCFG_MGR_MAIN_Create_InterCSC_Relation(void);


/*---------------------------------------
 *  Initialization
 *---------------------------------------
 */
/* FUNCTION NAME : NETCFG_MGR_MAIN_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_MAIN used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_MAIN_InitiateProcessResources(void);


/* FUNCTION NAME : NETCFG_MGR_MAIN_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_MGR_MAIN.
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
 *
 */
BOOL_T NETCFG_MGR_MAIN_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif /* NETCFG_MGR_IP_H */

