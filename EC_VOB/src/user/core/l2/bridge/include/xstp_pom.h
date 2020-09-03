/*-----------------------------------------------------------------------------
 * FILE NAME: XSTP_POM.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for XSTP OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/23     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef XSTP_POM_H
#define XSTP_POM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "xstp_om.h"
/*Tony.Lei work aroud for fixing bug 387*/
#include "sysrsc_mgr.h"

void XSTP_SHARE_OM_InitiateSystemResources(void);

void XSTP_SHARE_OM_AttachSystemResources(void);

void XSTP_SHARE_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);
/*end by Tony.Lei*/
/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for XSTP_POM in the calling process.
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
BOOL_T XSTP_POM_InitiateProcessResource(void);

/* ===================================================================== */
/* System Information function
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetForceVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the system force version
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : force version
 *-------------------------------------------------------------------------
 */
UI8_T   XSTP_POM_GetForceVersion(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMaxHopCount
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_hop_count
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_hop_count
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetMaxHopCount(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMaxInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the max_instance_number
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : max_instance_number
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetMaxInstanceNumber(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNumOfActiveTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the num_of_active_tree
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_active_tree
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetNumOfActiveTree(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRegionName
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region name
 * INPUT    : None
 * OUTPUT   : str       -- region name
 * RETURN   : None
 *-------------------------------------------------------------------------
 */
void    XSTP_POM_GetRegionName(UI8_T *str);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRegionRevision
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the region_revision
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : region_revision
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRegionRevision(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the spanning tree status
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : force version
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetSpanningTreeStatus(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTrapFlagTc
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_tc
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_tc
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetTrapFlagTc(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTrapFlagNewRoot
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the trap_flag_new_root
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : trap_flag_new_root
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetTrapFlagNewRoot(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void XSTP_POM_GetMstidFromMstConfigurationTableByVlan(UI32_T vid, UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextXstpMemberFromMstConfigurationTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next XSTP member form mst config table for a
 *            specified instance
 * INPUT    : mstid     -- this instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextXstpMemberFromMstConfigurationTable(UI32_T mstid,
                                                            UI32_T *vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMstFullMemberTopology
 * ------------------------------------------------------------------------
 * PURPOSE  : This function returns TRUE if mst_topology_method is full member topology.
 *            Otherwise, return FALSE.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 *-------------------------------------------------------------------------
 */
BOOL_T   XSTP_POM_IsMstFullMemberTopology(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetCurrentCfgInstanceNumber
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the current instance number created by user.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : num_of_cfg_msti
 *-------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetCurrentCfgInstanceNumber(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetInstanceEntryId
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : None
 * RETUEN   : entry_id (stg_id) for the specified xstid
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
UI8_T   XSTP_POM_GetInstanceEntryId(UI32_T xstid);

/*=============================================================================
 * Move from xstp_svc.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                                 VAL_dot1dStpPortState_blocking
 *                                 VAL_dot1dStpPortState_listening
 *                                 VAL_dot1dStpPortState_learning
 *                                 VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortStateByVlan(UI32_T vid, UI32_T lport, UI32_T *state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T Xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   U32_T  *state   -- the pointer of state value
 *                                 VAL_dot1dStpPortState_blocking
 *                                 VAL_dot1dStpPortState_listening
 *                                 VAL_dot1dStpPortState_learning
 *                                 VAL_dot1dStpPortState_forwarding
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortStateByInstance(UI32_T xstid, UI32_T lport, UI32_T *state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortForwardingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in VAL_dot1dStpPortState_forwarding,
 *              else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_IsPortForwardingStateByVlan(UI32_T vid, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortForwardingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in VAL_dot1dStpPortState_forwarding,
 *              else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_IsPortForwardingStateByInstance(UI32_T xstid, UI32_T lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortBlockingStateByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified vlan.
 * INPUT    :   UI32_T vid      -- vlan id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsPortBlockingStateByVlan(UI32_T vid, UI32_T lport);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsPortBlockingStateByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state with a specified instance.
 * INPUT    :   UI32_T xstid    -- mst instance id
 *              UI32_T lport    -- lport number
 * OUTPUT   :   None
 * RETURN   :   TRUE if the port state is in Blocking, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsPortBlockingStateByInstance(UI32_T xstid, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistingMstidByLport
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next mst instance id with a specified lport.
 * INPUT    :   UI32_T lport    -- lport number
 * OUTPUT   :   UI32_T *xstid   -- mst instance id
 * OUTPUT   :   UI32_T *xstid   -- next mst instance id
 * RETURN   :   TRUE if the next mstid is existing, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextExistingMstidByLport(UI32_T lport, UI32_T *xstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistingMemberVidByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the next member vlan id with a specified mst instance id.
 * INPUT    :   UI32_T xstid    -- mst instance id
 * OUTPUT   :   UI32_T *vid     -- vlan id
 * OUTPUT   :   UI32_T *vid     -- next member vlan id
 * RETURN   :   TRUE if the next member vlan is existing, else FALSE
 * NOTES    :   None
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextExistingMemberVidByMstid(UI32_T xstid, UI32_T *vid);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstInstanceIndexByMstid
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the entry_id for the specified xstid
 * INPUT    : xstid -- MST instance ID
 * OUTPUT   : mstudx -- MST instance INDEX
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T   XSTP_POM_GetMstInstanceIndexByMstid(UI32_T xstid, UI32_T *mstidx);


/*=============================================================================
 * Move from xstp_mgr.h
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningStaSystemStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningSystemSpanningTreeStatus(UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetStaSystemStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global spanning tree status.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetSystemSpanningTreeStatus(UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRuningSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningSystemSpanningTreeVersion(UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetSystemSpanningTreeVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree mode.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *mode          -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetSystemSpanningTreeVersion(UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningForwardDelay(UI32_T *forward_delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetForwardDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the forward_delay time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *forward_delay -- pointer of the forward_delay value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetForwardDelay(UI32_T *forward_delay);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningHelloTime(UI32_T *hello_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the hello_time information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *hello_time      -- pointer of the hello_time value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetHelloTime(UI32_T *hello_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 *              3. Time unit is 1/100 sec
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMaxAge(UI32_T *max_age);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMaxAge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the max_age information.
 * INPUT    :   None
 * OUTPUT   :   UI32_T *max_age         -- pointer of the max_age value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMaxAge(UI32_T *max_age);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPathCostMethod
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPathCostMethod(UI32_T *pathcost_method);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPathCostMethod_Ex
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the default path cost calculation method.
 * INPUT    :   UI32_T  *pathcost_method  -- pointer of the method value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPathCostMethod_Ex(UI32_T *pathcost_method);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningTransmissionLimit(UI32_T *tx_hold_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTransmissionLimit
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the transmission limit count vlaue.
 * INPUT    :   None
 * OUTPUT   :   UI32_T  *tx_hold_count  -- pointer of the TXHoldCount value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetTransmissionLimit(UI32_T *tx_hold_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstPriority(UI32_T mstid, UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge priority information.
 * INPUT    :   None
 * OUTPUT   :   UI16_T  mstid            -- instance value
 *              UI32_T  *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstPriority(UI32_T mstid, UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstPortPriority(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port priority for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *priority        -- pointer of the priority value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * REF      :   RFC-1493/dot1dStpPortEntry 2
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstPortPriority(UI32_T lport,
                                   UI32_T mstid,
                                   UI32_T *priority);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRunningPortLinkTypeMode(UI32_T lport,
                                            UI32_T mstid,
                                            UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortLinkTypeMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port link_type mode of the port for the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortLinkTypeMode(UI32_T lport,
                                     UI32_T mstid,
                                     UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRunningPortProtocolMigration(UI32_T lport,
                                                 UI32_T mstid,
                                                 UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortProtocolMigration
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get protocol_migration status for a port in the
 *              specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortProtocolMigration(UI32_T lport,
                                          UI32_T mstid,
                                          UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_POM_GetRunningPortAdminEdgePort(UI32_T lport,
                                             UI32_T mstid,
                                             UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortAdminEdgePort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get edge_port status for a port for in specified spanning
 *              tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *mode            -- pointer of the mode value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetPortAdminEdgePort(UI32_T lport,
                                      UI32_T mstid,
                                      UI32_T *mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDot1dMstPortEntry(UI32_T mstid,
                                     XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextDot1dMstPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port entry is available, the
 *              port_entry->dot1d_stp_port will be updated and the entry
 *              info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextDot1dMstPortEntry(UI32_T *mstid,
                                         XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextPortMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              lport                       -- lport value
 * OUTPUT   :   lport                       -- next lport
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextPortMemberByInstance(UI32_T mstid, UI32_T *lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 *              UI32_T mstid                -- instance value
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDot1dMstPortEntryX(UI32_T mstid,
                                      XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextDot1dMstPortEntryX
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available base port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                       -- instance value
 *              port_entry->dot1d_stp_port  -- key to specify a unique
 *                                             port entry
 * OUTPUT   :   *port_entry                 -- pointer of the specified port
 *                                             entry info
 *              mstid                       -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1. If next available port entry is available, the
 *                 port_entry->dot1d_stp_port will be updated and the entry
 *                 info will be retrieved from the table.
 *              2. State is backing for a port with no link.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextDot1dMstPortEntryX(UI32_T *mstid,
                                          XSTP_MGR_Dot1dStpPortEntry_T *port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified mst port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDot1dMstExtPortEntry(UI32_T mstid,
                                        UI32_T lport,
                                        XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_POM_GetNextDot1dMstExtPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next available ext port
 *              entry info can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   *lport                          -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *ext_port_entry                 -- pointer of the specified
 *                                                 port ext_entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   If next available port ext_entry is available, the
 *              ext_port_entry->dot1d_stp_port will be updated and the
 *              entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextDot1dMstExtPortEntry(UI32_T mstid,
                                            UI32_T *lport,
                                            XSTP_MGR_Dot1dStpExtPortEntry_T *ext_port_entry);

/*-------------------------------------------------------------------------
 * The following Functions only provide for MSTP
 *-------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstpRevisionLevel(UI32_T *revision);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpRevisionLevel
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP revision level value.
 * INPUT    :
 * OUTPUT   :   U32_T *revision     -- pointer of the revision value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpRevisionLevel(UI32_T *revision);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstpMaxHop(UI32_T *hop_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpMaxHop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the MSTP Max_Hop count.
 * INPUT    :
 * OUTPUT   :   U32_T *hop_count              -- pointer of max_hop count
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpMaxHop(UI32_T *hop_count);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpConfigurationEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the configuration entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :
 * OUTPUT   :   *mstp_entry      -- pointer of the configuration entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpConfigurationEntry(XSTP_MGR_MstpEntry_T  *mstp_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanMapped(UI32_T mstid,
                                          XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of map VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanMappedForMSB(UI32_T mstid,
                                                XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_NextGetMstpInstanceVlanMapped
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanMapped(UI32_T *mstid,
                                              XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpInstanceVlanMappedForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Mapping_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanMappedForMSB(UI32_T *mstid,
                                                    XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanConfiguration(UI32_T mstid,
                                                 XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpInstanceVlanConfigurationForMSB(UI32_T mstid,
                                                       XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the entry info of set VLANs to
 *              a instance can be successfully retrieved. Otherwise, false
 *              is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   From Configuration_Table.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningMstpInstanceVlanConfiguration(UI32_T mstid,
                                                        XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpInstanceVlanConfiguration
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (LSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanConfiguration(UI32_T *mstid,
                                                     XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpInstanceVlanConfigurationForMSB
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next entry info of map
 *              VLANs to a instance can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   mstid                 -- instance value.
 * OUTPUT   :   *mstp_instance_entry  -- pointer of the config entry info
 *              mstid                 -- instance value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP.
 *              From Configuration_Table (MSB).
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpInstanceVlanConfigurationForMSB(UI32_T *mstid,
                                                           XSTP_MGR_MstpInstanceEntry_T *mstp_instance_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMstInstanceExisting
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              mapping table (active).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsMstInstanceExisting(UI32_T mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistedInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance(active) for mst mapping table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextExistedInstance(UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMstInstanceExistingInMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funcion returns true if the mst instance exist for mst
 *              config table(inactive).Otherwise, returns false.
 * INPUT    :   UI32_T mstid             -- the instance id
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_IsMstInstanceExistingInMstConfigTable(UI32_T mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextExistedInstanceForMstConfigTable
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next existed MST instance (inactive) for mst config
 *            table.
 * INPUT    : mstid     -- mstid pointer
 * OUTPUT   : mstid     -- next mstid pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the instance list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextExistedInstanceForMstConfigTable(UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port role in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   UI32_T  *role                -- the pointer of role value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortRole(UI32_T lport,
                               UI32_T mstid,
                               UI32_T *role);

 /*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port state in a specified spanning tree.
 * INPUT    :   UI32_T lport                 -- lport number
 *              UI32_T mstid                 -- instance value
 * OUTPUT   :   U32_T  *state                -- the pointer of state value
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- get successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- lport number out of range
 *              XSTP_TYPE_RETURN_INDEX_OOR   -- mstid  out of range
 * NOTES    :   none
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortState(UI32_T lport,
                                UI32_T mstid,
                                UI32_T *state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextVlanMemberByInstance
 * ------------------------------------------------------------------------
 * PURPOSE  : Get the next vlan member for a specified instance
 * INPUT    : mstid                 -- instance value
 *            vid       -- vlan id pointer
 * OUTPUT   : vid       -- next vlan id pointer
 * RETURN   : TRUE if OK, or FALSE if at the end of the member list
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_GetNextVlanMemberByInstance(UI32_T mstid, UI32_T *vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstidFromMstConfigurationTableByVlan
 * ------------------------------------------------------------------------
 * PURPOSE  : Get mstid value form mst configuration table for a specified
 *            vlan.
 * INPUT    : vid       -- vlan number
 *            mstid     -- mstid value point
 * OUTPUT   : mstid     -- mstid value point
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstidFromMstConfigurationTableByVlan(UI32_T *vid, UI32_T *mstid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_IsMemberPortOfInstanceEx
 *-------------------------------------------------------------------------
 * PURPOSE  : Check whether the specified lport is the member of this
 *            spanning tree instance
 * INPUT    : mstid     -- mstid value
 *            lport     -- lport
 * OUTPUT   : None
 * RETUEN   : TRUE if the specified vlan is the member of this instance, else
 *            FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_POM_IsMemberPortOfInstanceEx(UI32_T mstid, UI32_T lport);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetConfigDigest
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the configuration digest
 * INPUT    :   None
 * OUTPUT   :   config_digest           -- pointer of a 16 octet buffer for
 *                                         the configuration digest
 * RETURN   :   TRUE/FALSE
 * NOTES    :   Ref to the description in 13.7, IEEE 802.1s-2002
 * ------------------------------------------------------------------------
 */
BOOL_T    XSTP_POM_GetConfigDigest(UI8_T *config_digest);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetMstpRowStatus(UI32_T mstid, UI32_T *row_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetNextMstpRowStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if row status field of the entry can be
 *            get successfully.  Otherwise, return false.
 * INPUT    : mstid       -- instance value
 * OUTPUT   : row_status  -- VAL_dot1qVlanStaticRowStatus_active
 *                           VAL_dot1qVlanStaticRowStatus_notInService
 *                           VAL_dot1qVlanStaticRowStatus_notReady
 *                           VAL_dot1qVlanStaticRowStatus_createAndGo
 *                           VAL_dot1qVlanStaticRowStatus_createAndWait
 *                           VAL_dot1qVlanStaticRowStatus_destroy
 * RETURN   : TRUE \ FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetNextMstpRowStatus(UI32_T *mstid, UI32_T *row_status);

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortAdminPathCost(UI32_T lport, UI32_T *admin_path_cost);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to external_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortOperPathCost(UI32_T lport, UI32_T *oper_path_cost);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortAdminPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the admin path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *admin_path_cost -- admin path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   1. If the default Path Cost is being used, return '0'.
 *              2. It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortAdminPathCost(UI32_T lport,
                                        UI32_T mstid,
                                        UI32_T *admin_path_cost);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetMstPortOperPathCost
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the oper path_cost of the port for specified spanning tree.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T mstid            -- instance value
 * OUTPUT   :   UI32_T *oper_path_cost  -- oper path_cost value.
 * RETURN   :   XSTP_TYPE_RETURN_OK          -- set successfully
 *              XSTP_TYPE_RETURN_ERROR       -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *              XSTP_TYPE_RETURN_PORTNO_OOR  -- port number out of range
 * NOTE     :   It is equal to internal_port_path_cost for mstp
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetMstPortOperPathCost(UI32_T lport, UI32_T mstid, UI32_T *oper_path_cost);

/* per_port spanning tree : begin */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortSpanningTreeStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortSpanningTreeStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the spanning tree status of the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   For SNMP
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortSpanningTreeStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetDesignatedRoot(UI32_T mstid,
                                  XSTP_MGR_BridgeIdComponent_T *designated_root);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetBridgeIdComponent
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the bridge_id_component for specified instance.
 * INPUT    :   mstid                    -- instance value
 * OUTPUT   :   *designated_root         -- pointer of the specified
 *                                          bridge_id_component
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetBridgeIdComponent(UI32_T mstid,
                                     XSTP_MGR_BridgeIdComponent_T *bridge_id_component);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortDesignatedRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated root for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              mstid                           -- instance value
 * OUTPUT   :   *designated_root                -- pointer of the specified
 *                                                 designated_root
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortDesignatedRoot(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_BridgeIdComponent_T *designated_root);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortDesignatedBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated bridge for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_bridge              -- pointer of the specified
 *                                                 port designated_bridge
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortDesignatedBridge(UI32_T lport,
                                        UI32_T mstid,
                                        XSTP_MGR_BridgeIdComponent_T *designated_bridge);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the designated port for specified port and instance.
 * INPUT    :   lport                           -- lport number
 *              UI32_T mstid                    -- instance value
 * OUTPUT   :   *designated_port                -- pointer of the specified
 *                                                 designated_port
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortDesignatedPort(UI32_T lport,
                                      UI32_T mstid,
                                      XSTP_MGR_PortIdComponent_T *designated_port);

/* per_port spanning tree : end */

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- status value.
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortRootGuardStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get running root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 * OUTPUT   :   UI32_T *status          -- status value.
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     :   None.
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortRootGuardStatus(UI32_T lport, UI32_T *status);
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBpduGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU guard status on the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortBpduGuardStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBpduGuardStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get running BPDU Guard status on the specified port.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBpduGuardStatus(UI32_T lport, UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBPDUGuardAutoRecovery
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery status on the specified port.
 * INPUT   : lport -- lport number
 * OUTPUT  : status -- the status value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T  *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBPDUGuardAutoRecovery
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery status for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : status -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBPDUGuardAutoRecovery(UI32_T lport,
                                                    UI32_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBPDUGuardAutoRecoveryInterval
 * ------------------------------------------------------------------------
 * PURPOSE : Get the BPDU guard auto recovery interval on the specified
 *           port.
 * INPUT   : lport -- lport number
 * OUTPUT  : interval -- the interval value
 * RETURN  : XSTP_TYPE_RETURN_CODE_E
 * NOTE    : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                     UI32_T *interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBPDUGuardAutoRecoveryInterval
 *-------------------------------------------------------------------------
 * PURPOSE : Get running BPDU guard auto recovery interval for the specified
 *           port.
 * INPUT   : lport -- the logical port number
 * OUTPUT  : interval -- the status value
 * RETURN  : SYS_TYPE_Get_Running_Cfg_T
 * NOTES   : None
 *-------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBPDUGuardAutoRecoveryInterval(UI32_T lport,
                                                            UI32_T *interval);
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortBpduFilterStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU filter status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetPortBpduFilterStatus(UI32_T lport, UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortBpduFilterStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get per port variable bpdu_filter_status value.
 * INPUT    : lport     -- the logical port number
 * OUTPUT   : status    -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    : (interface function)
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortBpduFilterStatus(UI32_T lport, UI32_T *status);
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */


#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the cisco prestandard compatibility status
 * INPUT    :   UI32_T status           -- the status value
 * OUTPUT   :   UI32_T status           -- the status value
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_POM_GetCiscoPrestandardCompatibility(UI32_T *status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningCiscoPrestandardCompatibility
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the cisco prestandard compatibility status.
 * INPUT    : UI32_T status           -- the status value
 * OUTPUT   : UI32_T status           -- the status value
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTES    :
 *------------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningCiscoPrestandardCompatibility(UI32_T *status);
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get TC propage stop status
 * INPUT    :   UI32_T lport            -- lport number
 *              BOOL)T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   enum XSTP_TYPE_RETURN_CODE_E
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetPortTcPropStop(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetRunningPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get TC propage stop status
 * INPUT    :   UI32_T lport            -- lport number
 *              BOOL)T status           -- the status value
 * OUTPUT   : None
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS - need to store configuration
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - configure no change
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_POM_GetRunningPortTcPropStop(UI32_T lport, BOOL_T *status);
#endif /*end of #if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)*/

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTcPropGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get port list for specified group ID.
 * INPUT    : group_id     -- group ID
 * OUTPUT   : portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetTcPropGroupPortbitmap(UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                BOOL_T *has_port_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_POM_GetTcProplNextGroupPortbitmap
 * ------------------------------------------------------------------------
 * PURPOSE  : Get next group ID and port list.
 * INPUT    : group_id_p   -- group ID pointer
 * OUTPUT   : group_id_p   -- group ID pointer
 *            portbitmap   -- group member ports
 *            has_port_p   -- have port or not
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T XSTP_POM_GetTcPropNextGroupPortbitmap(UI32_T *group_id_p,
                                     UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
                                     BOOL_T *has_port_p);
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

#endif /* #ifndef XSTP_POM_H */

