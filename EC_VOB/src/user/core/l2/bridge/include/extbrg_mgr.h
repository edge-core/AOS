/* Package Name: EXTBRG_MGR.H
 * Purpose: This package provides the services to support the base information of bridge devices.
 * Notes: 1. This package provides the services to comply the RFC1493 BRIDGE MIB and
 *           RFC 2674 EXTENDED BRIDGE MIB.
 *        2. This package is reusable package, and shall be portable to any platform.
 */

#ifndef  EXTBRG_MGR_H
#define  EXTBRG_MGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sysfun.h"


/* NAMING CONSTANT DECLARATIONS
 */

#define EXTBRG_MGR_IPCMSG_TYPE_SIZE sizeof(union EXTBRG_MGR_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
	EXTBRG_MGR_IPC_GETDOT1DDEVICECAPABILITIES,
	EXTBRG_MGR_IPC_GETDOT1DPORTCAPABILITIES,
	EXTBRG_MGR_IPC_GETNEXTDOT1DPORTCAPABILITIES,
	EXTBRG_MGR_IPC_GETDOT1DGMRPSTATUS,
	EXTBRG_MGR_IPC_SETDOT1DGMRPSTATUS,
	EXTBRG_MGR_IPC_GETDOT1DTRAFFICCLASSESENABLED,
	EXTBRG_MGR_IPC_SETDOT1DTRAFFICCLASSESENABLED
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in XSTP_MGR_IpcMsg_T.data
 */
#define EXTBRG_MGR_GET_MSG_SIZE(field_name)                       \
            (EXTBRG_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((EXTBRG_MGR_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
	union EXTBRG_MGR_IpcMsg_Type_U
	{
		UI32_T cmd;
		BOOL_T ret_bool;
		UI32_T ret_ui32;
	} type; /* the intended action or return value */

	union
	{
        UI32_T arg_ui32;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
        } arg_grp_ui32_ui32;
	} data; /* the argument(s) for the function corresponding to cmd */
} EXTBRG_MGR_IpcMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This procedure initiates the system resource required for this package.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This procedure initiates the data based and enables the IEEE802.1D
 *            1998 priority queueing managemenet capability.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This procedure initiates the data based and enables the IEEE802.1D
 *            1998 priority queueing managemenet capability.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This procedure initiates the data based and enables the IEEE802.1D
 *            1998 priority queueing managemenet capability.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T  EXTBRG_MGR_GetOperationMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetDot1dDeviceCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the extended capability of this bridge device.
 * Input:   None.
 * Output:  bridge_device_capability - capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_device_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_GetDot1dDeviceCapabilities(UI32_T *bridge_device_capability);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetFirstDot1dPortCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This function returns the extended capability of first available bridge port.
 * Input:   lport_ifindex.
 * Output:  lport_ifindex.
 *          bridge_port_capability - capabilities defined as a bit map
 * Return: TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_port_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_GetFirstDot1dPortCapabilities(UI32_T *lport_ifindex, UI32_T *bridge_port_capability);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetDot1dPortCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This function returns the extended capability of a given bridge port.
 * Input:   lport_ifindex.
 * Output:  bridge_port_capability - capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_port_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_GetDot1dPortCapabilities(UI32_T lport_ifindex, UI32_T *bridge_port_capability);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetNextDot1dPortCapabilities
 *--------------------------------------------------------------------------
 * Purpose: This function returns the extended capability of next available bridge port.
 * Input:   lport_ifindex.
 * Output:  lport_ifindex.
 *          bridge_port_capability      -- capabilities defined as a bit map
 * Return:  TRUE/FALSE.
 * Note:    1. Please refer to the naming constant declarations for detailed bit map
 *             definition of bridge_port_capability.
 *          2. When bit is set to '1', the corresponding capability is provided.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_GetNextDot1dPortCapabilities(UI32_T *linear_port_id, UI32_T *bridge_port_capability);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetDot1dGmrpStatus
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the GMRP operation status of bridge device.
 * Input:   None.
 * Output:  gmrp_status - VAL_dot1dGmrpStatus_enabled \
 *                        VAL_dot1dGmrpStatus_disabled
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of gmrp_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_GetDot1dGmrpStatus(UI32_T *gmrp_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_SetDot1dGmrpStatus
 *--------------------------------------------------------------------------
 * Purpose: This procedure set the GMRP operation status to a given value.
 * Input:   gmrp_status - VAL_dot1dGmrpStatus_enabled \
 *                        VAL_dot1dGmrpStatus_disabled
 * Output:  None.
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of gmrp_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_SetDot1dGmrpStatus(UI32_T gmrp_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetDot1dTrafficClassesEnabled
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns the traffic classes operation status of bridge device.
 * Input: None.
 * Output: traffic_class_status  - VAL_dot1dTrafficClassesEnabled_true \
 *                                 VAL_dot1dTrafficClassesEnabled_false
 * Return: TRUE/FALSE.
 * Note: Please refer to the naming constant declarations for definition of traffic_class_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_GetDot1dTrafficClassesEnabled(UI32_T *traffic_class_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_SetDot1dTrafficClassesEnabled
 *--------------------------------------------------------------------------
 * Purpose: This procedure enables the traffic class operation on the bridge.
 * Input:   traffic_class_status - VAL_dot1dTrafficClassesEnabled_true \
 *                                 VAL_dot1dTrafficClassesEnabled_false
 * Output:  None.
 * Return:  TRUE/FALSE.
 * Note:    Please refer to the naming constant declarations for definition of traffic_class_status.
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_SetDot1dTrafficClassesEnabled(UI32_T traffic_class_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_GetBridgeVlanLearningMode
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns true if vlan learning mode of the device can be return
 *          successfully.  Otherwise, return FALSE.
 * Input:   None.
 * Output:  vlan_learning_mode - VAL_dot1qConstraintType_independent \
 *                               VAL_dot1qConstraintType_shared
 * Return:  TRUE/FALSE.
 * Note:    None
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_GetBridgeVlanLearningMode(UI32_T *vlan_learning_mode);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_SetBridgeVlanLearningMode
 *--------------------------------------------------------------------------
 * Purpose: This procedure returns true if vlan learning mode of the device can be set
 *          successfully.  Otherwise, return FALSE.
 * Input:   vlan_learning_mode - VAL_dot1qConstraintType_independent \
 *                               VAL_dot1qConstraintType_shared
 * Output:  None.
 * Return:  TRUE/FALSE.
 * Note:    None
 *--------------------------------------------------------------------------
 */
BOOL_T  EXTBRG_MGR_SetBridgeVlanLearningMode(UI32_T vlan_learning_mode);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: EXTBRG_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for bridge extension mgr.
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
BOOL_T EXTBRG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


#endif /* #ifndef EXTBRG_MGR_H */
