/*-----------------------------------------------------------------------------
 * FILE NAME: VLAN_POM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for VLAN OM IPC.
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

#ifndef VLAN_POM_H
#define VLAN_POM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "vlan_om.h"
#include "vlan_type.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : VLAN_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for VLAN_POM in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_InitiateProcessResource(void);*/
#define VLAN_POM_InitiateProcessResource()

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetVlanEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan info is
              available.  Otherwise, return false.
 * INPUT    : vlan_info->vid  -- specify which vlan information to be retrieved
 * OUTPUT   : returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);*/
#define VLAN_POM_GetVlanEntry(vlan_info) VLAN_OM_GetVlanEntry(vlan_info)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetNextVlanEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next vlan info entry is available.
 *            Otherwise, false is returned.
 * INPUT    : vlan_info->vid  -- specify which vlan information to be retrieved
 * OUTPUT   : return next available vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetNextVlanEntry(VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_info);*/
#define VLAN_POM_GetNextVlanEntry(vlan_info) VLAN_OM_GetNextVlanEntry(vlan_info)

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetVlanPortEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan port entry is
              available. Otherwise, false is returned
 * INPUT    : vlan_port_info->lport_ifidx  -- specify which port information to
 *            be retrieved
 * OUTPUT   : returns the specific vlan port info.
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------*/
/*BOOL_T VLAN_POM_GetVlanPortEntry(VLAN_OM_Vlan_Port_Info_T *vlan_port_info);*/
#define VLAN_POM_GetVlanPortEntry(vlan_port_info) VLAN_OM_GetVlanPortEntry(vlan_port_info)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qConstraintTypeDefault
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the constrain type for this device
 *            can be retrieve successfully.  Otherwise, return false.
 * INPUT    : None
 * OUTPUT   : VAL_dot1qConstraintTypeDefault_independent\
 *            VAL_dot1qConstraintTypeDefault_shared
 * RETURN   : TRUE \ FALSE
 * NOTES    : System_default is IVL.
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetDot1qConstraintTypeDefault(UI32_T *constrain_type);*/
#define VLAN_POM_GetDot1qConstraintTypeDefault(constrain_type) VLAN_OM_GetDot1qConstraintTypeDefault(constrain_type)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qVlanVersionNumber
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
/*UI32_T VLAN_POM_GetDot1qVlanVersionNumber(void);*/
#define VLAN_POM_GetDot1qVlanVersionNumber VLAN_OM_GetDot1qVlanVersionNumber

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qMaxVlanId
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
/*UI32_T VLAN_POM_GetDot1qMaxVlanId(void);*/
#define VLAN_POM_GetDot1qMaxVlanId VLAN_OM_GetDot1qMaxVlanId

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qMaxSupportedVlans
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
/*UI32_T VLAN_POM_GetDot1qMaxSupportedVlans(void);*/
#define VLAN_POM_GetDot1qMaxSupportedVlans VLAN_OM_GetDot1qMaxSupportedVlans

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qNumVlans
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
/*UI32_T VLAN_POM_GetDot1qNumVlans(void);*/
#define VLAN_POM_GetDot1qNumVlans VLAN_OM_GetDot1qNumVlans

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qVlanNumDeletes
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
/*UI32_T VLAN_POM_GetDot1qVlanNumDeletes(void);*/
#define VLAN_POM_GetDot1qVlanNumDeletes VLAN_OM_GetDot1qVlanNumDeletes

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qVlanStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       -- the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetDot1qVlanStaticEntry(UI32_T vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);*/
#define VLAN_POM_GetDot1qVlanStaticEntry(vid, vlan_entry) VLAN_OM_GetDot1qVlanStaticEntry(vid, vlan_entry)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetNextDot1qVlanStaticEntry
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available static vlan entry is
              available.  Otherwise, return false.
 * INPUT    : vid       --  the specific vlan id.
 * OUTPUT   : *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : The vid parameter is as the primary search key.
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetNextDot1qVlanStaticEntry(UI32_T *vid, VLAN_MGR_Dot1qVlanStaticEntry_T *vlan_entry);*/
#define VLAN_POM_GetNextDot1qVlanStaticEntry(vid, vlan_entry) VLAN_OM_GetNextDot1qVlanStaticEntry(vid, vlan_entry)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qVlanCurrentEntry
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
/*BOOL_T VLAN_POM_GetDot1qVlanCurrentEntry(UI32_T time_mark, UI32_T vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);*/
#define VLAN_POM_GetDot1qVlanCurrentEntry(time_mark, vid, vlan_entry) VLAN_OM_GetDot1qVlanCurrentEntry(time_mark, vid, vlan_entry)

/*----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetNextDot1qVlanCurrentEntry_forUI
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available current vlan entry is
 *            available.  Otherwise, return false.
 * INPUT    : time_mark -- the primary search key
 *            vid       -- the secondary search key
 * OUTPUT   : *vid      -- the next vlan id
 *            *vlan_entry -- returns the specific vlan info
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : vlan_om will return the specific vlan entry only in the case if
 *            dot1q_vlan_time_mark of the next available entry is greater than or
 *            equal to the input time_mark.
 *------------------------------------------------------------------------------*/
/*BOOL_T VLAN_POM_GetNextDot1qVlanCurrentEntry_forUI(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);*/
#define VLAN_POM_GetNextDot1qVlanCurrentEntry_forUI(time_mark, vid, vlan_entry) VLAN_OM_GetNextDot1qVlanCurrentEntry_forUI(time_mark, vid, vlan_entry)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetNextDot1qVlanCurrentEntry
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
/*BOOL_T VLAN_POM_GetNextDot1qVlanCurrentEntry(UI32_T time_mark, UI32_T *vid, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);*/
#define VLAN_POM_GetNextDot1qVlanCurrentEntry(time_mark, vid, vlan_entry) VLAN_OM_GetNextDot1qVlanCurrentEntry(time_mark, vid, vlan_entry)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetDot1qNextFreeLocalVlanIndex
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
/*BOOL_T VLAN_POM_GetDot1qNextFreeLocalVlanIndex(UI32_T *vid);*/
#define VLAN_POM_GetDot1qNextFreeLocalVlanIndex(vid) VLAN_OM_GetDot1qNextFreeLocalVlanIndex(vid)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetManagementVlan
 *-----------------------------------------------------------------------------
 * PURPOSE  : get the global management VLAN
 * INPUT    : none
 * OUTPUT   : vid - management vlan id
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. this function is paired with SetGlobalManagementVlan()
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetManagementVlan(UI32_T *vid);*/
#define VLAN_POM_GetManagementVlan(vid) VLAN_OM_GetManagementVlan(vid)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetNextVlanId
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
/*BOOL_T VLAN_POM_GetNextVlanId(UI32_T time_mark, UI32_T *vid);*/
#define VLAN_POM_GetNextVlanId(time_mark, vid) VLAN_OM_GetNextVlanId(time_mark, vid)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_IsVlanExisted
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
/*BOOL_T VLAN_POM_IsVlanExisted(UI32_T vid);*/
#define VLAN_POM_IsVlanExisted(vid) VLAN_OM_IsVlanExisted(vid)

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_IsPortVlanMember_forUI
 *--------------------------------------------------------------------------
 * PURPOSE  : This funcion returns true if current port is in vlan's member
 *            list (egress_port).  Otherwise, returns false.
 * INPUT    : dot1q_vlan_index   -- vlan index number
 *            lport_ifindex -- the specified port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/
/*BOOL_T VLAN_POM_IsPortVlanMember_forUI(UI32_T dot1q_vlan_index, UI32_T lport_ifindex);*/
#define VLAN_POM_IsPortVlanMember_forUI(dot1q_vlan_index, lport_ifindex) VLAN_OM_IsPortVlanMember_forUI(dot1q_vlan_index, lport_ifindex)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_IsPortVlanMember
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
/*BOOL_T VLAN_POM_IsPortVlanMember(UI32_T vid_ifindex, UI32_T lport_ifindex);*/
#define VLAN_POM_IsPortVlanMember(vid_ifindex, lport_ifindex) VLAN_OM_IsPortVlanMember(vid_ifindex, lport_ifindex)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_IsVlanUntagPortListMember
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
/*BOOL_T VLAN_POM_IsVlanUntagPortListMember(UI32_T vid_ifindex, UI32_T lport_ifindex);*/
#define VLAN_POM_IsVlanUntagPortListMember(vid_ifindex, lport_ifindex) VLAN_OM_IsVlanUntagPortListMember(vid_ifindex, lport_ifindex)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_IsVlanForbiddenPortListMember
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
/*BOOL_T VLAN_POM_IsVlanForbiddenPortListMember(UI32_T vid_ifindex, UI32_T lport_ifindex);*/
#define VLAN_POM_IsVlanForbiddenPortListMember(vid_ifindex, lport_ifindex) VLAN_OM_IsVlanForbiddenPortListMember(vid_ifindex, lport_ifindex)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_IfTableLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the last time a vlan entry is create or deleted
 *            from VLAN_OM
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
/*UI32_T VLAN_POM_IfTableLastUpdateTime(void);*/
#define VLAN_POM_IfTableLastUpdateTime VLAN_OM_IfTableLastUpdateTime

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_IfStackLastUpdateTime
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns the last time vlan egress port list is modified.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : Time tick
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
/*UI32_T VLAN_POM_IfStackLastUpdateTime(void);*/
#define VLAN_POM_IfStackLastUpdateTime VLAN_OM_IfStackLastUpdateTime


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetUncertifiedStaticEgressPorts
 *-----------------------------------------------------------------------------
 * PURPOSE  : Get the static egress ports which are not cerified by Radius server
 * INPUT    : vid           -- the search key
 * OUTPUT   : egress_ports  -- the port bit map
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetUncertifiedStaticEgressPorts(UI32_T vid, UI8_T *egress_ports);*/
#define VLAN_POM_GetUncertifiedStaticEgressPorts(vid, egress_ports) VLAN_OM_GetUncertifiedStaticEgressPorts(vid, egress_ports)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetNextVlanId_ByLport
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
/*BOOL_T  VLAN_POM_GetNextVlanId_ByLport(UI32_T lport_ifindex, UI32_T member_type,
                                       UI32_T member_status, UI32_T *vid,
                                       BOOL_T *is_tagged, BOOL_T *is_static);*/

#define VLAN_POM_GetNextVlanId_ByLport(lport_ifindex, member_type, member_status, vid, is_tagged, is_static) \
        VLAN_OM_GetNextVlanId_ByLport(lport_ifindex, member_type, member_status, vid, is_tagged, is_static)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetGlobalDefaultVlanEx
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default VLAN for this device
 *            can be get successfully.  Otherwise, return false.
 * INPUT    : none
 * OUTPUT   : vid
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetGlobalDefaultVlanEx(UI32_T *vid);*/
#define VLAN_POM_GetGlobalDefaultVlanEx(vid) VLAN_OM_GetGlobalDefaultVlan_Ex(vid)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetRunningGlobalDefaultVlan
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
/*UI32_T VLAN_POM_GetRunningGlobalDefaultVlan(UI32_T *vid);*/
#define VLAN_POM_GetRunningGlobalDefaultVlan(vid) VLAN_OM_GetRunningGlobalDefaultVlan(vid)

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetNextDot1qVlanCurrentEntry_With_PortJoined
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
/*BOOL_T VLAN_POM_GetNextDot1qVlanCurrentEntry_With_PortJoined(UI32_T time_mark, UI32_T *vid, UI32_T lport, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry);*/
#define VLAN_POM_GetNextDot1qVlanCurrentEntry_With_PortJoined(time_mark, vid, lport, vlan_entry) VLAN_OM_GetNextDot1qVlanCurrentEntry_With_PortJoined(time_mark, vid, lport, vlan_entry)

/*BOOL_T VLAN_POM_IsVlanUp( UI32_T vid);*/
#define VLAN_POM_IsVlanUp(vid) VLAN_OM_IsVlanUp(vid)

/*BOOL_T VLAN_POM_GetDot1qPortVlanEntry(UI32_T lport_ifindex, VLAN_OM_Dot1qPortVlanEntry_T *vlan_port_entry);*/
#define VLAN_POM_GetDot1qPortVlanEntry(lport_ifindex, vlan_port_entry) VLAN_OM_GetDot1qPortVlanEntry(lport_ifindex, vlan_port_entry)

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_POM_GetVlanPortPvid
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan port entry is
              available. Otherwise, false is returned
 * INPUT    : lport_ifindex  -- specify which port information to be retrieved
 * OUTPUT   : dot1q_pvid_index -- the pvid information of the lport_ifindex.
 * RETURN   : the start address of the entry, otherwise NULL
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
/*BOOL_T VLAN_POM_GetVlanPortPvid(UI32_T lport_ifindex, UI32_T *dot1q_pvid_index);*/
#define VLAN_POM_GetVlanPortPvid(lport_ifindex, dot1q_pvid_index) VLAN_OM_GetVlanPortPvid(lport_ifindex, dot1q_pvid_index)

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
UI32_T VLAN_POM_GetRunningVlanParameters(UI32_T vid, VLAN_TYPE_Vlan_RunningCfg_T *vlan_cfg);

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
UI32_T VLAN_POM_GetRunningVlanPortParameters(UI32_T lport_ifindex, VLAN_TYPE_Vlan_Port_RunningCfg_T *vlan_port_cfg);

#endif /* #ifndef VLAN_POM_H */
