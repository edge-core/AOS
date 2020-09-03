#ifndef __BEN_VLAN_NET_INC__
#define __BEN_VLAN_NET_INC__
#include <linux/version.h>
#include <linux/if_vlan.h>

/*  Uncomment this if you want debug traces to be shown. */
//#define VLAN_DEBUG

#define VLAN_NET_MAX_VID     4096

#define VLAN_ERR KERN_ERR
#define VLAN_INF KERN_INFO
#define VLAN_DBG KERN_ALERT /* change these... to debug, having a hard time
                             * changing the log level at run-time..for some reason.
                             */

/*

These I use for memory debugging.  I feared a leak at one time, but
I never found it..and the problem seems to have dissappeared.  Still,
I'll bet they might prove useful again... --Ben


#define VLAN_MEM_DBG(x, y, z) printk(VLAN_DBG "%s:  "  x, __FUNCTION__, y, z);
#define VLAN_FMEM_DBG(x, y) printk(VLAN_DBG "%s:  " x, __FUNCTION__, y);
*/

/* This way they don't do anything! */
#define VLAN_MEM_DBG(x, y, z)
#define VLAN_FMEM_DBG(x, y)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
extern int VLAN_NET_id;
extern struct net_device* VLAN_NET_DeviceTable[VLAN_NET_MAX_VID];
#endif

/*  Find a VLAN device by the MAC address of its Ethernet device, and
 *  it's VLAN ID.  The default configuration is to have VLAN's scope
 *  to be box-wide, so the MAC will be ignored.  The mac will only be
 *  looked at if we are configured to have a separate set of VLANs per
 *  each MAC addressable interface.  Note that this latter option does
 *  NOT follow the spec for VLANs, but may be useful for doing very
 *  large quantities of VLAN MUX/DEMUX onto FrameRelay or ATM PVCs.
 *
 *  Must be invoked with rcu_read_lock (ie preempt disabled)
 *  or with RTNL.
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_NET_Create_InterCSC_Relation
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
void VLAN_NET_Create_InterCSC_Relation(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_CreateVlanDev
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Create/register net_device by vid
 * INPUT    : vid       -- vlan id
 *            macaddr   -- mac address
 * OUTPUT   : None
 * RETURN   : Success: net_device pointer
 *          : failure: NULL
 * NOTE     : None
 * ----------------------------------------------------------------------------------
 */
struct net_device* VLAN_NET_CreateVlanDev(UI32_T vid, UI8_T *macaddr);

/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_DestroyVlanDev
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Destroy/unregister net_device by vid
 * INPUT    : vid   -- vlan id
 * OUTPUT   : None
 * RETURN   : Success: TRUE
 *          : failure: FALSE
 * NOTE     : None
 * ----------------------------------------------------------------------------------
 */
BOOL_T VLAN_NET_DestroyVlanDev(UI32_T vid);

/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_GetVlanDevFromVid
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Get net_device by vid
 * INPUT    : vid   -- vlan id
 * OUTPUT   : None
 * RETURN   : Success: the net_device pointer
 *          : failure: NULL
 * NOTE     : None
 * ----------------------------------------------------------------------------------
 */
struct net_device* VLAN_NET_GetVlanDevFromVid(UI32_T vid);

/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_GetVidFromVlanDev
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Get vid by net_device
 * INPUT    : netdev   -- net_device pointer
 * OUTPUT   : None
 * RETURN   : Success: the vid of this net_device
 *          : failure: 0
 * NOTE     : None
 * ----------------------------------------------------------------------------------
 */
UI32_T VLAN_NET_GetVidFromVlanDev(struct net_device *netdev);

/*
 * Function VLAN_NET_proto_init (pro)
 *
 *    Initialize VLAN protocol layer,
 *
 */
I32_T VLAN_NET_proto_init(void);

/*
 *     Module 'remove' entry point.
 *     o delete /proc/net/router directory and static entries.
 */
void VLAN_NET_cleanup_module(void);

#endif /* !(__BEN_VLAN_NET_INC__) */
