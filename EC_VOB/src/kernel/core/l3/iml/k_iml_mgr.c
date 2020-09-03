/* MODULE NAME:  k_iml_mgr.c
 * PURPOSE:
 *   IML_MGR implementations for linux kernel.
 *
 * NOTES:
 *
 * HISTORY
 *    7/23/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sys_module.h"
#include "k_sysfun.h"
#include "k_l_mm.h"
#include "k_l_ipcmem.h"
#include "l_cvrt.h"

#include "iml_type.h"
#include "k_iml_mgr.h"
#include "vlan_net.h"

/* linux kernel header files
 */
#include "linux/etherdevice.h"
#include "net/ip.h"
#include "net/ipv6.h"
#include "linux/wait.h"
#include "linux/spinlock.h"
#include "linux/string.h"
#include "linux/version.h"
#include <linux/spinlock.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
/* #define K_IML_MGR_DEBUG */

#ifdef K_IML_MGR_DEBUG
#define IML_MGR_DBG printk
#else
#define IML_MGR_DBG(...)
#endif

#define IML_MGR_ENTER_CRITICAL_SECTION SYSFUN_EnterCriticalSection
#define IML_MGR_LEAVE_CRITICAL_SECTION SYSFUN_LeaveCriticalSection

/* DATA TYPE DECLARATIONS
 */

typedef struct IML_MGR_IPPktEntry_S
{
    L_MM_Mref_Handle_T *mref_handle_p;
    UI16_T              vid;
} IML_MGR_IPPktEntry_T;

typedef struct IML_MGR_IPPktListCB_S
{
    UI16_T               ip_pkt_list_head;
    UI16_T               ip_pkt_list_tail;
    UI16_T               pkt_count;
    spinlock_t          lock;
    wait_queue_head_t    wq;
    IML_MGR_IPPktEntry_T iml_mgr_tx_ippkt_buffer[SYS_ADPT_MAX_NBR_OF_IML_TX_IP_PKT];
} IML_MGR_IPPktListCB_T;

/* TBD:move to iml_mgr.h */
typedef struct IML_MGR_RecvPktFromTCPIPStackArgs_S
{
    UI16_T tag_info;
    UI16_T packet_type;
    UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
} IML_MGR_RecvPktFromTCPIPStackArgs_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void IML_MGR_SkbReleaseCallback(struct sk_buff *skb);
static UI32_T IML_MGR_SendPktToTCPIPStack(UI32_T mref_handle_offset, UI16_T tag_info, BOOL_T is_tagged_packet, UI32_T process_packet_type);
static UI32_T IML_MGR_RecvPktFromTCPIPStack(UI32_T *mref_handle_offset_p, IML_MGR_RecvPktFromTCPIPStackArgs_T *args_p);
static UI32_T IML_MGR_DeqFromIPTxPktList(L_MM_Mref_Handle_T **mref_handle_pp, UI32_T *vid_p);
static UI32_T IML_MGR_Syscall(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4);
extern int arp_rcv(struct sk_buff *skb, struct net_device *dev,
                   struct packet_type *pt, struct net_device *orig_dev);

/* STATIC VARIABLE DECLARATIONS
 */
static IML_MGR_IPPktListCB_T ip_tx_pkt_list_cb;

static UI32_T skb_cb_rx_flag = 0;
static UI32_T callback_cookie = 0;
static wait_queue_head_t rx_lock_wq;
static DEFINE_SPINLOCK(skb_cb_rx_flag_lock);
static DEFINE_SPINLOCK(ip_tx_pkt_lis_lock);

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   Initialize IML_MGR.
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
void IML_MGR_Init(void)
{
    spin_lock_init(&(ip_tx_pkt_list_cb.lock));
    init_waitqueue_head(&(ip_tx_pkt_list_cb.wq));
    ip_tx_pkt_list_cb.pkt_count=
        ip_tx_pkt_list_cb.ip_pkt_list_head=
        ip_tx_pkt_list_cb.ip_pkt_list_tail=0;

    init_waitqueue_head(&rx_lock_wq);
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_Create_InterCSC_Relation
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
void IML_MGR_Create_InterCSC_Relation(void)
{
    /* TBD: add SYSFUN_SYSCALL_IML_MGR to SYSFUN_SYSCALL_CMD_ID_E in sysfun.h
     */
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_IML_MGR, IML_MGR_Syscall);
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_EnqToIPTxPktList
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   This function will enqueue the ip packet from linux kernel tcp/ip stack
 *   to the tail of the tx packet list.
 *
 * 	INPUT:
 *   skb  --  The packet to be transmitted. This skb will be freed when return.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   IML_TYPE_RETVAL_OK                           --  Success
 *   IML_TYPE_RETVAL_L_MM_ALLOCATE_TX_BUFFER_FAIL --  Fail to allocate tx buffer
 *   IML_TYPE_RETVAL_TX_BUF_FULL                  --  IP packet tx buf is full
 *   IML_TYPE_RETVAL_GET_VID_FROM_VLAN_DEV_ERROR  --  Fail to get vid from vlan net device
 *
 * 	NOTES:
 *   1. Need not to free skb in this function, the caller VLAN_NET_hard_start_xmit
 *      will take care of it.
 *   2. Pdu of the output mref_handle will point to the ip header.
 *
 *--------------------------------------------------------------------------
 */
UI32_T IML_MGR_EnqToIPTxPktList(struct sk_buff *skb)
{
    SYSFUN_IntMask_T oint;
    UI32_T prev_caller_tid=0;
    uintptr_t prev_caller_addr=0;
    UI32_T vid, pdu_len;
    L_MM_Mref_Handle_T *mref_handle_p;
    UI8_T              *pdu;

    IML_MGR_DBG("Enter IML_MGR_EnqToIPTxPktList\r\n");

    vid = VLAN_NET_GetVidFromVlanDev(skb->dev);
    if(vid==0)
    {
        IML_MGR_DBG("%s:VLAN_NET_GetVidFromVlanDev fail\r\n", __FUNCTION__);
        return IML_TYPE_RETVAL_GET_VID_FROM_VLAN_DEV_ERROR;
    }

    /* check whether the list is full */
    IML_MGR_ENTER_CRITICAL_SECTION(&ip_tx_pkt_lis_lock, oint);
    if(ip_tx_pkt_list_cb.pkt_count>=SYS_ADPT_MAX_NBR_OF_IML_TX_IP_PKT)
    {
        if (!netif_queue_stopped(skb->dev))
            netif_stop_queue(skb->dev);
        IML_MGR_DBG("%s:ip_tx_pkt_list_cb is full\r\n", __FUNCTION__);
        IML_MGR_LEAVE_CRITICAL_SECTION(&ip_tx_pkt_lis_lock, oint);
        return IML_TYPE_RETVAL_TX_BUF_FULL;
    }

    /* convert skb to mref */
    /* space for ethernet header is already reserved by K_L_MM_AllocateTxBuffer
     * padding to 60 bytes if the total packet size is less than 60 bytes
     * (would be 64 bytes including CRC)
     */
    mref_handle_p=K_L_MM_AllocateTxBuffer((skb->len < 60)? 46: skb->len - 14, L_MM_USER_ID2(SYS_MODULE_IML, IML_TYPE_TRACE_ID_ENQ_TO_IP_PKT_LIST));
    if(mref_handle_p==NULL)
    {
        IML_MGR_DBG("%s:K_L_MM_AllocateTxBuffer fail\r\n", __FUNCTION__);
        IML_MGR_LEAVE_CRITICAL_SECTION(&ip_tx_pkt_lis_lock, oint);
        return IML_TYPE_RETVAL_L_MM_ALLOCATE_TX_BUFFER_FAIL;
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

    IML_MGR_DBG("%s:pdu_len=%d,skb->len=%d\r\n", __FUNCTION__, (int)pdu_len, skb->len);
    IML_MGR_DBG("%s:da=0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\r\n", __FUNCTION__, pdu[-14], pdu[-13], pdu[-12], pdu[-11], pdu[-10], pdu[-9]);
    IML_MGR_DBG("%s:sa=0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\r\n", __FUNCTION__, pdu[-8], pdu[-7], pdu[-6], pdu[-5], pdu[-4], pdu[-3]);
    IML_MGR_DBG("%s:packet_type=0x%02x-0x%02x\r\n", __FUNCTION__, pdu[-2], pdu[-1]);

    IML_MGR_DBG("%s:ip header first 4 bytes:0x%02x 0x%02x 0x%02x 0x%02x\r\n", __FUNCTION__, pdu[0], pdu[1], pdu[2], pdu[3]);

    /* append to the tail of the ip pkt list */
    ip_tx_pkt_list_cb.iml_mgr_tx_ippkt_buffer[ip_tx_pkt_list_cb.ip_pkt_list_tail].mref_handle_p=mref_handle_p;
    ip_tx_pkt_list_cb.iml_mgr_tx_ippkt_buffer[ip_tx_pkt_list_cb.ip_pkt_list_tail].vid=vid;
    ip_tx_pkt_list_cb.ip_pkt_list_tail++;
    ip_tx_pkt_list_cb.ip_pkt_list_tail %= SYS_ADPT_MAX_NBR_OF_IML_TX_IP_PKT;
    ip_tx_pkt_list_cb.pkt_count++;
    IML_MGR_LEAVE_CRITICAL_SECTION(&ip_tx_pkt_lis_lock, oint);

    IML_MGR_DBG("%s:ip_pkt_count=%d\r\n", __FUNCTION__, (int)ip_tx_pkt_list_cb.pkt_count);

    wake_up(&(ip_tx_pkt_list_cb.wq));

    return IML_TYPE_RETVAL_OK;
}

/* LOCAL SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME : IML_MGR_SkbReleaseCallback
 *--------------------------------------------------------------------------
 * PURPOSE:
 *    Callback function when the skb_buff is released
 *
 * INPUT:
 *    skb   -- target skb_buff which is freed
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None
 *
 * NOTES:
 *    The place where callback function is called is in kfree_skb() of skbbuff.c
 *--------------------------------------------------------------------------
 */
static void IML_MGR_SkbReleaseCallback(struct sk_buff *skb)
{
    SYSFUN_IntMask_T oint;

    IML_MGR_ENTER_CRITICAL_SECTION(&skb_cb_rx_flag_lock, oint);
    if (skb->callback_cookie == ~skb_cb_rx_flag)
    {
        skb_cb_rx_flag = skb->callback_cookie;
        IML_MGR_LEAVE_CRITICAL_SECTION(&skb_cb_rx_flag_lock, oint);

        wake_up(&rx_lock_wq);
    }
    else
        IML_MGR_LEAVE_CRITICAL_SECTION(&skb_cb_rx_flag_lock, oint);
}


/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_SendPktToTCPIPStack
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   This function will send packet in format of mref to linux kernel tcp/ip
 *   stack.
 *
 * 	INPUT:
 *   mref_handle_offset  -- offset of mref_handle_p for L_IPCMEM to get pointer
 *   tag_info            -- vlan tag info of the incoming packet which is contained
 *                          in mref
 *   is_tagged_packet    -- TRUE if the incomping packet in mref is tagged
 *                          FALSE otherwise
 *   process_packet_type -- indicate how packet is processed
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   IML_TYPE_RETVAL_OK                    --  Success
 *   IML_TYPE_RETVAL_NET_DEVICE_NOT_EXIST  --  The net device of the correspoding vlan
 *                                             from where the packet comes cannot be found
 *   IML_TYPE_RETVAL_MREF_ERROR            --  An error occurs when operates on mref handle
 *   IML_TYPE_RETVAL_ALLOC_SKB_FAIL        --  Fail to allocate socket buffer
 *
 * 	NOTES:
 *   Pdu of the received mref_handle must point to ethernet header.
 *--------------------------------------------------------------------------
 */
static UI32_T IML_MGR_SendPktToTCPIPStack(UI32_T mref_handle_offset, UI16_T tag_info, BOOL_T is_tagged_packet, UI32_T process_packet_type)
{
    L_MM_Mref_Handle_T *mref_handle_p;
    UI32_T              pdu_len, prev_caller_tid=0;
    uintptr_t           prev_caller_addr=0;
    struct net_device  *vlan_net_dev_p;
    I16_T               buf_size;
    UI8_T              *pdu;
    struct sk_buff     *skb;
    SYSFUN_IntMask_T    oint;
    int rc;

    mref_handle_p = (L_MM_Mref_Handle_T*)K_L_IPCMEM_GetPtr(mref_handle_offset);
    pdu = K_L_MM_Mref_GetPdu(mref_handle_p, __builtin_return_address(0), &pdu_len, &prev_caller_addr, &prev_caller_tid);
    if (prev_caller_addr!=0)
    {
        /* this error message can be logged to flash when next Get PDU operation
         * is performed from user space
         */
        printk("<0>%s(%d)prev_caller_addr=%p tid=%lu\n", __FUNCTION__, __LINE__, (void*)prev_caller_addr, (unsigned long)prev_caller_tid);
    }

    if(pdu==NULL)
    {
        return IML_TYPE_RETVAL_MREF_ERROR;
    }

    vlan_net_dev_p = VLAN_NET_GetVlanDevFromVid(tag_info & 0x0fff);

    /*no L3 interface exist*/
    if(vlan_net_dev_p==NULL)
    {
        K_L_MM_Mref_Release(&mref_handle_p);
        return IML_TYPE_RETVAL_NET_DEVICE_NOT_EXIST;
    }

    /* need to strip vlan tag */
    buf_size = (I16_T)pdu_len + ((is_tagged_packet)?(-4):0);

    /* convert mref into sk_buff */
    skb = alloc_skb(buf_size+2, GFP_KERNEL);
    if(skb==NULL)
    {
        K_L_MM_Mref_Release(&mref_handle_p);
        return IML_TYPE_RETVAL_ALLOC_SKB_FAIL;
    }

    /* for ip addr in ip header can align to 4-byte boundary */
    skb_reserve(skb, 2);
    /* copy buffer from mref to skb */
    {
        UI8_T *dest_p;

        dest_p=skb_put(skb, buf_size);
        if(is_tagged_packet==TRUE)
        {
            /* copy ethernet header without vlan tag */
            memcpy(dest_p, pdu, 12);
            /* copy payload */
            memcpy(dest_p+12, pdu+16, buf_size-12);
        }
        else
            memcpy(dest_p, pdu, buf_size);
    }

    /* When egress interface is the same as ingress interface, packets will be copied to CPU
     * with reason bcmRxReasonIcmpRedirect. These packets needn't do software routing.
     * The following code is added to drop packets after the sending of ICMP redirect
     * in Linux kernel. These kind of packets will be marked with "skb->not_forward == 1"
     * and dropped at ip_forward() or ip6_forward()
     */
    if (mref_handle_p->pkt_info.not_forward)
        skb->not_forward = 1;

    K_L_MM_Mref_Release(&mref_handle_p);

    /* setup skb fields */
    skb->dev = vlan_net_dev_p;
    skb->protocol=eth_type_trans(skb, vlan_net_dev_p);
    skb->ip_summed=CHECKSUM_NONE;

    if (process_packet_type == IML_TYPE_PROCESS_PACKET_DIRECT_CALL)
    {
        /* The following code snippet needs to refer to  netif_receive_skb()
         * So it might be changed among different linux kernel version.
         * To minimize the porting effort, maybe we should call netif_receive_skb()
         * instead of doing dispatch by ourself.
         */
#if (KERNEL_VERSION(2,6,19) == LINUX_VERSION_CODE)
        skb->h.raw = skb->nh.raw = skb->data;
    	skb->mac_len = skb->nh.raw - skb->mac.raw;
#elif ((KERNEL_VERSION(2,6,22) == LINUX_VERSION_CODE) || \
       (KERNEL_VERSION(2,6,32) == LINUX_VERSION_CODE) || \
       (KERNEL_VERSION(3,8,13) == LINUX_VERSION_CODE) || \
       (KERNEL_VERSION(3,2,65) == LINUX_VERSION_CODE) || \
       (KERNEL_VERSION(4,19,67) == LINUX_VERSION_CODE))
    	skb_reset_network_header(skb);
    	skb_reset_transport_header(skb);
    	skb->mac_len = skb->network_header - skb->mac_header;
#else
        #error "Please check netif_receive_skb() for correct implemrntation."
#endif

        /* invoke linux kernel ip pkt entry function
         */
        IML_MGR_DBG("%s:skb->pkt_type=%d,skb->protocol=%d\r\n", __FUNCTION__, (int)skb->pkt_type, (int)skb->protocol);

        if(skb->protocol==htons(ETH_P_ARP)) /* arp packet */
        {
            rc=arp_rcv(skb, vlan_net_dev_p, NULL, NULL);
            IML_MGR_DBG("%s:arp_rcv rc=%d\r\n", __FUNCTION__, rc);
        }
        else if(skb->protocol==htons(ETH_P_IP)) /* ip packet */
        {
            rc=ip_rcv(skb, vlan_net_dev_p, NULL, NULL);
            IML_MGR_DBG("%s:ip_rcv rc=%d\r\n", __FUNCTION__, rc);
        }
#if defined(SYS_CPNT_IPV6) && (SYS_CPNT_IPV6 == TRUE)
        else if(skb->protocol==htons(ETH_P_IPV6)) /* ipv6 packet */
        {
            rc=ipv6_rcv(skb, vlan_net_dev_p, NULL, NULL);
            IML_MGR_DBG("%s:ipv6_rcv rc=%d\r\n", __FUNCTION__, rc);
        }
#endif
        else  /* other type of packet, use ip_rcv currently */
        {
            rc=ip_rcv(skb, vlan_net_dev_p, NULL, NULL);
            IML_MGR_DBG("%s:ip_rcv rc=%d (other)\r\n", __FUNCTION__, rc);
        }
    }
    else
    {
        if (process_packet_type == IML_TYPE_PROCESS_PACKET_NETIF_RX_WAIT)
        {
            skb->release_callback = IML_MGR_SkbReleaseCallback;
            skb->magic_word = skb;

            IML_MGR_ENTER_CRITICAL_SECTION(&skb_cb_rx_flag_lock, oint);
            skb->callback_cookie = ++callback_cookie;
            skb_cb_rx_flag = ~callback_cookie;
            IML_MGR_LEAVE_CRITICAL_SECTION(&skb_cb_rx_flag_lock, oint);
        }
        else
        {
            skb->release_callback = NULL;
        }

        rc = netif_rx(skb);
        if(rc!=NET_RX_SUCCESS)
        {
            printk("<7>IML_MGR netif_receive_skb error rc=%d\n", rc);
        }
        else if (process_packet_type == IML_TYPE_PROCESS_PACKET_NETIF_RX_WAIT)
        {
            wait_event_interruptible_timeout(rx_lock_wq, skb_cb_rx_flag == callback_cookie,(HZ * 20));
        }
    }

    return IML_TYPE_RETVAL_OK;
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_RecvPktFromTCPIPStack
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   This function will receive the packets coming from linux kernel tcp/ip
 *   stack.
 *
 * 	INPUT:
 *   None.
 *
 * 	OUTPUT:
 *   mref_handle_offset_p  -- offset of mref_handle_p for L_IPCMEM to get pointer
 *   args_p->tag_info      -- the vlan tag info for the outgoing ip packet
 *   args_p->packet_type   -- the packet type of the outgoing ip packet
 *   args_p->dst_mac       -- destination mac addr
 *   args_p->src_mac       -- source mac addr
 *
 *
 * 	RETURN:
 *   IML_TYPE_RETVAL_OK                    --  Success
 *   IML_TYPE_RETVAL_INVALID_ARG           --  Invalid input arguments
 *                                             from where the packet comes cannot be found
 *   IML_TYPE_RETVAL_INTR                  --  The operation is interrupted by a signal
 *   IML_TYPE_RETVAL_MREF_ERROR            --  An error occurs when operates on mref handle
 *
 * 	NOTES:
 *   Pdu of the output mref_handle will point to the ip header.
 *--------------------------------------------------------------------------
 */
static UI32_T IML_MGR_RecvPktFromTCPIPStack(UI32_T *mref_handle_offset_p, IML_MGR_RecvPktFromTCPIPStackArgs_T *args_p)
{
    L_MM_Mref_Handle_T *mref_handle_p;
    UI32_T           pdu_len,vid, ret, prev_caller_tid=0;
    uintptr_t        prev_caller_addr=0;
    UI8_T*           pdu;

    if(args_p==NULL)
        return IML_TYPE_RETVAL_INVALID_ARG;

    IML_MGR_DBG("%s:ip_pkt_count=%d\r\n", __FUNCTION__, (int)ip_tx_pkt_list_cb.pkt_count);

    /* blocked if ip pkt list is empty */
    if(ip_tx_pkt_list_cb.pkt_count==0)
    {
        int rc;

        rc=wait_event_interruptible(ip_tx_pkt_list_cb.wq, ip_tx_pkt_list_cb.pkt_count!=0);
        if(rc!=0)
            return IML_TYPE_RETVAL_INTR;
    }


    /* dequeue one node from ip_pkt_list head */
    if(IML_TYPE_RETVAL_OK != (ret=IML_MGR_DeqFromIPTxPktList(&mref_handle_p, &vid)))
    {
        IML_MGR_DBG("%s:IML_MGR_DeqFromIPTxPktList fail:ip_pkt_count=%d\r\n", __FUNCTION__, (int)ip_tx_pkt_list_cb.pkt_count);
        return ret;
    }

    IML_MGR_DBG("%s:IML_MGR_DeqFromIPTxPktList ok:ip_pkt_count=%d\r\n", __FUNCTION__, (int)ip_tx_pkt_list_cb.pkt_count);

    *mref_handle_offset_p = K_L_IPCMEM_GetOffset(mref_handle_p);
    pdu = K_L_MM_Mref_GetPdu(mref_handle_p, __builtin_return_address(0), &pdu_len, &prev_caller_addr, &prev_caller_tid);
    if (prev_caller_addr!=0)
    {
        /* this error message can be logged to flash when next Get PDU operation
         * is performed from user space
         */
        printk("<0>%s(%d)prev_caller_addr=%p tid=%lu\n", __FUNCTION__, __LINE__, (void*)prev_caller_addr, (unsigned long)prev_caller_tid);
    }

    if(pdu==NULL)
    {
        return IML_TYPE_RETVAL_MREF_ERROR;
    }

    /* set priority as 0 for now */
    args_p->tag_info = 0xfff & vid;
    memcpy(args_p->dst_mac, pdu-14 , SYS_ADPT_MAC_ADDR_LEN);
    memcpy(args_p->src_mac, pdu-8, SYS_ADPT_MAC_ADDR_LEN);
	args_p->packet_type = ntohs(*(UI16_T*)(pdu-2));

    return IML_TYPE_RETVAL_OK;
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_DeqFromIPTxPktList
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *   This function will dequeue the ip packet from the head of the tx packet list.
 *
 * 	INPUT:
 *   None.
 *
 * 	OUTPUT:
 *   mref_handle_pp  --  The mref handle of the packet to be transmitted.
 *   vid_p           --  The vid of the packet to be transmitted.
 *
 * 	RETURN:
 *   IML_TYPE_RETVAL_OK               --  Success
 *   IML_TYPE_RETVAL_TX_BUF_EMPTY     --  Tx packet buffer is empty
 *   IML_TYPE_RETVAL_INVALID_ARG      --  Invalid input arguments
 *
 * 	NOTES:
 *   Pdu of the output mref_handle will point to the ip header.
 *--------------------------------------------------------------------------
 */
static UI32_T IML_MGR_DeqFromIPTxPktList(L_MM_Mref_Handle_T **mref_handle_pp, UI32_T *vid_p)
{
    SYSFUN_IntMask_T oint;
    struct net_device  *vlan_net_dev_p;
    UI32_T vid;
    BOOL_T do_wake_queue = FALSE;

    if(ip_tx_pkt_list_cb.pkt_count==0)
        return IML_TYPE_RETVAL_TX_BUF_EMPTY;

    if(mref_handle_pp==NULL || vid_p==NULL)
        return IML_TYPE_RETVAL_INVALID_ARG;

    IML_MGR_ENTER_CRITICAL_SECTION(&ip_tx_pkt_lis_lock, oint);

    /* dequeue from the head of the ip pkt list */
    *mref_handle_pp=ip_tx_pkt_list_cb.iml_mgr_tx_ippkt_buffer[ip_tx_pkt_list_cb.ip_pkt_list_head].mref_handle_p;
    *vid_p = ip_tx_pkt_list_cb.iml_mgr_tx_ippkt_buffer[ip_tx_pkt_list_cb.ip_pkt_list_head].vid;

    ip_tx_pkt_list_cb.ip_pkt_list_head++;
    ip_tx_pkt_list_cb.ip_pkt_list_head %= SYS_ADPT_MAX_NBR_OF_IML_TX_IP_PKT;
    ip_tx_pkt_list_cb.pkt_count--;
    if (ip_tx_pkt_list_cb.pkt_count == SYS_ADPT_MAX_NBR_OF_IML_TX_IP_PKT / 3)
        do_wake_queue = TRUE;
    IML_MGR_LEAVE_CRITICAL_SECTION(&ip_tx_pkt_lis_lock, oint);

    if (do_wake_queue) {
        for (vid = 1; vid <= VLAN_NET_MAX_VID; vid++) {
            vlan_net_dev_p = VLAN_NET_GetVlanDevFromVid(vid);
            if (vlan_net_dev_p == NULL)
                continue;
            if (netif_queue_stopped(vlan_net_dev_p))
                netif_wake_queue(vlan_net_dev_p);
        }
    }

    return IML_TYPE_RETVAL_OK;
}

/* system call implementation
 */

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : IML_MGR_Syscall
 *--------------------------------------------------------------------------
 * 	PURPOSE:
 *  	This function implements the system call for IML_MGR.
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
static UI32_T IML_MGR_Syscall(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4)
{
    UI32_T ret;
    int cmd = L_CVRT_PTR_TO_UINT(arg0);

    switch(cmd)
    {
        case IML_TYPE_SYSCALL_CMD_SEND_PKT_TO_TCP_IP_STACK:
            /* arg1(IN): UI32_T mref_handle_offset
             * arg2(IN): UI16_T tag_info
             * arg3(IN): BOOL_T is_tagged_packet
             * arg4(IN): BOOL_T is_wait_finish;
             */
            ret=IML_MGR_SendPktToTCPIPStack(L_CVRT_PTR_TO_UINT(arg1), L_CVRT_PTR_TO_UINT(arg2), L_CVRT_PTR_TO_UINT(arg3), L_CVRT_PTR_TO_UINT(arg4));
            break;
        case IML_TYPE_SYSCALL_CMD_RECV_PKT_FROM_TCP_IP_STACK:
            /* arg1(OUT): UI32_T *mref_handle_offset_p
             * arg2(OUT): IML_MGR_RecvPktFromTCPIPStackArgs_T *args_p
             */
        {
            UI32_T mref_handle_offset;
            IML_MGR_RecvPktFromTCPIPStackArgs_T args;

            ret=IML_MGR_RecvPktFromTCPIPStack(&mref_handle_offset, &args);
            if(ret==IML_TYPE_RETVAL_OK)
            {
                SYSFUN_CopyToUser((void*)arg1, &mref_handle_offset, sizeof(UI32_T));
                SYSFUN_CopyToUser((void*)arg2, &args, sizeof(args));
            }
        }
            break;
        default:
            ret=IML_TYPE_RETVAL_UNKNOWN_SYSCALL_CMD;
    }

    return ret;
}

