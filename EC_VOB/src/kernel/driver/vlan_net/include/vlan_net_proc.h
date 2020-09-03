#ifndef __BEN_VLAN_NET_PROC_INC__
#define __BEN_VLAN_NET_PROC_INC__

#include <linux/version.h>

#ifdef CONFIG_PROC_FS
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
int VLAN_NET_proc_init(struct net *net);
void VLAN_NET_proc_cleanup(struct net *net);
#else
int VLAN_NET_proc_init(void);
void VLAN_NET_proc_cleanup (void);
#endif

/* On linux kernel v2.6.32, functions listed below:
 *   register_pernet_gen_device()
 *   unregister_pernet_gen_device()
 *   proc_net_mkdir()
 *   proc_net_remove()
 * which are exported by linux kernel only
 * allows the kernel module that adopt GPL license to call. Accton lkm now
 * adopt Proprietary license so it is not allowed to call these functions.
 * If you need to use this debug feature, you need to define VLAN_NET_DEBUG_PROC
 * and replace MODULE_LICENSE("Proprietary"); with MODULE_LICENSE("GPL");
#define VLAN_NET_DEBUG_PROC 1
 */

int VLAN_NET_proc_rem_dev(struct net_device *vlandev);
int VLAN_NET_proc_add_dev (struct net_device *vlandev);


#else /* No CONFIG_PROC_FS */

#define VLAN_NET_proc_init()	(0)
#define VLAN_NET_proc_cleanup()	do {} while(0)
#define VLAN_NET_proc_add_dev(dev)	({(void)(dev), 0;})
#define VLAN_NET_proc_rem_dev(dev)	({(void)(dev), 0;})

#endif

#endif /* !(__BEN_VLAN_NET_PROC_INC__) */
