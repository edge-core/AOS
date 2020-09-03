/* MODULE NAME:  vlan_net.c
 * PURPOSE:
 *     For manipulate craft port net device in kernel space.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    8/31/2010 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2010
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_mm.h"
#include "l_ipcmem.h"
#include "vlan_net.h"
#include "vlan_net_type.h"
#include "dev_nicdrv_gateway.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_CRAFT_PORT==TRUE)
static BOOL_T VLAN_NET_GetCraftPortTxPkt(L_MM_Mref_Handle_T **mref_handle_pp);
static void VLAN_NET_CraftPortTxTaskMain(void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
#if (SYS_CPNT_CRAFT_PORT==TRUE)
static UI32_T craft_port_tx_task_id;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
#if (SYS_CPNT_CRAFT_PORT==TRUE)
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_NET_SendPacketToCraftPortNetDevice
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Send a packet to craft port net device.
 *
 * 	INPUT:
 *   None.
 *
 * 	OUTPUT:
 *   pkt_buffer - packet to be sent to craft port net device.
 *   pkt_len    - packet length
 *
 * 	RETURN:
 *   None.
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
void VLAN_NET_SendPacketToCraftPortNetDevice(UI8_T * pkt_buffer, UI32_T pkt_len)
{
    SYSFUN_Syscall(SYSFUN_SYSCALL_VLAN_NET, VLAN_NET_SYSCALL_CMD_SEND_PACKET_TO_CRAFT_PORT_NET_DEVICE, (UI32_T)pkt_buffer, pkt_len, 0, 0);
}
#endif

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_NET_CreateTask
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   This function will spawn tasks for VLAN_NET
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
BOOL_T VLAN_NET_CreateTask(void)
{
#if (SYS_CPNT_CRAFT_PORT==TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
    if(SYSFUN_SpawnThread(SYS_BLD_CRAFT_PORT_TX_THREAD_PRIORITY,
                          SYS_BLD_CRAFT_PORT_TX_THREAD_SCHED_POLICY,
                          SYS_BLD_CRAFT_PORT_TX_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          VLAN_NET_CraftPortTxTaskMain,
                          NULL,
                          &craft_port_tx_task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread VLAN_NET_CraftPortTxTaskMain fail.\n", __FUNCTION__);
        return FALSE;
    }
#endif
    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
#if (SYS_CPNT_CRAFT_PORT==TRUE)
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_NET_GetCraftPortTxPkt
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   This function will get a packet to be sent from craft port net device.
 *
 * 	INPUT:
 *   None.
 *
 * 	OUTPUT:
 *   mref_handle_pp  --  The mref handle of the packet to be transmitted.
 *
 * 	RETURN:
 *   TRUE      --  Success
 *   FALSE     --  No packet to transmit
 *
 * 	NOTES:
 *   1. Pdu of the output mref_handle will point to the end of packet type
 *      of ethernet header. (i.e. offset 14 from the begining of the packet)
 *   2. All packets are untagged.
 *   3. The calling thread will be blocked until retrieving a packet to
 *      trasmit.
 *--------------------------------------------------------------------------
 */
static BOOL_T VLAN_NET_GetCraftPortTxPkt(L_MM_Mref_Handle_T **mref_handle_pp)
{
    UI32_T mref_handle_offset;
    BOOL_T ret;

    ret=SYSFUN_Syscall(SYSFUN_SYSCALL_VLAN_NET, VLAN_NET_SYSCALL_CMD_RECV_PACKET_FROM_CRAFT_PORT_NET_DEVICE, (UI32_T)(&mref_handle_offset), 0, 0, 0);
    if(ret==TRUE)
    {
        *mref_handle_pp = (L_MM_Mref_Handle_T*)L_IPCMEM_GetPtr(mref_handle_offset);
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : VLAN_NET_CraftPortTxTaskMain
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Task body of craft port tx task. This task will get the packets
 *   to be transmited to the craft port from the netdevice in kernel space,
 *   and call dev_nicdrv_gateway to send packet.
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
static void VLAN_NET_CraftPortTxTaskMain(void)
{
    L_MM_Mref_Handle_T *mref_handle_p;
    UI32_T             pdu_len;
    void*              packet_p;

    while(1)
    {
        if(VLAN_NET_GetCraftPortTxPkt(&mref_handle_p)==TRUE)
        {
            /* move mref pdu pointer to the beginning of the packet
             */
            packet_p=L_MM_Mref_MovePdu(mref_handle_p, -14, &pdu_len);
            if(packet_p)
            {
                DEV_NICDRV_GATEWAY_SendPacketToCraftPort(packet_p, pdu_len, mref_handle_p);
            }
            else
            {
                printf("%s: L_MM_Mref_MovePdu failed\r\n", __FUNCTION__);
            }
        }
    }
}
#endif

