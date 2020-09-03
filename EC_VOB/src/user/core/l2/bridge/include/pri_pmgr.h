/*-----------------------------------------------------------------------------
 * FILE NAME: PRI_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for PRI MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/25     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef PRI_PMGR_H
#define PRI_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "pri_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : PRI_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resources for PRI_PMGR in the calling process.
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
BOOL_T PRI_PMGR_InitiateProcessResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_SetDot1dPortDefaultUserPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the default user priority of the
 *            specific port can be set successfully.  Otherwise, return false.
 * INPUT    : lport_ifindex        -- the specified port number
 *            default_priority     -- the default priority for the port
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. lport_ifindex is the key to identify a port's default user
 *               priority.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_SetDot1dPortDefaultUserPriority(UI32_T lport_ifindex, UI32_T default_priority);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_SetDot1dPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the mapped traffic class of the
 *            priority list.  Otherwise, return false.
 * INPUT    : lport_ifindex -- PER PORT CONFIGURATION IS CURRENTLY NOT SUPPORTED.
 *            traffic_class -- the mapped traffic classs
 *            priority      -- user priority bit list
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the vaule will be determined by the port
 *               default user priority.
 *            2. The traffic_class will the traffic class mapped to the user
 *               priority of received frame.
 *            3. This function is device dependant. For those devices which
 *               do not support port traffic class configuration capability,
 *               this function will always return FALSE.
 *            4.This API is for CLI Only
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_SetDot1dPriority(UI32_T lport_ifindex, UI32_T traffic_class, UI8_T priority);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_SetDot1dPortNumTrafficClasses
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the number of priority supported
 *            by the specified port can be set.  otherwise, return false.
 * INPUT    : lport_ifindex            - the specified port number
 *            number_of_traffic_class - the number of priority queue
 * OUTPUT   : none
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The port_id is the key to identify the number of traffic
 *               classes of a given port.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_SetDot1dPortNumTrafficClasses(UI32_T lport_ifindex, UI32_T number_of_traffic_class);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_GetDot1dPortPriorityEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the port priority entry can be
 *            retrieve from database.  Otherwise, return false.
 * INPUT    : lport_ifindex      - Key to identify specific entry
 * OUTPUT   : priority_entry    - the priority entry which contains information
 *                                of specific port.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The lport_ifindex is the key to identify a port's default user
 *               priority entry.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_GetDot1dPortPriorityEntry(UI32_T lport_ifindex, PRI_MGR_Dot1dPortPriorityEntry_T *priority_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_GetNextDot1dPortPriorityEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available port priority
 *            can be retrieve from database.  Otherwise, return false.
 * INPUT    : lport_ifindex      - Key to identify specific entry
 * OUTPUT   : lport_ifindex      - Key to identify the next available entry
 *            priority_entry    - the priority entry which contains information
 *                                of specific port.
 * RETURN   : TRUE \ FALSE
 * NOTES    : 1. The lport_ifindex is the key to identify a port's default user
 *               priority entry.
 *            2. The user_priority defines the default ingress user priority
 *               assigned for untagged frame on this port.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_GetNextDot1dPortPriorityEntry(UI32_T *lport_ifindex, PRI_MGR_Dot1dPortPriorityEntry_T *priority_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_GetDot1dTrafficClassEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the traffic class of the specific user
 *            priority and lport_ifindex can be retrieve successfully.  Otherwise,
 *            returns false.
 * INPUT    : traffic_class_entry.lport_ifindex  - the primary key to identify a
 *                                                 port traffic class
 *            traffic_class_entry.dot1d_traffic_class_priority - the secondary key
 *                                                               to identify a port
 *                                                               traffic class
 * OUTPUT   : *traffic_class - the mapped traffic classs
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the value will be determined by the port
 *               default user priority.
 *            2. The traffic_class will be the traffic class mapped to the user
 *               priority of received frame.  Traffic class is a number in the
 *               range (0..(dot1dPortNumTrafficClasses-1)).
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_GetDot1dTrafficClassEntry(UI32_T lport_ifindex,
                                          UI32_T priority,
                                          PRI_MGR_Dot1dTrafficClassEntry_T *traffic_class_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_GetNextDot1dTrafficClassEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the next available traffic class of
 *            the specific user priority and lport_ifindex can be retrieve
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex  - the primary key to identify a port traffic class
 *            priority - the secondary key to identify a port traffic class
 * OUTPUT   : lport_ifindex  - the primary key to identify a port traffic class
 *            priority - the secondary key to identify a port traffic class
 *            *traffic_class_entry - the specific traffic class
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the value will be determined by the port
 *               default user priority.
 *            2. The traffic_class will be the traffic class mapped to the user
 *               priority of received frame.  Traffic class is a number in the
 *               range (0..(dot1dPortNumTrafficClasses-1)).
 *            3. This function is device dependant. For those devices which
 *               do not support port traffic class configuration capability,
 *               this function will always return FALSE.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_GetNextDot1dTrafficClassEntry(UI32_T *lport_ifindex,
                                              UI32_T *priority,
                                              PRI_MGR_Dot1dTrafficClassEntry_T *traffic_class_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_SetDot1dTrafficClass
 *--------------------------------------------------------------------------
 * PURPOSE  : Using to set the mapped traffic class of the user priority by
 *            the specified port.
 * INPUT    : lport_ifindex        -- the specified port number
 *            user_priority -- the user priority
 *            traffic_class -- the mapped traffic classs
 * OUTPUT   : none
 * RETURN   : TRUE  -- the value can set successfully
 *            FLASE -- the value can't set successfully
 * NOTES    : 1. The user_priority will be the same value of user priority
 *               of received frame.
 *               For tagged frame, the value will be determined by the tagging
 *               header.
 *               For untagged frame, the value will be determined by the port
 *               default user priority.
 *            2. The traffic_class will be the traffic class mapped to the user
 *               priority of received frame.  Traffic class is a number in the
 *               range (0..(dot1dPortNumTrafficClasses-1)).
 *            3. This function is device dependant. For those devices which
 *               do not support port traffic class configuration capability,
 *               this function will always return FALSE.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_SetDot1dTrafficClass(UI32_T lport_ifindex, UI32_T user_priority, UI32_T traffic_class);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_GetDot1dUserPriorityRegenEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the user regenrate priority of the
 *            specific user priority and lport_ifindex can be retrieve
 *            successfully. Otherwise, returns false.
 * INPUT    : lport_ifindex - the specific port information to be retrieve.
 *            user_regen_entry.dot1d_user_priority - the specific priority to
 *                                                   be retrieve.
 * OUTPUT   : user_regen_entry.dot1d_regen_user_priority - the regenrated user
 *                                                         priority for the
 *                                                         designated port and priority.
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. dot1d_user_priority is the User Priority for a frame received
 *               on this port.
 *            2. dot1d_regen_user_priority is the Regenerated User Priority
 *               the incoming User Priority is mapped to for this port.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_GetDot1dUserPriorityRegenEntry(UI32_T lport_ifindex,
                                               PRI_MGR_Dot1dUserPriorityRegenEntry_T *user_regen_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_GetNextDot1dUserPriorityRegenEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if next available user regenrate priority
 *            of the specific user priority and lport_ifindex can be retrieve
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex - the specific port information to be retrieve.
 * OUTPUT   : user_regen_entry.dot1d_user_priority - the specific priority to
 *                                                  be retrieve.
 *            user_regen_entry.dot1d_regen_user_priority - the regenrated user
 *                                                        priority for the
 *                                                        designated port and priority.
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. dot1d_user_priority is the User Priority for a frame received
 *               on this port.
 *            2. dot1d_regen_user_priority is the Regenerated User Priority
 *               the incoming User Priority is mapped to for this port.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_GetNextDot1dUserPriorityRegenEntry(UI32_T *lport_ifindex,
                                                   PRI_MGR_Dot1dUserPriorityRegenEntry_T *user_regen_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PRI_PMGR_SetDot1dUserPriorityRegenEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific user regenrated priority
 *            of the specific user priority and lport_ifindex can be set
 *            successfully.  Otherwise, returns false.
 * INPUT    : lport_ifindex - the specific port information to be retrieve.
 *            user_regen_entry.dot1d_user_priority - the specific priority to
 *                                                  be retrieve.
 *            user_regen_entry.dot1d_regen_user_priority - the regenrated user
 *                                                        priority for the
 *                                                        designated port and priority.
 * OUTPUT   : NONE
 * RETURN   : TRUE  \ FALSE
 * NOTES    : 1. dot1d_user_priority is the User Priority for a frame received
 *               on this port.
 *            2. dot1d_regen_user_priority is the Regenerated User Priority
 *               the incoming User Priority is mapped to for this port.
 *--------------------------------------------------------------------------
 */
BOOL_T PRI_PMGR_SetDot1dUserPriorityRegenEntry(UI32_T lport_ifindex,
                                               PRI_MGR_Dot1dUserPriorityRegenEntry_T user_regen_entry);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME  - PRI_PMGR_GetRunningPortPriorityParameters
 * ----------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific non-default user priority associated with lport_ifindex
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  lport_ifindex - the specific port number
 * OUTPUT: port_cfg  - structure which contains non-defulat priority value.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default user priority value
 * ----------------------------------------------------------------------------
 */
UI32_T PRI_PMGR_GetRunningPortPriorityParameters(UI32_T lport_ifindex, PRI_MGR_PortPriority_RunningCfg_T *port_cfg);

/* ----------------------------------------------------------------------------
 * FUNCTION NAME  - PRI_PMGR_GetNextRunningTrafficClassParameters
 * ----------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available traffic class associated with lport_ifindex
 *          can be retrieved successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  lport_ifindex - the specific port number
 *         traffic_class_cfg->dot1d_traffic_class_priority - specific prioriry mapped to
 *                                                           traffic class
 * OUTPUT: traffic_class_cfg  - structure which contains non-defulat traffic class value.
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default user priority value
 * ----------------------------------------------------------------------------
 */
UI32_T PRI_PMGR_GetNextRunningTrafficClassParameters(PRI_MGR_TrafficClass_RunningCfg_T *traffic_class_cfg);


#endif /* #ifndef PRI_PMGR_H */
