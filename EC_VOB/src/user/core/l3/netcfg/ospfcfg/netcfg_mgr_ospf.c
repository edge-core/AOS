/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_OSPF.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/11/27     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#include <string.h>
#include "sysfun.h"
#include "sys_type.h"
#include "ip_lib.h"
#include "netcfg_om_ospf.h"
#include "netcfg_type.h"
#include "netcfg_mgr_ospf.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "ospf_pmgr.h"
#include "vlan_lib.h"
#include "ospf_type.h"
#include "l_radix.h"
#include "l_string.h"
#include "l_ls_prefix.h"
#include "netcfg_om_ip.h"
#include "l4_pmgr.h"

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))

SYSFUN_DECLARE_CSC


static BOOL_T is_provision_complete = FALSE;

//static UI32_T NETCFG_MGR_OSPF_RedistributeTypeCheck(char* protocol);

#if 0
/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeTypeCheck
 * PURPOSE:
 *      Check the protocol type
 *
 * INPUT:
 *      protocol.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OSPF_Redistribute_Connected/
 *      NETCFG_TYPE_OSPF_Redistribute_Static/
 *      NETCFG_TYPE_OSPF_Redistribute_Rip/
 *      NETCFG_TYPE_OSPF_Redistribute_Max
 */
static UI32_T NETCFG_MGR_OSPF_RedistributeTypeCheck(char* protocol)
{
    if (! strcmp ("connected", protocol))
        return NETCFG_TYPE_OSPF_Redistribute_Connected;
    else if (! strcmp ("static", protocol))
        return NETCFG_TYPE_OSPF_Redistribute_Static;
    else if (! strcmp ("rip", protocol))
        return NETCFG_TYPE_OSPF_Redistribute_Rip;
    else
        return NETCFG_TYPE_OSPF_Redistribute_Max;
}
#endif

/* FUNCTION NAME : NETCFG_MGR_OSPF_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_OSPF used system resource, eg. protection semaphore.
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
BOOL_T NETCFG_MGR_OSPF_InitiateProcessResources(void)
{
    NETCFG_OM_OSPF_Init();
    return TRUE;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_EnterMasterMode
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
void NETCFG_MGR_OSPF_EnterMasterMode (void)
{
    NETCFG_OM_OSPF_Init();
    SYSFUN_ENTER_MASTER_MODE();
} 

/* FUNCTION NAME : NETCFG_MGR_OSPF_ProvisionComplete
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
void NETCFG_MGR_OSPF_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
} 

/* FUNCTION NAME : NETCFG_MGR_OSPF_EnterSlaveMode
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
void NETCFG_MGR_OSPF_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_SetTransitionMode
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
void NETCFG_MGR_OSPF_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_EnterTransitionMode
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
void NETCFG_MGR_OSPF_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    NETCFG_OM_OSPF_DeleteAllOspfMasterEntry();
    is_provision_complete = FALSE;
}   

/* FUNCTION NAME : NETCFG_MGR_OSPF_SummaryAddrSet
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      address.
*      mask
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_SummaryAddrSet(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask)
{   
    int ret;
  
    ret = NETCFG_OM_OSPF_GetSummaryAddr(vr_id, proc_id, address, mask);
    if ( ret == NETCFG_TYPE_ENTRY_EXIST )
        return NETCFG_TYPE_OK;   
    /* SET */ 
    if(OSPF_PMGR_SummaryAddressSet(vr_id, proc_id, address, mask) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_AddSummaryAddr(vr_id, proc_id, address, mask) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_SummaryAddressUnset
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      address.
*      mask
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_SummaryAddrUnset(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask)
{   
    int ret;
  
    ret = NETCFG_OM_OSPF_GetSummaryAddr(vr_id, proc_id, address, mask);
    if ( ret != NETCFG_TYPE_ENTRY_EXIST )
        return NETCFG_TYPE_OK;   
    /* Unset */ 
    if(OSPF_PMGR_SummaryAddressUnset(vr_id, proc_id, address, mask) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DelSummaryAddr(vr_id, proc_id, address, mask) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_AutoCostSet
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ref_bandwidth.
*      
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AutoCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T ref_bandwidth)
{     
    if(OSPF_PMGR_AutoCostSet(vr_id, proc_id, ref_bandwidth) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_SetAutoCostRefBandwidth(vr_id, proc_id, ref_bandwidth) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_AutoCostUnset
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      
*      
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AutoCostUnset(UI32_T vr_id, UI32_T proc_id)
{     
    if(OSPF_PMGR_AutoCostUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_UnsetAutoCostRefBandwidth(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeProtoSet
* PURPOSE:
*     Set ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeProtoSet(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{     
    if(OSPF_PMGR_RedistributeProtoSet(vr_id, proc_id, proto_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeProtoSet(vr_id, proc_id, proto_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeProtoUnset
* PURPOSE:
*     Unset ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeProtoUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{     
    if(OSPF_PMGR_RedistributeProtoUnset(vr_id, proc_id, proto_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeProtoUnset(vr_id, proc_id, proto_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeMetricTypeSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric_type)
{     
    if(OSPF_PMGR_RedistributeMetricTypeSet(vr_id, proc_id, proto_type, metric_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeMetricTypeSet(vr_id, proc_id, proto_type, metric_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeMetricTypeUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{     
    if(OSPF_PMGR_RedistributeMetricTypeUnset(vr_id, proc_id, proto_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeMetricTypeUnset(vr_id, proc_id, proto_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeMetricSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric)
{     
    if(OSPF_PMGR_RedistributeMetricSet(vr_id, proc_id, proto_type, metric) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeMetricSet(vr_id, proc_id, proto_type, metric) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeMetricUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{     
    if(OSPF_PMGR_RedistributeMetricUnset(vr_id, proc_id, proto_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeMetricUnset(vr_id, proc_id, proto_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeTagSet
* PURPOSE:
*     Set ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      tag
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeTagSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T tag)
{     
    if(OSPF_PMGR_RedistributeTagSet(vr_id, proc_id, proto_type, tag) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeTagSet(vr_id, proc_id, proto_type, tag) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeTagUnset
* PURPOSE:
*     Unset ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeTagUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{     
    if(OSPF_PMGR_RedistributeTagUnset(vr_id, proc_id, proto_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeTagUnset(vr_id, proc_id, proto_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *route_map)
{     
    if(OSPF_PMGR_RedistributeRoutemapSet(vr_id, proc_id, proto_type, route_map) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeRoutemapSet(vr_id, proc_id, proto_type, route_map) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_RedistributeRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RedistributeRoutemapUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{     
    if(OSPF_PMGR_RedistributeRoutemapUnset(vr_id, proc_id, proto_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_RedistributeRoutemapUnset(vr_id, proc_id, proto_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoMetricTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric_type)
{     
    if(OSPF_PMGR_DefaultInfoMetricTypeSet(vr_id, proc_id, metric_type) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoMetricTypeSet(vr_id, proc_id, metric_type) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoMetricTypeUnset(UI32_T vr_id, UI32_T proc_id )
{     
    if(OSPF_PMGR_DefaultInfoMetricTypeUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoMetricTypeUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric)
{     
    if(OSPF_PMGR_DefaultInfoMetricSet(vr_id, proc_id, metric) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoMetricSet(vr_id, proc_id, metric) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoMetricUnset(UI32_T vr_id, UI32_T proc_id )
{     
    if(OSPF_PMGR_DefaultInfoMetricUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoMetricUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *route_map)
{     
    if(OSPF_PMGR_DefaultInfoRoutemapSet(vr_id, proc_id, route_map) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoRoutemapSet(vr_id, proc_id, route_map) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}



/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoRoutemapUnset( UI32_T vr_id, UI32_T proc_id )
{     
    if(OSPF_PMGR_DefaultInfoRoutemapUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoRoutemapUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoAlwaysSet
* PURPOSE:
*     Set ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoAlwaysSet(UI32_T vr_id, UI32_T proc_id)
{     
    if(OSPF_PMGR_DefaultInfoAlwaysSet(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoAlwaysSet(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoAlwaysUnset
* PURPOSE:
*     Unset ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoAlwaysUnset(UI32_T vr_id, UI32_T proc_id)
{     
    if(OSPF_PMGR_DefaultInfoAlwaysUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoAlwaysUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoSet
* PURPOSE:
*     Set ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoSet(UI32_T vr_id, UI32_T proc_id)
{     
    if(OSPF_PMGR_DefaultInfoSet(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoSet(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultInfoUnset
* PURPOSE:
*     Unset ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultInfoUnset(UI32_T vr_id, UI32_T proc_id)
{     
    if(OSPF_PMGR_DefaultInfoUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultInfoUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }   
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextSummaryAddress
* PURPOSE:
*     Getnext summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetNextSummaryAddress(NETCFG_MGR_SUMMARY_CONFIG_T *summary_entry_p)
{
    int ret;
    UI32_T addr;
    UI32_T masklen;

    addr = summary_entry_p->addr;
    masklen = summary_entry_p->masklen;
    ret = NETCFG_OM_OSPF_GetNextSummaryAddr(summary_entry_p->vr_id, summary_entry_p->proc_id, &addr, &masklen);
    if ( ret == NETCFG_TYPE_OK )
    {
        summary_entry_p->addr = addr;
        summary_entry_p->masklen = masklen;
        return NETCFG_TYPE_OK;
    }
    return NETCFG_TYPE_FAIL;
}
/* FUNCTION NAME : NETCFG_MGR_OSPF_GetAutoCostRefBandwidth
* PURPOSE:
*      Get auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      refbw
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_MGR_OSPF_GetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id, UI32_T *refbw)
{    
    return NETCFG_OM_OSPF_GetAutoCostRefBandwidth(vr_id, proc_id, refbw);
}
/* FUNCTION NAME : NETCFG_MGR_OSPF_GetRedistributeConfig
* PURPOSE:
*      Get redistribute configuration inormation.
*
* INPUT:
*            
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_MGR_OSPF_GetRedistributeConfig(NETCFG_MGR_REDIST_CONFIG_T *redist_config)
{    
    int ret;
    UI32_T originate;
    NETCFG_OM_OSPF_REDIST_CONF_T redist_original;

    ret = NETCFG_OM_OSPF_GetRedistributeConfig( redist_config->vr_id,
                                                redist_config->proc_id,
                                                (char *)redist_config->proto,
                                                &redist_original );
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    ret = NETCFG_OM_OSPF_GetDefaultInfoAlways( redist_config->vr_id,
                                               redist_config->proc_id,
                                               &originate );
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;   
    redist_config->flags = redist_original.flags;
    redist_config->tag = redist_original.tag;
    redist_config->metric = redist_original.metric;
    redist_config->default_origin = originate;
    if ( CHECK_FLAG(redist_config->flags, OSPF_REDIST_ROUTE_MAP) )
        strncpy((char *)redist_config->route_map, 
                redist_original.route_map.name,
                strlen(redist_original.route_map.name));

    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_MGR_OSPF_GetDefaultInfoConfig
* PURPOSE:
*      Get default information configuration.
*
* INPUT:
*      redist_config
*      
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_MGR_OSPF_GetDefaultInfoConfig(NETCFG_MGR_REDIST_CONFIG_T *redist_config)
{    
    int ret;
    UI32_T originate;
    NETCFG_OM_OSPF_REDIST_CONF_T redist_original;

    ret = NETCFG_OM_OSPF_GetDefaultInfoConfig( redist_config->vr_id,
                                               redist_config->proc_id,
                                               &redist_original );
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;
    ret = NETCFG_OM_OSPF_GetDefaultInfoAlways( redist_config->vr_id,
                                               redist_config->proc_id,
                                               &originate );
    if ( ret != NETCFG_TYPE_OK )
        return NETCFG_TYPE_FAIL;    
    redist_config->flags = redist_original.flags;
    redist_config->tag = redist_original.tag;
    redist_config->metric = redist_original.metric;
    redist_config->default_origin = originate;
    if ( CHECK_FLAG(redist_config->flags, OSPF_REDIST_ROUTE_MAP) )
        strncpy((char *)redist_config->route_map, 
                redist_original.route_map.name,
                strlen(redist_original.route_map.name));

    return NETCFG_TYPE_OK;
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_OSPF_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for OSPF MGR.
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
BOOL_T NETCFG_MGR_OSPF_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }
    
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.result_bool = FALSE;
        msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    switch (msg_p->type.cmd)
    {
        case NETCFG_MGR_OSPF_IPC_ROUTEROSPFSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RouterOspfSet(msg_p->data.arg_grp2.arg1,
                                                                    msg_p->data.arg_grp2.arg2,
                                                                    msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_ROUTEROSPFUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RouterOspfUnset(msg_p->data.arg_grp2.arg1,
                                                                    msg_p->data.arg_grp2.arg2,
                                                                    msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONTYPESET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfAuthenticationTypeSet(msg_p->data.arg_grp7.arg1,
                                                                       msg_p->data.arg_grp7.arg2,
                                                                       msg_p->data.arg_grp7.arg3,
                                                                       msg_p->data.arg_grp7.arg4,
                                                                       msg_p->data.arg_grp7.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONTYPEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfAuthenticationTypeUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONKEYSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfAuthenticationKeySet(msg_p->data.arg_grp10.arg1,
                                                                       msg_p->data.arg_grp10.arg2,
                                                                       msg_p->data.arg_grp10.arg3,
                                                                       msg_p->data.arg_grp10.arg4,
                                                                       msg_p->data.arg_grp10.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONKEYUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfAuthenticationKeyUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFMESSAGEDIGESTKEYSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfMessageDigestKeySet(msg_p->data.arg_grp11.arg1,
                                                                       msg_p->data.arg_grp11.arg2,
                                                                       msg_p->data.arg_grp11.arg3,
                                                                       msg_p->data.arg_grp11.arg4,
                                                                       msg_p->data.arg_grp11.arg5,
                                                                       msg_p->data.arg_grp11.arg6);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFMESSAGEDIGESTKEYUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfMessageDigestKeyUnset(msg_p->data.arg_grp7.arg1,
                                                                       msg_p->data.arg_grp7.arg2,
                                                                       msg_p->data.arg_grp7.arg3,
                                                                       msg_p->data.arg_grp7.arg4,
                                                                       msg_p->data.arg_grp7.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFPRIORITYSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfPrioritySet(msg_p->data.arg_grp7.arg1,
                                                                       msg_p->data.arg_grp7.arg2,
                                                                       msg_p->data.arg_grp7.arg3,
                                                                       msg_p->data.arg_grp7.arg4,
                                                                       msg_p->data.arg_grp7.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFPRIORITYUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfPriorityUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFCOSTSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfCostSet(msg_p->data.arg_grp9.arg1,
                                                                       msg_p->data.arg_grp9.arg2,
                                                                       msg_p->data.arg_grp9.arg3,
                                                                       msg_p->data.arg_grp9.arg4,
                                                                       msg_p->data.arg_grp9.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFCOSTUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfCostUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFDEADINTERVALSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfDeadIntervalSet(msg_p->data.arg_grp9.arg1,
                                                                       msg_p->data.arg_grp9.arg2,
                                                                       msg_p->data.arg_grp9.arg3,
                                                                       msg_p->data.arg_grp9.arg4,
                                                                       msg_p->data.arg_grp9.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFDEADINTERVALUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfDeadIntervalUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFHELLOINTERVALSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfHelloIntervalSet(msg_p->data.arg_grp9.arg1,
                                                                       msg_p->data.arg_grp9.arg2,
                                                                       msg_p->data.arg_grp9.arg3,
                                                                       msg_p->data.arg_grp9.arg4,
                                                                       msg_p->data.arg_grp9.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFHELLOINTERVALUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfHelloIntervalUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFRETRANSMITINTERVALSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfRetransmitIntervalSet(msg_p->data.arg_grp9.arg1,
                                                                       msg_p->data.arg_grp9.arg2,
                                                                       msg_p->data.arg_grp9.arg3,
                                                                       msg_p->data.arg_grp9.arg4,
                                                                       msg_p->data.arg_grp9.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFRETRANSMITINTERVALUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfRetransmitIntervalUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFTRANSMITDELAYSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfTransmitDelaySet(msg_p->data.arg_grp9.arg1,
                                                                       msg_p->data.arg_grp9.arg2,
                                                                       msg_p->data.arg_grp9.arg3,
                                                                       msg_p->data.arg_grp9.arg4,
                                                                       msg_p->data.arg_grp9.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_IFTRANSMITDELAYUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_IfTransmitDelayUnset(msg_p->data.arg_grp8.arg1,
                                                                       msg_p->data.arg_grp8.arg2,
                                                                       msg_p->data.arg_grp8.arg3,
                                                                       msg_p->data.arg_grp8.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_NETWORKSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_NetworkSet(msg_p->data.arg_grp5.arg1,
                                                                 msg_p->data.arg_grp5.arg2,
                                                                 msg_p->data.arg_grp5.arg3,
                                                                 msg_p->data.arg_grp5.arg4,
                                                                 msg_p->data.arg_grp5.arg5,
                                                                 msg_p->data.arg_grp5.arg6);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_NETWORKUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_NetworkUnset(msg_p->data.arg_grp5.arg1,
                                                                   msg_p->data.arg_grp5.arg2,
                                                                   msg_p->data.arg_grp5.arg3,
                                                                   msg_p->data.arg_grp5.arg4,
                                                                   msg_p->data.arg_grp5.arg5,
                                                                   msg_p->data.arg_grp5.arg6);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_ROUTERIDSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RouterIdSet(msg_p->data.arg_grp2.arg1,
                                                                  msg_p->data.arg_grp2.arg2,
                                                                  msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_ROUTERIDUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RouterIdUnset(msg_p->data.arg_grp1.arg1,
                                                                    msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_OSPF_IPC_TIMERSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_TimerSet(msg_p->data.arg_grp3.arg1,
                                                               msg_p->data.arg_grp3.arg2,
                                                               msg_p->data.arg_grp3.arg3,
                                                               msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_OSPF_IPC_TIMERUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_TimerUnset(msg_p->data.arg_grp1.arg1,
                                                                 msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
            
        case NETCFG_MGR_OSPF_IPC_DEFAULTMETRICSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultMetricSet(msg_p->data.arg_grp2.arg1,
                                                                       msg_p->data.arg_grp2.arg2,
                                                                       msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_OSPF_IPC_DEFAULTMETRICUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultMetricUnset(msg_p->data.arg_grp1.arg1,
                                                                         msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_PASSIVEIFSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_PassiveIfSet(msg_p->data.arg_grp6.arg1,
                                                                   msg_p->data.arg_grp6.arg2,
                                                                   msg_p->data.arg_grp6.arg4,
                                                                   msg_p->data.arg_grp6.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_PASSIVEIFUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_PassiveIfUnset(msg_p->data.arg_grp6.arg1,
                                                                     msg_p->data.arg_grp6.arg2,
                                                                     msg_p->data.arg_grp6.arg4,
                                                                     msg_p->data.arg_grp6.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_COMPATIBLERFC1583SET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_CompatibleRfc1583Set(msg_p->data.arg_grp1.arg1,
                                                                           msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_COMPATIBLERFC1583UNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_CompatibleRfc1583Unset(msg_p->data.arg_grp1.arg1,
                                                                             msg_p->data.arg_grp1.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_SUMMARY_ADDRSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_SummaryAddrSet(msg_p->data.arg_grp3.arg1,
                                                                     msg_p->data.arg_grp3.arg2,
                                                                     msg_p->data.arg_grp3.arg3,
                                                                     msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_SUMMAYR_ADDRUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_SummaryAddrUnset(msg_p->data.arg_grp3.arg1,
                                                                       msg_p->data.arg_grp3.arg2,
                                                                       msg_p->data.arg_grp3.arg3,
                                                                       msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_AUTOCOSTSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AutoCostSet(msg_p->data.arg_grp2.arg1,
                                                                  msg_p->data.arg_grp2.arg2,
                                                                  msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_AUTOCOSTUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AutoCostUnset( msg_p->data.arg_grp2.arg1,
                                                                     msg_p->data.arg_grp2.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_PROTOSET:
              msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeProtoSet( msg_p->data.arg_grp6.arg1,
                                                                              msg_p->data.arg_grp6.arg2,
                                                                              msg_p->data.arg_grp6.arg4 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_PROTOUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeProtoUnset( msg_p->data.arg_grp6.arg1,
                                                                              msg_p->data.arg_grp6.arg2,
                                                                              msg_p->data.arg_grp6.arg4 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeMetricSet( msg_p->data.arg_grp6.arg1,
                                                                             msg_p->data.arg_grp6.arg2,
                                                                             msg_p->data.arg_grp6.arg4,
                                                                             msg_p->data.arg_grp6.arg3 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeMetricUnset( msg_p->data.arg_grp6.arg1,
                                                                               msg_p->data.arg_grp6.arg2,
                                                                               msg_p->data.arg_grp6.arg4 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICTYPESET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeMetricTypeSet( msg_p->data.arg_grp6.arg1,
                                                                                 msg_p->data.arg_grp6.arg2,
                                                                                 msg_p->data.arg_grp6.arg4,
                                                                                 msg_p->data.arg_grp6.arg3 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICTYPEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeMetricTypeUnset( msg_p->data.arg_grp6.arg1,
                                                                                   msg_p->data.arg_grp6.arg2,
                                                                                   msg_p->data.arg_grp6.arg4 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_ROUTEMAPSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeRoutemapSet( msg_p->data.arg_grp_route_map.arg1,
                                                                               msg_p->data.arg_grp_route_map.arg2,
                                                                               msg_p->data.arg_grp_route_map.arg3,
                                                                               msg_p->data.arg_grp_route_map.arg4 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_ROUTEMAPUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeRoutemapUnset( msg_p->data.arg_grp6.arg1,
                                                                                 msg_p->data.arg_grp6.arg2,
                                                                                 msg_p->data.arg_grp6.arg4 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_TAGSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeTagSet( msg_p->data.arg_grp6.arg1,
                                                                          msg_p->data.arg_grp6.arg2,
                                                                          msg_p->data.arg_grp6.arg4,
                                                                          msg_p->data.arg_grp6.arg3 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_TAGUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_RedistributeTagUnset( msg_p->data.arg_grp6.arg1,
                                                                            msg_p->data.arg_grp6.arg2,
                                                                            msg_p->data.arg_grp6.arg4 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoMetricSet( msg_p->data.arg_grp6.arg1,
                                                                            msg_p->data.arg_grp6.arg2,
                                                                            msg_p->data.arg_grp6.arg3 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoMetricUnset( msg_p->data.arg_grp6.arg1,
                                                                              msg_p->data.arg_grp6.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICTYPESET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoMetricTypeSet( msg_p->data.arg_grp6.arg1,
                                                                                msg_p->data.arg_grp6.arg2,
                                                                                msg_p->data.arg_grp6.arg3 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICTYPEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoMetricTypeUnset( msg_p->data.arg_grp6.arg1,
                                                                                  msg_p->data.arg_grp6.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ROUTEMAPSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoRoutemapSet( msg_p->data.arg_grp_route_map.arg1,
                                                                              msg_p->data.arg_grp_route_map.arg2,
                                                                              msg_p->data.arg_grp_route_map.arg3 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ROUTEMAPUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoRoutemapUnset( msg_p->data.arg_grp6.arg1,
                                                                                msg_p->data.arg_grp6.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ALWAYSSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoAlwaysSet( msg_p->data.arg_grp1.arg1, 
                                                                            msg_p->data.arg_grp1.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
    
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ALWWAYSUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoAlwaysUnset( msg_p->data.arg_grp1.arg1, 
                                                                              msg_p->data.arg_grp1.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_SET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoSet( msg_p->data.arg_grp1.arg1, 
                                                                      msg_p->data.arg_grp1.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;            
            break;
        case NETCFG_MGR_OSPF_IPC_DEFAULTINFO_UNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_DefaultInfoUnset( msg_p->data.arg_grp1.arg1, 
                                                                        msg_p->data.arg_grp1.arg2 );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;   
            
        case NETCFG_MGR_OSPF_IPC_GETOSPFIFENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetOspfIfEntry(msg_p->data.arg_grp12.arg1,
                                                                             &msg_p->data.arg_grp12.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
            break;

        case NETCFG_MGR_OSPF_IPC_GETOSPFIFENTRYBYIFINDEX:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetOspfIfEntryByIfindex(msg_p->data.arg_grp12.arg1,
                                                                             &msg_p->data.arg_grp12.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
            break;

        case NETCFG_MGR_OSPF_IPC_GETNEXTOSPFIFENTRY:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetNextOspfIfEntry(msg_p->data.arg_grp12.arg1,
                                                                             &msg_p->data.arg_grp12.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
            break;

        case NETCFG_MGR_OSPF_IPC_GETNEXTOSPFIFENTRYBYIFINDEX:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetNextOspfIfEntryByIfindex(msg_p->data.arg_grp12.arg1,
                                                                             &msg_p->data.arg_grp12.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
            break;
            
        case NETCFG_MGR_OSPF_IPC_GETRUNNINGIFENTRYBYIFINDEX:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetRunningIfEntryByIfindex(msg_p->data.arg_grp13.arg1,
                                                                             &msg_p->data.arg_grp13.arg2);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp13);
            break;
        case NETCFG_MGR_OSPF_IPC_GETNEXT_SUMMARY_ADDR:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetNextSummaryAddress(&msg_p->data.summary_addr_entry);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_p->data.summary_addr_entry);
            break;
        case NETCFG_MGR_OSPF_IPC_GET_REDIST_AUTOCOST:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetAutoCostRefBandwidth( msg_p->data.arg_grp2.arg1,
                                                                               msg_p->data.arg_grp2.arg2,
                                                                               &msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_p->data.arg_grp2);
            break;   
        case NETCFG_MGR_OSPF_IPC_GET_REDIST_CONFIG:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetRedistributeConfig( &msg_p->data.redist_entry );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_p->data.redist_entry);
            break;    
        case NETCFG_MGR_OSPF_IPC_GET_DEFAULT_INFO_ORIGINATE:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetDefaultInfoConfig( &msg_p->data.redist_entry );
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_p->data.redist_entry);
            break;    
 

        case NETCFG_MGR_OSPF_IPC_AREASTUBSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaStubSet(msg_p->data.arg_grp3.arg1,
                                                                  msg_p->data.arg_grp3.arg2,
                                                                  msg_p->data.arg_grp3.arg3,
                                                                  msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREASTUBUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaStubUnset(msg_p->data.arg_grp3.arg1,
                                                                    msg_p->data.arg_grp3.arg2,
                                                                    msg_p->data.arg_grp3.arg3,
                                                                    msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREASTUBNOSUMMARYSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaStubNoSummarySet(msg_p->data.arg_grp3.arg1,
                                                                           msg_p->data.arg_grp3.arg2,
                                                                           msg_p->data.arg_grp3.arg3,
                                                                           msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREASTUBNOSUMMARYUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaStubNoSummaryUnset(msg_p->data.arg_grp3.arg1,
                                                                             msg_p->data.arg_grp3.arg2,
                                                                             msg_p->data.arg_grp3.arg3,
                                                                             msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_OSPF_IPC_AREADEFAULTCOSTSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaDefaultCostSet(msg_p->data.arg_grp4.arg1,
                                                                         msg_p->data.arg_grp4.arg2,
                                                                         msg_p->data.arg_grp4.arg3,
                                                                         msg_p->data.arg_grp4.arg4,
                                                                         msg_p->data.arg_grp4.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREADEFAULTCOSTUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaDefaultCostUnset(msg_p->data.arg_grp3.arg1,
                                                                           msg_p->data.arg_grp3.arg2,
                                                                           msg_p->data.arg_grp3.arg3,
                                                                           msg_p->data.arg_grp3.arg4);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREARANGESET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaRangeSet(msg_p->data.arg_grp5.arg1,
                                                                   msg_p->data.arg_grp5.arg2,
                                                                   msg_p->data.arg_grp5.arg3,
                                                                   msg_p->data.arg_grp5.arg4,
                                                                   msg_p->data.arg_grp5.arg5,
                                                                   msg_p->data.arg_grp5.arg6);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREARANGENOADVERTISESET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaRangeNoAdvertiseSet(msg_p->data.arg_grp5.arg1,
                                                                              msg_p->data.arg_grp5.arg2,
                                                                              msg_p->data.arg_grp5.arg3,
                                                                              msg_p->data.arg_grp5.arg4,
                                                                              msg_p->data.arg_grp5.arg5,
                                                                              msg_p->data.arg_grp5.arg6);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
        
        case NETCFG_MGR_OSPF_IPC_AREARANGEUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaRangeUnset(msg_p->data.arg_grp5.arg1,
                                                                     msg_p->data.arg_grp5.arg2,
                                                                     msg_p->data.arg_grp5.arg3,
                                                                     msg_p->data.arg_grp5.arg4,
                                                                     msg_p->data.arg_grp5.arg5,
                                                                     msg_p->data.arg_grp5.arg6);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREANSSASET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaNssaSet(msg_p->data.arg_grp14.arg1,
                                                                  msg_p->data.arg_grp14.arg2,
                                                                  msg_p->data.arg_grp14.arg3,
                                                                  msg_p->data.arg_grp14.arg4,
                                                                  &(msg_p->data.arg_grp14.arg5));
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREANSSAUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaNssaUnset(msg_p->data.arg_grp4.arg1,
                                                                    msg_p->data.arg_grp4.arg2,
                                                                    msg_p->data.arg_grp4.arg3,
                                                                    msg_p->data.arg_grp4.arg4,
                                                                    msg_p->data.arg_grp4.arg5);
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_GETINSTANCEPARA:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetInstancePara(&(msg_p->data.arg_instance_para));
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_p->data.arg_instance_para);
            break;

        case NETCFG_MGR_OSPF_IPC_GETNEXTPASSIVEIF:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetNextPassiveIf(msg_p->data.arg_grp15.arg1,
                                                                       msg_p->data.arg_grp15.arg2,
                                                                       &(msg_p->data.arg_grp15.arg3));
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_p->data.arg_grp15);
            break;

        case NETCFG_MGR_OSPF_IPC_GETNEXTNETWORK:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_GetNextNetwork(msg_p->data.arg_grp16.arg1,
                                                                     msg_p->data.arg_grp16.arg2,
                                                                     &(msg_p->data.arg_grp16.arg3));
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_p->data.arg_grp16);
            break;

        case NETCFG_MGR_OSPF_IPC_AREAVLINKSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaVirtualLinkSet(msg_p->data.arg_grp17.arg1,
                                                                         msg_p->data.arg_grp17.arg2,
                                                                         msg_p->data.arg_grp17.arg3,
                                                                         msg_p->data.arg_grp17.arg4,
                                                                         &(msg_p->data.arg_grp17.arg5));
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;

        case NETCFG_MGR_OSPF_IPC_AREAVLINKUNSET:
            msg_p->type.result_ui32 = NETCFG_MGR_OSPF_AreaVirtualLinkUnset(msg_p->data.arg_grp17.arg1,
                                                                           msg_p->data.arg_grp17.arg2,
                                                                           msg_p->data.arg_grp17.arg3,
                                                                           msg_p->data.arg_grp17.arg4,
                                                                           &(msg_p->data.arg_grp17.arg5));
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
            break;
            
       
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_bool = FALSE;
            msgbuf_p->msg_size = NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE;
    }
    
    return TRUE;
} 



/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterOspfSet
* PURPOSE:
*     Set router ospf.
*
* INPUT:
*      msg_p -- ospf message buf.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RouterOspfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_Instance_T top;
    UI32_T instance_count;

    memset(&top, 0, sizeof(NETCFG_OM_OSPF_Instance_T));

    result = NETCFG_OM_OSPF_GetInstanceEntry(vr_id, proc_id, &top);
    if(result == NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST)
    {
        return NETCFG_TYPE_FAIL;
    }    
    else
    {
        if (NETCFG_OM_OSPF_GetInstanceCount(vr_id, &instance_count) != NETCFG_TYPE_OK ||
            instance_count >= SYS_ADPT_MAX_OSPF_PROCESS_NBR)
        {
            return NETCFG_TYPE_FAIL;
        }
    
        if(OSPF_PMGR_RouterOspfSet(vr_id, vrf_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_OSPF_InstanceAdd(vr_id, vrf_id, proc_id) != TRUE)
            {
                OSPF_PMGR_RouterOspfUnset(vr_id, vrf_id, proc_id);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                /* Enable trap OSPF packets when first instance added
                 */
                if (instance_count == 0)
                {
                    L4_PMGR_TrapPacket2Cpu(TRUE, RULE_TYPE_PacketType_OSPF);
                }
                
                return  NETCFG_TYPE_OK;
            }
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterOspfUnset
* PURPOSE:
*     Unset router ospf.
*
* INPUT:
*      msg_p -- ospf message buf.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RouterOspfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_Instance_T top;
    UI32_T instance_count;
    
    memset(&top, 0, sizeof(NETCFG_OM_OSPF_Instance_T));

    result = NETCFG_OM_OSPF_GetInstanceEntry(vr_id, proc_id, &top);
    
    if(result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST)
    {
        return NETCFG_TYPE_FAIL;
    }
    else if(result == NETCFG_TYPE_INSTANCE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }    
    else
    {
        if(OSPF_PMGR_RouterOspfUnset(vr_id, vrf_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else 
        {
            if(NETCFG_OM_OSPF_InstanceDelete(vr_id, vrf_id, proc_id) != TRUE)
            {
                OSPF_PMGR_RouterOspfSet(vr_id, vrf_id, proc_id);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                if (NETCFG_OM_OSPF_GetInstanceCount(vr_id, &instance_count) == NETCFG_TYPE_OK)
                {
                    /* Remove trap OSPF packet if no instance exist
                     */
                    if (instance_count == 0)
                    {
                        L4_PMGR_TrapPacket2Cpu(FALSE, RULE_TYPE_PacketType_OSPF);
                    }
                }

                return  NETCFG_TYPE_OK;
            }
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalInterfaceAdd
* PURPOSE:
*     When add an l3 interface signal OSPF.
*
* INPUT:
*      ifindex.
*      mtu
*      bandwidth
*      if_flags
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
UI32_T NETCFG_MGR_OSPF_SignalInterfaceAdd(UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID; 
    if(OSPF_PMGR_InterfaceAdd(vr_id, vrf_id, ifindex, mtu, bandwidth, if_flags) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return  NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultOspfInterfaceAdd(vr_id, ifindex) != TRUE)
        {
            OSPF_PMGR_InterfaceDelete(vr_id, vrf_id, ifindex);
            return  NETCFG_TYPE_FAIL;
        }
    }
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalInterfaceDelete
* PURPOSE:
*     When delete an l3 interface signal OSPF.
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
UI32_T NETCFG_MGR_OSPF_SignalInterfaceDelete(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    if(OSPF_PMGR_InterfaceDelete(vr_id, vrf_id, ifindex) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return  NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_DefaultOspfInterfaceDelete(vr_id, ifindex) != TRUE)
        {
            return  NETCFG_TYPE_FAIL;
        }
    }
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalLoopbackInterfaceAdd
* PURPOSE:
*     When add a loopback interface signal OSPF.
*
* INPUT:
*      ifindex.
*      if_flags
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
UI32_T NETCFG_MGR_OSPF_SignalLoopbackInterfaceAdd(UI32_T ifindex, UI16_T if_flags)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    UI32_T   vrf_id = SYS_DFLT_VRF_ID;
    if(OSPF_PMGR_InterfaceAdd(vr_id, vrf_id, ifindex, 0, 0, if_flags) != OSPF_TYPE_RESULT_SUCCESS)
        return  NETCFG_TYPE_FAIL;

    /* There's no need to store the loopback interface in OSPF_OM,
     * as nothing can be configured to the loopback interface for OSPF.
     */
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifCreate
* PURPOSE:
*     When an IP address create signal OSPF.
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
UI32_T NETCFG_MGR_OSPF_SignalRifCreate(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    NETCFG_OM_OSPF_IfParams_T oip;
    struct pal_in4_addr addr;
    UI32_T ret;
    
    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));
    addr.s_addr = ip_addr;
    ret = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, TRUE, addr, &oip);
    if(ret == NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_OK;
    }
    else if(ret != NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(OSPF_PMGR_IpAddressAdd(vr_id, ifindex, ip_addr, ip_mask, primary) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_OspfInterfaceAdd(vr_id, ifindex, ip_addr, ip_mask) != TRUE)
            {
                OSPF_PMGR_IpAddressDelete(vr_id, ifindex, ip_addr, ip_mask);
                return  NETCFG_TYPE_FAIL;
            }
                
        }
    }
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifDelete
* PURPOSE:
*     When an IP address Delete signal OSPF.
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
UI32_T NETCFG_MGR_OSPF_SignalRifDelete(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    NETCFG_OM_OSPF_IfParams_T oip;
    struct pal_in4_addr addr;
    UI32_T ret;
    
    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));
    addr.s_addr = ip_addr;
    ret = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, TRUE, addr, &oip);
    if(ret == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(ret != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(OSPF_PMGR_IpAddressDelete(vr_id, ifindex, ip_addr, ip_mask) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_OspfInterfaceDelete(vr_id, ifindex, ip_addr) != TRUE)
            {
                OSPF_PMGR_IpAddressAdd(vr_id, ifindex, ip_addr, ip_mask, primary);
                return  NETCFG_TYPE_FAIL;
            }
                
        }
    }
    return NETCFG_TYPE_OK;
    
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifUp
* PURPOSE:
*     When an l3 interface primary rif up signal OSPF.
*
* INPUT:
*      ifindex,
*     
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
UI32_T NETCFG_MGR_OSPF_SignalRifUp(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    OSPF_PMGR_InterfaceUp(vr_id, ifindex);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifDown
* PURPOSE:
*     When an l3 interface primary rif down signal OSPF.
*
* INPUT:
*      ifindex,
*     
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
UI32_T NETCFG_MGR_OSPF_SignalRifDown(UI32_T ifindex)
{
    UI32_T   vr_id = SYS_DFLT_VR_ID;
    OSPF_PMGR_InterfaceDown(vr_id, ifindex);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_IfAuthenticationTypeSet
* PURPOSE:
*     Set OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex 
*      type
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
    if (type != NETCFG_TYPE_OSPF_AUTH_NULL 
            && type != NETCFG_TYPE_OSPF_AUTH_SIMPLE
            && type != NETCFG_TYPE_OSPF_AUTH_CRYPTOGRAPHIC)
    {     
        return NETCFG_TYPE_FAIL; 
    }
    if(oip.auth_type == type)
        return NETCFG_TYPE_OK;
    if(OSPF_PMGR_IfAuthenticationTypeSet(vr_id, ifindex, type, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfAuthenticationTypeSet(vr_id, ifindex, type, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfAuthenticationTypeSet(vr_id, ifindex, oip.auth_type, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_IfAuthenticationTypeUnset
* PURPOSE:
*     Unset OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex 
*      type
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_AUTH_TYPE))
    {
        return NETCFG_TYPE_OK;
    }
    if(OSPF_PMGR_IfAuthenticationTypeUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfAuthenticationTypeUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfAuthenticationTypeSet(vr_id, ifindex, oip.auth_type, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfAuthenticationKeySet
* PURPOSE:
*      Set OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
    
    if(strcmp(oip.auth_simple, auth_key) == 0)
        return NETCFG_TYPE_OK;
    
    if(OSPF_PMGR_IfAuthenticationKeySet(vr_id, ifindex, auth_key, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfAuthenticationKeySet(vr_id, ifindex, auth_key, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfAuthenticationKeySet(vr_id, ifindex, oip.auth_simple, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfAuthenticationKeyUnset
* PURPOSE:
*      Unset OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_AUTH_SIMPLE))
    {
        return NETCFG_TYPE_OK;
    }
    
    if(OSPF_PMGR_IfAuthenticationKeyUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfAuthenticationKeyUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfAuthenticationKeySet(vr_id, ifindex, oip.auth_simple, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfMessageDigestKeySet
* PURPOSE:
*      Set OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;

    result = NETCFG_OM_OSPF_CheckCryptKeyExist(vr_id, ifindex, key_id, addr_flag, addr);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    }
    
    if(OSPF_PMGR_IfMessageDigestKeySet(vr_id, ifindex, key_id, auth_key, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfMessageDigestKeySet(vr_id, ifindex, key_id, auth_key, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfMessageDigestKeyUnset(vr_id, ifindex, key_id, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfMessageDigestKeyUnset
* PURPOSE:
*      Unset OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_CryptKey_T ck;

    memset(&ck, 0, sizeof(NETCFG_OM_OSPF_CryptKey_T));

    result = NETCFG_OM_OSPF_GetCryptKeyEntry(vr_id, ifindex, key_id, addr_flag, addr, &ck);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_OK;
    }    
    
    if(OSPF_PMGR_IfMessageDigestKeyUnset(vr_id, ifindex, key_id, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfMessageDigestKeyUnset(vr_id, ifindex, key_id, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfMessageDigestKeySet(vr_id, ifindex, key_id, ck.auth_key, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfPrioritySet
* PURPOSE:
*      Set OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      priority
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
   
    if(oip.priority == priority)
        return NETCFG_TYPE_OK;
   
    if(OSPF_PMGR_IfPrioritySet(vr_id, ifindex, priority, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfPrioritySet(vr_id, ifindex, priority, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfPrioritySet(vr_id, ifindex, oip.priority, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfPriorityUnset
* PURPOSE:
*      Unset OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_PRIORITY))
    {
        return NETCFG_TYPE_OK;
    }
  
    if(OSPF_PMGR_IfPriorityUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfPriorityUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfPrioritySet(vr_id, ifindex, oip.priority, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfCostSet
* PURPOSE:
*      Set OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      cost
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
   
    if (CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_OUTPUT_COST) && (oip.output_cost == cost))
        return NETCFG_TYPE_OK;
   
    if(OSPF_PMGR_IfCostSet(vr_id, ifindex, cost, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfCostSet(vr_id, ifindex, cost, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfCostSet(vr_id, ifindex, oip.output_cost, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfCostUnset
* PURPOSE:
*      Unset OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_OUTPUT_COST))
    {
        return NETCFG_TYPE_OK;
    }
  
    if(OSPF_PMGR_IfCostUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfCostUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfCostSet(vr_id, ifindex, oip.output_cost, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfDeadIntervalSet
* PURPOSE:
*      Set OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
   
    if(oip.dead_interval == interval)
        return NETCFG_TYPE_OK;
   
    if(OSPF_PMGR_IfDeadIntervalSet(vr_id, ifindex, interval, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfDeadIntervalSet(vr_id, ifindex, interval, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfDeadIntervalSet(vr_id, ifindex, oip.dead_interval, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfDeadIntervalUnset
* PURPOSE:
*      Unset OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL))
    {
        return NETCFG_TYPE_OK;
    }
  
    if(OSPF_PMGR_IfDeadIntervalUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfDeadIntervalUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfDeadIntervalSet(vr_id, ifindex, oip.dead_interval, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfHelloIntervalSet
* PURPOSE:
*      Set OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
   
    if(oip.hello_interval == interval)
        return NETCFG_TYPE_OK;
   
    if(OSPF_PMGR_IfHelloIntervalSet(vr_id, ifindex, interval, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfHelloIntervalSet(vr_id, ifindex, interval, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfHelloIntervalSet(vr_id, ifindex, oip.hello_interval, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfHelloIntervalUnset
* PURPOSE:
*      Unset OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_HELLO_INTERVAL))
    {
        return NETCFG_TYPE_OK;
    }
  
    if(OSPF_PMGR_IfHelloIntervalUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfHelloIntervalUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfHelloIntervalSet(vr_id, ifindex, oip.hello_interval, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfRetransmitIntervalSet
* PURPOSE:
*      Set OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
   
    if(oip.retransmit_interval == interval)
        return NETCFG_TYPE_OK;
   
    if(OSPF_PMGR_IfRetransmitIntervalSet(vr_id, ifindex, interval, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfRetransmitIntervalSet(vr_id, ifindex, interval, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfRetransmitIntervalSet(vr_id, ifindex, oip.retransmit_interval, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfRetransmitIntervalUnset
* PURPOSE:
*      Unset OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_RETRANSMIT_INTERVAL))
    {
        return NETCFG_TYPE_OK;
    }
  
    if(OSPF_PMGR_IfRetransmitIntervalUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfRetransmitIntervalUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfRetransmitIntervalSet(vr_id, ifindex, oip.retransmit_interval, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfTransmitDelaySet
* PURPOSE:
*       Set OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      delay
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;   
    } 
   
    if(oip.transmit_delay == delay)
        return NETCFG_TYPE_OK;
   
    if(OSPF_PMGR_IfTransmitDelaySet(vr_id, ifindex, delay, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfTransmitDelaySet(vr_id, ifindex, delay, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfTransmitDelaySet(vr_id, ifindex, oip.transmit_delay, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
    
}

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfTransmitDelayUnset
* PURPOSE:
*      Unset OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    UI32_T result = 0;
    NETCFG_OM_OSPF_IfParams_T oip;

    memset(&oip, 0, sizeof(NETCFG_OM_OSPF_IfParams_T));

    result = NETCFG_OM_OSPF_GetOspfInterfaceEntry(vr_id, ifindex, addr_flag, addr, &oip);
    if(result == NETCFG_TYPE_OSPF_IPINTERFACE_NOT_EXIST)
    {
        return NETCFG_TYPE_OK;
    }
    else if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_FAIL;
    }    
    if (! CHECK_FLAG(oip.config, NETCFG_OM_OSPF_IF_PARAM_TRANSMIT_DELAY))
    {
        return NETCFG_TYPE_OK;
    }
  
    if(OSPF_PMGR_IfTransmitDelayUnset(vr_id, ifindex, addr_flag, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else 
    {
        if(NETCFG_OM_OSPF_IfTransmitDelayUnset(vr_id, ifindex, addr_flag, addr) != TRUE)
        {
            OSPF_PMGR_IfTransmitDelaySet(vr_id, ifindex, oip.transmit_delay, addr_flag, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return  NETCFG_TYPE_OK;
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_NetworkSet
* PURPOSE:
*     Set ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_NetworkSet(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_OSPF_Network_T  network_entry;
    struct L_ls_prefix p;
    struct pal_in4_addr addr;
    
    memset(&addr, 0, sizeof(struct pal_in4_addr));
    memset(&network_entry, 0, sizeof(NETCFG_TYPE_OSPF_Network_T));
    memset(&p, 0, sizeof(struct L_ls_prefix));
    addr.s_addr = network_addr;
    L_ls_prefix_ipv4_set (&p, masklen, addr);
    network_entry.lp = &p;
    network_entry.format = format;
    network_entry.area_id = area_id;
    network_entry.network = network_addr;
    network_entry.masklen = masklen;

    result = NETCFG_OM_OSPF_GetNetworkEntry(vr_id, proc_id, &network_entry);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if(result == NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_OSPF_NETWORK_ALREADY_EXIST;
    }
    else
    {
        if(OSPF_PMGR_NetworkSet(vr_id, proc_id, network_addr, masklen, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_NetworkSet(vr_id, proc_id, network_entry) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_NetworkUnset(vr_id, proc_id, network_addr, masklen, area_id, format);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }

}

/* FUNCTION NAME : NETCFG_MGR_OSPF_NetworkUnset
* PURPOSE:
*     Delete ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_NetworkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_OSPF_Network_T  network_entry;
    struct L_ls_prefix p;
    struct pal_in4_addr addr;
    
    memset(&addr, 0, sizeof(struct pal_in4_addr));
    memset(&network_entry, 0, sizeof(NETCFG_TYPE_OSPF_Network_T));
    memset(&p, 0, sizeof(struct L_ls_prefix));
    addr.s_addr = network_addr;
    L_ls_prefix_ipv4_set (&p, masklen, addr);
    network_entry.lp = &p;
    network_entry.format = format;
    network_entry.area_id = area_id;
    network_entry.network = network_addr;
    network_entry.masklen = masklen;

    result = NETCFG_OM_OSPF_GetNetworkEntry(vr_id, proc_id, &network_entry);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if(result != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(network_entry.area_id != area_id)
        {
            return NETCFG_TYPE_OSPF_AREA_ID_NOT_MATCH;
        }
        
        if(OSPF_PMGR_NetworkUnset(vr_id, proc_id, network_addr, masklen, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_NetworkUnset(vr_id, proc_id, network_entry) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_NetworkSet(vr_id, proc_id, network_addr, masklen, network_entry.area_id, network_entry.format);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }

}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextNetwork
 * PURPOSE:
 *      Get next network .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_MGR_OSPF_GetNextNetwork(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry)
{
    UI32_T result;
    
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    result = NETCFG_OM_OSPF_GetNextNetworkEntry(vr_id,proc_id,entry);   
    return result;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterIdSet
* PURPOSE:
*     Set ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*      router_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    struct pal_in4_addr config_router_id;
    
    memset(&config_router_id, 0, sizeof(struct pal_in4_addr));

    result = NETCFG_OM_OSPF_GetRouterId(vr_id, proc_id, &config_router_id);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if((result == NETCFG_TYPE_OK) && (config_router_id.s_addr == router_id))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_RouterIdSet(vr_id, proc_id, router_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_RouterIdSet(vr_id, proc_id, router_id) != NETCFG_TYPE_OK)
            {
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterIdUnset
* PURPOSE:
*     Unset ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RouterIdUnset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    struct pal_in4_addr config_router_id;
    
    memset(&config_router_id, 0, sizeof(struct pal_in4_addr));

    result = NETCFG_OM_OSPF_GetRouterId(vr_id, proc_id, &config_router_id);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if(result == NETCFG_TYPE_OK)
    {
        if(OSPF_PMGR_RouterIdUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_RouterIdUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
            {
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_TimerSet
* PURPOSE:
*     Set ospf timer.
*
* INPUT:
*      vr_id,
*      proc_id,
*      delay,
*      hold.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_TimerSet(UI32_T vr_id, UI32_T proc_id, UI32_T delay, UI32_T hold)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_OSPF_Timer_T timer, timer_temp;
    
    memset(&timer, 0, sizeof(NETCFG_TYPE_OSPF_Timer_T));
    memset(&timer_temp, 0, sizeof(NETCFG_TYPE_OSPF_Timer_T));
    
    result = NETCFG_OM_OSPF_GetTimer(vr_id, proc_id, &timer_temp);
    if(result != NETCFG_TYPE_OK)
        return result;

    if(OSPF_PMGR_TimerSet(vr_id, proc_id, delay, hold) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        timer.delay = delay;
        timer.hold = hold;
        if(NETCFG_OM_OSPF_TimerSet(vr_id, proc_id, timer) != NETCFG_TYPE_OK)
        {
            OSPF_PMGR_TimerSet(vr_id, proc_id, timer_temp.delay, timer_temp.hold);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_TimerUnset
* PURPOSE:
*     Set ospf timer to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_TimerUnset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_OSPF_Timer_T timer_temp;
    
    memset(&timer_temp, 0, sizeof(NETCFG_TYPE_OSPF_Timer_T));
    
    result = NETCFG_OM_OSPF_GetTimer(vr_id, proc_id, &timer_temp);
    if(result != NETCFG_TYPE_OK)
        return result;

    if((timer_temp.delay != SYS_DFLT_OSPF_SPF_DELAY_DEFAULT) || (timer_temp.hold != SYS_DFLT_OSPF_SPF_HOLDTIME_DEFAULT))
    {
        if(OSPF_PMGR_TimerUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_TimerUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_TimerSet(vr_id, proc_id, timer_temp.delay, timer_temp.hold);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
    else
    {
        return NETCFG_TYPE_OK;
    }
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    UI32_T temp_metric;
    BOOL_T flag;

    result = NETCFG_OM_OSPF_GetDefaultMetric(vr_id, proc_id, &flag, &temp_metric);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if((flag == TRUE) && (temp_metric == metric))
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_DefaultMetricSet(vr_id, proc_id, metric) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_DefaultMetricSet(vr_id, proc_id, metric) != NETCFG_TYPE_OK)
            {
                if(flag)
                {
                    OSPF_PMGR_DefaultMetricSet(vr_id, proc_id, temp_metric);
                }
                else
                {
                    OSPF_PMGR_DefaultMetricUnset(vr_id, proc_id);
                }
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultMetricUnset
* PURPOSE:
*     Set ospf default metric to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    UI32_T temp_metric;
    BOOL_T flag;

    result = NETCFG_OM_OSPF_GetDefaultMetric(vr_id, proc_id, &flag, &temp_metric);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if(flag == FALSE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_DefaultMetricUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_DefaultMetricUnset(vr_id, proc_id) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_DefaultMetricSet(vr_id, proc_id, temp_metric);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_PassiveIfSet
* PURPOSE:
*     Set ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_PassiveIfSet(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr)
{
#if 0
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_OSPF_Passive_If_T pass_if;
    
    if(ifname == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&pass_if, 0, sizeof(NETCFG_TYPE_OSPF_Passive_If_T));  
    pass_if.addr = addr;
    if(NETCFG_MGR_OSPF_GetIfindexFromIfname(ifname, &(pass_if.ifindex))!= TRUE)
        return NETCFG_TYPE_INVALID_ARG;
    
    result = NETCFG_OM_OSPF_GetPassiveIf(vr_id, proc_id, &pass_if);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST)||
       (result == NETCFG_TYPE_INSTANCE_NOT_EXIST)||
       (result == NETCFG_TYPE_INVALID_ARG)||
       (result == NETCFG_TYPE_OK))
        return result;

    if(OSPF_PMGR_PassiveIfSet(vr_id, proc_id, ifname, addr) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        if(NETCFG_OM_OSPF_PassiveIfSet(vr_id, proc_id, pass_if.ifindex, addr) != NETCFG_TYPE_OK)
        {
            OSPF_PMGR_PassiveIfUnset(vr_id, proc_id, ifname, addr);
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            return NETCFG_TYPE_OK;
        }
    }
#endif
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_PassiveIfUnset
* PURPOSE:
*     Unset ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_PassiveIfUnset(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr)
{
#if 0
    UI32_T result = NETCFG_TYPE_FAIL;
    NETCFG_TYPE_OSPF_Passive_If_T pass_if;

    if(ifname == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    memset(&pass_if, 0, sizeof(NETCFG_TYPE_OSPF_Passive_If_T));  
    pass_if.addr = addr;
    if(NETCFG_MGR_OSPF_GetIfindexFromIfname(ifname, &(pass_if.ifindex))!= TRUE)
        return NETCFG_TYPE_INVALID_ARG;
    
    result = NETCFG_OM_OSPF_GetPassiveIf(vr_id, proc_id, &pass_if);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST)||
       (result == NETCFG_TYPE_INSTANCE_NOT_EXIST)||
       (result == NETCFG_TYPE_INVALID_ARG))
        return result;

    if(result == NETCFG_TYPE_CAN_NOT_GET)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_PassiveIfUnset(vr_id, proc_id, ifname, addr) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_PassiveIfUnset(vr_id, proc_id, pass_if.ifindex, addr) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_PassiveIfSet(vr_id, proc_id, ifname, addr);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
#endif
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextPassiveIf
 * PURPOSE:
 *      Get next passive interface .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_MGR_OSPF_GetNextPassiveIf(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Passive_If_T *entry)
{
    UI32_T result;
    
    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    result = NETCFG_OM_OSPF_GetNextPassiveIf(vr_id,proc_id,entry);   
    return result;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_CompatibleRfc1583Set
* PURPOSE:
*     Set ospf compatible rfc1853.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_CompatibleRfc1583Set(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    BOOL_T status;

    result = NETCFG_OM_OSPF_GetCompatibleRfc1583Status(vr_id, proc_id, &status);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if(status == TRUE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_CompatibleRfc1853Set(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_CompatibleRfc1583Set(vr_id, proc_id) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_CompatibleRfc1853Unset(vr_id, proc_id);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_CompatibleRfc1583Unset
* PURPOSE:
*     Unset ospf compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_CompatibleRfc1583Unset(UI32_T vr_id, UI32_T proc_id)
{
    UI32_T result = NETCFG_TYPE_FAIL;
    BOOL_T status;

    result = NETCFG_OM_OSPF_GetCompatibleRfc1583Status(vr_id, proc_id, &status);
    if((result == NETCFG_TYPE_OSPF_MASTER_NOT_EXIST) || (result == NETCFG_TYPE_INSTANCE_NOT_EXIST))
        return result;

    if(status == FALSE)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_CompatibleRfc1853Unset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_CompatibleRfc1583Unset(vr_id, proc_id) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_CompatibleRfc1853Set(vr_id, proc_id);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetOspfIfEntry
* PURPOSE:
*     Get ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_OM_IP_InetRifConfig_T rif;

    memset(&rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    //rif.addr = entry->ident.address;
    IP_LIB_UI32toArray(entry->ident.address.u.prefix4.s_addr, rif.addr.addr);
    
    if(NETCFG_OM_IP_GetInetRif(&rif) == NETCFG_TYPE_OK)
    {
        // entry->ident.address = rif.addr;
        IP_LIB_ArraytoUI32(rif.addr.addr, (UI32_T *)&(entry->ident.address.u.prefix4.s_addr));
        
        entry->ifindex = rif.ifindex;
        if(OSPF_PMGR_GetOspfInterfaceEntry(vr_id, entry) != OSPF_TYPE_RESULT_SUCCESS)
            return NETCFG_TYPE_FAIL;
        else
            return NETCFG_TYPE_OK;
    }
    
    return NETCFG_TYPE_FAIL;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetOspfIfEntryByIfindex
* PURPOSE:
*     Get ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_TYPE_InetRifConfig_T rif;

    memset(&rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    //rif.addr = entry->ident.address;
    IP_LIB_UI32toArray(entry->ident.address.u.prefix4.s_addr, rif.addr.addr);
    rif.ifindex = entry->ifindex;
    if(NETCFG_OM_IP_GetRifFromInterface(&rif) == NETCFG_TYPE_OK)
    {
        //entry->ident.address = rif.addr;
        IP_LIB_ArraytoUI32(rif.addr.addr, (UI32_T *)&(entry->ident.address.u.prefix4.s_addr));

        if(OSPF_PMGR_GetOspfInterfaceEntry(vr_id, entry) != OSPF_TYPE_RESULT_SUCCESS)
            return NETCFG_TYPE_FAIL;
        else
            return NETCFG_TYPE_OK;
    }
    
    return NETCFG_TYPE_FAIL;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextOspfIfEntry
* PURPOSE:
*     Get next ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetNextOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_TYPE_InetRifConfig_T rif;

    memset(&rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    //rif.addr = entry->ident.address;
    IP_LIB_UI32toArray(entry->ident.address.u.prefix4.s_addr, rif.addr.addr);
    rif.addr.type = L_INET_ADDR_TYPE_IPV4;
    rif.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

    while(NETCFG_OM_IP_GetNextIPv4RifConfig(&rif) == NETCFG_TYPE_OK)
    {
        //entry->ident.address = rif.addr;
        IP_LIB_ArraytoUI32(rif.addr.addr, (UI32_T *)&(entry->ident.address.u.prefix4.s_addr));

        entry->ifindex = rif.ifindex;
        if(OSPF_PMGR_GetOspfInterfaceEntry(vr_id, entry) != OSPF_TYPE_RESULT_SUCCESS)
            continue;
        else
            return NETCFG_TYPE_OK;
    }
    
    return NETCFG_TYPE_FAIL;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextOspfIfEntryByIfindex
* PURPOSE:
*     Get next ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetNextOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_OM_IP_InetRifConfig_T rif;

    memset(&rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

//    rif.addr = entry->ident.address;
    IP_LIB_UI32toArray(entry->ident.address.u.prefix4.s_addr, rif.addr.addr);
    rif.addr.type = L_INET_ADDR_TYPE_IPV4;
    rif.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

    while(NETCFG_OM_IP_GetNextInetRifByIfindex(&rif, entry->ifindex) == NETCFG_TYPE_OK)
    {
        //entry->ident.address = rif.addr;
        IP_LIB_ArraytoUI32(rif.addr.addr, (UI32_T *)&(entry->ident.address.u.prefix4.s_addr));

        if(OSPF_PMGR_GetOspfInterfaceEntry(vr_id, entry) != OSPF_TYPE_RESULT_SUCCESS)
            continue;
        else
            return NETCFG_TYPE_OK;
    }
    
    return NETCFG_TYPE_FAIL;
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetRunningIfEntryByIfindex
* PURPOSE:
*     Get ospf interface config information by ifindex.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetRunningIfEntryByIfindex(UI32_T vr_id, NETCFG_TYPE_OSPF_IfConfig_T *entry)
{
    NETCFG_OM_IP_InetRifConfig_T rif;

    memset(&rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    if(entry->default_flag)
    {
        if(NETCFG_OM_OSPF_GetRunningIfEntryByIfindex(vr_id, entry) != NETCFG_TYPE_OK)
            return NETCFG_TYPE_FAIL;
        
        entry->default_flag = FALSE;
        return NETCFG_TYPE_OK;
    }
    else
    {
        // rif.addr.u.prefix4.s_addr = entry->ip_address;
        IP_LIB_UI32toArray(entry->ip_address, rif.addr.addr);

        while(NETCFG_OM_IP_GetNextInetRifByIfindex(&rif, entry->ifindex) == NETCFG_TYPE_OK)
        {
            // entry->ip_address = rif.addr.u.prefix4.s_addr;
            IP_LIB_ArraytoUI32(rif.addr.addr, &(entry->ip_address));

            if(NETCFG_OM_OSPF_GetRunningIfEntryByIfindex(vr_id, entry) != NETCFG_TYPE_OK)
                continue;
            else
                return NETCFG_TYPE_OK;
        }
    }
 
    return NETCFG_TYPE_FAIL;
}
/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubSet
* PURPOSE:
*     Set ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    if(OSPF_PMGR_AreaStubSet(vr_id, proc_id, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubUnset
* PURPOSE:
*     Unset ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    if(OSPF_PMGR_AreaStubUnset(vr_id, proc_id, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubNoSummarySet
* PURPOSE:
*     Set ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    if(OSPF_PMGR_AreaStubNoSummarySet(vr_id, proc_id, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubNoSummaryUnset
* PURPOSE:
*     Unset ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubNoSummaryUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    if(OSPF_PMGR_AreaStubNoSummaryUnset(vr_id, proc_id, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaDefaultCostSet
* PURPOSE:
*     Set ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      cost.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost)
{
    #if 0
    UI32_T result = NETCFG_TYPE_FAIL;
    UI32_T default_cost;

    result = NETCFG_OM_OSPF_GetAreaDefaultCost(vr_id, proc_id, &default_cost);
    if(result != NETCFG_TYPE_OK)
        return result;

    if(default_cost == cost)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_AreaDefaultCostSet(vr_id, proc_id, area_id, format, cost) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_AreaDefaultCostSet(vr_id, proc_id, area_id, format, cost) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_AreaDefaultCostSet(vr_id, proc_id, area_id, format, default_cost);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
    #endif
    if(OSPF_PMGR_AreaDefaultCostSet(vr_id, proc_id, area_id, format, cost) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaDefaultCostUnset
* PURPOSE:
*     Unset ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    #if 0
    UI32_T result = NETCFG_TYPE_FAIL;
    UI32_T default_cost;

    result = NETCFG_OM_OSPF_GetAreaDefaultCost(vr_id, proc_id, &default_cost);
    if(result != NETCFG_TYPE_OK)
        return result;

    if(default_cost == SYS_DFLT_OSPF_STUB_DEFAULT_COST)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        if(OSPF_PMGR_AreaDefaultCostUnset(vr_id, proc_id, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
        {
            return NETCFG_TYPE_FAIL;
        }
        else
        {
            if(NETCFG_OM_OSPF_AreaDefaultCostUnset(vr_id, proc_id, area_id, format) != NETCFG_TYPE_OK)
            {
                OSPF_PMGR_AreaDefaultCostSet(vr_id, proc_id, area_id, format, default_cost);
                return NETCFG_TYPE_FAIL;
            }
            else
            {
                return NETCFG_TYPE_OK;
            }
        }
    }
    #endif
    if(OSPF_PMGR_AreaDefaultCostUnset(vr_id, proc_id, area_id, format) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}


/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaRangeSet
* PURPOSE:
*     Set ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaRangeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    if(OSPF_PMGR_AreaRangeSet(vr_id, proc_id, area_id, format,range_addr,range_masklen) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaRangeNoAdvertiseSet
* PURPOSE:
*     Set ospf area range no advertise.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaRangeNoAdvertiseSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    if(OSPF_PMGR_AreaRangeNoAdvertiseSet(vr_id, proc_id, area_id, format,range_addr,range_masklen) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaRangeUnset
* PURPOSE:
*     Unset ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaRangeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    if(OSPF_PMGR_AreaRangeUnset(vr_id, proc_id, area_id, format,range_addr,range_masklen) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaNssaSet
* PURPOSE:
*     Set ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      nssa_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaNssaSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Nssa_Para_T *nssa_para)
{
    OSPF_TYPE_Area_Nssa_Para_T nssa;

    memset(&nssa, 0, sizeof(nssa));
    memcpy(&nssa, nssa_para, sizeof(nssa));
    if(OSPF_PMGR_AreaNssaSet(vr_id, proc_id, area_id, format, &nssa) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaNssaUnset
* PURPOSE:
*     Unset ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      flag.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaNssaUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T flag)
{
    if(OSPF_PMGR_AreaNssaUnset(vr_id, proc_id, area_id, format,flag) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetInstanceStatistics
* PURPOSE:
*     Get ospf instance some parameters.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id.
*
* OUTPUT:
*      entry
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetInstancePara(NETCFG_TYPE_OSPF_Instance_Para_T *entry)
{
    return NETCFG_OM_OSPF_GetInstancePara(entry);
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaVirtualLinkSet
* PURPOSE:
*     Set ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaVirtualLinkSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para)
{
    OSPF_TYPE_Area_Virtual_Link_Para_T vlink_param;

    memset(&vlink_param, 0, sizeof(vlink_param));
    memcpy(&vlink_param, vlink_para, sizeof(vlink_param));

    if(OSPF_PMGR_AreaVirtualLinkSet(vr_id, proc_id, area_id, format, &vlink_param) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaVirtualLinkUnset
* PURPOSE:
*     Unset ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaVirtualLinkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para)
{
    OSPF_TYPE_Area_Virtual_Link_Para_T vlink_param;

    memset(&vlink_param, 0, sizeof(vlink_param));
    memcpy(&vlink_param, vlink_para, sizeof(vlink_param));

    if(OSPF_PMGR_AreaVirtualLinkUnset(vr_id, proc_id, area_id, format, &vlink_param) != OSPF_TYPE_RESULT_SUCCESS)
    {
        return NETCFG_TYPE_FAIL;
    }
    else
    {
        return NETCFG_TYPE_OK;
    }       
}


