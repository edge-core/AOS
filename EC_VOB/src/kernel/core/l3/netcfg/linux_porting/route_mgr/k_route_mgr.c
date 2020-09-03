/* MODULE NAME:  k_route_mgr.h
 * PURPOSE:
 *   linux kernel module to provide APIs for ROUTE_MGR
 *
 * NOTES:
 *
 * HISTORY
 *    3/2/2010 - KH Shi Created
 *
 * Copyright(C)      Accton Corporation, 2010
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_module.h"
#include "sys_adpt.h"
#include "k_sysfun.h"
#include "k_l_mm.h"
#include "k_l_ipcmem.h"
#include "l_inet.h"

#include "netcfg_type.h"
#include "netcfg_netdevice.h"
#include "k_route_mgr.h"

#include <linux/netdevice.h> /* dev_get_by_index */
#include <linux/in6.h> /* in6_addr */
#include <net/ip6_route.h>    /* rt6_mtu_change */
#include <net/addrconf.h>     /* ipv6_dev_get_saddr */
#include <net/flow.h>         /* struct flowi */
#include <net/ip_fib.h>       /* struct fib_result */
#include <net/ip6_fib.h>      /* struct rt6_info */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T ROUTE_MGR_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4);

/* STATIC VARIABLE DECLARATIONS
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
extern struct net init_net;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : ROUTE_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize ROUTE_MGR
 *
 * 	INPUT:   
 *   None.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   None.
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
void ROUTE_MR_Init(void)
{
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ROUTE_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE:
 *   This function initializes all function pointer registration operations.
 *
 * INPUT:
 *   None.
 *    
 * OUTPUT:
 *   None.
 *
 * RETURN
 *   None.
 *
 * NOTES:
 *   None.
 *--------------------------------------------------------------------------*/
void ROUTE_MGR_Create_InterCSC_Relation(void)
{
    /* TBD: add SYSFUN_SYSCALL_IML_MGR to SYSFUN_SYSCALL_CMD_ID_E in sysfun.h
     */
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_ROUTE_MGR, ROUTE_MGR_Syscall);
}

/* LOCAL SUBPROGRAM BODIES
 */

/* system call implementation
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : ROUTE_MGR_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function implements the system call for ROUTE
 *
 * 	INPUT:
 *   cmd        --  The command to be executed
 *   arg1-arg4  --  The meaning of arg1 to arg4 depends on the cmd.
 *
 * 	OUTPUT:
 *   arg1-arg4  --  The meaning of arg1 to arg4 depends on the cmd.
 *
 * 	RETURN:
 *   The meaning of the return value depends on the cmd.
 *
 * 	NOTES:
 *   When cmd is invalid, return value is always IML_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD
 *--------------------------------------------------------------------------
 */
static UI32_T ROUTE_MGR_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4)
{       
    UI32_T ret = NETCFG_TYPE_FAIL;

    switch(cmd)
    {
        case NETCFG_TYPE_SYSCALL_CMD_ROUTE_MGR_RT6_MTU_CHANGE:
        {
            /* arg 1    vlan_ifindex 
             * arg 2    mtu
             */
            unsigned mtu;
            struct net_device *dev;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
            dev = dev_get_by_index(&init_net, (int) arg1);
#else
            dev = dev_get_by_index((int) arg1);
#endif
            mtu = arg2;
            rt6_mtu_change(dev, mtu);
            dev_put(dev);
            ret = NETCFG_TYPE_OK;
            break;
         }   

#if 0
        case NETCFG_TYPE_SYSCALL_CMD_ROUTE_MGR_GET_BEST_ROUTING_INTERFACE:
        {
            L_INET_AddrIp_T dest_addr, src_addr, nexthop_addr;
            UI32_T          out_ifindex;
            UI8_T           zero_addr[SYS_ADPT_IPV6_ADDR_LEN] = {0};

            SYSFUN_CopyFromUser((void *) &dest_addr, (void *)arg1, sizeof(L_INET_AddrIp_T));

            if (dest_addr.type == L_INET_ADDR_TYPE_IPV4 ||
                dest_addr.type == L_INET_ADDR_TYPE_IPV4Z)
            {
                struct fib_result res;
                __be32 src, gw;

                struct flowi fl = {
                    .nl_u = {
                        .ip4_u = {
                            .daddr = 0,
                            .scope = RT_SCOPE_UNIVERSE,
                        },
                    },
                    .oif = 0,
                };

                if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(dest_addr.addr))
                    fl.fl4_scope = RT_SCOPE_LINK;

                memcpy(&(fl.fl4_dst), dest_addr.addr, sizeof(fl.fl4_dst));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
                if (fib_lookup(&init_net, &fl, &res) == 0)
#else
                if (fib_lookup(&fl, &res) == 0)
#endif
                {
                    out_ifindex = (UI32_T)FIB_RES_OIF(res);

                    gw = FIB_RES_GW(res);
                    if (0 == memcmp(&gw, zero_addr, SYS_ADPT_IPV4_ADDR_LEN))
                    {
                        nexthop_addr = dest_addr;
                    }
                    else
                    {
                        memset(&nexthop_addr, 0, sizeof(L_INET_AddrIp_T));
                        nexthop_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                        memcpy(nexthop_addr.addr, &gw, SYS_ADPT_IPV4_ADDR_LEN);
                        if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(nexthop_addr.addr))
                        {
                            nexthop_addr.type = L_INET_ADDR_TYPE_IPV4Z;
                            NETCFG_NETDEVICE_IfindexToZoneId(out_ifindex, &nexthop_addr.zoneid);
                        }
                        else
                        {
                            nexthop_addr.type = L_INET_ADDR_TYPE_IPV4;
                        }
                    }

                    src = FIB_RES_PREFSRC(res);
                    memset(&src_addr, 0, sizeof(L_INET_AddrIp_T));
                    src_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                    memcpy(src_addr.addr, &src, SYS_ADPT_IPV4_ADDR_LEN);
                    if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(src_addr.addr))
                    {
                        src_addr.type = L_INET_ADDR_TYPE_IPV4Z;
                        NETCFG_NETDEVICE_IfindexToZoneId(out_ifindex, &src_addr.zoneid);
                    }
                    else
                    {
                        src_addr.type = L_INET_ADDR_TYPE_IPV4;
                    }

                    SYSFUN_CopyToUser((void *)arg2, (void *)&src_addr, sizeof(L_INET_AddrIp_T));
                    SYSFUN_CopyToUser((void *)arg3, (void *)&nexthop_addr, sizeof(L_INET_AddrIp_T));
                    SYSFUN_CopyToUser((void *)arg4, (void *)&out_ifindex, sizeof(UI32_T));

                    fib_res_put(&res);

                    ret = NETCFG_TYPE_OK;
                }
            }
            else if (dest_addr.type == L_INET_ADDR_TYPE_IPV6 ||
                     dest_addr.type == L_INET_ADDR_TYPE_IPV6Z)
            {
                struct in6_addr dest, src, nh;
                struct rt6_info *rt;
                UI32_T ifindex;

                memcpy(dest.s6_addr, dest_addr.addr, sizeof(dest.s6_addr));
                if (dest_addr.type == L_INET_ADDR_TYPE_IPV6Z)
                {
                    NETCFG_NETDEVICE_ZoneIdToIfindex(dest_addr.zoneid, &ifindex);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
                    rt = rt6_lookup(&init_net, &dest, NULL, ifindex, 1);
#else
                    rt = rt6_lookup(&dest, NULL, ifindex, 1);
#endif
                }
                else
                {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
                    rt = rt6_lookup(&init_net, &dest, NULL, 0, 0);
#else
                    rt = rt6_lookup(&dest, NULL, 0, 0);
#endif
                }

                if (rt)
                {
                    if (rt->rt6i_dev)
                        out_ifindex = rt->rt6i_dev->ifindex;
                    else
                        out_ifindex = 0;

                    if (0 == memcmp(rt->rt6i_gateway.s6_addr, zero_addr, SYS_ADPT_IPV6_ADDR_LEN))
                    {
                        nexthop_addr = dest_addr;
                        memcpy(nh.s6_addr, dest_addr.addr, sizeof(nh.s6_addr));
                    }
                    else
                    {
                        memset(&nexthop_addr, 0, sizeof(L_INET_AddrIp_T));
                        nexthop_addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                        memcpy(nexthop_addr.addr, rt->rt6i_gateway.s6_addr, SYS_ADPT_IPV6_ADDR_LEN);
                        if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(nexthop_addr.addr))
                        {
                            nexthop_addr.type = L_INET_ADDR_TYPE_IPV6Z;
                            NETCFG_NETDEVICE_IfindexToZoneId(out_ifindex, &nexthop_addr.zoneid);
                        }
                        else
                        {
                            nexthop_addr.type = L_INET_ADDR_TYPE_IPV6;
                        }
                        nh = rt->rt6i_gateway;
                    }

                    memset(&src_addr, 0, sizeof(L_INET_AddrIp_T));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
                    if (0 ==ipv6_dev_get_saddr(&init_net, rt->rt6i_dev, &nh, 0, &src))
#else
                    if (0 == ipv6_get_saddr(&rt->u.dst, &nh, &src))
#endif
                    {
                        src_addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                        memcpy(src_addr.addr, src.s6_addr, SYS_ADPT_IPV6_ADDR_LEN);
                        if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(src_addr.addr))
                        {
                            src_addr.type = L_INET_ADDR_TYPE_IPV6Z;
                            NETCFG_NETDEVICE_IfindexToZoneId(out_ifindex, &src_addr.zoneid);
                        }
                        else
                        {
                            src_addr.type = L_INET_ADDR_TYPE_IPV6;
                        }
                    }

                    SYSFUN_CopyToUser((void *)arg2, (void *)&src_addr, sizeof(L_INET_AddrIp_T));
                    SYSFUN_CopyToUser((void *)arg3, (void *)&nexthop_addr, sizeof(L_INET_AddrIp_T));
                    SYSFUN_CopyToUser((void *)arg4, (void *)&out_ifindex, sizeof(UI32_T));

                    dst_release(&rt->u.dst);
                    ret = NETCFG_TYPE_OK;
                }
            }
            break;
        }
#endif

        default:
            ret = NETCFG_TYPE_UNKNOWN_SYSCALL_CMD;
            break;
    }

    return ret;
}

