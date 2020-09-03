/* MODULE NAME: vxlan_mgr.c
 * PURPOSE:
 *   Initialize the resource and provide some functions for the VXLAN module.
 * NOTES:
 *   None
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "backdoor_mgr.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include "sysfun.h"
#include "l_inet.h"
#include "sys_time.h"
#include "debug_mgr.h"
#include "vlan_om.h"
#include "amtrl3_pmgr.h"
#include "swdrv.h"
#include "swdrv_type.h"
#include "swdrvl3.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_route.h"

#include "amtrl3_pmgr.h"
#include "amtrl3_type.h"
#include "amtrl3_om.h"
#include "swctrl.h"
#include "trk_mgr.h"
#include "ip_lib.h"
#include "ipal_route.h"
#include "ipal_types.h"
#include "vlan_lib.h"
#include "vxlan_backdoor.h"
#include "vxlan_om.h"
#include "vxlan_type.h"
#include "vxlan_mgr.h"
#include "amtr_pmgr.h"
#include "swctrl_pom.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#if(SYS_CPNT_DEBUG == TRUE)

#define VXLAN_MGR_DEBUG_PRINTF(__flag__, format_string, ...)                                    \
{                                                                                               \
    if(     (__flag__)&VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG                                          \
       ||   (__flag__)&VXLAN_TYPE_DEBUG_FLAG_VNI_MSG                                            \
       ||   (__flag__)&VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG                                           \
      )                                                                                         \
    {                                                                                           \
        static int year, month, day, hour, minute, second;                                      \
        static char header_buffer[80];                                                          \
        SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);                \
        sprintf(header_buffer, "\r\n%d:%d:%d: VXLAN:", hour, minute, second);                  \
        DEBUG_MGR_Printf(DEBUG_TYPE_VXLAN,                                                      \
                         DEBUG_TYPT_MATCH_ALL,                                                  \
                         (__flag__),                                                            \
                         0,                                                                     \
                         "%s (%d) "format_string"", header_buffer, __LINE__, ## __VA_ARGS__);   \
    }                                                                                           \
    if (VXLAN_OM_GetDebug(__flag__) == TRUE)                                                    \
    {                                                                                           \
        BACKDOOR_MGR_Printf("\r\n%s(%d): "format_string"\r\n",                                  \
            __FUNCTION__, __LINE__, ##__VA_ARGS__);                                             \
    }                                                                                           \
}
#else
#define VXLAN_MGR_DEBUG_PRINTF(__flag__, format_string, ...)            \
{                                                                       \
    if (VXLAN_OM_GetDebug(__flag__) == TRUE)                            \
    {                                                                   \
        BACKDOOR_MGR_Printf("\r\n%s(%d): "format_string"\r\n",          \
            __FUNCTION__, __LINE__, ##__VA_ARGS__);                     \
    }                                                                   \
}
#endif

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void VXLAN_MGR_MultiIp2Mac(UI8_T *m_ip_p, UI8_T *mac_ar);
static UI32_T VXLAN_MGR_AddFloodRVtepEntry(VXLAN_OM_VNI_T *vni_entry_p, VXLAN_OM_RVtep_T *rvtep_entry_p);
static UI32_T VXLAN_MGR_AddFloodMulticastEntry(VXLAN_OM_VNI_T *vni_entry_p, VXLAN_OM_RVtep_T *rvtep_entry_p);

static BOOL_T VXLAN_MGR_CreateFloodUcVtepToChip(
    UI32_T              vfi_id,
    UI32_T              bcast_group,
    L_INET_AddrIp_T     *l_vtep_p,
    L_INET_AddrIp_T     *r_vtep_p);
static BOOL_T VXLAN_MGR_DestroyFloodUcVtepToChip(VXLAN_OM_RVtep_T *rvtep_entry);
static BOOL_T VXLAN_MGR_AddOneNhopForFloodUcVtepToChip(
    VXLAN_OM_VNI_T      *vni_entry_p,
    VXLAN_OM_RVtep_T    *rvtep_entry_p,
    L_INET_AddrIp_T     *nexthop_ip_p,
    UI32_T              nexthop_if);
static BOOL_T VXLAN_MGR_DelOneNhopForFloodUcVtepToChip(
    VXLAN_OM_VNI_T      *vni_entry_p,
    VXLAN_OM_RVtep_T    *rvtep_entry_p,
    L_INET_AddrIp_T     *nexthop_ip_p,
    UI32_T              nexthop_if);
static BOOL_T VXLAN_MGR_UpdateVtepNhop(
    VXLAN_OM_VNI_T      *vni_entry_p,
    VXLAN_OM_RVtep_T    *rvtep_entry_p,
    L_INET_AddrIp_T     *nexthops_ip_p,
    UI32_T              *nexthops_if_p,
    UI32_T              nexthop_cnt);

static UI32_T VXLAN_MGR_UpdateVtepSourceIp(L_INET_AddrIp_T *changed_ip_p);
static UI32_T VXLAN_MGR_DeleteVtepSourceIp(UI32_T changed_ifindex, L_INET_AddrIp_T *changed_ip_p);

static BOOL_T VXLAN_MGR_ActivateVtep(UI32_T vni);
static BOOL_T VXLAN_MGR_DeactivateVtep(UI32_T vni);

static BOOL_T VXLAN_MGR_AddAccessPort(UI32_T vni, UI32_T vid, UI32_T ifindex);
static BOOL_T VXLAN_MGR_DeleteAccessPort(UI32_T vni, UI32_T vid, UI32_T ifindex);

/* STATIC VARIABLE DEFINITIONS
 */
SYSFUN_DECLARE_CSC
static int task_id;
static int socket_id;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: VXLAN_MGR_InitiateSystemResources
 * PURPOSE : Initiate system resources for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_InitiateSystemResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    VXLAN_OM_InitiateSystemResources();
    return;
} /* End of VXLAN_MGR_InitiateSystemResources */

/* FUNCTION NAME: VXLAN_MGR_Create_InterCSC_Relation
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_Create_InterCSC_Relation(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("VXLAN",
        SYS_BLD_VXLAN_GROUP_IPCMSGQ_KEY, VXLAN_BACKDOOR_Main);
    return;
} /* End of VXLAN_MGR_Create_InterCSC_Relation */



/* FUNCTION NAME: VXLAN_MGR_SetTransitionMode
 * PURPOSE : Process SetTransitionMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    SYSFUN_SET_TRANSITION_MODE();

    return;
} /* End of VXLAN_MGR_SetTransitionMode */

/* FUNCTION NAME: VXLAN_MGR_EnterTransitionMode
 * PURPOSE : Process EnterTransitionMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    SYSFUN_ENTER_TRANSITION_MODE();
    VXLAN_OM_ClearAll();
    return;
} /* End of VXLAN_MGR_EnterTransitionMode */

/* FUNCTION NAME: VXLAN_MGR_EnterMasterMode
 * PURPOSE : Process EnterMasterMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    VXLAN_OM_SetAllDefault();
    SWDRV_SetVxlanStatus(TRUE, TRUE);
    SWDRV_SetVxlanUdpPort(VXLAN_TYPE_DFLT_UDP_DST_PORT);
    SYSFUN_ENTER_MASTER_MODE();

    return;
} /* End of VXLAN_MGR_EnterMasterMode */

/* FUNCTION NAME: VXLAN_MGR_EnterSlaveMode
 * PURPOSE : Process EnterSlaveMode for VXLAN MGR.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    SYSFUN_ENTER_SLAVE_MODE();

    return;
} /* End of VXLAN_MGR_EnterSlaveMode */


/* FUNCTION NAME: VXLAN_MGR_CreateTask
 * PURPOSE : Create and start task
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : never returns.
 * NOTES   : None.
 */
 void VXLAN_MGR_CreateTask(void)
{
    UI32_T thread_id;

    if(SYSFUN_SpawnThread(SYS_BLD_VXLAN_CSC_THREAD_PRIORITY,
                          SYS_BLD_VXLAN_CSC_SCHED_POLICY,
                          SYS_BLD_VXLAN_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          VXLAN_MGR_TaskMain,
                          NULL,
                          &thread_id)!=SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

    task_id = thread_id;
} /* End of VXLAN_MGR_CreateTask() */


/* FUNCTION NAME: VXLAN_MGR_ProcessReceivedPacket
 * PURPOSE : Process the received VTEP packet.
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           src_unit      -- unit
 *           src_port      -- src_port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_ProcessReceivedPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T *dst_mac,
                                  UI8_T *src_mac,
                                  UI16_T tag_info,
                                  UI16_T type,
                                  UI32_T pkt_length,
                                  UI32_T src_unit,
                                  UI32_T src_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */
    return;
} /* End of VXLAN_MGR_ProcessReceivedPacket */

/* FUNCTION NAME: VXLAN_MGR_RouteChange_CallBack
 * PURPOSE : when nsm has ipv4/ipv6 route change, it will call back to VXLAN.
 *           If there's a remote VTEP route change, shall update driver and OM.
 * INPUT   : address_family  -- ipv4 or ipv6
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RouteChange_CallBack(UI32_T address_family)
{
    VXLAN_OM_VNI_T                      vni_entry;
    VXLAN_OM_RVtep_T                    rvtep_entry;
    AMTRL3_TYPE_InetHostRouteEntry_T    host_entry;
    SWDRVL3_TunnelIntfL3_T              tl3_entry;
    NETCFG_TYPE_L3_Interface_T          vlan_intf;
    L_INET_AddrIp_T                     src_ip;
    L_INET_AddrIp_T                     tmp_ip;
    L_INET_AddrIp_T                     nexthops_ip_ar[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
    UI32_T                              nexthops_if_ar[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
    UI32_T                              nexthop_cnt = 1;
    UI32_T                              owner;
    UI32_T                              i=0;

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&src_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&tmp_ip, 0, sizeof(L_INET_AddrIp_T));

    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_DATABASE_MSG, "Get VTEP, vni[%lu], vfi[0x%lx], IP[%d.%d.%d.%d]\r\n",
            (unsigned long)rvtep_entry.vni, (unsigned long)rvtep_entry.vfi,
            rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);

        /* Get bcast_group by rvtep_entry.vni
         */
        memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
        vni_entry.vni = rvtep_entry.vni;
        if (VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Failed to get VNI [0x%lx] bcast_group\r\n", (unsigned long)vni_entry.vni);
            continue;
        }
        /* If we can find route to remote VTEP, update to driver and OM.
         * If not, delete VTEP
         */
        if (NETCFG_TYPE_OK == NETCFG_PMGR_ROUTE_FindBestRoute(&rvtep_entry.ip, &nexthops_ip_ar[0], &nexthops_if_ar[0], &owner))
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "VTEP route change , vfi[0x%lx], D_IP[%d.%d.%d.%d], S_IP[%d.%d.%d.%d], nexthop_cnt[%lu], owner[%lu]\r\n",
                (unsigned long)rvtep_entry.vfi,
                rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3],
                rvtep_entry.s_ip.addr[0],rvtep_entry.s_ip.addr[1], rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3],
                (unsigned long)nexthop_cnt, (unsigned long)owner);

            for (i=0; i<nexthop_cnt; i++)
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    " new N_IP[%d] = %d.%d.%d.%d\r\n",
                    i, nexthops_ip_ar[i].addr[0], nexthops_ip_ar[i].addr[1],
                    nexthops_ip_ar[i].addr[2], nexthops_ip_ar[i].addr[3]);

                /* use rvtep's ip as nexthop bcz it's directly connected
                 */
                if (IPAL_ROUTE_IS_BGP_UNNUMBERED_IPV4_LLA(nexthops_ip_ar[i].addr))
                    nexthops_ip_ar[i] = rvtep_entry.ip;
            }

            for (i=0; i<rvtep_entry.nexthop_cnt; i++)
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    " old N_IP[%d] = %d.%d.%d.%d\r\n",
                    i, rvtep_entry.nexthops_ip_ar[i].addr[0], rvtep_entry.nexthops_ip_ar[i].addr[1],
                    rvtep_entry.nexthops_ip_ar[i].addr[2], rvtep_entry.nexthops_ip_ar[i].addr[3]);
            }

            if (TRUE != VXLAN_MGR_UpdateVtepNhop(&vni_entry, &rvtep_entry, nexthops_ip_ar, nexthops_if_ar, nexthop_cnt))
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Failed to add VTEP to chip, vfi [0x%lx], D_IP[%d.%d.%d.%d]\r\n",
                    (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                continue;
            }
        }
        else
        {
            /* Not find route entry, shall remove this VTEP from chip and update OM entry.
             */
            memset(&nexthops_ip_ar, 0, sizeof(nexthops_ip_ar));
            memset(&nexthops_if_ar, 0, sizeof(nexthops_if_ar));
            nexthop_cnt = 0;
            if ( FALSE == VXLAN_MGR_UpdateVtepNhop(&vni_entry, &rvtep_entry, nexthops_ip_ar, nexthops_if_ar, nexthop_cnt))
            {
                /* Error happen */
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Del remote VTEP error!\r\nvfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                    (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                continue;
            }
        }
    }
}

/* FUNCTION NAME: VXLAN_MGR_VlanCreate_Callback
 * PURPOSE : Service the callback from VLAN_MGR when a vlan is created
 * INPUT   : vid_ifidx     -- specify which vlan has just been created
 *           vlan_status
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanCreate_Callback(UI32_T vid_ifindex, UI32_T vlan_status)
{
#if 0  //wind
    SWDRVL3_TunnelIntfL3_T tl3_entry;
    UI32_T vid = 0;
    UI32_T cur_vni = 0;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "VLAN create,  vid_ifindex[%lu]\r\n", (unsigned long)vid_ifindex);
    memset(&tl3_entry, 0, sizeof(SWDRVL3_TunnelIntfL3_T));
    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);

    /* Create L3 interface of the vlan for access port if it is not existed.
     */
    //if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
    {
        if(0 == VXLAN_OM_GetL3If(vid))
        {
            tl3_entry.vid = vid;
            SWCTRL_GetCpuMac(tl3_entry.src_mac);
            tl3_entry.is_add = TRUE;
            if (SWDRVL3_L3_NO_ERROR == SWDRVL3_AddTunnelIntfL3(&tl3_entry))
            {
                VXLAN_OM_SetL3If(vid, tl3_entry.l3_intf_id, TRUE);
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "SWDRVL3_AddTunnelIntfL3 error! vid[%lu]\r\n", (unsigned long)vid);
            }
        }
    }
#endif
    return;
}

/* FUNCTION NAME: VXLAN_MGR_VlanDestroy_Callback
 * PURPOSE : Service the callback from VLAN_MGR when a vlan is destroyed
 * INPUT   : vid_ifidx     -- specify which vlan has just been destroyed
 *           vlan_status
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanDestroy_Callback(UI32_T vid_ifindex, UI32_T vlan_status)
{
#if 0 //wind
    SWDRVL3_TunnelIntfL3_T tl3_entry;
    UI32_T vid = 0;
    UI32_T l3_if = 0;
    UI32_T cur_vni = 0;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "VLAN Destroy,  vid_ifindex[%lu]\r\n", (unsigned long)vid_ifindex);

    memset(&tl3_entry, 0, sizeof(SWDRVL3_TunnelIntfL3_T));
    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);

    /* Delete L3 interface of the vlan for access port if it is existed.
     */
    if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
    {
        l3_if = VXLAN_OM_GetL3If(vid);
        if(0 != l3_if)
        {
            /* Fill l3_intf_id for del
             */
            tl3_entry.is_add = FALSE;
            tl3_entry.l3_intf_id = l3_if;
            if (SWDRVL3_L3_NO_ERROR == SWDRVL3_AddTunnelIntfL3(&tl3_entry))
            {
                VXLAN_OM_SetL3If(vid, 0, FALSE);
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    "SWDRVL3_AddTunnelIntfL3 error! vid[%lu], l3_intf_id[%lu]\r\n", (unsigned long)vid, (unsigned long)l3_if);
            }
        }
    }
#endif
    return;
}

/* FUNCTION NAME: VXLAN_MGR_VlanMemberAdd_Callback
 * PURPOSE : Service the callback from VXLAN_MGR when a lport is added to a
 *           vlan's member set.
 * INPUT   : vid_ifidx     -- specify which vlan's member set to be add to
 *           lport_ifindex -- specify which port to be added to the member set
 *           vlan_status   --
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanMemberAdd_Callback(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
#if 0  //wind
    VXLAN_OM_VNI_T        vni_entry;
    VXLAN_OM_RVtep_T      rvtep_entry;
    L_INET_AddrIp_T       l_vtep_ip;
    L_INET_AddrIp_T       r_vtep_ip;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  vxlan_port = 0;
    UI32_T  l3_if = 0;
    UI32_T  cur_vni = 0;
    UI16_T  udp_port = 0;
    UI16_T  e_vlan = 0;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Add VLAN member, vid_ifindex[%lu], lport_ifindex[%lu]\r\n",
        (unsigned long)vid_ifindex, (unsigned long)lport_ifindex);

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&l_vtep_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&r_vtep_ip, 0, sizeof(L_INET_AddrIp_T));

    VXLAN_OM_GetUdpDstPort(&udp_port);
    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, e_vlan);

    l3_if = VXLAN_OM_GetL3If(e_vlan);
    if (VXLAN_OM_GetVlanVniMapEntry(e_vlan, &cur_vni) == VXLAN_TYPE_RETURN_OK)
    {
        vni_entry.vni = cur_vni;
        if (VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Fail to get VNI entry!\r\n");
            return;
        }

        type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id);
        if (SWCTRL_LPORT_TRUNK_PORT == type)
        {
            port = trunk_id;
        }
        /* Create UC VTEP for access port.
         */
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
            "Create UC VTEP\r\n vfi [0x%lx], e_vlan[%u], l3_if[%lu] port[%lu/%lu], udp_port[%u], bcast_group[0x%lx]\r\n",
            (unsigned long)vni_entry.vfi, e_vlan, (unsigned long)l3_if, (unsigned long)unit, (unsigned long)port, udp_port, (unsigned long)vni_entry.bcast_group);

        if (TRUE == SWDRV_CreateVxlanAccessPort(vni_entry.vfi, l3_if, unit, port, mac_ar, DEV_SWDRV_VXLAN_MATCH_PORT_VLAN, &vxlan_port))
        {
            SWDRV_AddVtepIntoMcastGroup(vni_entry.bcast_group, vxlan_port, TRUE);
            VXLAN_OM_SetAccessVxlanPort(e_vlan, lport_ifindex, vxlan_port, TRUE);
        }
        else
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "SWDRV_CreateVxlanAccessPort return FALSE!\r\n");
        }

        /* Set network port to driver layer after first access port is created.
         */
        if (VXLAN_OM_GetVlanMemberCounter(e_vlan) == 1)
        {
            VXLAN_MGR_ActivateVtep(cur_vni);
        }/* Create network port */
    }/* End of VXLAN_OM_GetVlanVniMapEntry ()*/
#endif
    return;
}/*End of VXLAN_MGR_VlanMemberAdd_Callback*/

/* FUNCTION NAME: VXLAN_MGR_VlanMemberDelete_Callback
 * PURPOSE : Service the callback from VXLAN_MGR when a port is remove from
 *           vlan's member set.
 * INPUT   : vid_ifidx     -- specify which vlan's member set to be deleted from
 *           lport_ifindex -- specify which port to be deleted
 *           vlan_status   --
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_VlanMemberDelete_Callback(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
#if 0  //wind
    VXLAN_OM_VNI_T      vni_entry;
    VXLAN_OM_RVtep_T    rvtep_entry;
    SWCTRL_Lport_Type_T type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  vxlan_port = 0, l_vxlan_port = 0;
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  cur_vni = 0;
    UI16_T  udp_port = 0;
    UI16_T  e_vlan = 0;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Delete VLAN member, vid_ifindex [%lu], lport_ifindex[%lu] \r\n",
        (unsigned long)vid_ifindex, (unsigned long)lport_ifindex);

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));

    VXLAN_OM_GetUdpDstPort(&udp_port);
    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, e_vlan);
    if (VXLAN_OM_GetVlanVniMapEntry(e_vlan, &cur_vni) == VXLAN_TYPE_RETURN_OK)
    {
        vni_entry.vni = cur_vni;
        if (VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Fail to get VNI entry!\r\n");
            return;
        }
        type = SWCTRL_LogicalPortToUserPort(lport_ifindex, &unit, &port, &trunk_id);
        if (SWCTRL_LPORT_TRUNK_PORT == type)
        {
            port = trunk_id;
        }
        vxlan_port = VXLAN_OM_GetAccessVxlanPort(e_vlan, lport_ifindex);

        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
            "Destroy UC VTEP\r\n vfi [0x%lx], e_vlan[%u], port[%lu/%lu], uc_vxlan_port[0x%lx] \r\n",
            (unsigned long)vni_entry.vfi, e_vlan, (unsigned long)unit, (unsigned long)port, (unsigned long)vxlan_port);

        if (vxlan_port != 0)
        {
            SWDRV_SetVxlanPortLearning(vxlan_port, FALSE);
#if 0
            AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(lport_ifindex,
                vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
            VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_port, l_vxlan_port);
            AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
            if (TRUE == SWDRV_DestroyVxlanPort(vni_entry.vfi, vxlan_port, FALSE))
            {
                /* remove access port
                 */
                VXLAN_OM_SetAccessVxlanPort(e_vlan, lport_ifindex, 0, FALSE);
            }
            else
            {
                SWDRV_SetVxlanPortLearning(vxlan_port, TRUE);
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "SWDRV_DestroyVxlanPort return FALSE!\r\n");
            }
        }

        /* If no access port in the VNI, to remove all network ports from chip.
         */
        if (VXLAN_OM_GetVlanMemberCounter(e_vlan) == 0)
        {
            VXLAN_MGR_DeactivateVtep(cur_vni);
        }
    }
    return;
#endif
}/*End of VXLAN_MGR_VlanMemberDelete_Callback*/

/* FUNCTION NAME: VXLAN_MGR_RifActive_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif active.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifActive_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
#if 0
    VXLAN_OM_VNI_T   vni_entry;
    VXLAN_OM_RVtep_T rvtep_entry;
    UI16_T vid=0;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Rif active, ifindex [%lu], addr[%d.%d.%d.%d] \r\n",
         (unsigned long)ifindex, addr_p->addr[0], addr_p->addr[1], addr_p->addr[2], addr_p->addr[3]);

    /* First create L3 interface when network port is added.
     * VXLAN_MGR_AddFloodRVtepEntry() and VXLAN_MGR_AddFloodMulticastEntry().
     */

    if(SYS_ADPT_CRAFT_INTERFACE_IFINDEX == ifindex)
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Craft prot ifindex, needn't to process it\r\n");
        return;
    }

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);
    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        /* Src IP is active.
         */
        if (rvtep_entry.vid == vid)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Update U VTEP \r\n");

            /* Check Route.
            */
            vni_entry.vni = rvtep_entry.vni;
            VXLAN_OM_GetVniEntry(&vni_entry);
            VXLAN_MGR_AddFloodRVtepEntry(&vni_entry, &rvtep_entry);
        }
    }

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    while (VXLAN_OM_GetNextFloodMulticast(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (    (rvtep_entry.vid == vid)
            &&  (0 == rvtep_entry.mc_vxlan_port)
           )
        {
            /* Src IP is active.
             */
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Update M VTEP \r\n");

            /* Check Route.
             */
            vni_entry.vni = rvtep_entry.vni;
            VXLAN_OM_GetVniEntry(&vni_entry);
            VXLAN_MGR_AddFloodMulticastEntry(&vni_entry, &rvtep_entry);
        }
    }
#endif
    return;
}/*End of VXLAN_MGR_RifActive_Callback*/

/* FUNCTION NAME: VXLAN_MGR_RifDown_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif down.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifDown_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
#if 0
    VXLAN_OM_VNI_T          vni_entry;
    VXLAN_OM_RVtep_T        rvtep_entry;
    SWDRVL3_TunnelIntfL3_T  tl3_entry;
    SWCTRL_Lport_Type_T     type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  l3_if = 0;
    UI32_T  cur_vni = 0;
    UI32_T  l_vxlan_port = 0;
    UI16_T  vid;


    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, " Rif down, ifindex [%lu], addr[%d.%d.%d.%d] \r\n",
        (unsigned long)ifindex, addr_p->addr[0], addr_p->addr[1], addr_p->addr[2], addr_p->addr[3]);

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&tl3_entry, 0, sizeof(SWDRVL3_TunnelIntfL3_T));
    VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);

    /* For flood unicast
     */
    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (    (rvtep_entry.vid == vid)
            &&  (memcmp(&rvtep_entry.s_ip, addr_p, sizeof(L_INET_AddrIp_T)) == 0)
           )
        {
            /* Delete orginal UC and MC VTEP in chip for network port.
             */
            if (FALSE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
            {
                /* Error happen */
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    "VXLAN_MGR_DestroyFloodUcVtepToChip error!\r\n vfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                    (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                continue;
            }

            /* Update OM.
             */
            memset(&(rvtep_entry.s_ip), 0, sizeof(L_INET_AddrIp_T));
            memset(&(rvtep_entry.nexthop_ip), 0, sizeof(L_INET_AddrIp_T));
            rvtep_entry.vid = 0;
            rvtep_entry.lport = 0;
            rvtep_entry.uc_vxlan_port = 0;
            rvtep_entry.mc_vxlan_port = 0;
            memset(rvtep_entry.mac_ar, 0, SYS_ADPT_MAC_ADDR_LEN);
            VXLAN_OM_AddFloodRVtep(&rvtep_entry);
        }
    }

    /* For flood multicast
     */
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    while (VXLAN_OM_GetNextFloodMulticast(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (    (rvtep_entry.vid == vid)
            &&  (memcmp(&rvtep_entry.s_ip, addr_p, sizeof(L_INET_AddrIp_T)) == 0)
           )
        {
            /* Delete orginal VTEP in chip
             */
            if (0 != rvtep_entry.mc_vxlan_port)
            {
                type = SWCTRL_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
                if (SWCTRL_LPORT_TRUNK_PORT == type)
                {
                    port = trunk_id;
                }

                SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, FALSE);
#if 0
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(rvtep_entry.lport,
                    vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(rvtep_entry.mc_vxlan_port, l_vxlan_port);
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                    vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                if (FALSE == SWDRV_DestroyVTEP(rvtep_entry.vfi, unit, port,
                             FALSE, rvtep_entry.mc_vxlan_port))
                {
                    /* Error happen */
                    SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, TRUE);
                    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Destroy VTEP error!\r\nvfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                        (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                    continue;
                }
            }

            /* Update OM.
             */
            memset(&rvtep_entry.s_ip, 0, sizeof(L_INET_AddrIp_T));
            rvtep_entry.uc_vxlan_port = 0;
            rvtep_entry.mc_vxlan_port = 0;
            VXLAN_OM_AddFloodMulticast(&rvtep_entry);
        }
    }

    /* If the VLAN had been associated a VNI, shall not remove L3 interface.
     * To handle L3 interface on access ports when VLAN create/destroy callback and VLAN-VNI mapping change.
     */
    if (    (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        &&  (0 != cur_vni)
       )
    {
        return;
    }

    /* After delete all newtork port VTEPs,
     * and then delete L3 interface of the vlan if it is existed.
     */
    l3_if = VXLAN_OM_GetL3If(vid);

    if (0 != l3_if)
    {
        tl3_entry.l3_intf_id = l3_if;
        tl3_entry.is_add = FALSE;
        if (SWDRVL3_L3_NO_ERROR == SWDRVL3_AddTunnelIntfL3(&tl3_entry))
        {
            VXLAN_OM_SetL3If(vid, 0, FALSE);
        }
        else
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "SWDRVL3_AddTunnelIntfL3 error! l3_intf_id[%lu]\r\n", (unsigned long)l3_if);
        }
    }
#endif
    return;
}/*End of VXLAN_MGR_RifDown_Callback*/


static BOOL_T VXLAN_MGR_AddAccessPort(UI32_T vni, UI32_T vid, UI32_T ifindex)
{
    VXLAN_OM_VNI_T vni_entry;
    UI32_T vxlan_port;
    SWCTRL_Lport_Type_T type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T unit, port, trunk_id, l3_if;
    UI32_T match_flag;
    UI32_T pvid, pvid_ifindex;
    UI16_T udp_port;
    UI8_T  mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI32_T ori_vid = vid;

    vni_entry.vni = vni;
    if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetVniEntry(&vni_entry))
        return FALSE;

    vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, ifindex);
    if (vxlan_port != 0) /* it is already an exist access port */
        return FALSE;

    /* The corresponding l3 interface must exist first
     */
    l3_if = VXLAN_OM_GetL3If(vid);
#if 0  //not check vlan
    if (    (TRUE == VLAN_OM_IsVlanExisted(vid))
        &&  (0 == l3_if)
       )
#endif
    if (0 == l3_if)
    {
        SWDRVL3_TunnelIntfL3_T  tl3_entry;

        memset(&tl3_entry, 0, sizeof(SWDRVL3_TunnelIntfL3_T));
        tl3_entry.vid = vid;
        SWCTRL_GetCpuMac(tl3_entry.src_mac);
        tl3_entry.is_add = TRUE;
        if (SWDRVL3_L3_NO_ERROR == SWDRVL3_AddTunnelIntfL3(&tl3_entry))
        {
            l3_if = tl3_entry.l3_intf_id;
            VXLAN_OM_SetL3If(vid, l3_if, TRUE);
        }
        else
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Not create VXLAN interface for VLAN %u\r\n", vid);
            return FALSE;
        }
    }

    type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
    if (SWCTRL_LPORT_TRUNK_PORT == type)
    {
        port = trunk_id;
    }

    if (vid == 0) /* per-port access port, used pvid as egress L3 interface */
    {
        match_flag = DEV_SWDRV_VXLAN_MATCH_PORT;
        VLAN_OM_GetVlanPortPvid(ifindex, &pvid_ifindex);
        VLAN_IFINDEX_CONVERTTO_VID(pvid_ifindex, vid);
    }
    else
    {
        match_flag = DEV_SWDRV_VXLAN_MATCH_PORT_VLAN;
    }

    l3_if = VXLAN_OM_GetL3If(vid);
    VXLAN_OM_GetUdpDstPort(&udp_port);

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
        "Create UC VTEP on trunk port\r\n vfi [0x%lx], e_vlan[%u], l3_if[%lu] port[%lu/%lu], udp_port[%u], bcast_group[0x%lx]\r\n",
        (unsigned long)vni_entry.vfi, vid, (unsigned long)l3_if, (unsigned long)unit, (unsigned long)port, udp_port, (unsigned long)vni_entry.bcast_group);

    if (!SWDRV_CreateVxlanAccessPort(vni_entry.vfi, l3_if, unit, port, mac_ar, match_flag, &vxlan_port))
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "SWDRV_CreateVxlanAccessPort return FALSE!\r\n");
    }

    SWDRV_AddVtepIntoMcastGroup(vni_entry.bcast_group, vxlan_port, TRUE);
    VXLAN_OM_SetAccessVxlanPort(ori_vid, ifindex, vxlan_port, TRUE);
    VXLAN_OM_SetL3IfUseCount(ori_vid, TRUE);
    vni_entry.nbr_of_acc_port++;
    VXLAN_OM_SetVniEntry(&vni_entry);

    return TRUE;
}

static BOOL_T VXLAN_MGR_DeleteAccessPort(UI32_T vni, UI32_T vid, UI32_T ifindex)
{
    VXLAN_OM_VNI_T vni_entry;
    UI32_T vxlan_port;
    SWCTRL_Lport_Type_T type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T unit, port, trunk_id, l3_if;
    UI32_T l_vxlan_port;
    UI16_T use_count = 0;

    vni_entry.vni = vni;
    if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetVniEntry(&vni_entry))
        return FALSE;

    vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, ifindex);
    if (vxlan_port == 0) /* can't find corresponding Vxlan access port */
        return FALSE;

    type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
    if (SWCTRL_LPORT_TRUNK_PORT == type)
    {
        port = trunk_id;
    }

    SWDRV_SetVxlanPortLearning(vxlan_port, FALSE);

    VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_port, l_vxlan_port);
    AMTR_PMGR_DeleteAddrByVidAndLPort(l_vxlan_port, vni_entry.vfi);

    if (!SWDRV_DestroyVxlanPort(vni_entry.vfi, vxlan_port, FALSE))
    {
        SWDRV_SetVxlanPortLearning(vxlan_port, TRUE);
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "SWDRV_DestroyVxlanPort return FALSE!\r\n");
        return FALSE;
    }

    VXLAN_OM_SetAccessVxlanPort(vid, ifindex, 0, FALSE);
    VXLAN_OM_SetL3IfUseCount(vid, FALSE);
    use_count = VXLAN_OM_GetL3IfUseCount(vid);

    if (0 == use_count)
    {
        l3_if = VXLAN_OM_GetL3If(vid);

        if(0 != l3_if)
        {
            SWDRVL3_TunnelIntfL3_T tl3_entry;

            memset(&tl3_entry, 0, sizeof(tl3_entry));
            tl3_entry.is_add = FALSE;
            tl3_entry.l3_intf_id = l3_if;

            if (SWDRVL3_L3_NO_ERROR == SWDRVL3_AddTunnelIntfL3(&tl3_entry))
            {
                VXLAN_OM_SetL3If(vid, 0, FALSE);
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    "SWDRVL3_AddTunnelIntfL3 error! vid[%lu], l3_intf_id[%lu]\r\n", (unsigned long)vid, (unsigned long)l3_if);
            }
        }
    }

    VXLAN_OM_DelPortVlanVniMap(ifindex, vid, vni);

    if (vni_entry.nbr_of_acc_port > 0)
        vni_entry.nbr_of_acc_port--;
    VXLAN_OM_SetVniEntry(&vni_entry);

    return TRUE;
}

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberAdd1st_CallBack
 * PURPOSE : Handle the callback event happening when the first port is added
 *           to a trunk.
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : Remove VTEPs if trunk member is VTEP's egress port.
 */
void VXLAN_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VXLAN_OM_VNI_T      vni_entry;
    VXLAN_OM_RVtep_T    rvtep_entry;
    L_INET_AddrIp_T     l_vtep_ip;
    L_INET_AddrIp_T     r_vtep_ip;
    SWCTRL_Lport_Type_T type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  vid = 0, vxlan_port = 0, cur_vni = 0, l_vxlan_port = 0;
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  l3_if = 0;
    UI16_T  udp_port = 0;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Add frist trunk member[%lu] to trunk_ifindex[%lu] \r\n",
        (unsigned long)member_ifindex, (unsigned long)trunk_ifindex);

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&l_vtep_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&r_vtep_ip, 0, sizeof(L_INET_AddrIp_T));

#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    /* Do nothing. AMTRL3 will handle the event for flood unicast.
     */
#else
    /* Network port: for flood unicast
     */
    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (rvtep_entry.lport == member_ifindex)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                "Have VTEP on trunk member, vni[%lu],  D_IP[%d.%d.%d.%d]\r\n",
                (unsigned long)rvtep_entry.vni, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            /* Delete orginal UC and MC VTEP in chip for network port.
             */
            if (FALSE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
            {
                /* Error happen */
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    "VXLAN_MGR_DestroyFloodUcVtepToChip error!\r\n vfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                    (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                continue;
            }

            /* Update orginal recored in OM
             */
            memset(&(rvtep_entry.nexthop_ip), 0, sizeof(L_INET_AddrIp_T));
            rvtep_entry.vid = 0;
            rvtep_entry.lport = 0;
            rvtep_entry.uc_vxlan_port = 0;
            rvtep_entry.mc_vxlan_port = 0;
            memset(rvtep_entry.mac_ar, 0, SYS_ADPT_MAC_ADDR_LEN);
            VXLAN_OM_AddFloodRVtep(&rvtep_entry);
        }
    }
#endif

    /* Network port: for flood multicast.
     * Do nothing because we don't support trunk port.
     */

    /* Access port: remove trunk member VTEP and add trunk port VTEP
     */
    vid = 0;
    while (VLAN_OM_GetNextVlanId(0, &vid))
    {
        if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, member_ifindex);
            if (vxlan_port != 0)
            {
                VXLAN_MGR_DeleteAccessPort(cur_vni, vid, member_ifindex);
                VXLAN_MGR_AddAccessPort(cur_vni, vid, trunk_ifindex);
            }
        }
    }

    return;
}

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberAdd_CallBack
 * PURPOSE : Service the callback from SWCTRL when the port joins the trunk
 *           as the 2nd or the following member
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : Remove VTEPs if trunk member is VTEP's egress port.
 */
void VXLAN_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VXLAN_OM_VNI_T          vni_entry;
    VXLAN_OM_RVtep_T        rvtep_entry;
    UI32_T  vid = 0, vxlan_port = 0, cur_vni = 0, l_vxlan_port = 0;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Add trunk member[%lu] to trunk_ifindex[%lu] \r\n",
        (unsigned long)member_ifindex, (unsigned long)trunk_ifindex);
    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));

#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    /* Do nothing. AMTRL3 will handle the event for flood unicast.
     */
#else
    /* Network port: for flood unicast
     */
    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (rvtep_entry.lport == member_ifindex)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                "Have VTEP on trunk member, vni[%lu],  D_IP[%d.%d.%d.%d]\r\n",
                (unsigned long)rvtep_entry.vni, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            /* Delete orginal UC and MC VTEP in chip for network port.
             */
            if (FALSE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
            {
                /* Error happen */
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    "VXLAN_MGR_DestroyFloodUcVtepToChip error!\r\n vfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                    (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                continue;
            }

            /* Update orginal recored in OM
             */
            memset(&(rvtep_entry.nexthop_ip), 0, sizeof(L_INET_AddrIp_T));
            rvtep_entry.vid = 0;
            rvtep_entry.lport = 0;
            rvtep_entry.uc_vxlan_port = 0;
            rvtep_entry.mc_vxlan_port = 0;
            memset(rvtep_entry.mac_ar, 0, SYS_ADPT_MAC_ADDR_LEN);
            VXLAN_OM_AddFloodRVtep(&rvtep_entry);
        }
    }
#endif
    /* Network port: for flood multicast.
     * Do nothing because we don't support trunk port.
     */

    /* Access port: remove trunk member VTEP
     */
    vid = 0;
    while (VLAN_OM_GetNextVlanId(0, &vid))
    {
        if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, member_ifindex);
            if (vxlan_port != 0)
            {
                VXLAN_MGR_DeleteAccessPort(cur_vni, vid, member_ifindex);
            }
        }
    }

    return;
}

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberDelete_CallBack
 * PURPOSE : Handle the callback event happening when a logical port is deleted
 *           from a trunk.
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    UI32_T  vid, vxlan_port, cur_vni;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, " Remove trunk member[%lu] from trunk_ifindex[%lu] \r\n",
        (unsigned long)member_ifindex, (unsigned long)trunk_ifindex);

    /* Network port: Do nothing.
     */

    /* Access port: add trunk member VTEP
     */
    vid = 0;
    while (VLAN_OM_GetNextVlanId(0, &vid))
    {
        if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, trunk_ifindex);
            if (vxlan_port != 0)
            {
                /* Create UC VTEP for trunk member.
                 */
                VXLAN_MGR_AddAccessPort(cur_vni, vid, member_ifindex);
            }
        }
    }

    return;
}

/* FUNCTION NAME: VXLAN_MGR_TrunkMemberDeleteLst_CallBack
 * PURPOSE : Handle the callback event happening when the last port is deleted
 *           from a trunk.
 * INPUT   : UI32_T    trunk_ifindex
 *           UI32_T    member_ifindex
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : Remove VTEPs if trunk port is VTEP's egress port.
 */
void VXLAN_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
    VXLAN_OM_VNI_T      vni_entry;
    VXLAN_OM_RVtep_T    rvtep_entry;
    L_INET_AddrIp_T     l_vtep_ip;
    L_INET_AddrIp_T     r_vtep_ip;
    SWCTRL_Lport_Type_T type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  vid = 0, vxlan_port = 0, cur_vni = 0, l_vxlan_port = 0;
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  l3_if = 0;
    UI16_T  udp_port = 0;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, " Remove last trunk member[%lu] from trunk_ifindex[%lu] \r\n",
        (unsigned long)member_ifindex, (unsigned long)trunk_ifindex);

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&l_vtep_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&r_vtep_ip, 0, sizeof(L_INET_AddrIp_T));

#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    /* Do nothing. AMTRL3 will handle the event for flood unicast.
     */
#else
    /* Network port: for flood unicast
     */
    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (rvtep_entry.lport == trunk_ifindex)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                "Have VTEP on trunk port, vni[%lu],  D_IP[%d.%d.%d.%d]\r\n",
                (unsigned long)rvtep_entry.vni, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            /* Delete orginal UC and MC VTEP in chip for network port.
             */
            if (FALSE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
            {
                /* Error happen */
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                    "VXLAN_MGR_DestroyFloodUcVtepToChip error!\r\n vfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                    (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                continue;
            }

            /* Update orginal recored in OM
             */
            memset(&(rvtep_entry.nexthop_ip), 0, sizeof(L_INET_AddrIp_T));
            rvtep_entry.vid = 0;
            rvtep_entry.lport = 0;
            rvtep_entry.uc_vxlan_port = 0;
            rvtep_entry.mc_vxlan_port = 0;
            memset(rvtep_entry.mac_ar, 0, SYS_ADPT_MAC_ADDR_LEN);
            VXLAN_OM_AddFloodRVtep(&rvtep_entry);
        }
    }
#endif

    /* Network port: for flood multicast.
     * Do nothing because we don't support trunk port.
     */

    /* Access port: remove trunk port VTEP and add trunk member VTEP
     */
    vid = 0;
    while (VLAN_OM_GetNextVlanId(0, &vid))
    {
        if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, trunk_ifindex);
            if (vxlan_port != 0)
            {
                VXLAN_MGR_DeleteAccessPort(cur_vni, vid, trunk_ifindex);

                /* Create UC VTEP for removed trunk member.
                 */
                VXLAN_MGR_AddAccessPort(cur_vni, vid, member_ifindex);
            }
        }
    }

    return;
}

/* FUNCTION NAME: VXLAN_MGR_RifCreated_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif created.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifCreated_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI32_T cur_ifinex;
    VXLAN_OM_RVtep_T rvtep_entry;
    VXLAN_OM_VNI_T   vni_entry;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Rif created , ifindex[%lu], IP[%d.%d.%d.%d]\r\n",
        (unsigned long)ifindex,
        addr_p->addr[0], addr_p->addr[1], addr_p->addr[2], addr_p->addr[3]);

    VXLAN_OM_GetSrcIf(&cur_ifinex);
    if (0 != cur_ifinex)
    {
        if (cur_ifinex == ifindex)
        {
            memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
            rif_config.ifindex = ifindex;
            memcpy(&(rif_config.addr), addr_p, sizeof(L_INET_AddrIp_T));
            if (    (NETCFG_TYPE_OK == NETCFG_POM_IP_GetRifFromInterface(&rif_config))
                &&  (NETCFG_TYPE_MODE_PRIMARY == rif_config.ipv4_role)
               )
            {
                VXLAN_MGR_UpdateVtepSourceIp(addr_p);
            }
        }
    }
}

/* FUNCTION NAME: VXLAN_MGR_RifDestroyed_Callback
 * PURPOSE : Service the callback from VLAN_MGR when when Rif destroyed.
 * INPUT   : ifindex  -- the ifindex  of active rif
 *           addr_p  --  the addr_p of active rif
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_RifDestroyed_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p)
{
    UI32_T cur_ifindex;
    UI32_T unit = 0, port = 0, trunk_id = 0;
    SWCTRL_Lport_Type_T     type = SWCTRL_LPORT_UNKNOWN_PORT;
    VXLAN_OM_RVtep_T rvtep_entry;
    VXLAN_OM_VNI_T   vni_entry;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Rif destroyed , ifindex[%lu], IP[%d.%d.%d.%d]\r\n",
        (unsigned long)ifindex,
        addr_p->addr[0], addr_p->addr[1], addr_p->addr[2], addr_p->addr[3]);
    VXLAN_OM_GetSrcIf(&cur_ifindex);
    if (0 != cur_ifindex)
    {
        if (cur_ifindex == ifindex)
        {
            VXLAN_MGR_DeleteVtepSourceIp(ifindex, addr_p);
        }
    }
}

/* FUNCTION NAME: VXLAN_MGR_PortLinkDown_CallBack
 * PURPOSE : Handle the callback event happening when the link is down.
 * INPUT   : ifindex -- which logical port
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_PortLinkDown_CallBack(UI32_T ifindex)
{
    VXLAN_OM_VNI_T  vni_entry;
    UI32_T  vid = 0, vxlan_port = 0, cur_vni = 0, l_vxlan_port = 0;

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    while (VLAN_OM_GetNextVlanId(0, &vid))
    {
        if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            vni_entry.vni = cur_vni;
            VXLAN_OM_GetVniEntry(&vni_entry);
            vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, ifindex);

            if (vxlan_port != 0)
            {
                SWDRV_SetVxlanPortLearning(vxlan_port, FALSE);

                /* We kept real VXLAN ifindex in chip.
                 * When we call AMTR to remove MAC, shall convert to logical vxlan ifindex.
                 */
                VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_port, l_vxlan_port);
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                    vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                SWDRV_SetVxlanPortLearning(vxlan_port, TRUE);
            }
        }
    }

    return;
}

/* FUNCTION NAME: VXLAN_MGR_PvidChange_CallBack
 * PURPOSE : Handle the callback event happening when the pvid of port change
 * INPUT   : lport_ifindex -- which logical port
 *           old_pvid      -- old pvid
 *           new_pvid      -- new pvid
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
void VXLAN_MGR_PvidChange_CallBack(UI32_T lport_ifindex, UI32_T old_pvid, UI32_T new_pvid)
{
    UI32_T vni = 0;

    if (0 == VXLAN_OM_GetAccessVxlanPort(0, lport_ifindex))
        return;

    if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetPortVlanVniMapEntry(lport_ifindex, 0, &vni))
        return;

    VXLAN_MGR_DeleteAccessPort(vni, 0, lport_ifindex);
    VXLAN_MGR_AddAccessPort(vni, 0, lport_ifindex);

    return;
}

/* FUNCTION NAME: VXLAN_MGR_SetUdpDstPort
 * PURPOSE : Set UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_SetUdpDstPort(UI16_T port_no)
{
    VXLAN_OM_RVtep_T rvtep_entry;
    UI32_T ret = VXLAN_TYPE_RETURN_ERROR;

    if (   (port_no < VXLAN_TYPE_UDP_DST_PORT_MIN)
        || (port_no > VXLAN_TYPE_UDP_DST_PORT_MAX)
       )
    {
        return ret;
    }

    /* If network vtep is existed, shall not change UDP port.
     */
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextFloodRVtep(&rvtep_entry))
    {
        return VXLAN_TYPE_ENTRY_EXISTED;
    }

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextFloodMulticast(&rvtep_entry))
    {
        return VXLAN_TYPE_ENTRY_EXISTED;
    }

    if (SWDRV_SetVxlanUdpPort(port_no) == TRUE)
    {
        ret = VXLAN_OM_SetUdpDstPort(port_no);
    }
    return ret;
}

/* FUNCTION NAME: VXLAN_MGR_AddFloodRVtep
 * PURPOSE : Flooding to which remote VTEP, when received packet lookup bridge table fail.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : vni=0, set all created VPNs.
 */
UI32_T VXLAN_MGR_AddFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p)
{
    VXLAN_OM_VNI_T      vni_entry;
    VXLAN_OM_RVtep_T    rvtep_entry;
    VXLAN_OM_RVtep_T    mc_rvtep_entry;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  unit = 0, port = 0, trunk_id = 0, l_vxlan_port = 0;

    if (vni > VXLAN_TYPE_VNI_ID_MAX)
    {
        return ret;
    }

    if (    (TRUE == IP_LIB_IsLoopBackIp(ip_p->addr))
        ||  (TRUE == IP_LIB_IsZeroNetwork(ip_p->addr))
        ||  (TRUE == IP_LIB_IsBroadcastIp(ip_p->addr))
        ||  (TRUE == IP_LIB_IsIpInClassD(ip_p->addr))
        ||  (TRUE == IP_LIB_IsIpInClassE(ip_p->addr))
       )
    {
        return VXLAN_TYPE_IP_INVALID;
    }

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&mc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));

    if (vni != 0)
    {
        vni_entry.vni = vni;
        if(VXLAN_OM_GetVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
        {
            rvtep_entry.vni = vni_entry.vni;
            memcpy(&rvtep_entry.ip, ip_p, sizeof(L_INET_AddrIp_T));
            ret = VXLAN_MGR_AddFloodRVtepEntry(&vni_entry, &rvtep_entry);
            if (    (ret == VXLAN_TYPE_RETURN_OK)
                ||  (ret == VXLAN_TYPE_ROUTE_NOT_FIND)
                ||  (ret == VXLAN_TYPE_SRC_IF_NOT_FIND)
                ||  (ret == VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
               )
            {
                /* If have multicast ip in the VNI, to delete its network tunnel port in chip.
                 */
                memset(&mc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
                mc_rvtep_entry.vni = vni_entry.vni;
                if (VXLAN_OM_GetFloodMulticastByVni(&mc_rvtep_entry) == VXLAN_TYPE_RETURN_OK)
                {
                    if (0 != mc_rvtep_entry.mc_vxlan_port)
                    {
                        type = SWCTRL_LogicalPortToUserPort(mc_rvtep_entry.lport, &unit, &port, &trunk_id);
                        if (SWCTRL_LPORT_TRUNK_PORT == type)
                        {
                            port = trunk_id;
                        }

                        SWDRV_SetVxlanPortLearning(mc_rvtep_entry.mc_vxlan_port, FALSE);
#if 0
                        AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(mc_rvtep_entry.lport,
                            vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                        VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(mc_rvtep_entry.mc_vxlan_port, l_vxlan_port);
                        AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                            vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                        if (FALSE == SWDRV_DestroyVxlanPort(rvtep_entry.vfi, mc_rvtep_entry.mc_vxlan_port, FALSE))
                        {
                            SWDRV_SetVxlanPortLearning(mc_rvtep_entry.mc_vxlan_port, TRUE);
                            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Destroy VTEP error!\r\n");
                            return VXLAN_TYPE_DRIVER_ERROR;
                        }
                        else
                        {
                            mc_rvtep_entry.uc_vxlan_port = 0;
                            mc_rvtep_entry.mc_vxlan_port = 0;
                            ret = VXLAN_OM_AddFloodMulticast(&mc_rvtep_entry);
                        }
                    }
                }
            }
        }
    }
    else
    {
        ret = VXLAN_TYPE_RETURN_OK;
        while(VXLAN_OM_GetNextVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
        {
            rvtep_entry.vni = vni_entry.vni;
            memcpy(&rvtep_entry.ip, ip_p, sizeof(L_INET_AddrIp_T));
            ret = VXLAN_MGR_AddFloodRVtepEntry(&vni_entry, &rvtep_entry);
            if (    (ret != VXLAN_TYPE_RETURN_OK)
                &&  (ret != VXLAN_TYPE_ROUTE_NOT_FIND)
                &&  (ret != VXLAN_TYPE_SRC_IF_NOT_FIND)
                &&  (ret != VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
               )
            {
                break;
            }
            else
            {
                /* Delete multicast in chip if have multicast ip in the VNI.
                 */
                memset(&mc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
                mc_rvtep_entry.vni = vni_entry.vni;
                if (VXLAN_OM_GetFloodMulticastByVni(&mc_rvtep_entry) == VXLAN_TYPE_RETURN_OK)
                {
                    if (0 != mc_rvtep_entry.mc_vxlan_port)
                    {
                        type = SWCTRL_LogicalPortToUserPort(mc_rvtep_entry.lport, &unit, &port, &trunk_id);
                        if (SWCTRL_LPORT_TRUNK_PORT == type)
                        {
                            port = trunk_id;
                        }

                        SWDRV_SetVxlanPortLearning(mc_rvtep_entry.mc_vxlan_port, FALSE);
#if 0
                        AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(mc_rvtep_entry.lport,
                            vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                        VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(mc_rvtep_entry.mc_vxlan_port, l_vxlan_port);
                        AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                            vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
                        if (FALSE == SWDRV_DestroyVxlanPort(mc_rvtep_entry.vfi, mc_rvtep_entry.mc_vxlan_port, FALSE))
                        {
                            SWDRV_SetVxlanPortLearning(mc_rvtep_entry.mc_vxlan_port, TRUE);
                            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Destroy VTEP error!\r\n");
                            return VXLAN_TYPE_DRIVER_ERROR;
                        }
                        else
                        {
                            mc_rvtep_entry.uc_vxlan_port = 0;
                            mc_rvtep_entry.mc_vxlan_port = 0;
                            ret = VXLAN_OM_AddFloodMulticast(&mc_rvtep_entry);
                        }
                    }
                }
            }
        }
    }

    return ret;

}

/* FUNCTION NAME: VXLAN_MGR_DelFloodRVtep
 * PURPOSE : Delete remote VTEP with specified IP.
 * INPUT   : vni -- VXLAN ID
 *           ip_p -- IP address of remote VTEP
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : vni=0, set all created VPNs..
 */
UI32_T VXLAN_MGR_DelFloodRVtep(UI32_T vni, L_INET_AddrIp_T *ip_p)
{
    VXLAN_OM_VNI_T   vni_entry;
    VXLAN_OM_RVtep_T rvtep_entry;
    VXLAN_OM_RVtep_T uc_rvtep_entry;
    VXLAN_OM_RVtep_T mc_rvtep_entry;
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id;

    if (vni > VXLAN_TYPE_VNI_ID_MAX)
    {
        return ret;
    }

    if (    (TRUE == IP_LIB_IsLoopBackIp(ip_p->addr))
        ||  (TRUE == IP_LIB_IsZeroNetwork(ip_p->addr))
        ||  (TRUE == IP_LIB_IsBroadcastIp(ip_p->addr))
        ||  (TRUE == IP_LIB_IsIpInClassD(ip_p->addr))
        ||  (TRUE == IP_LIB_IsIpInClassE(ip_p->addr))
       )
    {
        return VXLAN_TYPE_IP_INVALID;
    }

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&uc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&mc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));

    if (vni != 0)
    {
        rvtep_entry.vni = vni;
        memcpy(&rvtep_entry.ip, ip_p, sizeof(L_INET_AddrIp_T));
        if (VXLAN_OM_GetFloodRVtepByVniIp(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
        {
            UI16_T l_vxlan_port;
            AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;

            memset(&vxlan_tunnel, 0, sizeof(AMTRL3_OM_VxlanTunnelEntry_T));

            vni_entry.vni = vni;
            if (VXLAN_OM_GetVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
            {
                vxlan_tunnel.vfi_id = vni_entry.vfi;
                vxlan_tunnel.local_vtep = rvtep_entry.s_ip;
                vxlan_tunnel.remote_vtep = rvtep_entry.ip;

                if (TRUE == AMTRL3_OM_GetVxlanTunnelEntry(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel))
                {
                    VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_tunnel.uc_vxlan_port, l_vxlan_port);
                    AMTR_PMGR_DeleteAddrByVidAndLPort(l_vxlan_port, vni_entry.vfi);
                }
            }

            /* Delete UC and MC VTEP for network port.
             */
            if ( TRUE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
            {
                ret = VXLAN_OM_DelFloodRVtep(&rvtep_entry);

                if (vni_entry.vfi != 0)
                {
                    /* If no unicast flooding in the VNI, let multicast flooding work.
                     */
                    uc_rvtep_entry.vni = vni;
                    mc_rvtep_entry.vni = vni;
                    if (    (VXLAN_TYPE_RETURN_OK == ret)
                        &&  (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetNextFloodRVtepByVni(&uc_rvtep_entry))
                        &&  (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetFloodMulticastByVni(&mc_rvtep_entry))
                       )
                    {
                        ret = VXLAN_MGR_AddFloodMulticastEntry(&vni_entry, &mc_rvtep_entry);
                    }
                }
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Destroy VTEP error!\r\n");
                return VXLAN_TYPE_DRIVER_ERROR;
            }
        }
        else
        {
            /* Delete a unexisted entry, no error.
             */
            ret = VXLAN_TYPE_RETURN_OK;
        }
    }
    else
    {
        while(VXLAN_OM_GetNextVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
        {
            rvtep_entry.vni = vni_entry.vni;
            memcpy(&rvtep_entry.ip, ip_p, sizeof(L_INET_AddrIp_T));
            if (VXLAN_OM_GetFloodRVtepByVniIp(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
            {
                if (    (TRUE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
                    &&  (VXLAN_TYPE_RETURN_OK == VXLAN_OM_DelFloodRVtep(&rvtep_entry))
                   )
                {
                    memset(&uc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
                    memset(&mc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
                    uc_rvtep_entry.vni = vni;
                    mc_rvtep_entry.vni = vni;
                    if (    (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetNextFloodRVtepByVni(&uc_rvtep_entry))
                        &&  (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetFloodMulticastByVni(&mc_rvtep_entry))
                       )
                    {
                        ret = VXLAN_MGR_AddFloodMulticastEntry(&vni_entry, &mc_rvtep_entry);
                        if (    (ret != VXLAN_TYPE_RETURN_OK)
                            &&  (ret != VXLAN_TYPE_SRC_IP_NOT_FIND)
                            &&  (ret != VXLAN_TYPE_SRC_IF_NOT_FIND)
                            &&  (ret != VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
                           )
                        {
                            return VXLAN_TYPE_RETURN_ERROR;
                        }
                    }
                    continue;
                }
                else
                {
                    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Destroy VTEP error! VNI[%lu]\r\n",
                        (unsigned long)rvtep_entry.vni);
                    return VXLAN_TYPE_RETURN_ERROR;
                }
            }
        }
        ret = VXLAN_TYPE_RETURN_OK;
    }

    return ret;
}
/* FUNCTION NAME: VXLAN_MGR_AddFloodMulticast
 * PURPOSE : Flooding to which multicast group, when received packet lookup bridge table fail.
 * INPUT   : vni -- VXLAN ID
 *           m_ip_p -- IP address of multicast group
 *           vid -- VLAN ID
 *           lport -- logical port
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_AddFloodMulticast(UI32_T vni, L_INET_AddrIp_T *m_ip_p, UI16_T vid, UI32_T lport)
{
    VXLAN_OM_VNI_T   vni_entry;
    VXLAN_OM_RVtep_T rvtep_entry;
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;

    if (vni > VXLAN_TYPE_VNI_ID_MAX)
    {
        return ret;
    }

    if (FALSE == IP_LIB_IsMulticastIp(m_ip_p->addr))
    {
        return VXLAN_TYPE_IP_INVALID;
    }

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));

    if (vni != 0)
    {
        vni_entry.vni = vni;
        if (VXLAN_OM_GetVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
        {
            rvtep_entry.vni = vni_entry.vni;

            /* Each VNI only have one multicast group.
             */
            if (   (VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
                && (    (0 != memcmp(&rvtep_entry.ip, &m_ip_p, sizeof(L_INET_AddrIp_T)))
                    ||  (rvtep_entry.vid != vid)
                    ||  (rvtep_entry.lport != lport)
                   )
               )
            {
                return VXLAN_TYPE_ENTRY_EXISTED;
            }
            memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            rvtep_entry.vni = vni;
            memcpy(&rvtep_entry.ip, m_ip_p, sizeof(L_INET_AddrIp_T));
            rvtep_entry.vid = vid;
            rvtep_entry.lport = lport;
            ret = VXLAN_MGR_AddFloodMulticastEntry(&vni_entry, &rvtep_entry);
        }
        else
        {
            ret = VXLAN_TYPE_VNI_NOT_MATCH;
        }
    }
    else
    {
        while(VXLAN_OM_GetNextVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
        {
            rvtep_entry.vni = vni_entry.vni;

            /* Each VNI only have one multicast group.
             */
            if (   (VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
                && (    (0 != memcmp(&rvtep_entry.ip, &m_ip_p, sizeof(L_INET_AddrIp_T)))
                    ||  (rvtep_entry.vid != vid)
                    ||  (rvtep_entry.lport != lport)
                   )
               )
            {
                continue;
            }
            memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            rvtep_entry.vni = vni_entry.vni;
            memcpy(&rvtep_entry.ip, m_ip_p, sizeof(L_INET_AddrIp_T));
            rvtep_entry.vid = vid;
            rvtep_entry.lport = lport;
            ret = VXLAN_MGR_AddFloodMulticastEntry(&vni_entry, &rvtep_entry);
            if (    (ret != VXLAN_TYPE_RETURN_OK)
                &&  (ret != VXLAN_TYPE_SRC_IP_NOT_FIND)
                &&  (ret != VXLAN_TYPE_SRC_IF_NOT_FIND)
                &&  (ret != VXLAN_TYPE_SRC_IF_IP_NOT_FIND)
               )
            {
                break;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME: VXLAN_MGR_DelFloodMulticast
 * PURPOSE : Delete flooding multicast group.
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_DelFloodMulticast(UI32_T vni)
{
    VXLAN_OM_VNI_T   vni_entry;
    VXLAN_OM_RVtep_T rvtep_entry;
    L_INET_AddrIp_T  tmp_m_ip;
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  l_vxlan_port = 0;
    BOOL_T  driver_ret;

    if (vni > VXLAN_TYPE_VNI_ID_MAX)
    {
        return ret;
    }

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&tmp_m_ip, 0, sizeof(L_INET_AddrIp_T));

    if (vni != 0)
    {
        rvtep_entry.vni = vni;
        if (VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
        {
            /* Delete MC VTEP.
             */
            if (    (0 != memcmp(&(rvtep_entry.s_ip), &tmp_m_ip, sizeof(L_INET_AddrIp_T)))
                &&  (0 != rvtep_entry.mc_vxlan_port)
               )
            {
                /* Delete VTEP in chip.
                 */
                type = SWCTRL_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
                if (SWCTRL_LPORT_TRUNK_PORT == type)
                {
                    port = trunk_id;
                }

                SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, FALSE);
#if 0
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(rvtep_entry.lport,
                    vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(rvtep_entry.mc_vxlan_port, l_vxlan_port);
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                    vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                if (FALSE == SWDRV_DestroyVxlanPort(rvtep_entry.vfi, rvtep_entry.mc_vxlan_port, FALSE))
                {
                    SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, TRUE);
                    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Destroy VTEP error!\r\n");
                    return VXLAN_TYPE_DRIVER_ERROR;
                }
            }

            ret = VXLAN_OM_DelFloodMulticast(&rvtep_entry);
        }
        else
        {
            /* Delete a unexisted entry, no error.
             */
            ret = VXLAN_TYPE_RETURN_OK;
        }
    }
    else
    {
        while(VXLAN_OM_GetNextVniEntry(&vni_entry) == VXLAN_TYPE_RETURN_OK)
        {
            rvtep_entry.vni = vni_entry.vni;
            if (VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
            {
               /* Delete MC VTEP.
                */
                if (    (0 != memcmp(&(rvtep_entry.s_ip), &tmp_m_ip, sizeof(L_INET_AddrIp_T)))
                    &&  (0 != rvtep_entry.mc_vxlan_port)
                   )
                {
                    /* Delete VTEP in chip.
                     */
                    type = SWCTRL_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
                    if (SWCTRL_LPORT_TRUNK_PORT == type)
                    {
                        port = trunk_id;
                    }

                    SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, FALSE);
#if 0
                    AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(rvtep_entry.lport,
                        vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                    VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(rvtep_entry.mc_vxlan_port, l_vxlan_port);
                    AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                        vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                    if (FALSE == SWDRV_DestroyVxlanPort(vni_entry.vfi, rvtep_entry.mc_vxlan_port, FALSE))
                    {
                        SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, TRUE);
                        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Destroy VTEP error! vfi[0x%lx], port[%lu/%lu], VXLAN port[0x%lx]\r\n",
                            (unsigned long)rvtep_entry.vni, (unsigned long)unit, (unsigned long)port, (unsigned long)rvtep_entry.mc_vxlan_port);
                        return VXLAN_TYPE_DRIVER_ERROR;
                    }
                }
                VXLAN_OM_DelFloodMulticast(&rvtep_entry);
            }
        }

        ret = VXLAN_TYPE_RETURN_OK;
    }

    return ret;
}

/* FUNCTION NAME: VXLAN_MGR_AddVpn
 * PURPOSE : Create Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_MGR_AddVpn(UI32_T vni)
{
    VXLAN_OM_VNI_T          vni_entry;
    SWDRV_VxlanVpnInfo_T    vpn_info;

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    vni_entry.vni = vni;
    if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetVniEntry(&vni_entry))
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "VNI %lu is already exist.\r\n", (unsigned long)vni);
        return VXLAN_TYPE_VNI_EXIST;
    }

    memset(&vpn_info, 0, sizeof(SWDRV_VxlanVpnInfo_T));
    vpn_info.vnid = vni;
    if (!SWDRV_SetVxlanVpn(&vpn_info, TRUE))
        return VXLAN_TYPE_RETURN_ERROR;

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    vni_entry.vni = vpn_info.vnid;
    vni_entry.vfi = vpn_info.vfi;
    vni_entry.bcast_group = vpn_info.bc_group;
    return VXLAN_OM_SetVniEntry(&vni_entry);
}

/* FUNCTION NAME: VXLAN_MGR_DeleteVpn
 * PURPOSE : Destroy a Vxlan VPN
 * INPUT   : vni -- VXLAN ID
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_ENTRY_NOT_EXISTED /
 *           VXLAN_TYPE_ENTRY_EXISTED /
 *           VXLAN_TYPE_RETURN_ERROR
 * NOTES   : None
 */
UI32_T VXLAN_MGR_DeleteVpn(UI32_T vni)
{
    VXLAN_OM_VNI_T          vni_entry;
    SWDRV_VxlanVpnInfo_T    vpn_info;
    VXLAN_OM_RVtep_T         rvtep_entry;
    UI32_T lport = 0;
    UI16_T vid = 0;

    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    vni_entry.vni = vni;
    if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetVniEntry(&vni_entry))
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "VNI %lu not exist.\r\n", (unsigned long)vni);
        return VXLAN_TYPE_VNI_NOT_EXIST;
    }

    /* If network vtep is existed, shall not unbind.
     */
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    rvtep_entry.vni = vni;
    if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextFloodRVtepByVni(&rvtep_entry))
    {
        return VXLAN_TYPE_ENTRY_EXISTED;
    }

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    rvtep_entry.vni = vni;
    if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry))
    {
        return VXLAN_TYPE_ENTRY_EXISTED;
    }

    AMTR_PMGR_DeleteAddrByVidAndLifeTime(vni_entry.vfi ,AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT);

    /* when the vni is deleted, those access port belonging to the vni also need to be deleted,
       Those access port still exist in the OM, but have no practical effect. Otherwise creating
       the same access port by the same vni will failed.
    */
    while (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextPortVlanVniMapByVni(vni_entry.vni, &lport, &vid))
    {
        VXLAN_MGR_SetPortVlanVniMap(lport, vid, vni_entry.vni, FALSE);
    }

    memset(&vpn_info, 0, sizeof(SWDRV_VxlanVpnInfo_T));
    vpn_info.vnid = vni_entry.vni;
    vpn_info.vfi = vni_entry.vfi;
    vpn_info.bc_group = vni_entry.bcast_group;
    if (!SWDRV_SetVxlanVpn(&vpn_info, FALSE))
    {
        return VXLAN_TYPE_RETURN_ERROR;
    }

    return VXLAN_OM_DelVniEntry(&vni_entry);
}

/* FUNCTION NAME: VXLAN_MGR_SetVlanVniMap
 * PURPOSE : Configure VLAN and VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           vid -- VLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_SetVlanVniMap(UI16_T vid, UI32_T vni, BOOL_T is_add)
{
    L_INET_AddrIp_T       l_vtep_ip;
    L_INET_AddrIp_T       r_vtep_ip;
    VXLAN_OM_VNI_T        vni_entry;
    VXLAN_OM_RVtep_T            rvtep_entry;
    SWDRV_VxlanVpnInfo_T        vpn_info;
    SWDRVL3_TunnelIntfL3_T      tl3_entry;
    UI32_T ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T vfi = 0;
    UI32_T bcast_group = 0;
    UI32_T lport = 0;
    UI32_T vid_ifindex = 0;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  unit = 0;
    UI32_T  port = 0;
    UI32_T  trunk_id = 0;
    UI32_T  vxlan_port = 0, l_vxlan_port = 0;
    UI32_T  l3_if = 0;
    UI32_T  cur_vni = 0;
    BOOL_T  access_port_created = FALSE;
    UI16_T  udp_port = 0;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};

    if (   (vid < VXLAN_TYPE_VLAN_ID_MIN)
        || (vid > VXLAN_TYPE_VLAN_ID_MAX)
       )
    {
        return ret;
    }

    if (   (vni < VXLAN_TYPE_VNI_ID_MIN)
        || (vni > VXLAN_TYPE_VNI_ID_MAX)
       )
    {
        return ret;
    }

    memset(&l_vtep_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&r_vtep_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&vpn_info, 0, sizeof(SWDRV_VxlanVpnInfo_T));
    memset(&tl3_entry, 0, sizeof(SWDRVL3_TunnelIntfL3_T));

    if (is_add == TRUE)
    {
        if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            if (cur_vni == vni)
            {
                /* No change
                 */
                return VXLAN_TYPE_RETURN_OK;
            }
            else
            {
                /* Each VLAN can be assigned to only one VNI
                 */
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "VLAN %u is assigned to VNI %lu\r\n", vid, (unsigned long)cur_vni);
                return VXLAN_TYPE_RETURN_ERROR;
            }
        }

        memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
        vni_entry.vni = vni;
        if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetVniEntry(&vni_entry))
        {
            /* Each VNI can be assigned to only one VLAN
             */
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Another VLAN is assigned to VNI %lu\r\n", (unsigned long)vni);
            return VXLAN_TYPE_RETURN_ERROR;
        }

        vpn_info.vnid = vni;
        if (SWDRV_SetVxlanVpn(&vpn_info, is_add) == TRUE)
        {
            memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
            vni_entry.vni = vpn_info.vnid;
            vni_entry.vfi = vpn_info.vfi;
            vni_entry.bcast_group = vpn_info.bc_group;
            VXLAN_OM_SetVlanVniMap(vid, vni);
            ret = VXLAN_OM_SetVniEntry(&vni_entry);

            if (ret == VXLAN_TYPE_RETURN_OK)
            {
                /* Do VLAN-VNI mapping after creating VLAN, shall create l3 interface here.
                 */
                l3_if = VXLAN_OM_GetL3If(vid);
#if 0
                if (    (TRUE == VLAN_OM_IsVlanExisted(vid))
                    &&  (0 == l3_if)
                   )
#endif
                if (0 == l3_if)
                {
                    tl3_entry.vid = vid;
                    SWCTRL_GetCpuMac(tl3_entry.src_mac);
                    tl3_entry.is_add = TRUE;
                    if (SWDRVL3_L3_NO_ERROR == SWDRVL3_AddTunnelIntfL3(&tl3_entry))
                    {
                        l3_if = tl3_entry.l3_intf_id;
                        VXLAN_OM_SetL3If(vid, l3_if, TRUE);
                    }
                    else
                    {
                        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Not create VXLAN interface for VLAN %u\r\n", vid);
                    }
                }

                /* Add access port
                 */
                lport   = 0;
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                VXLAN_OM_GetUdpDstPort(&udp_port);
                while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    if (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
                    {
                        /* Create UC VTEP for access port.
                         */
                        type = SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
                        if (SWCTRL_LPORT_TRUNK_PORT == type)
                        {
                            port = trunk_id;
                        }
                        if (TRUE == SWDRV_CreateVxlanAccessPort(vni_entry.vfi, l3_if, unit, port, mac_ar, DEV_SWDRV_VXLAN_MATCH_PORT_VLAN, &vxlan_port))
                        {
                            /* Add to multicast group for access port.
                             */
                            SWDRV_AddVtepIntoMcastGroup(vni_entry.bcast_group, vxlan_port, TRUE);
                            VXLAN_OM_SetAccessVxlanPort(vid, lport, vxlan_port, TRUE);
                            access_port_created = TRUE;
                        }
                        else
                        {
                            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Not create access port %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
                            return VXLAN_TYPE_DRIVER_ERROR;
                        }
                    }
                }/* End of while() */

                if (access_port_created == TRUE)
                {
                    /* Set network port to driver layer after first access port is created.
                     */
                    VXLAN_MGR_ActivateVtep(vni);
                }
            }
        }/* End of SWDRV_SetVxlanVpn() */
        else
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Not create VPN %lu\r\n", (unsigned long)vni);
        }
    } /* End of is_add */
    else
    {
        if (VXLAN_OM_GetVlanVniMapEntry(vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            /* Check vni.
             */
            if (vni != cur_vni)
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "VLAN %u is assigned to VNI %lu\r\n", vid, (unsigned long)cur_vni);
                return VXLAN_TYPE_VNI_NOT_MATCH;
            }

            /* If network vtep is existed, shall not unbind.
             */
            memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            rvtep_entry.vni = vni;
            if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextFloodRVtepByVni(&rvtep_entry))
            {
                return VXLAN_TYPE_ENTRY_EXISTED;
            }

            memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            rvtep_entry.vni = vni;
            if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry))
            {
                return VXLAN_TYPE_ENTRY_EXISTED;
            }

            vni_entry.vni = vni;
            if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetVniEntry(&vni_entry))
            {
                /* Delete access port
                 */
                lport   = 0;
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                VXLAN_OM_GetUdpDstPort(&udp_port);
                while (SWCTRL_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    if (VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
                    {
                        type = SWCTRL_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
                        if (SWCTRL_LPORT_TRUNK_PORT == type)
                        {
                            port = trunk_id;
                        }
                        vxlan_port = VXLAN_OM_GetAccessVxlanPort(vid, lport);
                        if (vxlan_port != 0)
                        {
                            SWDRV_SetVxlanPortLearning(vxlan_port, FALSE);
#if 0
                            AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(lport,
                                vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                            VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_port, l_vxlan_port);
                            AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                                vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                            if (TRUE == SWDRV_DestroyVxlanPort(vni_entry.vfi, vxlan_port, FALSE))
                            {
                                VXLAN_OM_SetAccessVxlanPort(vid, lport, 0, FALSE);
                            }
                            else
                            {
                                SWDRV_SetVxlanPortLearning(vxlan_port, TRUE);
                                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "SWDRV_DestroyVxlanPort return error! vfi[0x%lx], VXLAN port[0x%lx]\r\n",
                                    (unsigned long)vni_entry.vfi, (unsigned long)vxlan_port);
                                return VXLAN_TYPE_DRIVER_ERROR;
                            }
                        }
                    }
                }

                /* If no access port in the VNI, to remove all network ports from chip.
                 */
                if (VXLAN_OM_GetVlanMemberCounter(vid) == 0)
                {
                    VXLAN_MGR_DeactivateVtep(vni);
                }

                /* First clear MAC and then delete VPN
                 */
                AMTR_PMGR_DeleteAddrByVidAndLifeTime(vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                /* Delete VLAN-VNI mapping
                 */
                vpn_info.vnid = vni_entry.vni;
                vpn_info.vfi = vni_entry.vfi;
                vpn_info.bc_group = vni_entry.bcast_group;
                if (SWDRV_SetVxlanVpn(&vpn_info, is_add) == TRUE)
                {
                    VXLAN_OM_SetVlanVniMap(vid, 0);
                    ret = VXLAN_OM_DelVniEntry(&vni_entry);
                }
                else
                {
                    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Fail to delete VPN  %lu with vfi[0x%lx] and bcast_group[0x%lx]\r\n",
                        (unsigned long)vni, (unsigned long)vni_entry.vfi, (unsigned long)vni_entry.bcast_group);
                }
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "VXLAN_OM_GetVniEntry error!\r\n");
            }
        }
        else
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "VXLAN_OM_GetVlanVniMapEntry return error!\r\n");
        }
    }
    return ret;
}

/* FUNCTION NAME: VXLAN_MGR_SetPortVlanVniMap
 * PURPOSE : Configure Port (and VLAN) to VNI mapping relationship.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 *           vni -- VXLAN ID
 *           is_add -- TRUE means add, FALSE means delete.
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : vid = 0 means per-port access port
 */
UI32_T VXLAN_MGR_SetPortVlanVniMap(UI32_T lport, UI16_T vid, UI32_T vni, BOOL_T is_add)
{
    VXLAN_OM_VNI_T        vni_entry;
    //SWDRV_VxlanVpnInfo_T  vpn_info;
    UI32_T vid_ifindex = 0;
    UI32_T cur_vni = 0;

    if (lport < 1 || lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        return VXLAN_TYPE_RETURN_ERROR;

    if (vid != 0 && (vid < VXLAN_TYPE_VLAN_ID_MIN || vid > VXLAN_TYPE_VLAN_ID_MAX))
        return VXLAN_TYPE_RETURN_ERROR;

    if (vni < VXLAN_TYPE_VNI_ID_MIN || vni > VXLAN_TYPE_VNI_ID_MAX)
        return VXLAN_TYPE_RETURN_ERROR;

    if (is_add)
    {
        if (VXLAN_OM_GetPortVlanVniMapEntry(lport, vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            if (cur_vni == vni) /* No change */
                return VXLAN_TYPE_RETURN_OK;
        }

        if (0 != VXLAN_OM_GetAccessVxlanPort(vid, lport))
        {
            AMTR_TYPE_AddrEntry_T addr_entry;
            UI32_T unit  = 0;
            UI32_T port  = 0;
            UI32_T trunk = 0;

            memset(&addr_entry,0,sizeof(AMTR_TYPE_AddrEntry_T));
            /* check if the same port & vid that have been tied to the other vni
               that static MAC has already set to.
            */
            while (AMTR_PMGR_GetNextRunningStaticAddrEntry(&addr_entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
            {
                if (SWCTRL_LPORT_VXLAN_PORT == SWCTRL_POM_LogicalPortToUserPort((UI32_T)addr_entry.ifindex,&unit,&port,&trunk))
                {
                    UI32_T r_vxlan_port = 0;
                    UI16_T tmp_vid;
                    UI32_T tmp_port;
                    UI32_T tmp_vni;

                    if (VXLAN_TYPE_IS_L_PORT(addr_entry.ifindex))
                        VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(addr_entry.ifindex, r_vxlan_port);

                    if (VXLAN_OM_GetVlanNlportOfAccessPort(r_vxlan_port, &tmp_vid, &tmp_port))
                    {
                        if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetPortVlanVniMapEntry(tmp_port, tmp_vid, &tmp_vni))
                        {
                            if ((tmp_vid==vid) && (tmp_port==lport) && (tmp_vni==cur_vni))
                            {
                                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "The current VNI %lu is already tied to static MAC.\r\n", (unsigned long)cur_vni);
                                return VXLAN_TYPE_RETURN_ERROR;
                            }
                        }
                    }
                }
            }
            VXLAN_MGR_DeleteAccessPort(cur_vni, vid, lport);
        }

        memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
        vni_entry.vni = vni;
        if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetVniEntry(&vni_entry))
        {
            return VXLAN_TYPE_VNI_NOT_EXIST;
#if 0
            memset(&vpn_info, 0, sizeof(SWDRV_VxlanVpnInfo_T));
            vpn_info.vnid = vni;
            if (!SWDRV_SetVxlanVpn(&vpn_info, TRUE))
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Fail to create VPN %lu\r\n", (unsigned long)vni);
                return VXLAN_TYPE_RETURN_ERROR;
            }

            memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
            vni_entry.vni = vpn_info.vnid;
            vni_entry.vfi = vpn_info.vfi;
            vni_entry.bcast_group = vpn_info.bc_group;
            VXLAN_OM_SetVniEntry(&vni_entry);
#endif
        }

        VXLAN_OM_SetPortVlanVniMap(lport, vid, vni);

        /* Add access port
         */
        if (vid != 0)
        {
            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
        }

        //if (vid == 0 || VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
        {
            if (VXLAN_MGR_AddAccessPort(vni, vid, lport))
            {
                /* Set network port to driver layer after first access port is created.
                 */
                if (vni_entry.nbr_of_acc_port == 0)
                    VXLAN_MGR_ActivateVtep(vni);
            }
        }
    } /* End of is_add */
    else
    {
        if (vid == 0)
        {
            UI32_T tmp_lport = lport;
            UI16_T tmp_vid = 0;

            if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextPortVlanVniMapByVni(vni, &tmp_lport, &tmp_vid))
            {
                if (lport == tmp_lport)
                    vid = tmp_vid;
                else
                    return VXLAN_TYPE_ACCESS_PORT_NOT_FIND;
            }
        }

        if (VXLAN_OM_GetPortVlanVniMapEntry(lport, vid, &cur_vni) == VXLAN_TYPE_RETURN_OK)
        {
            /* Check vni.
             */
            if (vni != cur_vni)
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VNI_MSG, "Port %lu VLAN %u is assigned to VNI %lu\r\n", (unsigned long)lport, vid, (unsigned long)cur_vni);
                return VXLAN_TYPE_VNI_NOT_MATCH;
            }

            memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
            vni_entry.vni = vni;
            if (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetVniEntry(&vni_entry))
            {
                if (VXLAN_OM_GetAccessVxlanPort(vid, lport) != 0)
                {
                    if (vni_entry.nbr_of_acc_port == 1) /* last access port */
                    {
                        VXLAN_MGR_DeactivateVtep(vni);
                        VXLAN_MGR_DeleteAccessPort(vni, vid, lport);
                        VXLAN_OM_SetPortVlanVniMap(lport, vid, 0);
#if 0
                        vpn_info.vnid = vni_entry.vni;
                        vpn_info.vfi = vni_entry.vfi;
                        vpn_info.bc_group = vni_entry.bcast_group;
                        if (SWDRV_SetVxlanVpn(&vpn_info, FALSE))
                        {
                            VXLAN_OM_DelVniEntry(&vni_entry);
                        }
#endif
                    }
                    else
                    {
                        VXLAN_MGR_DeleteAccessPort(vni, vid, lport);
                    }
                }
                else //not really create to chip, delete from om only ???
                {
                    VXLAN_OM_SetPortVlanVniMap(lport, vid, 0);
                }
            }
        }
    }

    return VXLAN_TYPE_RETURN_OK;
}


/* FUNCTION NAME: VXLAN_MGR_SetSrcIf
 * PURPOSE : Set source interface ifindex of VTEP.
 * INPUT   : ifindex -- interface index
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_MGR_SetSrcIf(UI32_T ifindex)
{
    UI32_T                      ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T                      cur_ifindex = 0;
    L_INET_AddrIp_T             no_ip;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Set Src Interface: task ID[%lu] ifindex[%lu]\r\n",
        SYSFUN_TaskIdSelf(), (unsigned long)ifindex);
    memset(&no_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    VXLAN_OM_GetSrcIf(&cur_ifindex);
    if (cur_ifindex == ifindex)
    {
        /* No change.
         */
        return VXLAN_TYPE_RETURN_OK;
    }

    if (ifindex == 0)
    {
        /* Unset source interface.
         * Delete all network tunnel ports in chip.
         */
        ret = VXLAN_MGR_DeleteVtepSourceIp(ifindex, &no_ip);
    }
    else
    {
        /* Get VLAN primary IP.
         * Delete all Delete all network tunnel ports in chip, and re-create them by using new IP.
         */
        rif_config.ifindex = ifindex;
        if(NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            ret = VXLAN_MGR_UpdateVtepSourceIp(&(rif_config.addr));
        }
    }

    if (ret == VXLAN_TYPE_RETURN_OK)
    {
        ret = VXLAN_OM_SetSrcIf(ifindex);
    }
    return ret;
}

/* FUNCTION NAME: VXLAN_MGR_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message for VXLAN MGR.
 * INPUT   : msgbuf_p -- input request ipc message buffer
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTES   : None.
 */
BOOL_T VXLAN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */
    VXLAN_MGR_IpcMsg_T *msg_p;

    /* BODY
     */
    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (VXLAN_MGR_IpcMsg_T *) msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msg_p->type.ret_ui32 = VXLAN_TYPE_RETURN_ERROR;
        msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding ERPS_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case VXLAN_MGR_IPCCMD_SETUDPDSTPORT:
            msg_p->type.ret_ui32 = VXLAN_MGR_SetUdpDstPort(msg_p->data.arg_ui16);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_ADDFLOODRVTEP:
            msg_p->type.ret_ui32 = VXLAN_MGR_AddFloodRVtep(msg_p->data.arg_grp_ui32_ipaddr.arg_ui32,
                                        &(msg_p->data.arg_grp_ui32_ipaddr.arg_ipaddr));
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_DELFLOODRVTEP:
            msg_p->type.ret_ui32 = VXLAN_MGR_DelFloodRVtep(msg_p->data.arg_grp_ui32_ipaddr.arg_ui32,
                                        &(msg_p->data.arg_grp_ui32_ipaddr.arg_ipaddr));
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_ADDFLOODMULTICAST:
            msg_p->type.ret_ui32 = VXLAN_MGR_AddFloodMulticast(msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ui32_1,
                                        &(msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ipaddr),
                                        msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ui16,
                                        msg_p->data.arg_grp_ui32_ipaddr_ui16_ui32.arg_ui32_2);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_DELFLOODMULTICAST:
            msg_p->type.ret_ui32 = VXLAN_MGR_DelFloodMulticast(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_ADDVPN:
            msg_p->type.ret_ui32 = VXLAN_MGR_AddVpn(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_DELETEVPN:
            msg_p->type.ret_ui32 = VXLAN_MGR_DeleteVpn(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_SETVLANVNIMAP:
            msg_p->type.ret_ui32 = VXLAN_MGR_SetVlanVniMap(msg_p->data.arg_grp_ui16_ui32_bool.arg_ui16,
                                        msg_p->data.arg_grp_ui16_ui32_bool.arg_ui32,
                                        msg_p->data.arg_grp_ui16_ui32_bool.arg_bool);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_SETPORTVLANVNIMAP:
            msg_p->type.ret_ui32 = VXLAN_MGR_SetPortVlanVniMap(msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_ui32_1,
                                        msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_ui16,
                                        msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_ui32_2,
                                        msg_p->data.arg_grp_ui32_ui16_ui32_bool.arg_bool);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        case VXLAN_MGR_IPCCMD_SETSRCIF:
            msg_p->type.ret_ui32 = VXLAN_MGR_SetSrcIf(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
            break;
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_bool = FALSE;
            msg_p->type.ret_ui32 = VXLAN_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = VXLAN_MGR_IPCMSG_TYPE_SIZE;
    }
    return TRUE;
} /* End of VXLAN_MGR_HandleIPCReqMsg */

/* LOCAL SUBPROGRAM BODIES
 */

static void VXLAN_MGR_MultiIp2Mac(UI8_T *m_ip_p, UI8_T *mac_ar)
{
    if(m_ip_p[0] == 0xff) /*ipv6*/
    {
        mac_ar[0] = 0x33;
        mac_ar[1] = 0x33;
        memcpy(&mac_ar[2], &m_ip_p[SYS_ADPT_IPV6_ADDR_LEN - 4], 4);
    }
    else /*ipv4*/
    {
        mac_ar[0] = 0x01;
        mac_ar[1] = 0x00;
        mac_ar[2] = 0x5E;
        memcpy(&mac_ar[3], &m_ip_p[SYS_ADPT_IPV4_ADDR_LEN - 3], 3);
        mac_ar[3] &= 0x7F;
    }
    return;
}

/* FUNCTION NAME: VXLAN_MGR_AddFloodRVtepEntry
 * PURPOSE : Update remote VTEP to driver and OM.
 * INPUT   : vni_entry_p->vni, vni_entry_p->vfi
 *           rvtep_entry_p->vni, rvtep_entry_p->ip
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
static UI32_T VXLAN_MGR_AddFloodRVtepEntry(VXLAN_OM_VNI_T *vni_entry_p, VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    L_INET_AddrIp_T src_ip;
    L_INET_AddrIp_T zero_ip;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    L_INET_AddrIp_T nexthops_ip_ar[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
    UI32_T  nexthops_if_ar[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
    UI32_T  nexthop_cnt = 1;
    UI32_T  owner;
    UI32_T  i;
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  nexthop_if = 0;
    UI32_T  l3_if =0;
    UI32_T  src_ifindex;
    BOOL_T  found_src = FALSE;
    UI16_T  udp_port = 0;
    UI16_T  e_vlan = 0;

    memset(&host_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    memset(&src_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&rif_config, 0, sizeof(rif_config));
    memset(&nexthops_ip_ar, 0, sizeof(nexthops_ip_ar));
    memset(&nexthops_if_ar, 0, sizeof(nexthops_if_ar));

    if (    (VXLAN_OM_GetFloodRVtepByVniIp(rvtep_entry_p) != VXLAN_TYPE_RETURN_OK)
        &&  (VXLAN_OM_GetTotalRVtepNumber(TRUE) >= SYS_ADPT_VXLAN_MAX_NBR_OF_UC_FLOOD_ENTRY)
       )
    {
        return VXLAN_TYPE_TABLE_FULL;
    }

    VXLAN_OM_GetUdpDstPort(&udp_port);

    /* Get source interface VLAN ID
     */
    VXLAN_OM_GetSrcIf(&src_ifindex);
    if (0 != src_ifindex)
    {
        /* Get primary IP on source interface VLAN
         */
        memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
        rif_config.ifindex = src_ifindex;
        if(NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            memcpy(&(rvtep_entry_p->s_ip), &(rif_config.addr), sizeof(L_INET_AddrIp_T));
            found_src = TRUE;
        }
        else
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Not find primary IP on source interface ifindex = %lu\r\n",src_ifindex);
            memset(&(rvtep_entry_p->s_ip), 0, sizeof(L_INET_AddrIp_T));
        }
    }
    else
    {
       VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Not set on source interface\r\n");
       memset(&(rvtep_entry_p->s_ip), 0, sizeof(L_INET_AddrIp_T));
    }

    if (NETCFG_TYPE_OK == NETCFG_PMGR_ROUTE_FindBestRoute(&(rvtep_entry_p->ip), &nexthops_ip_ar[0], &nexthops_if_ar[0], &owner))
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Configure remote VTEP , vfi[0x%lx], D_IP[%d.%d.%d.%d], S_IP[%d.%d.%d.%d], nexthop_cnt[%lu], owner[%lu]\r\n",
            vni_entry_p->vfi,
            rvtep_entry_p->ip.addr[0],rvtep_entry_p->ip.addr[1], rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3],
            rvtep_entry_p->s_ip.addr[0],rvtep_entry_p->s_ip.addr[1], rvtep_entry_p->s_ip.addr[2], rvtep_entry_p->s_ip.addr[3],
            nexthop_cnt, owner);

        for (i=0; i<nexthop_cnt; i++)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                " new N_IP[%lu] = %d.%d.%d.%d\r\n",
                i, nexthops_ip_ar[i].addr[0], nexthops_ip_ar[i].addr[1],
                nexthops_ip_ar[i].addr[2], nexthops_ip_ar[i].addr[3]);

            /* use rvtep's ip as nexthop bcz it's directly connected
             */
            if (IPAL_ROUTE_IS_BGP_UNNUMBERED_IPV4_LLA(nexthops_ip_ar[i].addr))
                nexthops_ip_ar[i] = rvtep_entry_p->ip;
        }

        for (i=0; i<rvtep_entry_p->nexthop_cnt; i++)
        {
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                " old N_IP[%d] = %d.%d.%d.%d\r\n",
                i, rvtep_entry_p->nexthops_ip_ar[i].addr[0], rvtep_entry_p->nexthops_ip_ar[i].addr[1],
                rvtep_entry_p->nexthops_ip_ar[i].addr[2], rvtep_entry_p->nexthops_ip_ar[i].addr[3]);
        }

        if (TRUE == found_src)
        {
            if (TRUE == VXLAN_MGR_UpdateVtepNhop(vni_entry_p, rvtep_entry_p, nexthops_ip_ar, nexthops_if_ar, nexthop_cnt))
            {
                return VXLAN_TYPE_RETURN_OK;
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "VXLAN_MGR_UpdateVtepNhop error!\r\n");
                return VXLAN_TYPE_RETURN_ERROR;
            }
        }
        else
        {
            /* Only update OM, not set to chip.
             */
            UI8_T zero_addr_ar[SYS_ADPT_IPV6_ADDR_LEN] = {0};

            rvtep_entry_p->vfi = vni_entry_p->vfi;
            memcpy(rvtep_entry_p->nexthops_ip_ar, nexthops_ip_ar, sizeof(L_INET_AddrIp_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
            if(memcmp(nexthops_ip_ar[0].addr, zero_addr_ar, rvtep_entry_p->ip.addrlen) == 0)
            {
                memcpy(&(rvtep_entry_p->nexthops_ip_ar[0]), &(rvtep_entry_p->ip), sizeof(L_INET_AddrIp_T));
            }
            memcpy(rvtep_entry_p->nexthops_if_ar, nexthops_if_ar, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
            rvtep_entry_p->nexthop_cnt = nexthop_cnt;
            rvtep_entry_p->vid = 0;
            rvtep_entry_p->lport = 0;
            rvtep_entry_p->uc_vxlan_port = 0;
            rvtep_entry_p->mc_vxlan_port = 0;
            memset(rvtep_entry_p->mac_ar, 0, SYS_ADPT_MAC_ADDR_LEN);
            ret = VXLAN_OM_AddFloodRVtep(rvtep_entry_p);
            if (ret == VXLAN_TYPE_RETURN_OK)
            {
                if (0 == src_ifindex)
                    return VXLAN_TYPE_SRC_IF_NOT_FIND;
                else
                    return VXLAN_TYPE_SRC_IF_IP_NOT_FIND;
            }
            else
            {
                return ret;
            }
        }
    }
    else
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Not find route entry for IP[%d.%d.%d.%d]\r\n",
            rvtep_entry_p->ip.addr[0], rvtep_entry_p->ip.addr[1], rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3]);
        /* Only update OM, not set to chip.
         */
        rvtep_entry_p->vfi = vni_entry_p->vfi;
        memset(rvtep_entry_p->nexthops_ip_ar, 0, sizeof(L_INET_AddrIp_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
        memset(rvtep_entry_p->nexthops_if_ar, 0, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
        rvtep_entry_p->nexthop_cnt = 0;
        ret = VXLAN_OM_AddFloodRVtep(rvtep_entry_p);
        if (ret == VXLAN_TYPE_RETURN_OK)
        {
            if (found_src == FALSE)
            {
                if (0 == src_ifindex)
                    return VXLAN_TYPE_SRC_IF_NOT_FIND;
                else
                    return VXLAN_TYPE_SRC_IF_IP_NOT_FIND;
            }
            return VXLAN_TYPE_ROUTE_NOT_FIND;
        }
        else
        {
            return ret;
        }
    }

    return ret;
}


/* FUNCTION NAME: VXLAN_MGR_AddFloodMulticastEntry
 * PURPOSE : Update remote VTEP to driver and OM.
 * INPUT   : vni_entry_p->vni, vni_entry_p->vfi
 *           rvtep_entry_p->vni, rvtep_entry_p->ip, rvtep_entry_p->vid, rvtep_entry_p->lport
 * OUTPUT  : None.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
static UI32_T VXLAN_MGR_AddFloodMulticastEntry(VXLAN_OM_VNI_T *vni_entry_p, VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    SWDRVL3_TunnelIntfL3_T tl3_entry;
    UI32_T  ret = VXLAN_TYPE_RETURN_ERROR;
    UI32_T  mc_vxlan_port = 0;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id;
    UI32_T  l3_if = 0;
    UI32_T  e_ifindex = 0;
    UI32_T  src_ifindex = 0, vni = 0;
    UI16_T  udp_port = 0, vid = 0;
    UI8_T   mac_ar[SYS_ADPT_MAC_ADDR_LEN] = {0};
    NETCFG_TYPE_L3_Interface_T vlan_intf;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    VXLAN_OM_RVtep_T   uc_rvtep_entry;

    if (VXLAN_OM_GetTotalRVtepNumber(FALSE) >= SYS_ADPT_VXLAN_MAX_NBR_OF_MC_FLOOD_ENTRY)
    {
        return VXLAN_TYPE_TABLE_FULL;
    }

    VXLAN_MGR_MultiIp2Mac(rvtep_entry_p->ip.addr, mac_ar);

    type = SWCTRL_LogicalPortToUserPort(rvtep_entry_p->lport, &unit, &port, &trunk_id);
    if (SWCTRL_LPORT_TRUNK_PORT == type)
    {
        port = trunk_id;
    }
    VXLAN_OM_GetUdpDstPort(&udp_port);

    /* Don't need to create L3 interface. Shall use L3 interface of configured egress VLAN.
     */
    memset(&vlan_intf, 0, sizeof (vlan_intf));
    VLAN_VID_CONVERTTO_IFINDEX(rvtep_entry_p->vid, e_ifindex);
    vlan_intf.ifindex = e_ifindex;
    if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetL3Interface(&vlan_intf))
    {
        return VXLAN_TYPE_RETURN_ERROR;
    }
    else
    {
        l3_if = vlan_intf.drv_l3_intf_index;
    }

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "set rvtep ip[%d.%d.%d.%d] , l_vtep_ip[%d.%d.%d.%d] \r\n"
        " bcast_group[0x%lx] vfi [0x%lx], e_vlan[%u], l3_if[%lu], port[%lu/%lu]\r\n"
        " udp_port[%u], mac[%02X:%02X:%02X:%02X:%02X:%02X] \r\n", rvtep_entry_p->ip.addr[0],
        rvtep_entry_p->ip.addr[1], rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3],
        rvtep_entry_p->s_ip.addr[0],rvtep_entry_p->s_ip.addr[1], rvtep_entry_p->s_ip.addr[2], rvtep_entry_p->s_ip.addr[3],
        vni_entry_p->bcast_group, vni_entry_p->vfi, rvtep_entry_p->vid, l3_if, unit, port, udp_port,
        mac_ar[0], mac_ar[1], mac_ar[2], mac_ar[3], mac_ar[4], mac_ar[5]);

    /* Get source interface VLAN ID
     */

    VXLAN_OM_GetSrcIf(&src_ifindex);
    if (src_ifindex == 0)
    {
        /* Update OM only.
         */
        memset(&(rvtep_entry_p->s_ip), 0, sizeof(L_INET_AddrIp_T));
        rvtep_entry_p->vfi = vni_entry_p->vfi;
        rvtep_entry_p->uc_vxlan_port = 0;
        rvtep_entry_p->mc_vxlan_port = 0;
        memcpy(rvtep_entry_p->mac_ar, mac_ar, SYS_ADPT_MAC_ADDR_LEN);
        ret = VXLAN_OM_AddFloodMulticast(rvtep_entry_p);
        ret = VXLAN_TYPE_SRC_IF_NOT_FIND;
    }
    else
    {
        /* Get primary ip on source interface VLAN
         */
        memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
        rif_config.ifindex = src_ifindex;
        if(NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            /* Check if flooding unicast IP is existed or not.
             * If existed, shall not set to chip to create VTEP.
             */
            memset(&uc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            uc_rvtep_entry.vni = rvtep_entry_p->vni;
            if(VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetNextFloodRVtepByVni(&uc_rvtep_entry))
            {
                /* Check if access port had been created.
                 */
                if (VXLAN_OM_GetVlanAndVniByVfi(vni_entry_p->vfi, &vid, &vni) == VXLAN_TYPE_RETURN_OK)
                {
                    rvtep_entry_p->vfi = vni_entry_p->vfi;
                    memcpy(&(rvtep_entry_p->s_ip), &(rif_config.addr), sizeof(L_INET_AddrIp_T));
                    memcpy(rvtep_entry_p->mac_ar, mac_ar, SYS_ADPT_MAC_ADDR_LEN);
                    rvtep_entry_p->uc_vxlan_port = 0;
                    rvtep_entry_p->mc_vxlan_port = 0;

                    if (VXLAN_OM_GetVlanMemberCounter(vid) > 0)
                    {
#if 0 //KH_SHI
                        /* Set to chip. Create MC VTEP and join multicast group for network port.
                         */
                        if (FALSE == SWDRV_CreateVTEP(vni_entry_p->vfi, l3_if, unit, port, udp_port,
                                    mac_ar, TRUE, FALSE, &(rif_config.addr), &(rvtep_entry_p->ip), &mc_vxlan_port))
                        {
                            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "SWDRV_CreateVTEP return FALSE!\r\n");
                            ret = VXLAN_TYPE_DRIVER_ERROR;
                        }
                        SWDRV_AddVtepIntoMcastGroup(vni_entry_p->bcast_group, mc_vxlan_port, TRUE);
                        rvtep_entry_p->mc_vxlan_port = mc_vxlan_port;
#endif
                    }
                    else
                    {
                        /* No access port, update OM, and don't set to chip.
                         */
                        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "No access port in VNI %lu\r\n", vni);
                    }

                    ret = VXLAN_OM_AddFloodMulticast(rvtep_entry_p);
                }
            }
        }
        else
        {
            /* Update OM only.
             */
            memset(&(rvtep_entry_p->s_ip), 0, sizeof(L_INET_AddrIp_T));
            rvtep_entry_p->vfi = vni_entry_p->vfi;
            rvtep_entry_p->uc_vxlan_port = 0;
            rvtep_entry_p->mc_vxlan_port = 0;
            memcpy(rvtep_entry_p->mac_ar, mac_ar, SYS_ADPT_MAC_ADDR_LEN);
            ret = VXLAN_OM_AddFloodMulticast(rvtep_entry_p);
            ret = VXLAN_TYPE_SRC_IF_IP_NOT_FIND;
        }
    }
    return ret;
}


/* FUNCTION NAME: VXLAN_MGR_CreateFloodUcVtepToChip
 * PURPOSE : To create vtep for vxlan.
 * INPUT   : vfi_id       - vfi to create vtep
 *           bcast_group  - bcast group to set
 *           is_mc        - TRUE if it's mc vtep
 *           l_vtep_p     - local ip of vtep, not used if it's access vtep
 *           r_vtep_p     - remote ip of vtep, not used if it's access vtep
 *           nexthops_ip_p   - nexthops
 *           nexthops_if_p - interface of nethops
 *           nexthop_cnt    - number of nexthops
 * OUTPUT  : none
 * RETURN  : TRUE/FALSE
 * NOTE    : Create 1 UC VTEP + 1 MC VTEP and join multicast group for network port.
 */
static BOOL_T VXLAN_MGR_CreateFloodUcVtepToChip(
    UI32_T              vfi_id,
    UI32_T              bcast_group,
    L_INET_AddrIp_T     *l_vtep_p,
    L_INET_AddrIp_T     *r_vtep_p)
{
    AMTRL3_TYPE_VxlanTunnelEntry_T  vxlan_tunnel;
    BOOL_T ret = TRUE;
    UI32_T i;
    UI16_T udp_port;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Cteate VXLAN Tunnel, vfi_id [0x%lx], bcast_group[0x%lx] \r\n"
        " l_vtep[%d.%d.%d.%d], r_vtep[%d.%d.%d.%d]\r\n",
        vfi_id, bcast_group,
        l_vtep_p->addr[0], l_vtep_p->addr[1], l_vtep_p->addr[2], l_vtep_p->addr[3],
        r_vtep_p->addr[0], r_vtep_p->addr[1], r_vtep_p->addr[2], r_vtep_p->addr[3]);

#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    VXLAN_OM_GetUdpDstPort(&udp_port);
    memset(&vxlan_tunnel, 0, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));
    vxlan_tunnel.vfi_id = vfi_id;
    memcpy(&vxlan_tunnel.local_vtep, l_vtep_p, sizeof(L_INET_AddrIp_T));
    memcpy(&vxlan_tunnel.remote_vtep, r_vtep_p, sizeof(L_INET_AddrIp_T));
    vxlan_tunnel.is_mc = TRUE;
    vxlan_tunnel.udp_port = udp_port;
    vxlan_tunnel.bcast_group = bcast_group;

    ret = AMTRL3_PMGR_AddVxlanTunnel(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel);
    if (FALSE == ret)
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Failled to add vxlan tunnel!\r\n");
    }
#endif
    return ret;
}

 /* FUNCTION NAME: VXLAN_MGR_DestroyFloodUcVtepToChip
 * PURPOSE : To destroy vtep for vxlan.
 * INPUT   : rvtep_entry
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 */
static BOOL_T VXLAN_MGR_DestroyFloodUcVtepToChip(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    AMTRL3_TYPE_VxlanTunnelEntry_T  vxlan_tunnel;
    UI32_T vfi_index;
    BOOL_T ret = TRUE;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG,
        "Destroy VTEP: vfi [0x%lx], IP[%d.%d.%d.%d]\r\n",
        rvtep_entry_p->vfi, rvtep_entry_p->ip.addr[0], rvtep_entry_p->ip.addr[1],
        rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3]);
#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    memset(&vxlan_tunnel, 0, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));
    vxlan_tunnel.vfi_id = rvtep_entry_p->vfi;
    memcpy(&vxlan_tunnel.local_vtep, &(rvtep_entry_p->s_ip), sizeof(L_INET_AddrIp_T));
    memcpy(&vxlan_tunnel.remote_vtep, &(rvtep_entry_p->ip), sizeof(L_INET_AddrIp_T));

    ret = AMTRL3_PMGR_DeleteVxlanTunnel(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel);
    if (FALSE == ret)
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Failled to delete vxlan tunnel!\r\n");
    }
#endif
    return ret;
}

 /* FUNCTION NAME: VXLAN_MGR_UpdateVtepSourceIp
 * PURPOSE : To update vtep when primary ip set on sourece interface VLAN.
 * INPUT   : changed_ip_p  -- ponter of primary ip.
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
static UI32_T VXLAN_MGR_UpdateVtepSourceIp(L_INET_AddrIp_T *changed_ip_p)
{
    VXLAN_OM_RVtep_T    rvtep_entry;
    VXLAN_OM_RVtep_T    uc_rvtep_entry;
    VXLAN_OM_VNI_T      vni_entry;
    L_INET_AddrIp_T     no_ip;
    L_INET_AddrIp_T     nexthop_ip;
    SWCTRL_Lport_Type_T type = SWCTRL_LPORT_UNKNOWN_PORT;
    NETCFG_TYPE_L3_Interface_T  vlan_intf;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    AMTRL3_TYPE_InetHostRouteEntry_T    host_entry;
    UI32_T cur_vid = 0;
    UI32_T l3_if = 0;
    UI32_T e_vlan = 0, e_ifindex = 0;
    UI32_T unit = 0, port = 0, trunk_id = 0;
    UI32_T uc_vxlan_port = 0, mc_vxlan_port=0, l_vxlan_port = 0;
    UI32_T i;
    UI16_T udp_port = 0;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG,
        "Update source IP [%d.%d.%d.%d]\r\n",
        changed_ip_p->addr[0], changed_ip_p->addr[1], changed_ip_p->addr[2], changed_ip_p->addr[3]);

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&host_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    memset(&nexthop_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&no_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&uc_rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));

    VXLAN_OM_GetUdpDstPort(&udp_port);
    /* Update Flooding unicast IP address.
     */
    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (0 != memcmp(&(rvtep_entry.s_ip), &changed_ip_p, sizeof(L_INET_AddrIp_T)))
        {
            /* Get bcast_group by rvtep_entry.vni
             */
            memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
            vni_entry.vni = rvtep_entry.vni;
            if (VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
            {
                return VXLAN_TYPE_RETURN_ERROR;
            }

            if (0 != memcmp(&(rvtep_entry.s_ip), &no_ip, sizeof(L_INET_AddrIp_T)))
            {
                /* Delete old.
                 */
                if (FALSE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
                {
                    return VXLAN_TYPE_RETURN_ERROR;
                }
                else
                {
                    memset(&(rvtep_entry.s_ip), 0, sizeof(L_INET_AddrIp_T));
                    rvtep_entry.uc_vxlan_port = 0;
                    rvtep_entry.mc_vxlan_port = 0;
                }
            }

            /* Update orginal recored in OM
             */
            memcpy(&(rvtep_entry.s_ip), changed_ip_p, sizeof(L_INET_AddrIp_T));
            VXLAN_OM_AddFloodRVtep(&rvtep_entry);
            /* Add new.
             * Frist create vxlan tunnel and then add all nexthops to chip.
             */
            if ( (rvtep_entry.nexthop_cnt > 0)
                 && (TRUE == VXLAN_MGR_CreateFloodUcVtepToChip(rvtep_entry.vfi, vni_entry.bcast_group,
                              changed_ip_p, &(rvtep_entry.ip))
                    )
               )
            {
                for (i=0; i<rvtep_entry.nexthop_cnt; i++)
                {
                    if (FALSE == VXLAN_MGR_AddOneNhopForFloodUcVtepToChip(&vni_entry,
                                    &rvtep_entry, &(rvtep_entry.nexthops_ip_ar[i]),
                                    rvtep_entry.nexthops_if_ar[i]))
                    {
                        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG,
                            "Add nexthop error! N_IP[%d.%d.%d.%d], N_IF[%lu]\r\n",
                            rvtep_entry.nexthops_ip_ar[i].addr[0], rvtep_entry.nexthops_ip_ar[i].addr[1],
                            rvtep_entry.nexthops_ip_ar[i].addr[2], rvtep_entry.nexthops_ip_ar[i].addr[3]);
                        return VXLAN_TYPE_RETURN_ERROR;
                    }
                }
            }
        }
    }

    /* Update Flooding multicast group IP address.
     */
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    while (VXLAN_OM_GetNextFloodMulticast(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (0 != memcmp(&(rvtep_entry.s_ip), &changed_ip_p, sizeof(L_INET_AddrIp_T)))
        {
            /* Get bcast_group by rvtep_entry.vni
             */
            memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
            vni_entry.vni = rvtep_entry.vni;
            if (VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
            {
                return VXLAN_TYPE_RETURN_ERROR;
            }
            type = SWCTRL_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
            if (SWCTRL_LPORT_TRUNK_PORT == type)
            {
                port = trunk_id;
            }
            if (0 != mc_vxlan_port)
            {
                /* Delete old.
                 */
                SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, FALSE);
#if 0
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(rvtep_entry.lport,
                vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(rvtep_entry.mc_vxlan_port, l_vxlan_port);
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                if (FALSE == SWDRV_DestroyVxlanPort(rvtep_entry.vfi, rvtep_entry.mc_vxlan_port, FALSE))
                {
                    SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, TRUE);
                    return VXLAN_TYPE_RETURN_ERROR;
                }
                else
                {
                    memset(&(rvtep_entry.s_ip), 0, sizeof(L_INET_AddrIp_T));
                    rvtep_entry.uc_vxlan_port = 0;
                    rvtep_entry.mc_vxlan_port = 0;
                }
            }
            /* Add new.
             * If flooding unicast IP is existed, don't set flooding multicast IP to chip.
             */
            uc_rvtep_entry.vni = rvtep_entry.vni;
            if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_GetNextFloodRVtepByVni(&uc_rvtep_entry))
            {
                memset(&vlan_intf, 0, sizeof (vlan_intf));
                VLAN_VID_CONVERTTO_IFINDEX(rvtep_entry.vid, e_ifindex);
                vlan_intf.ifindex = e_ifindex;
                NETCFG_POM_IP_GetL3Interface(&vlan_intf);
                l3_if = vlan_intf.drv_l3_intf_index;
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG,
                    "Create VTEP for multicast: vfi[0x%lx], l3_if[%lu], unit[%lu], port[%lu], udp_port[%lu], mac_ar[%02x:%02x:%02x:%02x:%02x:%02x]\r\n",
                    vni_entry.vfi, l3_if, unit, port, udp_port,
                    rvtep_entry.mac_ar[0], rvtep_entry.mac_ar[1], rvtep_entry.mac_ar[2],
                    rvtep_entry.mac_ar[3], rvtep_entry.mac_ar[4], rvtep_entry.mac_ar[5]);
#if 0 //KH_SHI
                if (TRUE == SWDRV_CreateVTEP(vni_entry.vfi, l3_if, unit, port, udp_port,
                     rvtep_entry.mac_ar, TRUE, FALSE, changed_ip_p,
                     &(rvtep_entry.ip), &rvtep_entry.mc_vxlan_port))
                {
                    SWDRV_AddVtepIntoMcastGroup(vni_entry.bcast_group, rvtep_entry.mc_vxlan_port, TRUE);
                    memcpy(&(rvtep_entry.s_ip), changed_ip_p, sizeof(L_INET_AddrIp_T));
                    rvtep_entry.uc_vxlan_port = 0;
                }
                else
                {
                    return VXLAN_TYPE_RETURN_ERROR;
                }
#endif
            }
            else
            {
                memcpy(&(rvtep_entry.s_ip), changed_ip_p, sizeof(L_INET_AddrIp_T));
            }
            VXLAN_OM_AddFloodMulticast(&rvtep_entry);
        }
    }
    return VXLAN_TYPE_RETURN_OK;
}

/* FUNCTION NAME: VXLAN_MGR_DeleteVtepSourceIp
 * PURPOSE : To delete vtep when unset sourece interface VLAN or primary ip.
 * INPUT   : changed_ifindex  -- new source ifindex
 *           changed_ip_p  -- ponter of primary ip.
 * OUTPUT  : None
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
static UI32_T VXLAN_MGR_DeleteVtepSourceIp(UI32_T changed_ifindex, L_INET_AddrIp_T *changed_ip_p)
{
    VXLAN_OM_RVtep_T    rvtep_entry;
    VXLAN_OM_VNI_T      vni_entry;
    L_INET_AddrIp_T     no_ip;
    SWCTRL_Lport_Type_T type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T              cur_ifindex = 0;
    UI32_T              unit = 0, port = 0, trunk_id = 0, l_vxlan_port = 0;
    UI16_T              udp_port = 0;

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&no_ip, 0, sizeof(L_INET_AddrIp_T));

    VXLAN_OM_GetSrcIf(&cur_ifindex);
    VXLAN_OM_GetUdpDstPort(&udp_port);

    /* Update Flooding unicast IP address.
     */
    while (VXLAN_OM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        if (    (   (changed_ifindex ==0)
                 && (0 != memcmp(&(rvtep_entry.s_ip), &no_ip, sizeof(L_INET_AddrIp_T)))
                )
             || (   (changed_ifindex == cur_ifindex)
                 && (0 == memcmp(&(rvtep_entry.s_ip), changed_ip_p, sizeof(L_INET_AddrIp_T)))
                )
            )
        {
            /* Delete orginal network tunnel port in chip
             */
            if (TRUE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
            {
                /* Update OM.
                 */
                memset(&(rvtep_entry.s_ip), 0, sizeof(L_INET_AddrIp_T));
                rvtep_entry.uc_vxlan_port = 0;
                rvtep_entry.mc_vxlan_port = 0;
                VXLAN_OM_AddFloodRVtep(&rvtep_entry);
            }
            else
            {
                return VXLAN_TYPE_RETURN_ERROR;
            }
        }
    }

    /* Update Flooding multicast group IP address.
     */
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    while (VXLAN_OM_GetNextFloodMulticast(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        /* Get bcast_group by rvtep_entry.vni
         */
        memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
        vni_entry.vni = rvtep_entry.vni;
        if (VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
        {
            return VXLAN_TYPE_RETURN_ERROR;
        }

        /* Delete orginal network tunnel port in chip.
         */
        if (    (   (changed_ifindex ==0)
                 && (0 != memcmp(&(rvtep_entry.s_ip), &no_ip, sizeof(L_INET_AddrIp_T)))
                )
             || (   (changed_ifindex == cur_ifindex)
                 && (0 == memcmp(&(rvtep_entry.s_ip), changed_ip_p, sizeof(L_INET_AddrIp_T)))
                )
            )
        {
            if (0 != rvtep_entry.mc_vxlan_port)
            {
                type = SWCTRL_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
                if (SWCTRL_LPORT_TRUNK_PORT == type)
                {
                    port = trunk_id;
                }

                SWDRV_SetVxlanPortLearning(rvtep_entry.mc_vxlan_port, FALSE);
#if 0
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(rvtep_entry.lport,
                    vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);
#endif
                VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(rvtep_entry.mc_vxlan_port, l_vxlan_port);
                AMTR_PMGR_DeleteAddrByLifeTimeAndVidRangeAndLPort_Sync(l_vxlan_port,
                    vni_entry.vfi, vni_entry.vfi, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT);

                if (FALSE == SWDRV_DestroyVxlanPort(rvtep_entry.vfi, rvtep_entry.mc_vxlan_port, FALSE))
                {
                    return VXLAN_TYPE_RETURN_ERROR;
                }
            }
            /* Update OM.
             */
            memset(&rvtep_entry.s_ip, 0, sizeof(L_INET_AddrIp_T));
            rvtep_entry.uc_vxlan_port = 0;
            rvtep_entry.mc_vxlan_port = 0;
            VXLAN_OM_AddFloodMulticast(&rvtep_entry);
        }
    }
    return VXLAN_TYPE_RETURN_OK;
}

static BOOL_T VXLAN_MGR_UpdateVtepNhop(
    VXLAN_OM_VNI_T      *vni_entry_p,
    VXLAN_OM_RVtep_T    *rvtep_entry_p,
    L_INET_AddrIp_T     *nexthops_ip_p,
    UI32_T              *nexthops_if_p,
    UI32_T              nexthop_cnt)
{
    UI32_T  cur_nexthop_if = 0;
    UI32_T  uc_vxlan_port = 0;
    UI32_T  mc_vxlan_port = 0;
    UI32_T  l3_if = 0;
    BOOL_T  create_vtep_flag = FALSE;
    BOOL_T  destroy_vtep_flag =FALSE;
    BOOL_T  found = FALSE;
    BOOL_T  is_host_route = FALSE;
    UI32_T  i = 0, j = 0;
    UI16_T  e_vlan = 0;
    UI8_T zero_addr_ar[SYS_ADPT_IPV6_ADDR_LEN] = {0};

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
       "Update VTEP Nexthops, vfi [0x%lx], D_IP[%d.%d.%d.%d], S_IP[%d.%d.%d.%d], Nexthop Counter[%lu]\r\n",
        vni_entry_p->vfi, rvtep_entry_p->ip.addr[0],rvtep_entry_p->ip.addr[1],
        rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3],
        rvtep_entry_p->s_ip.addr[0],rvtep_entry_p->s_ip.addr[1],
        rvtep_entry_p->s_ip.addr[2], rvtep_entry_p->s_ip.addr[3],
        nexthop_cnt);
    if (nexthop_cnt >= 1)
    {
        /* Local host route, it's nexthop will be zero.
         */
        if(     (memcmp(nexthops_ip_p[0].addr, zero_addr_ar, rvtep_entry_p->ip.addrlen) == 0)
           &&   (nexthop_cnt == 1)
          )
        {
            memcpy(&(nexthops_ip_p[0]), &(rvtep_entry_p->ip), sizeof(L_INET_AddrIp_T));
        }

        rvtep_entry_p->vfi = vni_entry_p->vfi;

        /* Check current nexthop_cnt in OM, if counter is zero , frist create trunnel port.
         */
        if (rvtep_entry_p->nexthop_cnt == 0)
        {
            /* Check if access port had been created.
             */
            if (vni_entry_p->nbr_of_acc_port > 0)
            {
                if (TRUE == VXLAN_MGR_CreateFloodUcVtepToChip(vni_entry_p->vfi, vni_entry_p->bcast_group,
                        &(rvtep_entry_p->s_ip), &(rvtep_entry_p->ip))
                   )
                {
                    /* Add all nexthops to chip.
                     */
                    for(i=0; i<nexthop_cnt; i++)
                    {
                        if (FALSE == VXLAN_MGR_AddOneNhopForFloodUcVtepToChip(vni_entry_p,
                                        rvtep_entry_p, &(nexthops_ip_p[i]), nexthops_if_p[i]))
                        {
                            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                               "Failed to add nexthop to chip, vfi [0x%lx], D_IP[%d.%d.%d.%d], S_IP[%d.%d.%d.%d], N_IP[%d.%d.%d.%d], N_IF[%lu]\r\n",
                                vni_entry_p->vfi, rvtep_entry_p->ip.addr[0],rvtep_entry_p->ip.addr[1],
                                rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3],
                                rvtep_entry_p->s_ip.addr[0],rvtep_entry_p->s_ip.addr[1],
                                rvtep_entry_p->s_ip.addr[2], rvtep_entry_p->s_ip.addr[3],
                                nexthops_ip_p[i].addr[0], nexthops_ip_p[i].addr[1],
                                nexthops_ip_p[i].addr[2], nexthops_ip_p[i].addr[3],
                                nexthops_if_p[i]);
                            return FALSE;
                        }
                    }
                }
                else
                {
                    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Failed to add VTEP to chip, vfi [0x%lx], D_IP[%d.%d.%d.%d], S_IP[%d.%d.%d.%d]\r\n",
                        vni_entry_p->vfi, rvtep_entry_p->ip.addr[0],rvtep_entry_p->ip.addr[1],
                        rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3],
                        rvtep_entry_p->s_ip.addr[0],rvtep_entry_p->s_ip.addr[1],
                        rvtep_entry_p->s_ip.addr[2], rvtep_entry_p->s_ip.addr[3]);
                    return FALSE;
                }
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "No access port in VNI %lu\r\n", vni_entry_p->vni);
            }
        }
        else
        {
            /* Compare nexthops.
             */
            for(i=0; i<rvtep_entry_p->nexthop_cnt; i++)
            {
                found = FALSE;
                for(j=0; j<nexthop_cnt; j++)
                {
                    if(     (0 == memcmp(&(rvtep_entry_p->nexthops_ip_ar[i]), &(nexthops_ip_p[j]), sizeof(L_INET_AddrIp_T)))
                        &&  (rvtep_entry_p->nexthops_if_ar[i] == nexthops_if_p[j])
                      )
                    {
                        found = TRUE;
                        break;
                    }
                }

                if (FALSE == found)
                {
                    /* The nexthop is in OM, but not in new.
                     * Shall remove the nexthop from chip.
                     */
                    if (TRUE != VXLAN_MGR_DelOneNhopForFloodUcVtepToChip(vni_entry_p, rvtep_entry_p,
                                    &(rvtep_entry_p->nexthops_ip_ar[i]), rvtep_entry_p->nexthops_if_ar[i])
                       )
                    {
                        return FALSE;
                    }
                }
            }

            for(i=0; i<nexthop_cnt; i++)
            {
                found = FALSE;
                for(j=0; j<rvtep_entry_p->nexthop_cnt; j++)
                {
                    if(     (0 == memcmp(&(nexthops_ip_p[i]), &(rvtep_entry_p->nexthops_ip_ar[j]), sizeof(L_INET_AddrIp_T)))
                       &&   (nexthops_if_p[i] == rvtep_entry_p->nexthops_if_ar[j])
                      )
                    {
                        found = TRUE;
                        break;
                    }
                }
                if (FALSE == found)
                {
                    /* The nexthop is new, not in OM.
                     * Shall add the nexthop to chip.
                     */
                    if (TRUE != VXLAN_MGR_AddOneNhopForFloodUcVtepToChip(vni_entry_p, rvtep_entry_p,
                                    &(nexthops_ip_p[i]), nexthops_if_p[i])
                       )
                    {
                        return FALSE;
                    }
                }
            }
        }
        /* Update OM.
         */
        memset(rvtep_entry_p->nexthops_ip_ar, 0, sizeof(L_INET_AddrIp_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
        memset(rvtep_entry_p->nexthops_if_ar, 0, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
        for(i=0;i<nexthop_cnt;i++)
        {
            memcpy(&(rvtep_entry_p->nexthops_ip_ar[i]), &(nexthops_ip_p[i]), sizeof(L_INET_AddrIp_T));
            rvtep_entry_p->nexthops_if_ar[i] = nexthops_if_p[i];
        }
        rvtep_entry_p->nexthop_cnt = nexthop_cnt;
        if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_AddFloodRVtep(rvtep_entry_p))
        {
             return FALSE;
        }
    } /* if (nexthop_cnt >= 1)*/
    else
    {
        /* Only destroy.
           Not find route entry, shall remove this VTEP from chip and update OM entry.
         */
        if (TRUE == VXLAN_MGR_DestroyFloodUcVtepToChip(rvtep_entry_p))
        {
            memset(rvtep_entry_p->nexthops_ip_ar, 0, sizeof(L_INET_AddrIp_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
            memset(rvtep_entry_p->nexthops_if_ar, 0, sizeof(UI32_T)*SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE);
            rvtep_entry_p->nexthop_cnt = 0;
            if (VXLAN_TYPE_RETURN_OK != VXLAN_OM_AddFloodRVtep(rvtep_entry_p))
            {
                return FALSE;
            }
        }
        else
        {
            /* Error happen */
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Destroy VTEP error!\r\n vfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                rvtep_entry_p->vfi, rvtep_entry_p->ip.addr[0],rvtep_entry_p->ip.addr[1],
                rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3]);
            return FALSE;
        }
    }

    return TRUE;
}

/* FUNCTION NAME: VXLAN_MGR_AddOneNhopForFloodUcVtepToChip
 * PURPOSE : To add a nexthop to VXLAN tunnel.
 * INPUT   : vni_entry_p    - pointer of VNI entry
 *           rvtep_entry_p  - pointer of remote VTEP entry
 *           nexthops_ip_p  - pointer of nexthop IP
 *           nexthops_if_p  - pointer of nexthop interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 */
static BOOL_T VXLAN_MGR_AddOneNhopForFloodUcVtepToChip(
    VXLAN_OM_VNI_T      *vni_entry_p,
    VXLAN_OM_RVtep_T    *rvtep_entry_p,
    L_INET_AddrIp_T     *nexthop_ip_p,
    UI32_T              nexthop_if)
{
    AMTRL3_TYPE_VxlanTunnelNexthopEntry_T  tunnel_nexthop;
    BOOL_T  ret = TRUE;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Add one nexthop \r\n "
        " vni[%lu], vfi[0x%lx], l_vtep[%d.%d.%d.%d], r_vtep[%d.%d.%d.%d], nexthop[%d.%d.%d.%d], nexthop_if[%lu]\\r\n",
        vni_entry_p->vni, vni_entry_p->vfi,
        rvtep_entry_p->s_ip.addr[0], rvtep_entry_p->s_ip.addr[1], rvtep_entry_p->s_ip.addr[2], rvtep_entry_p->s_ip.addr[3],
        rvtep_entry_p->ip.addr[0], rvtep_entry_p->ip.addr[1], rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3],
        nexthop_ip_p->addr[0], nexthop_ip_p->addr[1], nexthop_ip_p->addr[2], nexthop_ip_p->addr[3], nexthop_if);

#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    memset(&tunnel_nexthop, 0, sizeof(AMTRL3_TYPE_VxlanTunnelNexthopEntry_T));
    tunnel_nexthop.vfi_id = vni_entry_p->vfi;
    memcpy(&tunnel_nexthop.local_vtep, &(rvtep_entry_p->s_ip), sizeof(L_INET_AddrIp_T));
    memcpy(&tunnel_nexthop.remote_vtep, &(rvtep_entry_p->ip), sizeof(L_INET_AddrIp_T));
    memcpy(&tunnel_nexthop.nexthop_addr, nexthop_ip_p, sizeof(L_INET_AddrIp_T));
    tunnel_nexthop.nexthop_ifindex = nexthop_if;

    ret = AMTRL3_PMGR_AddVxlanTunnelNexthop(SYS_ADPT_DEFAULT_FIB, &tunnel_nexthop);
    if (FALSE == ret)
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Failed to add a nexthop of vxlan tunnel!\r\n");
    }
#endif

    return TRUE;
}

/* FUNCTION NAME: VXLAN_MGR_DelOneNhopForFloodUcVtepToChip
 * PURPOSE : To remove a nexthop from VXLAN tunnel.
 * INPUT   : vni_entry_p    - pointer of VNI entry
 *           rvtep_entry_p  - pointer of remote VTEP entry
 *           nexthops_ip_p  - pointer of nexthop IP
 *           nexthops_if_p  - pointer of nexthop interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 */
static BOOL_T VXLAN_MGR_DelOneNhopForFloodUcVtepToChip(
    VXLAN_OM_VNI_T      *vni_entry_p,
    VXLAN_OM_RVtep_T    *rvtep_entry_p,
    L_INET_AddrIp_T     *nexthop_ip_p,
    UI32_T              nexthop_if)
{
    AMTRL3_TYPE_VxlanTunnelNexthopEntry_T  tunnel_nexthop;
    BOOL_T  ret = TRUE;

    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Delete one nexthop \r\n"
        "  vni[%lu], vfi[0x%lx], l_vtep[%d.%d.%d.%d], r_vtep[%d.%d.%d.%d], nexthop[%d.%d.%d.%d], nexthop_if[%lu]\r\n",
        vni_entry_p->vni, vni_entry_p->vfi,
        rvtep_entry_p->s_ip.addr[0], rvtep_entry_p->s_ip.addr[1], rvtep_entry_p->s_ip.addr[2], rvtep_entry_p->s_ip.addr[3],
        rvtep_entry_p->ip.addr[0], rvtep_entry_p->ip.addr[1], rvtep_entry_p->ip.addr[2], rvtep_entry_p->ip.addr[3],
        nexthop_ip_p->addr[0], nexthop_ip_p->addr[1], nexthop_ip_p->addr[2], nexthop_ip_p->addr[3], nexthop_if);

#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    memset(&tunnel_nexthop, 0, sizeof(AMTRL3_TYPE_VxlanTunnelNexthopEntry_T));
    tunnel_nexthop.vfi_id = vni_entry_p->vfi;
    memcpy(&tunnel_nexthop.local_vtep, &(rvtep_entry_p->s_ip), sizeof(L_INET_AddrIp_T));
    memcpy(&tunnel_nexthop.remote_vtep, &(rvtep_entry_p->ip), sizeof(L_INET_AddrIp_T));
    memcpy(&tunnel_nexthop.nexthop_addr, nexthop_ip_p, sizeof(L_INET_AddrIp_T));
    tunnel_nexthop.nexthop_ifindex = nexthop_if;

    ret = AMTRL3_PMGR_DeleteVxlanTunnelNexthop(SYS_ADPT_DEFAULT_FIB, &tunnel_nexthop);
    if (FALSE == ret)
    {
        VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "Failed to delete a nexthop of vxlan tunnel!\r\n");
    }
#endif

    return TRUE;
}

/* FUNCTION NAME: VXLAN_MGR_ActivateVtep
 * PURPOSE : To active VTEPs after a access port is created in specidied VNI.
 * INPUT   :
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    : Set network port to driver layer after first access port is created.
 */
static BOOL_T VXLAN_MGR_ActivateVtep(UI32_T vni)
{
    VXLAN_OM_RVtep_T    rvtep_entry;
    VXLAN_OM_VNI_T      vni_entry;
    L_INET_AddrIp_T     s_ip;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  l3_if =0, i;
    UI32_T  unit, port, trunk_id;
    UI32_T  mc_vxlan_port = 0;
    UI32_T  uc_vxlan_port = 0;
    UI16_T  udp_port = 0;

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
    memset(&s_ip, 0, sizeof(L_INET_AddrIp_T));

    vni_entry.vni = vni;
    if(VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
    {
        return FALSE;
    }

    VXLAN_OM_GetUdpDstPort(&udp_port);
    rvtep_entry.vni = vni;
    while (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextFloodRVtepByVni(&rvtep_entry))
    {
        /* Check nexthop is existed or not.
         */
        if (rvtep_entry.nexthop_cnt > 0 )
        {
            l3_if = VXLAN_OM_GetL3If(rvtep_entry.vid);

            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "set rvtep ip[%d.%d.%d.%d] \r\n l_vtep_ip[%d.%d.%d.%d] \r\n"
                " vfi [%lu], e_vlan[%u], l3_if[%lu], udp_port[%u] \r\n", rvtep_entry.ip.addr[0],
                rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3],
                rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1], rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3],
                (unsigned long)vni_entry.vfi, rvtep_entry.vid, (unsigned long)l3_if, udp_port);

            /* Set to chip.
             */
            if (TRUE != VXLAN_MGR_CreateFloodUcVtepToChip(vni_entry.vfi, vni_entry.bcast_group,
                    &(rvtep_entry.s_ip), &(rvtep_entry.ip))
               )
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "VXLAN_MGR_CreateFloodUcVtepToChip error!\r\n");
                continue;
            }

            /* Add all nexthops to chip.
             */
            for(i=0; i<rvtep_entry.nexthop_cnt; i++)
            {
                if (FALSE == VXLAN_MGR_AddOneNhopForFloodUcVtepToChip(&vni_entry,
                                &rvtep_entry, &(rvtep_entry.nexthops_ip_ar[i]), rvtep_entry.nexthops_if_ar[i]))
                {
                    VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                       "Failed to add nexthop to chip, vfi [0x%lx], D_IP[%d.%d.%d.%d], S_IP[%d.%d.%d.%d], N_IP[%d.%d.%d.%d], N_IF[%lu]\r\n",
                        vni_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1],
                        rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3],
                        rvtep_entry.s_ip.addr[0],rvtep_entry.s_ip.addr[1],
                        rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3],
                        rvtep_entry.nexthops_ip_ar[i].addr[0], rvtep_entry.nexthops_ip_ar[i].addr[1],
                        rvtep_entry.nexthops_ip_ar[i].addr[2], rvtep_entry.nexthops_ip_ar[i].addr[3],
                        rvtep_entry.nexthops_if_ar[i]);
                    return FALSE;
                }
            }

            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                "RVTEP Created\r\n vfi [0x%lx], IP[%d.%d.%d.%d]\r\n",
                (unsigned long)vni_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1],
                rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
        }
    }

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    rvtep_entry.vni = vni;
    if (VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        /* Check s_ip is existed or not.
         */
        if (memcmp(&rvtep_entry.s_ip, &s_ip, sizeof(L_INET_AddrIp_T)) != 0)
        {
            type = SWCTRL_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
            if (SWCTRL_LPORT_TRUNK_PORT == type)
            {
                port = trunk_id;
            }

            /* Create L3 interface of the vlan for network port if it is not existed.
             */
            l3_if = VXLAN_OM_GetL3If(rvtep_entry.vid);

            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "set rvtep ip[%d.%d.%d.%d] , l_vtep_ip[%d.%d.%d.%d] \r\n"
                " bcast_group[0x%lx] vfi [0x%lx], e_vlan[%u], l3_if[%lu], port[%lu/%lu]\r\n"
                " udp_port[%u], mac[%02X:%02X:%02X:%02X:%02X:%02X] \r\n", rvtep_entry.ip.addr[0],
                rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3],
                rvtep_entry.s_ip.addr[0], rvtep_entry.s_ip.addr[1], rvtep_entry.s_ip.addr[2], rvtep_entry.s_ip.addr[3],
                (unsigned long)vni_entry.bcast_group, (unsigned long)vni_entry.vfi,
                rvtep_entry.vid, (unsigned long)l3_if, (unsigned long)unit, (unsigned long)port, udp_port,
                rvtep_entry.mac_ar[0], rvtep_entry.mac_ar[1], rvtep_entry.mac_ar[2],
                rvtep_entry.mac_ar[3], rvtep_entry.mac_ar[4], rvtep_entry.mac_ar[5]);

#if 0
            /* Set to chip. Create MC VTEP and join multicast group for network port.
             */
            if (TRUE == SWDRV_CreateVTEP(vni_entry.vfi, l3_if, unit, port, udp_port,
                        rvtep_entry.mac_ar, TRUE, FALSE, &(rvtep_entry.s_ip), &(rvtep_entry.ip), &mc_vxlan_port))
            {
                SWDRV_AddVtepIntoMcastGroup(vni_entry.bcast_group, mc_vxlan_port, unit, port, TRUE);
                rvtep_entry.vfi = vni_entry.vfi;
                rvtep_entry.uc_vxlan_port = 0;
                rvtep_entry.mc_vxlan_port = mc_vxlan_port;
                VXLAN_OM_AddFloodMulticast(&rvtep_entry);
            }
            else
            {
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_VTEP_MSG, "SWDRV_CreateVTEP return FALSE!\r\n");
                return FALSE;
            }
#endif
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                "RVTEP Created \r\n vfi [0x%lx], IP[%d.%d.%d.%d]\r\n",
                (unsigned long)vni_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1],
                rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
        }
    }
    return TRUE;
}

/* FUNCTION NAME: VXLAN_MGR_DeactivateVtep
 * PURPOSE : To inactive VTEPs after no access port in specidied VNI.
 * INPUT   :
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    : No access port, remove all Vteps from chip.
 */
static BOOL_T VXLAN_MGR_DeactivateVtep(UI32_T vni)
{
    VXLAN_OM_RVtep_T rvtep_entry;
    VXLAN_OM_VNI_T   vni_entry;
    SWCTRL_Lport_Type_T  type = SWCTRL_LPORT_UNKNOWN_PORT;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id;

    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));

    vni_entry.vni = vni;
    if(VXLAN_OM_GetVniEntry(&vni_entry) != VXLAN_TYPE_RETURN_OK)
    {
        return FALSE;
    }

    rvtep_entry.vni =  vni;
    while (VXLAN_TYPE_RETURN_OK == VXLAN_OM_GetNextFloodRVtepByVni(&rvtep_entry))
    {
        /* Delete orginal UC and MC VTEP in chip for network port.
         */
        if (FALSE == VXLAN_MGR_DestroyFloodUcVtepToChip(&rvtep_entry))
        {
            /* Error happen */
            VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG,
                "VXLAN_MGR_DestroyFloodUcVtepToChip error!\r\n vfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1],
                rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
            continue;
        }

        /* For unicast remote VTEP, VTEP info is recorded by AMTRL3, VXLAN don't need to update OM.
         */
    }
    memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
    rvtep_entry.vni = vni;
    if (VXLAN_OM_GetFloodMulticastByVni(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
    {
        /* Delete orginal VTEP in chip
         */
        if (0 != rvtep_entry.mc_vxlan_port)
        {
            type = SWCTRL_LogicalPortToUserPort(rvtep_entry.lport, &unit, &port, &trunk_id);
            if (SWCTRL_LPORT_TRUNK_PORT == type)
            {
                port = trunk_id;
            }

#if 0
            if (FALSE == SWDRV_DestroyVTEP(rvtep_entry.vfi, unit, port,
                         FALSE, rvtep_entry.mc_vxlan_port))
            {
                /* Error happen */
                VXLAN_MGR_DEBUG_PRINTF(VXLAN_TYPE_DEBUG_FLAG_EVENT_MSG, "Destroy VTEP error!\r\nvfi [0x%lx], D_IP[%d.%d.%d.%d] \r\n",
                    (unsigned long)rvtep_entry.vfi, rvtep_entry.ip.addr[0],rvtep_entry.ip.addr[1], rvtep_entry.ip.addr[2], rvtep_entry.ip.addr[3]);
                return FALSE;
            }
#endif
        }

        /* Update OM.
         */
        rvtep_entry.uc_vxlan_port = 0;
        rvtep_entry.mc_vxlan_port = 0;
        VXLAN_OM_AddFloodMulticast(&rvtep_entry);
    }
    return TRUE;
}

/* FUNCTION NAME: VXLAN_MGR_TaskMain
 * PURPOSE : listen
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : never returns.
 * NOTES   : None.
 */
void VXLAN_MGR_TaskMain()
{
    VXLAN_OM_VxlanHeader_T vxlan_header;
    struct sockaddr_in srvr;
    struct sockaddr_in from;
    size_t fromlen = sizeof(struct sockaddr_in);
    int rc = -1;
    int    recv_bytes;


    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */
    memset(&vxlan_header, 0, sizeof(VXLAN_OM_VxlanHeader_T));

    if ((socket_id = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        SYSFUN_Debug_Printf("%s: cannot create socket!\r\n", __FUNCTION__);
    }

    srvr.sin_family = PF_INET;
    srvr.sin_port = htons(VXLAN_TYPE_DFLT_UDP_DST_PORT);
    srvr.sin_addr.s_addr = 0L;
    if (bind(socket_id,(struct sockaddr*)&srvr, sizeof (srvr)) == -1)
    {
        SYSFUN_Debug_Printf("%s: cannot create socket!\r\n", __FUNCTION__);
    }
#if 0
    while(1)
    {
        recv_bytes = recvfrom(socket_id, &vxlan_header, sizeof(VXLAN_OM_VxlanHeader_T),
                     0, (struct sockaddr *)&from, &fromlen));

        if(0 < recv_bytes)
        {
            /* Parse data and auto create VTEP.
             */
        }

        sleep(2);
    }
#endif
} /* End of VXLAN_MGR_TaskMain() */
