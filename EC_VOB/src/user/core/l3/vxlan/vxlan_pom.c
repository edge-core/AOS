/* MODULE NAME: vxlan_pom.c
 * PURPOSE:
 *    VXLAN POM APIs.
 *
 * NOTES:
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_inet.h"
#include "amtrl3_pom.h"
#include "amtrl3_type.h"
#include "ip_lib.h"
#include "vxlan_pom.h"
#include "vxlan_om.h"
#include "vxlan_type.h"
#include "string.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define VXLAN_POM_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = VXLAN_OM_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    VXLAN_OM_IpcMsg_T *msg_p = (VXLAN_OM_IpcMsg_T*)msgbuf_p->msg_buf;\
    msgbuf_p->cmd = SYS_MODULE_VXLAN;\
    msgbuf_p->msg_size = msg_size;\

#define VXLAN_POM_SEND_WAIT_MSG_P() \
do{\
    if(SYSFUN_OK!=SYSFUN_SendRequestMsg(msgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,\
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size , msgbuf_p))\
    {\
        SYSFUN_Debug_Printf("%s():SYSFUN_SendRequestMsg fail\n", __FUNCTION__);\
        return VXLAN_TYPE_RETURN_ERROR;\
    }\
}while(0)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T msgq_handle;

/* EXPORTED FUNCTION SPECIFICATIONS
 */
/* FUNCTION NAME: VXLAN_POM_InitiateProcessResources
 * PURPOSE : Initiate system resource for VXLAN OM.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_POM_InitiateProcessResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    /* BODY
     */

    /* get the handle of ipc message queues for MLAG OM */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __func__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: VXLAN_POM_GetUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetUdpDstPort(UI16_T *port_no)
{
#if 0
    VXLAN_POM_DECLARE_MSG_P(arg_ui16);
    msg_p->type.cmd = VXLAN_OM_IPC_GETUDPDSTPORT;
    msg_p->data.arg_ui16 = *port_no;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *port_no = msg_p->data.arg_ui16;
    return msg_p->type.ret_ui32;
#endif
    return VXLAN_OM_GetUdpDstPort(port_no);
}

/* FUNCTION NAME: VXLAN_POM_GetRunningUdpDstPort
 * PURPOSE : Get UDP port number.
 * INPUT   : port_no -- UDP port number
 * OUTPUT  : port_no -- UDP port number
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_POM_GetRunningUdpDstPort(UI16_T *port_no)
{
    VXLAN_POM_DECLARE_MSG_P(arg_ui16);
    msg_p->type.cmd = VXLAN_OM_IPC_GETRUNNINGUDPDSTPORT;
    msg_p->data.arg_ui16 = *port_no;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *port_no = msg_p->data.arg_ui16;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetFloodRVtepByVniIp
 * PURPOSE : Get remote VTEP with specified VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetFloodRVtepByVniIp(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_rvtep);
    msg_p->type.cmd = VXLAN_OM_IPC_GETFLOODRVTEPBYVNIIP;
    msg_p->data.arg_rvtep = *rvtep_entry_p;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *rvtep_entry_p = msg_p->data.arg_rvtep;
#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    {
        AMTRL3_TYPE_InetHostRouteEntry_T    vxlan_host;
        UI32_T vfi_index;
        UI32_T i=0;

        for (i=0; i<rvtep_entry_p->nexthop_cnt; i++)
        {
            memset(&vxlan_host, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
            vxlan_host.dst_vid_ifindex = rvtep_entry_p->nexthops_if_ar[i];
            memcpy(&vxlan_host.dst_inet_addr, &(rvtep_entry_p->nexthops_ip_ar[i]), sizeof(L_INET_AddrIp_T));
            if (TRUE == AMTRL3_POM_GetInetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &vxlan_host))
            {
                rvtep_entry_p->nexthops_lport_ar[i] = vxlan_host.lport;
            }
            else
            {
                rvtep_entry_p->lport = 0;
            }
        }
    }
#endif
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetNextFloodRVtepByVni
 * PURPOSE : Get next remote VTEP with VNI.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextFloodRVtepByVni(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_rvtep);
    msg_p->type.cmd = VXLAN_OM_IPC_GETNEXTFLOODRVTEPBYVNI;
    msg_p->data.arg_rvtep = *rvtep_entry_p;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *rvtep_entry_p = msg_p->data.arg_rvtep;
#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    {
        AMTRL3_TYPE_InetHostRouteEntry_T    vxlan_host;
        UI32_T vfi_index;
        UI32_T i=0;

        for (i=0; i<rvtep_entry_p->nexthop_cnt; i++)
        {
            memset(&vxlan_host, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
            vxlan_host.dst_vid_ifindex = rvtep_entry_p->nexthops_if_ar[i];
            memcpy(&vxlan_host.dst_inet_addr, &(rvtep_entry_p->nexthops_ip_ar[i]), sizeof(L_INET_AddrIp_T));
            if (TRUE == AMTRL3_POM_GetInetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &vxlan_host))
            {
                rvtep_entry_p->nexthops_lport_ar[i] = vxlan_host.lport;
            }
            else
            {
                rvtep_entry_p->lport = 0;
            }
        }
    }
#endif
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetNextFloodRVtep
 * PURPOSE : Get next remote VTEP with VNI+IP.
 * INPUT   : rvtep_entry_p->vni
 *           rvtep_entry_p->ip
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextFloodRVtep(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_rvtep);
    msg_p->type.cmd = VXLAN_OM_IPC_GETNEXTFLOODRVTEP;
    msg_p->data.arg_rvtep = *rvtep_entry_p;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *rvtep_entry_p = msg_p->data.arg_rvtep;

#if (VXLAN_TYPE_SET_UC_FLOOD_VTEP_BY_AMTRL3 == TRUE)
    {
        AMTRL3_TYPE_InetHostRouteEntry_T    vxlan_host;
        UI32_T vfi_index;
        UI32_T i=0;

        for (i=0; i<rvtep_entry_p->nexthop_cnt; i++)
        {
            memset(&vxlan_host, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
            vxlan_host.dst_vid_ifindex = rvtep_entry_p->nexthops_if_ar[i];
            memcpy(&vxlan_host.dst_inet_addr, &(rvtep_entry_p->nexthops_ip_ar[i]), sizeof(L_INET_AddrIp_T));
            if (TRUE == AMTRL3_POM_GetInetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB, &vxlan_host))
            {
                rvtep_entry_p->nexthops_lport_ar[i] = vxlan_host.lport;
            }
            else
            {
                rvtep_entry_p->lport = 0;
            }
        }
    }
#endif
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetFloodMulticastByVni
 * PURPOSE : Get remote VTEP with multicast group.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetFloodMulticastByVni(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_rvtep);
    msg_p->type.cmd = VXLAN_OM_IPC_GETFLOODMULTICASTBYVNI;
    msg_p->data.arg_rvtep = *rvtep_entry_p;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *rvtep_entry_p = msg_p->data.arg_rvtep;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetNextFloodMulticast
 * PURPOSE : Get remote VTEP with multicast group.
 * INPUT   : rvtep_entry_p->vni
 * OUTPUT  : rvtep_entry_p.
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextFloodMulticast(VXLAN_OM_RVtep_T *rvtep_entry_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_rvtep);
    msg_p->type.cmd = VXLAN_OM_IPC_GETNEXTFLOODMULTICAST;
    msg_p->data.arg_rvtep = *rvtep_entry_p;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *rvtep_entry_p = msg_p->data.arg_rvtep;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetNextPortVlanVniMapByVni
 * PURPOSE : Get next port (and VLAN) to VNI mapping relationship.
 * INPUT   : vni -- VXLAN ID
 *           lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * OUTPUT  : lport_p -- port ifindex
 *           vid_p -- VLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetNextPortVlanVniMapByVni(UI32_T vni, UI32_T *lport_p, UI16_T *vid_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETNEXTPORTVLANVNIMAPBYVNI;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1 = vni;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2 = *lport_p;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16 = *vid_p;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *lport_p = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2;
    *vid_p = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME - VXLAN_POM_GetVlanVniMapEntry
 * PURPOSE : Get a VLAN-VNI entry.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_POM_GetVlanVniMapEntry(UI16_T vid, UI32_T *vni)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETVLANVNIMAPENTRY;
    msg_p->data.arg_grp_ui16_ui32.arg_ui16 = vid;
    msg_p->data.arg_grp_ui16_ui32.arg_ui32 = *vni;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *vni = msg_p->data.arg_grp_ui16_ui32.arg_ui32;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME - VXLAN_POM_GetNextVlanVniMapEntry
 * PURPOSE : Get next VLAN-VNI entry.
 * INPUT   : vid -- VLAN ID
 *           vni -- VXLAN ID
 * OUTPUT  : vid -- VLAN ID
 *           vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vid=0 to get the first entry
 */
UI32_T VXLAN_POM_GetNextVlanVniMapEntry(UI16_T *vid, UI32_T *vni)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETNEXTVLANVNIMAPENTRY;
    msg_p->data.arg_grp_ui16_ui32.arg_ui16 = *vid;
    msg_p->data.arg_grp_ui16_ui32.arg_ui32 = *vni;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *vid = msg_p->data.arg_grp_ui16_ui32.arg_ui16;
    *vni = msg_p->data.arg_grp_ui16_ui32.arg_ui32;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetVniByVfi
 * PURPOSE : Get VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : None.
 * RETURN  : vni, returns -1 when error
 * NOTES   : None.
 */
I32_T VXLAN_POM_GetVniByVfi(UI32_T vfi)
{
    VXLAN_POM_DECLARE_MSG_P(arg_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETVNIBYVFI;
    msg_p->data.arg_ui32 = vfi;

    VXLAN_POM_SEND_WAIT_MSG_P();

    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME - VXLAN_POM_GetNextVniEntry
 * PURPOSE : Get a next VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : vni=0 to get the first entry
 */
UI32_T VXLAN_POM_GetNextVniEntry(VXLAN_OM_VNI_T *vni_entry_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_vni_entry);
    msg_p->type.cmd = VXLAN_OM_IPC_GETNEXTVNIENTRY;
    memcpy(&msg_p->data.arg_vni_entry, vni_entry_p, sizeof(VXLAN_OM_VNI_T));

    VXLAN_POM_SEND_WAIT_MSG_P();

    memcpy(vni_entry_p, &msg_p->data.arg_vni_entry, sizeof(VXLAN_OM_VNI_T));

    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetSrcIf
 * PURPOSE : Get source interface ifindex of VTEP.
 * INPUT   : None
 * OUTPUT  : ifindex_p -- source interface ifindex
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetSrcIf(UI32_T *ifindex_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETSRCIF;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *ifindex_p = msg_p->data.arg_ui32;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetRunningSrcIf
 * PURPOSE : Get source interface ifindex of VTEP..
 * INPUT   : None
 * OUTPUT  : ifindex_p -- interface ifindex
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES   : None.
 */
SYS_TYPE_Get_Running_Cfg_T VXLAN_POM_GetRunningSrcIf(UI32_T *ifindex_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETRUNNINGSRCIF;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *ifindex_p = msg_p->data.arg_ui32;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetVlanAndVniByVfi
 * PURPOSE : Get VLAN and VNI value from VFI
 * INPUT   : vfi         -- virtual forwarding instance
 * OUTPUT  : vid         -- vlan ID
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetVlanAndVniByVfi(UI32_T vfi, UI16_T *vid, UI32_T *vni)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32_ui32);
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1 = vfi;
    msg_p->type.cmd = VXLAN_OM_IPC_GETVLANANDVNIBYVFI;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16 = *vid;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2 = *vni;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *vid = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16;
    *vni = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetLportOfAccessPort
 * PURPOSE : Get logical port number of access port on specified VLAN.
 * INPUT   : vxlan_port  -- vxlan port nunmber of access port
 *           vid         -- vlan ID
 * OUTPUT  : lport       -- logical port number
 *           vni         -- vxlan ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetLportOfAccessPort(UI32_T vxlan_port, UI16_T vid, UI32_T *lport)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32_ui32);
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1 = vxlan_port;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16 = vid;
    msg_p->type.cmd = VXLAN_OM_IPC_GETLPORTOFACCESSPORT;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2 = *lport;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *lport = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetNextAccessVxlanPort
 * PURPOSE : Get next access VXLAN port.
 * INPUT   : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 *           vxlan_port_p  -- output vxlan port.
 * OUTPUT  : TRUE/FALSE
 * RETURN  : None.
 * NOTES   : None.
 */
BOOL_T VXLAN_POM_GetNextAccessVxlanPort(UI16_T *vid_p, UI32_T *lport_p, UI32_T *vxlan_port_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32_ui32);
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16 = *vid_p;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1 = *lport_p;
    msg_p->type.cmd = VXLAN_OM_IPC_GETNEXTACCESSPORT;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *vid_p = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16;
    *lport_p = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1;
    *vxlan_port_p = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_bool;
}

/* FUNCTION NAME - VXLAN_POM_GetPortVlanVniMapEntry
 * PURPOSE : Get Port+VLAN to VNI mapping.
 * INPUT   : lport -- port ifindex
 *           vid -- VLAN ID
 * OUTPUT  : vni -- VXLAN ID
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_POM_GetPortVlanVniMapEntry(UI32_T lport, UI16_T vid, UI32_T *vni)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32_ui32);
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1 = lport;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16 = vid;
    msg_p->type.cmd = VXLAN_OM_IPC_GETPORTVLANVNIMAPENTRY;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2 = *vni;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *vni = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetAccessVxlanPort
 * PURPOSE : Get access VXLAN port.
 * INPUT   : vid         -- VLAN ID.
 *           lport       -- port number.
 * OUTPUT  : vxlan_port  -- output vxlan port.
 * RETURN  : None.
 * NOTES   : None.
 */
UI32_T VXLAN_POM_GetAccessVxlanPort(UI16_T vid, UI32_T vni)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETACCESSVXLANPORT;
    msg_p->data.arg_grp_ui16_ui32.arg_ui16 = vid;
    msg_p->data.arg_grp_ui16_ui32.arg_ui32 = vni;
    VXLAN_POM_SEND_WAIT_MSG_P();

    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME - VXLAN_POM_GetVniEntry
 * PURPOSE : Get a VNI entry.
 * INPUT   : vni_entry_p
 * OUTPUT  : vni_entry_p
 * RETURN  : VXLAN_TYPE_RETURN_OK /
 *           VXLAN_TYPE_RETURN_ERROR.
 * NOTE    : None
 */
UI32_T VXLAN_POM_GetVniEntry(VXLAN_OM_VNI_T *vni_entry_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_vni_entry);
    msg_p->type.cmd = VXLAN_OM_IPC_GETVNIENTRY;
    memcpy(&msg_p->data.arg_vni_entry, vni_entry_p, sizeof(VXLAN_OM_VNI_T));
    VXLAN_POM_SEND_WAIT_MSG_P();

    memcpy(vni_entry_p, &msg_p->data.arg_vni_entry, sizeof(VXLAN_OM_VNI_T));
    return msg_p->type.ret_ui32;
}

/* FUNCTION NAME: VXLAN_POM_GetVlanNlportOfAccessPort
 * PURPOSE : Get VID and port from access VXLAN port.
 * INPUT   : vxlan_port    -- vxlan port.
 * OUTPUT  : vid_p         -- VLAN ID.
 *           lport_p       -- port number.
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 */
BOOL_T VXLAN_POM_GetVlanNlportOfAccessPort(UI32_T vxlan_port, UI16_T *vid_p, UI32_T *lport_p)
{
    VXLAN_POM_DECLARE_MSG_P(arg_grp_ui16_ui32_ui32);
    msg_p->type.cmd = VXLAN_OM_IPC_GETVLANNLPORTOFACCESSPORT;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_1 = vxlan_port;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16 = *vid_p;
    msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2 = *lport_p;
    VXLAN_POM_SEND_WAIT_MSG_P();

    *vid_p = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui16;
    *lport_p = msg_p->data.arg_grp_ui16_ui32_ui32.arg_ui32_2;
    return msg_p->type.ret_bool;
}

/* LOCAL SUBPROGRAM BODIES
 */
