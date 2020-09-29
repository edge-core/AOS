/* Module Name: netcfg_netdevice.h
 * Purpose:
 *      netcfg_netdevice.h provide macros to convert between VID and device name.
 *      If there are any other macro are needed to convert for Linux netdevice,
 *      They can still be added here.
 *
 * Notes:
 *      None
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2007.07.20  --  Max Chen,   Created
 *
 *
 * Copyright(C)      Accton Corporation, 2007.
 */

#ifndef NETCFG_NETDEVICE_H
#define NETCFG_NETDEVICE_H

/* INCLUDE FILE DECLARATIONS
 */
#ifdef __KERNEL__
#include <linux/kernel.h>
#else
#include "stdio.h"
#include "ip_lib.h"
#endif
#include "sys_type.h"
#include "sys_cpnt.h"
#include "vlan_lib.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETCFG_NETDEVICE_IFNAME_SIZE   20 /* must consistent with INTERFACE_NAMSIZ in ZebOS */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
static inline BOOL_T NETCFG_NETDEVICE_VidToIfname(UI32_T vid, char *ifname)
{
    if (IS_VLAN_ID_VAILD(vid))
    {
        sprintf(ifname, "VLAN%lu",(unsigned long)vid);
        return TRUE;
    }

    sprintf(ifname, "INVALID");
    return FALSE;
}

static inline BOOL_T NETCFG_NETDEVICE_IfnameToVid(char *ifname, UI32_T *vid)
{
    unsigned long tmp;
    *vid = 0;
    if (sscanf(ifname, "VLAN%lu", &tmp)==0)
        return FALSE;
    *vid = (UI32_T) tmp;
    if (!IS_VLAN_ID_VAILD(*vid))
        return FALSE;
    return TRUE;
}

/* ifindex could be vlan or tunnel ifindex */
static inline BOOL_T NETCFG_NETDEVICE_IfindexToZoneId(UI32_T ifindex, UI32_T *zoneid_p)
{
    if (ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
    {
        *zoneid_p = 0;
        return  FALSE;
    }
    *zoneid_p = ifindex - SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 1;
    return  TRUE;
}

static inline BOOL_T NETCFG_NETDEVICE_ZoneIdToIfindex(UI32_T zoneid, UI32_T *ifindex_p)
{
    if (zoneid < 1)
    {
        *ifindex_p = 0;
        return  FALSE;
    }
    *ifindex_p = zoneid + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1;
    return  TRUE;
}

#ifndef __KERNEL__
static inline BOOL_T NETCFG_NETDEVICE_IfindexToIfname(UI32_T ifindex, char *ifname)
{
    UI32_T id;

    if (IS_VLAN_IFINDEX_VAILD(ifindex))
    {
        VLAN_IFINDEX_CONVERTTO_VID(ifindex,id);
        sprintf(ifname, "VLAN%lu",(unsigned long)id);
        return TRUE;
    }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    else if (IS_TUNNEL_IFINDEX(ifindex))
    {
        IP_LIB_ConvertTunnelIdFromIfindex(ifindex, &id);
        sprintf(ifname, "TUNNEL%lu",(unsigned long)id); /* must be identical with PAL_TUNNEL_PREFIX */
        return TRUE;
    }
#endif
    else if (IP_LIB_ConvertLoopbackIfindexToId(ifindex, &id))
    {
        sprintf(ifname, "lo%lu",(unsigned long)id);
        return TRUE;
    }
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    else if (SYS_ADPT_CRAFT_INTERFACE_IFINDEX == ifindex)
    {
        sprintf(ifname, "CRAFT");
        return TRUE;
    }
#endif
    else if (SYS_ADPT_LOOPBACK_IF_INDEX_BASE == ifindex)
    {
        sprintf(ifname, "lo");
        return TRUE;
    }

    sprintf(ifname, "INVALID");
    return FALSE;
}

static inline BOOL_T NETCFG_NETDEVICE_IfnameToIfindex(char *ifname, UI32_T *ifindex)
{
    unsigned long tmp_int;
    unsigned long tmp_ifindex;

    *ifindex = 0;
    if (sscanf(ifname, "VLAN%lu", &tmp_int)==1)
    {
        if (!IS_VLAN_ID_VAILD(tmp_int))
            return FALSE;
        VLAN_VID_CONVERTTO_IFINDEX(tmp_int, tmp_ifindex);
        *ifindex = (UI32_T) tmp_ifindex;
        return TRUE;
    }

#if (SYS_CPNT_IP_TUNNEL == TRUE)
    if (sscanf(ifname, "TUNNEL%lu", &tmp_int)==1)
    {
        if (!IP_LIB_ConvertTunnelIdToIfindex((UI32_T)tmp_int, ifindex))
            return FALSE;
        return TRUE;
    }
#endif

    if (sscanf(ifname, "lo%lu", &tmp_int)==1)
    {
        if (!IP_LIB_ConvertLoopbackIdToIfindex((UI32_T)tmp_int, ifindex))
            return FALSE;
        return TRUE;
    }

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    {
        if (strlen(ifname) == 5 &&
             memcmp(ifname, "CRAFT", 5) == 0)
        {
            *ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
            return TRUE;
        }
    }
#endif

    return FALSE;
}

#if (SYS_CPNT_IP_TUNNEL == TRUE)
static inline BOOL_T NETCFG_NETDEVICE_TunnelIfindexToIfname(UI32_T ifindex, char *ifname)
{
    UI32_T id;

    if (IS_TUNNEL_IFINDEX(ifindex))
    {
        IP_LIB_ConvertTunnelIdFromIfindex(ifindex, &id);
        sprintf(ifname, "TUNNEL%lu",(unsigned long)id); /* must be identical with PAL_TUNNEL_PREFIX */
        return TRUE;
    }

    sprintf(ifname, "INVALID");
    return FALSE;
}
#endif

static inline BOOL_T NETCFG_NETDEVICE_LoopbackIfindexToIfname(UI32_T ifindex, char *ifname)
{
    UI32_T id;

    if (IP_LIB_ConvertLoopbackIfindexToId(ifindex, &id))
    {
        sprintf(ifname, "lo%lu",(unsigned long)id);
        return TRUE;
    }

    sprintf(ifname, "INVALID");
    return FALSE;
}

#endif /* #ifndef __KERNEL__ */

#endif

