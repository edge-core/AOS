/* Module Name: k_umh.c
 * Purpose:
 *      This module is for user mode helper.
 *
 * Notes:
 *      None.
 *
 * HISTORY
 *       Date       --  Modifier    Reason
 *       2014.11.26 --  Squid       Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/unistd.h>
#include <linux/kmod.h>
#include <net/ipv6.h>
#include <linux/inetdevice.h>
#include <linux/version.h>

#if ( LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0) )
    #include <linux/seq_file.h>
#endif

#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"

#if (SYS_CPNT_NETCFG_IP_REDIRECT == TRUE)

/* NAMING CONSTANT DECLARARTIONS
 */
#define IF_TAG_PID              "IF_PID"
#define IF_TAG_COMM             "IF_COMM"
#define IF_TAG_CMD              "IF_CMD"
#define IF_TAG_FAMI             "IF_FAMI"
#define IF_TAG_NAME             "IF_NAME"
#define IF_TAG_ADDR             "IF_ADDR"
#define IF_TAG_MASK             "IF_MASK"
#define DFT_UMH_IPCFG_PATH      "/initrd/etc/ip_config.sh"
#define UMH_IPCFG_PROC_ENTRY    "umh_ipcfg"
#define UMH_IPCFG_PATH_MAXLEN   150
#define UMH_RES_PROC_ENTRY_FMT  "umh_res_%d_%d"

#define K_UMH_NETLINK_MSG       1
#define K_UMH_IOCTL_MSG         0

#if 0
    #define K_UMH_IPCFG_DBG
#endif

/* TYPE DEFINITIONS
 */

/* MACRO DEFINITIONS
 */

#ifdef K_UMH_IPCFG_DBG
    #define K_UMH_DBG_MSG(fmt,...)  printk("<0>%s:%d "fmt"\n",      \
                                        __FUNCTION__, __LINE__,     \
                                        ##__VA_ARGS__)
#else
    #define K_UMH_DBG_MSG(fmt,...)
#endif

/* LOCAL FUNCTIONS DECLARATIONS
 */
static struct proc_dir_entry *K_UMH_IpcfgPathProcCreate(char *fname);
static int K_UMH_SetupEnvIp4(char *envp[], int family, unsigned int cmd, unsigned long arg);
static int K_UMH_SetupEnvIp6(char *envp[], int family, unsigned int cmd, unsigned long arg);
static int K_UMH_SetupEnvIp4_NL(char *envp[], int family, unsigned int cmd, unsigned long arg);
static int K_UMH_SetupEnvIp6_NL(char *envp[], int family, unsigned int cmd, unsigned long arg);
static int K_UMH_SetupEnv(char *envp[], int is_nl, int family, unsigned int cmd, unsigned long arg);
static int K_UMH_CustomSockIoctl(int is_nl, int family, unsigned int cmd, unsigned long arg);


static char umh_ipcfg_path[UMH_IPCFG_PATH_MAXLEN] = DFT_UMH_IPCFG_PATH;
static struct proc_dir_entry *umh_ipcfg_pentry = NULL;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* PURPOSE: Clean up procedure for user mode helper.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None.
 */
void K_UMH_Cleanup( void )
{
    extern int (*umh_sock_ioctl)(int, int, unsigned int, unsigned long);

    umh_sock_ioctl = NULL;

    if (NULL != umh_ipcfg_pentry)
        remove_proc_entry(UMH_IPCFG_PROC_ENTRY, NULL);
}

/* PURPOSE: Init procedure for user mode helper.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None.
 */
void K_UMH_Init( void )
{
    extern int (*umh_sock_ioctl)(int, int, unsigned int, unsigned long);

    umh_sock_ioctl = K_UMH_CustomSockIoctl;

    umh_ipcfg_pentry = K_UMH_IpcfgPathProcCreate(UMH_IPCFG_PROC_ENTRY);
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/* PURPOSE: Write event handler for umh_ipcfg uner /proc .
 * INPUT  : TBD.
 * OUTPUT : None
 * RETURN : count for bytes written/ -EFAULT if failed
 * NOTES  : None.
 */
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0) )

static int K_UMH_IpcfgPathProcShow(struct seq_file *m, void *v)
{
    seq_printf(m, "%s\n", umh_ipcfg_path);
    return 0;
}

static int K_UMH_IpcfgPathProcOpen(struct inode *inode, struct  file *file)
{
    return single_open(file, K_UMH_IpcfgPathProcShow, NULL);
}

static ssize_t K_UMH_IpcfgPathProcWrite(
    struct file *fp, const char *userBuf, size_t count, loff_t *off)
{
    if (count > UMH_IPCFG_PATH_MAXLEN)
        count = UMH_IPCFG_PATH_MAXLEN;

    if (copy_from_user(umh_ipcfg_path, userBuf, count))
        return -EFAULT;

    umh_ipcfg_path[count-1] =0;

    return count;
}

static struct file_operations K_UMH_IpcfgPathProcFops = {
    .owner   = THIS_MODULE,
    .open    = K_UMH_IpcfgPathProcOpen,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
    .write   = K_UMH_IpcfgPathProcWrite,
};

/* PURPOSE: To create umh_ipcfg entry uner /proc.
 * INPUT  : fname.
 * OUTPUT : None
 * RETURN : pointer to proc_dir_entry, NULL if failed.
 * NOTES  : None.
 */
static struct proc_dir_entry *K_UMH_IpcfgPathProcCreate(char *fname)
{
    struct proc_dir_entry *tmp_proc;

    tmp_proc = proc_create(fname, S_IRUGO | S_IWUSR, NULL, &K_UMH_IpcfgPathProcFops);

    return tmp_proc;
}

/* PURPOSE: Write event handler for umh result entry under /proc.
 * INPUT  : TBD.
 * OUTPUT : None
 * RETURN : count for bytes written, -EFAULT if failed.
 * NOTES  : None.
 */
static ssize_t K_UMH_IpcfgResProcWrite(
    struct file *fp, const char __user *buffer, size_t count, loff_t *off)
{
    int     *data_p = PDE_DATA(file_inode(fp));
    int     param;
    char    str_buf[10];

    if (  (count > sizeof(str_buf) -1)
        ||(copy_from_user(str_buf, buffer, count)))
        return -EFAULT;

    str_buf[count -1] = '\0';

    sscanf(str_buf, "%d", &param);
    *data_p = param;
    K_UMH_DBG_MSG("param has been set to %d", param);

    return count;
}

static struct file_operations K_UMH_IpcfgResProcFops = {
    .owner   = THIS_MODULE,
    .write   = K_UMH_IpcfgResProcWrite,
};

/* PURPOSE: To create entry under /proc.
 * INPUT  : fname - name for entry
 * OUTPUT : None
 * RETURN : pointer to proc_dir_entry, NULL if failed.
 * NOTES  : None.
 */
static struct proc_dir_entry *K_UMH_IpcfgResProcCreate(char *fname, char *data_p)
{
    struct proc_dir_entry *tmp_proc;

    tmp_proc = proc_create_data(fname, S_IWUSR, NULL, &K_UMH_IpcfgResProcFops, data_p);

    return tmp_proc;
}

/* PURPOSE: To process ioctl cmd redirected from kernel space.
 * INPUT  :  is_nl - 1 if netlink msg, 0 if ioctl msg
 *          family - protocol family
 *             cmd - ioctl cmd
 *             arg - ioctl arg
 * OUTPUT :
 * RETURN :      0 - cmd process ok,    caller should not continue.
 *         -EFAULT - cmd process error, caller should not continue.
 *         -EINVAL - cmd not processed, caller should continue to do
 *                                      normal processing.
 * NOTES  : None.
 */
static int K_UMH_CustomSockIoctl(int is_nl, int family, unsigned int cmd, unsigned long arg)
{
    int  ret = -EFAULT;
    int  umh_ret;
    int  proc_res;
    char *argv[] = { umh_ipcfg_path, NULL };
    char tag_ar[6][30];
    char tag_ar_b[100];
    char proc_name[30];
    char *envp[] = {
          tag_ar[0],
          tag_ar[1],
          tag_ar[2],
          tag_ar[3],
          tag_ar[4],
          tag_ar[5],
          tag_ar_b,
          "HOME=/",
          "TERM=linux",
          "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

    /* do normal processing when it's netlink msg from AOS
     */
    if (K_UMH_NETLINK_MSG == is_nl)
    {
        char tmp;

        if (SYSFUN_OK == SYSFUN_TaskIDToName(0, &tmp, 1))
            return -EINVAL;
    }

    if (0 != K_UMH_SetupEnv(envp, is_nl, family, cmd, arg))
        return ret;

    if (K_UMH_NETLINK_MSG == is_nl)
        cmd = (cmd == RTM_NEWADDR) ? SIOCSIFADDR: SIOCDIFADDR;

    sprintf(proc_name, UMH_RES_PROC_ENTRY_FMT, current->pid, cmd);
    if (! K_UMH_IpcfgResProcCreate(proc_name, (char *)&proc_res))
        return -EINVAL;

    umh_ret = call_usermodehelper( argv[0], argv, envp, UMH_WAIT_PROC );
    if (umh_ret <0)
    {
        /* fail to execute user mode helper, go normal process ??
         */
        K_UMH_DBG_MSG("umh fail -%d", umh_ret);

        if (umh_ret != -ENOMEM)
            ret = -EINVAL;
    }
    else
    {
        ret = proc_res;
        K_UMH_DBG_MSG("ret-%d", ret);

        switch (ret)
        {
        case 0:
            break;
        case EINVAL:
            ret = -EINVAL;
            break;
        default:
            ret = -EFAULT;
            break;
        }
    }

    remove_proc_entry(proc_name, NULL);

    return ret;
}

#else

static int K_UMH_IpcfgPathProcWrite(
    struct file *file, const char __user * buffer, unsigned long count, void *data)
{
    if (count > UMH_IPCFG_PATH_MAXLEN)
        count = UMH_IPCFG_PATH_MAXLEN;

    if (copy_from_user(umh_ipcfg_path, buffer, count))
        return -EFAULT;

    umh_ipcfg_path[count-1] =0;

    return count;
}

/* PURPOSE: Read event handler for umh_ipcfg uner /proc.
 * INPUT  : TBD.
 * OUTPUT : None
 * RETURN : count for bytes read.
 * NOTES  : None.
 */
static int K_UMH_IpcfgPathProcRead (
    char *buf, char **start, off_t offset, int len, int *eof, void *unused)
{
    *eof = 1;
    return sprintf(buf, "%s\n", umh_ipcfg_path);
}

/* PURPOSE: To create umh_ipcfg entry uner /proc.
 * INPUT  : fname.
 * OUTPUT : None
 * RETURN : pointer to proc_dir_entry, NULL if failed.
 * NOTES  : None.
 */
static struct proc_dir_entry *K_UMH_IpcfgPathProcCreate(char *fname)
{
    struct proc_dir_entry *my_proc;

    my_proc = create_proc_entry(fname, S_IRUGO | S_IWUSR, NULL);
    if (!my_proc){
        K_UMH_DBG_MSG("failed to make %s", fname);
        return NULL;
    }
    K_UMH_DBG_MSG("created %s", fname);
    my_proc->read_proc  = K_UMH_IpcfgPathProcRead;
    my_proc->write_proc = K_UMH_IpcfgPathProcWrite;

    return my_proc;
}

/* PURPOSE: Write event handler for umh result entry under /proc.
 * INPUT  : TBD.
 * OUTPUT : None
 * RETURN : count for bytes written, -EFAULT if failed.
 * NOTES  : None.
 */
static int K_UMH_IpcfgResProcWrite(
    struct file *file, const char __user *buffer, unsigned long count, void *data)
{
    char    str_buf[10];
    int     param;

    if (  (count > sizeof(str_buf) -1)
        ||(copy_from_user(str_buf, buffer, count)))
        return -EFAULT;

    str_buf[count -1] = '\0';

    sscanf(str_buf, "%d", &param);
    *((int *)data) = param;
    K_UMH_DBG_MSG("param has been set to %d", param);

    return count;
}

/* PURPOSE: To create entry under /proc.
 * INPUT  : fname - name for entry
 * OUTPUT : None
 * RETURN : pointer to proc_dir_entry, NULL if failed.
 * NOTES  : None.
 */
static struct proc_dir_entry *K_UMH_IpcfgResProcCreate(char *fname)
{
    struct proc_dir_entry *my_proc;

    my_proc = create_proc_entry(fname, S_IWUSR, NULL);
    if (!my_proc){
        K_UMH_DBG_MSG("failed to make %s", fname);
        return NULL;
    }
    K_UMH_DBG_MSG("created %s", fname);
    //my_proc->read_proc = my_proc_read;
    my_proc->write_proc = K_UMH_IpcfgResProcWrite;
    my_proc->data = kmalloc(sizeof(int), GFP_KERNEL);

    /* init value to run default case in K_UMH_CustomSockIoctl.
     */
    *((int *)my_proc->data) = 0xff01;

    return my_proc;
}

/* PURPOSE: To process ioctl cmd redirected from kernel space.
 * INPUT  :  is_nl - 1 if netlink msg, 0 if ioctl msg
 *          family - protocol family
 *             cmd - ioctl cmd
 *             arg - ioctl arg
 * OUTPUT :
 * RETURN :      0 - cmd process ok,    caller should not continue.
 *         -EFAULT - cmd process error, caller should not continue.
 *         -EINVAL - cmd not processed, caller should continue to do
 *                                      normal processing.
 * NOTES  : None.
 */
static int K_UMH_CustomSockIoctl(int is_nl, int family, unsigned int cmd, unsigned long arg)
{
    int                     ret = -EFAULT;
    int                     umh_ret;
    struct proc_dir_entry   *my_proc;
    char *argv[] = { umh_ipcfg_path, NULL };
    char tag_ar[6][30];
    char tag_ar_b[100];
    char proc_name[30];
    char *envp[] = {
          tag_ar[0],
          tag_ar[1],
          tag_ar[2],
          tag_ar[3],
          tag_ar[4],
          tag_ar[5],
          tag_ar_b,
          "HOME=/",
          "TERM=linux",
          "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

    /* do normal processing when it's netlink msg from AOS
     */
    if (K_UMH_NETLINK_MSG == is_nl)
    {
        char tmp;

        if (SYSFUN_OK == SYSFUN_TaskIDToName(0, &tmp, 1))
            return -EINVAL;
    }

    if (0 != K_UMH_SetupEnv(envp, is_nl, family, cmd, arg))
        return ret;

    if (K_UMH_NETLINK_MSG == is_nl)
        cmd = (cmd == RTM_NEWADDR) ? SIOCSIFADDR: SIOCDIFADDR;

    sprintf(proc_name, UMH_RES_PROC_ENTRY_FMT, current->pid, cmd);
    my_proc=K_UMH_IpcfgResProcCreate(proc_name);

    umh_ret = call_usermodehelper( argv[0], argv, envp, UMH_WAIT_PROC );
    if (umh_ret <0)
    {
        /* fail to execute user mode helper, go normal process ??
         */
        K_UMH_DBG_MSG("umh fail -%d", umh_ret);

        if (umh_ret != -ENOMEM)
            ret = -EINVAL;
    }
    else
    {
        ret = *((int *)my_proc->data);
        K_UMH_DBG_MSG("ret-%d", ret);

        switch (ret)
        {
        case 0:
            break;
        case EINVAL:
            ret = -EINVAL;
            break;
        default:
            ret = -EFAULT;
            break;
        }
    }

    kfree(my_proc->data);
    remove_proc_entry(proc_name, NULL);

    return ret;
}
#endif /* #if ( LINUX_VERSION_CODE > KERNEL_VERSION(3,10,0) ) */

/* PURPOSE: To setup ipv4 env vars for user mode helper of ioctl msg.
 * INPUT  : family - protocol family
 *             cmd - ioctl cmd
 *             arg - ioctl arg
 * OUTPUT :   envp - env vars
 * RETURN :      0 - cmd process ok,    caller should not continue.
 *         -EFAULT - cmd process error, caller should not continue.
 *         -EINVAL - cmd not processed, caller should continue to do
 *                                      normal processing.
 * NOTES  : None.
 */
static int K_UMH_SetupEnvIp4(char *envp[], int family, unsigned int cmd, unsigned long arg)
{
    int                 ret =0;
    struct ifreq        ifr;

    if (copy_from_user(&ifr, (void __user *)arg, sizeof(ifr)))
        ret = -EFAULT;

    if (0 == ret)
    {
        ifr.ifr_name[IFNAMSIZ - 1] = 0;

        switch (cmd)
        {
        case SIOCSIFNETMASK:
        case SIOCSIFADDR:
            {   //name, addr, mask
                struct sockaddr_in *addr_p = (struct sockaddr_in *) &ifr.ifr_addr;
                // sin_addr.s_addr is in network order
                unsigned char *tmp_cp = (unsigned char *)&addr_p->sin_addr.s_addr;

                sprintf(envp[0], "%s=%d", IF_TAG_PID,  current->pid);
                sprintf(envp[1], "%s=%s", IF_TAG_COMM, current->comm);
                sprintf(envp[2], "%s=%d", IF_TAG_CMD,  cmd);
                sprintf(envp[3], "%s=%d", IF_TAG_FAMI, family);
                sprintf(envp[4], "%s=%s", IF_TAG_NAME, ifr.ifr_name);
                sprintf(envp[5], "%s=%d.%d.%d.%d",
                    (cmd == SIOCSIFADDR) ? IF_TAG_ADDR : IF_TAG_MASK,
                    (int)tmp_cp[0], (int)tmp_cp[1], (int)tmp_cp[2], (int)tmp_cp[3]
                    );
                ret = 0;
            }
            break;
        default:
            ret = -EINVAL;
            break;
        }
    }

    return ret;
}

/* PURPOSE: To setup ipv6 env vars for user mode helper of ioctl msg.
 * INPUT  : family - protocol family
 *             cmd - ioctl cmd
 *             arg - ioctl arg
 * OUTPUT :   envp - env vars
 * RETURN :      0 - cmd process ok,    caller should not continue.
 *         -EFAULT - cmd process error, caller should not continue.
 *         -EINVAL - cmd not processed, caller should continue to do
 *                                      normal processing.
 * NOTES  : None.
 */
static int K_UMH_SetupEnvIp6(char *envp[], int family, unsigned int cmd, unsigned long arg)
{
    int                 ret =0;
    struct in6_ifreq    ifr6;

    if (copy_from_user(&ifr6, (void __user *)arg, sizeof(ifr6)))
        ret = -EFAULT;

    if (0 == ret)
    {
        switch (cmd)
        {
//        case SIOCSIFNETMASK:
        case SIOCSIFADDR:
        case SIOCDIFADDR:
            {   //ifidx, addr, prefix len
                sprintf(envp[0], "%s=%d", IF_TAG_PID,  current->pid);
                sprintf(envp[1], "%s=%s", IF_TAG_COMM, current->comm);
                sprintf(envp[2], "%s=%d", IF_TAG_CMD,  cmd);
                sprintf(envp[3], "%s=%d", IF_TAG_FAMI, family);
                sprintf(envp[4], "%s=%d", IF_TAG_NAME, ifr6.ifr6_ifindex);
                sprintf(envp[5], "%s=%u", IF_TAG_MASK, ifr6.ifr6_prefixlen);
                sprintf(envp[6], "%s=%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                    IF_TAG_ADDR,
                    ifr6.ifr6_addr.s6_addr16[0],
                    ifr6.ifr6_addr.s6_addr16[1],
                    ifr6.ifr6_addr.s6_addr16[2],
                    ifr6.ifr6_addr.s6_addr16[3],
                    ifr6.ifr6_addr.s6_addr16[4],
                    ifr6.ifr6_addr.s6_addr16[5],
                    ifr6.ifr6_addr.s6_addr16[6],
                    ifr6.ifr6_addr.s6_addr16[7]
                    );
                ret = 0;
            }
            break;
        default:
            ret = -EINVAL;
            break;
        }
    }

    return ret;
}

/* PURPOSE: To setup ipv4 env vars for user mode helper of netlink msg.
 * INPUT  : family - protocol family
 *             cmd - ioctl cmd
 *             arg - ioctl arg
 * OUTPUT :   envp - env vars
 * RETURN :      0 - cmd process ok,    caller should not continue.
 *         -EFAULT - cmd process error, caller should not continue.
 *         -EINVAL - cmd not processed, caller should continue to do
 *                                      normal processing.
 * NOTES  : None.
 */
static int K_UMH_SetupEnvIp4_NL(char *envp[], int family, unsigned int cmd, unsigned long arg)
{
    int                 ret =0;
    struct in_ifaddr    *ifa = (void *) arg;

    if (NULL != ifa)
    {
        switch (cmd)
        {
        case RTM_NEWADDR:
        case RTM_DELADDR:
            {   //name, addr, mask
                char *dev_name_p;
                // ifa->ifa_address is in network order */
                unsigned char *tmp_cp;

                sprintf(envp[0], "%s=%d", IF_TAG_PID,  current->pid);
                sprintf(envp[1], "%s=%s", IF_TAG_COMM, current->comm);
                sprintf(envp[2], "%s=%d", IF_TAG_CMD,
                            (cmd == RTM_DELADDR) ? SIOCDIFADDR : SIOCSIFADDR);
                sprintf(envp[3], "%s=%d", IF_TAG_FAMI, family);

                dev_name_p = (NULL == ifa->ifa_dev) ?
                                ifa->ifa_label : ifa->ifa_dev->dev->name;
                sprintf(envp[4], "%s=%s", IF_TAG_NAME, dev_name_p);
                tmp_cp = (unsigned char *)&ifa->ifa_address;
                sprintf(envp[5], "%s=%d.%d.%d.%d",
                    IF_TAG_ADDR,
                    (int)tmp_cp[0], (int)tmp_cp[1], (int)tmp_cp[2], (int)tmp_cp[3]
                    );
                tmp_cp = (unsigned char *)&ifa->ifa_mask;
                sprintf(envp[6], "%s=%d.%d.%d.%d",
                    IF_TAG_MASK,
                    (int)tmp_cp[0], (int)tmp_cp[1], (int)tmp_cp[2], (int)tmp_cp[3]
                    );
            }
            break;
        default:
            ret = -EINVAL;
            break;
        }
    }

    return ret;
}

/* PURPOSE: To setup ipv6 env vars for user mode helper of netlink msg.
 * INPUT  : family - protocol family
 *             cmd - ioctl cmd
 *             arg - ioctl arg
 * OUTPUT :   envp - env vars
 * RETURN :      0 - cmd process ok,    caller should not continue.
 *         -EFAULT - cmd process error, caller should not continue.
 *         -EINVAL - cmd not processed, caller should continue to do
 *                                      normal processing.
 * NOTES  : None.
 */
static int K_UMH_SetupEnvIp6_NL(char *envp[], int family, unsigned int cmd, unsigned long arg)
{
    int                 ret =0;
    struct inet6_ifaddr *ifa6 = (void *) arg;

    if (NULL != ifa6)
    {
        switch (cmd)
        {
        case RTM_NEWADDR:
        case RTM_DELADDR:
            {   //ifidx, addr, prefix len
                sprintf(envp[0], "%s=%d", IF_TAG_PID,  current->pid);
                sprintf(envp[1], "%s=%s", IF_TAG_COMM, current->comm);
                sprintf(envp[2], "%s=%d", IF_TAG_CMD,
                            (cmd == RTM_DELADDR) ? SIOCDIFADDR : SIOCSIFADDR);
                sprintf(envp[3], "%s=%d", IF_TAG_FAMI, family);
                sprintf(envp[4], "%s=%lu", IF_TAG_NAME, ifa6->cstamp);
                sprintf(envp[5], "%s=%u",  IF_TAG_MASK, ifa6->prefix_len);
                sprintf(envp[6], "%s=%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",
                    IF_TAG_ADDR,
                    (ifa6->addr.s6_addr[0] << 8 | ifa6->addr.s6_addr[1]),
                    (ifa6->addr.s6_addr[2] << 8 | ifa6->addr.s6_addr[3]),
                    (ifa6->addr.s6_addr[4] << 8 | ifa6->addr.s6_addr[5]),
                    (ifa6->addr.s6_addr[6] << 8 | ifa6->addr.s6_addr[7]),
                    (ifa6->addr.s6_addr[8] << 8 | ifa6->addr.s6_addr[9]),
                    (ifa6->addr.s6_addr[10] << 8 | ifa6->addr.s6_addr[11]),
                    (ifa6->addr.s6_addr[12] << 8 | ifa6->addr.s6_addr[13]),
                    (ifa6->addr.s6_addr[14] << 8 | ifa6->addr.s6_addr[15])
                    );
            }
            break;
        default:
            ret = -EINVAL;
            break;
        }
    }

    return ret;
}

/* PURPOSE: To setup env vars for user mode helper.
 * INPUT  :  is_nl - 1 if netlink msg, 0 if ioctl msg
 *          family - protocol family
 *             cmd - ioctl cmd
 *             arg - ioctl arg
 * OUTPUT :   envp - env vars
 * RETURN :      0 - cmd process ok,    caller should not continue.
 *         -EFAULT - cmd process error, caller should not continue.
 *         -EINVAL - cmd not processed, caller should continue to do
 *                                      normal processing.
 * NOTES  : None.
 */
static int K_UMH_SetupEnv(
    char *envp[], int is_nl, int family, unsigned int cmd, unsigned long arg)
{
    int ret =-EINVAL;

    envp[6][0] = '\0';

    if (K_UMH_NETLINK_MSG == is_nl)
    {
        switch (family)
        {
        case PF_INET:
            return K_UMH_SetupEnvIp4_NL(envp, family, cmd, arg);

        case PF_INET6:
            return K_UMH_SetupEnvIp6_NL(envp, family, cmd, arg);

        default:
            break;
        }
    }
    else
    {
        switch (family)
        {
        case PF_INET:
            return K_UMH_SetupEnvIp4(envp, family, cmd, arg);

        case PF_INET6:
            return K_UMH_SetupEnvIp6(envp, family, cmd, arg);

        default:
            break;
        }
    }

    return ret;
}

#endif /* #if (SYS_CPNT_NETCFG_IP_REDIRECT == TRUE) */

