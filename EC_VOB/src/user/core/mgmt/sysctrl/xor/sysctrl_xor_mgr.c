/* Module Name: SYSCTRL_XOR_MGR.C
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
 *          LACP, and is not called by application layer component.
 *
 * History:
 *          Date        Modifier        Reason
 *          2005/7/13   Justin Jan      Create this file
 *          2005/7/20   Aaron Chuang    Coding
 *
 * Copyright(C)      Accton Corporation, 2005
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "leaf_es3626a.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt_xor.h"
#include "sysfun.h"
/*#include "backdoor_mgr.h"*//*Timon*/
#include "lacp_pom.h"
#include "swctrl_pom.h"
#include "sysctrl_xor_mgr.h"

#include "xstp_pom.h"

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_om.h"
#endif /*#if (SYS_CPNT_RSPAN == TRUE)*/

#if (SYS_CPNT_PFC == TRUE)
#include "pfc_om.h"
#endif

#if (SYS_CPNT_MLAG == TRUE)
#include "mlag_pom.h"
#endif

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
#include "swctrl_pom.h"
#endif

#ifdef SYS_DFLT_XOR_QOS_PORT_EXCLUSION
#include "rule_om.h"
#endif

/* NAMING CONSTANT DECLARARTIONS
 */
/* to avoid error of undeclared SYS_DFLT_XOR_xxx,
 *   add new definition here,
 *   if the new definition is not defined in every project.
 */
/* exclusion bit
 */
#ifndef SYS_DFLT_XOR_XSTP_PORT_EXCLUSION
    #define SYS_DFLT_XOR_XSTP_PORT_EXCLUSION    0
#endif

/* permit rule
 */
#ifndef SYS_DFLT_XOR_DELETE_VLAN_RULE
    #define SYS_DFLT_XOR_DELETE_VLAN_RULE       0
#endif

#ifndef SYS_DFLT_XOR_SET_TO_XSTP_PORT_RULE
    #define SYS_DFLT_XOR_SET_TO_XSTP_PORT_RULE  0
#endif

/* TYPE DEFINITIONS
 */
typedef struct
{
    UI32_T xor_rule_bit;
    void *xor_conflict_func;
} SYSCTRL_XOR_MGR_XorRule_T;


/* MACRO FUNCTION DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T SYSCTRL_XOR_MGR_PermitWithIfindexByRule(UI32_T xor_rule, UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_PermitWithVidByRule(UI32_T xor_rule, UI32_T vid);
#if (SYS_CPNT_LACP == TRUE)
static BOOL_T SYSCTRL_XOR_MGR_IsEnabledLacp(UI32_T ifindex);
#endif
static BOOL_T SYSCTRL_XOR_MGR_IsTrunkMember(UI32_T ifindex);
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
static BOOL_T SYSCTRL_XOR_MGR_IsPrivatePort(UI32_T ifindex);
#endif
static BOOL_T SYSCTRL_XOR_MGR_IsMirrorToPort(UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_IsMirroredPort(UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_IsSecurityPort(UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_IsExistingPort(UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_IsTrunkPort(UI32_T ifindex);
#if (SYS_CPNT_RSPAN == TRUE)
static BOOL_T SYSCTRL_XOR_MGR_IsRspanMirrorToPort(UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_IsRspanMirroredPort(UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_IsRspanUplinkPort(UI32_T ifindex);
#endif /* End of #if (SYS_CPNT_RSPAN == TRUE) */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
static BOOL_T SYSCTRL_XOR_MGR_IsVlanMacMirrorToPort(UI32_T ifindex);
#endif

static BOOL_T SYSCTRL_XOR_MGR_IsEnabledSpanningTree(UI32_T ifindex);
#if (SYS_CPNT_EAPS == TRUE)
static BOOL_T SYSCTRL_XOR_MGR_IsEapsRingPort(UI32_T ifindex);
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
static BOOL_T SYSCTRL_XOR_MGR_IsEnabledPfc(UI32_T ifindex);
static BOOL_T SYSCTRL_XOR_MGR_IsEnabledFc(UI32_T ifindex);
#endif

#if (SYS_CPNT_MLAG == TRUE)
static BOOL_T SYSCTRL_XOR_MGR_IsMlagPort(UI32_T ifindex);
#endif

static BOOL_T SYSCTRL_XOR_MGR_IsQosPort(UI32_T ifindex);

#if 0 /*Timon*/
static void   SYSCTRL_XOR_MGR_BackDoorMenu(void);
static BOOL_T SYSCTRL_XOR_MGR_BackDoorGetIfindex(UI32_T *ifindex);
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingEnabledLACP(void);
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingJoinedToTrunk(void);
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToPrivatePort(void);
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToMirror(void);
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToSecurityPort(void);
#endif


/* STATIC VARIABLE DECLARATIONS
 */

static UI32_T sysctrl_xor_mgr_sem_id;
static UI32_T original_priority;
static BOOL_T sysctrl_xor_mgr_debug_print = FALSE;

SYSCTRL_XOR_MGR_XorRule_T sysctrl_xor_mgr_ifindex_rules[] =
{
#if (SYS_CPNT_LACP == TRUE)
    { SYS_DFLT_XOR_LACP_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsEnabledLacp },
#endif
    { SYS_DFLT_XOR_TRUNK_MEMBER_EXCLUSION, SYSCTRL_XOR_MGR_IsTrunkMember },
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
    { SYS_DFLT_XOR_PRIVATE_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsPrivatePort },
#endif
    { SYS_DFLT_XOR_MIRROR_TO_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsMirrorToPort },
    { SYS_DFLT_XOR_MIRRORED_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsMirroredPort },
    { SYS_DFLT_XOR_SECURITY_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsSecurityPort },
#if (SYS_CPNT_RSPAN == TRUE)
    { SYS_DFLT_XOR_RSPAN_MIRROR_TO_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsRspanMirroredPort },
    { SYS_DFLT_XOR_RSPAN_MIRRORED_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsRspanMirrorToPort },
    { SYS_DFLT_XOR_RSPAN_UPLINK_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsRspanUplinkPort },
#endif
#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
    { SYS_DFLT_XOR_MIRROR_TO_PORT_NOT_VLAN_MAC_MIRROR_EXCLUSION, SYSCTRL_XOR_MGR_IsVlanMacMirrorToPort },
#endif
#if (SYS_CPNT_EAPS == TRUE)
    { SYS_DFLT_XOR_EAPS_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsEapsRingPort },
#endif
#if (SYS_CPNT_PFC == TRUE)
    { SYS_DFLT_XOR_PFC_PORT_EXCLUSION,  SYSCTRL_XOR_MGR_IsEnabledPfc },
    { SYS_DFLT_XOR_FC_PORT_EXCLUSION,   SYSCTRL_XOR_MGR_IsEnabledFc  },
#endif
    { SYS_DFLT_XOR_XSTP_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsEnabledSpanningTree },
#if (SYS_CPNT_MLAG == TRUE)
    { SYS_DFLT_XOR_MLAG_PORT_EXCLUSION, SYSCTRL_XOR_MGR_IsMlagPort },
#endif

#ifdef SYS_DFLT_XOR_QOS_PORT_EXCLUSION
    {SYS_DFLT_XOR_QOS_PORT_EXCLUSION,   SYSCTRL_XOR_MGR_IsQosPort},
#endif
};

SYSCTRL_XOR_MGR_XorRule_T sysctrl_xor_mgr_vid_rules[] =
{
};


/* EXPORTED SUBPROGRAM BODIES
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
void SYSCTRL_XOR_MGR_Init(void)
{
    /* Register a xor backdoor */
/*    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sysctrl_xor_mgr", SYSCTRL_XOR_MGR_BackDoorMenu);*//*Timon*/

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYSCTRL_XOR_MGR_AttachSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Attach system resource for SYSCTRL_XOR_MGR in the context of the
 *           calling process.
 *
 * INPUT   : None
 *
 * OUTPUT  : None
 *
 * RETURN  : None
 *
 * NOTES   : None
 *-----------------------------------------------------------------------------
 */
void SYSCTRL_XOR_MGR_AttachSystemResources(void)
{
    if (SYSFUN_OK!=SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSCTRL_XOR_MGR, &sysctrl_xor_mgr_sem_id))
    {
        printf("%s(): SYSFUN_GetSem fails.\n", __FUNCTION__);
    }
}

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingEnabledLACP(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* The ifindex is a trunk port */
    if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_ENABLE_LACP_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitBeingEnabledLACP() */

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingJoinedToTrunk(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* The ifindex is a trunk port */
    if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_JOIN_TO_TRUNK_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitBeingJoinedToTrunk() */

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePort(UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
    UI32_T              ifindex;

    for(ifindex=1; ifindex<=SYS_ADPT_TOTAL_NBR_OF_LPORT; ifindex++)
    {
        /* The Table means:
         * Downlink_port_list[0] is 0x80 when ifindex 1 is a private port.
         * Downlink_port_list[0] is 0x10 when ifindex 4 is a private port.
         * Downlink_port_list[0] is 0x90 when ifindex 1 and ifindex 4 both are private ports.
         *
         *  ------------------------------------------------------------------
         * |  ifindex  |  (ifindex-1)/8  |  downlink_port_list[ifindex-1)/8]  |
         *  ------------------------------------------------------------------
         * |     1     |         0       |                0x80                |
         * |     2     |         0       |                0x40                |
         * |     3     |         0       |                0x20                |
         * |     4     |         0       |                0x10                |
         * |     5     |         0       |                0x08                |
         * |     6     |         0       |                0x04                |
         * |     7     |         0       |                0x02                |
         * |     8     |         0       |                0x01                |
         * |     9     |         1       |                0x80                |
         * |    10     |         1       |                0x40                |
         * |     .     |         .       |                 .                  |
         *  ------------------------------------------------------------------
         */
        /* The ifindex is a private port */
        if((downlink_port_list[(ifindex-1)/8]<<((ifindex-1)%8))&0x80)
        {
            /* the ifindex can be set to private port */
            if (FALSE == SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePortByIfindex(ifindex))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePort */

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePortByIfindex(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION)
    /* NOTE:
     *   current implementation for single session on linux platform
     *   is for user ports, not logical ports.
     */

    /* The ifindex is a trunk port */
    if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(ifindex))
    {
        return FALSE;
    }
#endif

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_PRIVATE_PORT_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePortByIfindex */
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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToMirror(UI32_T mirrored_ifindex, UI32_T mirror_to_ifindex)
{
    /* Check mirror to port */
    if(0 != mirror_to_ifindex)
    {
        /* The ifindex is not existing */
        if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(mirror_to_ifindex))
        {
            return FALSE;
        }

        /* The ifindex is a trunk port */
        if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(mirror_to_ifindex))
        {
            return FALSE;
        }

        /* Check XOR rules */
        if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_MIRROR_TO_PORT_RULE, mirror_to_ifindex))
        {
            return FALSE;
        }
    } /* End of if(0 != mirror_to_ifindex) */

    /* Check mirrored port */
    if(0 != mirrored_ifindex)
    {
        /* The ifindex is not existing */
        if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(mirrored_ifindex))
        {
            return FALSE;
        }

        /* The ifindex is a trunk port */
        if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(mirrored_ifindex))
        {
            return FALSE;
        }

        /* Check XOR rules */
        if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_MIRRORED_PORT_RULE, mirrored_ifindex))
        {
            return FALSE;
        }
    } /* End of if(0 != mirrored_ifindex) */

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitBeingSetToMirror */

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToSecurityPort(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* The ifindex is a trunk port */
    if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_SECURITY_PORT_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitBeingSetToSecurityPort() */

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
BOOL_T SYSCTRL_XOR_MGR_GetSemaphore()
{
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(sysctrl_xor_mgr_sem_id);
    return TRUE;
}

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
BOOL_T SYSCTRL_XOR_MGR_ReleaseSemaphore()
{
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(sysctrl_xor_mgr_sem_id, original_priority);
    return TRUE;
}

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingDeleteVlan(UI32_T vid)
{
    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithVidByRule(SYS_DFLT_XOR_DELETE_VLAN_RULE, vid))
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToPfcPort(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_PFC_PORT_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToFcPort(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_FC_PORT_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
}
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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingDestroyTrunk(UI32_T trunk_id)
{
    UI32_T  trk_ifidx;

    if (FALSE == SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trk_ifidx))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_DESTROY_TRUNK_RULE, trk_ifidx))
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToEapsPort(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* The ifindex is a trunk port */
    if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_EAPS_PORT_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToXSTPPort(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(SYS_DFLT_XOR_SET_TO_XSTP_PORT_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetToMlagPort(UI32_T ifindex)
{
    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(ifindex))
    {
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(
            SYS_DFLT_XOR_SET_TO_MLAG_PORT_RULE, ifindex))
    {
        return FALSE;
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitWithIfindexByRule
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex can be enabled lacp.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_PermitWithIfindexByRule(UI32_T xor_rule, UI32_T ifindex)
{
    int i;

    for (i = 0; xor_rule != 0 && i < sizeof(sysctrl_xor_mgr_ifindex_rules)/sizeof(*sysctrl_xor_mgr_ifindex_rules); i++)
    {
        if (xor_rule & sysctrl_xor_mgr_ifindex_rules[i].xor_rule_bit)
        {
            if (((BOOL_T (*)(UI32_T))sysctrl_xor_mgr_ifindex_rules[i].xor_conflict_func)(ifindex))
            {
                return FALSE;
            }
            xor_rule ^= sysctrl_xor_mgr_ifindex_rules[i].xor_rule_bit;
        }
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitWithIfindexByRule() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_PermitWithVidByRule
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the specified xor rule
 *            is permitted for specified vid
 * INPUT   : xor_rule -- xor rule to check
 *           vid      -- vid to check
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_PermitWithVidByRule(UI32_T xor_rule, UI32_T vid)
{
    int i;

    for (i = 0; xor_rule != 0 && i < sizeof(sysctrl_xor_mgr_vid_rules)/sizeof(*sysctrl_xor_mgr_vid_rules); i++)
    {
        if (xor_rule & sysctrl_xor_mgr_vid_rules[i].xor_rule_bit)
        {
            if (((BOOL_T (*)(UI32_T))sysctrl_xor_mgr_vid_rules[i].xor_conflict_func)(vid))
            {
                return FALSE;
            }
            xor_rule ^= sysctrl_xor_mgr_vid_rules[i].xor_rule_bit;
        }
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_PermitWithVidByRule() */

#if (SYS_CPNT_EAPS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsEapsRingPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is an EAPS ring port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsEapsRingPort(UI32_T ifindex)
{
    if (FALSE == EAPS_MGR_RingPort_Check(ifindex))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a EAPS ring port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsEapsRingPort() */
#endif /* #if (SYS_CPNT_EAPS == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsEnabledPfc
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is PFC enabled.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsEnabledPfc(UI32_T ifindex)
{
    UI32_T  tmp_data;

    if (  (TRUE == PFC_OM_GetDataByField(
                    ifindex, PFC_TYPE_FLDE_PORT_ENABLE, &tmp_data))
        &&(TRUE == tmp_data)
       )
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a PFC enabled port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsEnabledPfc() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsEnabledFc
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is FC enabled.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsEnabledFc(UI32_T ifindex)
{
    if (TRUE == SWCTRL_POM_isPortFlowControlEnabled(ifindex))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a FC enabled port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsEnabledFc() */
#endif /* #if (SYS_CPNT_PFC == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsEnabledSpanningTree
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is enabled spanning tree.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is enabled spanning tree.
 *           FALSE: The ifindex is not enabled spanning tree.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsEnabledSpanningTree(UI32_T ifindex)
{
    UI32_T status;

    if (    (XSTP_POM_GetPortSpanningTreeStatus(ifindex, &status) == TRUE)
        &&  (status == VAL_staPortSystemStatus_enabled)
       )
    {
        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsEnabledSpanningTree() */

#if (SYS_CPNT_LACP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsEnabledLacp
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is enabled LACP.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is enabled LACP.
 *           FALSE: The ifindex is not enabled LACP.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsEnabledLacp(UI32_T ifindex)
{
    UI32_T              lacp_state;

    /* Ifindex is a LACP port. */
    if(SYS_TYPE_GET_RUNNING_CFG_FAIL == LACP_POM_GetRunningDot3adLacpPortEnabled(ifindex, &lacp_state))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld get running LACP port enabled failed.\r\n", __FUNCTION__, ifindex);
        }

        return FALSE;
    }

    /* Ifindex is enabled LACP. */
    if(VAL_lacpPortStatus_enabled == lacp_state)
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is enabled LACP.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsEnabledLacp() */
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsTrunkMember
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a trunk member.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a static trunk member
 *           FALSE: The ifindex is not static trunk member
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsTrunkMember(UI32_T ifindex)
{
    UI32_T trunk_ifindex;
    BOOL_T is_static;

    /* Ifindex is a static trunk member. */
    if(TRUE == SWCTRL_POM_IsTrunkMember(ifindex, &trunk_ifindex, &is_static))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a trunk member.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsTrunkMember() */

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsPrivatePort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a private port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a private port
 *           FALSE: The ifindex is not private port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsPrivatePort(UI32_T ifindex)
{
    /* The ifindex is not existing (too large) */
    if(SYS_ADPT_TOTAL_NBR_OF_LPORT < ifindex)
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is not existing (too large).\r\n", __FUNCTION__, ifindex);
        }

        return FALSE;
    }

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION)
    /* for multiple session,
     * uplink ports is no longer allowed to be forwarded to all ports,
     * so check it.
     */
    if(TRUE == SWCTRL_POM_IsPortPrivateVlanUplinkMember(ifindex))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a private port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }
#endif /* (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION) */

    /* Ifindex is a private port. */
    if(TRUE == SWCTRL_POM_IsPortPrivateVlanDownlinkMember(ifindex))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a private port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsPrivatePort() */
#endif /* (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsMirrorToPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a mirror
 *           destination port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a mirror destination port
 *           FALSE: The ifindex is not a mirror destination port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsMirrorToPort(UI32_T ifindex)
{
    SWCTRL_MirrorEntry_T mirror_entry;

    /* Clear mirror_entry */
    memset(&mirror_entry, 0, sizeof(mirror_entry));

    /* Get the next mirror entry. */
    while(TRUE == SWCTRL_POM_GetNextMirrorEntry(&mirror_entry))
    {
        /* Ifindex is a mirror to port. */
        if(ifindex == mirror_entry.mirror_destination_port)
        {
            if(TRUE == sysctrl_xor_mgr_debug_print)
            {
                printf("%s(): ifindex %ld is a mirror to port.\r\n", __FUNCTION__, ifindex);
            }

            return TRUE;
        }
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsMirrorToPort() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsMirroredPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a mirrored port
 *           FALSE: The ifindex is not a mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsMirroredPort(UI32_T ifindex)
{
    SWCTRL_MirrorEntry_T mirror_entry;

    /* Clear mirror entry */
    memset(&mirror_entry, 0, sizeof(mirror_entry));

    /* Get the next mirror entry. */
    while(TRUE == SWCTRL_POM_GetNextMirrorEntry(&mirror_entry))
    {
        /* Ifindex is a mirrored port. */
        if(ifindex == mirror_entry.mirror_source_port)
        {
            if(TRUE == sysctrl_xor_mgr_debug_print)
            {
                printf("%s(): ifindex %ld is a mirrored port.\r\n", __FUNCTION__, ifindex);
            }

            return TRUE;
        }
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsMirroredPort() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsSecurityPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is security port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is security port.
 *           FALSE: The ifindex is not security port.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsSecurityPort(UI32_T ifindex)
{
    UI32_T port_security_enabled_by_who;

    /* Ifindex is a mirrored port. */
    if(TRUE == SWCTRL_POM_IsSecurityPort(ifindex, &port_security_enabled_by_who))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a security port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsSecurityPort() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsExistingPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is existing.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is existing.
 *           FALSE: The ifindex is not existing.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsExistingPort(UI32_T ifindex)
{
    UI32_T              unit, port, trunk;
    SWCTRL_Lport_Type_T port_type;

    /* The ifindex is not existing (too large) */
    if(SYS_ADPT_TOTAL_NBR_OF_LPORT < ifindex)
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is not existing (too large).\r\n", __FUNCTION__, ifindex);
        }

        return FALSE;
    }

    /* Transfer ifindex to user port */
    port_type = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk);

    /* The ifindex is not existing */
    if(SWCTRL_LPORT_UNKNOWN_PORT == port_type)
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is not existing.\r\n", __FUNCTION__, ifindex);
        }

        return FALSE;
    }

    return TRUE;
} /* End of SYSCTRL_XOR_MGR_IsExistingPort() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsTrunkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is trunk port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is trunk port.
 *           FALSE: The ifindex is not trunk port.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsTrunkPort(UI32_T ifindex)
{
    UI32_T              unit, port, trunk;
    SWCTRL_Lport_Type_T port_type;

    /* Transfer ifindex to user port */
    port_type = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk);

    /* The ifindex is a trunk port */
    if(SWCTRL_LPORT_TRUNK_PORT == port_type)
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a trunk port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
} /* End of SYSCTRL_XOR_MGR_IsTrunkPort() */

#if 0 /*Timon*/
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_BackDoor_Menu
 * -------------------------------------------------------------------------
 * FUNCTION: This function will show sysctrl_xor_mgr backdoor menu.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SYSCTRL_XOR_MGR_BackDoorMenu(void)
{
    UI8_T  ch;
    BOOL_T exit_form_loop = FALSE;

    /* Show backdoor munu */
    while(!exit_form_loop)
    {
        printf("\n========== SYSCTRL_XOR_MGR BackDoor Menu ==========\n");
        printf(" 0. Exit.\n");
        printf(" 1. %s debug print.\n", sysctrl_xor_mgr_debug_print ? "Disable" : "Enable");
        printf(" 2. Can ifindex be enabled LACP?\n");
        printf(" 3. Can ifindex be joined to trunk?\n");
        printf(" 4. Can ifindex be set to private port?\n");
        printf(" 5. Can ifindex be set to mirror?\n");
        printf(" 6. Can ifindex be set to security port?\n");
        printf("\n Select = ");

        ch = getchar();
        printf("%c\n",ch);

        /* Select backdoor item */
        switch(ch)
        {
            case '0' :
                exit_form_loop = TRUE;
                break;
            case '1':
                sysctrl_xor_mgr_debug_print = !sysctrl_xor_mgr_debug_print;
                break;
            case '2':
                SYSCTRL_XOR_MGR_BackDoorPermitBeingEnabledLACP();
                break;
            case '3':
                SYSCTRL_XOR_MGR_BackDoorPermitBeingJoinedToTrunk();
                break;
            case '4':
                SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToPrivatePort();
                break;
            case '5':
                SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToMirror();
                break;
            case '6':
                SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToSecurityPort();
                break;
            default :
                ch = 0;
                break;
        }
    } /* End of while() */
} /* End of SYSCTRL_XOR_MGR_BackDoorMenu() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_BackDoorGetIfindex
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get the ifindex value.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : Get an ifindex successfully
 *           FALSE: Get an null ifindex
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_BackDoorGetIfindex(UI32_T *ifindex)
{
    UI8_T   ch;
    BOOL_T  success = FALSE;

    *ifindex = 0;
    printf(" Ifindex = ");

    /* Get input */
    while((ch = getchar()) != '\n')
    {
        ch -= 0x30;

        if(ch > 9)
        {
            continue;
        }

        printf("%d", ch);

        *ifindex = (*ifindex * 10) + ch;
        success  = TRUE;
    }

    printf("\n");

    /* Get an null ifindex */
    if(FALSE == success)
    {
        return FALSE;
    }
    else /* Get an ifindex successfully */
    {
        return TRUE;
    }
} /* End of SYSCTRL_XOR_MGR_BackDoorGetIfindex() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_BackDoorPermitBeingEnabledLACP
 * -------------------------------------------------------------------------
 * FUNCTION: Back door will test whether the ifindex can be enabled LACP.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingEnabledLACP(void)
{
    UI32_T ifindex;

    /* Can not get an ifindex. */
    if(FALSE == SYSCTRL_XOR_MGR_BackDoorGetIfindex(&ifindex))
    {
        return FALSE;
    }

    /* The ifindex can be enabled LACP. */
    if(TRUE == SYSCTRL_XOR_MGR_PermitBeingEnabledLACP(ifindex))
    {
        printf(" Yes, ifindex %ld can be enabled LACP.\n", ifindex);
        return TRUE;
    }

    printf(" No, ifindex %ld can NOT be enabled LACP.\n", ifindex);
    return FALSE;
} /* End of SYSCTRL_XOR_MGR_BackDoorPermitBeingEnabledLACP() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_BackDoorPermitBeingJoinedToTrunk
 * -------------------------------------------------------------------------
 * FUNCTION: Back door will test whether the ifindex can be joined to trunk.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingJoinedToTrunk(void)
{
    UI32_T ifindex;

    /* Can not get an ifindex. */
    if(FALSE == SYSCTRL_XOR_MGR_BackDoorGetIfindex(&ifindex))
    {
        return FALSE;
    }

    /* The ifindex can be joined to trunk. */
    if(TRUE == SYSCTRL_XOR_MGR_PermitBeingJoinedToTrunk(ifindex))
    {
        printf(" Yes, ifindex %ld can be joined to trunk.\n", ifindex);
        return TRUE;
    }

    printf(" No, ifindex %ld can NOT be joined to trunk.\n", ifindex);
    return FALSE;
} /* End of SYSCTRL_XOR_MGR_BackDoorPermitBeingJoinedToTrunk() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToPrivatePort
 * -------------------------------------------------------------------------
 * FUNCTION: Back door will test whether the ifindex can be set to private port.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToPrivatePort(void)
{
    UI32_T ifindex;
    UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    /* Can not get an ifindex. */
    if(FALSE == SYSCTRL_XOR_MGR_BackDoorGetIfindex(&ifindex))
    {
        return FALSE;
    }

    /* The ifindex is not existing (too large) */
    if(SYS_ADPT_TOTAL_NBR_OF_LPORT < ifindex)
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is not existing (too large).\r\n", __FUNCTION__, ifindex);
        }

        return FALSE;
    }

    /* Clear downlink port list array */
    memset(downlink_port_list, 0, sizeof(downlink_port_list));

    /* Transfer ifindex to downlink port list */
    downlink_port_list[(ifindex-1)/8] = 0x80 >> ((ifindex-1)%8);

    /* The ifindex can be set to private. */
    if(TRUE == SYSCTRL_XOR_MGR_PermitBeingSetToPrivatePort(downlink_port_list))
    {
        printf(" Yes, ifindex %ld can be set to private port.\n", ifindex);
        return TRUE;
    }

    printf(" No, ifindex %ld can NOT be set to private port.\n", ifindex);
    return FALSE;
} /* End of SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToPrivatePort() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToMirror
 * -------------------------------------------------------------------------
 * FUNCTION: Back door will test whether the ifindex can set to
 *           mirror_to/mirrored port.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToMirror(void)
{
    UI32_T mirror_to_ifindex, mirrored_ifindex;

    printf(" Mirror to");

    /* Can not get mirror to port. */
    if(FALSE == SYSCTRL_XOR_MGR_BackDoorGetIfindex(&mirror_to_ifindex))
    {
        return FALSE;
    }

    printf(" Mirrored ");

    /* Can not get mirrored port. */
    if(FALSE == SYSCTRL_XOR_MGR_BackDoorGetIfindex(&mirrored_ifindex))
    {
        return FALSE;
    }

    /* The mirror to ifindex/mirrored ifindex can be set to mirror to port/mirrored port */
    if(TRUE == SYSCTRL_XOR_MGR_PermitBeingSetToMirror(mirrored_ifindex, mirror_to_ifindex))
    {
        printf(" Yes, ifindex %ld can be set to mirror to port.\n", mirror_to_ifindex);
        printf(" Yes, ifindex %ld can be set to mirrored port.\n", mirrored_ifindex);
        return TRUE;
    }

    printf(" No, ifindex %ld can NOT be set to mirror to port.\n", mirror_to_ifindex);
    printf(" No, ifindex %ld can NOT be set to mirrored port.\n", mirrored_ifindex);
    return FALSE;
} /* End of SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToMirror() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToSecurityPort
 * -------------------------------------------------------------------------
 * FUNCTION: Back door will test whether the ifindex can be joined to trunk.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE : Successfully (permit)
 *           FALSE: Failed (not permit)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToSecurityPort(void)
{
    UI32_T ifindex;

    /* Can not get an ifindex. */
    if(FALSE == SYSCTRL_XOR_MGR_BackDoorGetIfindex(&ifindex))
    {
        return FALSE;
    }

    /* The ifindex can be set to security port. */
    if(TRUE == SYSCTRL_XOR_MGR_PermitBeingSetToSecurityPort(ifindex))
    {
        printf(" Yes, ifindex %ld can be set to security port.\n", ifindex);
        return TRUE;
    }

    printf(" No, ifindex %ld can NOT be set to security port.\n", ifindex);
    return FALSE;
} /* End of SYSCTRL_XOR_MGR_BackDoorPermitBeingSetToSecurityPort() */
#endif /* #if 0 */

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
BOOL_T SYSCTRL_XOR_MGR_PermitBeingSetRspanEntry( UI8_T target_port, UI8_T port_role )
{
    UI32_T xor_rule;

    /* The ifindex is not existing */
    if (FALSE == SYSCTRL_XOR_MGR_IsExistingPort(target_port))
    {
        return FALSE;
    }

    /* The ifindex is a trunk port */
    if (TRUE == SYSCTRL_XOR_MGR_IsTrunkPort(target_port))
    {
        return FALSE;
    }

    switch (port_role)
    {
    case LEAF_rspanSrcTxPorts:
    case LEAF_rspanSrcRxPorts:
        xor_rule = SYS_DFLT_XOR_SET_TO_RSPAN_MIRRORED_PORT_RULE;
        break;

    case LEAF_rspanDstPort:
        xor_rule = SYS_DFLT_XOR_SET_TO_RSPAN_MIRROR_TO_PORT_RULE;
        break;

    case LEAF_rspanRemotePorts:
        xor_rule = SYS_DFLT_XOR_SET_TO_RSPAN_UPLINK_PORT_RULE;
        break;

    default:
        return FALSE;
    }

    /* Check XOR rules */
    if (!SYSCTRL_XOR_MGR_PermitWithIfindexByRule(xor_rule, target_port))
    {
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsRspanMirrorToPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN
 *           destination port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN destination port
 *           FALSE: The ifindex is not a RSPAN destination port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsRspanMirrorToPort(UI32_T ifindex)
{
    if ( ! RSPAN_OM_IsRspanMirrorToPort (ifindex) )
        return FALSE ;

    if(TRUE == sysctrl_xor_mgr_debug_print)
    {
        printf("%s(): ifindex %ld is a RSPAN mirror to port.\r\n", __FUNCTION__, ifindex);
    }

    return TRUE ;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsRspanMirroredPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsRspanMirroredPort(UI32_T ifindex)
{
    if ( ! RSPAN_OM_IsRspanMirroredPort (ifindex) )
        return FALSE ;

    if(TRUE == sysctrl_xor_mgr_debug_print)
    {
        printf("%s(): ifindex %ld is a RSPAN mirrored port.\r\n", __FUNCTION__, ifindex);
    }

    return TRUE ;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsRspanUplinkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsRspanUplinkPort(UI32_T ifindex)
{
    if ( ! RSPAN_OM_IsRspanUplinkPort (ifindex) )
        return FALSE ;

    if(TRUE == sysctrl_xor_mgr_debug_print)
    {
        printf("%s(): ifindex %ld is a RSPAN uplink port.\r\n", __FUNCTION__, ifindex);
    }

    return TRUE ;
}
#endif /* End of #if (SYS_CPNT_RSPAN == TRUE) */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsVlanMacMirrorToPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a VLAN/MAC
 *           mirror-to port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a VLAN/MAC mirror-to port
 *           FALSE: The ifindex is not a VLAN/MAC mirror-to port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsVlanMacMirrorToPort(UI32_T ifindex)
{
    UI32_T valn_mac_mirror_to_ifindex;

    SWCTRL_POM_GetVlanAndMacMirrorDestPort(&valn_mac_mirror_to_ifindex);
    if(ifindex==valn_mac_mirror_to_ifindex)
        return TRUE;

    return FALSE;
}
#endif

#if (SYS_CPNT_MLAG == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsMlagPort
 * -------------------------------------------------------------------------
 * FUNCTION: Check whether an interface is a MLAG member or peer link.
 * INPUT   : ifindex -- interface index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsMlagPort(UI32_T ifindex)
{
    if (MLAG_POM_IsMlagPort(ifindex) == TRUE)
    {
        if (sysctrl_xor_mgr_debug_print == TRUE)
        {
            printf("%s: ifindex %lu is a MLAG port.\r\n", __func__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
}
#endif /* #if (SYS_CPNT_MLAG == TRUE) */



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYSCTRL_XOR_MGR_IsQosPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is QoS (ACL/Diffserv
 *           binded) port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is QoS port.
 *           FALSE: The ifindex is not QoS port.
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static BOOL_T SYSCTRL_XOR_MGR_IsQosPort(UI32_T ifindex)
{
    if (TRUE == RULE_OM_IsQosEnabledPort(ifindex))
    {
        if(TRUE == sysctrl_xor_mgr_debug_print)
        {
            printf("%s(): ifindex %ld is a qos port.\r\n", __FUNCTION__, ifindex);
        }

        return TRUE;
    }

    return FALSE;
}