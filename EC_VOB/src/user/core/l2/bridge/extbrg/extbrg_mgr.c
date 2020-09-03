/* Package Name: EXTBRG_MGR.C
 * Purpose: This package provides the services to support the base information of bridge devices.
 * Notes: 1. This package provides the services to comply the RFC1493 BRIDGE MIB and
 *           RFC2674 EXTENDED BRIDGE MIB.
 *        2. This package is reusable package, and shall be portable to any platform.
 *
 * History: 11-05-2001      Amytu       Modify to fit into Mercury
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "leaf_2674p.h"
#include "leaf_2674q.h"
#include "leaf_sys.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "amtr_pmgr.h"
#include "extbrg_mgr.h"
#include "swctrl.h"


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
   /* Configure the GMRP operation of bridge device.
    */
   UI32_T    gmrp_status;

   /* Configure the Traffic Classes operation of bridge device.
    */
   UI32_T    traffic_class_operation_status;

   /*UI8_T    padding;       *//* EE section size must be even number */
   UI32_T    ivl_svl;       /* 1: IVL, 2: SVL */
} EXTBRG_MGR_802_1P_1Q_Global_Config_Data_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void EXTBRG_MGR_FactoryReset(void);


/* STATIC VARIABLE DECLARATIONS
 */

/* Allen Cheng: Deleted
static UI32_T  ext_bridge_operation_mode;
*/
static EXTBRG_MGR_802_1P_1Q_Global_Config_Data_T ext_bridge_global_cfg_data;
SYSFUN_DECLARE_CSC


/* EXPORTED SUBPROGRAM BODIES
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
void  EXTBRG_MGR_Initiate_System_Resources(void)
{
    return;
} /* End of EXTBRG_MGR_Initiate_System_Resources() */

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
void  EXTBRG_MGR_Create_InterCSC_Relation(void)
{
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This procedure initiates the data based and enables the IEEE802.1D
 *            1998 priority queueing managemenet capability.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : following Lewis modified for EE DB conversion 5/17/00
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_EnterMasterMode(void)
{
    EXTBRG_MGR_FactoryReset();   /* amy add */
    SYSFUN_ENTER_MASTER_MODE();
    return;
} /* End of EXTBRG_MGR_EnterMasterMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This procedure initiates the data based and enables the IEEE802.1D
 *            1998 priority queueing managemenet capability.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : following Lewis modified for EE DB conversion 5/17/00
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    return;
} /* End of EXTBRG_MGR_EnterTransitionMode() */

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
void  EXTBRG_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
} /* end of EXTBRG_MGR_SetTransitionMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - EXTBRG_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This procedure initiates the data based and enables the IEEE802.1D
 *            1998 priority queueing managemenet capability.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : following Lewis modified for EE DB conversion 5/17/00
 *--------------------------------------------------------------------------
 */
void  EXTBRG_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return;
} /* End of EXTBRG_MGR_EnterSlaveMode() */

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
SYS_TYPE_Stacking_Mode_T  EXTBRG_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
} /* end of EXTBRG_MGR_GetOperationMode() */

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
BOOL_T  EXTBRG_MGR_GetDot1dDeviceCapabilities(UI32_T *bridge_device_capability)
{
   /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (ext_bridge_global_cfg_data.ivl_svl == VAL_dot1qConstraintType_independent)
    {
        *bridge_device_capability = (SYS_VAL_dot1dDeviceCapabilities_dot1dTrafficClasses              |
                                     SYS_VAL_dot1dDeviceCapabilities_dot1qStaticEntryIndividualPort   |
                                     SYS_VAL_dot1dDeviceCapabilities_dot1qIVLCapable                  |
                                     SYS_VAL_dot1dDeviceCapabilities_dot1qConfigurablePvidTagging);
    }
    else
    {
        *bridge_device_capability = (SYS_VAL_dot1dDeviceCapabilities_dot1dTrafficClasses              |
                                   SYS_VAL_dot1dDeviceCapabilities_dot1qStaticEntryIndividualPort   |
                                   SYS_VAL_dot1dDeviceCapabilities_dot1qSVLCapable                  |
                                   SYS_VAL_dot1dDeviceCapabilities_dot1qConfigurablePvidTagging);
    }

    return TRUE;
} /* End of EXTBRG_MGR_GetDot1dDeviceCapabilities() */

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
BOOL_T  EXTBRG_MGR_GetFirstDot1dPortCapabilities(UI32_T *lport_ifindex, UI32_T *bridge_port_capability)
{
   /* BODY */

   *lport_ifindex = 0;
   return EXTBRG_MGR_GetNextDot1dPortCapabilities (lport_ifindex, bridge_port_capability);
} /* End of EXTBRG_MGR_GetFirstDot1dPortCapabilities() */

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
BOOL_T  EXTBRG_MGR_GetDot1dPortCapabilities(UI32_T lport_ifindex, UI32_T *bridge_port_capability)
{
    /* LOCAL VARIABLE DECLARATIONS */
    UI32_T      unit, port, trunk_id;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (SWCTRL_LogicalPortToUserPort(lport_ifindex,&unit, &port, &trunk_id))


    {
        *bridge_port_capability = SYS_ADPT_BRIDGE_PORT_CAPABILITIES;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} /* End of EXTBRG_MGR_GetDot1dPortCapabilities() */

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
BOOL_T  EXTBRG_MGR_GetNextDot1dPortCapabilities(UI32_T *lport_ifindex, UI32_T *bridge_port_capability)
{
   /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (*lport_ifindex < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
    {
        *lport_ifindex = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER;
    }
    else
    {
        if (SWCTRL_GetNextLogicalPort(lport_ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
            return FALSE;
    }

    return EXTBRG_MGR_GetDot1dPortCapabilities(*lport_ifindex, bridge_port_capability);
} /* End of EXTBRG_MGR_GetNextDot1dPortCapabilities() */

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
BOOL_T  EXTBRG_MGR_GetDot1dGmrpStatus(UI32_T *gmrp_status)
{
   /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    *gmrp_status = ext_bridge_global_cfg_data.gmrp_status;

    return TRUE;
} /* End of EXTBRG_MGR_GetDot1dGmrpStatus() */

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
BOOL_T  EXTBRG_MGR_SetDot1dGmrpStatus(UI32_T gmrp_status)
{
   /* BODY */

   /* Not support!! */
   return FALSE;
} /* End of EXTBRG_MGR_SetDot1dGmrpStatus() */

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
BOOL_T  EXTBRG_MGR_GetDot1dTrafficClassesEnabled(UI32_T *traffic_class_status)
{
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    *traffic_class_status = ext_bridge_global_cfg_data.traffic_class_operation_status;

    return TRUE;
} /* End of EXTBRG_MGR_GetDot1dTrafficClassesEnabled() */

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
BOOL_T  EXTBRG_MGR_SetDot1dTrafficClassesEnabled(UI32_T traffic_class_status)
{
   /* BODY */

   /* Not support!! */
   return FALSE;
} /* End of EXTBRG_MGR_SetDot1dTrafficClassesEnabled() */

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
BOOL_T  EXTBRG_MGR_GetBridgeVlanLearningMode(UI32_T *vlan_learning_mode)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (ext_bridge_global_cfg_data.ivl_svl != VAL_dot1qConstraintType_independent &&
       ext_bridge_global_cfg_data.ivl_svl != VAL_dot1qConstraintType_shared)
    {
        return FALSE;
    } /* End of if */

    *vlan_learning_mode = ext_bridge_global_cfg_data.ivl_svl;
    return TRUE;
} /* end of EXTBRG_MGR_GetBridgeVlanLearningMode() */

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
BOOL_T  EXTBRG_MGR_SetBridgeVlanLearningMode(UI32_T vlan_learning_mode)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    } /* End of if */

    if (ext_bridge_global_cfg_data.ivl_svl != VAL_dot1qConstraintType_independent &&
       ext_bridge_global_cfg_data.ivl_svl != VAL_dot1qConstraintType_shared)
    {
        return FALSE;
    } /* End of if */

    if (ext_bridge_global_cfg_data.ivl_svl == vlan_learning_mode)
    {
        return TRUE;
    } /* End of if */

    ext_bridge_global_cfg_data.ivl_svl = vlan_learning_mode;

    if (vlan_learning_mode == VAL_dot1qConstraintType_independent)
        AMTR_PMGR_SetLearningMode(VAL_dot1qConstraintType_independent);
    else
        AMTR_PMGR_SetLearningMode(VAL_dot1qConstraintType_shared);

    return TRUE;
} /* end of EXTBRG_MGR_SetBridgeVlanLearningMode() */

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
BOOL_T EXTBRG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    EXTBRG_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (EXTBRG_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = EXTBRG_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding EXTBRG_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case EXTBRG_MGR_IPC_GETDOT1DDEVICECAPABILITIES:
        	msg_p->type.ret_bool =
                EXTBRG_MGR_GetDot1dDeviceCapabilities(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case EXTBRG_MGR_IPC_GETDOT1DPORTCAPABILITIES:
        	msg_p->type.ret_bool = EXTBRG_MGR_GetDot1dPortCapabilities(
        	    msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
        	    &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case EXTBRG_MGR_IPC_GETNEXTDOT1DPORTCAPABILITIES:
        	msg_p->type.ret_bool = EXTBRG_MGR_GetNextDot1dPortCapabilities(
        	    &msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
        	    &msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_grp_ui32_ui32);
            break;

        case EXTBRG_MGR_IPC_GETDOT1DGMRPSTATUS:
        	msg_p->type.ret_bool =
                EXTBRG_MGR_GetDot1dGmrpStatus(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case EXTBRG_MGR_IPC_SETDOT1DGMRPSTATUS:
        	msg_p->type.ret_bool =
                EXTBRG_MGR_SetDot1dGmrpStatus(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = EXTBRG_MGR_IPCMSG_TYPE_SIZE;
            break;

        case EXTBRG_MGR_IPC_GETDOT1DTRAFFICCLASSESENABLED:
        	msg_p->type.ret_bool =
                EXTBRG_MGR_GetDot1dTrafficClassesEnabled(&msg_p->data.arg_ui32);
            msgbuf_p->msg_size = EXTBRG_MGR_GET_MSG_SIZE(arg_ui32);
            break;

        case EXTBRG_MGR_IPC_SETDOT1DTRAFFICCLASSESENABLED:
        	msg_p->type.ret_bool =
                EXTBRG_MGR_SetDot1dTrafficClassesEnabled(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = EXTBRG_MGR_IPCMSG_TYPE_SIZE;
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msgbuf_p->msg_size = EXTBRG_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of EXTBRG_MGR_HandleIPCReqMsg */


/* LOCAL SUBPROGRAM BODIES
 */

/* Purpose: This procedure resets the IEEE802.1P priority configuration to
 *          factory default value.
 * Parameters:
 *    Input: None
 *    Output: None
 * Return   : None
 */
static void EXTBRG_MGR_FactoryReset(void)
{
   /* BODY */

   /* Set port_default_user_priority_table to factory default value.
    */
   ext_bridge_global_cfg_data.gmrp_status = SYS_DFLT_1D_GMRP_STATUS;

   ext_bridge_global_cfg_data.traffic_class_operation_status = SYS_DFLT_1P_TRAFFIC_CLASSES_ENABLED;

   /* Lewis */
   ext_bridge_global_cfg_data.ivl_svl = SYS_DFLT_1Q_CONSTRAIN_TYPE;
   AMTR_PMGR_SetLearningMode(SYS_DFLT_1Q_CONSTRAIN_TYPE);
}/* End of EXTBRG_MGR_FactoryReset() */


/* End of EXTBRG_MGR.C */
