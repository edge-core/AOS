//#include <linuxkernel.h>

#include <linux/capability.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/datalink.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/init.h>
#include <net/p8022.h>
#include <net/arp.h>
#include <linux/rtnetlink.h>
#include <linux/notifier.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
#include <linux/uaccess.h> /* for copy_from_user */
#else
#include <asm/uaccess.h> /* for copy_from_user */
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#include <../net/8021q/vlan.h>
#include <net/netns/generic.h>
#else
#include <linux/if_vlan.h>
#endif

#include <linux/wait.h>

#include "sys_cpnt.h"
#include "netcfg_netdevice.h"
#include "sys_type.h"
#include "vlan_net.h"
#include "vlan_net_type.h"
#include "vlan_net_proc.h"
#include "iml_type.h"
#include "k_iml_mgr.h"
#include "k_l_mm.h"
#include "k_l_ipcmem.h"
#include "k_sysfun.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#define VLAN_NAME "vlan"
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
#define VLAN_DEV_INFO vlan_dev_priv
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#define VLAN_DEV_INFO vlan_dev_info
#endif

#if (SYS_CPNT_CRAFT_PORT == TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
#define MAX_NBR_OF_CRAFT_PORT_TX_PKT 30
typedef struct VLAN_NET_CraftPortTxPktListCB_S
{
    UI16_T               tx_pkt_list_head;
    UI16_T               tx_pkt_list_tail;
    UI16_T               pkt_count;
    wait_queue_head_t    wq;
    L_MM_Mref_Handle_T   *tx_pkt_mrefs[MAX_NBR_OF_CRAFT_PORT_TX_PKT];
} VLAN_NET_CraftPortTxPktListCB_T;
#endif


#ifdef VLAN_NET_DEBUG
#define DBGMSG printk
#else
#define DBGMSG(...)
#endif

/* Global VLAN variables */

#ifdef Wally0
/* Our listing of VLAN group(s) */
static struct hlist_head VLAN_NET_group_hash[VLAN_GRP_HASH_SIZE];
#define VLAN_NET_grp_hashfn(IDX)    ((((IDX) >> VLAN_GRP_HASH_SHIFT) ^ (IDX)) & VLAN_GRP_HASH_MASK)

static int VLAN_NET_device_event(struct notifier_block *, unsigned long, void *);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
static int VLAN_NET_ioctl_handler(struct net *net, void __user *arg);
#else
static int VLAN_NET_ioctl_handler(void __user *arg);
#endif

static BOOL_T VLAN_NET_Unregister_device(UI32_T vlan_id);
static struct net_device *VLAN_NET_Register_device(
                     UI8_T *macaddr, UI32_T vlan_id);
#if (SYS_CPNT_CRAFT_PORT==TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
static void VLAN_NET_CreateCraftPortDev(void);
static UI32_T VLAN_NET_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4);
#endif


struct net_device* VLAN_NET_DeviceTable[VLAN_NET_MAX_VID];

#if (SYS_CPNT_CRAFT_PORT==TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
static struct net_device* craft_port_netdev=NULL;
static VLAN_NET_CraftPortTxPktListCB_T craft_port_tx_pkt_list_cb;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
int VLAN_NET_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
static const struct net_device_ops vlan_net_netdev_ops = {
	.ndo_start_xmit =  VLAN_NET_hard_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
};

#if defined(VLAN_NET_DEBUG_PROC)
int VLAN_NET_id;

static int vlan_net_init_net(struct net *net)
{
	int err;
	struct vlan_net *vn;

	err = -ENOMEM;
	vn = kzalloc(sizeof(struct vlan_net), GFP_KERNEL);
	if (vn == NULL)
		goto err_alloc;

	err = net_assign_generic(net, VLAN_NET_id, vn);
	if (err < 0)
		goto err_assign;

	err = VLAN_NET_proc_init(net);
	if (err < 0)
		goto err_proc;

	return 0;

err_proc:
	/* nothing */
err_assign:
	kfree(vn);
err_alloc:
	return err;
}

static void vlan_net_exit_net(struct net *net)
{
	struct vlan_net *vn;

	vn = net_generic(net, VLAN_NET_id);
	VLAN_NET_proc_cleanup(net);
	kfree(vn);
}
static struct pernet_operations VLAN_NET_ops = {
	.init = vlan_net_init_net,
	.exit = vlan_net_exit_net,
};
#endif /* end of #if defined(VLAN_NET_DEBUG_PROC) */

#endif /* end of #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)) */

/* End of global variables definitions. */

/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_hard_start_xmit
 * ----------------------------------------------------------------------------------
 * PURPOSE  : The packet-transimission function provided by vlan netdevice
 * INPUT    : skb  -  The packet to be transmitted
 *            dev  -
 * OUTPUT   : None
 * RETURN   :
 *            NETDEV_TX_OK: driver took care of packet
 *            NETDEV_TX_BUSY: driver tx path was busy
 *            NETDEV_TX_LOCKED: driver tx lock was already taken
 * NOTE     :
 *    1. The return values are defined in netdevice.h of linux kernel source code.
 * ----------------------------------------------------------------------------------
 */
int VLAN_NET_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    UI32_T ret,rc;

    rc=IML_MGR_EnqToIPTxPktList(skb);
    ret=NETDEV_TX_OK;

    switch(rc)
    {
        case IML_TYPE_RETVAL_OK:
            DBGMSG("<0>IML_MGR_EnqToIPTxPktList OK\r\n");
            break;
        case IML_TYPE_RETVAL_L_MM_ALLOCATE_TX_BUFFER_FAIL:
        case IML_TYPE_RETVAL_TX_BUF_FULL:
            DBGMSG("<0>IML_MGR_EnqToIPTxPktList TX fail\r\n");
            ret=NETDEV_TX_BUSY;
            break;
        case IML_TYPE_RETVAL_GET_VID_FROM_VLAN_DEV_ERROR:
            DBGMSG("<0>IML_MGR_EnqToIPTxPktList fail(Vid not exists)\r\n");
            /* there is no way to handle packet to vid that doesn't exist
             * treat it as tx ok
             */
            break;
        default:
            /* unknown error, treat it as tx ok
             */
            printk("<0>IML_MGR_EnqToIPTxPktList fail(rc=%d)\r\n", (int)rc);
    }

    if(ret==NETDEV_TX_OK)
        kfree_skb(skb);

    return ret;
}


/*
 * Function VLAN_NET_proto_init (pro)
 *
 *    Initialize VLAN protocol layer,
 *
 */
I32_T VLAN_NET_proto_init(void)
{
    I32_T err;
#ifdef VLAN_DEBUG
    printk(VLAN_INF "%s %s\n",
           __FUNCTION__, VLAN_NAME);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#if defined(VLAN_NET_DEBUG_PROC)
	err = register_pernet_gen_device(&VLAN_NET_id, &VLAN_NET_ops);
#else
    err = 0;
#endif
#else
    /* proc file system initialization */
    err = VLAN_NET_proc_init();
#endif
    if (err < 0) {
        printk(KERN_ERR
               "%s %s: can't create entry in proc filesystem!\n",
               __FUNCTION__, VLAN_NAME);
        return err;
    }


#ifdef Wally0
    dev_add_pack(&VLAN_NET_packet_type);

    /* Register us to receive netdevice events */
    err = register_netdevice_notifier(&VLAN_NET_notifier_block);
    if (err < 0) {
        printk(KERN_ERR
               "%s %s: can't register to netdevice notifier!\n",
               __FUNCTION__, VLAN_NAME);
        dev_remove_pack(&VLAN_NET_packet_type);
        VLAN_NET_proc_cleanup();
        return err;
    }
#endif

    vlan_ioctl_set(VLAN_NET_ioctl_handler);

    memset((void*)(VLAN_NET_DeviceTable), 0, sizeof(VLAN_NET_DeviceTable));

#ifdef VLAN_DEBUG
    printk(VLAN_INF "%s return\n",
           __FUNCTION__);
#endif

#if (SYS_CPNT_CRAFT_PORT==TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
    init_waitqueue_head(&(craft_port_tx_pkt_list_cb.wq));
    craft_port_tx_pkt_list_cb.pkt_count =
        craft_port_tx_pkt_list_cb.tx_pkt_list_head =
        craft_port_tx_pkt_list_cb.tx_pkt_list_tail = 0;
    VLAN_NET_CreateCraftPortDev();
#endif

    return 0;
}


/*
 *     Module 'remove' entry point.
 *     o delete /proc/net/router directory and static entries.
 */
void VLAN_NET_cleanup_module(void)
{
    UI32_T  vid;

#ifdef VLAN_DEBUG
    printk(VLAN_DBG "%s: \n", __FUNCTION__);
#endif
    vlan_ioctl_set(NULL);

#ifdef Wally0
    /* Un-register us from receiving netdevice events */
    unregister_netdevice_notifier(&VLAN_NET_notifier_block);

    dev_remove_pack(&VLAN_NET_packet_type);

#endif

    /* cleanup all devices() */
    for(vid = 1 ; vid <= VLAN_NET_MAX_VID; vid++)
    {
        VLAN_NET_Unregister_device(vid);
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#if defined(VLAN_NET_DEBUG_PROC)
	unregister_pernet_gen_device(VLAN_NET_id, &VLAN_NET_ops);
#endif
#else
    VLAN_NET_proc_cleanup();
#endif

//  synchronize_net();
}


static BOOL_T VLAN_NET_Unregister_device(UI32_T vlan_id)
{
    struct net_device *dev;


    /* sanity check */
    if( (vlan_id == 0) || (vlan_id > VLAN_NET_MAX_VID) )
    {
        printk(VLAN_DBG "%s: invalid VID: %ld\n", __FUNCTION__, (unsigned long)vlan_id);
        return FALSE;
    }

    dev = VLAN_NET_DeviceTable[vlan_id-1];
    if (dev)
    {
#ifdef VLAN_DEBUG
            printk(VLAN_DBG "%s: unregister %s(%ld)\n", __FUNCTION__, dev->name, (unsigned long)vlan_id);
#endif
        rtnl_lock();

//        ASSERT_RTNL();
//        synchronize_net();
//        dev_put(dev);
        unregister_netdevice(dev);

        /* Remove proc entry */
        VLAN_NET_proc_rem_dev(dev);

        VLAN_NET_DeviceTable[vlan_id-1] = NULL;

        rtnl_unlock();

        free_netdev(dev);

        return TRUE;
    }
    else
    {
#ifdef VLAN_DEBUG
//      printk(VLAN_DBG "%s: WARNING: Could not find VID %ld.\n", __FUNCTION__,vlan_id);
#endif
        return FALSE;
    }
}

static void VLAN_NET_setup(struct net_device *new_dev)
{
#ifdef Wally0
    SET_MODULE_OWNER(new_dev);

    /* new_dev->ifindex = 0;  it will be set when added to
     * the global list.
     * iflink is set as well.
     */
    new_dev->get_stats = vlan_dev_get_stats;

    /* Make this thing known as a VLAN device */
    new_dev->priv_flags |= IFF_802_1Q_VLAN;
    new_dev->open = VLAN_NET_dev_open;
    new_dev->stop = VLAN_NET_dev_stop;
    new_dev->do_ioctl = VLAN_NET_dev_ioctl;
    new_dev->set_multicast_list = VLAN_NET_dev_set_multicast_list;
    new_dev->destructor = free_netdev;

    /* Set us up to have no queue, as the underlying Hardware device
     * can do all the queueing we could want.
     */
    new_dev->tx_queue_len = 0;

    new_dev->change_mtu = VLAN_NET_dev_change_mtu;
    new_dev->set_mac_address = VLAN_NET_dev_set_mac_address;
#endif
    ether_setup(new_dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    new_dev->priv_flags        |= IFF_802_1Q_VLAN; /* we will not enable vlan in linux kernel, it's ok to use IFF_802_1Q_VLAN */
	new_dev->netdev_ops     = &vlan_net_netdev_ops;
#endif

}


/*
 * vlan network devices have devices nesting below it, and are a special
 * "super class" of normal network devices; split their locks off into a
 * separate class since they always nest.
 */
//static struct lock_class_key VLAN_NET_netdev_xmit_lock_key;


/*  Attach a VLAN device to a mac address (ie Ethernet Card).
 *  Returns the device that was created, or NULL if there was
 *  an error of some kind.
 */
static struct net_device *VLAN_NET_Register_device(UI8_T *macaddr,
                           UI32_T vlan_id)
{
    struct net_device *new_dev;
    char              name[NETCFG_NETDEVICE_IFNAME_SIZE+1];

    NETCFG_NETDEVICE_VidToIfname(vlan_id, name);
#ifdef VLAN_DEBUG
    printk(VLAN_DBG "%s: if_name -:%s:- vid: %ld\n",
        __FUNCTION__, name, vlan_id);
#endif

    if( (vlan_id == 0) || (vlan_id > VLAN_NET_MAX_VID) )
        goto out_ret_null;

    if ( VLAN_NET_DeviceTable[vlan_id-1] != NULL )
    {
        /* was already registered. */
        printk(VLAN_DBG "%s: ALREADY had VLAN registered\n", __FUNCTION__);
        goto out_ret_null;
    }

    /* From this point on, all the data structures must remain
     * consistent.
     */
    rtnl_lock();

#ifdef VLAN_DEBUG
        printk(VLAN_DBG "Prepare to call alloc_netdev() for name -:%s:-\n", name);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
    new_dev = alloc_netdev(sizeof(struct vlan_dev_priv), name,
                   NET_NAME_UNKNOWN, VLAN_NET_setup);

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
    new_dev = alloc_netdev(sizeof(struct vlan_dev_priv), name,
                   VLAN_NET_setup);

#else
    new_dev = alloc_netdev(sizeof(struct vlan_dev_info), name,
                   VLAN_NET_setup);
#endif

    if (new_dev == NULL) goto out_unlock;

#ifdef VLAN_DEBUG
    printk(VLAN_DBG "Allocated new name -:%s:-\n", new_dev->name);
#endif

// Wally    new_dev->flags = real_dev->flags;
    new_dev->flags &= ~IFF_UP;
    netif_carrier_on(new_dev);
#ifdef Wally0
    new_dev->state = (real_dev->state & ((1<<__LINK_STATE_NOCARRIER) |
                         (1<<__LINK_STATE_DORMANT))) |
             (1<<__LINK_STATE_PRESENT);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
    VLAN_MEM_DBG("new_dev->priv malloc, addr: %p  size: %i\n",
             new_dev->priv,
             sizeof(struct vlan_dev_priv));
#else
    VLAN_MEM_DBG("new_dev->priv malloc, addr: %p  size: %i\n",
             new_dev->priv,
             sizeof(struct vlan_dev_info));
#endif
    memcpy(new_dev->dev_addr, (void*)macaddr, new_dev->addr_len);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
    new_dev->hard_start_xmit = VLAN_NET_hard_start_xmit;
#endif

    VLAN_DEV_INFO(new_dev)->vlan_id = vlan_id; /* 1 through VLAN_VID_MASK */
    VLAN_DEV_INFO(new_dev)->real_dev = NULL; // real_dev;
    VLAN_DEV_INFO(new_dev)->dent = NULL;
    VLAN_DEV_INFO(new_dev)->flags = 1;

#ifdef VLAN_DEBUG
    printk(VLAN_DBG "About to register net device \n");
#endif

    if (register_netdevice(new_dev))
        goto out_free_newdev;

#ifdef Wally0
    lockdep_set_class(&new_dev->_xmit_lock, &VLAN_NET_netdev_xmit_lock_key);

    new_dev->iflink = real_dev->ifindex;
    VLAN_NET_transfer_operstate(real_dev, new_dev);

    linkwatch_fire_event(new_dev); /* _MUST_ call rfc2863_policy() */
#endif

    VLAN_NET_DeviceTable[vlan_id-1] = new_dev;

    if (VLAN_NET_proc_add_dev(new_dev)<0)/* create it's proc entry */
                printk(KERN_WARNING "VLAN: failed to add proc entry for %s\n",
                                     new_dev->name);

    rtnl_unlock();


#ifdef VLAN_DEBUG
    printk(VLAN_DBG "Allocated new device successfully, returning.\n");
#endif
    return new_dev;

out_free_newdev:
    free_netdev(new_dev);

out_unlock:
    rtnl_unlock();

out_ret_null:
    return NULL;
}

#ifdef Wally0
static int VLAN_NET_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
#ifdef VLAN_DEBUG
    printk(VLAN_DBG "%s: event(%ld), grp(%lx),ifindex(%d)\n", __FUNCTION__, event, (long)grp, dev->ifindex);
#endif
}
#endif

/*
 *  VLAN IOCTL handler.
 *  o execute requested action or pass command to the device driver
 *   arg is really a struct vlan_ioctl_args __user *.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
static int VLAN_NET_ioctl_handler(struct net *net, void __user *arg)
#else
static int VLAN_NET_ioctl_handler(void __user *arg)
#endif
{
    I32_T err = 0;
    struct vlan_ioctl_args args;

    if (copy_from_user(&args, arg, sizeof(struct vlan_ioctl_args)))
        return -EFAULT;

    /* Null terminate this sucker, just in case. */
    args.device1[23] = 0;
    args.u.device2[23] = 0;

#ifdef VLAN_DEBUG
        printk(VLAN_DBG "%s: args.cmd: %x\n", __FUNCTION__, args.cmd);
#endif

    return err;
}

#if (SYS_CPNT_CRAFT_PORT==TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_craftport_hard_start_xmit
 * ----------------------------------------------------------------------------------
 * PURPOSE  : The packet-transimission function provided by craft port netdevice
 * INPUT    : skb  -  The packet to be transmitted
 *            dev  -
 * OUTPUT   : None
 *            NETDEV_TX_OK: driver took care of packet
 *            NETDEV_TX_BUSY: driver tx path was busy
 *            NETDEV_TX_LOCKED: driver tx lock was already taken
 * NOTE     :
 *    1. The return values are defined in netdevice.h of linux kernel source code.
 * ----------------------------------------------------------------------------------
 */
static int VLAN_NET_craftport_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    SYSFUN_IntMask_T oint;
    UI32_T pdu_len, prev_caller_tid=0;
    uintptr_t prev_caller_addr=0;
    L_MM_Mref_Handle_T *mref_handle_p;
    UI8_T              *pdu;

    DBGMSG("Enter VLAN_NET_craftport_hard_start_xmit\r\n");

    /* check whether the list is full */
    if(craft_port_tx_pkt_list_cb.pkt_count==MAX_NBR_OF_CRAFT_PORT_TX_PKT)
    {
        if (!netif_queue_stopped(skb->dev))
            netif_stop_queue(skb->dev);
        DBGMSG("%s:craft_port_tx_pkt_list_cb is full\r\n", __FUNCTION__);
        return NETDEV_TX_BUSY;
    }

    /* convert skb to mref */
    /* space for ethernet header is already reserved by K_L_MM_AllocateTxBuffer
     * padding to 60 bytes if the total packet size is less than 60 bytes
     * (would be 64 bytes including CRC)
     */
    mref_handle_p=K_L_MM_AllocateTxBuffer((skb->len < 60)? 46: skb->len - 14, L_MM_USER_ID2(0, 0));
    if(mref_handle_p==NULL)
    {
        DBGMSG("%s:K_L_MM_AllocateTxBuffer fail\r\n", __FUNCTION__);
        return NETDEV_TX_BUSY;
    }

    pdu = K_L_MM_Mref_GetPdu(mref_handle_p, __builtin_return_address(0), &pdu_len, &prev_caller_addr, &prev_caller_tid);
    if (prev_caller_addr!=0)
    {
        /* this error message can be logged to flash when next Get PDU operation
         * is performed from user space
         */
        printk("<0>%s(%d)prev_caller_addr=%p tid=%lu\n", __FUNCTION__, __LINE__, (void*)prev_caller_addr, (unsigned long)prev_caller_tid);
    }

    memcpy(pdu-14, skb->data, skb->len);

    /* set padding bytes to 0
     */
    if(pdu_len>skb->len)
        memset(pdu-14+skb->len, 0, pdu_len-skb->len);

    DBGMSG("%s:pdu_len=%d,skb->len=%d\r\n", __FUNCTION__, (int)pdu_len, skb->len);
    DBGMSG("%s:da=0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\r\n", __FUNCTION__, pdu[-14], pdu[-13], pdu[-12], pdu[-11], pdu[-10], pdu[-9]);
    DBGMSG("%s:sa=0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\r\n", __FUNCTION__, pdu[-8], pdu[-7], pdu[-6], pdu[-5], pdu[-4], pdu[-3]);
    DBGMSG("%s:packet_type=0x%02x-0x%02x\r\n", __FUNCTION__, pdu[-2], pdu[-1]);

    DBGMSG("%s:ip header first 4 bytes:0x%02x 0x%02x 0x%02x 0x%02x\r\n", __FUNCTION__, pdu[0], pdu[1], pdu[2], pdu[3]);

    oint = SYSFUN_InterruptLock();
    /* append to the tail of the ip pkt list */
    craft_port_tx_pkt_list_cb.tx_pkt_mrefs[craft_port_tx_pkt_list_cb.tx_pkt_list_tail]=mref_handle_p;
    craft_port_tx_pkt_list_cb.tx_pkt_list_tail++;
    craft_port_tx_pkt_list_cb.tx_pkt_list_tail %= MAX_NBR_OF_CRAFT_PORT_TX_PKT;
    craft_port_tx_pkt_list_cb.pkt_count++;
    SYSFUN_InterruptUnlock(oint);

    DBGMSG("%s:craft_port_tx_pkt_count=%d\r\n", __FUNCTION__, (int)craft_port_tx_pkt_list_cb.pkt_count);

    wake_up(&(craft_port_tx_pkt_list_cb.wq));

    kfree_skb(skb);
    return NETDEV_TX_OK;

}

/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_craftport_setup
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Setup net_device for craft port
 * INPUT    : new_dev  -  net device of the craft port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------
 */
static void VLAN_NET_craftport_setup(struct net_device *new_dev)
{
    ether_setup(new_dev);
    new_dev->flags &= ~IFF_UP;
    netif_carrier_on(new_dev);
    new_dev->hard_start_xmit = VLAN_NET_craftport_hard_start_xmit;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : VLAN_NET_CreateCraftPortDev
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Create/register net_device for craft port
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function is only called once upon initialization
 * ----------------------------------------------------------------------------------
 */
static void VLAN_NET_CreateCraftPortDev(void)
{
    rtnl_lock();
	craft_port_netdev=alloc_netdev(0, "CRAFT",
                                   VLAN_NET_craftport_setup);
    if(craft_port_netdev==NULL)
        printk("<0>Failed to create craft port net device");

    if(register_netdevice(craft_port_netdev))
    {
        printk("<0>Failed to register craft port net device");
        free_netdev(craft_port_netdev);
    }
	rtnl_unlock();
}

#endif /* end of #if (SYS_CPNT_CRAFT_PORT==TRUE) */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_NET_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *  	This function implements the system call for VLAN_NET.
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
 *   None.
 *--------------------------------------------------------------------------
 */
static UI32_T VLAN_NET_Syscall(UI32_T cmd, UI32_T arg1, UI32_T arg2, UI32_T arg3, UI32_T arg4)
{
    UI32_T ret=FALSE;

    switch(cmd)
    {
#if (SYS_CPNT_CRAFT_PORT==TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
        case VLAN_NET_SYSCALL_CMD_SEND_PACKET_TO_CRAFT_PORT_NET_DEVICE:
        {
            UI8_T          *dest_p;
            struct sk_buff *skb;
            int            rc;

            /* arg1: [IN] UI8_T* pkt_buffer
             * arg2: [IN] UI32_T pkt_length
             */
            if(craft_port_netdev==NULL)
            {
                printk("<0>%s: craft port net device not created\n", __FUNCTION__);
            }
            else
            {
                skb = alloc_skb(arg2+2, GFP_KERNEL);
                if(skb==NULL)
                {
                    printk("%s: Failed to allocate skb.\n", __FUNCTION__);
                }
                else
                {
                    /* for ip addr in ip header can align to 4-byte boundary */
                    skb_reserve(skb, 2);
                    dest_p=skb_put(skb, arg2);
                    SYSFUN_CopyFromUser(dest_p, (void*)arg1, arg2);
                    /* setup skb fields */
                    skb->dev = craft_port_netdev;
                    skb->protocol=eth_type_trans(skb, craft_port_netdev);
                    skb->ip_summed=CHECKSUM_NONE;
                    rc=netif_receive_skb(skb);
                    if(rc!=NET_RX_SUCCESS)
                        printk("%s: netif_receive_skb returns error=%d\n", __FUNCTION__, rc);
                    else
                        ret=TRUE;
                }
            }
        }
            break;

        case VLAN_NET_SYSCALL_CMD_RECV_PACKET_FROM_CRAFT_PORT_NET_DEVICE:
        {
            SYSFUN_IntMask_T oint;
            UI32_T mref_handle_offset;
            L_MM_Mref_Handle_T *mref_handle_p;

            /* arg1: [OUT] UI32_T *mref_handle_offset_p
             */


            /* blocked if ip pkt list is empty
             */
            if(craft_port_tx_pkt_list_cb.pkt_count==0)
            {
                int rc;

                rc=wait_event_interruptible(craft_port_tx_pkt_list_cb.wq, craft_port_tx_pkt_list_cb.pkt_count!=0);
                if(rc!=0)
                    return FALSE;
            }

            if(craft_port_tx_pkt_list_cb.pkt_count!=0)
            {
                /* dequeue from the head of craft port tx pkt list */
                mref_handle_p=craft_port_tx_pkt_list_cb.tx_pkt_mrefs[craft_port_tx_pkt_list_cb.tx_pkt_list_head];

                oint = SYSFUN_InterruptLock();
                craft_port_tx_pkt_list_cb.tx_pkt_list_head++;
                craft_port_tx_pkt_list_cb.tx_pkt_list_head %= MAX_NBR_OF_CRAFT_PORT_TX_PKT;
                craft_port_tx_pkt_list_cb.pkt_count--;
                SYSFUN_InterruptUnlock(oint);

                if (craft_port_tx_pkt_list_cb.pkt_count == MAX_NBR_OF_CRAFT_PORT_TX_PKT / 3)
                {
                    if(netif_queue_stopped(craft_port_netdev))
                        netif_wake_queue(craft_port_netdev);
                }

                mref_handle_offset=K_L_IPCMEM_GetOffset(mref_handle_p);
                SYSFUN_CopyToUser((void*)arg1, &mref_handle_offset, sizeof(UI32_T));
                ret=TRUE;
            }
            else
            {
                SYSFUN_CopyToUser((void*)arg1, &mref_handle_offset, sizeof(UI32_T));
            }
        }
            break;
#endif /* end of #if (SYS_CPNT_CRAFT_PORT==TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT) */
        default:
            ret=FALSE;
            break;
    }

    return ret;
}

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
void VLAN_NET_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_VLAN_NET, VLAN_NET_Syscall);
}

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
struct net_device* VLAN_NET_CreateVlanDev(UI32_T vid, UI8_T macaddr[SYS_ADPT_MAC_ADDR_LEN])
{
    struct net_device* netdev;

    netdev = VLAN_NET_Register_device(macaddr, vid);
    if( netdev == NULL )
    {
        // can not allocate or register net_device
    }
    return netdev;
}

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
BOOL_T VLAN_NET_DestroyVlanDev(UI32_T vid)
{
    BOOL_T    err;

    err = VLAN_NET_Unregister_device(vid);

    return err;
}

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
struct net_device* VLAN_NET_GetVlanDevFromVid(UI32_T vid)
{
    if (vid == 0 || vid > VLAN_NET_MAX_VID)
        return NULL;

    return VLAN_NET_DeviceTable[vid-1];
}

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
UI32_T VLAN_NET_GetVidFromVlanDev(struct net_device *netdev)
{
    UI32_T vid;

    if( !netdev ) return 0;

    NETCFG_NETDEVICE_IfnameToVid(netdev->name, &vid);
    if( VLAN_DEV_INFO(netdev)->vlan_id != vid )
    {
        // error, vid is not consistent with name
    }

    return (UI32_T)vid;
}
