#include <linux/version.h>
#include <linux/module.h>
#include <linux/stddef.h>	/* offsetof(), etc. */
#include <linux/errno.h>	/* return codes */
#include <linux/kernel.h>
#include <linux/slab.h>		/* kmalloc(), kfree() */
#include <linux/mm.h>
#include <linux/string.h>	/* inline mem*, str* functions */
#include <linux/init.h>		/* __initfunc et al. */
#include <asm/byteorder.h>	/* htons(), etc. */
#include <asm/uaccess.h>	/* copy_to_user */
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include "vlan_net_proc.h"
#include "vlan_net.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#include <../net/8021q/vlan.h>
#include <net/netns/generic.h>
#endif


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
#define VLAN_DEV_INFO vlan_dev_priv
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#define VLAN_DEV_INFO vlan_dev_info
#endif

/****** Function Prototypes *************************************************/

/* Methods for preparing data for reading proc entries */
static int VLAN_NET_seq_show(struct seq_file *seq, void *v);
static void *VLAN_NET_seq_start(struct seq_file *seq, loff_t *pos);
static void *VLAN_NET_seq_next(struct seq_file *seq, void *v, loff_t *pos);
static void VLAN_NET_seq_stop(struct seq_file *seq, void *);
static int vlandev_seq_show(struct seq_file *seq, void *v);

/*
 *	Global Data
 */


/*
 *	Names of the proc directory entries 
 */

static const char name_root[]	 = "vlan";
static const char name_conf[]	 = "config";

/*
 *	Structures for interfacing with the /proc filesystem.
 *	VLAN creates its own directory /proc/net/vlan with the folowing
 *	entries:
 *	config		device status/configuration
 *	<device>	entry for each  device
 */

/*
 *	Generic /proc/net/vlan/<file> file and inode operations 
 */

static struct seq_operations VLAN_NET_seq_ops = {
	.start = VLAN_NET_seq_start,
	.next = VLAN_NET_seq_next,
	.stop = VLAN_NET_seq_stop,
	.show = VLAN_NET_seq_show,
};

static int VLAN_NET_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &VLAN_NET_seq_ops);
}

static struct file_operations VLAN_NET_fops = {
	.owner	 = THIS_MODULE,
	.open    = VLAN_NET_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};

/*
 *	/proc/net/vlan/<device> file and inode operations
 */

static int vlandev_seq_open(struct inode *inode, struct file *file)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
    return single_open(file, vlandev_seq_show, PDE_DATA(inode));
#else
	return single_open(file, vlandev_seq_show, PDE(inode)->data);
#endif
}

static struct file_operations vlandev_fops = {
	.owner = THIS_MODULE,
	.open    = vlandev_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
};

/*
 * Proc filesystem derectory entries.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
/*
 *	/proc/net/vlan 
 */

static struct proc_dir_entry *proc_vlan_net_dir;

/*
 *	/proc/net/vlan/config 
 */

static struct proc_dir_entry *proc_vlan_net_conf;
#endif

#if 0
/* Strings */
static const char *VLAN_NET_name_type_str[VLAN_NAME_TYPE_HIGHEST] = {
    [VLAN_NAME_TYPE_RAW_PLUS_VID]       = "VLAN_NAME_TYPE_RAW_PLUS_VID",
    [VLAN_NAME_TYPE_PLUS_VID_NO_PAD]	= "VLAN_NAME_TYPE_PLUS_VID_NO_PAD",
    [VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD]= "VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD",
    [VLAN_NAME_TYPE_PLUS_VID]		= "VLAN_NAME_TYPE_PLUS_VID",
};
#endif
/*
 *	Interface functions
 */

/*
 *	Clean up /proc/net/vlan entries
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
void VLAN_NET_proc_cleanup(struct net *net)
{
#if defined(VLAN_NET_DEBUG_PROC)
	struct vlan_net *vn = net_generic(net, VLAN_NET_id);

	if (vn->proc_vlan_conf)
		remove_proc_entry(name_conf, vn->proc_vlan_dir);

	if (vn->proc_vlan_dir)
		proc_net_remove(net, name_root);
#endif
	/* Dynamically added entries should be cleaned up as their vlan_device
	 * is removed, so we should not have to take care of it here...
	 */
}
#else
void VLAN_NET_proc_cleanup(void)
{
	if (proc_vlan_net_conf)
		remove_proc_entry(name_conf, proc_vlan_net_dir);

	if (proc_vlan_net_dir)
		proc_net_remove(name_root);

	/* Dynamically added entries should be cleaned up as their vlan_net_device
	 * is removed, so we should not have to take care of it here...
	 */
}
#endif

/*
 *	Create /proc/net/vlan entries
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
int VLAN_NET_proc_init(struct net *net)
{
#if defined(VLAN_NET_DEBUG_PROC)
	struct vlan_net *vn = net_generic(net, VLAN_NET_id);

	vn->proc_vlan_dir = proc_net_mkdir(net, name_root, net->proc_net);
	if (!vn->proc_vlan_dir)
		goto err;

	vn->proc_vlan_conf = proc_create(name_conf, S_IFREG|S_IRUSR|S_IWUSR,
				     vn->proc_vlan_dir, &VLAN_NET_fops);
	if (!vn->proc_vlan_conf)
		goto err;
	return 0;

err:
	printk("%s: can't create entry in proc filesystem!\n", __FUNCTION__);
	VLAN_NET_proc_cleanup(net);
	return -ENOBUFS;
#else
    return 0;
#endif
}
#else
int VLAN_NET_proc_init(void)
{
	proc_vlan_net_dir = proc_mkdir(name_root, proc_net);
	if (proc_vlan_net_dir) {
		proc_vlan_net_conf = create_proc_entry(name_conf,
						   S_IFREG|S_IRUSR|S_IWUSR,
						   proc_vlan_net_dir);
		if (proc_vlan_net_conf) {
			proc_vlan_net_conf->proc_fops = &VLAN_NET_fops;
			return 0;
		}
	}
	VLAN_NET_proc_cleanup();
	return -ENOBUFS;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
static inline int VLAN_NET_is_vlan_net_dev(struct net_device *dev)
{
	return dev->priv_flags & IFF_802_1Q_VLAN;
}
#endif

/*
 *	Add directory entry for VLAN device.
 */

int VLAN_NET_proc_add_dev (struct net_device *vlandev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
	struct vlan_dev_priv *dev_info = VLAN_DEV_INFO(vlandev);
#else
	struct vlan_dev_info *dev_info = VLAN_DEV_INFO(vlandev);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#if defined(VLAN_NET_DEBUG_PROC)
	struct vlan_net *vn = net_generic(dev_net(vlandev), VLAN_NET_id);


	if (!VLAN_NET_is_vlan_net_dev(vlandev)) {
		printk(KERN_ERR
		       "ERROR:	VLAN_NET_proc_add, device -:%s:- is NOT a VLAN\n",
		       vlandev->name);
		return -EINVAL;
	}
#else
    return 0;
#endif
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
    dev_info->dent = proc_create(vlandev->name, S_IFREG|S_IRUSR|S_IWUSR, NULL, &vlandev_fops);

    if (!dev_info->dent)
        return -ENOBUFS;

#else

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#if defined(VLAN_NET_DEBUG_PROC)
	dev_info->dent = create_proc_entry(vlandev->name,
					   S_IFREG|S_IRUSR|S_IWUSR,
					   vn->proc_vlan_dir);
#endif
#else
	dev_info->dent = create_proc_entry(vlandev->name,
					   S_IFREG|S_IRUSR|S_IWUSR,
					   proc_vlan_net_dir);
#endif


	if (!dev_info->dent)
		return -ENOBUFS;

	dev_info->dent->proc_fops = &vlandev_fops;
	dev_info->dent->data = vlandev;

#ifdef VLAN_DEBUG
	printk(KERN_ERR "VLAN_NET_proc_add, device -:%s:- being added.\n",
	       vlandev->name);
#endif
#endif

	return 0;
}

/*
 *	Delete directory entry for VLAN device.
 */
int VLAN_NET_proc_rem_dev(struct net_device *vlandev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#if defined(VLAN_NET_DEBUG_PROC)
	struct vlan_net *vn = net_generic(dev_net(vlandev), VLAN_NET_id);
#else
    return 0;
#endif
#endif

	if (!vlandev) {
		printk(VLAN_ERR "%s: invalid argument: %p\n",
			__FUNCTION__, vlandev);
		return -EINVAL;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#if defined(VLAN_NET_DEBUG_PROC)
	if (!VLAN_NET_is_vlan_net_dev(vlandev)) {
		printk(VLAN_DBG "%s: invalid argument, device: %s is not a VLAN device, priv_flags: 0x%4hX.\n",
			__FUNCTION__, vlandev->name, vlandev->priv_flags);
		return -EINVAL;
	}
#endif
#endif


#ifdef VLAN_DEBUG
	printk(VLAN_DBG "%s: dev: %p\n", __FUNCTION__, vlandev);
#endif

	/** NOTE:  This will consume the memory pointed to by dent, it seems. */
	if (VLAN_DEV_INFO(vlandev)->dent) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#if defined(VLAN_NET_DEBUG_PROC)
		remove_proc_entry(VLAN_DEV_INFO(vlandev)->dent->name, vn->proc_vlan_dir);
#endif
#else
		remove_proc_entry(VLAN_DEV_INFO(vlandev)->dent->name, proc_vlan_net_dir);
#endif
		VLAN_DEV_INFO(vlandev)->dent = NULL;
	}

	return 0;
}

/****** Proc filesystem entry points ****************************************/

/*
 * The following few functions build the content of /proc/net/vlan/config
 */

/* starting at dev, find a VLAN device */
#if (KERNEL_VERSION(2,6,19) == LINUX_VERSION_CODE)
static struct net_device *VLAN_NET_skip(struct net_device *dev) 
{
/*
	while (dev && !(dev->priv_flags & IFF_802_1Q_VLAN)) 
		dev = dev->next;
*/

	return dev;
}
#endif

/* start read of /proc/net/vlan/config */ 
static void *VLAN_NET_seq_start(struct seq_file *seq, loff_t *pos)
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32))
    __acquires(dev_base_lock)
#endif
{
	struct net_device *dev;
	loff_t i = 1;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    struct net *net = seq_file_net(seq);
#endif

	read_lock(&dev_base_lock);

	if (*pos == 0)
		return SEQ_START_TOKEN;
	
#if (KERNEL_VERSION(2,6,19) == LINUX_VERSION_CODE) /* jerry.du 2009-05-08, added for linux 2.6.22.18 has no dev_base  */
	for (dev = VLAN_NET_skip(dev_base); dev && i < *pos; 
	     dev = VLAN_NET_skip(dev->next), ++i);
#elif  (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    for_each_netdev(net, dev)
    {
        if (!VLAN_NET_is_vlan_net_dev(dev))
            continue;

        if (i++ == *pos)
            return dev;
    }
    return NULL;
#else
	printk("please check\n");
#endif
		 
	return  (i == *pos) ? dev : NULL;
} 

static void *VLAN_NET_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
#if (KERNEL_VERSION(2,6,19) == LINUX_VERSION_CODE) /* jerry.du 2009-05-08, added for linux 2.6.22.18 has no dev_base  */
	++*pos;

	return VLAN_NET_skip((v == SEQ_START_TOKEN)  
			    ? dev_base 
			    : ((struct net_device *)v)->next);
#elif  (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
	struct net_device *dev;
	struct net *net = seq_file_net(seq);

	++*pos;

	dev = (struct net_device *)v;
	if (v == SEQ_START_TOKEN)
		dev = net_device_entry(&net->dev_base_head);

	for_each_netdev_continue(net, dev) {
		if (!VLAN_NET_is_vlan_net_dev(dev))
			continue;

		return dev;
	}

	return NULL;

#else
	printk("please check\n");
#endif
}

static void VLAN_NET_seq_stop(struct seq_file *seq, void *v)
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,32))
    __releases(dev_base_lock)
#endif
{
	read_unlock(&dev_base_lock);
}

static int VLAN_NET_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN) {
/*		const char *nmtype = NULL;

		seq_puts(seq, "VLAN Dev name	 | VLAN ID\n");

		if (VLAN_NET_name_type < ARRAY_SIZE(VLAN_NET_name_type_str))
		    nmtype =  VLAN_NET_name_type_str[VLAN_NET_name_type];

		seq_printf(seq, "Name-Type: %s\n", 
			   nmtype ? nmtype :  "UNKNOWN" );
*/
        seq_puts(seq, "VLAN Dev name  | VLAN ID\n");

	} else {
		const struct net_device *vlandev = v;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
		const struct vlan_dev_priv *dev_info = VLAN_DEV_INFO(vlandev);
#else
		const struct vlan_dev_info *dev_info = VLAN_DEV_INFO(vlandev);
#endif

		seq_printf(seq, "%-15s| %d  \n",  vlandev->name,  
			   dev_info->vlan_id);
	}
	return 0;
}

static int vlandev_seq_show(struct seq_file *seq, void *offset)
{
	struct net_device *vlandev = (struct net_device *) seq->private;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
	const struct vlan_dev_priv *dev_info = VLAN_DEV_INFO(vlandev);
    struct rtnl_link_stats64 stat;
    struct rtnl_link_stats64 *stats;
#else
	const struct vlan_dev_info *dev_info = VLAN_DEV_INFO(vlandev);
	const struct net_device_stats *stats;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
	static const char fmt[] = "%30s %12llu\n";
#else
	static const char fmt[] = "%30s %12lu\n";
#endif
	int i;

	if ((vlandev == NULL) || (!(vlandev->priv_flags & IFF_802_1Q_VLAN)))
		return 0;

	seq_printf(seq, "%s  VID: %d	 REORDER_HDR: %i  dev->priv_flags: %hx\n",
		       vlandev->name, dev_info->vlan_id,
		       (int)(dev_info->flags & 1), vlandev->priv_flags);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65))
    {
        stats = dev_get_stats(vlandev, &stat);
    }
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    stats = dev_get_stats(vlandev);
#else
	stats = vlan_dev_get_stats(vlandev);
#endif

	seq_printf(seq, fmt, "total frames received", stats->rx_packets);
	seq_printf(seq, fmt, "total bytes received", stats->rx_bytes);
	seq_printf(seq, fmt, "Broadcast/Multicast Rcvd", stats->multicast);
	seq_puts(seq, "\n");
	seq_printf(seq, fmt, "total frames transmitted", stats->tx_packets);
	seq_printf(seq, fmt, "total bytes transmitted", stats->tx_bytes);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,2,65))
	seq_printf(seq, fmt, "total headroom inc", 
		   dev_info->cnt_inc_headroom_on_tx);
	seq_printf(seq, fmt, "total encap on xmit", 
		   dev_info->cnt_encap_on_xmit);
#endif
	seq_printf(seq, "Device: %s", dev_info->real_dev->name);
	/* now show all PRIORITY mappings relating to this VLAN */
	seq_printf(seq, 
		       "\nINGRESS priority mappings: 0:%lu  1:%lu  2:%lu  3:%lu  4:%lu  5:%lu  6:%lu 7:%lu\n",
		       (unsigned long)(dev_info->ingress_priority_map[0]),
		       (unsigned long)(dev_info->ingress_priority_map[1]),
		       (unsigned long)(dev_info->ingress_priority_map[2]),
		       (unsigned long)(dev_info->ingress_priority_map[3]),
		       (unsigned long)(dev_info->ingress_priority_map[4]),
		       (unsigned long)(dev_info->ingress_priority_map[5]),
		       (unsigned long)(dev_info->ingress_priority_map[6]),
		       (unsigned long)(dev_info->ingress_priority_map[7]));

	seq_printf(seq, "EGRESSS priority Mappings: ");
	for (i = 0; i < 16; i++) {
		const struct vlan_priority_tci_mapping *mp
			= dev_info->egress_priority_map[i];
		while (mp) {
			seq_printf(seq, "%lu:%hu ",
				   (unsigned long)(mp->priority), ((mp->vlan_qos >> 13) & 0x7));
			mp = mp->next;
		}
	}
	seq_puts(seq, "\n");

	return 0;
}
