/* Module Name: PING_MGR.H
 * Purpose:
 *      This component implements the standard Ping-MIB functionality (rfc2925).
 *      The ping task will sends out ICMP ECHO request and waits for ICMP ECHO reply.
 *      Both Send and receive operation are done through socket.
 *
 * Notes:
 *      1. This design references the TRACEROUTE module.
 *      2. Currently support IPv4 address only.
 *      3. The CLI_API_Ping command also been modified.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/3      --  peter_yu    Create to support partial standard Ping-MIB.
 *      2007/12     --  peter_yu    Porting to linux platform.
 *                                  (1) IP address format is changed to UI8_T[SYS_ADPT_IPV4_ADDR_LEN].
 *                                  (2) Handle IPC Message.
 * Copyright(C)      Accton Corporation, 2007
 */


#ifndef _PING_H
#define _PING_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "ping_type.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PING_MGR_RCV_BUF_SIZE                   1500 //SYS_BLD_MAX_PING_OUT_BACK_SIZE

#define PING_MGR_MSGBUF_TYPE_SIZE sizeof(union PING_MGR_IPCMsg_Type_U)

/* command used in IPC message
 */
enum
{
    // PING_MGR_IPC_GETFORCEVERSION = 1,
    PING_MGR_IPCCMD_SETCTLENTRY = 1,
    PING_MGR_IPCCMD_SETCTLADMINSTATUS,
    PING_MGR_IPCCMD_SETCTLTARGETADDRESS,
    PING_MGR_IPCCMD_SETCTLDATASIZE,
    PING_MGR_IPCCMD_SETCTLPROBECOUNT,
    PING_MGR_IPCCMD_SETCTLROWSTATUS,
    PING_MGR_IPCCMD_SETCTLENTRYBYFIELD,
};

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in PING_MGR_IpcMsg_T.data
 */
#define PING_MGR_GET_MSG_SIZE(field_name)                       \
            (PING_MGR_MSGBUF_TYPE_SIZE +                        \
             sizeof(((PING_MGR_IPCMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
/* IPC message structure
 */
typedef struct
{
	union PING_MGR_IPCMsg_Type_U
	{
		UI32_T cmd;
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/ 
        UI32_T result_i32;   /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
	} type; /* the intended action or return value */

	union
	{
	    PING_TYPE_PingCtlEntry_T          ctrl_entry;
	    PING_TYPE_PingProbeHistoryEntry_T probe_history_entry;
	    PING_TYPE_PingResultsEntry_T      results_entry;

	    struct
	    {
	        PING_TYPE_PingCtlEntry_T ctrl_entry;
	        UI32_T                   ui32;
	    } ctrl_entry_ui32;
	    struct
	    {
	        PING_TYPE_PingCtlEntry_T ctrl_entry;
	        //UI32_T                   ui32;
	        L_INET_AddrIp_T           addr;
	    } ctrl_entry_addr;
	    struct
	    {
	        PING_TYPE_PingCtlEntry_T ctrl_entry;
            PING_TYPE_CtlEntryField_T field; 
	    } ctrl_entry_field;


	} data; /* the argument(s) for the function corresponding to cmd */
} PING_MGR_IPCMsg_T;

BOOL_T ping_backdoor_debug;   /* put here for share with ping_mgr.c & ping_om.c */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : PING_MGR_Initiate_System_Resources
 * PURPOSE:
 *      Initialize working space of ping utility.
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
 *      1. working space is used to limit amount of ping service could be supported.
 *      2. The max simutanious ping requests is defined in SYS_BLD.h.
 */
void PING_MGR_Initiate_System_Resources(void);

/* FUNCTION NAME : PING_MGR_EnterMasterMode
 * PURPOSE:
 *      Ping enters master mode.
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
void PING_MGR_EnterMasterMode (void);

/* FUNCTION NAME : PING_MGR_EnterSlaveMode
 * PURPOSE:
 *      Ping enters Slave mode mode.
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
void PING_MGR_EnterSlaveMode (void);

/* FUNCTION NAME : PING_MGR_EnterTransitionMode
 * PURPOSE:
 *      Ping enters Transition mode mode.
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
 */
void PING_MGR_EnterTransitionMode (void);

/* FUNCTION NAME : PING_MGR_SetTransitionMode
 * PURPOSE:
 *      ping enters set transition mode.  Clear OM
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
void PING_MGR_SetTransitionMode(void);

/* FUNCTION NAME - PING_MGR_GetOperationMode
 * PURPOSE  :
 *      This functions returns the current operation mode of this component
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
 *      None. */
SYS_TYPE_Stacking_Mode_T  PING_MGR_GetOperationMode(void);

/* FUNCTION NAME : PING_MGR_CreateSocket
 * PURPOSE:
 *      Create socket for all workspace.
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
 *      None.
 */
UI32_T PING_MGR_CreateSocket(void);

/* FUNCTION NAME : PING_MGR_CloseSocket
 * PURPOSE:
 *      Close socket for all workspace.
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
 *      None.
 */
UI32_T PING_MGR_CloseSocket(void);

/* FUNCTION NAME: PING_MGR_TriggerPing
 * PURPOSE:
 *          To scan any active workspace, then send/wait pkts.
 * INPUT:
 *          None
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 */
UI32_T  PING_MGR_TriggerPing(void);

/* FUNCTION NAME: PING_MGR_SetCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 *          3. key: ping_ctl_owner_index, ping_ctl_test_name.
 */
UI32_T PING_MGR_SetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);

/* FUNCTION NAME: PING_MGR_SetCtlAdminStatus
 * PURPOSE:
 *          To enable or disable ping control entry
 * INPUT:
 *          ctl_entry_p         -- the specific control entry.
 *          ctrl_admin_status   -- the admin status of the to enable or disable the ping.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1.This API is only used by "set by filed", not "set by record".
 */
UI32_T PING_MGR_SetCtlAdminStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T ctrl_admin_status);

/* FUNCTION NAME: PING_MGR_SetCtlRowStatus
 * PURPOSE:
 *          To set row status field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the  specific control entry.
 *          ctrl_row_status -- the row status of the specified control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. The PingCtlEntry should not be modified. So we create a local entry for local use.
 */

UI32_T PING_MGR_SetCtlRowStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T ctrl_row_status);

/* FUNCTION NAME: PING_MGR_SetCtlTargetAddress
 * PURPOSE:
 *          To set the target address field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          taget_addr      -- the target address of the remote host.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the target address for the specified index when admin_status is enabled.
 *          2. Currently we do not support the domain name query of the target address.
 */
//UI32_T PING_MGR_SetCtlTargetAddress(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T taget_addr);
UI32_T PING_MGR_SetCtlTargetAddress(PING_TYPE_PingCtlEntry_T *ctl_entry_p, L_INET_AddrIp_T* target_addr_p);

/* FUNCTION NAME: PING_MGR_SetCtlDataSize
 * PURPOSE:
 *          To set the data size field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          data_size       -- the size of data portion in ICMP pkt.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the data size for the specified index when admin_status is enabled.
 *          2. SNMP range: 0..65507, CLI range: 32-512.
 */
UI32_T PING_MGR_SetCtlDataSize(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T data_size);

/* FUNCTION NAME: PING_MGR_SetCtlProbeCount
 * PURPOSE:
 *          To set the probe count field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          probe_count     -- the number of ping packet
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the probe count for the specified index when admin_status is enabled.
 *          2. SNMP range: 1-15, CLI range: 1-16. So 0 is not allowed.
 */
UI32_T PING_MGR_SetCtlProbeCount(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T probe_count);

/* FUNCTION NAME : PING_MGR_CreateWorkSpace
 * PURPOSE:
 *      Create workspace for routing path from src to dst.
 *
 * INPUT:
 *      dst_ip          -- probe target IP address.
 *      src_ip          -- interface IP address which send out probe packet.
 *      workspace_index -- location of this entry
 * OUTPUT:
 *      None.
 * RETURN:
 *      PING_TYPE_OK  -- successfully create the workspace.
 *      PING_TYPE_INVALID_ARG -- src_ip or dst_ip is invalid value.
 *      PING_TYPE_NO_MORE_WORKSPACE
 *      PING_TYPE_NO_MORE_SOCKET
 *      PING_TYPE_NO_MORE_ENTRY
 *      PING_TYPE_FAIL
 * NOTES:
 *      1. If same task create workspace twice, cause previous one auto-free.
 */
UI32_T PING_MGR_CreateWorkSpace(UI32_T workspace_index , PING_TYPE_PingCtlEntry_T *ctl_entry_p);

/* FUNCTION NAME: PING_MGR_SetCtlEntryByField
 * PURPOSE:
 *          Set only the field of the entry.
 * INPUT:
 *          ctl_entry_p -- the pointer of the specified ctl entry.
 *          field       -- field.
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 *          PING_TYPE_INVALID_ARG
 * NOTES:
 *          1. set only the field of the entry.
 */
UI32_T PING_MGR_SetCtlEntryByField(PING_TYPE_PingCtlEntry_T *ctl_entry_p, PING_TYPE_CtlEntryField_T field);

/* FUNCTION NAME : PING_MGR_FreeWorkSpace
 * PURPOSE:
 *      Release working space to ping utility.
 *
 * INPUT:
 *      workspace_index -- the starting address of workspace handler.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      PING_TYPE_OK - the space is return to utility.
 *      PING_TYPE_INVALID_WORK_SPACE - the pointer is no valid pointer of working space,
 *                                     maybe not owned by this task.
 *
 * NOTES:
 *      1. After free workspace, handler will set to NULL.
 */
UI32_T PING_MGR_FreeWorkSpace(UI32_T workspace_index);

/* FUNCTION NAME: PING_MGR_SetWorkSpaceAdminStatus
 * PURPOSE:
 *          Sync admin status in the work space.
 * INPUT:
 *          table_index -- table index to be synchronize.
 *          status      -- {VAL_pingCtlAdminStatus_disabled|VAL_pingCtlAdminStatus_enabled}.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *
 */
UI32_T PING_MGR_SetWorkSpaceAdminStatus(UI32_T table_index, UI32_T status);

/* FUNCTION NAME : PING_MGR_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for PING_MGR.
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
BOOL_T PING_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif

