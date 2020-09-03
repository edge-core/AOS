/* -------------------------------------------------------------------------------------
 * FILE NAME: VLAN_OM.h
 *
 * PURPOSE: This package serves as a database to store Q-bridge MIB defined information.
 * NOTES:   1. The two primary functions of this file is to maintain dot1qVlanStaticTable
 *             and dot1qPortVlanTable as defined in Q-bridge MIB.
 *          2. The vlan_om only contain two tables. One is vlan_table which contains all
 *             information of each VLAN (ex: creation time, VID, egress ports,
 *             untagged ports¡Ketc), and another is vlan_port_table which contains all
 *             information of each port (ex: PVID, acceptable_frame_type,
 *             ingress filtering...etc). The most content of these two tables is defined
 *             in RFC2674.
 *
 * MODIFICATION HISOTRY:
 * MODIFIER        DATE        DESCRIPTION
 * -------------------------------------------------------------------------------------
 * cpyang       6-19-2001      First created
 * amytu        8-01-2001      Revised VLAN_OM based on Q-bridge MIB definition.
 *              8-13-2002      Add private vlan data types:
 *                             { per vlan: private_vlan_type |
 *                               per port: vlan_port_private_vlan_type }
 * -------------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation 2001
 * -------------------------------------------------------------------------------------*/


#ifndef _VLAN_OM_H
#define _VLAN_OM_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "vlan_type.h"
#include "sysrsc_mgr.h"
#include "l_mm_type.h"
#include "l_pt.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define VLAN_OM_IPCMSG_TYPE_SIZE sizeof(union VLAN_OM_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    VLAN_OM_IPC_GETVLANENTRY,
    VLAN_OM_IPC_GETNEXTVLANENTRY,
    VLAN_OM_IPC_GETVLANPORTENTRY,
    VLAN_OM_IPC_GETDOT1QCONSTRAINTTYPEDEFAULT,
    VLAN_OM_IPC_GETDOT1QVLANVERSIONNUMBER,
    VLAN_OM_IPC_GETDOT1QMAXVLANID,
    VLAN_OM_IPC_GETDOT1QMAXSUPPORTEDVLANS,
    VLAN_OM_IPC_GETDOT1QNUMVLANS,
    VLAN_OM_IPC_GETDOT1QVLANNUMDELETES,
    VLAN_OM_IPC_GETDOT1QVLANSTATICENTRY,
    VLAN_OM_IPC_GETNEXTDOT1QVLANSTATICENTRY,
    VLAN_OM_IPC_GETDOT1QVLANCURRENTENTRY,
    VLAN_OM_IPC_GETNEXTDOT1QVLANCURRENTRY_FORUI,
    VLAN_OM_IPC_GETNEXTDOT1QVLANCURRENTENTRY,
    VLAN_OM_IPC_GETDOT1QNEXTFREELOCALVLANINDEX,
    VLAN_OM_IPC_GETMANAGEMENTVLAN,
    VLAN_OM_IPC_GETNEXTVLANID,
    VLAN_OM_IPC_ISVLANEXISTED,
    VLAN_OM_IPC_ISPORTVLANMEMBER_FORUI,
    VLAN_OM_IPC_ISPORTVLANMEMBER,
    VLAN_OM_IPC_ISVLANUNTAGPORTLISTMEMBER,
    VLAN_OM_IPC_ISVLANFORBIDDENPORTLISTMEMBER,
    VLAN_OM_IPC_CONVERTTOIFINDEX,
    VLAN_OM_IPC_CONVERTFROMIFINDEX,
    VLAN_OM_IPC_IFTABLELASTUPDATETIME,
    VLAN_OM_IPC_IFSTACKLASTUPDATETIME,

#if (SYS_CPNT_MAC_VLAN == TRUE)
    VLAN_OM_IPC_GETARPFORWARDMACVLAN,
#endif
    VLAN_OM_IPC_GETUNCERTIFIEDSTATICEGRESSPORTS,
    VLAN_OM_IPC_GETNEXTVLANID_BYLPORT,
    VLAN_OM_IPC_GETGLOBALDEFAULTVLANEX,
    VLAN_OM_IPC_GETRUNNINGGLOBALDEFAULTVLAN,
    VLAN_OM_IPC_GETVLANUPSTATE,
    VLAN_OM_IPC_GETNEXTDOT1QVLANCURRENTENTRY_WITH_PORTJOINED,
    VLAN_OM_IPC_GETVLANPORTPVID
};


/* MACRO FUNCTION DECLARATIONS
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in VLAN_OM_IpcMsg_T.data
 */
#define VLAN_OM_GET_MSG_SIZE(field_name)                        \
            (VLAN_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((VLAN_OM_IpcMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */

/* IPC message structure
 */
typedef struct
{
    union VLAN_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        BOOL_T ret_bool;
		UI32_T ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        UI32_T                          arg_ui32;
        VLAN_OM_Dot1qVlanCurrentEntry_T arg_current_entry;
        VLAN_OM_Vlan_Port_Info_T        arg_port_info;
        VLAN_MGR_Dot1qVlanStaticEntry_T arg_static_entry;
        struct
        {
            UI32_T                          arg1;
            VLAN_MGR_Dot1qVlanStaticEntry_T arg2;
        } arg_grp1;
        struct
        {
            UI32_T                          arg1;
            UI32_T                          arg2;
            VLAN_OM_Dot1qVlanCurrentEntry_T arg3;
        } arg_grp2;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
        } arg_grp3;
        struct
        {
            UI32_T arg1;
            UI8_T  arg2[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
        } arg_grp4;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
            UI32_T arg4;
            BOOL_T arg5;
            BOOL_T arg6;
        } arg_grp5;
        struct
        {
            UI32_T                          arg1;
            UI32_T                          arg2;
            UI32_T                          arg3;
            VLAN_OM_Dot1qVlanCurrentEntry_T arg4;
        } arg_grp6;
#if (SYS_CPNT_MAC_VLAN == TRUE)
        struct
        {
            UI32_T  out_vlan;
            UI8_T   mac_address[SYS_ADPT_MAC_ADDR_LEN];
        } arg_arp_fwd_mac_vlan;
#endif
    } data;
} VLAN_OM_IpcMsg_T;


typedef struct
{
 SYSFUN_DECLARE_CSC_ON_SHMEM

 L_PT_ShMem_Descriptor_T              vlan_shared_table_desc;
 VLAN_OM_Dot1qVlanCurrentEntry_T      vlan_shared_table[SYS_ADPT_MAX_NBR_OF_VLAN];
/* The index of the vlan_shared_entry_offset array is the vid,
 * and the value of vlan_shared_entry_offset[vid] indicate the offset of the corresponding
 * VLAN entry which is stored in the vlan_shared_table
 */
 UI32_T                               vlan_shared_entry_offset[SYS_ADPT_MAX_VLAN_ID];

 VLAN_OM_Vlan_Port_Info_T             vlan_shared_port_table[SYS_ADPT_RS232_1_IF_INDEX_NUMBER];
#if (SYS_CPNT_MAC_VLAN == TRUE)
 VLAN_TYPE_MacVlanEntry_T   mac_vlan_table[SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY];
 UI16_T mac_vlan_entry_count;
#endif
 /* The total numbers of current existent VLAN. */
 UI32_T   vlan_shard_om_current_cfg_vlan;
/* The global gvrp status. It's difference from the gvrp status on a port.
   Enable gvrp on a specific port must enable global gvrp status first. */
 UI32_T   vlan_shared_global_dot1q_gvrp_status;
/* The number of times a VLAN entry has been deleted from the dot1qVlanCurrentTable (for any reason).
   If an entry is deleted, then inserted, and then deleted, this counter will be incremented by 2. */
UI32_T   vlan_shard_num_vlan_deletes;
/* The value of time when the vlan_table or vlan_port_table was modified. */
 UI32_T   vlan_shard_last_update_time;
/* Specify whether IVL or SVL is the default constraint.
    VAL_dot1qConstraintTypeDefault_independent
    VAL_dot1qConstraintTypeDefault_shared
*/
 UI32_T   vlan_shard_dot1q_constraint_type_default;

 UI32_T   vlan_shard_default_vlan_id;


 UI32_T   vlan_shard_original_priority;

 UI32_T   vlan_shard_if_table_last_change;
 UI32_T   vlan_shard_if_stack_last_change;
 UI32_T   vlan_shard_management_vlan_id;

} VLAN_Shmem_Data_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan info is
              available.  Otherwise, return false.
 * INPUT    : vlan_info->vid  -- specify which vlan information to be retrieved
 * OUTPUT   : returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan info entry is available.
 *            Otherwise, false is returned.
 * INPUT    : vlan_info->vid  -- specify which vlan information to be retrieved
 * OUTPUT   : return next available vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanPortEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan port entry is
              available. Otherwise, false is returned
 * INPUT    : vlan_port_info->lport_ifidx  -- specify which port information to
 *            be retrieved
 * OUTPUT   : returns the specific vlan port info.
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetVlanPortEntry(VLAN_OM_Vlan_Port_Info_T *vlan_port_info);

BOOL_T VLAN_OM_GetVlanPortEntryByIfindex(UI32_T lport_ifindex,VLAN_OM_VlanPortEntry_T *vlan_port_entry);
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanPortEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan port entry is available.
 *            Otherwise, false is returned.
 * INPUT    : vlan_port_info->lport_ifidx  -- specify which port information
 *            to be retrieved
 * OUTPUT   : next available VLAN port entry
 * RETURN   : the start address of the next available vlan port entry.
 *            otherwise, NULL.
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextVlanPortEntry(VLAN_OM_Vlan_Port_Info_T *vlan_port_info);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanMember
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next Vlan member
 * INPUT    : vlan_ifindex  -- vlan ifindex
 *            lport         -- specified lport number
 *            type          -- one of the following type
 *                             VLAN_OM_VlanMemberType_CurrentUntagged   (0)
 *                             VLAN_OM_VlanMemberType_CurrentEgress     (1)
 *                             VLAN_OM_VlanMemberType_StaticEgress      (2)
 *                             VLAN_OM_VlanMemberType_ForbiddenEgress   (3)
 * OUTPUT   : lport         -- the next lport number of the specified lport
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : VLAN_OM_ASCENT is FALSE defined locally
 * ------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_GetNextVlanMember(UI32_T vlan_ifindex, UI32_T *lport, VLAN_OM_VlanMemberType_T type);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetMaxSupportVlanID
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the max vlan id number supported by the device.
 * INPUT    : None.
 * OUTPUT   : max_support_vid
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetMaxSupportVlanID(UI32_T *max_support_vid);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetMaxSupportVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the maximum number of vlan supported
 *            by the system if returned.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : max_support_vlan
 * RETURN   : TRUE/FALSE
 * NOTES    : This value is defined in SYS_ADPT.H
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetMaxSupportVlan(UI32_T *max_support_vlan);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetCurrentNumbOfVlanConfigured
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of vlans currently
 *             configured in the device is availabe.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : current_cfg_vlan
 * RETURN   : TRUE/FALSE
 * NOTES    : The total number of vlans currently configured in the device will include
 *            the static configured vlan and dynamic GVRP configured vlan.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetCurrentNumOfVlanConfigured(UI32_T *current_cfg_vlan);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetnumVlanDeletes
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of times the specific
 *            vlan deleted from the device is available.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : Number of vlan deleted from the system.
 * RETURN   : TRUE \ FALSE
 * NOTES    : The total number of times specific vlans has been deleted from the
 *            device will include the static and dynamic GVRP vlan delete.
 *--------------------------------------------------------------------------*/
UI32_T  VLAN_OM_GetnumVlanDeletes(void);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetCurrentNumbOfVlanConfigured
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the total number of vlans currently
 *             configured in the device is availabe.  Otherwise, false is returned.
 * INPUT    : None.
 * OUTPUT   : current_cfg_vlan
 * RETURN   : TRUE/FALSE
 * NOTES    : The total number of vlans currently configured in the device will include
 *            the static configured vlan and dynamic GVRP configured vlan.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetCurrentConfigVlan(UI32_T *current_cfg_vlan);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_LastUpdateTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the time tick the last time vlan_om is
 *            updated.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : timetick.
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T  VLAN_OM_LastUpdateTime();

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanDeleteFrequency
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the gvrp status for the device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : gvrp_status - VAL_dot1qGvrpStatus_enabled \
 *                          VAL_dot1qGvrpStatus_disabled
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetGlobalGvrpStatus(UI32_T *gvrp_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qConstraintTypeDefault
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetDot1qConstraintTypeDefault(UI32_T *constrain_type);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetGlobalDefaultVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be get successfully.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : default vlan id
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VLAN_OM_GetGlobalDefaultVlan();

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNext_Vlan_With_PortJoined
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan info entry for the
 *            specified port is available. Otherwise, false is returned.
 * INPUT    : lport                        -- specify the lport ifindex
 *            vlan_info->dot1q_vlan_index  -- specify which vlan information to be retrieved
 * OUTPUT   : return next available vlan info
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNext_Vlan_With_PortJoined(UI32_T lport, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);

/*=============================================================================
 * Moved from vlan_mgr.h
 *=============================================================================
 */

/* Dot1qBase group
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanVersionNumber
 *-----------------------------------------------------------------------------
 * PURPOSE  : It is used to identify the vlan version of the system.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The vlan version number
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 12.10.1.1 {dot1qBase 1}
 *               for detailed information.
 *            2. It's always return "1" right now.
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qVlanVersionNumber(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qMaxVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the maximum IEEE 802.1Q VLAN ID that
 *            this device supported.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The maximum number of VLAN ID
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 9.3.2.3 {dot1qBase 2}
 *               for detailed information.
 *            2. It returns the "SYS_ADPT_MAX_VLAN_ID" defined of system.
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qMaxVlanId(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_Dot1qMaxSupportedVlans
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the maximum (total) number of IEEE 802.1Q
 *            VLANs that this device supports.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The maximum number of VLAN supported by the system.
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 12.10.1.1"{dot1qBase 3}
 *               for detailed information.
 *            2. It returns the "SYS_ADPT_MAX_NBR_OF_VLAN" defined of system.
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qMaxSupportedVlans(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qNumVlans
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the number of IEEE 802.1Q VLANs are
 *            currently configured in the system.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The current numbers of VLAN configured
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 Section 12.7.1.1" {dot1qBase 4}
 *               for detailed information.
 *            2. It returns the numbers of static vlan.
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qNumVlans(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qGvrpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the GVRP status of the bridge can be
 *            successfully retrieved.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : gvrp_status - VAL_dot1qGvrpStatus_enabled \ VAL_dot1qGvrpStatus_disabled
 * RETURN   : TRUE \ FALSE
 * NOTES    : The administrative status requested by management for GVRP.  The
 *            value enabled(1) indicates that GVRP should be enabled on this
 *            device, on all ports for which it has not been specifically disabled.
 *            When disabled(2), GVRP is disabled on all ports and all GVRP packets
 *            will be forwarded transparently.  This object affects all GVRP
 *            Applicant and Registrar state machines.  A transition from disabled(2)
 *            to enabled(1) will cause a reset of all GVRP state machines on all ports
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qGvrpStatus(UI32_T *gvrp_status);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_CurrentConfiguredMaxVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the largest vlan id currently existed in
 *            the database.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : The largest vlan id value
 * NOTES    : For CLI use
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_CurrentConfiguredMaxVlanId(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanNumDeletes
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the number of vlan that has been deleted
 *            from vlan database.
 * INPUT    : None
 * OUTPUT   : Number of vlan that has been deleted.
 * RETURN   : The numbers of a VLAN entry has been deleted
 * NOTES    : 1. Please refer to IEEE 802.1Q/D11 {dot1qVlan 1} for detailed
 *               information.
 *            2. If an entry s deleted, then inserted, and then deleted, this
 *               counter will be incremented by 2
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetDot1qVlanNumDeletes(void);

/* Es3626a Private Mib
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       -- the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qVlanStaticEntry(UI32_T vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       --  the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextDot1qVlanStaticEntry(UI32_T *vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanCurrentEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the entry is greater than or equal to the
 *            input time_mark.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qVlanCurrentEntry(UI32_T time_mark, UI32_T vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanCurrentEntry_forUI
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the next available entry is greater than or
 *            equal to the input time_mark.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntry_forUI(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanCurrentEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the next available entry is greater than or
 *             equal to the input time_mark.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntry(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);
/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextDot1qVlanCurrentEntrySortByTimemark
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
              available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the next available entry is greater than or
 *            equal to the input time_mark.
 *------------------------------------------------------------------------------*/
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntrySortByTimemark(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qNextFreeLocalVlanIndex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available value for dot1qVlanIndex
 *            of a local VLAN entry in dot1qVlanStaticTable can be retrieve successfully.
 *            Otherwise, return FALSE.
 * INPUT    : none
 * OUTPUT   : *vid      -- the next vlan id
 * RETURN   : TRUE\ FALSE
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetDot1qNextFreeLocalVlanIndex(UI32_T *vid);

/* Management VLAN API
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetManagementVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : get the global management VLAN
 * INPUT    : none
 * OUTPUT   : vid - management vlan id
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. this function is paired with SetGlobalManagementVlan()
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetManagementVlan(UI32_T *vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextIpInterface
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the nex VLAN which is labeled as IP interface
 * INPUT    : vid
 * OUTPUT   : vid - management vlan id
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. This function is coupled with VLAN_OM_SetIpInterface()
 *               and VLAN_OM_LeaveManagementVlan()
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextIpInterface(UI32_T *vid);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetL3IPv6VlanState
 * ----------------------------------------------------------------------------
 * PURPOSE  : This function the state returns true if the vlan is l3 ipv6 vlan
 *            Otherwise, return FALSE.
 * INPUT    : vid - vlan id
 * OUTPUT   : state-is l3 ipv6 vlan or not.
 * RETURN   : TRUE \ FALSE
 * NOTES    : To be phased out, please use VLAN_OM_GetVlanMgmtIpState() instead
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetL3IPv6VlanState(UI32_T vid, BOOL_T *state);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanMgmtIpState
 * ----------------------------------------------------------------------------
 * PURPOSE  : Get management vlan state and ip interface state of a vlan.
 * INPUT    : vid - the identifier of a vlan
 * OUTPUT   : mgmt_state - whether is management vlan
 *            ip_state   - whether has ip interface and which version
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetVlanMgmtIpState(UI32_T vid, BOOL_T* mgmt_state, UI8_T* ip_state);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanMgmtIpState
 * ----------------------------------------------------------------------------
 * PURPOSE  : Get management vlan state and ip interface state of next vlan.
 * INPUT    : vid - the identifier of a vlan
 * OUTPUT   : vid        - the identifier of the next existed vlan
 *            mgmt_state - whether is management vlan
 *            ip_state   - whether has ip interface and which version
 * RETURN   : TRUE \ FALSE
 * NOTES    : None
 * ----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextVlanMgmtIpState(UI32_T* vid, BOOL_T* mgmt_state, UI8_T* ip_state);

/*  Miselleneous API
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanId
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next vlan id whose create_time is
 *            greater then time_mark is available.  Otherwise, return false.
 * INPUT    : time_mark -- time mark of serching key
 * OUTPUT   : *vid       -- vlan id
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. Time mark as the searching key.
 *            2. The condition for dynamic vlan existed is that if there is
 *               at least one port register within the member set.
 *            3. The condition for static vlan existed is that if the status
 *               is in "Row_Status_Active".
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextVlanId(UI32_T time_mark, UI32_T *vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsVlanExisted
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan exist in the database.
 *            Otherwise, return false.
 * INPUT    : vid     -- specified vlan id
 * OUTPUT   : none
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The condition for dynamic vlan existed is that if there is
 *               one port regist within the port list.
 *            2. When a vlan is existed, it must either dynamic "Register" or
 *               static "Row_Status_Active" to identify active.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsVlanExisted(UI32_T vid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetDot1qVlanRowStatus
 *--------------------------------------------------------------------------
 * PURPOSE	:
 * INPUT		: vid  -- specified vlan id
 * OUTPUT 	: none
 * RETURN 	:
 * NOTES		: none
 *--------------------------------------------------------------------------*/
UI32_T VLAN_OM_GetDot1qVlanRowStatus(UI32_T vid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsPortVlanMember_forUI
 *--------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in vlan's member
 *            list (egress_port).  Otherwise, returns false.
 * INPUT    : dot1q_vlan_index   -- vlan index number
              lport_ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VLAN_OM_IsPortVlanMember_forUI(UI32_T dot1q_vlan_index, UI32_T lport_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsPortVlanMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in vlan's member
 *            list (egress_port_list).  Otherwise, returns false.
 * INPUT    : vid_ifindex   -- vlan index number
              lport_ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsPortVlanMember(UI32_T vid_ifindex, UI32_T lport_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsVlanUntagPortListMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if lport_ifindex is in vlan_info's
 *            dot1q_vlan_current_untagged_ports, otherwise, return false.
 * INPUT    : vlan_info -- typedef vlan struct that holds vlan information
              lport -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsVlanUntagPortListMember(UI32_T vid_ifindex, UI32_T lport_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsVlanForbiddenPortMember
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if lport_ifindex is in vlan entry's
 *            dot1q_vlan_forbidden_egress_ports.  Otherwise, returns false.
 * INPUT    : vlan_info -- typedef vlan struct that holds vlan information
              lport -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsVlanForbiddenPortListMember(UI32_T vid_ifindex, UI32_T lport_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IfTableLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the last time a vlan entry is create or deleted
 *            from VLAN_OM
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_IfTableLastUpdateTime();

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IfStackLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the last time vlan egress port list is modified.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_IfStackLastUpdateTime();

#if (SYS_CPNT_MAC_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_ClearMacVlanDatabase
 *-------------------------------------------------------------------------
 * PURPOSE  : Clear the MAC VLAN OM
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void VLAN_OM_ClearMacVlanDatabase(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the MAC VLAN entry
 * INPUT    : mac_address       - only allow unitcast address
 *            mask              - mask
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 * RETURN   : TRUE/FALSE        - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetMacVlanEntry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next MAC VLAN entry
 * INPUT    : mac_address       - only allow unitcast address
 *            mask              - mask
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 * RETURN   : TRUE/FALSE        - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextMacVlanEntry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextMacVlanEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next MAC VLAN entry
 * INPUT    : next_index_p      - om index value
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 *            *next_index_p     - current array index
 * RETURN   : TRUE/FALSE        - TRUE if successful;FALSE if failed
 * NOTES    : -1 to get first
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextMacVlanEntryByIndex(I32_T *next_index_p, VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_SetMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the MAC VLAN entry
 * INPUT    : mac_address   - only allow unitcast address
 *            vid           - the VLAN ID
 *                            the valid value is 1 ~ SYS_DFLT_DOT1QMAXVLANID
 *            mask          - mask
 *            priority      - the priority
 *                            the valid value is 0 ~ 7
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : if SYS_CPNT_MAC_VLAN_WITH_PRIORITY == FALSE, it's recommanded
 *            that set input priority to 0.
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_SetMacVlanEntry(UI8_T *mac_address, UI8_T *mask, UI16_T vid, UI8_T priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_DeleteMacVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Delete Mac Vlan entry
 * INPUT    : mac_address   - only allow unitcast address
 *            mask          - mask
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_DeleteMacVlanEntry(UI8_T *mac_address, UI8_T* mask);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsMacVlanTableFull
 *-------------------------------------------------------------------------
 * PURPOSE  : Verify om is not space to store
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE    - TRUE if successful;FALSE if failed
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsMacVlanTableFull();
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - VLAN_OM_GetNextRunningMacVlanEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : Get next RUNNING MAC VLAN entry.
 * INPUT    : mac_address       - only allow unitcast address
 *                                use 0 to get the first entry
 * OUTPUT   : mac_vlan_entry    - the MAC VLAN entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T VLAN_OM_GetNextRunningMacVlanEntry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry);
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetArpForwardMacVlan
 *-----------------------------------------------------------------------------
 * PURPOSE : Check has mac vlan define and return input port's forward vlan for arp
 * INPUT   : lport    - to check which port
 * OUTPUT  : out_vlan - the arp shall forward port
 * RETURN  : TRUE  - has arp forward vlan
 *           FALSE - no arp group define or not forward vlan
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetArpForwardMacVlan(UI8_T *mac_address, UI32_T *out_vlan);
/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_OM_CorrectIngressVidFromMacBasedVlan
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Correct the vid information
 * INPUT    :
 *            src_mac     -- Source address
 *            tag_info    -- raw tagged info of the packet
 *            src_unit    -- source unit
 *            src_port    -- source port
 * OUTPUT   : tag_info    -- raw tagged info of the packet
 * RETURN   : TRUE  -- change vid
 *            FALSE -- not mac vlan configure for this src mac
 * NOTE     :
 * ----------------------------------------------------------------------------------*/
BOOL_T VLAN_OM_CorrectIngressVidFromMacBasedVlan(
	                                             UI8_T *src_mac,
                                                 UI16_T *tag_info);
#endif /* end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

#if (SYS_CPNT_SOFTBRIDGE == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanList_ByLport
 *-------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if the next vlans in which the
 *            specified port joins is available. Otherwise, returns false.
 * INPUT    : lport     -- logical port number
 *            vid       -- the vlan ID
 *            num       -- number of the following vlans on demand
 * OUTPUT   : vlist     -- the pointer of the output buffer for the following
 *                         vlan list
 *            rtn_num   -- number of the returned vlans following the specified
 *                         vid
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextVlanList_ByLport(UI32_T lport, UI16_T vid, UI32_T num, UI16_T *vlist, UI16_T *rtn_num);

#endif /* #if (SYS_CPNT_SOFTBRIDGE == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetUncertifiedStaticEgressPorts
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the static egress ports which are not cerified by Radius server
 * INPUT    : vid           -- the search key
 * OUTPUT   : egress_ports  -- the port bit map
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetUncertifiedStaticEgressPorts(UI32_T vid, UI8_T *egress_ports);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNextVlanId_ByLport
 *-----------------------------------------------------------------------------
 * PURPOSE  : To know the specified port belongs to which VLANs
 * INPUT    : lport_ifindex     -- specify the lport
 *            member_type       -- VLAN_MGR_UNTAGGED_ONLY
 *                                 VLAN_MGR_TAGGED_ONLY
 *                                 VLAN_MGR_BOTH
 *            member_status     -- VLAN_MGR_PERMANENT_ONLY
 *                                 VLAN_MGR_DYNAMIC_ONLY
 *                                 VLAN_MGR_BOTH
 *            vid               -- the specific vlan id.
 * OUTPUT   : *vid              -- the next vlan id
 *            *is_tagged        -- only meaningful when member_type is VLAN_MGR_BOTH
 *                                 TRUE[tagged member]
 *                                 FALSE[untagged member]
 *            *is_static        -- only meaningful when member_status is VLAN_MGR_BOTH
 *                                 TRUE[static member]
 *                                 FALSE[dynamic member]
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  VLAN_OM_GetNextVlanId_ByLport (UI32_T lport_ifindex, UI32_T member_type,
                                       UI32_T member_status, UI32_T *vid,
                                       BOOL_T *is_tagged, BOOL_T *is_static);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetGlobalDefaultVlan_Ex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be get successfully.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : vid
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetGlobalDefaultVlan_Ex(UI32_T *vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetRunningGlobalDefaultVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be get successfully.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : vid
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : if the return value is SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *            there shall be "no vlan [SYS_DFLT_1Q_PORT_VLAN_PVID]" command
 *            in the running config
 *-----------------------------------------------------------------------------
 */
UI32_T VLAN_OM_GetRunningGlobalDefaultVlan(UI32_T *vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetNext_Vlan_With_PortJoined
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan info entry for the
 *            specified port is available. Otherwise, false is returned.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 *            lport     -- the third search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : 1. vlan_om will return the specific vlan entry only in the case if
 *               dot1q_vlan_time_mark of the next available entry is greater than or
 *               equal to the input time_mark.
 *            2. if the port only joins VLAN statically but currently, this function
 *               will not return vlan info for this vlan
 *-----------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetNextDot1qVlanCurrentEntry_With_PortJoined(UI32_T time_mark, UI32_T *vid, UI32_T lport, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for VLAN OM.
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
BOOL_T VLAN_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

BOOL_T VLAN_OM_IsVlanUp(UI32_T vid);

void VLAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);
void VLAN_OM_AttachSystemResources(void);
void VLAN_OM_InitiateSystemResources(void);
BOOL_T VLAN_OM_GetDot1qPortVlanEntry(UI32_T lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *vlan_port_entry);
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanStatus
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan status in the database.
 * INPUT    : vid     -- specified vlan id
 * OUTPUT   : none
 *-----------------------------------------------------------------------------
 */
UI8_T VLAN_OM_GetVlanStatus(UI32_T vid);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_GetVlanPortPvid
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan port entry is
              available. Otherwise, false is returned
 * INPUT    : lport_ifindex  -- specify which port information to be retrieved
 * OUTPUT   : dot1q_pvid_index -- the pvid information of the lport_ifindex.
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_GetVlanPortPvid(UI32_T lport_ifindex, UI32_T *dot1q_pvid_index);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_OM_IsPortVlanStaticMember
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if a port is a static member of a VLAN.
 * INPUT    : vid   -- vlan index number
              lport -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE  -- is static member
 *            FALSE -- is not static member
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T VLAN_OM_IsPortVlanStaticMember(UI32_T vid, UI32_T lport);

BOOL_T VLAN_OM_GetPortAddedVlanBitmap(UI32_T lport, 
                                      UI8_T tagged_bitmap[(SYS_ADPT_MAX_NBR_OF_VLAN/8)+1], 
                                      UI8_T untagged_bitmap[(SYS_ADPT_MAX_NBR_OF_VLAN/8)+1]);

BOOL_T VLAN_OM_IsPortVlanInactiveMember(UI32_T vid, UI32_T lport);

#endif /* End of VLAN_OM.H */
