/*-----------------------------------------------------------------------------
 * FILE NAME: LACP_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for LACP MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/21     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef LACP_PMGR_H
#define LACP_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "lacp_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : LACP_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for LACP_PMGR in the calling process.
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
BOOL_T LACP_PMGR_InitiateProcessResource(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified Agg entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   agg_entry->dot3ad_agg_index
 * OUTPUT   :   agg_entry                  -- aggregator entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggEntry(LACP_MGR_Dot3adAggEntry_T *agg_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port list entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortListEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next aggregator index of
 *              the specified Agg entry info can be successfully retrieved.
 *              Otherwise, false is returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortListEntry(LACP_MGR_Dot3adAggPortListEntry_T *agg_port_list_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   agg_port_entry
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortEntry(LACP_MGR_Dot3adAggPortEntry_T *agg_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortStatsEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port stats entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortStatsEntry(LACP_MGR_Dot3adAggPortStatsEntry_T *agg_port_stats_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adAggPortDebugEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port debug entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adAggPortDebugEntry(LACP_MGR_Dot3adAggPortDebugEntry_T *agg_port_debug_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adLagMibObjects
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified lag mib objects
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   base_entry                  -- base entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adLagMibObjects(LACP_MGR_LagMibObjects_T *lag_mib_objects);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adLacpPortEnabled
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set LACP on port to be enable or disable.
 * INPUT    :   UI32_T lacp_state         -- VAL_lacpPortStatus_enabled or VAL_lacpPortStatus_disabled (defined in Leaf_es3626a.h)
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_OK           -- set successfully
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adLacpPortEnabled(UI32_T ifindex, UI32_T lacp_state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetNextDot3adLacpPortEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the next lacp port entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   None
 * OUTPUT   :   lacp_port_entry -- port entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
BOOL_T LACP_PMGR_GetNextDot3adLacpPortEntry(LACP_MGR_Dot3adLacpPortEntry_T *lacp_port_entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_system_priority information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  priority    -- the dot3ad_agg_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggActorSystemPriority(UI16_T agg_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI16_T  admin_key   -- the dot3ad_agg_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggActorAdminKey(UI16_T agg_index, UI16_T admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDefaultDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_actor_admin_key information to default.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDefaultDot3adAggActorAdminKey(UI16_T agg_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_actor_admin_key information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggActorAdminKey(UI16_T agg_index, UI16_T *admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggCollectorMaxDelay
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_collector_max_delay information.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  max_delay   -- the dot3ad_agg_collector_max_delay value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggCollectorMaxDelay(UI16_T agg_index, UI32_T max_delay);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorSystemPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_actor_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_actor_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorAdminKey(UI16_T port_index, UI16_T *admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDefaultDot3adAggPortActorAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Restore the dot3ad_agg_port_actor_admin_key to default value.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDefaultDot3adAggPortActorAdminKey(UI16_T port_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorOperKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_oper_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  oper_key    -- the dot3ad_agg_port_actor_oper_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorOperKey(UI16_T port_index, UI16_T oper_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_system_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortPartnerAdminSystemPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_system_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_system_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortPartnerAdminSystemPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminSystemId
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_system_id information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   *system_id  -- the dot3ad_agg_port_partner_admin_system_id value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminSystemId(UI16_T port_index, UI8_T *system_id);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_key   -- the dot3ad_agg_port_partner_admin_key value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortPartnerAdminKey
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_key information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *admin_key  -- the dot3ad_agg_port_partner_admin_key value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortPartnerAdminKey(UI16_T port_index, UI16_T *admin_key);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_actor_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorPortPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminPort
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  admin_port  -- the dot3ad_agg_port_partner_admin_port value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminPort(UI16_T port_index, UI16_T admin_port);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI16_T  priority    -- the dot3ad_agg_port_partner_admin_port_priority value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortPartnerAdminPortPriority
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_partner_admin_port_priority information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_partner_admin_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortPartnerAdminPortPriority(UI16_T port_index, UI16_T *priority);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_actor_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_actor_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortActorAdminState(UI16_T port_index, UI8_T admin_state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortPartnerAdminState
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the dot3ad_agg_port_partner_admin_state information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 *              UI8_T   admin_state -- the dot3ad_agg_port_partner_admin_state value
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggPortPartnerAdminState(UI16_T port_index, UI8_T admin_state);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_PMGR_SetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the port with long term or short term timeout
 * INPUT    :   UI32_T  unit  -- the unit number
 *              UI32_T  port  -- ther port number
 *              UI32_T  timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_PMGR_GetDot3adAggPortActorLACP_Timeout(UI32_T unit, UI32_T port, UI32_T *timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggPortActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_port_lacp_timeout information.
 * INPUT    :   UI16_T  port_index  -- the dot3ad_agg_port_index
 * OUTPUT   :   UI16_T  *priority   -- the dot3ad_agg_port_actor_port_priority value
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggPortActorLACP_Timeout(UI16_T port_index, UI32_T *timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_SetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   set the lacp port channel with long term or short term timeout.
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  timeout   -- long or short term timeout
 * OUTPUT   :   None
 * RETURN   :   LACP_RETURN_SUCCESS/LACP_RETURN_ERROR
 * NOTE     :   None
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_SetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   get the lacp port channel with long term or short term timeout
 * INPUT    :   UI16_T  agg_index   -- the dot3ad_agg_index
 *              UI32_T  *timeout -- long or short term timeout
 * RETURN   :   LACP_RETURN_SUCCESS - success
 *              LACP_RETURN_ERROR - fail
 * NOTES    :
 * ------------------------------------------------------------------------
 */
UI32_T LACP_PMGR_GetDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LACP_PMGR_GetRunningDot3adAggActorLACP_Timeout
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the dot3ad_agg_port_actor_lacp_timeout information.
 * INPUT    :   UI16_T  agg_index  -- the dot3ad_agg_index
 * OUTPUT   :   UI16_T  *timeout   -- long or short term timeout
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * ------------------------------------------------------------------------
 */
UI32_T  LACP_PMGR_GetRunningDot3adAggActorLACP_Timeout(UI16_T agg_index, UI32_T *timeout);

#endif /* #ifndef LACP_PMGR_H */
