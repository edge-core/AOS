#ifndef SNMP_PMGR_H
#define SNMP_PMGR_H

#include "snmp_mgr.h"

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PMGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for SNMP_MGR.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void SNMP_PMGR_Init(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_SetSnmpCommunityStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  The funtion to set the Rfc2576 Community MIB status.
 * INPUT:    comm_string_name, sec_name
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SetSnmpCommunityStatus(UI8_T *comm_name, UI32_T row_status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_SetDefaultSnmpEngineID
 *---------------------------------------------------------------------------
 * PURPOSE: This function will set the SNMP engineID to default value.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  1. success: SNMP_MGR_ERROR_OK
 *          2. failure: SNMP_MGR_ERROR_EXCEED_LIMIT
 *                      SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                      SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                      SNMP_MGR_ERROR_FAIL
 * NOTE:    This function is called when use the no snmp-server engineID,
 *          it will restore the engineID to default.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SetDefaultSnmpEngineID();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetSnmpStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the SNMP status .
 * INPUT:    None.
 * OUTPUT:   snmpstats
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     1.For CLI only. 2.SNMP_STATS_T is defined in <envoy/h/snmpstats.h>
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetSnmpStatus( SNMP_MGR_STATS_T *snmpstats  );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_CreateSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE:  This function used to create a SNMP V3 Group
 * INPUT:    entry.
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:    1. if entry.snmpv3_group_model !=  SNMP_MGR_SNMPV3_MODEL_V3, then the agent will auto set
 *                     then entry.snmpv3_group_security_level == SNMP_MGR_SNMPV3_SECURITY_LEVEL_NOAUTH,
 *                 2. if no readview and write view is input, then the agent will auto set the readview name to "defaultview" and
 *                     write view name to NULL.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_CreateSnmpV3Group(SNMP_MGR_SnmpV3GroupEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_CreateSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call SNMP_MGR_CreateSnmpV3UserWithFlag()
 *           to create a SNMP V3 User.
 * INPUT:    entry.
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_EXCEED_LIMIT
 *                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                         SNMP_MGR_ERROR_FAIL
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_CreateSnmpV3User(SNMP_MGR_SnmpV3UserEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_CreateSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the transition mode.
 * INPUT:    entry.
 * OUTPUT:   None..
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:        The entry.snmpv3_view_subtree is in the ASCII from of an OID. and the mask is also in the ASCII from.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_CreateSnmpV3View(SNMP_MGR_SnmpV3ViewEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_DeleteSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete a SNMP V3 View
 * INPUT:    snmpv3_view_name[], snmpv3_view_subtree[]
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:          SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteSnmpV3View(UI8_T *snmpv3_view_name, UI8_T * snmpv3_wildcard_subtree);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_DeleteSnmpV3ViewByName
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete all the SNMP V3 View by a specific  view Name
 * INPUT:    snmpv3_view_name[]
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteSnmpV3ViewByName(UI8_T *snmpv3_view_name );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_DeleteSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete a SNMP V3 Group.
 * INPUT:    snmpv3_group_name[], security_model, security_level
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteSnmpV3Group(UI8_T *snmpv3_group_name, UI8_T  security_model ,  UI8_T security_level);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetEngineID
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Get the SNMP Engine ID, the Engine ID is fixed to 13 octets.
 * INPUT:    engineID.
 * OUTPUT:   engineID.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetEngineID(UI8_T *engineID, UI32_T *engineIDLen);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetRunningEngineID
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Get the SNMP Engine ID, the Engine ID is fixed to 13 octets.
 * INPUT:    engineID.
 * OUTPUT:   engineID.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_GetRunningEngineID ( UI8_T *engineID, UI32_T *engineIDLen);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetEngineBoots
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Get the SNMP Engine boots.
 * INPUT:    engineBoots.
 * OUTPUT:   engineBoots
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetEngineBoots ( UI32_T *engineBoots);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_DeleteSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete a SNMP V3 User
 * INPUT:    secourity_model, snmpv3_user_name[].
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_EXCEED_LIMIT
 *                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                         SNMP_MGR_ERROR_FAIL
 * NOTE:     1. None
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteSnmpV3User(SNMP_MGR_Snmpv3_Model_T  security_model,  UI8_T * snmpv3_user_name);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  The  Get  Function of Snmp V3 user/
 * INPUT:     entry->snmpv3_user_engine_id, entry->snmpv3_user_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetSnmpV3User(SNMP_MGR_SnmpV3UserEntry_T  *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of Snmp V3 user.
 * INPUT:        entry->snmpv3_user_engine_id, entry->snmpv3_user_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:        1.entry->snmpv3_user_engine_id and entry->snmpv3_user_name = "" to get the first entry.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_GetNextSnmpV3User(SNMP_MGR_SnmpV3UserEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextRunningSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Running Function of Snmp V3 user, to get the first entry with the key of 0.
 * INPUT:        entry->snmpv3_user_engine_id, entry->snmpv3_user_name
 * OUTPUT:   entry.
 * RETURN:   RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:      1.entry->snmpv3_user_engine_id and entry->snmpv3_user_name = "" to get the first entry.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_GetNextRunningSnmpV3User(SNMP_MGR_SnmpV3UserEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of SNMP V3 Group.
 * INPUT:    entry->snmpv3_group_name, entry->snmpv3_group_context_prefix, entry->snmpv3_group_model, entry->snmpv3_group_security_level.
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:    1.To get the first entry, input all the key to 0 or NULL
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextSnmpV3Group( SNMP_MGR_SnmpV3GroupEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextRunningSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext  Running Function of SNMP V3 Group.
 * INPUT:    entry->snmpv3_group_name, entry->snmpv3_group_context_prefix, entry->snmpv3_group_model, entry->snmpv3_group_security_level.
 * OUTPUT:   entry.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:      1.To get the first entry, input all the key to 0 or NULL
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_GetNextRunningSnmpV3Group( SNMP_MGR_SnmpV3GroupEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextSnmpV3ViewName
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of Snmp V3 View Name.
 * INPUT:   view_name,
 * OUTPUT:  view_name
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_EXCEED_LIMIT
 *                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                         SNMP_MGR_ERROR_FAIL
 * NOTE:     1. To get the first entry with the key of 0 or null.
 *           2. For Web only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextSnmpV3ViewName(UI8_T *view_name);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of Snmp V3 View
 * INPUT:      entry->snmpv3_view_name, entry->snmpv3_view_subtree
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     1. To get the first entry with the key of 0 or null.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextSnmpV3View(SNMP_MGR_SnmpV3ViewEntry_T  *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextRunningSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Ruuning Function of Snmp V3 View.
 * INPUT:      entry->snmpv3_view_name, entry->snmpv3_view_subtree
 * OUTPUT:   entry.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:     1. To get the first entry with the key of 0 or null.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_GetNextRunningSnmpV3View(SNMP_MGR_SnmpV3ViewEntry_T  *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_Set_EngineID
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Set the SNMP V3 EngineID
 * INPUT:    newengineID..
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     All uiser will delete when an new engineID is set..
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_Set_EngineID(UI8_T *newengineID, UI32_T engineIDLen);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetNextSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the next available SNMP community
 *          string can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community string
 * OUTPUT: comm_string                  - next available SNMP community info
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *        2. Any invalid(0) community string will be skip duing the GetNext operation.
 */
UI32_T SNMP_PMGR_GetNextSnmpCommunity(SNMP_MGR_SnmpCommunity_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetNextRunningSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default SNMP community can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community
 * OUTPUT: snmp_comm                  - next available non-default SNMP community
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default SNMP community.
 *        3. Community string name is an "ASCII Zero" string (char array ending with '\0').
 *        4. Any invalid(0) SNMP community will be skip during the GetNext operation.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextRunningSnmpCommunity(SNMP_MGR_SnmpCommunity_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_CreateSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if we  can  successfully
 *         create  the specified community string. Otherwise, false is returned.
 *
 * INPUT: comm_string_name  - (key) to specify a unique SNMP community string
 *        access_right      - the access level for this SNMP community
 * OUTPUT: None
 * RETURN: 1. success(SNMP_MGR_ERROR_OK),  2. failure (see snmp_mgr.h)
 * NOTES: 1. This function will create a new community string to the system
 *           if the specified comm_string_name does not exist, and total number
 *           of community string configured is less than
 *           SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING.
 *        2. This function will update the access right an existed community
 *           string if the specified comm_string_name existed already.
 *        3. False is returned if total number of community string configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING
 * ---------------------------------------------------------------------
 */
UI32_T  SNMP_PMGR_CreateSnmpCommunity(UI8_T *comm_string_name, UI32_T access_right);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_RemoveSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if we can successfully remove a Snmp community
 *
 * INPUT: comm_string_name  - (key) to specify a unique SNMP community string
 *
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will remove a snmp community.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_RemoveSnmpCommunity(UI8_T *comm_string_name);

/*---------------------------------------------------------------------------+
 * Routine Name : SNMP_PMGR_Enable_Snmp_Agent                                 +
 *---------------------------------------------------------------------------+
 * Purpose :      This function will enable the snmp agent                   +
 * Input    :     None                                                       +
 * Output   :     None                                                       +
 * Return   :     SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK                      +
 * Note     :     None                                                       +
 *---------------------------------------------------------------------------*/
UI32_T  SNMP_PMGR_Enable_Snmp_Agent(void);

/*---------------------------------------------------------------------------+
 * Routine Name : SNMP_PMGR_Disable_Snmp_Agent               +
 *---------------------------------------------------------------------------  +
 * Purpose :      This function will disable the snmp agent              +
 * Input    :     None                                                                        +
 * Output   :     None                                                                        +
 * Return   :     SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK                                                                       +
 * Note     :     None                                                                       +
 *---------------------------------------------------------------------------*/
UI32_T  SNMP_PMGR_Disable_Snmp_Agent(void);

/*---------------------------------------------------------------------------+
 * Routine Name : SNMP_PMGR_Get_AgentStatus                                   +
 *---------------------------------------------------------------------------+
 * Purpose :      This function will get the agent status                    +
 * Input    :     status                                                     +
 * Output   :     status                                                     +
 * Return   :     True/False                                                 +
 * Note     :     none                                                       +
 *---------------------------------------------------------------------------*/
BOOL_T SNMP_PMGR_Get_AgentStatus(BOOL_T *status);

/*-------------------------------------------------------------------------- +
 * Routine Name : SNMP_PMGR_GetRunningSnmpAgentStatus                         +
 *---------------------------------------------------------------------------+
 * Purpose  :     This function will get the agent status                    +
 * Input    :     status                                                     +
 * Output   :     status                                                     +
 * RETURN   :     SYS_TYPE_GET_RUNNING_CFG_SUCCESS,                          +
 *                SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or                     +
 *                SYS_TYPE_GET_RUNNING_CFG_FAIL                              +
 * Note     :     none                                                       +
 *---------------------------------------------------------------------------*/
UI32_T SNMP_PMGR_GetRunningSnmpAgentStatus(BOOL_T *status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetNextTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the next available trap receiver
 *          can be retrieved successfully. Otherwise, error code is returned.
 *
 * INPUT: entry->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: entry            - next available trap receiver info
 * RETURN: SNMP_MGR_ERROR_OK; 	SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_FAIL;
 * NOTES:1.This function will return trap receiver from the smallest ip addr.
 *       2.To get the first entry, input the key as 0.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetTrapReceiverByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the specified trap receiver
 *          can be retrieved successfully.
 *          Otherwise, false is returned.
 * INPUT: index of trap receiver(just a seq#) -> key
 * OUTPUT: trap_receiver            - trap receiver info
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. Status of each trap receiver is defined as following:
 *              - invalid(0): the entry of this trap receiver is deleted/purged
 *              - valid(1): this trap receiver is enabled
 *
 *           Set status to invalid(0) will delete/purge a trap receiver.
 *
 *           Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *
 *        4. The total number of trap receivers supported by the system
 *           is defined by SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *        5. By default, there is no trap receiver configued in the system.
 *        6. For any trap event raised by any subsystem, a SNMP Trap shall be
 *           sent to all of the enabled trap receivers.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetTrapReceiverByIndex(UI32_T trap_index, SNMP_MGR_TrapDestEntry_T *trap_receiver);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_SetTrapReceiverCommStringNameByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the community string name can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        trap_dest_community      - the SNMP community string for this trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified trap_dest_address does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           status of this new trap receiver will be set to disabled(2)
 *           by default.
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SetTrapReceiverCommStringNameByIndex(UI32_T trap_index, UI8_T *comm_string_name);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_SetTrapDestAddressByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the TrapDestProtocol can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        TrapDestAddress
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SetTrapDestAddressByIndex(UI32_T trap_index, UI32_T port, L_INET_AddrIp_T  addr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_SetTrapReceiverStatusByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the status can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        status                - the status for this trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SetTrapReceiverStatusByIndex(UI32_T trap_index, UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetNextRunningTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default trap receiver can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: entry->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: entry            - next available non-default trap receiver info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default trap receiver.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextRunningTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry);

/*----------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be retrieved successfully. Otherwise, error code is returned.
 *
 * INPUT: entry->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: entry            - next available trap receiver info
 * RETURN: SNMP_MGR_ERROR_OK; 	SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_FAIL;
 * NOTES:1.This function will return trap receiver from the smallest ip addr.
 *
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_DeleteTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be deleted successfully. Otherwise, error code is returned.
 *
 * INPUT:   ip_addr    - (key) to specify a unique trap receiver
 * OUTPUT:  none
 * RETURN:  SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTE:    none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteTrapReceiver(L_INET_AddrIp_T ip_addr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_DeleteTrapReceiverWithTargetAddrName
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be deleted successfully. Otherwise, error code is returned.
 *
 * INPUT:   ip_addr    - (key) to specify a unique trap receiver
 *          target_addr_name
 * OUTPUT:  none
 * RETURN:  SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTE:   none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteTrapReceiverWithTargetAddrName(L_INET_AddrIp_T ip_addr, UI8_T *target_name);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be created successfully. Otherwise, error code is returned.
 *
 * INPUT: entry    - (key) to specify a unique trap receiver
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *         2. error code is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 *
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SetTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry_p);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_SetTrapReceiverWithTargetAddressName
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be created successfully. Otherwise, error code is returned.
 *
 * INPUT: entry    - (key) to specify a unique trap receiver
 *        target_addr_name
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *         2. error code is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 *
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SetTrapReceiverWithTargetAddrName(SNMP_MGR_TrapDestEntry_T *entry, UI8_T *target_name);

/* Remote EngineID/User API*/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_CreateSnmpRemoteUser
 * ---------------------------------------------------------------------
 * PURPOSE: This function will Create the Snmp Remote User
 * INPUT : remote ip, entry
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_CreateSnmpRemoteUser(UI32_T remote_ip, SNMP_MGR_SnmpV3UserEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_DeleteSnmpRemoteUser
 * ---------------------------------------------------------------------
 * PURPOSE: This function will delete the Snmp Remote User
 * INPUT : entry
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteSnmpRemoteUser(SNMP_MGR_SnmpV3UserEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetSnmpRemoteUserEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get the Snmp Remote User entry
 * INPUT : user_entry
 * OUTPUT: user_entry
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetSnmpRemoteUserEntry (SNMP_MGR_SnmpV3UserEntry_T *user_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetNextSnmpRemoteUserEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will getnext the Snmp Remote User entry
 * INPUT : user_entry
 * OUTPUT: user_entry
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextSnmpRemoteUserEntry (SNMP_MGR_SnmpV3UserEntry_T *user_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetNextRunningSnmpRemoteUserEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will getnext  the Snmp Remote User entry for cli running config
 * INPUT : user_entry
 * OUTPUT: user_entry
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_FAIL/SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextRunningSnmpRemoteUserEntry (SNMP_MGR_SnmpV3UserEntry_T *user_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_CreateRemoteEngineID
 * ---------------------------------------------------------------------
 * PURPOSE: This function will create the remote engineID Entry
 * INPUT : user_entry
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_CreateRemoteEngineID(SNMP_MGR_SnmpRemoteEngineID_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_DeleteRemoteEngineID
 * ---------------------------------------------------------------------
 * PURPOSE: This function will delete the remote engineID Entry
 * INPUT : user_entry
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK
 * NOTES : remove the remote EngineID Entry and all users which engineID assoiate on it
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteRemoteEngineID(SNMP_MGR_SnmpRemoteEngineID_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_DeleteRemoteEngineIDEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will delete the remote engineID Entry
 * INPUT : user_entry
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK
 * NOTES : Only remove the remote EngineID Entry
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteRemoteEngineIDEntry(SNMP_MGR_SnmpRemoteEngineID_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_GetSnmpRemoteEngineIDEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get the remote engineID Entry
 * INPUT : entry
 * OUTPUT: entry
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetSnmpRemoteEngineIDEntry (SNMP_MGR_SnmpRemoteEngineID_T *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextSnmpRemoteEngineIDEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext function of the Remote EngineID entry.
 * INPUT:    entry.
 * OUTPUT:   entry
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextSnmpRemoteEngineIDEntry( SNMP_MGR_SnmpRemoteEngineID_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextRunningSnmpRemoteEngineIDEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Running Function of Remote EngineID Entry
 * INPUT:    entry.
 * OUTPUT:   entry
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL/SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextRunningSnmpRemoteEngineIDEntry( SNMP_MGR_SnmpRemoteEngineID_T *entry);

#ifdef NET_SNMP_INCLUDES_H /* workaround: avoid to include net-snmp */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextSnmpTargetAddrTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will retrieve the next snmpTargetAddrTable in Rfc3413 Target MIB
 * INPUT:    entry->snmp_target_addr_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_NO_ENTRY_EXIST
 *                         SNMP_MGR_ERROR_FAIL
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextSnmpTargetAddrTable(SNMP_MGR_SnmpTargetAddrEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetSnmpTargetAddrTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will retrieve the snmpTargetAddrTable in Rfc3413 Target MIB
 * INPUT:    entry->snmp_target_addr_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_NO_ENTRY_EXIST
 *                         SNMP_MGR_ERROR_FAIL
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetSnmpTargetAddrTable(SNMP_MGR_SnmpTargetAddrEntry_T *entry);
#endif

#if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_CreateSnmpNotifyFilterProfileTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will create snmpNotifyFilterProfileTable in Rfc3413 Notify MIB
 * INPUT:       entry
 * OUTPUT:    entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                2. failure:       SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                     SNMP_MGR_ERROR_FAIL
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_CreateSnmpNotifyFilterProfileTable(SNMP_MGR_SnmpNotifyFilterProfileEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_DeleteSnmpNotifyFilterProfileTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete the snmNotifyFilterProfileTable in Rfc3413 notify MIB
 * INPUT:    entry->snmp_notify_filter_profile_ip - (main key) to specify a taget host ip.
 * OUTPUT:   entry.
 * RETURN:   1. success:    SNMP_MGR_ERROR_OK
 *           2. failure:    SNMP_MGR_ERROR_NO_ENTRY_EXIST
 *                          SNMP_MGR_ERROR_FAIL
 *                          SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_DeleteSnmpNotifyFilterProfileTable(SNMP_MGR_SnmpNotifyFilterProfileEntry_T *entry);

BOOL_T NLM_PMGR_SetConfigLogAdminStatus(UI8_T *index, UI32_T admin_status);
BOOL_T NLM_PMGR_SetConfigLogFilterName(UI8_T *index, UI8_T *filter_name);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextSnmpNotifyFilterProfileTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will retrieve the next SnmpNotifyFilterProfileTable in Rfc3413 notify MIB
 * INPUT:       snmp_target_params_name
 * OUTPUT:    entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                2. failure:       SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                     SNMP_MGR_ERROR_FAIL
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextSnmpNotifyFilterProfileTable(SNMP_MGR_SnmpNotifyFilterProfileEntry_T *entry);

BOOL_T NLM_PMGR_GetConfigLogEntry(SNMP_MGR_NlmConfigLog_T *entry);
#endif

BOOL_T SNMP_PMGR_GetRmonAlarmTable(SNMP_MGR_RmonAlarmEntry_T *entry_p);
BOOL_T SNMP_PMGR_GetNextRmonAlarmTable(SNMP_MGR_RmonAlarmEntry_T *entry_p);
BOOL_T SNMP_PMGR_GetNextRmonStatisticsTableByLport(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T SNMP_PMGR_CreateRmonAlarmEntry(SNMP_MGR_RmonAlarmEntry_T *entry_p);
BOOL_T SNMP_PMGR_ModifyRmonAlarmEntry(SNMP_MGR_RmonAlarmEntry_T *entry_p);
BOOL_T SNMP_PMGR_DeleteRmonAlarmEntry(UI32_T index);
UI32_T SNMP_PMGR_GetNextRunningRmonAlarmTable(SNMP_MGR_RmonAlarmEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextRunningRmonAlarmDeletedDefaultEntry
 * ---------------------------------------------------------------------
 * PURPOSE : This function is used to get the default alarm entries
 *           that are not exist anymore.
 * INPUT   : entry_p
 * OUTPUT  : entry_p
 * RETURN  : One of SYSLOG_REMOTE_RETURN_VALUE_E.
 * NOTES   : If return value is SYS_TYPE_GET_RUNNING_CFG_SUCCESS, it
 *           means that this default entry (ID specified in entry_p->id)
 *           is deleted.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextRunningRmonAlarmDeletedDefaultEntry(SNMP_MGR_RmonAlarmEntry_T *entry_p);

BOOL_T SNMP_PMGR_GetRmonEventTable(SNMP_MGR_RmonEventEntry_T *entry_p);
BOOL_T SNMP_PMGR_GetNextRmonEventTable(SNMP_MGR_RmonEventEntry_T *entry_p);
BOOL_T SNMP_PMGR_CreateRmonEventEntry(SNMP_MGR_RmonEventEntry_T *entry_p);
BOOL_T SNMP_PMGR_ModifyRmonEventEntry(SNMP_MGR_RmonEventEntry_T *entry_p);
BOOL_T SNMP_PMGR_DeleteRmonEventEntry(UI32_T index);
UI32_T SNMP_PMGR_GetNextRunningRmonEventTable(SNMP_MGR_RmonEventEntry_T *entry_p);

BOOL_T SNMP_PMGR_GetRmonStatisticsTable(SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T SNMP_PMGR_GetNextRmonStatisticsTable(SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T SNMP_PMGR_CreateRmonStatisticsEntry(SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T SNMP_PMGR_ModifyRmonStatisticsEntry(SNMP_MGR_RmonStatisticsEntry_T *entry_p);
BOOL_T SNMP_PMGR_DeleteRmonStatisticsEntryByLport(UI32_T if_index, UI32_T index);
UI32_T SNMP_PMGR_GetNextRunningRmonStatisticsTableByLport(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextRunningRmonStatisticsDeletedDefaultEntry
 * ---------------------------------------------------------------------
 * PURPOSE : This function is used to get the default statistics entries
 *           that are not exist anymore.
 * INPUT   : entry_p
 * OUTPUT  : entry_p
 * RETURN  : One of SYSLOG_REMOTE_RETURN_VALUE_E.
 * NOTES   : If return value is SYS_TYPE_GET_RUNNING_CFG_SUCCESS, it
 *           means that this default entry (ID specified in entry_p->id)
 *           is deleted.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextRunningRmonStatisticsDeletedDefaultEntry(UI32_T lport, SNMP_MGR_RmonStatisticsEntry_T *entry_p);

BOOL_T SNMP_PMGR_GetRmonHistoryControlTable(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T SNMP_PMGR_GetNextRmonHistoryControlTable(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T SNMP_PMGR_GetNextRmonHistoryControlTableByLport(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T SNMP_PMGR_CreateRmonHistoryControlEntry(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T SNMP_PMGR_ModifyRmonHistoryControlEntry(SNMP_MGR_RmonHistoryControlEntry_T *entry_p);
BOOL_T SNMP_PMGR_DeleteRmonHistoryControlEntryByLport(UI32_T if_index, UI32_T index);
UI32_T SNMP_PMGR_GetNextRunningRmonHistoryControlTableByLport(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_GetNextRunningRmonHistoryControlDeletedDefaultEntryByLport
 * ---------------------------------------------------------------------
 * PURPOSE : This function is used to get the default history entries
 *           that are not exist anymore.
 * INPUT   : entry_p
 * OUTPUT  : entry_p
 * RETURN  : One of SYSLOG_REMOTE_RETURN_VALUE_E.
 * NOTES   : If return value is SYS_TYPE_GET_RUNNING_CFG_SUCCESS, it
 *           means that this default entry (ID specified in entry_p->id)
 *           is deleted.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_GetNextRunningRmonHistoryControlDeletedDefaultEntryByLport(UI32_T lport, SNMP_MGR_RmonHistoryControlEntry_T *entry_p);

BOOL_T SNMP_PMGR_GetNextRmonHistoryTableByControlIndex(SNMP_MGR_RmonHistoryEntry_T *entry_p);

/*------------------------------------------------------------------|
 * ROUTINE NAME - SNMP_PMGR_Packets1H                                    |
 * ROUTINE NAME - SNMP_PMGR_Octets1H                                     |
 * ROUTINE NAME - SNMP_MGR_BCasts1H                                     |
 * ROUTINE NAME - SNMP_MGR_MCasts1H                                     |
 * ROUTINE NAME - SNMP_MGR_Fragments1H                                  |
 * ROUTINE NAME - SNMP_MGR_Collisions1H                                 |
 * ROUTINE NAME - SNMP_MGR_Errors1H                                     |
 *------------------------------------------------------------------|
 * FUNCTION: RMON history for 1 hour                                |
 *                                                                  |
 * INPUT   : UI32_T logical_unit - which unit to get                |
 *           UI32_T port         - which port to get                |
 * OUTPUT  : UI32_T *value       - value                            |
 * RETURN  : BOOL_T True : Successfully, False : Failed             |
 *------------------------------------------------------------------*/
BOOL_T SNMP_PMGR_Packets1H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Octets1H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_BCasts1H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_MCasts1H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Fragments1H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Collisions1H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Errors1H (UI32_T unit, UI32_T port, UI32_T *value);

/*------------------------------------------------------------------|
 * ROUTINE NAME - SNMP_PMGR_Packets6H                                   |
 * ROUTINE NAME - SNMP_PMGR_Octets6H                                    |
 * ROUTINE NAME - SNMP_PMGR_BCasts6H                                    |
 * ROUTINE NAME - SNMP_PMGR_MCasts6H                                    |
 * ROUTINE NAME - SNMP_PMGR_Fragments6H                                 |
 * ROUTINE NAME - SNMP_PMGR_Collisions6H                                |
 * ROUTINE NAME - SNMP_PMGR_Errors6H                                    |
 *------------------------------------------------------------------|
 * FUNCTION: RMON history for 6 hours                              |
 *                                                                  |
 * INPUT   : UI32_T logical_unit - which unit to get                |
 *           UI32_T port         - which port to get                |
 * OUTPUT  : UI32_T *value       - value                            |
 * RETURN  : BOOL_T True : Successfully, False : Failed             |
 *------------------------------------------------------------------*/
BOOL_T SNMP_PMGR_Packets6H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Octets6H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_BCasts6H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_MCasts6H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Fragments6H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Collisions6H (UI32_T unit, UI32_T port, UI32_T *value);

BOOL_T SNMP_PMGR_Errors6H (UI32_T unit, UI32_T port, UI32_T *value);

#if(SYS_CPNT_SNMPV3_TARGET_NAME_STYLE == SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_SnmpNotifyCreate
 * ---------------------------------------------------------------------
 * PURPOSE: This function create v3 inform and trap by one function.
 * INPUT:target_addr_name,notify_type,
 *             user_name, dest_ip_addres, dest_port,
 *             remote_engineID,  remote_engineIDLen,
 *             password_from_config,  auth_protocol,
 *             auth_key_len,  authentication_password,
 *             priv_protocol, priv_key_len,
 *             privacy_password,retry_interval,
 *             retry_count
 * OUTPUT: SNMP_MGR_CONFIG_ERROR_E
 * RETURN: SNMP_MGR_ERROR_OK;
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_SET_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_GET_TRAP
 * NOTE:  target_addr_name , dest_ip_addres , ( remote_engineID + user_name ) is key
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SnmpNotifyCreate(UI8_T *target_addr_name,UI32_T notify_type,
            UI8_T *user_name, UI32_T dest_ip_addres, UI32_T dest_port,
            UI8_T *remote_engineID, UI32_T  remote_engineIDLen,
            BOOL_T password_from_config,  UI32_T auth_protocol,
            UI32_T auth_key_len, UI8_T * authentication_password,
            UI32_T  priv_protocol, UI32_T priv_key_len,
            UI8_T *privacy_password,UI32_T retry_interval,
            UI32_T retry_count );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_SnmpNotifyDel
 * ---------------------------------------------------------------------
 * PURPOSE: This function del v3 inform and trap by one function.
 * INPUT:target_addr_name
 * OUTPUT: SNMP_MGR_CONFIG_ERROR_E
 * RETURN: SNMP_MGR_ERROR_OK;
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_SET_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_GET_TRAP
 * NOTE:  target_addr_name , dest_ip_addres , ( remote_engineID + user_name ) is key
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SnmpNotifyDel(UI8_T *snmp_target_addr_name );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_PMGR_SnmpNotifyModify
 * ---------------------------------------------------------------------
 * PURPOSE: This function modify v3 inform and trap by one function.
 * INPUT:target_addr_name,notify_type,
 *             user_name, dest_ip_addres, dest_port,
 *             remote_engineID,  remote_engineIDLen,
 *             password_from_config,  auth_protocol,
 *             auth_key_len,  authentication_password,
 *             priv_protocol, priv_key_len,
 *             privacy_password,retry_interval,
 *             retry_count
 * OUTPUT: SNMP_MGR_CONFIG_ERROR_E
 * RETURN: SNMP_MGR_ERROR_OK;
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_SET_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_GET_TRAP
 * NOTE:  target_addr_name , dest_ip_addres , ( remote_engineID + user_name ) is key
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_PMGR_SnmpNotifyModify(UI8_T *target_addr_name,UI32_T notify_type,
            UI8_T *user_name, UI32_T dest_ip_addres, UI32_T dest_port,
            UI8_T *remote_engineID, UI32_T  remote_engineIDLen,
            BOOL_T password_from_config,  UI32_T auth_protocol,
            UI32_T auth_key_len, UI8_T * authentication_password,
            UI32_T  priv_protocol, UI32_T priv_key_len,
            UI8_T *privacy_password,UI32_T retry_interval,
            UI32_T retry_count );

#endif /* (SYS_CPNT_SNMPV3_TARGET_NAME_STYLE == SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM)            */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_PMGR_GetSnmpEnableAuthenTraps
 * ---------------------------------------------------------------------
 *  FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           is successfully retrieved.  Otherwise, return false.
 * INPUT   : None.
 * OUTPUT  : snmp_enable_authen_traps - VAL_snmpEnableAuthenTraps_enabled /
 *                                      VAL_snmpEnableAuthenTraps_disabled
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_PMGR_GetSnmpEnableAuthenTraps(UI8_T *snmp_enable_authen_traps);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_PMGR_SetSnmpEnableAuthenTraps
 * ---------------------------------------------------------------------
 * FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           can be successfully configured.  Otherwise, return false.
 * INPUT   : snmp_enable_authen_traps - VAL_snmpEnableAuthenTraps_enabled /
 *                                      VAL_snmpEnableAuthenTraps_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_PMGR_SetSnmpEnableAuthenTraps(UI8_T snmp_enable_authen_traps);
/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_PMGR_SetSnmpEnableLinkUpDownTraps
 * ---------------------------------------------------------------------
 * FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           can be successfully configured.  Otherwise, return false.
 * INPUT   : link_up_down_traps - VAL_ifLinkUpDownTrapEnable_enabled /
 *                                      VAL_ifLinkUpDownTrapEnable_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_PMGR_SetSnmpEnableLinkUpDownTraps(UI8_T link_up_down_traps);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_PMGR_ReqSendTrap
 * ---------------------------------------------------------------------
 *  FUNCTION: Request trap manager for send trap.
 *
 *  INPUT    : Variable's instance and value that should be bound in
 *             trap PDU.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : This procedure shall not be invoked before SNMP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */
void SNMP_PMGR_ReqSendTrap(TRAP_EVENT_TrapData_T *trap_data_p);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_PMGR_ReqSendTrapOptional
 * ---------------------------------------------------------------------
 *  FUNCTION: Request trap manager for send trap.
 *
 *  INPUT    : 1. trap_data_p: Variable's instance and value that should be bound in
 *                                                    trap PDU.
 *             2. flag    1:send trap and log; 0:log only, don't send trap
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : This procedure shall not be invoked before SNMP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */
void SNMP_PMGR_ReqSendTrapOptional(TRAP_EVENT_TrapData_T *trap_data_p, TRAP_EVENT_SendTrapOption_E flag);

/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_PMGR_NotifyStaTplgChanged
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that network topology is changed due to the
 *          STA enabled.
 * Parameter:
 * Return: None.
 * Note: When STA enabled, all the ports will go through STA algorithm to
 *       determine its operation state. During this period, the trap management
 *       shall wait until STA becomes stable. Otherwise, the trap message
 *       will be lost if the port is not in forwarding state.
 * -----------------------------------------------------------------------------
 */
void SNMP_PMGR_NotifyStaTplgChanged (void);

/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_PMGR_NotifyStaTplgStabled
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that STA has been enabled, and at least one of the port enters
 *          forwarding state. The network topology shall be stabled after couple seconds.
 * Parameter:
 * Return: None.
 * Note: This notification only informs that at least one of STA port enters forwarding state.
 *       To make sure all the STA ports enters stable state, we shall wait for few more seconds
 *       before we can send trap messages.
 * -----------------------------------------------------------------------------
 */
void SNMP_PMGR_NotifyStaTplgStabled (void);

#if(SYS_CPNT_CLUSTER==TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_PMGR_SetClusterRole
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is set snmp next cluster role.
 * INPUT:    role
 * OUTPUT:
 * RETURN:
 * NOTE:
 *---------------------------------------------------------------------------
 */
void SNMP_PMGR_SetClusterRole (UI32_T role);
#endif /*end of #if(SYS_CPNT_CLUSTER==TRUE)*/

#endif /*#ifndef SNMP_PMGR_H*/

