/*-----------------------------------------------------------------------------
 * MODULE NAME: L2_L4_PROC_COMM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module provides APIs for CSCs within the L2_L4 process to get common
 *    resources of the L2_L4 process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/17     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_type.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"
#include "sys_cpnt.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

/* the handle of thread group
 */

static L_THREADGRP_Handle_T gvrp_group_tg_handle;

static L_THREADGRP_Handle_T l2mcast_group_tg_handle;

static L_THREADGRP_Handle_T l2mux_group_tg_handle;

static L_THREADGRP_Handle_T l4_group_tg_handle;

static L_THREADGRP_Handle_T lacp_group_tg_handle;

static L_THREADGRP_Handle_T netaccess_group_tg_handle;

static L_THREADGRP_Handle_T sta_group_tg_handle;

static L_THREADGRP_Handle_T swctrl_group_tg_handle;

static L_THREADGRP_Handle_T amtrl3_group_tg_handle;

static L_THREADGRP_Handle_T udphelper_group_tg_handle;
static L_THREADGRP_Handle_T oam_group_tg_handle;

static L_THREADGRP_Handle_T dhcpsnp_group_tg_handle;
#if (SYS_CPNT_CFM == TRUE)
static L_THREADGRP_Handle_T cfm_group_tg_handle;
#endif

#if (SYS_CPNT_DAI == TRUE)
static L_THREADGRP_Handle_T dai_group_tg_handle;
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
static L_THREADGRP_Handle_T pppoe_ia_group_tg_handle;
#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
static L_THREADGRP_Handle_T ovsvtep_group_tg_handle;
#endif

#if (SYS_CPNT_DCB_GROUP == TRUE)
static L_THREADGRP_Handle_T dcb_group_tg_handle;
#endif

#if (SYS_CPNT_MLAG == TRUE)
static L_THREADGRP_Handle_T mlag_group_tg_handle;
#endif

#if (SYS_CPNT_VXLAN == TRUE)
static L_THREADGRP_Handle_T vxlan_group_tg_handle;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the system resource which is common for all CSCs in
 *           L2_L4 process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T L2_L4_PROC_COMM_InitiateProcessResources(void)
{
    gvrp_group_tg_handle = L_THREADGRP_Create();
    if (gvrp_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    l2mcast_group_tg_handle = L_THREADGRP_Create();
    if (l2mcast_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    l2mux_group_tg_handle = L_THREADGRP_Create();
    if (l2mux_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    l4_group_tg_handle = L_THREADGRP_Create();
    if (l4_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    lacp_group_tg_handle = L_THREADGRP_Create();
    if (lacp_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    netaccess_group_tg_handle = L_THREADGRP_Create();
    if (netaccess_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    sta_group_tg_handle = L_THREADGRP_Create();
    if (sta_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    swctrl_group_tg_handle = L_THREADGRP_Create();
    if (swctrl_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_AMTRL3 == TRUE)
    amtrl3_group_tg_handle = L_THREADGRP_Create();
    if (amtrl3_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
        udphelper_group_tg_handle = L_THREADGRP_Create();
        if (udphelper_group_tg_handle == NULL)
        {
            printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
            return FALSE;
        }
#endif

#if (SYS_CPNT_DAI == TRUE)
            dai_group_tg_handle = L_THREADGRP_Create();
            if (dai_group_tg_handle == NULL)
            {
                printf("\n%s(): L_THREADGRP_Create dai_group_tg_handle fail.\n", __FUNCTION__);
                return FALSE;
            }
#endif

    oam_group_tg_handle = L_THREADGRP_Create();
    if (oam_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }

    dhcpsnp_group_tg_handle = L_THREADGRP_Create();
    if (dhcpsnp_group_tg_handle == NULL)
    {
        printf("\n%s(): L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }
#if (SYS_CPNT_CFM == TRUE)
    cfm_group_tg_handle = L_THREADGRP_Create();
    if (cfm_group_tg_handle == NULL)
    {
        printf("\n%s(): CFM L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
    pppoe_ia_group_tg_handle = L_THREADGRP_Create();
    if (pppoe_ia_group_tg_handle == NULL)
    {
        printf("\n%s(): PPPOE IA L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }
#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
        ovsvtep_group_tg_handle = L_THREADGRP_Create();
        if (ovsvtep_group_tg_handle == NULL)
        {
            printf("\n%s(): OVSVTEP L_THREADGRP_Create fail.\n", __FUNCTION__);
            return FALSE;
        }
#endif

#if (SYS_CPNT_DCB_GROUP == TRUE)
    dcb_group_tg_handle = L_THREADGRP_Create();
    if (dcb_group_tg_handle == NULL)
    {
        printf("\n%s(): DCB L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }
#endif

#if (SYS_CPNT_MLAG == TRUE)
    mlag_group_tg_handle = L_THREADGRP_Create();
    if (mlag_group_tg_handle == NULL)
    {
        printf("\n%s(): MLAG L_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    vxlan_group_tg_handle = L_THREADGRP_Create();
    if (vxlan_group_tg_handle == NULL)
    {
        printf("\n%s(): VXLAN_THREADGRP_Create fail.\n", __FUNCTION__);
        return FALSE;
    }
#endif

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetGvrpGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for GVRP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetGvrpGroupTGHandle(void)
{
    return gvrp_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetL2McastGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for L2Mcast group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetL2McastGroupTGHandle(void)
{
    return l2mcast_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetL2muxGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for L2MUX group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetL2muxGroupTGHandle(void)
{
    return l2mux_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetL4GroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for L4 group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetL4GroupTGHandle(void)
{
    return l4_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetLacpGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for LACP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetLacpGroupTGHandle(void)
{
    return lacp_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetNetaccessGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for NETACCESS group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetNetaccessGroupTGHandle(void)
{
    return netaccess_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetStaGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for STA group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetStaGroupTGHandle(void)
{
    return sta_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetSwctrlGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for SWCTRL group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetSwctrlGroupTGHandle(void)
{
    return swctrl_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetAmtrl3GroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for AMTRL3 group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetAmtrl3GroupTGHandle(void)
{
    return amtrl3_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetUdphelperGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for UDPHELPER group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetUdphelperGroupTGHandle(void)
{
    return udphelper_group_tg_handle;
}
#if (SYS_CPNT_DAI == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetDaiGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for UDPHELPER group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetDaiGroupTGHandle(void)
{
    return dai_group_tg_handle;
}
#endif
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetOamGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for LACP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetOamGroupTGHandle(void)
{
    return oam_group_tg_handle;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetDhcpsnpGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for DHCPSNP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetDhcpsnpGroupTGHandle(void)
{
    return dhcpsnp_group_tg_handle;
}

#if (SYS_CPNT_CFM == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetCfmGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for cfm group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetCfmGroupTGHandle(void)
{
    return cfm_group_tg_handle;
}
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetPppoeiaGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for PPPOE IA group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetPppoeiaGroupTGHandle(void)
{
    return pppoe_ia_group_tg_handle;
}
#endif

#if (SYS_CPNT_OVSVTEP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetOvsvtepGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for OVSVTEP group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetOvsvtepGroupTGHandle(void)
{
    return ovsvtep_group_tg_handle;
}
#endif /* #if (SYS_CPNT_OVSVTEP == TRUE) */

#if (SYS_CPNT_DCB_GROUP == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetDcbGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for DCB group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetDcbGroupTGHandle(void)
{
    return dcb_group_tg_handle;
}
#endif

#if (SYS_CPNT_MLAG == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetMlagGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for MLAG group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetMlagGroupTGHandle(void)
{
    return mlag_group_tg_handle;
}
#endif

#if (SYS_CPNT_VXLAN == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME : L2_L4_PROC_COMM_GetVxlanGroupTGHandle
 *-----------------------------------------------------------------------------
 * PURPOSE : Get the thread group handler for VXLAN group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T L2_L4_PROC_COMM_GetVxlanGroupTGHandle(void)
{
    return vxlan_group_tg_handle;
}
#endif

/* LOCAL SUBPROGRAM BODIES
 */
