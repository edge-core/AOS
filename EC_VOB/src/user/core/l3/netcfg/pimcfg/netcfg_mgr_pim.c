/*-----------------------------------------------------------------------------
* FILE NAME: NETCFG_MGR_PIM.C
*-----------------------------------------------------------------------------
* PURPOSE:
*
* NOTES:
*    None.
*
* HISTORY:
*    2008/05/18     --- Hongliang, Create
*
* Copyright(C)      Accton Corporation, 2008
*-----------------------------------------------------------------------------
*/
#include <string.h>
#include "sysfun.h"
#include "sys_type.h"
#include "ip_lib.h"
#include "netcfg_type.h"
#include "netcfg_mgr_pim.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "netcfg_om_pim.h"
#include "pim_pmgr.h"
#include "pim_type.h"
#include "vlan_lib.h"
#include "l_radix.h"
#include "l_string.h"

#define NETCFG_MGR_PIM_SYS_DFLT_VR_ID      0
#define NETCFG_MGR_PIM_SYS_DFLT_VRF_ID     0

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))

SYSFUN_DECLARE_CSC

static BOOL_T is_provision_complete = FALSE;

/* FUNCTION NAME : NETCFG_MGR_PIM_InitiateProcessResources
* PURPOSE:
*      Initialize NETCFG_MGR_PIM used system resource, eg. protection semaphore.
*      Clear all working space.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE  - Success
*      FALSE - Fail
*/
BOOL_T NETCFG_MGR_PIM_InitiateProcessResources(void)
{
    NETCFG_OM_PIM_Init();
    return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_EnterMasterMode
* PURPOSE:
*      Make Routing Engine enter master mode, handling all TCP/IP configuring requests,
*      and receiving packets.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      None.
*
* NOTES:
*
*/
void NETCFG_MGR_PIM_EnterMasterMode (void)
{
    NETCFG_OM_PIM_Init();
    SYSFUN_ENTER_MASTER_MODE();
} 

/* FUNCTION NAME : NETCFG_MGR_PIM_ProvisionComplete
* PURPOSE:
*      1. Let default gateway CFGDB into route when provision complete.
*
* INPUT:
*        None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      None.
*
*
* NOTES:
*
*/
void NETCFG_MGR_PIM_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
} 

/* FUNCTION NAME : NETCFG_MGR_PIM_EnterSlaveMode
* PURPOSE:
*      Make Routing Engine enter slave mode, discarding all TCP/IP configuring requests,
*      and receiving packets.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      None.
*
* NOTES:
*      1. In slave mode, just rejects function request and discard incoming message.
*/
void NETCFG_MGR_PIM_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetTransitionMode
* PURPOSE:
*      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
*      discarding TCP/IP configuring requests, and receiving packets.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      None.
*
* NOTES:
*      1. In Transition Mode, must make sure all messages in queue are read
*         and dynamic allocated space is free, resource set to INIT state.
*      2. All function requests and incoming messages should be dropped.
*/
void NETCFG_MGR_PIM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_PIM_EnterTransitionMode
* PURPOSE:
*      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
*      discarding TCP/IP configuring requests, and receiving packets.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      None.
*
* NOTES:
*      1. In Transition Mode, must make sure all messages in queue are read
*         and dynamic allocated space is free, resource set to INIT state.
*      2. All function requests and incoming messages should be dropped.
*/
void NETCFG_MGR_PIM_EnterTransitionMode (void)
{
    is_provision_complete = FALSE;
    SYSFUN_ENTER_TRANSITION_MODE();
}   
/* FUNCTION NAME : NETCFG_MGR_PIM_Debug
* PURPOSE:
*     PIM debug on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_EnableDenseMode(UI32_T ifindex)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.mode == NETCFG_TYPE_PIM_PIM_MODE_DENSE )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_EnableDenseMode(ifindex) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            entry.mode = NETCFG_TYPE_PIM_PIM_MODE_DENSE;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_DisableDenseMode(ifindex);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_DisableDenseMode
* PURPOSE:
*     Disable dense mode on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_DisableDenseMode(UI32_T ifindex)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.mode == NETCFG_TYPE_PIM_PIM_MODE_NONE )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_DisableDenseMode(ifindex) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            entry.mode = NETCFG_TYPE_PIM_PIM_MODE_NONE;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_EnableDenseMode(ifindex);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_EnableSparseMode
* PURPOSE:
*     Enable sparse mode on some interface.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_EnableSparseMode(UI32_T ifindex)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.mode == NETCFG_TYPE_PIM_PIM_MODE_SPARSE )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_EnableSparseMode(ifindex) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            entry.mode = NETCFG_TYPE_PIM_PIM_MODE_SPARSE;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_DisableSparseMode(ifindex);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_DisableSparseMode
* PURPOSE:
*     Disable sparse mode on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_DisableSparseMode(UI32_T ifindex)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.mode == NETCFG_TYPE_PIM_PIM_MODE_NONE )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_DisableSparseMode(ifindex) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            entry.mode = NETCFG_TYPE_PIM_PIM_MODE_NONE;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_EnableSparseMode(ifindex);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_SetHelloInterval
* PURPOSE:
*     set hello interval on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetHelloInterval(UI32_T ifindex, UI32_T interval)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    UI32_T old_interval;

    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.hello_period == interval )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetHelloInterval(ifindex,interval) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_interval = entry.hello_period;
            entry.hello_period = interval;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_SetHelloInterval(ifindex, old_interval);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_SetHelloHoldInterval
* PURPOSE:
*     set hello hold interval on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetHelloHoldInterval(UI32_T ifindex, UI32_T interval)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    UI32_T old_interval;

    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.holdtime == interval )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetHoldInterval(ifindex,interval) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_interval = entry.holdtime;
            entry.holdtime = interval;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_SetHoldInterval(ifindex, old_interval);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_UnSetNeighborFilter
* PURPOSE:
*     unset neighbor filter on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_UnSetNeighborFilter(UI32_T ifindex)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    UI8_T *old_filter_p;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.nbr_flt == NULL )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetNeighborFilter(ifindex,NULL) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_filter_p = entry.nbr_flt;
            entry.nbr_flt = NULL;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_SetNeighborFilter(ifindex, old_filter_p);
                return  NETCFG_TYPE_FAIL;
            }
            free(old_filter_p);
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_SetNeighborFilter
* PURPOSE:
*     set neighbor filter on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetNeighborFilter(UI32_T ifindex, UI8_T *flt)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    UI8_T old_filter[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
    BOOL_T old_filter_exist;

    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    if ( flt == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.nbr_flt != NULL && !strcmp(entry.nbr_flt,flt))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetNeighborFilter(ifindex,flt) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if ( entry.nbr_flt == NULL )
            {
                old_filter_exist = FALSE;
                entry.nbr_flt = malloc(strlen(flt) + 1);
            }
            else
            {
                old_filter_exist = TRUE;
                /* save the old value */
                strcpy(old_filter, entry.nbr_flt);
            }  
            strcpy(entry.nbr_flt, flt);
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_SetNeighborFilter(ifindex, old_filter_exist? old_filter : NULL);
                if ( old_filter_exist == FALSE )
                    free(entry.nbr_flt);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetExcludeGenID
* PURPOSE:
*     set pim to exclude generation ID on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetExcludeGenID(UI32_T ifindex, BOOL_T exclude_b)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    BOOL_T old_value;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.exclude_genid_yes == exclude_b )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetExcludeGenID(ifindex,exclude_b) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_value = entry.exclude_genid_yes;
            /* new value */
            entry.exclude_genid_yes = exclude_b;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_SetExcludeGenID(ifindex, exclude_b);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetDRPriority
* PURPOSE:
*     set DR priority on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetDRPriority(UI32_T ifindex, UI32_T priority)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    NETCFG_TYPE_PIM_If_T entry;
    UI32_T old_priority;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.dr_priority == priority )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetDrPriority(ifindex,priority) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_priority = entry.dr_priority;
            entry.dr_priority = priority;
            if(NETCFG_OM_PIM_UpdateInterfaceEntry(vr_id, &entry) != TRUE)
            {
                PIM_PMGR_SetDrPriority(ifindex, old_priority);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_UnSetAcceptRegisterList
* PURPOSE:
*     unset accepted register list on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_UnSetAcceptRegisterList(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI8_T *old_filter_p;
    BOOL_T old_filter_exist;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.rp_reg_filter == NULL )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetAcceptRegisterList(vr_id, vrf_id, NULL) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old pointer */
            old_filter_p = entry.rp_reg_filter;
            entry.rp_reg_filter = NULL;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetAcceptRegisterList(vr_id, vrf_id, old_filter_p);
                return  NETCFG_TYPE_FAIL;
            }
            free(entry.rp_reg_filter);
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetAcceptRegisterList
* PURPOSE:
*     set accepted register list on some interface
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetAcceptRegisterList(UI8_T *flt)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI8_T old_filter[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
    BOOL_T old_filter_exist;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    if ( flt == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.rp_reg_filter != NULL && !strcmp(entry.rp_reg_filter,flt))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetAcceptRegisterList(vr_id, vrf_id, flt) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if ( entry.rp_reg_filter == NULL )
            {
                old_filter_exist = FALSE;
                entry.rp_reg_filter = malloc(strlen(flt) + 1);
            }
            else
            {
                old_filter_exist = TRUE;
                /* save the old value */
                strcpy(old_filter, entry.rp_reg_filter);
            }            
            strcpy(entry.rp_reg_filter, flt);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetAcceptRegisterList(vr_id, vrf_id, old_filter_exist? old_filter : NULL );
                if ( old_filter_exist = FALSE )
                    free(entry.rp_reg_filter);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_EnableCrpPrefix
* PURPOSE:
*     Enable C-RP prefix
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_EnableCrpPrefix(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( FLAG_ISSET(entry.bsr.configs, NETCFG_OM_PIM_BSR_CONFIG_CRP_CISCO_PRIFIX))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_EnableCrpPrefix(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            SET_FLAG(entry.bsr.configs, NETCFG_OM_PIM_BSR_CONFIG_CRP_CISCO_PRIFIX);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_DisableCrpPrefix(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_DisableCrpPrefix
* PURPOSE:
*     Disable C-RP prefix
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_DisableCrpPrefix(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( !FLAG_ISSET(entry.bsr.configs, NETCFG_OM_PIM_BSR_CONFIG_CRP_CISCO_PRIFIX) )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_DisableCrpPrefix(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            UNSET_FLAG(entry.bsr.configs, NETCFG_OM_PIM_BSR_CONFIG_CRP_CISCO_PRIFIX);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_EnableCrpPrefix(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_IgnoreRpPriority
* PURPOSE:
*     Ignore RP priority
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_IgnoreRpPriority(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( FLAG_ISSET(entry.configs, NETCFG_OM_PIM_CONFIG_IGNORE_RP_SET_PRIORITY))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_IgnoreRpPriority(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            SET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_IGNORE_RP_SET_PRIORITY);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_NoIgnoreRpPriority(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_NoIgnoreRpPriority
* PURPOSE:
*     Dont ignore RP priority
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_NoIgnoreRpPriority(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( !FLAG_ISSET(entry.configs, NETCFG_OM_PIM_CONFIG_IGNORE_RP_SET_PRIORITY) )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_NoIgnoreRpPriority(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            UNSET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_IGNORE_RP_SET_PRIORITY);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_IgnoreRpPriority(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetJoinPruneInterval
* PURPOSE:
*     set joine/prune interval
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetJoinPruneInterval(UI32_T interval)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI32_T old_jp_interval;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.jp_periodic == interval )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetJoinPruneInerval(vr_id, vrf_id, interval) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_jp_interval = entry.jp_periodic;
            entry.jp_periodic = interval;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetJoinPruneInerval(vr_id, vrf_id, old_jp_interval);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_SetRegisterRateLimit
* PURPOSE:
*     set register rate limit
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetRegisterRateLimit(UI32_T limit)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI32_T old_limit;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.reg_rate_limit == limit )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetRegisterRateLimit(vr_id, vrf_id, limit) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_limit = entry.reg_rate_limit;
            entry.reg_rate_limit = limit;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetRegisterRateLimit(vr_id, vrf_id, old_limit);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetRegisterSuppressionTime
* PURPOSE:
*     set register suppression time
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetRegisterSuppressionTime(UI32_T time)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI32_T old_limit;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.reg_suppression == time )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetRegisterSuppressionTime(vr_id, vrf_id, time) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_limit = entry.reg_suppression;
            entry.reg_suppression = time;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetRegisterSuppressionTime(vr_id, vrf_id, old_limit);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetKat
* PURPOSE:
*     set register KAT(keep alive time)
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetKat(UI32_T time)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI32_T old_kat;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.rp_reg_kat == time )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetRegisterKAT(vr_id, vrf_id, time) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            /* save the old value */
            old_kat = entry.rp_reg_kat;
            entry.rp_reg_kat = time;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetRegisterKAT(vr_id, vrf_id, old_kat);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetRegisterChecksumGroupList
* PURPOSE:
*     set accepted register register checksum group list
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetRegisterChecksumGroupList(UI8_T *list)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI8_T old_filter[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
    BOOL_T old_filter_exist;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    if ( list == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.reg_checksum_filter != NULL && !strcmp(entry.reg_checksum_filter,list))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetRegisterChecksumList(vr_id, vrf_id, list) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if ( entry.reg_checksum_filter == NULL )
            {
                old_filter_exist = FALSE;
                entry.reg_checksum_filter = malloc(strlen(list) + 1);
            }
            else
            {
                old_filter_exist = TRUE;
                /* save the old value */
                strcpy(old_filter, entry.reg_checksum_filter);
            }            
            SET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_CISCO_REG_CKSUM);
            strcpy(entry.reg_checksum_filter, list);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetRegisterChecksumList(vr_id, vrf_id, old_filter_exist? old_filter : NULL );
                if ( old_filter_exist = FALSE )
                    free(entry.reg_checksum_filter);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_EnableRegisterChecksum
* PURPOSE:
*     Enable register checksum
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_EnableRegisterChecksum(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( FLAG_ISSET(entry.configs, NETCFG_OM_PIM_CONFIG_CISCO_REG_CKSUM) )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_EnableRegisterChecksum(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            SET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_CISCO_REG_CKSUM);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_DisableRegisterChecksum(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_DisableRegisterChecksum
* PURPOSE:
*     Disable register checksum
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_DisableRegisterChecksum(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( !FLAG_ISSET(entry.configs, NETCFG_OM_PIM_CONFIG_CISCO_REG_CKSUM) )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_DisableRegisterChecksum(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            UNSET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_CISCO_REG_CKSUM);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_EnableRegisterChecksum(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetPimSptThresholdInfinityGroupList
* PURPOSE:
*     set spt threshold infinity group list
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetPimSptThresholdInfinityGroupList(UI8_T *list)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    BOOL_T old_filter_exist;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    if ( list == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.spt_switch != NULL && !strcmp(entry.spt_switch,list))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetPimSptThresholdInfinityGroupList(vr_id, vrf_id, list) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if ( entry.spt_switch == NULL )
            {
                old_filter_exist = FALSE;
                entry.spt_switch = malloc(strlen(list) + 1);
            }
            else
            {
                old_filter_exist = TRUE;
            }            
            SET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_SPT_SWITCH);
            strcpy(entry.spt_switch, list);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_UnSetPimSptThresholdInfinity(vr_id, vrf_id);
                if ( old_filter_exist = FALSE )
                    free(entry.spt_switch);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetPimSptThresholdInfinity
* PURPOSE:
*     Set the SPT threshold infinity.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetPimSptThresholdInfinity(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( FLAG_ISSET(entry.configs, NETCFG_OM_PIM_CONFIG_SPT_SWITCH) )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetPimSptThresholdInfinity(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            SET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_SPT_SWITCH);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_UnSetPimSptThresholdInfinity(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_UnSetPimSptThresholdInfinity
* PURPOSE:
*     Unset SPT threshold infinity
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_UnSetPimSptThresholdInfinity(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( !FLAG_ISSET(entry.configs, NETCFG_OM_PIM_CONFIG_SPT_SWITCH) )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_UnSetPimSptThresholdInfinity(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            UNSET_FLAG(entry.configs, NETCFG_OM_PIM_CONFIG_SPT_SWITCH);
            if ( entry.spt_switch )
                free(entry.spt_switch);
            entry.spt_switch = NULL;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetPimSptThresholdInfinity(vr_id, vrf_id);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_EnableCandidateBsr
* PURPOSE:
*     Enable candidate BSR
*
* INPUT:
*      ifName.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_EnableCandidateBsr(UI8_T *ifName)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    BOOL_T old_name_exist;
    
    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    if ( ifName == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.bsr.ifname != NULL && !strcmp(entry.bsr.ifname ,ifName))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_EnableCandidateBsr(vr_id, vrf_id, ifName) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if ( entry.bsr.ifname == NULL )
            {
                old_name_exist = FALSE;
                entry.bsr.ifname = malloc(strlen(ifName) + 1);
            }
            else
            {
                old_name_exist = TRUE;
            }            
            strcpy(entry.bsr.ifname, ifName);
            SET_FLAG(entry.bsr.configs,NETCFG_OM_PIM_BSR_CONFIG_CANDIDATE);
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_DisableCandidateBsr(vr_id, vrf_id);
                if ( old_name_exist = FALSE )
                    free(entry.bsr.ifname);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_DisableCandidateBsr
* PURPOSE:
*     Disable candidate bsr
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_DisableCandidateBsr(void)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    char *ifname;

    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    /* Get interface entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( !FLAG_ISSET(entry.bsr.configs,NETCFG_OM_PIM_BSR_CONFIG_CANDIDATE) )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_DisableCandidateBsr(vr_id, vrf_id) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            UNSET_FLAG(entry.bsr.configs, NETCFG_OM_PIM_BSR_CONFIG_CANDIDATE);
            ifname = entry.bsr.ifname;
            entry.bsr.ifname = NULL;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_EnableCandidateBsr(vr_id, vrf_id, ifname);
                return  NETCFG_TYPE_FAIL;
            }
            if ( ifname )
                free(ifname);
        }
    }
    
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_SetCandidateBsrHash
* PURPOSE:
*     Set candidate BSR hash length
*
* INPUT:
*      hash      -- the hash length.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetCandidateBsrHash(UI8_T *ifName, UI32_T hash)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI32_T old_value;

    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    if ( ifName == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.bsr.my_hash_masklen == hash )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetCandidateBsrHash(vr_id, vrf_id, ifName, hash) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {        
            old_value = entry.bsr.my_hash_masklen;
            entry.bsr.my_hash_masklen = hash;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetCandidateBsrHash(vr_id, vrf_id, ifName, old_value);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetCandidateBsrPriority
* PURPOSE:
*     Set candidate BSR priority
*
* INPUT:
*      priority      -- the priority value.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetCandidateBsrPriority(UI8_T *ifName, UI32_T prioriy)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_OM_PIM_Instance_T entry;
    UI32_T old_value;

    memset(&entry, 0, sizeof(NETCFG_OM_PIM_Instance_T));
    if ( ifName == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get instance entry */
    if(NETCFG_OM_PIM_GetInstanceEntry(vr_id, vrf_id, &entry) == FALSE)
    {   
        return NETCFG_TYPE_FAIL;
    }
    if( entry.bsr.my_priority == prioriy )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetCandidateBsrPriority(vr_id, vrf_id, ifName, prioriy) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {        
            old_value = entry.bsr.my_priority;
            entry.bsr.my_priority = prioriy;
            if(NETCFG_OM_PIM_UpdateInstanceEntry(vr_id, vrf_id, &entry) != TRUE)
            {
                PIM_PMGR_SetCandidateBsrPriority(vr_id, vrf_id, ifName, old_value);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetPimRpCandidate
* PURPOSE:
*     Add RP candidate
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetPimRpCandidate(UI8_T *ifName)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    struct netcfg_om_pim_crp crp;
    
    if ( ifName == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get rp candidate */
    if(NETCFG_OM_PIM_GetRpCandidate(vr_id, vrf_id, ifName, &crp) == TRUE)
    {   
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetPimRpCandidate(vr_id, vrf_id, ifName) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {            
            if(NETCFG_OM_PIM_SetPimRpCandidate(vr_id, vrf_id, ifName) != TRUE)
            {
                PIM_PMGR_UnSetPimRpCandidate(vr_id, vrf_id, ifName);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_UnSetPimRpCandidate
* PURPOSE:
*     Remove RP candidate
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_UnSetPimRpCandidate(UI8_T *ifName)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    struct netcfg_om_pim_crp crp;
    
    if ( ifName == NULL )
        return NETCFG_TYPE_FAIL;
    /* Get rp candidate */
    if(NETCFG_OM_PIM_GetRpCandidate(vr_id, vrf_id, ifName, &crp) == FALSE)
    {   
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_UnSetPimRpCandidate(vr_id, vrf_id, ifName) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {            
            if(NETCFG_OM_PIM_UnSetPimRpCandidate(vr_id, vrf_id, ifName) != TRUE)
            {
                PIM_PMGR_SetPimRpCandidate(vr_id, vrf_id, ifName);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetPimRpCandidateGroupAddr
* PURPOSE:
*     Add RP candidate
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetPimRpCandidateGroupAddr(UI8_T *ifName, UI32_T groupAddr, UI32_T maskAddr)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    struct netcfg_om_pim_crp crp;
    
    if ( ifName == NULL )
        return NETCFG_TYPE_FAIL;
    if ( NETCFG_OM_PIM_GetRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr) == TRUE )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetPimRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {            
            if(NETCFG_OM_PIM_SetPimRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr) != TRUE)
            {
                PIM_PMGR_UnSetPimRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_UnSetPimRpCandidateGroupAddr
* PURPOSE:
*     Unset RP candidate group address
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_UnSetPimRpCandidateGroupAddr(UI8_T *ifName, UI32_T groupAddr, UI32_T maskAddr)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    struct netcfg_om_pim_crp crp;
    
    if ( ifName == NULL )
        return NETCFG_TYPE_FAIL;
    if ( NETCFG_OM_PIM_GetRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr) == FALSE )
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_UnSetPimRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {            
            if(NETCFG_OM_PIM_UnSetPimRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr) != TRUE)
            {
                PIM_PMGR_SetPimRpCandidateGroupAddr(vr_id, vrf_id, ifName, groupAddr, maskAddr);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SetRpAddress
* PURPOSE:
*     set static RP address
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_SetRpAddress(UI32_T addr)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    
    /* Get RP */
    if(NETCFG_OM_PIM_GetRpAddress(vr_id, vrf_id, addr) == TRUE)
    {   
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_SetPimRpAddress(vr_id, vrf_id, addr) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_PIM_SetPimRpAddress(vr_id, vrf_id, addr) != TRUE)
            {
                PIM_PMGR_UnSetPimRpAddress(vr_id, vrf_id, addr);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_UnSetRpAddress
* PURPOSE:
*     remove static RP address
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      NETCFG_TYPE_FAIL/NETCFG_TYPE_OK
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_PIM_UnSetRpAddress(UI32_T addr)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    
    /* Get RP */
    if(NETCFG_OM_PIM_GetRpAddress(vr_id, vrf_id, addr) == FALSE)
    {   
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_UnSetPimRpAddress(vr_id, vrf_id, addr) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_PIM_UnSetPimRpAddress(vr_id, vrf_id, addr) != TRUE)
            {
                PIM_PMGR_SetPimRpAddress(vr_id, vrf_id, addr);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }
    
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SignalInterfaceAdd
* PURPOSE:
*     When add an l3 interface signal PIM.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_PIM_SignalInterfaceAdd(UI32_T ifindex)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;

    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) == TRUE)
    {   
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_InterfaceAdd(vr_id, vrf_id, ifindex) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_PIM_AddInterface(vr_id, ifindex) != TRUE)
            {
                PIM_PMGR_InterfaceDelete(vr_id, vrf_id, ifindex);
                return  NETCFG_TYPE_FAIL;
            }
        }
    }

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_SignalInterfaceDelete
* PURPOSE:
*     When delete an l3 interface signal PIM.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_PIM_SignalInterfaceDelete(UI32_T ifindex)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
    entry.ifindex = ifindex;

    if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) != TRUE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(PIM_PMGR_InterfaceDelete(vr_id, vrf_id, ifindex) != PIM_TYPE_RESULT_OK)
        {
            return  NETCFG_TYPE_FAIL;
        }
        else
        {
            NETCFG_OM_PIM_DeleteInterface(vr_id, ifindex);
            return NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_PIM_SignalRifUp
* PURPOSE:
*     When an l3 interface primary rif up signal PIM.
*
* INPUT:
*      ifindex,
*      ipaddr,
*      ipmask,
*      primary: flag for if is primary rif
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_PIM_SignalRifUp(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    if(primary == NETCFG_TYPE_MODE_PRIMARY)
    {
        memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
        entry.ifindex = ifindex;

        if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) != TRUE)
        {
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
        }
        
        if(entry.if_status != TRUE)
        {
            if(PIM_PMGR_InterfaceUp(vr_id, vrf_id, ifindex,ip_addr, ip_mask) != PIM_TYPE_RESULT_OK)
            {
                return  NETCFG_TYPE_FAIL;
            }
            else
            {
                if(NETCFG_OM_PIM_SignalIpAddrAdd(vr_id, vrf_id, ifindex, ip_addr, ip_mask) != NETCFG_TYPE_OK)
                {
                    PIM_PMGR_InterfaceDown(vr_id, vrf_id, ifindex,ip_addr, ip_mask);
                    return  NETCFG_TYPE_FAIL;
                }
            }
        }
    }
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_PIM_SignalRifDown
* PURPOSE:
*     When an l3 interface primary rif down signal PIM.
*
* INPUT:
*      ifindex,
*      ipaddr,
*      ipmask,
*      primary: flag for if is primary rif
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_PIM_SignalRifDown(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary)
{
    UI32_T   vr_id = NETCFG_MGR_PIM_SYS_DFLT_VR_ID;
    UI32_T   vrf_id = NETCFG_MGR_PIM_SYS_DFLT_VRF_ID;
    NETCFG_TYPE_PIM_If_T entry;
    
    if(primary == NETCFG_TYPE_MODE_PRIMARY)
    {
        memset(&entry, 0, sizeof(NETCFG_TYPE_PIM_If_T));
        entry.ifindex = ifindex;

        if(NETCFG_OM_PIM_GetInterfaceEntry(vr_id, &entry) != TRUE)
        {
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
        }

        if(entry.if_status != FALSE)
        {
            if(PIM_PMGR_InterfaceDown(vr_id, vrf_id, ifindex,ip_addr, ip_mask) != PIM_TYPE_RESULT_OK)
                return  NETCFG_TYPE_FAIL;
            else
            {
                if(NETCFG_OM_PIM_SignalIpAddrDelete(vr_id, vrf_id,ifindex,ip_addr, ip_mask) != NETCFG_TYPE_OK)
                {
                    PIM_PMGR_InterfaceUp(vr_id, vrf_id, ifindex,ip_addr, ip_mask);
                    return  NETCFG_TYPE_FAIL;
                }
            }
        }
    }
    return NETCFG_TYPE_OK;
}

/*-----------------------------------------------------------------------------
* FUNCTION NAME - NETCFG_MGR_PIM_HandleIPCReqMsg
*-----------------------------------------------------------------------------
* PURPOSE : Handle the ipc request message for PIM MGR.
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
BOOL_T NETCFG_MGR_PIM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;
    
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }
    
    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.result_bool = FALSE;
        msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
        return TRUE;
    }
    
    switch (msg_p->type.cmd)
    {
        case NETCFG_MGR_PIM_IPC_ENABLE_DENSE_MODE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_EnableDenseMode(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_DISABLE_DENSE_MODE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_DisableDenseMode(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_HELLO_INTERVAL:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetHelloInterval(msg_p->data.arg_grp1.arg1,msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_HELLO_HOLD_INTERVAL:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetHelloHoldInterval(msg_p->data.arg_grp1.arg1,msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_NEIGHBOR_FILTER:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetNeighborFilter(msg_p->data.arg_grp2.arg1,msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_NEIGHBOR_FILTER:
            msg_p->type.result_bool = NETCFG_MGR_PIM_UnSetNeighborFilter(msg_p->data.arg_grp2.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_PIM_EXCLUDE_GENID:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetExcludeGenID(msg_p->data.arg_grp3.arg1,msg_p->data.arg_grp3.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_DR_PRIORITY:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetDRPriority(msg_p->data.arg_grp1.arg1,msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_DR_PRIORITY:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetDRPriority(msg_p->data.arg_grp1.arg1, SYS_DFLT_PIM_DR_PRIORITY);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_ENABLE_SPARSE_MODE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_EnableSparseMode(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_DISABLE_SPARSE_MODE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_DisableSparseMode(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_ACCEPT_REGISTER_LIST:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetAcceptRegisterList(msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_ACCEPT_REGISTER_LIST:
            msg_p->type.result_bool = NETCFG_MGR_PIM_UnSetAcceptRegisterList();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_ENABLE_CRP_PREFIX:
            msg_p->type.result_bool = NETCFG_MGR_PIM_EnableCrpPrefix();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_DISABLE_CRP_PREFIX:
            msg_p->type.result_bool = NETCFG_MGR_PIM_DisableCrpPrefix();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_IGNORE_RP_PRIORITY:
            msg_p->type.result_bool = NETCFG_MGR_PIM_IgnoreRpPriority();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_NO_IGNORE_RP_PRIORITY:
            msg_p->type.result_bool = NETCFG_MGR_PIM_NoIgnoreRpPriority();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_JOIN_PRUNE_INTERVAL:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetJoinPruneInterval(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_JOIN_PRUNE_INTERVAL:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetJoinPruneInterval(SYS_DFLT_PIM_JOINE_PRUNE_PERIODIC);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_RATE_LIMIT:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetRegisterRateLimit(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_REGISTER_RATE_LIMIT:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetRegisterRateLimit(SYS_DFLT_PIM_REGISTER_RATE_LIMIT);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_SUPPRESSION_TIME:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetRegisterSuppressionTime(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_REGISTER_SUPPRESSION_TIME:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetRegisterSuppressionTime(SYS_DFLT_PIM_REGISTER_SUPPRESSION_TIME);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_RP_ADDRESS:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetRpAddress(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_RP_ADDRESS:
            msg_p->type.result_bool = NETCFG_MGR_PIM_UnSetRpAddress(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_KAT_TIME:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetKat(msg_p->data.ui32_v);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_PIM_KAT_TIME:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetKat(SYS_DFLT_PIM_REGISTER_KAT);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_CHECKSUM:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetRegisterChecksumGroupList(msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;            
        case NETCFG_MGR_PIM_IPC_ENABLE_REGISTER_CHECKSUM:
            msg_p->type.result_bool = NETCFG_MGR_PIM_EnableRegisterChecksum();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_DISABLE_REGISTER_CHECKSUM:
            msg_p->type.result_bool = NETCFG_MGR_PIM_DisableRegisterChecksum();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;    
        case NETCFG_MGR_PIM_IPC_SET_RP_CANDIDATE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetPimRpCandidate(msg_p->data.arg_grp4.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_RP_CANDIDATE_GROUP_ADDR:
            msg_p->type.result_bool = 
                             NETCFG_MGR_PIM_SetPimRpCandidateGroupAddr(msg_p->data.arg_grp4.arg1,
                                                                       msg_p->data.arg_grp4.arg2,
                                                                       msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_RP_CANDIDATE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_UnSetPimRpCandidate(msg_p->data.arg_grp4.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_UNSET_RP_CANDIDATE_GROUP_ADDR:
            msg_p->type.result_bool =
                            NETCFG_MGR_PIM_UnSetPimRpCandidateGroupAddr(msg_p->data.arg_grp4.arg1,
                                                                        msg_p->data.arg_grp4.arg2,
                                                                        msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_ENABLE_BSR_CANDIDATE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_EnableCandidateBsr(msg_p->data.arg_grp4.arg1);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_DISABLE_BSR_CANDIDATE:
            msg_p->type.result_bool = NETCFG_MGR_PIM_DisableCandidateBsr();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_BSR_CANDIDATE_HASH:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetCandidateBsrHash(msg_p->data.arg_grp4.arg1,
                                                                         msg_p->data.arg_grp4.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_BSR_CANDIDATE_PRIORITY:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetCandidateBsrPriority(msg_p->data.arg_grp4.arg1,
                                                                             msg_p->data.arg_grp4.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_SPT_THRESHOLD_INFINITY:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetPimSptThresholdInfinity();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;            
        case NETCFG_MGR_PIM_IPC_UNSET_SPT_THRESHOLD_INFINITY:
            msg_p->type.result_bool = NETCFG_MGR_PIM_UnSetPimSptThresholdInfinity();
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_PIM_IPC_SET_SPT_THRESHOLD_INFINITY_WITH_GROUP_LIST:
            msg_p->type.result_bool = NETCFG_MGR_PIM_SetPimSptThresholdInfinityGroupList(msg_p->data.arg_grp2.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
            break;    

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_bool = TRUE;
            msg_p->type.result_ui32 = NETCFG_TYPE_FAIL;
            msgbuf_p->msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    }
    
    return TRUE;
} 

