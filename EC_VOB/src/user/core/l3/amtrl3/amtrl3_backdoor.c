/*------------------------------------------------------------------------
 * Module Name  :   amtrl3_backdoor.c
 *-------------------------------------------------------------------------
 * Purpose      :   This file supports a backdoor for the AMTRL3
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2008
 *-------------------------------------------------------------------------
 */
#include "amtrl3_backdoor.h"

#if (AMTRL3_SUPPORT_ACCTON_BACKDOOR == TRUE)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "backdoor_mgr.h"
#include "l_inet.h"
#include "ip_lib.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_threadgrp.h"
//#include "rule_om.h"
//#include "rule_ctrl.h"
#include "l2_l4_proc_comm.h"
#include "amtrl3_om.h"
#include "amtrl3_mgr.h"
#include "amtrl3_type.h"
#include "amtrl3_backdoor.h"
#include "leaf_2096.h"
#include "leaf_4001.h"
#include "leaf_es3626a.h"


#define KEY_ESC                         27
#define KEY_BACKSPACE                   8
#define KEY_EOS                         0

static L_THREADGRP_Handle_T tg_amtrl3_handle;
static UI32_T               backdoor_amtrl3_id;

/*Simon's dump memory  function*/
#define DUMP_MEMORY_BYTES_PER_LINE 32
#define DUMP_MEMORY(mptr,size) do{\
    int i;\
    unsigned char * ptr=(unsigned char *)mptr;\
    for(i=0;i<size;i++){\
        if(i%DUMP_MEMORY_BYTES_PER_LINE ==0){\
            if(i>0)printf("\r\n");\
            printf("%0.4xH\t",i);\
        }\
        printf("%0.2x", *ptr++);\
    }\
    printf("\r\n");\
}while(0)

/* ---------------------------------------------------------------------
 * common functions
 * --------------------------------------------------------------------- */
static void AMTRL3_BackDoor_Main(void);
static void AMTRL3_BackDoor_DisplayHostEntry(void);
static void AMTRL3_BackDoor_DisplayNotReadyHostEntry(void);
static void AMTRL3_BackDoor_DisplayNetEntry(void);
static void AMTRL3_BackDoor_DisplayDefaultRouteEntry(void);
static void AMTRL3_BackDoor_DisableActionMenu(void);
static void AMTRL3_BackDoor_DisplayMainMenu(void);
static void AMTRL3_BackDoor_DisplayResolvedNetEntry(void);
static void AMTRL3_BackDoor_FunctionUnitTest(void);
static void AMTRL3_BackDoor_SetDefaultRouteTrapToCpuFlag(void);
#if (SYS_CPNT_VXLAN == TRUE)
static void AMTRL3_BackDoor_DisplayVxlanTunnelEntry(void);
static void AMTRL3_BackDoor_DisplayVxlanTunnelNexthopEntry(void);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AMTRL3_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : AMTRL3 backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void AMTRL3_Backdoor_CallBack()
{
    tg_amtrl3_handle = L2_L4_PROC_COMM_GetAmtrl3GroupTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_amtrl3_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_amtrl3_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    BACKDOOR_MGR_Printf("AMTRL3 Backdoor!!\n");
    AMTRL3_BackDoor_Main();

    /* Leave thread group
     */
    L_THREADGRP_Leave(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}


/* Local Subprogram Definition
 */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_BACKDOOR_Init
 * ------------------------------------------------------------------------
 * FUNCTION : This function ititiates the backdoor function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void AMTRL3_BackDoor_Main(void)
{
    int     ch;
    BOOL_T  eof = FALSE;
    char    buf[18];
    char    *terminal;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

    /* BODY
     */
    while(!eof)
    {
        AMTRL3_BackDoor_DisplayMainMenu();

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int)strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");

        switch(ch)
        {
            case 0:
                eof = TRUE;
                break;
            case 1:
                AMTRL3_MGR_PrintDebugMode();
                ch = 0;
                while(TRUE)
                {
                    BACKDOOR_MGR_RequestKeyIn(buf, 15);
                    ch = (int)strtoul(buf, &terminal, 10);
                    if(ch == 0)
                        break;
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                    AMTRL3_MGR_SetDebugFlag(ch);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                    AMTRL3_MGR_PrintDebugMode();
                }
                break;
            case 2:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_BackDoor_DisplayHostEntry();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 3:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_BackDoor_DisplayNetEntry();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
			case 4:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
				AMTRL3_BackDoor_DisplayDefaultRouteEntry();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
				break;
            case 5:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_MGR_DisplayDebugCounters(fib_id);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 6:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_MGR_ClearDebugCounter(fib_id);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 7:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_BackDoor_DisplayNotReadyHostEntry();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 8:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_OM_GetDoulbeLinkListCounter(fib_id);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 9:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_MGR_MeasureNetRoutePerformance();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 10:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_MGR_MeasureHostRoutePerformance();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 11:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_BackDoor_DisableActionMenu();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 12:
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
                AMTRL3_BackDoor_DisplayResolvedNetEntry();
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
                break;
            case 13:
                AMTRL3_BackDoor_FunctionUnitTest();
                break;
#if (SYS_CPNT_VXLAN == TRUE)
            case 14:
                AMTRL3_BackDoor_DisplayVxlanTunnelEntry();
                break;
            case 15:
                AMTRL3_BackDoor_DisplayVxlanTunnelNexthopEntry();
                break;
#endif
            default:
                break;
        } /* end of switch */
    } /* end of while */
    return;
} /* end of AMTRL3_BackDoor_Main() */

/* ------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_BackDoor_DisplayHostEntry
 * ------------------------------------------------------------------------
 * FUNCTION : This function displays one or all host entries
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void AMTRL3_BackDoor_DisplayHostEntry(void)
{
    AMTRL3_OM_HostRouteEntry_T          host_route_entry;
    UI8_T  ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    char   status_str[18] = {0}, host_type[18] = {0};
    UI32_T  cnt= 0;
    char    dummy;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    char   tunnel_type[18] = {0};
#endif

    for (fib_id =0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_route_entry.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
        while (AMTRL3_OM_GetNextHostRouteEntry(action_flags, fib_id, &host_route_entry))
        {
            if(host_route_entry.key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV4 || host_route_entry.key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV4Z)
                L_INET_Ntoa(*(UI32_T*)&host_route_entry.key_fields.dst_inet_addr.addr, ip_str);
            else if(host_route_entry.key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6)
                AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
            else if(host_route_entry.key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z)
            {
                AMTRL3_MGR_Ntoa(&(host_route_entry.key_fields.dst_inet_addr), ip_str);
            }
            switch (host_route_entry.status)
            {
                case HOST_ROUTE_NOT_EXIST:
                    strcpy(status_str, "not_exist");
                    break;
                case HOST_ROUTE_UNRESOLVED:
                    strcpy(status_str, "unresolved");
                    break;
                case HOST_ROUTE_UNREFERENCE:
                    strcpy(status_str, "unreference");
                    break;
                case HOST_ROUTE_HOST_READY:
                    strcpy(status_str, "host_ready");
                    break;
                case HOST_ROUTE_GATEWAY_READY:
                    strcpy(status_str, "gateway_ready");
                    break;
                case HOST_ROUTE_READY_NOT_SYNC:
                    strcpy(status_str, "ready_not_sync");
                    break;
                case HOST_ROUTE_ERROR:
                    strcpy(status_str, "error_state");
                    break;
                case HOST_ROUTE_UNKNOWN:
                    strcpy(status_str, "unknown");
                    break;
                default:
                    strcpy(status_str, "none");
                    break;
            }

            switch(host_route_entry.entry_type)
            {
                case VAL_ipNetToPhysicalExtType_other:
                    strcpy(host_type, "other");
                    break;
                case VAL_ipNetToPhysicalExtType_invalid:
                    strcpy(host_type, "invalid");
                    break;
                case VAL_ipNetToPhysicalExtType_dynamic:
                    strcpy(host_type, "dynamic");
                    break;
                case VAL_ipNetToPhysicalExtType_static:
                    strcpy(host_type, "static");
                    break;
                case VAL_ipNetToPhysicalExtType_local:
                    strcpy(host_type, "local");
                    break;
                case VAL_ipNetToPhysicalExtType_broadcast:
                    strcpy(host_type, "broadcast");
                    break;
                case VAL_ipNetToPhysicalExtType_vrrp:
                    strcpy(host_type, "vrrp");
                    break;
                default:
                    strcpy(host_type, "none");
                    break;
            }

    #if (SYS_CPNT_IP_TUNNEL == TRUE)
            switch(host_route_entry.key_fields.tunnel_entry_type)
            {
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
                    strcpy(tunnel_type, "ISATAP");
                    break;
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
                    strcpy(tunnel_type, "6to4");
                    break;
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
                    strcpy(tunnel_type, "static");
                    break;
                default:
                    strcpy(tunnel_type, "none");
                    break;
            }
    #endif
            BACKDOOR_MGR_Printf("\n");
            BACKDOOR_MGR_Printf("FIB ID: %d\n", host_route_entry.key_fields.fib_id);
            BACKDOOR_MGR_Printf("IP: %s\n",ip_str);
            BACKDOOR_MGR_Printf("lport: %d, uport: %d, vid_ifindex: %d, entry_type: %s\n",
                    (int)host_route_entry.key_fields.lport, (int)host_route_entry.uport,
                    (int)host_route_entry.key_fields.dst_vid_ifindex, host_type);
            //BACKDOOR_MGR_Printf("   ref count %d, cb_ref_count %d, ecmp_ref_count %d, status: %s,\n",
            BACKDOOR_MGR_Printf("    ref_count: %d, ecmp_ref_count: %d, status: %s,\n",
                   (int)host_route_entry.ref_count,
                   //(int)host_route_entry.callback_ref_count,
                   (int)host_route_entry.ecmp_ref_count,
                   status_str);
    #if (SYS_CPNT_PBR == TRUE)
            BACKDOOR_MGR_Printf("    pbr_ref_count = %lu\n", (unsigned long)host_route_entry.pbr_ref_count);
    #endif
            BACKDOOR_MGR_Printf("    in_chip_status: %s, ref_count_in_chip: %lu, Time since last hit: %d sec, \n",
                   (host_route_entry.in_chip_status == TRUE) ? "True" : "False",
                   (unsigned long)host_route_entry.ref_count_in_chip,
                   (int)((SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND)-host_route_entry.hit_timestamp));
            BACKDOOR_MGR_Printf("    MAC = %02x-%02x-%02x-%02x-%02x-%02x,",
                   host_route_entry.key_fields.dst_mac[0],
                   host_route_entry.key_fields.dst_mac[1],
                   host_route_entry.key_fields.dst_mac[2],
                   host_route_entry.key_fields.dst_mac[3],
                   host_route_entry.key_fields.dst_mac[4],
                   host_route_entry.key_fields.dst_mac[5]);

            BACKDOOR_MGR_Printf(" HW_info: %p\n", host_route_entry.hw_info);
    #if (SYS_CPNT_VXLAN == TRUE)
            BACKDOOR_MGR_Printf(" vxlan uc hw_info: %p\n", host_route_entry.vxlan_uc_hw_info);
            BACKDOOR_MGR_Printf(" vxlan mc hw_info: %p\n", host_route_entry.vxlan_mc_hw_info);
            BACKDOOR_MGR_Printf(" vxlan uc ref count: %p\n", host_route_entry.vxlan_uc_ref_count);
            BACKDOOR_MGR_Printf(" vxlan mc ref count: %p\n", host_route_entry.vxlan_mc_ref_count);
    #endif

    #if (SYS_CPNT_IP_TUNNEL == TRUE)
            if(IS_TUNNEL_IFINDEX(host_route_entry.key_fields.dst_vid_ifindex))
            {//dump tunnel extentition
                BACKDOOR_MGR_Printf("Tunnel information:\r\n");
                BACKDOOR_MGR_Printf("    Destination=%lx:%lx:%lx:%lx/%d \n    Nexthop=%lx:%lx:%lx:%lx/%d(vlan %ld) \n    IPv4 SrcIP=%lx:%lx:%lx:%lx/%d(vlan %ld)\n",
                            EXPAND_IPV6(host_route_entry.key_fields.u.ip_tunnel.dest_inet_addr.addr),
                            host_route_entry.key_fields.u.ip_tunnel.dest_inet_addr.preflen,
                            EXPAND_IPV6(host_route_entry.key_fields.u.ip_tunnel.nexthop_inet_addr.addr),
                            host_route_entry.key_fields.u.ip_tunnel.nexthop_inet_addr.preflen,
                            (long)host_route_entry.key_fields.u.ip_tunnel.nexthop_vidifindex,
                            EXPAND_IPV6(host_route_entry.key_fields.u.ip_tunnel.src_inet_addr.addr),
                            host_route_entry.key_fields.u.ip_tunnel.src_inet_addr.preflen,
                            (long)host_route_entry.key_fields.u.ip_tunnel.src_vidifindex
                             );
                BACKDOOR_MGR_Printf("    tunnel_type: %s\n",tunnel_type);
                BACKDOOR_MGR_Printf("    hw tunnel index(l3 intf id):%lu\n", (unsigned long)host_route_entry.hw_tunnel_index);
            }
    #endif /*SYS_CPNT_IP_TUNNEL*/
            cnt++;
            if ((cnt % 10) == 0)
            {
                BACKDOOR_MGR_Printf (" --- More (Q:quit) ----\n");
                dummy = BACKDOOR_MGR_GetChar();
                if ((dummy == 'Q')||(dummy == 'q'))
                    return;
            }

        } /* end of while */
    }

	return;
} /* end of AMTRL3_BackDoor_DisplaysHostEntry() */


static void AMTRL3_BackDoor_DisplayNetEntry(void)
{
    UI8_T   ip_str[L_INET_MAX_IPADDR_STR_LEN]={0}, nhop_str[L_INET_MAX_IPADDR_STR_LEN]={0};
    AMTRL3_OM_NetRouteEntry_T   net_route_entry;
    UI32_T  cnt = 0;
    char    dummy;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char   type[18] = {0};
    char   proto[18] = {0};
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    UI8_T   tunnel_nhop_str[50] = {0};
    UI8_T   tunnel_type[18] = {0};
#endif

    for (fib_id =0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        memset(&net_route_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
        net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;

        while (AMTRL3_OM_GetNextNetRouteEntry(action_flags, fib_id, &net_route_entry))
        {
            if (fib_id != net_route_entry.inet_cidr_route_entry.fib_id)
                fib_id = net_route_entry.inet_cidr_route_entry.fib_id;

            if(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4 || net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV4Z)
                L_INET_Ntoa(*(UI32_T*)&net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr, ip_str);
            else if(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6 || net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest.type == L_INET_ADDR_TYPE_IPV6Z)
                AMTRL3_MGR_Ntoa(&(net_route_entry.inet_cidr_route_entry.inet_cidr_route_dest), ip_str);
            else
            {
                BACKDOOR_MGR_Printf("%s(%d)\n", __FUNCTION__, __LINE__);
                continue;
            }

            if(net_route_entry.inet_cidr_route_entry.inet_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV4 || net_route_entry.inet_cidr_route_entry.inet_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV4Z)
                L_INET_Ntoa(*(UI32_T*)&net_route_entry.inet_cidr_route_entry.inet_cidr_route_next_hop.addr, nhop_str);
            else if(net_route_entry.inet_cidr_route_entry.inet_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV6 || net_route_entry.inet_cidr_route_entry.inet_cidr_route_next_hop.type == L_INET_ADDR_TYPE_IPV6Z)
                AMTRL3_MGR_Ntoa(&(net_route_entry.inet_cidr_route_entry.inet_cidr_route_next_hop), nhop_str);

    #if (SYS_CPNT_IP_TUNNEL == TRUE)
            if(IS_TUNNEL_IFINDEX(net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index))
            {
                L_INET_Ntoa(*(UI32_T*)&net_route_entry.tunnel_nexthop_inet_addr.addr, tunnel_nhop_str);
            }
    #endif

            switch(net_route_entry.inet_cidr_route_entry.inet_cidr_route_type)
            {
                case VAL_ipCidrRouteType_other:
                    strcpy(type,"other");
                    break;
                case VAL_ipCidrRouteType_reject:
                    strcpy(type,"reject");
                    break;
                case VAL_ipCidrRouteType_local:
                    strcpy(type,"local");
                    break;
                case VAL_ipCidrRouteType_remote:
                    strcpy(type,"remote");
                    break;
                default:
                    strcpy(type,"none");
                    break;
            }

            switch(net_route_entry.inet_cidr_route_entry.inet_cidr_route_proto)
            {
                case VAL_ipCidrRouteProto_other:
                    strcpy(proto,"other");
                    break;
                case VAL_ipCidrRouteProto_local:
                    strcpy(proto,"local");
                    break;
                case VAL_ipCidrRouteProto_netmgmt:
                    strcpy(proto,"netmgmt");
                    break;
                case VAL_ipCidrRouteProto_icmp:
                    strcpy(proto,"icmp");
                    break;
                case VAL_ipCidrRouteProto_egp:
                    strcpy(proto,"egp");
                    break;
                case VAL_ipCidrRouteProto_ggp:
                    strcpy(proto,"ggp");
                    break;

                case VAL_ipCidrRouteProto_hello:
                    strcpy(proto,"hello");
                    break;
                case VAL_ipCidrRouteProto_rip:
                    strcpy(proto,"rip");
                    break;
                case VAL_ipCidrRouteProto_isIs:
                    strcpy(proto,"isIs");
                    break;
                case VAL_ipCidrRouteProto_esIs:
                    strcpy(proto,"esIs");
                    break;
                case VAL_ipCidrRouteProto_ciscoIgrp:
                    strcpy(proto,"ciscoIgrp");
                    break;
                case VAL_ipCidrRouteProto_bbnSpfIgp:
                    strcpy(proto,"bbnspfIgp");
                    break;
                case VAL_ipCidrRouteProto_ospf:
                    strcpy(proto,"ospf");
                    break;
                case VAL_ipCidrRouteProto_bgp:
                    strcpy(proto,"bgp");
                    break;
                case VAL_ipCidrRouteProto_idpr:
                    strcpy(proto,"idpr");
                    break;
                case VAL_ipCidrRouteProto_ciscoEigrp:
                    strcpy(proto,"ciscoEigrp");
                    break;
                default:
                    strcpy(proto,"none");
                    break;
            }

    //        BACKDOOR_MGR_Printf("IP: %s, nhop: %s, Ifindex: %d, type: %d, proto: %d, metric %d, \n",
    //                ip_str, nhop_str, (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index,
    //                (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_type,
    //                (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_proto,
    //                (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_metric1);
            BACKDOOR_MGR_Printf("\n");
            BACKDOOR_MGR_Printf("FIB ID: %d\n", net_route_entry.inet_cidr_route_entry.fib_id);
            BACKDOOR_MGR_Printf("IP: %s, \n", ip_str);
            BACKDOOR_MGR_Printf("nhop: %s, \n", nhop_str);
            BACKDOOR_MGR_Printf("Ifindex: %d, type: %s(%d), proto: %s(%d), metric %d, \n", (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index,
                    type,
                    (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_type,
                    proto,
                    (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_proto,
                    (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_metric1);
            BACKDOOR_MGR_Printf("    Prefix length: %d,  ", (int)net_route_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen);
            if (net_route_entry.net_route_status == AMTRL3_OM_NET_ROUTE_UNRESOLVED)
                strcpy((char*)ip_str, "UNRESOLVED");
            else if (net_route_entry.net_route_status == AMTRL3_OM_NET_ROUTE_RESOLVED)
                strcpy((char*)ip_str, "RESOLVED");
            else if (net_route_entry.net_route_status == AMTRL3_OM_NET_ROUTE_READY)
                strcpy((char*)ip_str, "READY");
            else
                strcpy((char*)ip_str, "None");
            BACKDOOR_MGR_Printf("net_route_status: %s,", ip_str);
            if(net_route_entry.flags & AMTRL3_TYPE_FLAGS_ECMP)
            {
                BACKDOOR_MGR_Printf("  ECMP,");
            }
            BACKDOOR_MGR_Printf(" HW_info: %p\n", net_route_entry.hw_info);

    #if (SYS_CPNT_IP_TUNNEL == TRUE)
            switch(net_route_entry.tunnel_entry_type)
            {
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP:
                    strcpy(tunnel_type, "ISATAP");
                    break;
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4:
                    strcpy(tunnel_type, "6to4");
                    break;
                case AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL:
                    strcpy(tunnel_type, "static");
                    break;
                default:
                    strcpy(tunnel_type, "none");
                    break;
            }

            if(IS_TUNNEL_IFINDEX(net_route_entry.inet_cidr_route_entry.inet_cidr_route_if_index))
            {
                BACKDOOR_MGR_Printf("Tunnel information:\n");
                BACKDOOR_MGR_Printf("    Tunnel entry type:%s, next hop:%s \n",tunnel_type,tunnel_nhop_str);
                if((net_route_entry.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4)||
                   (net_route_entry.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP))
                {
                    BACKDOOR_MGR_Printf("    Hit bit:%lx \n", (unsigned long)net_route_entry.tunnel_hit);
                    BACKDOOR_MGR_Printf("    Time since last hit: %d sec, \n",
                        (int)((SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND)-net_route_entry.tunnel_hit_timestamp));
                }
            }
    #endif

            cnt++;
            if ((cnt % 10) == 0)
            {
                BACKDOOR_MGR_Printf (" --- More (Q:quit) ----\n");
                dummy = BACKDOOR_MGR_GetChar();
                if ((dummy == 'Q')||(dummy == 'q'))
                    break;
            }

        } /* end of while */
    }

	return;
} /* end of AMTRL3_BackDoor_DisplayNetEntry() */


static void AMTRL3_BackDoor_DisplayDefaultRouteEntry(void)
{
	UI32_T 	action = 0;
    char    action_str[18] = {0};
    UI32_T  multipath_count = 0;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

    AMTRL3_MGR_GetDefaultRouteInfo(action_flags, fib_id, &action, &multipath_count);

    if (action == SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU)
        strcpy(action_str, "Trap_to_CPU");
    else if (action == SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE)
        strcpy(action_str, "Route");
    else if (action == SYS_CPNT_DEFAULT_ROUTE_ACTION_DROP)
        strcpy(action_str, "Drop");
    else
        strcpy(action_str, "None");
	BACKDOOR_MGR_Printf("Default route action: %s, multipath_count %d\n", action_str, multipath_count);
} /* end of  AMTRL3_BackDoor_DisplayDefaultRouteEntry() */

static void AMTRL3_BackDoor_DisplayNotReadyHostEntry(void)
{
    AMTRL3_OM_HostRouteEntry_T  host_route_entry;
    UI8_T  ip_str[18] = {0};
    char   status_str[18] = {0}, host_type[18] = {0};
    UI32_T  cnt= 0;
    char    dummy;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

    for (fib_id =0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        memset(&host_route_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
        host_route_entry.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
        while (AMTRL3_OM_GetNextHostRouteEntry(action_flags, fib_id, &host_route_entry))
        {
            if ((host_route_entry.status == HOST_ROUTE_HOST_READY)    ||
                (host_route_entry.status == HOST_ROUTE_GATEWAY_READY) ||
                (host_route_entry.status == HOST_ROUTE_READY_NOT_SYNC))
                continue;

            L_INET_Ntoa(*(UI32_T*)&host_route_entry.key_fields.dst_inet_addr.addr, ip_str);

            switch (host_route_entry.status)
            {
                case HOST_ROUTE_UNRESOLVED:
                    strcpy(status_str, "unresolved");
                    break;
                case HOST_ROUTE_UNREFERENCE:
                    strcpy(status_str, "unreference");
                    break;
                case HOST_ROUTE_ERROR:
                    strcpy(status_str, "error_state");
                    break;
                default:
                    strcpy(status_str, "none");
                    break;
            } /* end of switch */

            switch(host_route_entry.entry_type)
            {
                case VAL_ipNetToPhysicalExtType_other:
                    strcpy(host_type, "other");
                    break;
                case VAL_ipNetToPhysicalExtType_invalid:
                    strcpy(host_type, "invalid");
                    break;
                case VAL_ipNetToPhysicalExtType_dynamic:
                    strcpy(host_type, "dynamic");
                    break;
                case VAL_ipNetToPhysicalExtType_static:
                    strcpy(host_type, "static");
                    break;
                case VAL_ipNetToPhysicalExtType_local:
                    strcpy(host_type, "local");
                    break;
                case VAL_ipNetToPhysicalExtType_broadcast:
                    strcpy(host_type, "broadcast");
                    break;
                case VAL_ipNetToPhysicalExtType_vrrp:
                    strcpy(host_type, "vrrp");
                    break;
                default:
                    strcpy(host_type, "none");
                    break;
            }
            BACKDOOR_MGR_Printf("IP: %s, port: %d, vid_ifindex: %d, entry_type: %s, \n",
                    ip_str, (int)host_route_entry.key_fields.lport, (int)host_route_entry.key_fields.dst_vid_ifindex,
                    host_type);

            //BACKDOOR_MGR_Printf("   ref count %d, cb_ref_count %d, ecmp_ref_count %d, status: %s,\n",
            BACKDOOR_MGR_Printf("    ref_count: %d, ecmp_ref_count: %d, status: %s,\n",
                   (int)host_route_entry.ref_count,
                   //(int)host_route_entry.callback_ref_count,
                   (int)host_route_entry.ecmp_ref_count,
                   status_str);
            BACKDOOR_MGR_Printf("   in_chip_status: %s, Time since last hit: %d sec, \n",
                   (host_route_entry.in_chip_status == TRUE) ? "True" : "False",
                   (int)((SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND)-host_route_entry.hit_timestamp));
            BACKDOOR_MGR_Printf("    MAC = %02x-%02x-%02x-%02x-%02x-%02x,",
                   host_route_entry.key_fields.dst_mac[0],
                   host_route_entry.key_fields.dst_mac[1],
                   host_route_entry.key_fields.dst_mac[2],
                   host_route_entry.key_fields.dst_mac[3],
                   host_route_entry.key_fields.dst_mac[4],
                   host_route_entry.key_fields.dst_mac[5]);

            BACKDOOR_MGR_Printf(" HW_info: %p\n", host_route_entry.hw_info);
            cnt++;
            if ((cnt % 10) == 0)
            {
                BACKDOOR_MGR_Printf (" --- More (Q:quit) ----\n");
                dummy = BACKDOOR_MGR_GetChar();
                if ((dummy == 'Q')||(dummy == 'q'))
                    break;
            }
        } /* end of while */
    }

	return;
} /* end of AMTRL3_BackDoor_DisplayNotReadyHostEntry() */

#if (SYS_CPNT_VXLAN == TRUE)
static void AMTRL3_BackDoor_DisplayVxlanTunnelEntry(void)
{
    AMTRL3_OM_VxlanTunnelEntry_T vxlan_entry;
    UI8_T  ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

    memset(&vxlan_entry, 0, sizeof(AMTRL3_OM_VxlanTunnelEntry_T));
    while (AMTRL3_OM_GetNextVxlanTunnelEntry(fib_id, &vxlan_entry))
    {
        BACKDOOR_MGR_Printf("vfi_id = %lu\r\n", vxlan_entry.vfi_id);
        AMTRL3_MGR_Ntoa(&vxlan_entry.local_vtep, ip_str);
        BACKDOOR_MGR_Printf("local_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&vxlan_entry.remote_vtep, ip_str);
        BACKDOOR_MGR_Printf("remote_vtep = %s\r\n", ip_str);
        BACKDOOR_MGR_Printf("is mc = %u\r\n", vxlan_entry.is_mc);
        BACKDOOR_MGR_Printf("udp port = %u\r\n", vxlan_entry.udp_port);
        BACKDOOR_MGR_Printf("bcast group = %u\r\n", vxlan_entry.bcast_group);
        BACKDOOR_MGR_Printf("uc_vxlan_port = %lx\r\n", (unsigned long)vxlan_entry.uc_vxlan_port);
        BACKDOOR_MGR_Printf("mc_vxlan_port = %lx\r\n", (unsigned long)vxlan_entry.mc_vxlan_port);
        BACKDOOR_MGR_Printf("uc_hw_info = %lx\r\n", (unsigned long)vxlan_entry.uc_hw_info);
        BACKDOOR_MGR_Printf("mc_hw_info = %lx\r\n", (unsigned long)vxlan_entry.mc_hw_info);
    }

	return;
}

static void AMTRL3_BackDoor_DisplayVxlanTunnelNexthopEntry(void)
{
    AMTRL3_OM_VxlanTunnelNexthopEntry_T vxlan_nexthop;
    UI8_T  ip_str[L_INET_MAX_IPADDR_STR_LEN] = {0};
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

    memset(&vxlan_nexthop, 0, sizeof(AMTRL3_OM_VxlanTunnelNexthopEntry_T));
    while (AMTRL3_OM_GetNextVxlanTunnelNexthopEntry(fib_id, &vxlan_nexthop))
    {
        BACKDOOR_MGR_Printf("vfi_id = %lu\r\n", vxlan_nexthop.vfi_id);
        AMTRL3_MGR_Ntoa(&vxlan_nexthop.local_vtep, ip_str);
        BACKDOOR_MGR_Printf("local_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&vxlan_nexthop.remote_vtep, ip_str);
        BACKDOOR_MGR_Printf("remote_vtep = %s\r\n", ip_str);
        AMTRL3_MGR_Ntoa(&vxlan_nexthop.nexthop_addr, ip_str);
        BACKDOOR_MGR_Printf("nexthop addr = %s\r\n", ip_str);
        BACKDOOR_MGR_Printf("nexthop ifindex = %lu\r\n", vxlan_nexthop.nexthop_ifindex);
        BACKDOOR_MGR_Printf("uc bind status = %u\r\n", vxlan_nexthop.is_uc_binding);
        BACKDOOR_MGR_Printf("mc bind status = %u\r\n", vxlan_nexthop.is_mc_binding);
    }

	return;
}
#endif

static void AMTRL3_BackDoor_DisableActionMenu(void)
{
    int     ch;
    BOOL_T  eof = FALSE;
    char    buf[18];
    char    *terminal;
    UI8_T   v4_mode, v6_mode;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

    /* BODY
     */
    while(!eof)
    {
        BACKDOOR_MGR_Printf(" \n");
        BACKDOOR_MGR_Printf(" 0. Exit\n");
        BACKDOOR_MGR_Printf(" 1. Set ARP Action\n");
        BACKDOOR_MGR_Printf(" 2. Set Scan hit bit mode\n");
        BACKDOOR_MGR_Printf(" 3. Set Delete Host Route mode\n");
        BACKDOOR_MGR_Printf(" 4. Set Supernet Stauts \n");
        //BACKDOOR_MGR_Printf(" 5. Set Software Forwarding Status \n");
        BACKDOOR_MGR_Printf("     select = ");

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int)strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");

        switch(ch)
        {
            case 0:
                eof = TRUE;
                break;
            case 1:
                BACKDOOR_MGR_Printf ("\n");
                BACKDOOR_MGR_Printf ("0. Exit\n");
                BACKDOOR_MGR_Printf ("1: Normal Operation \n");
                BACKDOOR_MGR_Printf ("2: Disable ARP Request Unresolved Entry.\n");
                BACKDOOR_MGR_Printf ("3: Disable ARP Request Gateway Entry.\n");
                BACKDOOR_MGR_Printf ("4: Disable All ARP Request Action.\n");
                BACKDOOR_MGR_Printf("     select =");
                BACKDOOR_MGR_RequestKeyIn(buf, 15);
                ch = (int)strtoul(buf, &terminal, 10);
                BACKDOOR_MGR_Printf("\n");
                if (ch == 0)
                    break;
                AMTRL3_MGR_SetArpRequestFeature(ch);
                break;
            case 2:
                AMTRL3_MGR_SetHostRouteScanningOperation();
                break;
            case 3:
                AMTRL3_MGR_SetDeleteHostRouteOperation();
                break;
            case 4:
                v4_mode = 0;
                if(AMTRL3_MGR_GetSuperNettingStatus(action_flags, fib_id, &v4_mode,&v6_mode))
                    BACKDOOR_MGR_Printf("Current Supernet Status: Ipv4:%s, Ipv6:(not support)\n",
                           (v4_mode == AMTRL3_TYPE_SUPER_NET_ENABLE) ? "enable" : "disable");
                           //(v6_mode == AMTRL3_TYPE_SUPER_NET_ENABLE) ? "enable" : "disable");
                BACKDOOR_MGR_Printf("Enter Supernet Status:  (1) Enable, (2) Disable:  " );

                BACKDOOR_MGR_RequestKeyIn(buf, 15);
                ch = (int)strtoul(buf, &terminal, 10);
                BACKDOOR_MGR_Printf("\n");
                if (ch == 0)
                    break;
                if (ch == 1)
                    v4_mode = AMTRL3_TYPE_SUPER_NET_ENABLE;
                else if(ch == 2)
                    v4_mode = AMTRL3_TYPE_SUPER_NET_DISABLE;
                else
                    break;
                AMTRL3_MGR_SetSuperNettingStatus(action_flags, fib_id, v4_mode, v6_mode);
                if (AMTRL3_MGR_GetSuperNettingStatus(action_flags, fib_id, &v4_mode,&v6_mode))
                    BACKDOOR_MGR_Printf("Current Supernet Status: Ipv4:%s, Ipv6:(not support)\n",
                           (v4_mode == AMTRL3_TYPE_SUPER_NET_ENABLE) ? "enable" : "disable");
                break;
            #if 0
            case 5:
                mode = 0;
                if (AMTRL3_MGR_GetSoftwareForwardingStatus(&mode))
                    BACKDOOR_MGR_Printf("Cuurent Software Forwarding Status: %s\n",
                           (mode == AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE) ? "enable" : "disable");
                BACKDOOR_MGR_Printf("Enter Software Forwarding Status:  (1) Enable, (2) Disable:  " );

                BACKDOOR_MGR_RequestKeyIn(buf, 15);
                ch = (int)strtoul(buf, &terminal, 10);
                BACKDOOR_MGR_Printf("\n");
                if (ch == 0)
                    break;
                if (ch == 1)
                    mode = AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE;
                else if (ch == 2)
                    mode = AMTRL3_TYPE_SOFTWARE_FORWARDING_DISABLE;
                else
                    break;
                AMTRL3_MGR_SetSoftwareForwardingStatus(mode);
                if (AMTRL3_MGR_GetSoftwareForwardingStatus(&mode))
                    BACKDOOR_MGR_Printf("Current Supernet Status: %s\n",
                           (mode == AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE) ? "enable" : "disable");
                break;
            #endif
            default:
                break;
        } /* end of switch */
    } /* end of while */
    return;
} /* end of AMTRL3_BackDoor_DisableActionMenu() */

static void AMTRL3_BackDoor_DisplayMainMenu(void)
{
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf(" 0. Exit\n");
    BACKDOOR_MGR_Printf(" 1. Set AL3 debug flag\n");
    BACKDOOR_MGR_Printf(" 2. Display Host Entry\n");
    BACKDOOR_MGR_Printf(" 3. Display Net entry\n");
    BACKDOOR_MGR_Printf(" 4. Display default route entry\n");
    BACKDOOR_MGR_Printf(" 5. Show Debug Counter\n");
    BACKDOOR_MGR_Printf(" 6. Clear Debug Counters\n");
    BACKDOOR_MGR_Printf(" 7. Display Not-Ready Host Entry\n");
    BACKDOOR_MGR_Printf(" 8. Display OM DLL Counter\n");
    BACKDOOR_MGR_Printf(" 9. Measure Net route performance\n");
    BACKDOOR_MGR_Printf("10. Measure Host route performance\n");
    BACKDOOR_MGR_Printf("11. Disable Action Menu\n");
    BACKDOOR_MGR_Printf("12. Display Resolved Net entry\n");
    BACKDOOR_MGR_Printf("13. Begin function unit test\n");
#if (SYS_CPNT_VXLAN == TRUE)
    BACKDOOR_MGR_Printf("14. Display Vxlan Tunnel Entry\n");
    BACKDOOR_MGR_Printf("15. Display Vxlan Tunnel Nexthop Entry\n");
#endif
    BACKDOOR_MGR_Printf("     select = ");
    return;
} /* end of AMTRL3_BackDoor_DisplayMainMenu() */

static void AMTRL3_BackDoor_DisplayResolvedNetEntry(void)
{
    UI8_T   ip_str[18], nhop_str[18];
    AMTRL3_OM_ResolvedNetRouteEntry_T   net_route_entry;
    UI32_T  cnt = 0;
    char    dummy;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4 | AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;

    /* BODY
     */
    memset(&net_route_entry, 0, sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T));

    for (fib_id =0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        while (AMTRL3_OM_GetNextResolvedNetRouteEntry(action_flags, fib_id, &net_route_entry))
        {
            L_INET_Ntoa(*(UI32_T*)&net_route_entry.inet_cidr_route_dest.addr, ip_str);
            L_INET_Ntoa(*(UI32_T*)&net_route_entry.inet_cidr_route_next_hop.addr, nhop_str);

            BACKDOOR_MGR_Printf("FIB ID: %d\n", net_route_entry.fib_id);
            BACKDOOR_MGR_Printf("IP: %s, nhop: %s, inv-prefix length: %d, policy[0]: %d, policy[1]: %d\n",
                    ip_str, nhop_str,
                    (UI32_T)net_route_entry.inverse_prefix_length,
                    (UI32_T)net_route_entry.inet_cidr_route_policy[0],
                    (UI32_T)net_route_entry.inet_cidr_route_policy[1]);
            cnt++;
            if ((cnt % 10)==0)
            {
                BACKDOOR_MGR_Printf (" --- More (Q:quit) ----\n");
                dummy = BACKDOOR_MGR_GetChar();
                if ((dummy == 'Q')||(dummy == 'q'))
                    break;
            } /* end of if */
        } /* end of while */
    }
	return;
} /* end of AMTRL3_BackDoor_DisplayResolvedNetEntry() */

static void AMTRL3_BackDoor_DisplayFunctionUnitTestMenu(void)
{
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf(" 0. Exit\n");
    //BACKDOOR_MGR_Printf(" 1. Register callback in host entry (not support currently)\n");
    //BACKDOOR_MGR_Printf(" 2. Unregister callback in host entry (not support currently)\n");
    BACKDOOR_MGR_Printf(" 1. Add IPv4 host route\n");
    BACKDOOR_MGR_Printf(" 2. Delete IPv4 host route\n");
    BACKDOOR_MGR_Printf(" 3. Add IPv4 net route\n");
    BACKDOOR_MGR_Printf(" 4. Delete IPv4 net route\n");
    BACKDOOR_MGR_Printf(" 5. Add IPv6 host route\n");
    BACKDOOR_MGR_Printf(" 6. Delete IPv6 host route\n");
    BACKDOOR_MGR_Printf(" 7. Add IPv6 static net route\n");
    BACKDOOR_MGR_Printf(" 8. Delete IPv6 static net route\n");
    BACKDOOR_MGR_Printf(" 9. Add IPv6 local net route\n");
    BACKDOOR_MGR_Printf("10. Delete IPv6 local net route\n");
    BACKDOOR_MGR_Printf("11. Add ECMP route multi-path\n");
    BACKDOOR_MGR_Printf("12. Delete ECMP route\n");
    BACKDOOR_MGR_Printf("13. Callbacks Test\n");
    BACKDOOR_MGR_Printf("14. Set Chip Flag for debuging\n");
    BACKDOOR_MGR_Printf("15. Set ARP age out time(seconds)\n");
    BACKDOOR_MGR_Printf("     select = ");
    return;
}

/* for removal warning */
#if 0
static int AMTRL3_BACKDOOR_GetLine(char s[ ], int lim)
{
    int c, i;

    for(i = 0;
        (i < lim - 1) && ((c = BACKDOOR_MGR_GetChar()) != 0) && (c != '\n');
        ++i)
    {
        if(c == KEY_BACKSPACE)
        {
            if(i > 0)
            {
                BACKDOOR_MGR_Printf("%c", c);
                i -= 1;
                continue;
            }
        }

        s[i] = c;
        BACKDOOR_MGR_Printf("%c", c);
    }

    s[i] = '\0';

    return i;
}
#endif

static int AMTRL3_BACKDOOR_AtoIP(UI8_T *s, UI8_T *ip)
{
        UI8_T token[20];
        int   i,j;  /* i for s[]; j for token[] */
        int   k;    /* k for ip[] */

        UI8_T temp[4];

        i = 0;
        j = 0;
        k = 0;

        while (s[i] != '\0')
        {
            if (s[i] == '.')
            {
                token[j] = '\0';
                if (strlen((char *)token) < 1 || strlen((char *)token) > 3 ||
                    atoi((char *)token) < 0 || atoi((char *)token) > 255)
                {
                    return 0;
                }
                else if (k >= 4)
                {
                    return 0;
                }
                else
                {
                    temp[k++] =(UI8_T)atoi((char *)token);
                    i++; j = 0;
                }
            }
            else if (!(s[i] >= '0' && s[i] <= '9'))
            {
                return 0;
            }
            else
            {
                token[j++] = s[i++];
            }

        } /* while */

        token[j] = '\0';
        if (strlen((char *)token) < 1 || strlen((char *)token) > 3 ||
            atoi((char *)token) < 0 || atoi((char *)token) > 255)
        {
            return 0;
        }
        else if (k != 3)
        {
            return 0;
        }

        temp[k]=(UI8_T)atoi((char *)token);

        ip[0] = temp[0];
        ip[1] = temp[1];
        ip[2] = temp[2];
        ip[3] = temp[3];

        return 1;

}
// Convert Hexidecimal ASCII digits to integer
static int AMTRL3_BACKDOOR_AHtoI(char *token)
{
  int result=0, value_added=0, i=0;

   do {
   if((*(token+i) >= '0') && (*(token+i) <= '9'))
    value_added = (int) (*(token+i) - 48);
   else if((*(token+i) >= 'a') && (*(token+i) <= 'f'))
    value_added = (int) (*(token+i) - 87);
   else if((*(token+i) >= 'A') && (*(token+i) <= 'F'))
    value_added = (int) (*(token+i) - 55);
   else
    return -1;
   result = result * 16 + value_added;
   i++;
  } while(*(token+i) != '\0');

   if(result < 0 || result > 255)
    return -1;
   return result;
}

static int AMTRL3_BACKDOOR_AtoIPV6(char *s, UI8_T *ip)
{
        UI8_T token[50];
        int   i,j;  /* i for s[]; j for token[] */
        int   k,l;  /* k for ip[]; l for copying coutner */

        UI8_T temp[20];

        i = 0;
        j = 0;
        k = 0;
    	l = 0;

        while (s[i] != '\0')
        {
            if ((s[i] == ':') || (j == 2))
            {
		 token[j] = '\0';

                if (strlen((char *)token) < 1 || strlen((char *)token) > 2 ||
                    AMTRL3_BACKDOOR_AHtoI((char *)token) < 0 || AMTRL3_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
                    return 0;
                else if (k >= 16)  // Too many digits
                    return 0;
                else // token is ready
                {
                    temp[k++] =(UI8_T)AMTRL3_BACKDOOR_AHtoI((char *)token);
		    if(s[i] == ':')
                     i++;
		    j = 0;
                }
            }
            else if ((s[i] < '0' || s[i] > '9') && (s[i] < 'a' || s[i] > 'f') && (s[i] < 'A' || s[i] > 'F'))
                return 0;
            else
                token[j++] = s[i++];
        } /* while */

        token[j] = '\0';

        if (strlen((char *)token) < 1 || strlen((char *)token) > 2 ||
            AMTRL3_BACKDOOR_AHtoI((char *)token) < 0 || AMTRL3_BACKDOOR_AHtoI((char *)token) > 255) // Invalid Token
            return 0;
        else if (k >= 16)  // Too many digits
            return 0;


        temp[k]=(UI8_T)AMTRL3_BACKDOOR_AHtoI((char *)token);

        for(l=0;l<16;l++)
         ip[l] = temp[l];

        return 1;

}

static int AMTRL3_BACKDOOR_GetIP(UI32_T * IPaddr)
{
    int ret = 0;
    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    UI8_T   buffer[20] = {0};

    //if(AMTRL3_BACKDOOR_GetLine((char*)buffer, 20) > 0)
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    ret = AMTRL3_BACKDOOR_AtoIP(buffer, temp_ip);

    if(ret == 0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IPv4 address\n");
        return  ret;
    }

    //BACKDOOR_MGR_Printf("\n");

    return  1;
}

static int AMTRL3_BACKDOOR_GetIPV6(UI32_T * IPaddr)
{
    int ret = 0;
    UI8_T   *temp_ip = (UI8_T *)IPaddr;
    char   buffer[50] = {0};

    //if(AMTRL3_BACKDOOR_GetLine((char*)buffer, 20) > 0)
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 50);

    if(strlen(buffer) != 39)
     ret = 0;
    else
     ret = AMTRL3_BACKDOOR_AtoIPV6(buffer, temp_ip);

    if(ret == 0)
    {
        BACKDOOR_MGR_Printf("\nYou entered an invalid IPv6 address\n");
        return  ret;
    }

    //BACKDOOR_MGR_Printf("\n");

    return  1;
}



/* for removal warning */
#if 0
static UI32_T AMTRL3_BACKDOOR_AtoUI(UI8_T * s, int radix)
{
    int i;
    unsigned long n = 0;

    for (i = 0; s[i] == ' ' || s[i] == '\n' || s[i] == '\t'; i++)
        ;       /* skip white space */

    if (s[i] == '+' || s[i] == '-')
    {
        i++;    /* skip sign */
    }

    if (radix == 10)
    {
        for (n = 0; s[i] >= '0' && s[i] <= '9'; i++)
        {
            n = 10 * n + s[i] - '0';
        }
    }
    else if (radix == 16)
    {
        if ( (s[i] == '0') && (s[i+1] == 'x' || s[i+1] == 'X') ) /* Charles,*/
           i=i+2;                                                /* To skip the "0x" or "0X" */


        for (n = 0;
            (s[i] >= '0' && s[i] <= '9') ||
            (s[i] >= 'A' && s[i] <= 'F') ||
            (s[i] >= 'a' && s[i] <= 'f');
            i++)
        {
            if (s[i] >= '0' && s[i] <= '9')
            {
                n = 16 * n + s[i] - '0';
            }
            else if (s[i] >= 'A' && s[i] <= 'F')
            {
                n = 16 * n + s[i] - 'A'+ 10;
            }
            else if (s[i] >= 'a' && s[i] <= 'f')
            {
                n = 16 * n + s[i] - 'a'+ 10;
            }
        }
    }

    return (n);
}
#endif

#if 0
static void AMTRL3_BackDoor_TestRegisterHostEntryCallback(void)
{
    UI32_T nhop_ip;
    AMTRL3_MGR_IPHostRouteEntry_T nhop_entry;
    UI8_T   buffer[10] = {0};
    AMTRL3_MGR_NexthopStatusChangeCallback_T cb;
    void *cookie = NULL;

    BACKDOOR_MGR_Printf("\n\r Input nexthop ip address: ");
    AMTRL3_BACKDOOR_GetIP(&nhop_ip);

    BACKDOOR_MGR_Printf("\n\r Input callback function pointer: ");
    if(AMTRL3_BACKDOOR_GetLine(buffer, 10) > 0)
    {
        cb = (AMTRL3_MGR_NexthopStatusChangeCallback_T)AMTRL3_BACKDOOR_AtoUI(buffer, 10);
    }

    BACKDOOR_MGR_Printf("\n\r Input cookie: ");
    if(AMTRL3_BACKDOOR_GetLine(buffer, 10) > 0)
    {
        cookie = (void *)AMTRL3_BACKDOOR_AtoUI(buffer, 10);
    }

    AMTRL3_MGR_GetNexthopStatusAndRegisterCallback(nhop_ip, cb, cookie, &nhop_entry);
}

static void AMTRL3_BackDoor_TestUnregisterHostEntryCallback(void)
{
    UI32_T nhop_ip;
    UI8_T   buffer[10] = {0};
    AMTRL3_MGR_NexthopStatusChangeCallback_T cb;
    void *cookie = NULL;

    BACKDOOR_MGR_Printf("\n\r Input nexthop ip address: ");
    AMTRL3_BACKDOOR_GetIP(&nhop_ip);

    BACKDOOR_MGR_Printf("\n\r Input callback function pointer: ");
    if(AMTRL3_BACKDOOR_GetLine(buffer, 10) > 0)
    {
        cb = (AMTRL3_MGR_NexthopStatusChangeCallback_T)AMTRL3_BACKDOOR_AtoUI(buffer, 10);
    }

    BACKDOOR_MGR_Printf("\n\r Input cookie: ");
    if(AMTRL3_BACKDOOR_GetLine(buffer, 10) > 0)
    {
        cookie = (void *)AMTRL3_BACKDOOR_AtoUI(buffer, 10);
    }

    AMTRL3_MGR_UnregisterNexthopStatusChangeCallback(nhop_ip, cb, cookie);
}
#endif

static int AMTRL3_BackDoor_TestAddHostRoute(void)
{
    AMTRL3_TYPE_InetHostRouteEntry_T host_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4, i;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    UI32_T type = VAL_ipNetToPhysicalExtType_dynamic;
    char    *terminal;

    memset(&host_route_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    host_route_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
    BACKDOOR_MGR_Printf("\n\r Input dest host address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(host_route_entry.dst_inet_addr.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input dst if index(1001~4094): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    host_route_entry.dst_vid_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n\r Input lport(1~24 for dynamic, 65535 for static and local): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    host_route_entry.lport = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    /* dst mac */
    for( i = 0; i < SYS_ADPT_MAC_ADDR_LEN; i++)
    {
        BACKDOOR_MGR_Printf("\n\r Input dst_mac[%d]: ", i + 1);
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
        host_route_entry.dst_mac[i] = strtoul((char *)buffer, &terminal, 16);
    }

    BACKDOOR_MGR_Printf("\n\r Input host route type");
    BACKDOOR_MGR_Printf("(other-1, invalid-2, dynamic-3, ");
    BACKDOOR_MGR_Printf("static-4, local-101, broadcast-102, vrrp-103): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    type = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    AMTRL3_MGR_SetHostRoute(action_flags, fib_id, &host_route_entry, NULL, type);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestAddIPV6HostRoute(void)
{
    AMTRL3_TYPE_InetHostRouteEntry_T host_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6, i;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    UI32_T type = VAL_ipNetToPhysicalExtType_dynamic;
    char    *terminal;

    memset(&host_route_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    host_route_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(host_route_entry.dst_inet_addr.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input dst if index(1001~4094): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    host_route_entry.dst_vid_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n\r Input lport(1~24 for dynamic, 65535 for static and local): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    host_route_entry.lport = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    /* dst mac */
    for( i = 0; i < SYS_ADPT_MAC_ADDR_LEN; i++)
    {
        BACKDOOR_MGR_Printf("\n\r Input dst_mac[%d]: ", i + 1);
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 10);
        host_route_entry.dst_mac[i] = strtoul((char *)buffer, &terminal, 16);
    }

    BACKDOOR_MGR_Printf("\n\r Input host route type");
    BACKDOOR_MGR_Printf("(other-1, invalid-2, dynamic-3, ");
    BACKDOOR_MGR_Printf("static-4, local-101, broadcast-102, vrrp-103): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    type = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    AMTRL3_MGR_SetHostRoute(action_flags, fib_id, &host_route_entry, NULL, type);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestDeleteHostRoute(void)
{
    AMTRL3_TYPE_InetHostRouteEntry_T host_route_entry;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    memset(&host_route_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    host_route_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
    BACKDOOR_MGR_Printf("\n\r Input dest host address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(host_route_entry.dst_inet_addr.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n");
    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    AMTRL3_MGR_DeleteHostRoute(action_flags, fib_id, &host_route_entry);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestDeleteIPV6HostRoute(void)
{
    AMTRL3_TYPE_InetHostRouteEntry_T host_route_entry;
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    memset(&host_route_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    host_route_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(host_route_entry.dst_inet_addr.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n");
    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    AMTRL3_MGR_DeleteHostRoute(action_flags, fib_id, &host_route_entry);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestAddNetRoute(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    BACKDOOR_MGR_Printf("\n\r Input dest net address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~32): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;
    BACKDOOR_MGR_Printf("\n\r Input next hop address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input if index(1001~4094): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_if_index = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_remote;
    net_route_entry.partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_netmgmt;

    net_route_entry.partial_entry.inet_cidr_route_metric1 = 5;
    net_route_entry.partial_entry.inet_cidr_route_age = (SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);

    BACKDOOR_MGR_Printf("\n\r ECMP route? (1:yes / 0:no): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        UI32_T ecmp;
        ecmp = (UI32_T)strtoul((char *)buffer, &terminal, 10);
        if(ecmp == 1)
            action_flags |= AMTRL3_TYPE_FLAGS_ECMP;
    }

    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    if(action_flags & AMTRL3_TYPE_FLAGS_ECMP)
        AMTRL3_MGR_AddECMPRouteOnePath(action_flags, fib_id, &net_route_entry);
    else
        AMTRL3_MGR_SetInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestAddIPV6NetRoute(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input if index(1001~4094): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_if_index = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_remote;
    net_route_entry.partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_netmgmt;

    net_route_entry.partial_entry.inet_cidr_route_metric1 = 5;
    net_route_entry.partial_entry.inet_cidr_route_age = (SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);

    BACKDOOR_MGR_Printf("\n\r ECMP route? (1:yes / 0:no): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        UI32_T ecmp;
        ecmp = (UI32_T)strtoul((char *)buffer, &terminal, 10);
        if(ecmp == 1)
            action_flags |= AMTRL3_TYPE_FLAGS_ECMP;
    }

    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    if(action_flags & AMTRL3_TYPE_FLAGS_ECMP)
        AMTRL3_MGR_AddECMPRouteOnePath(action_flags, fib_id, &net_route_entry);
    else
        AMTRL3_MGR_SetInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);

    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);

    return 0;
}

static int AMTRL3_BackDoor_TestAddIPV6LocalNetRoute(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input if index(1001~4094): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_if_index = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_local;
    net_route_entry.partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_local;

    net_route_entry.partial_entry.inet_cidr_route_metric1 = 5;
    net_route_entry.partial_entry.inet_cidr_route_age = (SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);

    BACKDOOR_MGR_Printf("\n\r ECMP route? (1:yes / 0:no): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        UI32_T ecmp;
        ecmp = (UI32_T)strtoul((char *)buffer, &terminal, 10);
        if(ecmp == 1)
            action_flags |= AMTRL3_TYPE_FLAGS_ECMP;
    }

    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    if(action_flags & AMTRL3_TYPE_FLAGS_ECMP)
        AMTRL3_MGR_AddECMPRouteOnePath(action_flags, fib_id, &net_route_entry);
    else
        AMTRL3_MGR_SetInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);

    /* Release execution permission from the thread group handler if necessary
     */

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);

    return 0;
}

static int AMTRL3_BackDoor_TestDeleteNetRoute(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    BACKDOOR_MGR_Printf("\n\r Input dest net address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(1~32): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;
    BACKDOOR_MGR_Printf("\n\r Input next hop address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r ECMP route? (1:yes / 0:no): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        UI32_T ecmp;
        ecmp = (UI32_T)strtoul((char *)buffer, &terminal, 10);
        if(ecmp == 1)
            action_flags |= AMTRL3_TYPE_FLAGS_ECMP;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    if(action_flags & AMTRL3_TYPE_FLAGS_ECMP)
        AMTRL3_MGR_DeleteECMPRouteOnePath(action_flags, fib_id, &net_route_entry);
    else
        AMTRL3_MGR_DeleteInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestDeleteIPV6NetRoute(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(1~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr)))
        return 1;
    net_route_entry.partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_remote;
    net_route_entry.partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_netmgmt;


    BACKDOOR_MGR_Printf("\n\r ECMP route? (1:yes / 0:no): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        UI32_T ecmp;
        ecmp = (UI32_T)strtoul((char *)buffer, &terminal, 10);
        if(ecmp == 1)
            action_flags |= AMTRL3_TYPE_FLAGS_ECMP;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    if(action_flags & AMTRL3_TYPE_FLAGS_ECMP)
        AMTRL3_MGR_DeleteECMPRouteOnePath(action_flags, fib_id, &net_route_entry);
    else
        AMTRL3_MGR_DeleteInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestDeleteIPV6LocalNetRoute(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV6;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input dest host IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(1~64): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    net_route_entry.partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV6;
    BACKDOOR_MGR_Printf("\n\r Input next hop IPv6 address(x:x:x:x:x:x:x:x): ");
    if(!AMTRL3_BACKDOOR_GetIPV6((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_next_hop.addr)))
        return 1;
    net_route_entry.partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_local;
    net_route_entry.partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_local;


    BACKDOOR_MGR_Printf("\n\r ECMP route? (1:yes / 0:no): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    {
        UI32_T ecmp;
        ecmp = (UI32_T)strtoul((char *)buffer, &terminal, 10);
        if(ecmp == 1)
            action_flags |= AMTRL3_TYPE_FLAGS_ECMP;
    }

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    if(action_flags & AMTRL3_TYPE_FLAGS_ECMP)
        AMTRL3_MGR_DeleteECMPRouteOnePath(action_flags, fib_id, &net_route_entry);
    else
        AMTRL3_MGR_DeleteInetCidrRouteEntry(action_flags, fib_id, &net_route_entry);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestAddECMPRoutMultiPath(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry[8];
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;
    UI32_T  num = 1, dest_ip = 0, pfxlen = 0, i = 0;


    memset(net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T) * 8);
    BACKDOOR_MGR_Printf("\n\r Input dest net address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&dest_ip))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(0~32): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n\r Input number of next hops (1~8) ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    num = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    if(num < 1 || num > 8)
        num = 0;

    for(i = 0; i < num; i++)
    {
        net_route_entry[i].partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
        memcpy(&net_route_entry[i].partial_entry.inet_cidr_route_dest.addr, &dest_ip, 4);
        net_route_entry[i].partial_entry.inet_cidr_route_pfxlen = pfxlen;

        BACKDOOR_MGR_Printf("\n\r Input next hop[%d] (x.x.x.x): ", i + 1);
        if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(net_route_entry[i].partial_entry.inet_cidr_route_next_hop.addr)))
            return 1;
        net_route_entry[i].partial_entry.inet_cidr_route_next_hop.type = L_INET_ADDR_TYPE_IPV4;

        net_route_entry[i].partial_entry.inet_cidr_route_if_index = 1001;
        net_route_entry[i].partial_entry.inet_cidr_route_type = VAL_ipCidrRouteType_remote;
        net_route_entry[i].partial_entry.inet_cidr_route_proto = VAL_ipCidrRouteProto_netmgmt;
        net_route_entry[i].partial_entry.inet_cidr_route_metric1 = 5;
        net_route_entry[i].partial_entry.inet_cidr_route_age = (SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    }

    action_flags |= AMTRL3_TYPE_FLAGS_ECMP;

    BACKDOOR_MGR_Printf("\n");

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    AMTRL3_MGR_AddECMPRouteMultiPath(action_flags, fib_id, net_route_entry, num);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static int AMTRL3_BackDoor_TestDeleteECMPRoute(void)
{
    AMTRL3_TYPE_InetCidrRouteEntry_T net_route_entry;
    UI8_T   buffer[20] = {0};
    UI32_T action_flags = AMTRL3_TYPE_FLAGS_IPV4;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
    char    *terminal;

    memset(&net_route_entry, 0, sizeof(AMTRL3_TYPE_InetCidrRouteEntry_T));
    net_route_entry.partial_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
    BACKDOOR_MGR_Printf("\n\r Input dest net address(x.x.x.x): ");
    if(!AMTRL3_BACKDOOR_GetIP((UI32_T*)&(net_route_entry.partial_entry.inet_cidr_route_dest.addr)))
        return 1;

    BACKDOOR_MGR_Printf("\n\r Input prefix length(1~32): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    net_route_entry.partial_entry.inet_cidr_route_pfxlen = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    action_flags |= AMTRL3_TYPE_FLAGS_ECMP;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);
    AMTRL3_MGR_DeleteECMPRoute(action_flags, fib_id, &net_route_entry);
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return 0;
}

static void AMTRL3_BackDoor_SetEgressObjectFlag(void)
{
    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_SetEgressObjectFlag();
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_BackDoor_SetECMPTableSameEgressFlag(void)
{
    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_SetECMPTableSameEgressFlag();
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_BackDoor_SetChipFullFlag(void)
{
    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_SetChipFullFlag();
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_BackDoor_SetDefaultRouteTrapToCpuFlag(void)
{
    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_SetDefaultRouteTrapToCpuFlag();
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_BackDoor_SetLoaclHostTrapMyIpPktFlag(void)
{
    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_SetLoaclHostTrapMyIpPktFlag();
    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_Backdoor_TestForwardingTrunkMemberAdd(void)
{
    UI8_T   buffer[20] = {0};
    char    *terminal;
    UI32_T  trunk_ifindex, member_ifindex;

    BACKDOOR_MGR_Printf("\n\r Input Trunk ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    trunk_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n\r Input Member ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    member_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_ForwardingUPortAddToTrunk_CallBack(trunk_ifindex, member_ifindex);

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_Backdoor_TestForwardingTrunkMemberDelete(void)
{
    UI8_T   buffer[20] = {0};
    char    *terminal;
    UI32_T  trunk_ifindex, member_ifindex;

    BACKDOOR_MGR_Printf("\n\r Input Trunk ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    trunk_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n\r Input Member ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    member_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_ForwardingTrunkMemberDelete_CallBack(trunk_ifindex, member_ifindex);

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_Backdoor_TestTrunkMemberNotOperUp(void)
{
    UI8_T   buffer[20] = {0};
    char    *terminal;
    UI32_T  trunk_ifindex, member_ifindex;

    BACKDOOR_MGR_Printf("\n\r Input Trunk ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    trunk_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n\r Input Member ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    member_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_ForwardingTrunkMemberToNonForwarding_CallBack(trunk_ifindex, member_ifindex);

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_Backdoor_TestMacAgeOut(void)
{
    UI8_T   buffer[20] = {0};
    char    *terminal;
    UI32_T  num, i, j;
    AMTRL3_TYPE_AddrEntry_T addr_buff[5];

    memset(addr_buff, 0, sizeof(AMTRL3_TYPE_AddrEntry_T) * 5);

    BACKDOOR_MGR_Printf("\n\r Number of MAC age out: (1~5)");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    num = (UI32_T)strtoul((char *)buffer, &terminal, 10);
    if(num < 1 || num > 5)
        num = 0;

    for(i = 0; i < num; i++)
    {
        BACKDOOR_MGR_Printf("\n\r addr_buff[%d].vid:", i + 1);
        BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
        addr_buff[i].vid = (UI32_T)strtoul((char *)buffer, &terminal, 10);
        for(j = 0; j < SYS_ADPT_MAC_ADDR_LEN; j++)
        {
            BACKDOOR_MGR_Printf("\n\r addr_buff[%d].mac[%d]:", i + 1, j + 1);
            BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
            addr_buff[i].mac[j] = strtoul((char *)buffer, &terminal, 16);
        }
    }

    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_MacAgingOut_CallBack(num, addr_buff);

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_Backdoor_TestMACTableDeleteByPort(void)
{
    UI8_T   buffer[20] = {0};
    char    *terminal;
    UI32_T  ifindex, reason;

    BACKDOOR_MGR_Printf("\n\r Input Lport ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    reason = 4;

    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_MACTableDeleteByPort_CallBack(ifindex, reason);

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_Backdoor_TestMACTableDeleteByVIDnPort(void)
{
    UI8_T   buffer[20] = {0};
    char    *terminal;
    UI32_T  vid, lport_ifindex;

    BACKDOOR_MGR_Printf("\n\r Input VLAN ID: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    vid = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    BACKDOOR_MGR_Printf("\n\r Input Lport ifindex: ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    lport_ifindex = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_MACTableDeleteByVIDnPort_CallBack(vid, lport_ifindex);

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_BackDoor_TestSetARPAgeoutTime(void)
{
    UI8_T   buffer[20] = {0};
    char    *terminal;
    UI32_T  time = 0, action_flags = AMTRL3_TYPE_FLAGS_IPV4|AMTRL3_TYPE_FLAGS_IPV6, fib_id = 0;

    BACKDOOR_MGR_Printf("\n\r Input Age Out Time(seconds): ");
    BACKDOOR_MGR_RequestKeyIn((char *)buffer, 20);
    time = (UI32_T)strtoul((char *)buffer, &terminal, 10);

    L_THREADGRP_Execution_Request(tg_amtrl3_handle, backdoor_amtrl3_id);

    AMTRL3_MGR_SetIpNetToPhysicalEntryTimeout(action_flags, fib_id, time, time);

    L_THREADGRP_Execution_Release(tg_amtrl3_handle, backdoor_amtrl3_id);
    return;
}

static void AMTRL3_BackDoor_DisplayCallbackMenu(void)
{
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf(" 0. Exit\n");
    BACKDOOR_MGR_Printf(" 1. Trunk member add\n");
    BACKDOOR_MGR_Printf(" 2. Trunk member delete\n");
    BACKDOOR_MGR_Printf(" 3. Trunk member not operate up\n");
    BACKDOOR_MGR_Printf(" 4. MAC age out\n");
    BACKDOOR_MGR_Printf(" 5. MAC delete by port\n");
    BACKDOOR_MGR_Printf(" 6. MAC delete by VID and port\n");
    BACKDOOR_MGR_Printf("     select = ");
    return;
}

static void AMTRL3_BackDoor_TestCallbacks(void)
{
    int     ch;
    BOOL_T  eof = FALSE;
    char    buf[18];
    char    *terminal;

    /* BODY
     */
    while(!eof)
    {
        AMTRL3_BackDoor_DisplayCallbackMenu();

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int)strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");

        switch(ch)
        {
            case 0:
                eof = TRUE;
                break;
            case 1:
                AMTRL3_Backdoor_TestForwardingTrunkMemberAdd();
                break;
            case 2:
                AMTRL3_Backdoor_TestForwardingTrunkMemberDelete();
                break;
            case 3:
                AMTRL3_Backdoor_TestTrunkMemberNotOperUp();
                break;
            case 4:
                AMTRL3_Backdoor_TestMacAgeOut();
                break;
            case 5:
                AMTRL3_Backdoor_TestMACTableDeleteByPort();
                break;
            case 6:
                AMTRL3_Backdoor_TestMACTableDeleteByVIDnPort();
                break;
            default:
                break;
        }
    }
    return;
}

static void AMTRL3_BackDoor_DisplayChipFlagMenu(void)
{
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf(" 0. Exit\n");
    BACKDOOR_MGR_Printf(" 1. Support Egress object\n");
    BACKDOOR_MGR_Printf(" 2. Use same egress for ECMP table\n");
    BACKDOOR_MGR_Printf(" 3. Chip full flag\n");
    BACKDOOR_MGR_Printf(" 4. Use Default route trap to CPU\n");
    BACKDOOR_MGR_Printf(" 5. Use Loacl host to trap my IP pkt\n");
    BACKDOOR_MGR_Printf("     select = ");
    return;
}
static void AMTRL3_BackDoor_TestChipFlagSet(void)
{
    int     ch;
    BOOL_T  eof = FALSE;
    char    buf[18];
    char    *terminal;

    /* BODY
     */
    while(!eof)
    {
        AMTRL3_BackDoor_DisplayChipFlagMenu();

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int)strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");

        switch(ch)
        {
            case 0:
                eof = TRUE;
                break;
            case 1:
                AMTRL3_BackDoor_SetEgressObjectFlag();
                break;
            case 2:
                AMTRL3_BackDoor_SetECMPTableSameEgressFlag();
                break;
            case 3:
                AMTRL3_BackDoor_SetChipFullFlag();
                break;
            case 4:
                AMTRL3_BackDoor_SetDefaultRouteTrapToCpuFlag();
                break;
            case 5:
                AMTRL3_BackDoor_SetLoaclHostTrapMyIpPktFlag();
                break;
            default:
                break;
        }
    }
    return;
}

static void AMTRL3_BackDoor_FunctionUnitTest(void)
{
    int     ch;
    BOOL_T  eof = FALSE;
    char    buf[18];
    char    *terminal;

    /* BODY
     */
    while(!eof)
    {
        AMTRL3_BackDoor_DisplayFunctionUnitTestMenu();

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int)strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");

        switch(ch)
        {
            case 0:
                eof = TRUE;
                break;
            //case 1:
                //AMTRL3_BackDoor_TestRegisterHostEntryCallback();
                //break;
            //case 2:
                //AMTRL3_BackDoor_TestUnregisterHostEntryCallback();
                //break;
            case 1:
                AMTRL3_BackDoor_TestAddHostRoute();
                break;
            case 2:
                AMTRL3_BackDoor_TestDeleteHostRoute();
                break;
            case 3:
                AMTRL3_BackDoor_TestAddNetRoute();
                break;
            case 4:
                AMTRL3_BackDoor_TestDeleteNetRoute();
                break;
            case 5:
		AMTRL3_BackDoor_TestAddIPV6HostRoute();
		        break;
            case 6:
                AMTRL3_BackDoor_TestDeleteIPV6HostRoute();
                break;
            case 7:
                AMTRL3_BackDoor_TestAddIPV6NetRoute();
                break;
            case 8:
                AMTRL3_BackDoor_TestDeleteIPV6NetRoute();
                break;
            case 9:
                AMTRL3_BackDoor_TestAddIPV6LocalNetRoute();
                break;
            case 10:
                AMTRL3_BackDoor_TestDeleteIPV6LocalNetRoute();
                break;
            case 11:
                AMTRL3_BackDoor_TestAddECMPRoutMultiPath();
                break;
            case 12:
                AMTRL3_BackDoor_TestDeleteECMPRoute();
                break;
            case 13:
                AMTRL3_BackDoor_TestCallbacks();
                break;
            case 14:
                AMTRL3_BackDoor_TestChipFlagSet();
                break;
            case 15:
                AMTRL3_BackDoor_TestSetARPAgeoutTime();
                break;
            default:
                break;
        } /* end of switch */
    } /* end of while */
    return;
}


// Test check in
#endif
