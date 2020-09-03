/* MODULE NAME:  k_amtrl3_mgr.c
 * PURPOSE:
 *   linux kernel module to provide APIs for AMTRL3_MGR
 * 
 * NOTES:
 *
 * HISTORY
 *    11/17/2009 - Peter Yu, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_module.h"
#include "sys_adpt.h"
#include "l_inet.h"
#include "k_sysfun.h"
#include "k_l_mm.h"
#include "k_l_ipcmem.h"
#include "l_cvrt.h"

#include "k_amtrl3_mgr.h"

#include <linux/netdevice.h> /* dev_get_by_index */

#include <linux/in6.h> /* in6_addr */
#include <net/ndisc.h> /* neigh_hit_by_amtrl3, neigh_nd_lookup */
#include <net/arp.h>   /* arp_lookup */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T K_AMTRL3_MGR_Syscall(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4);

/* STATIC VARIABLE DECLARATIONS
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
extern struct net init_net;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : K_AMTRL3_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize K_AMTRL3_MGR
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
void K_AMTRL3_MGR_Init(void)
{
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - K_AMTRL3_MGR_Create_InterCSC_Relation
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
void K_AMTRL3_MGR_Create_InterCSC_Relation(void)
{
    /* TBD: add SYSFUN_SYSCALL_IML_MGR to SYSFUN_SYSCALL_CMD_ID_E in sysfun.h
     */
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_AMTRL3_MGR, K_AMTRL3_MGR_Syscall);
}

/* LOCAL SUBPROGRAM BODIES
 */

/* system call implementation
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : K_AMTRL3_MGR_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *  	This function implements the system call for AMTRL3
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
static UI32_T K_AMTRL3_MGR_Syscall(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    UI32_T ret = K_AMTRL3_MGR_RESULT_OK;
    int cmd = L_CVRT_PTR_TO_UINT(arg0);

    switch(cmd)
    {
        case K_AMTRL3_MGR_SYSCALL_CMD_HIT :
        {
            /* arg 1    char *addr, 
               arg 2    addrlen,
               arg 3    cha mac_addr[6], 
               arg 4    vlan_ifindex
            */
            
            struct in6_addr addr;
            int addr_len;
            char ha[SYS_ADPT_MAC_ADDR_LEN];
            struct net_device *dev;
            addr_len = L_CVRT_PTR_TO_UINT(arg2);
            
            /* hit only for ipv6 neigh currently */
            if(addr_len == SYS_ADPT_IPV6_ADDR_LEN)
            {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
                dev = dev_get_by_index(&init_net, L_CVRT_PTR_TO_UINT(arg4));
#else
                dev = dev_get_by_index(L_CVRT_PTR_TO_UINT(arg4));
#endif
                if (dev)
                {
                    SYSFUN_CopyFromUser((void *) addr.s6_addr, (void *)arg1, SYS_ADPT_IPV6_ADDR_LEN);
                    SYSFUN_CopyFromUser((void *)ha, (void *)arg3, SYS_ADPT_MAC_ADDR_LEN);
                    if (0 != neigh_hit_by_amtrl3(dev, &addr, ha))
                        ret = K_AMTRL3_MGR_RESULT_FAIL;
                    dev_put(dev);
                }
                else
                {
                    ret = K_AMTRL3_MGR_RESULT_INVALID_ARG;
                }
            }
            else
            {
                ret = K_AMTRL3_MGR_RESULT_INVALID_ARG;
            }
            break;
         }
        
        case K_AMTRL3_MGR_SYSCALL_CMD_GET_NEIGHBOR:
        {
            /* arg 1    IN  ifindex
             * arg 2    IN  L_INET_AddrIp_T
             * arg 3    OUT ha (hardware address)
             * arg 4    OUT state
             */
            struct net_device       *dev;
            L_INET_AddrIp_T         ip_addr;
            UI8_T                   ha[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T                   state;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
            dev = dev_get_by_index(&init_net, L_CVRT_PTR_TO_UINT(arg1));
#else
            dev = dev_get_by_index(L_CVRT_PTR_TO_UINT(arg1));
#endif
            if (dev)
            {
                SYSFUN_CopyFromUser((void *) &ip_addr, (void *)arg2, sizeof(ip_addr)); 
                if(ip_addr.type == L_INET_ADDR_TYPE_IPV6 ||
                   ip_addr.type == L_INET_ADDR_TYPE_IPV6Z)
                {
                    struct in6_addr addr;

                    memcpy(addr.s6_addr, ip_addr.addr, SYS_ADPT_IPV6_ADDR_LEN);                    
                    if ((0 == neigh_nd_lookup(dev, &addr, ha, &state)) && (state & NUD_VALID))
                    {
                        SYSFUN_CopyToUser((void *)arg3, (void *)ha, SYS_ADPT_MAC_ADDR_LEN);
                        SYSFUN_CopyToUser((void *)arg4, (void *)&state, sizeof(UI8_T));
                    }
                    else
                    {
                        ret = K_AMTRL3_MGR_RESULT_FAIL;
                    }
                }
                else if (ip_addr.type == L_INET_ADDR_TYPE_IPV4 ||
                         ip_addr.type == L_INET_ADDR_TYPE_IPV4Z)
                {
                    __be32 addr;

                    memcpy(&addr, ip_addr.addr, sizeof(addr));
                    if ((0 == arp_lookup(dev, addr, ha, &state)) && (state & NUD_VALID))
                    {
                        SYSFUN_CopyToUser((void *)arg3, (void *)ha, SYS_ADPT_MAC_ADDR_LEN);
                        SYSFUN_CopyToUser((void *)arg4, (void *)&state, sizeof(UI8_T));
                    }
                    else
                    {
                        ret = K_AMTRL3_MGR_RESULT_FAIL;
                    }                    
                }
                else
                {
                    ret = K_AMTRL3_MGR_RESULT_INVALID_ARG;
                }

                dev_put(dev);
            }
            else
            {
                ret = K_AMTRL3_MGR_RESULT_INVALID_ARG;
            }

            break;
        }
        
        default:
            ret = K_AMTRL3_MGR_RESULT_UNKNOWN_SYSCALL_CMD;
    }

    return ret;
}

