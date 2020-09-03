/* MODULE NAME: cmgr.h
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*     This file used to be the interface to all CSC.
*     When a action need be execused after many CSC execused first,
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - Give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitation}
*    {4. Any performance limitation}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    03/03/2008    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2008
*/

#ifndef _CMGR_H
#define _CMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define CMGR_EVENT_CALLBACK BIT_0

enum
{
    CMGR_IPC_VLAN_CHANGE,
    CMGR_IPC_GVRP_VLAN_CHANGE,
    CMGR_IPC_L3_VLAN_DESTROY,
    CMGR_IPC_VLAN_MEMBER_CHANGE,
    CMGR_IPC_GVRP_VLAN_MEMBER_CHANGE,
    CMGR_IPC_PORT_VLAN_CHANGE,
    CMGR_IPC_PORT_VLAN_MODE_CHANGE,
    CMGR_IPC_PORT_PVLAN_MODE_CHANGE,
    CMGR_IPC_PVID_CHANGE,
    CMGR_IPC_L3_IF_OPER_STATUS_CHANGE,
    CMGR_IPC_VLAN_NAME_CHANGE,
    CMGR_IPC_PROTOCOL_VLAN_CHANGE,
    CMGR_IPC_VLAN_MEMBER_TAG_CHANGE,
    CMGR_IPC_XSTP_PORT_STATE_CHANGE,
    CMGR_IPC_XSTP_VERSION_CHANGE,
    CMGR_IPC_XSTP_PORT_TOPO_CHANGE,
};

/* MACRO FUNCTIONS DECLARACTION
 */
#define CMGR_GET_MSG_SIZE(field_name)               \
    (sizeof(union CMGR_IpcMsg_Type_U) +             \
    sizeof(((CMGR_IpcMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    union CMGR_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
    } type; /* the intended action or return value */

    union
    {
        UI32_T  arg_ui32;

        struct
        {
            UI32_T  arg_ui32_1;
            UI32_T  arg_ui32_2;
        } arg_ui32_ui32;
        struct
        {
            UI32_T  arg_ui32_1;
            UI32_T  arg_ui32_2;
            UI32_T  arg_ui32_3;
        } arg_ui32_ui32_ui32;
        struct
        {
            UI32_T  arg_ui32_1;
            UI32_T  arg_ui32_2;
            BOOL_T  arg_bool_1;
            BOOL_T  arg_bool_2;
        } arg_ui32_ui32_bool_bool;
        struct
        {
            UI32_T  arg_ui32_1;
            UI32_T  arg_ui32_2;
            UI32_T  arg_ui32_3;
            BOOL_T  arg_bool_1;
            BOOL_T  arg_bool_2;
        } arg_ui32_ui32_ui32_bool_bool;
    } data; /* the argument(s) for the function corresponding to cmd */
} CMGR_IpcMsg_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_AttachSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function attach the resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CMGR_AttachSystemResources();
/* -------------------------------------------------------------------------
 * ROUTINE NAME - CMGR_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void CMGR_Create_InterCSC_Relation(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_SetPortAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE : The function set the port admin status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T CMGR_SetPortAdminStatus(UI32_T lport, UI32_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE : The function set the port admin status
 * INPUT   : ifindex        -- which port to set
 *           status         -- TRUE to be up; FALSE to be down
 *           reason         -- indicates role of caller
 *                             bitmap of SWCTRL_PORT_STATUS_SET_BY_XXX
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : CSCs that is in the layer higher than CMGR may call CMGR_SetPortStatus, 
 *           CSCs that is in the layer lower than CMGR should call 
 *           SYS_CALLBACK_MGR_SetPortStatusCallback() to notify CMGR to set port status
 *-------------------------------------------------------------------------
 */
BOOL_T CMGR_SetPortStatus(UI32_T ifindex, BOOL_T status, UI32_T reason);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_SetIpv6AddrAutoconfig
 *-------------------------------------------------------------------------
 * PURPOSE : The function set the ipv6 address autoconfig
 * INPUT   : ifindex        -- which vlan interface to set
 *           status         -- TRUE to be enable; FALSE to be disable
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T CMGR_SetIpv6AddrAutoconfig(UI32_T ifindex, BOOL_T status);

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
void CMGR_NotifyVlanChange(UI32_T *key_list, UI32_T cmd, UI32_T vlan_ifindex,
        UI32_T vlan_status, BOOL_T existed, BOOL_T merged);
void CMGR_NotifyVlanMemberChange(UI32_T *key_list, UI32_T cmd,
        UI32_T vlan_ifindex, UI32_T lport, UI32_T vlan_status, BOOL_T existed,
        BOOL_T merged);
void CMGR_NotifyL3VlanDestroy(UI32_T *key_list, UI32_T vlan_ifindex,
        UI32_T vlan_status);
void CMGR_NotifyPortVlanChange(UI32_T *key_list, UI32_T lport);
void CMGR_NotifyPortVlanModeChange(UI32_T *key_list, UI32_T cmd, UI32_T lport,
        UI32_T mode);
void CMGR_NotifyPvidChange(UI32_T *key_list, UI32_T lport, UI32_T old_pvid,
        UI32_T new_pvid);
void CMGR_NotifyL3IfOperStatusChange(UI32_T *key_list, UI32_T vlan_ifindex,
        UI32_T oper_status);
void CMGR_NotifyVlanNameChange(UI32_T *key_list, UI32_T vid);
void CMGR_NotifyProtocolVlanChange(UI32_T *key_list, UI32_T lport);
void CMGR_NotifyVlanMemberTagChange(UI32_T *key_list, UI32_T vlan_ifindex,
        UI32_T lport);
void CMGR_NotifyXstpPortStateChange(UI32_T *key_list, UI32_T xstid,
        UI32_T lport, BOOL_T xstp_state, BOOL_T xstp_merge);
void CMGR_NotifyXstpVersionChange(UI32_T *key_list, UI32_T mode, UI32_T status);
void CMGR_NotifyXstpPortTopoChange(UI32_T *key_list, UI32_T xstid,
        UI32_T lport);
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

#endif /* #ifndef _CMGR_H */
