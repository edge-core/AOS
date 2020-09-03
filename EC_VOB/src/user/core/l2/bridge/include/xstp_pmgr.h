/*-----------------------------------------------------------------------------
 * FILE NAME: XSTP_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for XSTP MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/04     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


#ifndef XSTP_PMGR_H
#define XSTP_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "xstp_mgr.h"
#include "xstp_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : XSTP_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for XSTP_PMGR in the calling process.
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
BOOL_T XSTP_PMGR_InitiateProcessResource(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetChangeStatePortListForbidden
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the flag which controls whether the
 *            XSTP_MGR_ChangeStatePortList is allowed to be added new
 *            element.
 * INPUT    : flag    -- TRUE:disallowed, FALSE:allowed
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_PMGR_SetChangeStatePortListForbidden(BOOL_T flag);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemSpanningTreeStatus
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the global spanning tree status.
 *
 * INPUT   : status -- the status value
 *                     VAL_xstpSystemStatus_enabled
 *                     VAL_xstpSystemStatus_disabled
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemSpanningTreeStatus(UI32_T status);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemSpanningTreeVersion
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the spanning tree mode.
 *
 * OUTPUT  : mode -- the mode value
 *                   VAL_dot1dStpVersion_stpCompatible(0)
 *                   VAL_dot1dStpVersion_rstp(2)
 *                   VAL_dot1dStpVersion_mstp(3)
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *
 * NOTE    : Default -- SYS_DFLT_STP_PROTOCOL_TYPE
 *           Can't set mode when the status is disabled.
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStp 16
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemSpanningTreeVersion(UI32_T mode);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetForwardDelay
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the forward_delay time information.
 *
 * INPUT   : forward_delay -- the forward_delay value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- forward_delay out of range
 *
 * NOTES   : 1. Time unit is 1/100 sec
 *           2. Range
 *              -- XSTP_TYPE_MIN_FORWARD_DELAY
 *              -- XSTP_TYPE_MAX_FORWARD_DELAY
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_FORWARD_DELAY
 *
 * REF     : RFC-1493/dot1dStp 14
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetForwardDelay(UI32_T forward_delay);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetHelloTime
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the hello_time information.
 *
 * INPUT   : hello_time -- the hello_time value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- hello_time out of range
 *
 * NOTES   : 1. Time unit is 1/100 sec
 *           2. Range
 *              -- XSTP_TYPE_MIN_HELLO_TIME
 *              -- XSTP_TYPE_MAX_HELLO_TIME
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_HELLO_TIME
 *
 * REF     : RFC-1493/dot1dStp 13
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetHelloTime(UI32_T hello_time);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMaxAge
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the max_age information.
 *
 * INPUT   : max_age -- the max_age value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- max_age out of range
 *
 * NOTES   : 1. Time unit is 1/100 sec
 *           2. Range
 *              -- XSTP_TYPE_MIN_MAXAGE
 *              -- XSTP_TYPE_MAX_MAXAGE
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_MAX_AGE
 *
 * REF     : RFC-1493/dot1dStp 12
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMaxAge(UI32_T max_age);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPathCostMethod
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the default path cost calculation method.
 *
 * INPUT   : pathcost_method -- the method value
 *                              VAL_dot1dStpPathCostDefault_stp8021d1998(1)
 *                              VAL_dot1dStpPathCostDefault_stp8021t2001(2)
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *
 * NOTES   : 1. Long
 *              -- 32-bit based values for default port path costs.
 *              -- VAL_dot1dStpPathCostDefault_stp8021t2001
 *           2. Short
 *              -- 16-bit based values for default port path costs.
 *              -- VAL_dot1dStpPathCostDefault_stp8021d1998
 *           3. Default
 *              -- The long method.
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStp 18
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPathCostMethod(UI32_T pathcost_method);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetTransmissionLimit
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the transmission limit count vlaue.
 *
 * INPUT   : tx_hold_count -- the TXHoldCount value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- tx_hold_count out of range
 *
 * NOTES   : 1. The value used by the Port Transmit state machine to
 *              limit the maximum transmission rate.
 *           2. Range
 *              -- XSTP_TYPE_MIN_TX_HOLD_COUNT
 *              -- XSTP_TYPE_MAX_TX_HOLD_COUNT
 *           3. Default
 *              -- XSTP_TYPE_DEFAULT_TX_HOLD_COUNT
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStp 17
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetTransmissionLimit(UI32_T tx_hold_count);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemGlobalPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the global priority value when the switch is MST mode.
 *
 * INPUT   : priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- priority out of range
 *
 * NOTES   : 1. When mode is STP or RSTP
 *              -- Only set the priority value for mstid = 0.
 *           2. When mode is MSTP
 *              -- Set the priority value for all MST instances.
 *           3. Range : 0 ~ 61440 (in steps of 4096)
 *                      XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                      XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *           4. DEFAULT : 32768
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 *
 * REF     : RFC-1493/dot1dStp 2
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemGlobalPriority(UI32_T priority);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetSystemBridgePriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the bridge priority value.
 *
 * INPUT   : priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- priority out of range
 *
 * NOTES   : 1. Only set the priority value for mstid = 0.
 *           2. Range : 0 ~ 61440 (in steps of 4096)
 *                      XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *                      XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *           3. DEFAULT : 32768
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 *
 * REF     : RFC-1493/dot1dStp 2
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetSystemBridgePriority(UI32_T priority);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the path_cost of the port.
 *
 * INPUT   : lport     -- lport number
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : 1. When mode is STP or RSTP
 *              -- For mstid = 0
 *                 Set the path cost value to the specified port.
 *           2. When mode is MSTP
 *              -- For all instances
 *                 Set the path cost value to the specified port.
 *           3. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *              -- Range : 0 ~ 200000000
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_32
 *           4. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *              -- Range : 0 ~ 65535
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_16
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortPathCost(UI32_T lport, UI32_T path_cost);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the port priority value.
 *
 * INPUT   : lport    -- lport number
 *           mstid    -- instance value
 *           priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- lport number out of range
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- priority out of range
 *
 * NOTES   : 1. When mode is STP or RSTP
 *              -- For mstid = 0
 *                 Set the priority value to the specified port.
 *           2. When mode is MSTP
 *              -- For all instances
 *                 Set the priority value to the specified port.
 *           3. Range : 0 ~ 240 (in steps of 16)
 *                      XSTP_TYPE_MIN_PORT_PRIORITY
 *                      XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP
 *           4. Default : 128
 *              -- XSTP_TYPE_DEFAULT_PORT_PRIORITY
 *
 * REF     : RFC-1493/dot1dStpPortEntry 2
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortPriority(UI32_T lport, UI32_T priority);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortLinkTypeMode
 * ----------------------------------------------------------------------------
 * PURPOSE : Set a link type for a port when mode is RSTP or MSTP.
 *
 * INPUT   : lport -- lport number
 *           mode  -- the status value
 *                    VAL_dot1dStpPortAdminPointToPoint_forceTrue(0)
 *                    VAL_dot1dStpPortAdminPointToPoint_forceFalse(1)
 *                    VAL_dot1dStpPortAdminPointToPoint_auto(2)
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK              -- set successfully
 *           XSTP_TYPE_RETURN_ERROR           -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR       -- mstid out of range
 *           XSTP_TYPE_RETURN_PORTNO_OOR      -- port number out of range
 *
 * NOTES   : Default value
 *           -- VAL_dot1dStpPortAdminPointToPoint_auto
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 4
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortLinkTypeMode(UI32_T lport, UI32_T mode);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortProtocolMigration
 * ----------------------------------------------------------------------------
 * PURPOSE : Set protocol_migration status for a port.
 *
 * INPUT   : lport -- lport number
 *           mode  -- the mode value
 *                    VAL_dot1dStpPortProtocolMigration_true
 *                    VAL_dot1dStpPortProtocolMigration_false
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid out of range
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : Default value
 *           -- FALSE
 *              XSTP_TYPE_DEFAULT_PORT_PROTOCOL_MIGRATION_STATUS
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 1
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortProtocolMigration(UI32_T lport, UI32_T mode);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortAdminEdgePort
 * ----------------------------------------------------------------------------
 * PURPOSE : Set edge_port status for a port.
 *
 * INPUT   : lport -- lport number
 *           mode  -- the mode value
 *                    VAL_dot1dStpPortAdminEdgePort_true
 *                    VAL_dot1dStpPortAdminEdgePort_false
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid out of range
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : Default value
 *           -- FALSE
 *              XSTP_TYPE_DEFAULT_PORT_ADMIN_EDGE_PORT
 *
 * REF     : draft-ietf-bridge-rstpmib-02/dot1dStpExtPortEntry 1
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortAdminEdgePort(UI32_T lport, UI32_T mode);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the priority of the specified MST instance
 *           when the switch is MST mode.
 *
 * INPUT   : priority -- the priority value
 *           mstid    -- instance value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : 1. Range : 0 ~ 61440 (in steps of 4096)
 *              -- XSTP_TYPE_MIN_BRIDGE_PRIORITY
 *              -- XSTP_TYPE_MAX_BRIDGE_PRIORITY_RSTP_MSTP
 *           2. DEFAULT : 32768
 *              -- XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPriority(UI32_T mstid, UI32_T priority);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortPriority
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the port priority for the specified spanning tree
 *           when mode is MSTP.
 *
 * INPUT   : lport    -- lport number
 *           mstid    -- instance value
 *           priority -- the priority value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- lport number out of range
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : 1. Range : 0 ~ 240 (in steps of 16)
 *              -- XSTP_TYPE_MIN_PORT_PRIORITY
 *              -- XSTP_TYPE_MAX_PORT_PRIORITY_RSTP_MSTP
 *           2. Default : 128
 *              -- XSTP_TYPE_DEFAULT_PORT_PRIORITY
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortPriority(UI32_T lport,
                                    UI32_T mstid,
                                    UI32_T priority);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the path_cost of the port for specified spanning tree
 *           when mode is MSTP.
 * INPUT   : lport     -- lport number
 *           mstid     -- instance value
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURNPORTNO_OOR         -- port number out of range
 *           XSTP_TYPE_RETURNINDEX_OOR          -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : 1. In long mode (VAL_dot1dStpPathCostDefault_stp8021t2001)
 *              -- Range : 0 ~ 200000000
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_32
 *           2. In short mode (VAL_dot1dStpPathCostDefault_stp8021d1998)
 *              -- Range : 0 ~ 65535
 *                 XSTP_TYPE_MIN_PORT_PATH_COST
 *                 XSTP_TYPE_MAX_PORT_PATH_COST_16
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortPathCost(UI32_T lport,
                                    UI32_T mstid,
                                    UI32_T path_cost);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_AttachVlanListToMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Attach the vlan(s) to the new instance.
 *
 * INPUT   : mstid     -- instance value
 *           range     -- range value
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_1K
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_2K
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_3K
 *                        XSTP_MSTP_SET_INSTANCE_VLANS_MAPPED_4K
 *           vlan_list -- pointer of vlan_list
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *           XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 *
 * NOTES   : 1. All vlans will join MST instance 0 by default.
 *           2. This API will automatically move this VLAN from old instance
 *              to new instance.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_AttachVlanListToMstConfigTable(UI32_T mstid,
                                                UI32_T range,
                                                UI8_T *vlan_list);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetVlanToMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Map vlan to an instance for mst configuration table.
 *
 * INPUT   : mstid -- instance value
 *           vlan  -- vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *           XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 *
 * NOTES   : 1. All vlans will join MST instance 0 by default.
 *           2. This function is for the vlan which is attached to the
 *              MST instance 0.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetVlanToMstConfigTable(UI32_T mstid, UI32_T vlan);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_AttachVlanToMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Attach the vlan to the new instance.
 *
 * INPUT   : mstid -- instance value
 *           vlan  -- vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED -- status is disabled
 *           XSTP_TYPE_RETURN_INDEX_OOR          -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX          -- vlan not existed
 *
 * NOTES   : 1. All vlans will join MST instance 0 by default.
 *           2. This API will automatically move this VLAN from old instance
 *              to new instance.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_AttachVlanToMstConfigTable(UI32_T mstid, UI32_T vlan);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_RemoveVlanFromMstConfigTable
 * ----------------------------------------------------------------------------
 * PURPOSE : Remove vlan from an instance for mst configuration table.
 *
 * INPUT   : mstid -- instance value
 *           vlan  -- vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_INDEX_OOR         -- mstid out of range
 *           XSTP_TYPE_RETURN_INDEX_NEX         -- vlan not existed
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_RemoveVlanFromMstConfigTable(UI32_T mstid, UI32_T vlan);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstpConfigurationName
 * ----------------------------------------------------------------------------
 * PURPOSE : Set MSTP configurstion name.
 *
 * INPUT   : config_name -- pointer of the config_name
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *
 * NOTES   : Default : the bridage address
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstpConfigurationName(UI8_T *config_name);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstpRevisionLevel
 * ----------------------------------------------------------------------------
 * PURPOSE : Set MSTP revision level value.
 *
 * INPUT   : revision -- revision value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *
 * NOTES   : Default : 0
 *           -- XSTP_TYPE_DEFAULT_CONFIG_REVISION
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstpRevisionLevel(UI32_T revision);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstpMaxHop
 * ----------------------------------------------------------------------------
 * PURPOSE :  Set MSTP Max_Hop count.
 *
 * INPUT   :  hop_count -- max_hop value
 *
 * OUTPUT  :  None.
 *
 * RETURN  :  XSTP_TYPE_RETURN_OK                -- set successfully
 *            XSTP_TYPE_RETURN_ERROR             -- failed
 *            XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *            XSTP_TYPE_RETURN_INDEX_OOR         -- hop_count out of range
 *
 * NOTES   :  Range   : 1 ~ 40
 *            -- XSTP_TYPE_MSTP_MIN_MAXHOP
 *            -- XSTP_TYPE_MSTP_MAX_MAXHOP
 *            Default : 20
 *            -- XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstpMaxHop(UI32_T hop_count);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_RestartStateMachine
 *-----------------------------------------------------------------------------
 * PURPOSE : If restart_state_machine flag is TRUE, retart state machine.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETUEN  : None.
 *
 * NOTES   : Call the function when user leave the spa mst config mode.
 * ----------------------------------------------------------------------------
 */
void XSTP_PMGR_RestartStateMachine(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortAutoPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Restore the default internal path_cost of the port for a instance.
 *
 * INPUT   : lport -- lport number
 *           mstid -- instance value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : internal_port_path_cost
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortAutoPathCost(UI32_T lport, UI32_T mstid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortAdminPathCostAgent
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the admin path_cost of the port.
 *
 * INPUT   : lport     -- lport number
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : Writing a value of '0' assigns the automatically calculated
 *           default Path Cost value to the port.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortAdminPathCostAgent(UI32_T lport, UI32_T path_cost);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortAutoPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Restore the default path_cost of the port.
 *
 * INPUT   : lport -- lport number
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURN_PORTNO_OOR        -- port number out of range
 *
 * NOTES   : external_port_path_cost
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortAutoPathCost(UI32_T lport);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetMstPortAdminPathCostAgent
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the admin path_cost of the port for specified spanning tree
 *           when mode is MSTP.
 *
 * INPUT   : lport     -- lport number
 *           mstid     -- instance value
 *           path_cost -- the path_cost value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                -- set successfully
 *           XSTP_TYPE_RETURN_ERROR             -- failed
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR -- not master mode
 *           XSTP_TYPE_RETURNPORTNO_OOR         -- port number out of range
 *           XSTP_TYPE_RETURNINDEX_OOR          -- mstid and priority
 *                                                 out of range
 *
 * NOTES   : Writing a value of '0' assigns the automatically calculated
             default Path Cost value to the port.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetMstPortAdminPathCostAgent(UI32_T lport,
                                              UI32_T mstid,
                                              UI32_T path_cost);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortSpanningTreeStatus
 * ----------------------------------------------------------------------------
 * PURPOSE : Set the spanning tree status for the specified port.
 *
 * INPUT   : lport  -- lport number
 *           status -- the status value
 *
 * OUTPUT  : None.
 *
 * RETURN  : XSTP_TYPE_RETURN_OK                 -- set successfully
 *           XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 *           XSTP_TYPE_RETURN_ERROR              -- failed
 *           XSTP_TYPE_RETURN_ST_STATUS_DISABLED
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortSpanningTreeStatus(UI32_T lport, UI32_T status);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetRunningMstPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the path_cost of the port.
 *
 * INPUT   : lport -- lport number
 *           mstid -- instance value
 *
 * OUTPUT  : path_cost -- pointer of the path_cost value
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default value.
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetRunningMstPortPathCost(UI32_T lport,
                                           UI32_T mstid,
                                           UI32_T *path_cost);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetMstPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the path_cost of the port.
 *
 * INPUT   : lport -- lport number
 *           mstid -- instance value
 *
 * OUTPUT  : path_cost -- pointer of the path_cost value
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : TRUE/FALSE
 *
 * NOTES   : For SNMP.
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetMstPortPathCost(UI32_T lport,
                                    UI32_T mstid,
                                    UI32_T *path_cost);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetLportDefaultPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the specified lport path cost.
 *
 * INPUT   : lport -- the lport number
 *
 * OUTPUT  : path_cost -- pointer of the path cost value
 *
 * RETURN  : XSTP_TYPE_RETURN_OK    -- OK
 *           XSTP_TYPE_RETURN_ERROR -- failed
 *
 * NOTE    : This value is calculated from specification.
 *-----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetLportDefaultPathCost(UI32_T lport, UI32_T *path_cost);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetDot1dMstEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the specified spanning tree entry
 *           info can be successfully retrieved. Otherwise, false is returned.
 *
 * INPUT   : mstid -- instance value
 *
 * OUTPUT  : entry -- the specified mst entry info
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetDot1dMstEntry(UI32_T mstid,
                                  XSTP_MGR_Dot1dStpEntry_T *entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetNextDot1dMstEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the next spanning tree entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 *
 * INPUT   : mstid -- instance value
 *
 * OUTPUT  : entry -- the specified mst entry info
 *           mstid -- instance value
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : For SNMP.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetNextDot1dMstEntry(UI32_T *mstid,
                                      XSTP_MGR_Dot1dStpEntry_T *entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetDot1dBaseEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the specified base entry
 *           info can be successfully retrieved. Otherwise, false is
 *           returned.
 *
 * INPUT   : None.
 *
 * OUTPUT  : base_entry -- base entry info
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetDot1dBaseEntry(XSTP_MGR_Dot1dBaseEntry_T *base_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetDot1dBasePortEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the specified base port entry
 *           info can be successfully retrieved. Otherwise, false is
 *           returned.
 *
 * INPUT   : base_port_entry->dot1d_base_port
 *                                       -- key to specify a unique base
 *                                          entry
 *
 * OUTPUT  : base_port_entry             -- base entry info of specified
 *                                          key
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME -- XSTP_PMGR_GetNextDot1dBasePortEntry
 * ----------------------------------------------------------------------------
 * PURPOSE : This funtion returns true if the next available base port
 *           entry info can be successfully retrieved. Otherwise, false
 *           is returned.
 *
 * INPUT   : base_port_entry->dot1d_base_port
 *                                       -- key to specify a unique base
 *                                          entry
 *
 * OUTPUT  : base_port_entry             -- xstp entry info of specified
 *                                          key
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : If next available stp entry is available, the
 *           base_port_entry->dot1d_base_port will be updated and the
 *           entry info will be retrieved from the table.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetNextDot1dBasePortEntry(XSTP_MGR_Dot1dBasePortEntry_T *base_port_entry);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetRunningMstpConfigurationName
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the MSTP configurstion name.
 *
 * INPUT   : None.
 *
 * OUTPUT  : config_name -- pointer of the config_name
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default value.
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetRunningMstpConfigurationName(UI8_T *config_name);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetMstpConfigurationName
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the MSTP configurstion name.
 *
 * INPUT   : None.
 *
 * OUTPUT  : config_name -- pointer of the config_name
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : For SNMP.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetMstpConfigurationName(UI8_T *config_name);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_VlanIsMstMember
 * ----------------------------------------------------------------------------
 * PURPOSE : This funcion returns true if the vlan is in the
 *           specified spanning tree. Otherwise, returns false.
 *
 * INPUT   : mstid -- the instance id
 *           vlan  -- the vlan value
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_VlanIsMstMember(UI32_T mstid, UI32_T vlan);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetPortStateByVlan
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the port state in a specified vlan.
 *
 * INPUT   : vid   -- vlan id
 *           lport -- lport number
 *
 * OUTPUT  : state -- the pointer of state value
 *                    XSTP_TYPE_PORT_STATE_DISCARDING
 *                    XSTP_TYPE_PORT_STATE_LEARNING
 *                    XSTP_TYPE_PORT_STATE_FORWARDING
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTES   : None.
 * ----------------------------------------------------------------------------
 */
BOOL_T XSTP_PMGR_GetPortStateByVlan(UI32_T vid,
                                    UI32_T lport,
                                    UI32_T *state);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetMappedInstanceByVlan
 * ----------------------------------------------------------------------------
 * PURPOSE : Get mapped instance by a specified vlan
 *
 * INPUT   : vid   -- vlan id
 *           mstid -- instance value pointer
 *
 * OUTPUT  : mstid -- instance value pointer
 *
 * RETURN  : TRUE if OK, or FALSE if at the end of the port list
 *
 * NOTE    : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T  XSTP_PMGR_GetMappedInstanceByVlan(UI32_T vid, UI32_T *mstid);

#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_GetRunningPortPathCost
 * ----------------------------------------------------------------------------
 * PURPOSE : Get the external path_cost of the port for CIST.
 *
 * INPUT   : lport         -- lport number
 *
 * OUTPUT  : ext_path_cost -- pointer of the path_cost value
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *
 * NOTES   : 1. This function shall only be invoked by CLI to save the
 *              "running configuration" to local or remote files.
 *           2. Since only non-default configuration will be saved, this
 *              function shall return non-default value.
 *
 * REF     : RFC-1493/dot1dStpPortEntry 5
 * ----------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_GetRunningPortPathCost(UI32_T lport, UI32_T *ext_path_cost);

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortRootGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set root guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_ENABLED
 *                                         XSTP_TYPE_PORT_ROOT_GUARD_DISABLED
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   Only designated port can be enabled root guard function now.
 * ------------------------------------------------------------------------
 */
UI32_T  XSTP_PMGR_SetPortRootGuardStatus(UI32_T lport, UI32_T status);
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBpduGuardStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU guard status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBpduGuardStatus(UI32_T lport, UI32_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBPDUGuardAutoRecovery
 * ------------------------------------------------------------------------
 * PURPOSE :  Set BPDU guard auto recovery status for the specified port.
 * INPUT   :  lport  -- the logical port number
 *            status -- the status value
 * OUTPUT  :  None
 * RETURN  :  XSTP_TYPE_RETURN_CODE_E
 * NOTE    :  None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBPDUGuardAutoRecovery(UI32_T lport, UI32_T status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBPDUGuardAutoRecoveryInterval
 * ------------------------------------------------------------------------
 * PURPOSE :  Set BPDU guard auto recovery status for the specified port.
 * INPUT   :  lport    -- the logical port number
 *            interval -- the interval value
 * OUTPUT  :  None
 * RETURN  :  XSTP_TYPE_RETURN_CODE_E
 * NOTE    :  None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBPDUGuardAutoRecoveryInterval(UI32_T lport, UI32_T interval);
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortBpduFilterStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the BPDU filter status for the specified port.
 * INPUT    :   UI32_T lport            -- lport number
 *              UI32_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortBpduFilterStatus(UI32_T lport, UI32_T status);
#endif /* #if (SYS_CPNT_STP_BPDU_FILTER == TRUE) */

#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetCiscoPrestandardCompatibility
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the cisco prestandard compatibility status
 * INPUT    :   status    -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetCiscoPrestandardCompatibility(UI32_T status);
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */

#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetPortTcPropStop
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port don't propagate TC
 * INPUT    :   UI32_T lport            -- lport number
 *              BOOL_T status           -- the status value
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK
 *              XSTP_TYPE_RETURN_ERROR
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetPortTcPropStop(UI32_T lport, BOOL_T status);
#endif
#if (SYS_CPNT_EAPS == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetEthRingPortRole
 * ------------------------------------------------------------------------
 * PURPOSE  :   To set the port role for eth ring protocol.
 * INPUT    :   lport     -- lport number (1-based)
 *              port_role -- port role to set
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetEthRingPortRole(
    UI32_T  lport,
    UI32_T  port_role);

#endif /* End of #if (SYS_CPNT_EAPS == TRUE) */

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_SetTcPropGroupPortList
 * ------------------------------------------------------------------------
 * PURPOSE  :   To add/remove the ports to/from a group.
 * INPUT    :   is_add       -- add or remove
 *              group_id     -- group ID
 *              portbitmap   -- ports
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_SetTcPropGroupPortList(BOOL_T is_add,
                                UI32_T group_id,
                                UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_PMGR_DelTcPropGroup
 * ------------------------------------------------------------------------
 * PURPOSE  :   To delete a group.
 * INPUT    :   group_id     -- group ID
 * OUTPUT   :   None
 * RETURN   :   XSTP_TYPE_RETURN_OK                 -- set successfully
 *              XSTP_TYPE_RETURN_ERROR              -- failed
 *              XSTP_TYPE_RETURN_MASTER_MODE_ERROR  -- not master mode
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
UI32_T XSTP_PMGR_DelTcPropGroup(UI32_T group_id);
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

#endif /* #ifndef XSTP_PMGR_H */

