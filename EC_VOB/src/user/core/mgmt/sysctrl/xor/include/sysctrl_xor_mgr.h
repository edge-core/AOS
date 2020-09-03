/* Module Name: SYSCTRL_XOR_MGR.H
 * Purpose: The XOR component (Exclusive OR) provides system view check when this
 *          setting maybe conflict with other setting and can't enable both.
 *          This component is core-layer internal component and will not be called
 *          by application function. System core-layer component need to call XOR
 *          to check out this setting is valid in system view or not? If it is valid,
 *          set it; else ignore and return FALSE to upper-layer component.
 *
 *          XOR component is no any database, and it just provides some APIs to do
 *          check for other core-layer component.
 *          XOR API function Need lock one XOR semaphore before it be called, and after
 *          calling function have finished whole operation, calling function will call
 *          one XOR free semaphore function to free XOR semaphore.
 *
 *          XOR will get some information from some core layer OM, like SWCTRL OM,
 *          LACP OM, and do some check itself base on exclusion rule, then report
 *          check result to calling function.
 *
 * Notes:
 *          This component is internal component. So, it is not exist any command from
 *          user view. This component is called by core layer component, like SWCTRL,
 *          LACP, ¡K and is not called by application layer component.
 *
 * History:
 *          Date        Modifier        Reason
 *          2005/7/13   Justin Jan      Create this file
 *          2005/7/20   Aaron Chuang    Coding
 *
 * Copyright(C)      Accton Corporation, 2005
 */

#ifndef SYSCTRL_XOR_MGR_H
#define SYSCTRL_XOR_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSCTRL_XOR_MGR_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resource for SYSCTRL_XOR_MGR.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void SYSCTRL_XOR_MGR_Init(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingEnabledLACP
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be enabled lacp.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingEnabledLACP(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingJoinedToTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be joined static
 *           trunk.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingJoinedToTrunk(UI32_T ifindex);

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the portlist can be set to private
 *           port.
 * INPUT   : downlink_port_list -- downlink port list
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePort(UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePortByIfindex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to private
 *           port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePortByIfindex(UI32_T ifindex);
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToMirror
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to
 *           mirror to port/mirrored port.
 * INPUT   : mirror_to_ifindex -- mirror-to-port ifindex
 *           mirrored_ifindex  -- mirrored-port ifindex
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : mirror_to_ifindex = 0: don't care mirror to port.
 *           mirrored_ifindex =  0: don't care mirrored port.
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToMirror(UI32_T mirrored_ifindex, UI32_T mirror_to_ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToSecurityPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to
 *           security port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToSecurityPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_GetSemaphore
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get XOR semphore.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_GetSemaphore();

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_ReleaseSemaphore
 * -------------------------------------------------------------------------
 * FUNCTION: This function will release XOR semphore.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_ReleaseSemaphore();

#if (SYS_CPNT_RSPAN == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetRspanEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the port can be set in the RSPAN
 *           session.
 * INPUT   : target_port  -- the port needs to check the xor relationship.
 *           port_role    -- LEAF_rspanSrcTxPorts / LEAF_rspanSrcRxPorts /
 *                           LEAF_rspanDstPort / LEAF_rspanRemotePorts
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetRspanEntry( UI8_T target_port, UI8_T port_role ) ;
#endif /* End of #if (SYS_CPNT_RSPAN == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToXSTPPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to spanning tree
 *           port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToXSTPPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingDeleteVlan
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the VLAN id can be deleted.
 * INPUT   : vid -- VLAN id to test
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed       (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingDeleteVlan(UI32_T vid);

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToPfcPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to
 *           PFC enabled port.
 * INPUT   : ifindex -- interface index to test
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed       (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToPfcPort(UI32_T ifindex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToFcPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to
 *           FC enabled port.
 * INPUT   : ifindex -- interface index to test
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed       (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToFcPort(UI32_T ifindex);
#endif /* #if (SYS_CPNT_PFC == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingDestroyTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the trunk id can be deleted.
 * INPUT   : trunk_id -- trunk id to test
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed       (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingDestroyTrunk(UI32_T trunk_id);

#if (SYS_CPNT_EAPS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToEapsPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to
 *           EAPS ring port.
 * INPUT   : ifindex -- interface index to test
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed       (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToEapsPort(UI32_T ifindex);
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

#if (SYS_CPNT_MLAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitBeingSetToMlagPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be set to
 *           MLAG peer link or MLAG member.
 * INPUT   : ifindex          -- interface index to test
 * OUTPUT  : None
 * RETURN  : TRUE : permitted
 *           FALSE: not permitted
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToMlagPort(UI32_T ifindex);
#endif

#endif /* SYSCTRL_XOR_MGR_H */

