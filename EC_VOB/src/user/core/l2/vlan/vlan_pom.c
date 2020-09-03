/*-----------------------------------------------------------------------------
 * FILE NAME: VLAN_POM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for VLAN OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/30     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "l_cvrt.h"
#include "swctrl_pom.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#include "vlan_pom.h"
#include "vlan_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

enum
{
    PVID_FIELD,
    INGRESS_FIELD,
    ACCEPTABLE_FRAME_TYPE_FIELD,
    GVRP_STATUS_FIELD,
    VLAN_PORT_MODE_FIELD,
};


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void VLAN_POM_CompareDefaultVlanPortList(BOOL_T default_value, UI8_T *vlan_list, UI8_T *cfg_list, BOOL_T *changed_status);
static void VLAN_POM_CompareDefaultPortField(UI32_T field_type, UI32_T current_value, UI32_T *cfg_value, BOOL_T *changed_status);


/* STATIC VARIABLE DEFINITIONS
 */


/* EXPORTED SUBPROGRAM BODIES
 */

/* ---------------------------------------------------------------------
 * FUNCTION NAME  - VLAN_POM_GetRunningVlanParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific vlan entry with non-default value associated with each entry
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  vidx - key to identify the specific vlan
 * OUTPUT: vlan_cfg - structure containing changed of status and non-defalut value.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default structure for each field for the device.
 * ----------------------------------------------------------------------------------*/
UI32_T VLAN_POM_GetRunningVlanParameters(UI32_T vid, VLAN_TYPE_Vlan_RunningCfg_T *vlan_cfg)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    VLAN_OM_Vlan_Port_Info_T            vlan_port_info;
    UI32_T                              port_ifindex;
#endif
    UI32_T                              vid_ifindex;
    BOOL_T                              default_value;

    /* BODY */

    if (vid < 1 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (vlan_cfg == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;
    if (VLAN_OM_GetVlanEntry(&vlan_info) == FALSE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_dynamicGvrp)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memset(vlan_cfg, 0, sizeof(*vlan_cfg));
    vlan_cfg->vid_ifindex = vid;

    if (strlen(vlan_info.dot1q_vlan_static_name) != 0)
    {
        strncpy(vlan_cfg->vlan_name, vlan_info.dot1q_vlan_static_name, SYS_ADPT_MAX_VLAN_NAME_LEN);
        vlan_cfg->name_changed = TRUE;
    }

    if (vlan_info.dot1q_vlan_alias[0])
    {
        strncpy(vlan_cfg->vlan_alias, vlan_info.dot1q_vlan_alias, VLAN_TYPE_ALIAS_NAME_LEN);
        vlan_cfg->alias_changed = TRUE;
    }

    if (vid == SYS_DFLT_1Q_PORT_VLAN_PVID)
        default_value = TRUE;
    else
        default_value = FALSE;

    VLAN_POM_CompareDefaultVlanPortList(default_value,
                                        vlan_info.dot1q_vlan_static_egress_ports,
                                        vlan_cfg->vlan_egress_ports,
                                        &vlan_cfg->egress_ports_changed);

    VLAN_POM_CompareDefaultVlanPortList(default_value,
                                        vlan_info.dot1q_vlan_static_untagged_ports,
                                        vlan_cfg->vlan_untagged_ports,
                                        &vlan_cfg->untag_ports_changed);

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    port_ifindex = 0;
    while (SWCTRL_POM_GetNextLogicalPort(&port_ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
    {
        vlan_port_info.lport_ifindex = port_ifindex;
        if (VLAN_OM_GetVlanPortEntry(&vlan_port_info))
        {
            if (    (vlan_port_info.vlan_port_entry.vlan_port_dual_mode)
                 && (vlan_port_info.vlan_port_entry.dual_mode_vlan_id == vid)
               )
            {
                L_CVRT_DEL_MEMBER_FROM_PORTLIST(vlan_cfg->vlan_untagged_ports, port_ifindex);
            }
        }
    }
#endif /* end of #if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)*/

    VLAN_POM_CompareDefaultVlanPortList(FALSE,
                                        vlan_info.dot1q_vlan_forbidden_egress_ports,
                                        vlan_cfg->vlan_forbidden_egress_ports,
                                        &vlan_cfg->forbidden_ports_changed);

    if (vlan_info.dot1q_vlan_static_row_status != VAL_dot1qVlanStaticRowStatus_active)
    {
        vlan_cfg->vlan_row_status = vlan_info.dot1q_vlan_static_row_status;
        vlan_cfg->row_status_changed = TRUE;
    }

    if (vlan_info.vlan_address_method != SYS_DFLT_VLAN_ADDRESS_METHOD)
    {
        vlan_cfg->vlan_address_method = vlan_info.vlan_address_method;
        vlan_cfg->address_method_changed = TRUE;
    }

#if (SYS_CPNT_RSPAN == TRUE)
    if (vlan_info.rspan_status != VAL_vlanStaticExtRspanStatus_vlan)
    {
        vlan_cfg->rspan_status = vlan_info.rspan_status;
        vlan_cfg->rspan_status_changed = TRUE;
    }
#endif

    if (    (default_value == FALSE)
         || (vlan_cfg->name_changed == TRUE)
         || (vlan_cfg->alias_changed == TRUE)
         || (vlan_cfg->egress_ports_changed == TRUE)
         || (vlan_cfg->untag_ports_changed == TRUE)
         || (vlan_cfg->forbidden_ports_changed == TRUE)
         || (vlan_cfg->row_status_changed == TRUE)
         || (vlan_cfg->address_method_changed == TRUE)
#if (SYS_CPNT_RSPAN == TRUE)
         || (vlan_cfg->rspan_status_changed == TRUE)
#endif
       )
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
} /* end of VLAN_POM_GetRunningVlanParameters() */

/* ---------------------------------------------------------------------
 * FUNCTION NAME  - VLAN_POM_GetRunningVlanPortParameters
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific vlan port entry with non-default value associated with each entry
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  lport_ifindex - key to identify the specific vlan port entry
 * OUTPUT: vlan_port_cfg - structure containing changed of status and non-defalut value.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default structure for each field for the device.
 * ----------------------------------------------------------------------------------*/
UI32_T VLAN_POM_GetRunningVlanPortParameters(UI32_T lport_ifindex, VLAN_TYPE_Vlan_Port_RunningCfg_T *vlan_port_cfg)
{
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
    UI32_T                      admin_pvid_ifindex;
    BOOL_T                      private_vlan_changed = FALSE;
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    BOOL_T                      q_trunk_member_changed;
#endif

    /* BODY */

    if (vlan_port_cfg == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    vlan_port_info.lport_ifindex = lport_ifindex;
    if (VLAN_OM_GetVlanPortEntry(&vlan_port_info) == FALSE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    memset(vlan_port_cfg, 0, sizeof(VLAN_TYPE_Vlan_Port_RunningCfg_T));
    vlan_port_cfg->lport_ifindex = lport_ifindex;

    if (vlan_port_info.port_item.auto_vlan_mode)
    {
        VLAN_VID_CONVERTTO_IFINDEX(vlan_port_info.port_item.admin_pvid, admin_pvid_ifindex);
        VLAN_POM_CompareDefaultPortField(PVID_FIELD,
                                         admin_pvid_ifindex,
                                         &vlan_port_cfg->pvid_index,
                                         &vlan_port_cfg->pvid_changed);
    }
    else
    {
        VLAN_POM_CompareDefaultPortField(PVID_FIELD,
                                         vlan_port_info.port_item.dot1q_pvid_index,
                                         &vlan_port_cfg->pvid_index,
                                         &vlan_port_cfg->pvid_changed);
    }

    if (vlan_port_info.port_item.auto_vlan_mode)
    {
        VLAN_POM_CompareDefaultPortField(ACCEPTABLE_FRAME_TYPE_FIELD,
                                         vlan_port_info.port_item.admin_acceptable_frame_types,
                                         &vlan_port_cfg->port_acceptable_frame_types,
                                         &vlan_port_cfg->acceptable_frame_types_changed);
    }
    else
    {
        VLAN_POM_CompareDefaultPortField(ACCEPTABLE_FRAME_TYPE_FIELD,
                                         vlan_port_info.port_item.dot1q_port_acceptable_frame_types,
                                         &vlan_port_cfg->port_acceptable_frame_types,
                                         &vlan_port_cfg->acceptable_frame_types_changed);
    }

    if (vlan_port_info.port_item.auto_vlan_mode)
    {
        VLAN_POM_CompareDefaultPortField(INGRESS_FIELD,
                                         vlan_port_info.port_item.admin_ingress_filtering,
                                         &vlan_port_cfg->port_ingress_filtering,
                                         &vlan_port_cfg->ingress_filtering_changed);
    }
    else
    {
        VLAN_POM_CompareDefaultPortField(INGRESS_FIELD,
                                         vlan_port_info.port_item.dot1q_port_ingress_filtering,
                                         &vlan_port_cfg->port_ingress_filtering,
                                         &vlan_port_cfg->ingress_filtering_changed);
    }

    VLAN_POM_CompareDefaultPortField(GVRP_STATUS_FIELD,
                                     vlan_port_info.port_item.dot1q_port_gvrp_status,
                                     &vlan_port_cfg->port_gvrp_status,
                                     &vlan_port_cfg->port_gvrp_status_changed);

    VLAN_POM_CompareDefaultPortField(VLAN_PORT_MODE_FIELD,
                                     vlan_port_info.vlan_port_entry.vlan_port_mode,
                                     &vlan_port_cfg->vlan_port_mode,
                                     &vlan_port_cfg->vlan_port_mode_changed);

    if (vlan_port_info.port_trunk_mode)
    {
        vlan_port_cfg->port_trunk_mode = vlan_port_info.port_trunk_mode;
        vlan_port_cfg->port_trunk_mode_changed = TRUE;
    } /* end of if */

#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
    if (vlan_port_info.vlan_port_entry.vlan_port_trunk_link_mode != VLAN_MGR_NOT_TRUNK_LINK)
    {
        vlan_port_cfg->port_trunk_link_mode = vlan_port_info.vlan_port_entry.vlan_port_trunk_link_mode;
        vlan_port_cfg->port_trunk_link_mode_changed = TRUE;
        q_trunk_member_changed = TRUE;
    }
#endif

    if (    (vlan_port_cfg->pvid_changed == TRUE)
         || (vlan_port_cfg->acceptable_frame_types_changed == TRUE)
         || (vlan_port_cfg->ingress_filtering_changed == TRUE)
         || (vlan_port_cfg->port_gvrp_status_changed == TRUE)
         || (vlan_port_cfg->vlan_port_mode_changed == TRUE)
         || (vlan_port_cfg->port_trunk_mode_changed == TRUE)
         || (private_vlan_changed == TRUE)
#if (SYS_CPNT_Q_TRUNK_MEMBER == TRUE)
         || (q_trunk_member_changed == TRUE)
#endif
       )
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
} /* end of VLAN_POM_GetRunningVlanPortParameters() */

/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_CompareDefaultVlanPortList
 *--------------------------------------------------------------------------
 * PURPOSE  : This function compares the specific vlan port list against
 *            system defined default vlan port list value.  If current vlan port
 *            list has been modified since the start of the system, then
 *            changed_status is set to TRUE and the modified list is copy to
 *            cfg_list.  Otherwise, changed_status is set to FALSE.
 * INPUT    : vlan_list - the specific vlan port list.
 *            cfg_list  - running config port list to copy to.
 * OUTPUT   : changed_status - modification status of the specific port list.
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static void VLAN_POM_CompareDefaultVlanPortList(BOOL_T default_value, UI8_T *vlan_list, UI8_T *cfg_list, BOOL_T *changed_status)
{
    UI32_T lport;

    /* BODY */
    for (lport = 1; lport <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; lport++)
    {
        if (    (default_value && !L_CVRT_IS_MEMBER_OF_PORTLIST(vlan_list, lport))
             || (!default_value && L_CVRT_IS_MEMBER_OF_PORTLIST(vlan_list, lport))
           )
        {
            memcpy(cfg_list, vlan_list, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
            *changed_status = TRUE;
            return;
        }
    }

    *changed_status = FALSE;
    return;
} /* end of VLAN_POM_CompareDefaultVlanPortList() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_CompareDefaultPortField
 *--------------------------------------------------------------------------
 * PURPOSE  : This function compares the specific field of a designated port against
 *            system defined default value.  If current value of the specific field
 *            has been modified since the start of the system, then changed_status
 *            is set to TRUE and the modified value is copy to
 *            cfg_value.  Otherwise, changed_status is set to FALSE.
 * INPUT    : field_type - the specific field.
 *            current_value - current value of the specific field
 *            cfg_value - value for running config.
 * OUTPUT   : changed_status - modification status of the specific port list.
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static void VLAN_POM_CompareDefaultPortField(UI32_T field_type, UI32_T current_value, UI32_T *cfg_value, BOOL_T *changed_status)
{
    /* BODY */

    *changed_status = FALSE;
    switch (field_type)
    {
        case PVID_FIELD:
            if (current_value == (SYS_DFLT_1Q_PORT_VLAN_PVID+SYS_ADPT_VLAN_1_IF_INDEX_NUMBER-1))
            {
                return;
            }
            break;

        case INGRESS_FIELD:
            if (current_value == SYS_DFLT_1Q_PORT_INGRESS_FILTERING)
            {
                return;
            }
            break;

        case ACCEPTABLE_FRAME_TYPE_FIELD:
            if (current_value == SYS_DFLT_1Q_PORT_ACCEPTABLE_FRAME_TYPES)
            {
                return;
            }
            break;

        case GVRP_STATUS_FIELD:
            if (current_value == SYS_DFLT_1Q_PORT_GVRP_STATUS)
            {
                return;
            }
            break;

        case VLAN_PORT_MODE_FIELD:
            if (current_value == SYS_DFLT_VLAN_PORT_MODE)
            {
                return;
            }
            break;

        default:
            return;
    }

    *cfg_value = current_value;
    *changed_status = TRUE;
    return;
} /* end of VLAN_POM_CompareDefaultPortField() */
