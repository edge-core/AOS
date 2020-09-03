/* Module Name: netcfg_mgr_ip.c
 * Purpose:
 *      NETCFG_MGR_IP provides l3ipvlan configuration management access-point for
 *      upper layer.
 *
 * Notes:
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       01/22/2008 --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

/* INCLUDE FILE DECLARATIONS
 */
#define BACKDOOR_OPEN
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "netcfg_type.h"
#include "netcfg_mgr_ip.h"
#include "netcfg_om_ip.h"
#include "netcfg_om_ip_private.h"
#include "ip_mgr.h"
#include "eh_mgr.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "vlan_om.h"
#include "dhcp_pmgr.h"
#include "dhcp_type.h"
#include "dhcp_mgr.h"
#include "stktplg_pom.h"
#include "lldp_pmgr.h"
#include "ip_lib.h"
#include "l_rstatus.h"
#include "l_stdlib.h"
#include "leaf_es3626a.h"
#include "ip_mgr.h"
#include "sys_callback_mgr.h"
#if (SYS_CPNT_UDP_HELPER == TRUE)
#include "udphelper_pmgr.h"
#endif
#ifdef  BACKDOOR_OPEN
#include "backdoor_mgr.h"
#endif
#include "l_threadgrp.h"
#include "netcfg_proc_comm.h"
#include "netcfg_mgr_nd.h"
#include "netcfg_mgr_rip.h"
#include "amtr_pmgr.h"
#include "ipal_types.h"
#include "ipal_if.h"
#include "ipal_vrrp.h"
#include "swdrvl3.h"
#if (SYS_CPNT_OSPF == TRUE)
#include "netcfg_mgr_ospf.h"
#endif
/* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
#include "ospf_pmgr.h"
#include "ospf6_pmgr.h"
#endif

#if (SYS_CPNT_BGP == TRUE)
#include "bgp_pmgr.h"
#endif

/*fuzhimin, 20090218*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#endif
#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_pmgr.h"
#endif
#include "amtrl3_pmgr.h"
#include "amtrl3_type.h"


#include "route_mgr.h"
#include "netcfg_om_route.h"
#include "netcfg_mgr_route.h" /* for NETCFG_MGR_ROUTE_FLAGS_IPV6 */

/*fuzhimin,20090218,end*/
#include "l_stdlib.h"

#if (SYS_CPNT_ROUTING == TRUE) && (SYS_CPNT_IPV6 == TRUE)
#include "l4_pmgr.h" /* L4_PMGR_TrapLinkLocalToCPU */
#endif

#if (SYS_CPNT_PBR == TRUE)
#include "netcfg_mgr_pbr.h"
#endif

/* work around patch to pass TAHI IPv6 ready logo test */
#if (SYS_CPNT_IPV6 == TRUE)
#if (SYS_CPNT_SWDRVL3 == TRUE)
#include "dev_swdrvl3_pmgr.h"
#endif
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define INTERFACE_STATUS_UP     TRUE
#define INTERFACE_STATUS_DOWN   FALSE
#define DFLT_IPV6_ENABLE            SYS_DFLT_IPV6_ENABLE
#define DFLT_IPV6_ADDR_AUTOCONFIG   SYS_DFLT_IPV6_ADDR_AUTOCONFIG

#define L_RSTATUS_NOT_EXIST      0  /* state         */ /* Not RFC value */
#define L_RSTATUS_ACTIVE         1  /* state, action */
#define L_RSTATUS_NOTINSERVICE   2  /*        action */
#define L_RSTATUS_NOTREADY       3  /* state         */
#define L_RSTATUS_CREATEANDGO    4  /*        action */
#define L_RSTATUS_CREATEANDWAIT  5  /*        action */
#define L_RSTATUS_DESTROY        6  /*        action */
#define L_RSTATUS_SET_OTHER_COLUMN_COLUMN      7  /*        action */ /* Not RFC value */
#define L_RSTATUS_ALLOCATED      8


#define NETCFG_MGR_IP_FLAG_IPV4     BIT_0               /* Operation on ipv4 entries */
#define NETCFG_MGR_IP_FLAG_IPV6     BIT_1               /* Operation on ipv6 entries */
#define NETCFG_MGR_IP_FLAG_IPV4V6   (BIT_0 | BIT_1)     /* Operation on ipv4 & ipv6 entries */


/* MACRO FUNCTION DECLARATIONS
 */
/*Simon's debug function*/
#define IPCFG_DEBUG_FLAG_BIT_DEBUG  0x01
#define IPCFG_DEBUG_FLAG_BIT_INFO   0x02
#define IPCFG_DEBUG_FLAG_BIT_NOTE   0x04
#define IPCFG_DEBUG_FLAG_BIT_TUNNEL 0x08
#define UNUSED __attribute__ ((__unused__))
#define DBGprintf(format,args...)  ((debug_flag&IPCFG_DEBUG_FLAG_BIT_DEBUG) == FALSE)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((debug_flag&IPCFG_DEBUG_FLAG_BIT_INFO)==FALSE)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((debug_flag&IPCFG_DEBUG_FLAG_BIT_NOTE)==FALSE)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define TUNNELprintf(format,args...) ((debug_flag&IPCFG_DEBUG_FLAG_BIT_TUNNEL)==FALSE)?0:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)


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


#define RAW_STATUS_TO_STRING(_sta_) (\
(_sta_==0)?"NotExist":\
(_sta_==VAL_netConfigStatus_2_active)?"Active":\
(_sta_==VAL_netConfigStatus_2_notInService)?"NotInService":\
(_sta_==VAL_netConfigStatus_2_notReady)?"NotReady":\
(_sta_==VAL_netConfigStatus_2_createAndGo)?"CreateAndGo":\
(_sta_==VAL_netConfigStatus_2_createAndWait)?"CreateAndWait":\
(_sta_==VAL_netConfigStatus_2_destroy)?"Destroy":\
(_sta_==7)?"OtherConfig":\
"unknown")





/*END Simon's debug function*/

#define DEBUG_PRINTF(args...) do { \
if (debug_flag) \
    printf(args); \
} while(0);

#define NETCFG_MGR_IP_UpdateDiffCounter(old_counter,new_counter,different)   \
    if ((new_counter) >= (old_counter))                                 \
    {                                                                   \
        (different) = (new_counter) - (old_counter);                    \
    }                                                                   \
    else                                                                \
    {                                                                   \
        (different) = 0xffffffff - (old_counter) + (new_counter) + 1;   \
    }

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T NETCFG_MGR_IP_CreateL3If(UI32_T vid);
static UI32_T NETCFG_MGR_IP_DeleteL3If(UI32_T vid);
static UI32_T NETCFG_MGR_IP_SetIpAddressMode(UI32_T ifindex,
                                            NETCFG_TYPE_IP_ADDRESS_MODE_T ip_addr_mode);
static UI32_T NETCFG_MGR_IP_InternalSetInetRif(NETCFG_OM_IP_InetRifConfig_T *rif_config);
static UI32_T NETCFG_MGR_IP_InternalSetRifActive(NETCFG_TYPE_InetRifConfig_T *rif_config_p,
                                                    NETCFG_OM_IP_InetRifConfig_T *rif_config);
static UI32_T NETCFG_MGR_IP_DeleteAllRifByIfindex(UI8_T action_flags, UI32_T ifindex);
static BOOL_T NETCFG_MGR_IP_CheckRifSemantic(void *rif_om_ptr);
static void NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceCreate(UI32_T ifindex,
                            UI32_T mtu, UI32_T bandwidth, UI16_T if_flags);
static void NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceDelete(UI32_T ifindex);
static void NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceCreate(UI32_T ifindex,
                            UI16_T if_flags);
static void NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceDelete(UI32_T ifindex);
static void NETCFG_MGR_IP_SignalAllProtocolRifCreate(NETCFG_TYPE_InetRifConfig_T *rif_p);
static void NETCFG_MGR_IP_SignalAllProtocolRifDelete(NETCFG_TYPE_InetRifConfig_T *rif_p);
static void NETCFG_MGR_IP_SignalAllProtocolRifActive(NETCFG_TYPE_InetRifConfig_T *rif_p);
static void NETCFG_MGR_IP_SignalAllProtocolRifDown(NETCFG_TYPE_InetRifConfig_T *rif_p);
static void NETCFG_MGR_IP_SignalInterfaceStatus(UI32_T ifindex, BOOL_T status);
static UI32_T NETCFG_MGR_IP_DeletePrePrimaryIPv4Rif(NETCFG_TYPE_InetRifConfig_T *rif_config);

static void NETCFG_MGR_IP_DeleteAllInterface(void);
static void NETCFG_MGR_IP_CreateDefaultLoopbackInterface();
static UI32_T NETCFG_MGR_IP_SetSingleManagementVlan(UI32_T ifindex);
static BOOL_T NETCFG_MGR_IP_IsReachMaxNbrOfRif(void);
#if (SYS_CPNT_IPV6 == TRUE)
static UI32_T NETCFG_MGR_IP_IPv6_SetAutoLinkLocal(UI32_T ifindex);
static UI32_T NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal(UI32_T ifindex);
static UI32_T NETCFG_MGR_IP_IPv6_GetIntIdFromMAC(UI8_T* int_id, UI8_T* mac_addr);
static UI32_T NETCFG_MGR_IP_IPv6_IPv6AddressEUI64(UI8_T* addr, UI16_T preflen, UI8_T* mac_addr);
#if 0
static UI32_T NETCFG_MGR_IP_IPv6_IPv6AddressIsatapEUI64(UI8_T* addr, UI16_T preflen, UI8_T* ipv4_addr);
static UI32_T NETCFG_MGR_IP_IPv6_IPv6AddressModifiedEUI64(UI8_T* addr, UI16_T preflen, UI8_T* ipv4_addr);
static UI32_T NETCFG_MGR_IP_IPv6_TunnelAddressEUI64(UI8_T* addr, UI16_T preflen, UI32_T sourc_vlan_ifindex);
#endif
#endif
static BOOL_T NETCFG_MGR_FSM(UI32_T action, UI32_T *state, BOOL_T (*active_check_fun)(void*rec), void *rec) ;

#if(SYS_CPNT_VIRTUAL_IP == TRUE)
static UI32_T NETCFG_MGR_IP_InternalSetVirtualRifActive(NETCFG_TYPE_InetRifConfig_T *rif_config_p);
static UI32_T NETCFG_MGR_IP_InternalSetVirtualRifDestroy(NETCFG_TYPE_InetRifConfig_T *rif_config_p);
#endif
static BOOL_T NETCFG_MGR_Is_ZeroAddr(L_INET_AddrIp_T* inetaddr)
{
    L_INET_AddrIp_T  zero_addr;
    memset(&zero_addr,0,sizeof(zero_addr));
    if(memcmp(inetaddr, &zero_addr, sizeof(zero_addr))==0)
        return TRUE;
    else
        return FALSE;
}
/*fuzhimin,20090218*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
static void NETCFG_OM_IP_DeleteEth0IpAddr();
#endif
/*fuzhimin,20090218,end*/
#if (SYS_CPNT_CRAFT_PORT == TRUE)
static UI32_T NETCFG_MGR_IP_DeleteAllCraftInterfaceInetAddress();
static void NETCFG_MGR_IP_SyncKernelCraftInterfaceInetAddress();
#endif

#if (SYS_CPNT_CRAFT_DHCLIENT == TRUE)
/* to start or to stop dhclient running on CRAFT port
 */
static void NETCFG_MGR_IP_SetCraftRunDHCP(BOOL_T to_start)
{
    FILE *fd=fopen(DHCLIENT_CRAFT_PID_FILE, "r");
    if (to_start == TRUE)
    {
        if (fd == NULL) /* not running yet */
        {
            system("dhclient -nw -q -pf "DHCLIENT_CRAFT_PID_FILE" CRAFT &");
        }
        else /* already running */
            fclose(fd);
    }
    else /* to stop */
    {
        if (fd != NULL) /* it's still running */
        {
//            system("dhclient -nw -q -pf "DHCLIENT_CRAFT_PID_FILE" -r CRAFT &"); /* the release will cuase the user set IP to be deleted */
            fclose(fd);
            system("kill -15 `cat "DHCLIENT_CRAFT_PID_FILE" `"); /* to terminate process */
            system("rm -f "DHCLIENT_CRAFT_PID_FILE); /* indicating no dhclient doing DHCP */
        }
    }
    return;
}
#else
#define NETCFG_MGR_IP_SetCraftRunDHCP(x) ;
#endif
static void NETCFG_MGR_IP_BackDoorMain(void);

/* STATIC VARIABLE DECLARATIONS
 */
SYSFUN_DECLARE_CSC


static UI8_T debug_flag = 0;

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME : NETCFG_MGR_IP_CreateL3If
 * PURPOSE:
 *      Create a L3 interface.
 *
 * INPUT:
 *      vid  -- vlan id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *      None.
 */
static UI32_T NETCFG_MGR_IP_CreateL3If(UI32_T vid)
{
    UI8_T cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T ifindex;
    UI32_T vid_ifindex;
    UI32_T if_status;
    NETCFG_TYPE_L3_Interface_T vlan_intf;
    IP_MGR_Interface_T ip_intf;
    UI32_T drv_l3_intf_index = SWDRVL3_HW_INFO_INVALID;
    UI32_T rc;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (VLAN_OM_IsVlanExisted(vid) == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_CREATE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "vid");
        return NETCFG_TYPE_INVALID_ARG;
    }

    /* Check if it's a created interface */
    memset(&vlan_intf, 0, sizeof (vlan_intf));
    VLAN_OM_ConvertToIfindex(vid, &ifindex);
    vlan_intf.ifindex = ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&vlan_intf) == NETCFG_TYPE_OK)
        return NETCFG_TYPE_OK;

    /* check OM table full in advance */
    if(NETCFG_OM_IP_IsL3InterfaceTableFull())
        return NETCFG_TYPE_TABLE_FULL;

    if (VLAN_PMGR_VlanChangeToL3Type(vid, &vid_ifindex, &if_status) != VLAN_MGR_RETURN_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_CREATE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 "VLAN_PMGR_VlanChangeToL3Type");
        DBGprintf("Fail to VlanChangeToL3Type");
        return NETCFG_TYPE_FAIL;
    }

    memset(&vlan_intf, 0, sizeof (vlan_intf));
    memset(&ip_intf, 0, sizeof (IP_MGR_Interface_T));
    STKTPLG_POM_GetLocalUnitBaseMac(cpu_mac);

    vlan_intf.ifindex = ifindex;
    vlan_intf.u.physical_intf.mtu = SYS_ADPT_IF_MTU;
    vlan_intf.iftype = VLAN_L3_IP_IFTYPE;
    memcpy(vlan_intf.u.physical_intf.hw_addr, cpu_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(vlan_intf.u.physical_intf.logical_mac, cpu_mac, SYS_ADPT_MAC_ADDR_LEN);
    /* TODO:
     * vai, should set a proper default value for bandwidth
     */
    vlan_intf.u.physical_intf.bandwidth = 0;

#if 0
    This in in linux src, include\linux\if.h,
    #define IFF_UP      0x1     /* interface is up      */
    #define IFF_RUNNING 0x40        /* interface RFC2863 OPER_UP    */
#endif
    /* Fake code */
    vlan_intf.u.physical_intf.if_flags |= IFF_UP;
    if (if_status == VAL_ifOperStatus_up)
        vlan_intf.u.physical_intf.if_flags |= IFF_RUNNING;

    ip_intf.ifindex = vlan_intf.ifindex;
    ip_intf.u.physical_intf.mtu = vlan_intf.u.physical_intf.mtu;
    ip_intf.u.physical_intf.if_flags = vlan_intf.u.physical_intf.if_flags;
    ip_intf.u.physical_intf.bandwidth = vlan_intf.u.physical_intf.bandwidth;
    memcpy(ip_intf.u.physical_intf.logical_mac, vlan_intf.u.physical_intf.hw_addr, SYS_ADPT_MAC_ADDR_LEN);


    if((rc = IP_MGR_CreateInterface(&ip_intf, &drv_l3_intf_index)) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                NETCFG_MGR_IP_CREATE_VLAN_INTERFACE_FUN_NO,
                EH_TYPE_MSG_FAILED_TO_ADD,
                SYSLOG_LEVEL_ERR,
                "IP_MGR_CreateInterface");
        DBGprintf("Fail to IP_MGR_CreateInterface, rc=%lu", (unsigned long)rc);
        return rc;
    }

    /* save drv_l3_intf_index */
    vlan_intf.drv_l3_intf_index = drv_l3_intf_index;

    if (NETCFG_OM_IP_CreateL3Interface(&vlan_intf) != NETCFG_TYPE_OK)
    {
        /* recover previous action */
        IP_MGR_DeleteInterface(ifindex);

        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_CREATE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO_ADD,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_OM_IP_CreateInterface");
        DBGprintf("fail to OM_IP_CreateL3Interface");
        if (VLAN_PMGR_VlanChangeToL2Type(vid) != VLAN_MGR_RETURN_OK)
            DBGprintf("Failed to VLAN_PMGR_VlanChangeToL2Type");

        return NETCFG_TYPE_FAIL;
    }

    NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceCreate(vlan_intf.ifindex, vlan_intf.u.physical_intf.mtu,
                                vlan_intf.u.physical_intf.bandwidth, vlan_intf.u.physical_intf.if_flags);

    if (if_status == VAL_ifOperStatus_up)
        NETCFG_MGR_IP_SignalInterfaceStatus(ifindex, INTERFACE_STATUS_UP);
    /* L3 interafce creation call back notification
     * and the status of IPv6AddrAutoconfig , for DHCPv6
     */
#if 0
    if(1 == DFLT_IPV6_ENABLE) // replace 1 to mib value (enable) in the future.
    {
        NETCFG_MGR_IP_IPv6Enable(ifindex);
    }
    else
    {
        NETCFG_MGR_IP_IPv6Disable(ifindex);
    }
#endif

#if (SYS_CPNT_IPV6 == TRUE)
    if(TRUE == DFLT_IPV6_ADDR_AUTOCONFIG)
        NETCFG_MGR_IP_IPv6AddrAutoconfigEnable(ifindex);
    else
        NETCFG_MGR_IP_IPv6AddrAutoconfigDisable(ifindex);
#if (SYS_CPNT_DHCPV6 == TRUE)
    SYS_CALLBACK_MGR_NETCFG_L3IfCreate(SYS_MODULE_IPCFG, ifindex, DFLT_IPV6_ADDR_AUTOCONFIG);
#endif
#endif

#if (SYS_CPNT_ROUTING == FALSE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE) /* for L2 switch */
#if (SYS_CPNT_CLUSTER == TRUE)
    /* only apply this if the configuration isn't on cluster vlan.
     * Otherwise cluster will delete L3If on the mgmt vlan.
     */
    if(vid != SYS_DFLT_CLUSTER_DEFAULT_VLAN)
#endif
    {
        UI32_T vid_tmp, vid_mgmt;
        NETCFG_TYPE_L3_Interface_T intf;
        NETCFG_OM_IP_InetRifConfig_T rif_om;

        memset(&intf, 0, sizeof(intf));
        memset(&rif_om, 0, sizeof(rif_om));

        /* delete other L3If which is not menagement vlan*/
        VLAN_OM_GetManagementVlan(&vid_mgmt);

        while(NETCFG_OM_IP_GetNextL3Interface(&intf) == NETCFG_TYPE_OK)
        {
            if(intf.ifindex == ifindex)
                continue;

            /* loopback int */
            if(FALSE == VLAN_OM_ConvertFromIfindex(intf.ifindex, &vid_tmp))
              continue;

            /* should skip cluster vlan, too. */
            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                continue;

            if(vid_tmp == vid_mgmt)
            {
                /* DEBUG */
                DEBUG_PRINTF("%s, %d, vlan %ld is mgmt vlan, skip delete it.\n", __FUNCTION__, __LINE__, (long)vid_tmp);
                continue;
            }
            /* DEBUG */
            DEBUG_PRINTF("%s, %d, vlan %ld is not mgmt vlan, delete it.\n", __FUNCTION__, __LINE__, (long)vid_tmp);
            NETCFG_MGR_IP_DeleteL3If(vid_tmp);
        }

    }
#endif

    /* check if rif reaches maximum number */
    if(TRUE == NETCFG_MGR_IP_IsReachMaxNbrOfRif())
        IPAL_IF_DisableIpv6AcceptRaPrefixInfo(ifindex);
    else
        IPAL_IF_EnableIpv6AcceptRaPrefixInfo(ifindex);

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_IP_DeleteL3If
 * PURPOSE:
 *      Delete a L3 interface,
 *      and leave mgmt vlan and change to L2type for Vlan CSC.
 *
 * INPUT:
 *      vid -- vlan id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      Should delete all rif entries of the interface.
 */
static UI32_T NETCFG_MGR_IP_DeleteL3If(UI32_T vid)
{
    UI32_T ifindex;
    NETCFG_TYPE_L3_Interface_T vlan_intf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (VLAN_OM_IsVlanExisted(vid) == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_DELETE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_NOT_EXIST,
                                 SYSLOG_LEVEL_ERR,
                                 "vid");
        return NETCFG_TYPE_INVALID_ARG;
    }

    /* Check if it's a created interface */
    memset(&vlan_intf, 0, sizeof (vlan_intf));
    VLAN_OM_ConvertToIfindex(vid, &ifindex);
    vlan_intf.ifindex = ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&vlan_intf) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_DELETE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_NO_VLAN_IN_INSTANCE,
                                 SYSLOG_LEVEL_ERR,
                                 "interface entry");
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    /* Delete all rif entries */
    if (NETCFG_MGR_IP_DeleteAllRifByIfindex(NETCFG_MGR_IP_FLAG_IPV4V6, vlan_intf.ifindex) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_DELETE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_MGR_IP_DeleteAllRifByIfindex");
        return NETCFG_TYPE_FAIL;
    }

    if (IP_MGR_DeleteInterface(vlan_intf.ifindex) == FALSE)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_DELETE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 SYSLOG_LEVEL_ERR,
                                 "IP_MGR_DeleteInterface");
        return NETCFG_TYPE_FAIL;
    }

#if (SYS_CPNT_ROUTING != TRUE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
        /* For L2 switch,
         * Leave management Vlan.
         */
        VLAN_PMGR_LeaveManagementVlan(vid);
#endif


    if (VLAN_PMGR_VlanChangeToL2Type(vid) != 0)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_DELETE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO_SET,
                                 SYSLOG_LEVEL_ERR,
                                 "VLAN_PMGR_VlanChangeToL3Type");
        return NETCFG_TYPE_FAIL;
    }

    if (NETCFG_OM_IP_DeleteL3Interface(&vlan_intf) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_DELETE_VLAN_INTERFACE_FUN_NO,
                                 EH_TYPE_MSG_FAILED_TO_DELETE,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_OM_IP_DeleteInterface");
        return NETCFG_TYPE_FAIL;
    }

    NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceDelete(vlan_intf.ifindex);

    /*Donny.li modify for VRRP*/
    SYS_CALLBACK_MGR_NETCFG_L3IfDestroy(SYS_MODULE_IPCFG, ifindex);

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SetIpAddressMode
 * PURPOSE:
 *      Set interface IP address mode.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *      ip_addr_mode-- one of {DHCP | BOOTP | USER_DEFINE}
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK -- keep this information in IPCFG_OM.
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED -- interface (ifindex do not exist)
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. Interface Ip Address Mode (or Access Method) kept in VLAN.
 *      2. Set both management vlan and address mode for IPv4.
 */
static UI32_T NETCFG_MGR_IP_SetIpAddressMode(UI32_T ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_T ip_addr_mode)
{
    UI32_T vid = 0; /* only valid for non-loopback int */
    NETCFG_TYPE_L3_Interface_T intf;
    I32_T result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_FAIL;

    memset(&intf, 0, sizeof(intf));
    /* For loopback interface, only accept USER_DEFINE mode */
    intf.ifindex = ifindex;
    if (NETCFG_TYPE_OK != (result=NETCFG_OM_IP_GetL3Interface(&intf)))
    {
        DBGprintf("result =%ld", (long)result);
        return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    }

    if (TRUE == IP_LIB_IsLoopbackInterface(intf.u.physical_intf.if_flags))
    {
        if (NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE != ip_addr_mode)
            return NETCFG_TYPE_FAIL;
    }
    else
    {
        if (VLAN_OM_ConvertFromIfindex(ifindex, &vid) == FALSE)
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
        if (VLAN_OM_IsVlanExisted(vid) == FALSE)
            return NETCFG_TYPE_INTERFACE_NOT_EXISTED;
    }

    if (ip_addr_mode != NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
        NETCFG_MGR_IP_DeleteAllRifByIfindex(NETCFG_MGR_IP_FLAG_IPV4, ifindex);

#if (SYS_CPNT_ROUTING == FALSE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE) /* for L2 switch */
#if (SYS_CPNT_CLUSTER == TRUE)
    /* only apply SetSingleManagementVlan, if the configuration isn't on cluster vlan.
     * Otherwise cluster will delete L3If on the normal vlan.
     */
    if(vid != SYS_DFLT_CLUSTER_DEFAULT_VLAN)
#endif
    NETCFG_MGR_IP_SetSingleManagementVlan(ifindex);
#endif

    if (NETCFG_OM_IP_SetIpAddressMode(ifindex, ip_addr_mode) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_FAIL;

    if (ip_addr_mode == NETCFG_TYPE_IP_ADDRESS_MODE_DHCP
#if (SYS_CPNT_BOOTP == TRUE)
        || ip_addr_mode == NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP
#endif
        )
    {
        SYS_CALLBACK_MGR_DHCPSetIfStatusCallback(SYS_MODULE_IPCFG, ifindex, DHCP_MGR_CLIENT_UP);
        SYS_CALLBACK_MGR_DHCPRestart3Callback(SYS_MODULE_IPCFG, DHCP_MGR_RESTART_CLIENT);
        SYS_CALLBACK_MGR_DHCPSetIfRoleCallback(SYS_MODULE_IPCFG, ifindex, DHCP_MGR_BIND_CLIENT);
    }
    else if (ip_addr_mode == NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
    {
        SYS_CALLBACK_MGR_DHCPSetIfStatusCallback(SYS_MODULE_IPCFG, ifindex, DHCP_MGR_CLIENT_DOWN);
        SYS_CALLBACK_MGR_DHCPRestart3Callback(SYS_MODULE_IPCFG, DHCP_MGR_RESTART_CLIENT);

    }

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SetInetRif
 * PURPOSE:
 *      To add or delete rif.
 * INPUT:
 *      rif_p               -- pointer to rif
 *      incoming_type       -- CLI/Web      1
 *                             SNMP         2
 *                             Dynamic      3
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_REJECT_SETTING_ENTRY
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This function is used for both IPv4/IPv6.
 */
UI32_T NETCFG_MGR_IP_SetInetRif(NETCFG_TYPE_InetRifConfig_T *rif_p,
                                        UI32_T incoming_type)
{
    UI32_T  row_status;
    NETCFG_TYPE_L3_Interface_T vlan_intf;
    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_IP_InetRifConfig_T rif_list;
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T ret = NETCFG_TYPE_OK;
    UI8_T  byte_mask[SYS_ADPT_IPV4_ADDR_LEN];
#if (SYS_CPNT_IPV6 == TRUE)
    UI32_T rc_iplib;
#endif
#if (SYS_CPNT_CLUSTER == TRUE)
    UI32_T vid = 0;
#endif

    NOTEprintf("set: on vlan %ld,(%d)%lx:%lx:%lx:%lx,status=%s,v4=%ld, v6=%ld,come=%ld",
                    (long)rif_p->ifindex,
                    rif_p->addr.type, L_INET_EXPAND_IPV6(rif_p->addr.addr),
                    RAW_STATUS_TO_STRING(rif_p->row_status),
                    (long)rif_p->ipv4_role ,
                    (long)rif_p->ipv6_addr_type,
                    (long)incoming_type
                );
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Parameter Validation
     */
    if(NULL == rif_p)
        return NETCFG_TYPE_INVALID_ARG;

    if ((incoming_type < NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB) ||
        (incoming_type > NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC))
        return NETCFG_TYPE_INVALID_ARG;

    if ((rif_p->row_status < VAL_netConfigStatus_2_active) ||
        (rif_p->row_status > VAL_netConfigStatus_2_destroy))
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    if(   (rif_p->addr.type == L_INET_ADDR_TYPE_IPV4)
       || (rif_p->addr.type == L_INET_ADDR_TYPE_IPV4Z)
      )
    {
        /* check ip_address, ip_mask,
         * 1. must be either class A, class B, or class C
         * 2. can't be network b'cast ip or network id
         * 3. can't be loop back ip
         * 4. can't be b'cast ip
         * 5. can't be m'cast ip
         * 6. mask is continue bit 1
         */

        memset(&byte_mask, 0, sizeof(SYS_ADPT_IPV4_ADDR_LEN));
        IP_LIB_CidrToMask(rif_p->addr.preflen, byte_mask);

        /* Check if the IP mask is continuous bit 1. */
        if(IP_LIB_IsValidNetworkMask(byte_mask) == FALSE)
        {
            return NETCFG_TYPE_INVALID_ARG;
        }

        /* check if it's a zero ip & mask*/
        if(IP_LIB_IsIpMaskZero(rif_p->addr.addr, byte_mask) == TRUE)
        {
            return NETCFG_TYPE_INVALID_ARG;
        }

        if ((rc_iplib = IP_LIB_IsValidForIpConfig(rif_p->addr.addr, byte_mask)) != IP_LIB_OK)
        {
            UI32_T  tmp_lo_id;

            /* allow to set x.x.x.x/32 on an loopback interface
             */
            if (  FALSE == (IP_LIB_ConvertLoopbackIfindexToId(rif_p->ifindex, &tmp_lo_id))
                ||(IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR != rc_iplib)
               )
            {
                DEBUG_PRINTF("IP_LIB_IsValidForIpConfig rc: %lx\r\n", (unsigned long)rc_iplib);
                return NETCFG_TYPE_INVALID_ARG;
            }
        }
    }
    else if ((L_INET_ADDR_TYPE_IPV6Z == rif_p->addr.type)
            && (rif_p->ipv6_addr_type == NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL))
    {
        /* Check valid link-local unicast */
        if(FALSE == L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_UNICAST(rif_p->addr.addr))
        {
            DEBUG_PRINTF("%d, link-local should be fe80::/64.\r\n", __LINE__);
            return NETCFG_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_FORMAT;
        }
    }


    /* check valid action value and for specfied incomming type */
    row_status = rif_p->row_status;
    switch (row_status)
    {
        case VAL_netConfigStatus_2_notInService:
            if (incoming_type != NETCFG_TYPE_IP_CONFIGURATION_TYPE_SNMP)
                return NETCFG_TYPE_REJECT_SETTING_ENTRY;

            break;
        case VAL_netConfigStatus_2_createAndGo :
            /* for snmp, if primary rif is existed, return fail. */

            /* for CLI/Web, if primary rif is existed, overwrite it. */
            /* for CLI/Web, if original addr_mode is BOOP/DHCP, it will change to USER_DEFINE */

            /* for IPv4 and incomming_type is Dynamic, the role of rif MUST be primary */
            if((L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type) && (incoming_type == NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC)&&(NETCFG_TYPE_MODE_PRIMARY != rif_p->ipv4_role) )
                return NETCFG_TYPE_REJECT_SETTING_ENTRY;

            /*  For SNMP operation, not allow CreateAndGo action,
             *  because not all fields necessary existed.
             */
            if (incoming_type == NETCFG_TYPE_IP_CONFIGURATION_TYPE_SNMP)
                return NETCFG_TYPE_REJECT_SETTING_ENTRY;

            break;
        case VAL_netConfigStatus_2_notReady :
            if (incoming_type != NETCFG_TYPE_IP_CONFIGURATION_TYPE_SNMP)
                return NETCFG_TYPE_REJECT_SETTING_ENTRY;

            break;
        case VAL_netConfigStatus_2_active:
        case VAL_netConfigStatus_2_createAndWait :
        case VAL_netConfigStatus_2_destroy :
            break;
        default:
            return NETCFG_TYPE_FAIL;
    }

    memset(&vlan_intf, 0, sizeof (vlan_intf));
    vlan_intf.ifindex = rif_p->ifindex;
    memset(&intf, 0, sizeof (intf));
    intf.ifindex = rif_p->ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&vlan_intf) != NETCFG_TYPE_OK)
    {
        EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                 NETCFG_MGR_IP_SET_IPV4_RIF_FUN_NO,
                                 EH_TYPE_MSG_NOT_EXIST,
                                 SYSLOG_LEVEL_ERR,
                                 "NETCFG_MGR_IP_SetInetRif(l3ipvlan)");
        DBGprintf("no l3 intf?");
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

#if 0 // peter
    memset(&inet_rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    memcpy(&inet_addr, rif_p->ip_addr, SYS_ADPT_IPV4_ADDR_LEN);
    memcpy(&inet_mask, rif_p->mask, SYS_ADPT_IPV4_ADDR_LEN);

    inet_rif.ifindex = rif_p->ifindex;
    inet_rif.role = rif_p->primary_interface;
    inet_rif.row_status = rif_p->row_status;
    L_PREFIX_InetAddr2Prefix(inet_addr, inet_mask, &(inet_rif.addr));
#endif

    /* Check if it is the same ipv4/ipv4z/ipv6/ipv6z address on same L3If.
     */
    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    rif_om.ifindex = rif_p->ifindex;
    memcpy(&rif_om.addr, &rif_p->addr, sizeof(L_INET_AddrIp_T));
    if(L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type)
    {
        if((NETCFG_TYPE_OK == NETCFG_OM_IP_GetInetRif(&rif_om))
            && (rif_om.ifindex == rif_p->ifindex)
            && (rif_om.ipv4_role == rif_p->ipv4_role)
            && (rif_om.row_status == VAL_netConfigStatus_2_active)
            && (rif_p->row_status == VAL_netConfigStatus_2_createAndGo))
        {
            return NETCFG_TYPE_OK;
        }
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if(L_INET_ADDR_TYPE_IPV6 == rif_p->addr.type)
    {
        if((NETCFG_TYPE_OK == NETCFG_OM_IP_GetInetRif(&rif_om))
            && (rif_om.ifindex == rif_p->ifindex)
            && (rif_om.ipv6_addr_type == rif_p->ipv6_addr_type)
            && (rif_om.ipv6_addr_config_type == rif_p->ipv6_addr_config_type)
            && (rif_om.row_status == VAL_netConfigStatus_2_active)
            && (rif_p->row_status == VAL_netConfigStatus_2_createAndGo))
        {
            return NETCFG_TYPE_OK;
        }
    }
#endif

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    rif_om.ifindex = rif_p->ifindex;
    rif_om.ipv4_role = rif_p->ipv4_role;
    rif_om.row_status = rif_p->row_status;
    memcpy(&rif_om.addr, &rif_p->addr, sizeof(L_INET_AddrIp_T));


#if(SYS_CPNT_VIRTUAL_IP == TRUE)
    /* If adding new non-virtual rif, check overlape for ipv4/ipv6, but not ipv4z/ipv6z */
    if ((rif_p->ipv4_role != NETCFG_TYPE_MODE_VIRTUAL) && (rif_p->row_status == VAL_netConfigStatus_2_createAndGo || rif_p->row_status == VAL_netConfigStatus_2_active)
        && (L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type || L_INET_ADDR_TYPE_IPV6 == rif_p->addr.type))
#else
    /* If adding new rif, check overlape for ipv4/ipv6, but not ipv4z/ipv6z */
    if ((rif_p->row_status == VAL_netConfigStatus_2_createAndGo || rif_p->row_status == VAL_netConfigStatus_2_active)
        && (L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type || L_INET_ADDR_TYPE_IPV6 == rif_p->addr.type))
#endif
    {
        ret = NETCFG_OM_IP_CheckAddressOverlap(&intf, &rif_om);
        if (ret == NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED)
        {
            DBGprintf("rif overlapped!");
            return ret;
        }
    }
    /* IPv4 */
    if(L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type)
    {
        if (incoming_type != NETCFG_TYPE_IP_CONFIGURATION_TYPE_SNMP)
        {
            /* If it's a updating of primary rif, delete the previous primary RIF first */
            ret = NETCFG_MGR_IP_DeletePrePrimaryIPv4Rif(rif_p);
            if (ret == NETCFG_TYPE_NO_CHANGE)
                return NETCFG_TYPE_OK;
            else if(ret != NETCFG_TYPE_OK)
                return ret;
        }
        else if (ret != NETCFG_TYPE_OK)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                     NETCFG_MGR_IP_SET_IPV4_RIF_FUN_NO,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "NETCFG_MGR_IP_DeletePrePrimaryIPv4Rif");
            return NETCFG_TYPE_FAIL;
        }

        /* If it's a deletion of primary rif, there should be no secondary rif exists
         */
        if ((rif_p->row_status == VAL_netConfigStatus_2_destroy) &&
            (rif_p->ipv4_role == NETCFG_TYPE_MODE_PRIMARY))
        {
            memset(&rif_list, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            while (NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_list, rif_p->ifindex) == NETCFG_TYPE_OK)
            {
                if((rif_list.ipv4_role == VAL_iPAddrPrimaryInterface_secondary) &&
                   (rif_list.row_status == VAL_netConfigStatus_2_active))
                {
                    return NETCFG_TYPE_MUST_DELETE_SECONDARY_FIRST;
                }
            }
        }

        /* There must be primary rif exist before adding secondary rif
         */
        if ((rif_p->row_status == VAL_netConfigStatus_2_createAndGo) &&
            (rif_p->ipv4_role == NETCFG_TYPE_MODE_SECONDARY))
        {
            BOOL_T primary_exist = FALSE;

            memset(&rif_list, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            while (NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_list, rif_p->ifindex) == NETCFG_TYPE_OK)
            {
                if((rif_list.ipv4_role == VAL_iPAddrPrimaryInterface_primary) &&
                   (rif_list.row_status == VAL_netConfigStatus_2_active))
                {
                    primary_exist = TRUE;
                    break;
                }
            }

            if (!primary_exist)
            {
                return NETCFG_TYPE_PRIMARY_IP_NOT_EXIST;
            }
        }

        memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

        rif_om.ifindex = rif_p->ifindex;
        rif_om.ipv4_role = rif_p->ipv4_role;
        rif_om.row_status = rif_p->row_status;
        memcpy(&rif_om.addr, &rif_p->addr, sizeof(L_INET_AddrIp_T));

        if (NETCFG_MGR_IP_InternalSetInetRif(&rif_om) != NETCFG_TYPE_OK)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                     NETCFG_MGR_IP_SET_IPV4_RIF_FUN_NO,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "NETCFG_MGR_IP_Internal_SetIPv4Rif");
            return NETCFG_TYPE_FAIL;
        }
    } /* L_INET_ADDR_TYPE_IPV4 */

    /* IPv6 */
#if (SYS_CPNT_IPV6 == TRUE)
    else if ((L_INET_ADDR_TYPE_IPV6 == rif_p->addr.type)
        || (L_INET_ADDR_TYPE_IPV6Z == rif_p->addr.type))
    {
        /* check ipv6 address and mask,
         1. Unspecified     ::/128
         2. Loopback        ::1/128
         3. Multicast       FF00::/8
         */

         /*
         * 2. can't be network b'cast ip or network id  ??
         * 3. can't be loop back ip
         * 4. can't be b'cast ip
         * 5. can't be m'cast ip
         */

        if(IP_LIB_OK != (rc_iplib=IP_LIB_CheckIPv6PrefixForInterface(rif_p->addr.addr, rif_p->addr.preflen)))
        {
            switch(rc_iplib)
            {
                case IP_LIB_INVALID_IPV6_UNSPECIFIED:
                    return NETCFG_TYPE_INVALID_IPV6_UNSPECIFIED;
                case IP_LIB_INVALID_IPV6_LOOPBACK:
                    return NETCFG_TYPE_INVALID_IPV6_LOOPBACK;
                case IP_LIB_INVALID_IPV6_MULTICAST:
                default:
                    return NETCFG_TYPE_INVALID_IPV6_MULTICAST;
            }
        }

    /* L2 sepecified
     */
    /* There can only a vlan has IPv4/IPv6 address.
     * p.s. IPv4 is implemented by set address mode.
     */
#if (SYS_CPNT_ROUTING == FALSE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE) /* for L2 switch */
#if (SYS_CPNT_CLUSTER == TRUE)
    VLAN_OM_ConvertFromIfindex(rif_p->ifindex, &vid);
    /* only apply SetSingleManagementVlan, if the configuration isn't on cluster vlan.
     * Otherwise cluster will delete L3If on the normal vlan.
     */
    if(vid != SYS_DFLT_CLUSTER_DEFAULT_VLAN)
#endif
        NETCFG_MGR_IP_SetSingleManagementVlan(rif_p->ifindex);
#endif

        if((row_status == VAL_netConfigStatus_2_createAndGo)
        ||(row_status == VAL_netConfigStatus_2_createAndWait))
        {
            switch(rif_p->ipv6_addr_type)
            {
                /* If config link-local address manually., and action is c+g(?) or c+w(?),
                 * we should delete the old one.
                 */
                case NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL:
                    {
                        NETCFG_TYPE_InetRifConfig_T inet_rif;


                        memset(&inet_rif, 0, sizeof(inet_rif));
                        inet_rif.ifindex = rif_p->ifindex;
                        inet_rif.addr.type = rif_p->addr.type;
                        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetLinkLocalRifFromInterface(&inet_rif))
                        {
                            memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
                            memcpy(&rif_om.addr, &inet_rif.addr, sizeof(L_INET_AddrIp_T));
                            rif_om.ifindex = inet_rif.ifindex;

                            if(memcmp(inet_rif.addr.addr, rif_p->addr.addr, SYS_ADPT_IPV6_ADDR_LEN))
                            {
                                /* there should be only one link-local, delete old one, and will add new one later */
                                rif_om.ipv6_addr_config_type = inet_rif.ipv6_addr_config_type;
                                rif_om.ipv6_addr_type = inet_rif.ipv6_addr_type;
                                rif_om.row_status = VAL_netConfigStatus_2_destroy;
                                NETCFG_MGR_IP_InternalSetInetRif(&rif_om);
                            }
                        /* keep going to add the new one */

                        }
                    }
                    break;

                case NETCFG_TYPE_IPV6_ADDRESS_TYPE_GLOBAL:
                    /* If the address is ipv6 global address, and action is c+g/c+w,
                     * we need to add automatic-generated link-local.
                     */
                    {
                        /* If there is no link-local addr, then add a auto one. */

                        NETCFG_OM_IP_InetRifConfig_T rif_om;

                        memset(&rif_om, 0, sizeof(rif_om));
                        rif_om.ifindex = rif_p->ifindex;
                        rif_om.addr.addr[0] = 0xFE;
                        rif_om.addr.addr[1] = 0x80;
                        rif_om.addr.type = L_INET_ADDR_TYPE_IPV6Z;
                        /* get next once, it should return the link-local if there is one */
                        if( NETCFG_TYPE_OK != NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_om, rif_p->ifindex))
                        {
                            NETCFG_MGR_IP_IPv6_SetAutoLinkLocal(rif_p->ifindex);
                        }
                    }
                    break;

                case NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64:
                    /* If the address is ipv6 EUI64 address, and action is c+g/c+w,
                     * we need to add automatic-generated link-local.
                     */
                    NETCFG_MGR_IP_IPv6_SetAutoLinkLocal(rif_p->ifindex);

                    /* continue */
                    break;

                default:
                    /* DEBUG */
                    printf("%s, Invalid ipv6_addr_type type: %ld for C+G or C+W\n", __FUNCTION__, (long)rif_p->ipv6_addr_type);
                    return NETCFG_TYPE_FAIL;
            } /* switch */
        }

        /* if there is already "ipv6 enable", it will fail to do action(add/delete) with the same link-local address manually. */
        if(rif_p->addr.type == L_INET_ADDR_TYPE_IPV6Z)
        {
            NETCFG_TYPE_InetRifConfig_T rif;

            memset(&rif, 0, sizeof(rif));
            rif.ifindex = rif_p->ifindex;
            rif.addr.type = L_INET_ADDR_TYPE_IPV6Z;

            if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetLinkLocalRifFromInterface(&rif))
            {
                /* for ipv6_addr_config_type, one is NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL, one is NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER (ipv6 enable) */
                if((rif_p->ipv6_addr_config_type != rif.ipv6_addr_config_type)
                   && (L_INET_CompareInetAddr((L_INET_Addr_T *) &rif_p->addr, (L_INET_Addr_T *) &rif.addr, 0)==0))
                {
                    return NETCFG_TYPE_FAIL;
                }
            }
       }

        memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

        memcpy(&rif_om.addr, &rif_p->addr, sizeof(L_INET_AddrIp_T));

        rif_om.ifindex = rif_p->ifindex;
        rif_om.ipv6_addr_type = rif_p->ipv6_addr_type;
        rif_om.ipv6_addr_config_type = rif_p->ipv6_addr_config_type;
        rif_om.row_status = rif_p->row_status;



        if (NETCFG_MGR_IP_InternalSetInetRif(&rif_om) != NETCFG_TYPE_OK)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_IPCFG,
                                     NETCFG_MGR_IP_SET_IPV4_RIF_FUN_NO,
                                     EH_TYPE_MSG_FAILED_TO_SET,
                                     SYSLOG_LEVEL_ERR,
                                     "NETCFG_MGR_IP_Internal_SetIPv6Rif");
            return NETCFG_TYPE_FAIL;
        }

        /* If we are going to destroy a link-local address, but
         * the interface is ipv6 enabled explicitly or autoconfig enabled or has global address
         * we need to add auto link-local addr back.
         */
        if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif_p->addr.addr)
        && (row_status == VAL_netConfigStatus_2_destroy))
        {
            BOOL_T ipv6_enable, has_ipv6_global, autoconfig;
            NETCFG_TYPE_InetRifConfig_T rif;

            has_ipv6_global = FALSE;
            autoconfig = FALSE;
            memset(&rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
            rif.ifindex = rif_p->ifindex;
            rif.addr.type = L_INET_ADDR_TYPE_IPV6;
            /* check if there is any global address */
            while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif))
            {
                /* check ipv6 global address */
                if(!L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif.addr.addr))
                {
                    has_ipv6_global = TRUE;
                    break;
                }
            }

            if((NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6EnableStatus(rif_p->ifindex, &ipv6_enable)
                    && ipv6_enable == TRUE)
                || (NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus(rif_p->ifindex, &autoconfig)
                    && autoconfig == TRUE)
                || has_ipv6_global)
            {
                NETCFG_MGR_IP_IPv6_SetAutoLinkLocal(rif_p->ifindex);
            }
        }

        /* If the address is the LAST ipv6 global address and action is destroy,
         * (there is no global address anymore)
         * we need to check if the interface is not "ipv6 enable" explicitly
         * and if the link-local is auto, then also destroy it.
         */
        {
            BOOL_T has_ipv6_global = FALSE;
            BOOL_T ipv6_explicit_enable = FALSE;
            NETCFG_TYPE_InetRifConfig_T rif;

            if(!L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif_p->addr.addr)
            && (row_status == VAL_netConfigStatus_2_destroy))
            {

                memset(&rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
                rif.ifindex = rif_p->ifindex;
                rif.addr.type = L_INET_ADDR_TYPE_IPV6;
                /* check if there is any global address */
                while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif))
                {
                    /* check ipv6 global address, is it right ? */
                    if(!L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif.addr.addr))
                    {
                        has_ipv6_global = TRUE;
                        break;
                    }
                } /* while */

                if((!has_ipv6_global)
                    && (NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6EnableStatus(rif_p->ifindex, &ipv6_explicit_enable)
                        && (!ipv6_explicit_enable)))
                {
                    NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal(rif_p->ifindex);
                }
            } /* if */
        }
    } /* L_INET_ADDR_TYPE_IPV6 */

#endif
    else
    {
        return NETCFG_TYPE_FAIL;
    }
    return NETCFG_TYPE_OK;
}

#if 0 /* there's no document mention about this eui-64*/
/*
    Generate EUI64 address based on IP
    input:    addr : IPv6 address , address maybe changed after this function
                preflen:
                sourc_vlan_ifindex: ip is based on source_vlan's primary IP
    output: addr
*/
static UI32_T NETCFG_MGR_IP_IPv6_TunnelAddressEUI64(UI8_T* addr, UI16_T preflen, UI32_T sourc_vlan_ifindex)
{
    UI32_T ret;
    NETCFG_OM_IP_InetRifConfig_T primary_rif;
    memset(&primary_rif,0,sizeof(primary_rif));
   if(NETCFG_TYPE_OK !=(ret = NETCFG_OM_IP_LookupPrimaryRif(&primary_rif, sourc_vlan_ifindex)))
   {
        DBGprintf("Fail to get source vlan IP for vidx %ld", sourc_vlan_ifindex);
        return NETCFG_TYPE_FAIL;
   }
   if(IP_LIB_IS_6TO4_ADDR(addr))
        return  NETCFG_MGR_IP_IPv6_IPv6AddressModifiedEUI64( addr,  preflen, primary_rif.addr.addr);
   else if (IP_LIB_IS_ISATAP_ADDR(addr))
        return  NETCFG_MGR_IP_IPv6_IPv6AddressIsatapEUI64( addr,  preflen, primary_rif.addr.addr);

   /* Manauel tunnel can't be set to eui64 ?*/
   return NETCFG_TYPE_FAIL;
}
#endif

/* FUNCTION NAME: NETCFG_MGR_IP_InternalSetInetRif
 * PURPOSE:
 *          To add or delete rif.
 * INPUT:
 *          rif_p               -- pointer to rif
 *          rif_p->rowStatus
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          NETCFG_TYPE_OK
 *          NETCFG_TYPE_FAIL
 *          NETCFG_TYPE_ENTRY_NOT_EXIST
 *          NETCFG_TYPE_NOT_ALL_CONFIG
 *
 * NOTES:
 *      1. This function is used for both IPv4/IPv6.
 *
 */

static UI32_T NETCFG_MGR_IP_InternalSetInetRif(NETCFG_OM_IP_InetRifConfig_T *rif_config)
{
     /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_TYPE_InetRifConfig_T inet_rif;
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    NETCFG_OM_IP_InetRifConfig_T primary_rif;
    NETCFG_OM_IP_InetRifConfig_T rif_list;
    NETCFG_TYPE_L3_Interface_T intf;
    UI32_T  state, transition_state;
    UI32_T  action;
    UI32_T rc, ret, rc_mgr, rc_om;

    /* BODY
     */
     INFOprintf("set if %ld ,%lx:%lx:%lx:%lx as %s"
                    ,   (long)rif_config->ifindex
                    ,   L_INET_EXPAND_IPV6(rif_config->addr.addr)
                    ,   RAW_STATUS_TO_STRING(rif_config->row_status)
                    );
    memset(&inet_rif, 0, sizeof (NETCFG_TYPE_InetRifConfig_T));
    memset(&intf, 0, sizeof (intf));
    memcpy(&rif_om, rif_config, sizeof (NETCFG_OM_IP_InetRifConfig_T));

    inet_rif.ifindex = rif_config->ifindex;
    inet_rif.ipv4_role = rif_config->ipv4_role;
    memcpy(&inet_rif.addr, &rif_config->addr, sizeof(L_INET_AddrIp_T));

    intf.ifindex = rif_config->ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&intf) != NETCFG_TYPE_OK)
    {
        DBGprintf("Fail to get l3 intf!");
        return NETCFG_TYPE_FAIL;
    }
    action = rif_config->row_status;
    switch (action)
    {
        case VAL_netConfigStatus_2_active:
            /* Check the RIF exist or not */
            if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetInetRif(&rif_om))
                return NETCFG_TYPE_ENTRY_NOT_EXIST;

            state = rif_om.row_status;
            switch(state)
            {
                case VAL_netConfigStatus_2_notInService:
                    ret = NETCFG_OM_IP_LookupPrimaryRif(&primary_rif, rif_config->ifindex);
                    if((ret != NETCFG_TYPE_OK) || (primary_rif.row_status != VAL_netConfigStatus_2_active))
                    {
                        if((rif_config->addr.type== L_INET_ADDR_TYPE_IPV4 ||rif_config->addr.type== L_INET_ADDR_TYPE_IPV4Z)
                            &&rif_config->ipv4_role != VAL_iPAddrPrimaryInterface_primary)
                        {
                            DBGprintf("not v4 primary!");
                            return NETCFG_TYPE_FAIL;
                        }
                    }
                    if((ret == NETCFG_TYPE_OK) && (primary_rif.row_status == VAL_netConfigStatus_2_active))
                        if(rif_config->ipv4_role == VAL_iPAddrPrimaryInterface_primary)
                            if(NETCFG_TYPE_OK != NETCFG_OM_IP_DeleteInetRif(&primary_rif))
                            {
                                DBGprintf("Fail to delete rif!");
                                return NETCFG_TYPE_FAIL;
                            }
                    rc = NETCFG_TYPE_FAIL;
                    rif_om.row_status = VAL_netConfigStatus_2_active;
                    rc = NETCFG_MGR_IP_InternalSetRifActive(&inet_rif, &rif_om);

#if (SYS_CPNT_LLDP == TRUE)
                    if (rc == NETCFG_TYPE_OK)
                    {
                        LLDP_PMGR_NotifyRifChanged(rif_om.ifindex);
                    }
#endif
                return rc;
                case VAL_netConfigStatus_2_notReady:
                    break;
                default:
                    return NETCFG_TYPE_FAIL;
            }
            break;

        case VAL_netConfigStatus_2_notInService:

            /* Check the RIF exist or not */
            if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetInetRif(&rif_om))
                return NETCFG_TYPE_ENTRY_NOT_EXIST;
            state = rif_om.row_status;
            switch(state)
            {
                case VAL_netConfigStatus_2_notReady:
                case VAL_netConfigStatus_2_notInService:
                    if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetInetRif(&rif_om))
                        return NETCFG_TYPE_ENTRY_NOT_EXIST;
                    rc = NETCFG_TYPE_FAIL;
                    if(NETCFG_TYPE_OK == NETCFG_OM_IP_DeleteInetRif(&rif_om))
                    {
                        memcpy(&rif_om, rif_config, sizeof (NETCFG_OM_IP_InetRifConfig_T));
                        rc = NETCFG_OM_IP_SetInetRif(&rif_om);
                        if(rc == NETCFG_TYPE_OK)
                        {
                            if(rif_om.ipv4_role == VAL_iPAddrPrimaryInterface_UnknownType)
                            {
                                rif_om.row_status = VAL_netConfigStatus_2_notReady;
                                if(NETCFG_TYPE_OK != NETCFG_OM_IP_SetInetRifRowStatus(&rif_om))
                                    rc = NETCFG_TYPE_FAIL;
                            }
                        }
                    }
                    return rc;

                case VAL_netConfigStatus_2_active:
                    if(rif_om.addr.type == L_INET_ADDR_TYPE_IPV4 ||rif_om.addr.type == L_INET_ADDR_TYPE_IPV4Z)
                    {
                        if(rif_om.ipv4_role == VAL_iPAddrPrimaryInterface_primary)
                        {

                            int count = 0;
                            while (NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_list, rif_config->ifindex) == NETCFG_TYPE_OK)
                            {
                                if((rif_list.ipv4_role == VAL_iPAddrPrimaryInterface_secondary) &&
                                    (rif_list.row_status == VAL_netConfigStatus_2_active))
                                    count++;
                            }
                            if(count > 0)
                                return NETCFG_TYPE_FAIL;
                        }
                    }

                    NETCFG_MGR_IP_SignalAllProtocolRifDown(&inet_rif);
                    rc = NETCFG_TYPE_FAIL;
                    rif_om.row_status = VAL_netConfigStatus_2_notInService;
                    if(NETCFG_TYPE_OK == NETCFG_OM_IP_SetInetRifRowStatus(&rif_om))
                    {
                        NETCFG_MGR_IP_SignalAllProtocolRifDelete(&inet_rif);
                        rc = NETCFG_TYPE_OK;
                    }
                    return rc;

                default:
                    return NETCFG_TYPE_FAIL;
            }
            break;
        case VAL_netConfigStatus_2_notReady:
            /* Check the RIF exist or not */
            if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetInetRif(&rif_om))
                return NETCFG_TYPE_ENTRY_NOT_EXIST;
            if(rif_config->addr.type == L_INET_ADDR_TYPE_IPV4 ||rif_config->addr.type == L_INET_ADDR_TYPE_IPV4Z )
            if(rif_config->ipv4_role == VAL_iPAddrPrimaryInterface_UnknownType)
            {
                INFOprintf("return due to ipv4 unknown??");
                return NETCFG_TYPE_OK;
            }
            rc = NETCFG_TYPE_FAIL;

            if(NETCFG_TYPE_OK == NETCFG_OM_IP_DeleteInetRif(&rif_om))
            {
                memcpy(&rif_om, rif_config, sizeof (NETCFG_OM_IP_InetRifConfig_T));
                if(NETCFG_TYPE_OK == NETCFG_OM_IP_SetInetRif(&rif_om))
                {
                    rif_om.row_status = VAL_netConfigStatus_2_notInService;
                    if(NETCFG_TYPE_OK == NETCFG_OM_IP_SetInetRifRowStatus(&rif_om))
                        rc = NETCFG_TYPE_OK;
                }
            }
            return rc;


        case VAL_netConfigStatus_2_createAndGo:
            state = L_RSTATUS_NOT_EXIST;
            transition_state = L_RSTATUS_Fsm(action, &state, NETCFG_MGR_IP_CheckRifSemantic, &rif_om);
            switch (state)
            {
                case VAL_netConfigStatus_2_active:
                    /*  create rif in IPCFG_OM */
                    rif_om.row_status = state;
#if (SYS_CPNT_IPV6 == TRUE)
                    if(NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64 == rif_om.ipv6_addr_type)
                    {
                        NETCFG_MGR_IP_IPv6_IPv6AddressEUI64(rif_om.addr.addr, rif_om.addr.preflen, intf.u.physical_intf.hw_addr);
                    }
#endif /*SYS_CPNT_IPV6*/
                    rc_om = NETCFG_OM_IP_SetInetRif(&rif_om);
                    /* new address should turn on invalid flag */
                    NETCFG_OM_IP_SetRifFlags(&rif_om, NETCFG_TYPE_RIF_FLAG_IPV6_INVALID);
                    if(NETCFG_TYPE_OK == rc_om)
                    {

#if (SYS_CPNT_ROUTING == TRUE) && (SYS_CPNT_IPV6 == TRUE)
                        /* Enable to trap my ipv6 link-local address pkt to CPU */
                        if(rif_om.addr.type == L_INET_ADDR_TYPE_IPV6Z)
                        {
                            UI32_T vid;


                            VLAN_OM_ConvertFromIfindex(rif_om.ifindex, &vid);

                            if(!L4_PMGR_TrapLinkLocalToCPU(TRUE, rif_om.addr.addr, vid))
                            {
                                DEBUG_PRINTF("Failed to trap link local to cpu.\n");
                                NETCFG_OM_IP_DeleteInetRif(&rif_om);
                                return NETCFG_TYPE_FAIL;
                            }
                        }
#endif
                        /* add ip address in IP_MGR */
                        memcpy(&inet_rif.addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));
                        rc = NETCFG_MGR_IP_InternalSetRifActive(&inet_rif, &rif_om);

                        if (rc != NETCFG_TYPE_OK)
                        {
                            DEBUG_PRINTF("Failed to add rif into kernel.\n");
                            NETCFG_OM_IP_DeleteInetRif(&rif_om);
                            return NETCFG_TYPE_FAIL;
                        }


                        /* return rc; */
                        return NETCFG_TYPE_OK; /* Alwayws OK. Ignore the active setting failure in ip_mgr */
                    }
                    else if (rc_om == NETCFG_TYPE_CAN_NOT_PERFORM_CHANGE)
                    {
                        return NETCFG_TYPE_FAIL;
                    }
                    else
                    {
                        /* Otherwise, set row status to be not ready */
                        rif_om.row_status = VAL_netConfigStatus_2_notReady;
                            DBGprintf("Fail to set OM");
                        NETCFG_OM_IP_SetInetRifRowStatus(&rif_om);

                        return NETCFG_TYPE_FAIL;
                    }
                    break;
                case VAL_netConfigStatus_2_notReady:
                    break;

                default:
                    return (NETCFG_TYPE_FAIL);
            }
            break;

        case VAL_netConfigStatus_2_createAndWait:
            state = L_RSTATUS_NOT_EXIST;
            transition_state = L_RSTATUS_Fsm(action, &state, NETCFG_MGR_IP_CheckRifSemantic, &rif_om);
            switch (state)
            {
                case VAL_netConfigStatus_2_notReady:
                case VAL_netConfigStatus_2_notInService:
                    /*  create rif in OM and set status to NotReady */
                    rif_om.row_status = state;
                    rif_om.ipv4_role = VAL_iPAddrPrimaryInterface_UnknownType;
                    rc = NETCFG_OM_IP_SetInetRif(&rif_om);
                    /* new address should turn on invalid flag */
                    NETCFG_OM_IP_SetRifFlags(&rif_om, NETCFG_TYPE_RIF_FLAG_IPV6_INVALID);
                    return rc;
                    break;
                default :
                    return NETCFG_TYPE_FAIL;
            }
            break;

        case VAL_netConfigStatus_2_destroy:

            state = rif_om.row_status;
            transition_state = L_RSTATUS_Fsm(action, &state, NETCFG_MGR_IP_CheckRifSemantic, &rif_om);
            if (L_RSTATUS_NOT_EXIST==state)
                state = VAL_netConfigStatus_2_destroy;
            switch (state)
            {
                case VAL_netConfigStatus_2_destroy:
#if (SYS_CPNT_IPV6 == TRUE)
                    if(NETCFG_TYPE_IPV6_ADDRESS_TYPE_EUI64 == rif_config->ipv6_addr_type)
                    {
                        NETCFG_MGR_IP_IPv6_IPv6AddressEUI64(rif_om.addr.addr, rif_om.addr.preflen, intf.u.physical_intf.hw_addr);
                    }
#endif
                    /* Check the RIF exist or not */
                    if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetInetRif(&rif_om))
                    {
                        DBGprintf("fail to get rif");
                        return NETCFG_TYPE_ENTRY_NOT_EXIST;
                    }
                    if((rif_config->addr.type== L_INET_ADDR_TYPE_IPV4 ||rif_config->addr.type== L_INET_ADDR_TYPE_IPV4Z)
                          && rif_om.ipv4_role != rif_config->ipv4_role)
                            return NETCFG_TYPE_ENTRY_NOT_EXIST;

#if (SYS_CPNT_ROUTING == TRUE) && (SYS_CPNT_IPV6 == TRUE)
                    /* Disable to trap my ipv6 link-local address pkt to CPU */
                    if(rif_om.addr.type == L_INET_ADDR_TYPE_IPV6Z)
                    {
                        UI32_T vid;

                        VLAN_OM_ConvertFromIfindex(rif_om.ifindex, &vid);

                        if(!L4_PMGR_TrapLinkLocalToCPU(FALSE, rif_om.addr.addr, vid))
                        {
                            DEBUG_PRINTF("Failed to disable trap link local to cpu.\n");
                            return NETCFG_TYPE_FAIL;
                        }
                    }
#endif

                    /* delete generated eui-64 address in IP_MGR */
                    memcpy(&inet_rif.addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

                    NETCFG_MGR_IP_SignalAllProtocolRifDown(&inet_rif);
#if(SYS_CPNT_VIRTUAL_IP == TRUE)
                    if(inet_rif.ipv4_role == NETCFG_TYPE_MODE_VIRTUAL)
                    {
                        rc_mgr = NETCFG_MGR_IP_InternalSetVirtualRifDestroy(&inet_rif);
                    }
                    else
#endif
                    {
                    rc_mgr = IP_MGR_DeleteInetRif(&inet_rif);
                    }
                    rc_om = NETCFG_OM_IP_DeleteInetRif(&rif_om);

                    if((rc_mgr != NETCFG_TYPE_OK) || (rc_om != NETCFG_TYPE_OK))
                    {
                        DEBUG_PRINTF("rif info corrupted at IPCFG_OM or IP_MGR/IPAL\n");
                    }
                    /* if rif is not active, it is only in OM, not in MGR,
                     * we can still destroy it and return OK
                     */
                    rc = rc_om;

                    NETCFG_MGR_IP_SignalAllProtocolRifDelete(&inet_rif);

#if (SYS_CPNT_LLDP == TRUE)
                    if (rc == NETCFG_TYPE_OK)
                    {
                        LLDP_PMGR_NotifyRifChanged(rif_om.ifindex);
                    }
#endif
                    return rc;
                default:
                    return NETCFG_TYPE_FAIL;
            }
            break;

        default:
            break;
    }

    return NETCFG_TYPE_FAIL;
}

#if(SYS_CPNT_VIRTUAL_IP == TRUE)
/* FUNCTION NAME: NETCFG_MGR_IP_CalculateVirtualMAC
 * PURPOSE:
 *          Calculate virtual Mac
 * INPUT:
 *          ip_addr
 * OUTPUT:
 *          virtual MAC
 * RETURN:
 *          NETCFG_TYPE_OK
 *          NETCFG_TYPE_FAIL
 * NOTES:
 *          None.
 */
static UI32_T NETCFG_MGR_IP_CalculateVirtualMAC(UI8_T *ip_addr, UI8_T *virtual_mac)
{
    memset(virtual_mac, 0, SYS_ADPT_MAC_ADDR_LEN);
    /* format is 00:12:CF:00:00:XX, XX is the last byte of ip_addr */
    virtual_mac[0] = VIRTUAL_MAC_BYTE0;
    virtual_mac[1] = VIRTUAL_MAC_BYTE1;
    virtual_mac[2] = VIRTUAL_MAC_BYTE2;
    virtual_mac[3] = VIRTUAL_MAC_BYTE3;
    virtual_mac[4] = VIRTUAL_MAC_BYTE4;
    virtual_mac[5] = ip_addr[3];

    return NETCFG_TYPE_OK;
}
static UI32_T NETCFG_MGR_IP_InternalSetVirtualRifActive(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    UI8_T virtual_mac[SYS_ADPT_MAC_ADDR_LEN] = {};
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    L_PREFIX_IPv4_T vip_prefix;

    if(NETCFG_TYPE_FAIL == NETCFG_MGR_IP_CalculateVirtualMAC(rif_config_p->addr.addr, virtual_mac))
    {
        return NETCFG_TYPE_FAIL;
    }
    if (!AMTRL3_PMGR_AddL3Mac(virtual_mac, rif_config_p->ifindex))
    {
        DEBUG_PRINTF("Failed to add AMTRL3 L3Mac.\r\n");
        return NETCFG_TYPE_FAIL;
    }

    /* clear AMTRL3 dynamic ARP entry */
    memset(&host_entry, 0, sizeof(host_entry));
    host_entry.dst_inet_addr.type = VAL_InetAddressType_ipv4;
    memcpy(host_entry.dst_inet_addr.addr, rif_config_p->addr.addr, SYS_ADPT_IPV4_ADDR_LEN);
    DEBUG_PRINTF("Clear dynamic ARP[%u.%u.%u.%u]", L_INET_EXPAND_IPV4(rif_config_p->addr.addr));
    AMTRL3_PMGR_DeleteHostRoute(AMTRL3_TYPE_FLAGS_IPV4, 0, &host_entry);

    memset(&vip_prefix, 0, sizeof(vip_prefix));
    IP_LIB_ArraytoUI32(rif_config_p->addr.addr, &vip_prefix.prefix.s_addr);
    vip_prefix.prefixlen = 32;
    if(IPAL_RESULT_OK !=
            IPAL_VRRP_AddVrrpVirturalIp(
                rif_config_p->ifindex,
                SYS_ADPT_MAX_NBR_OF_VRRP_GROUP +1, /* 17 */
                &vip_prefix))
    {
        DEBUG_PRINTF("Failed to IPAL_VRRP_AddVrrpVirturalIp\r\n");
        return NETCFG_TYPE_FAIL;
    }
    return NETCFG_TYPE_OK;
}

static UI32_T NETCFG_MGR_IP_InternalSetVirtualRifDestroy(NETCFG_TYPE_InetRifConfig_T *rif_config_p)
{
    UI8_T virtual_mac[SYS_ADPT_MAC_ADDR_LEN] = {};
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    L_PREFIX_IPv4_T vip_prefix;

    if(NETCFG_TYPE_FAIL == NETCFG_MGR_IP_CalculateVirtualMAC(rif_config_p->addr.addr, virtual_mac))
    {
        return NETCFG_TYPE_FAIL;
    }
    if (!AMTRL3_PMGR_DeleteL3Mac(virtual_mac, rif_config_p->ifindex))
    {
        DEBUG_PRINTF("Failed to delete AMTRL3 L3Mac.\r\n");
        return FALSE;
    }

    memset(&vip_prefix, 0, sizeof(vip_prefix));
    IP_LIB_ArraytoUI32(rif_config_p->addr.addr, &vip_prefix.prefix.s_addr);
    vip_prefix.prefixlen = 32;
    if(IPAL_RESULT_OK !=
            IPAL_VRRP_DeleteVrrpVirturalIp(
                rif_config_p->ifindex,
                SYS_ADPT_MAX_NBR_OF_VRRP_GROUP +1, /* 17 */
                &vip_prefix))
    {

        DEBUG_PRINTF("Failed to IPAL_VRRP_DeleteVrrpVirturalIp\r\n");
        return NETCFG_TYPE_FAIL;
    }

    return NETCFG_TYPE_OK;
}
#endif
/* FUNCTION NAME: NETCFG_MGR_IP_InternalSetRifActive
 * PURPOSE:
 *          A local function to set active for an existing rif entry.
 * INPUT:
 *          rif_config_p
 *          rif_om_p
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          NETCFG_TYPE_OK
 *          NETCFG_TYPE_FAIL
 *          NETCFG_TYPE_INTERFACE_NOT_EXISTED
 * NOTES:
 *      1.  Called by NETCFG_MGR_IP_InternalSetIPv4Rif for C+W or Active.
 */
UI32_T NETCFG_MGR_IP_InternalSetRifActive(NETCFG_TYPE_InetRifConfig_T *rif_config_p,
                                                    NETCFG_OM_IP_InetRifConfig_T *rif_om_p)
{
    UI32_T rc = NETCFG_TYPE_FAIL;
#if(SYS_CPNT_VIRTUAL_IP == TRUE)

    if(rif_config_p->ipv4_role == NETCFG_TYPE_MODE_VIRTUAL)
    {
        rc = NETCFG_MGR_IP_InternalSetVirtualRifActive(rif_config_p);
        return rc;
    }
    else
#endif
    {
        rc = IP_MGR_AddInetRif(rif_config_p);
        if (rc != NETCFG_TYPE_OK)
            return rc;
    }
    rc = NETCFG_OM_IP_SetInetRifRowStatus(rif_om_p);
    if (rc != NETCFG_TYPE_OK)
        return rc;

    NETCFG_MGR_IP_SignalAllProtocolRifCreate(rif_config_p);
    return rc;
}


/* FUNCTION NAME : NETCFG_MGR_IP_DeletePrePrimaryIPv4Rif
 * PURPOSE:
 *      Delete previous Primary RIF entry
 *
 * INPUT:
 *      rif_config->ifindex     -- ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK  -- previous primary rif is not exist or delete successfully
 *      NETCFG_TYPE_XXX -- failed
 *
 * NOTES:
 *      None.
 */
static UI32_T NETCFG_MGR_IP_DeletePrePrimaryIPv4Rif(NETCFG_TYPE_InetRifConfig_T *rif_config)
{
    NETCFG_TYPE_InetRifConfig_T  inet_rif;
    NETCFG_OM_IP_InetRifConfig_T primary_rif;
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T                       state, transition_state, rc = NETCFG_TYPE_OK;
    BOOL_T                       has_secondary;

    if (rif_config->ipv4_role == NETCFG_TYPE_MODE_SECONDARY)
        return NETCFG_TYPE_OK;
#if(SYS_CPNT_VIRTUAL_IP == TRUE)
    if (rif_config->ipv4_role == NETCFG_TYPE_MODE_VIRTUAL)
        return NETCFG_TYPE_OK;
#endif
    /* Only handle add/update primary RIF */
    if (rif_config->row_status == VAL_netConfigStatus_2_destroy)
        return NETCFG_TYPE_OK;

    memset(&primary_rif, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));

    primary_rif.ifindex = rif_config->ifindex;
    if (NETCFG_OM_IP_LookupPrimaryRif(&primary_rif, rif_config->ifindex) == NETCFG_TYPE_FAIL)
    {
        return NETCFG_TYPE_OK;
    }

    /* Exact the same configuration, should be silently ignored. */
    if(!memcmp(&primary_rif.addr, &(rif_config->addr), sizeof(L_INET_AddrIp_T)))
        return NETCFG_TYPE_NO_CHANGE;

#if 0
    /* If it's a update of primary rif, there should be no secondary rif exists
     */
    /* row_status is c+g or c+w, ipv4_role is primary */
    memset(&rif_tmp, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    while (NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_tmp, rif_config->ifindex) == NETCFG_TYPE_OK)
    {
        if((rif_tmp.ipv4_role == VAL_iPAddrPrimaryInterface_secondary) &&
           (rif_tmp.row_status == VAL_netConfigStatus_2_active))
        {
            DEBUG_PRINTF("Must delete secondary rif first.\n");
            return NETCFG_TYPE_MUST_DELETE_SECONDARY_FIRST;
        }
    }
#endif

    has_secondary = FALSE;
    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    while (NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_om, rif_config->ifindex) == NETCFG_TYPE_OK)
    {
        if((rif_om.ipv4_role == VAL_iPAddrPrimaryInterface_secondary) &&
           (rif_om.row_status == VAL_netConfigStatus_2_active))
        {
            has_secondary = TRUE;
            break;
        }
    }

    if (!has_secondary)
    {
        primary_rif.row_status = VAL_netConfigStatus_2_destroy;
        return NETCFG_MGR_IP_InternalSetInetRif(&primary_rif);
    }
    else
    {
        /* When there is secondary address when we add a primary address,
         * NSM will preform an update operation on the original primary address,
         * So we only delete it from the OM (we can't delete the primary address
         * in NSM when there exist secondary address)
         */
        state = primary_rif.row_status;
        transition_state = L_RSTATUS_Fsm(VAL_netConfigStatus_2_destroy, &state, NETCFG_MGR_IP_CheckRifSemantic, &primary_rif);
        if (L_RSTATUS_NOT_EXIST == state)
            state = VAL_netConfigStatus_2_destroy;

        if (state == VAL_netConfigStatus_2_destroy)
        {
            memset(&inet_rif, 0, sizeof (NETCFG_TYPE_InetRifConfig_T));
            inet_rif.ifindex = primary_rif.ifindex;
            inet_rif.ipv4_role = primary_rif.ipv4_role;
            memcpy(&inet_rif.addr, &primary_rif.addr, sizeof(L_INET_AddrIp_T));
            NETCFG_MGR_IP_SignalAllProtocolRifDown(&inet_rif);

            rc = NETCFG_OM_IP_DeleteInetRif(&primary_rif);
            if(rc != NETCFG_TYPE_OK)
            {
                DEBUG_PRINTF("rif info corrupted at IPCFG_OM\n");
            }

            NETCFG_MGR_IP_SignalAllProtocolRifDelete(&inet_rif);

#if (SYS_CPNT_LLDP == TRUE)
            if (rc == NETCFG_TYPE_OK)
            {
                LLDP_PMGR_NotifyRifChanged(primary_rif.ifindex);
            }
#endif
        }

        return rc;
    }
}


/* FUNCTION NAME : NETCFG_MGR_IP_DeleteAllRifByIfindex
 * PURPOSE:
 *      Destory all IPv4 or IPv6 rif by ifindex (currenly is only for vid_ifIndex).
 *
 * INPUT:
 *      action_flags    --  IPv4 or IPv6
 *      ifindex     -- ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      For IPv4 or IPv6 or both at the same time.
 */
static UI32_T NETCFG_MGR_IP_DeleteAllRifByIfindex(UI8_T action_flags, UI32_T ifindex)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    NETCFG_TYPE_InetRifConfig_T rif_entry;

    memset(&rif_om, 0, sizeof (NETCFG_OM_IP_InetRifConfig_T));

    while (NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_om, ifindex) == NETCFG_TYPE_OK)
    {
        if(L_INET_IS_IPV4_ADDR_TYPE(rif_om.addr.type) && !CHECK_FLAG(action_flags, NETCFG_MGR_IP_FLAG_IPV4))
        {
            continue;
        }
        else if(L_INET_IS_IPV6_ADDR_TYPE(rif_om.addr.type) && !CHECK_FLAG(action_flags, NETCFG_MGR_IP_FLAG_IPV6))
        {
            continue;
        }

        rif_entry.ifindex = rif_om.ifindex;
        memcpy(&rif_entry.addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

        rif_entry.ipv4_role = rif_om.ipv4_role;
        // useless, rif_entry.row_status = rif_om.row_status;

        NETCFG_MGR_IP_SignalAllProtocolRifDown(&rif_entry);
#if(SYS_CPNT_VIRTUAL_IP == TRUE)
    if(rif_entry.ipv4_role == NETCFG_TYPE_MODE_VIRTUAL)
    {
        NETCFG_MGR_IP_InternalSetVirtualRifDestroy(&rif_entry);
    }
    else
#endif
    {
        IP_MGR_DeleteInetRif(&rif_entry);
    }
        NETCFG_OM_IP_DeleteInetRif(&rif_om);

        NETCFG_MGR_IP_SignalAllProtocolRifDelete(&rif_entry);
    }

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_IP_CheckRifSemantic
 * PURPOSE:
 *      Check whether rif validation
 *
 * INPUT:
 *      rif_om_ptr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      1. TODO: shall I check all fields?
 */
static BOOL_T NETCFG_MGR_IP_CheckRifSemantic(void *rif_om_ptr)
{
    NETCFG_OM_IP_InetRifConfig_T *rif_p = rif_om_ptr;
    NETCFG_TYPE_L3_Interface_T intf;
    UI32_T vid;

    if (0 == rif_p)
        return FALSE;

    memset(&intf, 0, sizeof(intf));
    intf.ifindex = rif_p->ifindex;
    NOTEprintf("search key=%ld", (long)intf.ifindex);
    if (NETCFG_TYPE_OK != NETCFG_OM_IP_GetL3Interface(&intf))
    {
        DBGprintf("fail to get L3Interface: ifindex=%ld", (long)intf.ifindex);
        return FALSE;
    }

    if (TRUE == IP_LIB_IsLoopbackInterface(intf.u.physical_intf.if_flags))
    {
        DBGprintf( "This is loopback interface");
        return TRUE;
    }
    VLAN_OM_ConvertFromIfindex(rif_p->ifindex, &vid);
    if (!VLAN_OM_IsVlanExisted(vid))
        return FALSE;

    return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceCreate
 * PURPOSE:
 *      Signal other CSCs that a l3ipvlan is created.
 *
 * INPUT:
 *      ifindex
 *      mtu
 *      bandwidth
 *      if_flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceCreate(UI32_T ifindex,
                            UI32_T mtu, UI32_T bandwidth, UI16_T if_flags)
{
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_SignalInterfaceAdd(ifindex);
#endif /* SYS_CPNT_RIP */
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_SignalInterfaceAdd(ifindex, mtu, bandwidth, if_flags);
#endif /* SYS_CPNT_OSPF */

    /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
    OSPF6_PMGR_SignalL3IfCreate(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, ifindex, mtu, bandwidth, if_flags);
#endif /* SYS_CPNT_OSPF6 */

#if (SYS_CPNT_BGP == TRUE)
    BGP_PMGR_SignalL3IfCreate(ifindex);
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
        (void)UDPHELPER_PMGR_L3IfCreate(ifindex);
#endif

    return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceDelete
 * PURPOSE:
 *      Signal other CSCs that a l3ipvlan is deleted.
 *
 * INPUT:
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceDelete(UI32_T ifindex)
{
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_SignalInterfaceDelete(ifindex);
#endif /* SYS_CPNT_RIP */
#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_SignalInterfaceDelete(ifindex);
#endif /* SYS_CPNT_OSPF */

    /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
    OSPF6_PMGR_SignalL3IfDestroy(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, ifindex);
#endif /* SYS_CPNT_OSPF6 */
#if (SYS_CPNT_BGP == TRUE)
    BGP_PMGR_SignalL3IfDestroy(ifindex);
#endif
#if (SYS_CPNT_UDP_HELPER == TRUE)
    (void)UDPHELPER_PMGR_L3IfDelete(ifindex);
#endif

    return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolRifCreate
 * PURPOSE:
 *      Signal other CSCs that an IPv4 RIF is created.
 *
 * INPUT:
 *      rif_p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolRifCreate(NETCFG_TYPE_InetRifConfig_T *rif_p)
{
    if (NULL == rif_p)
        return;

#if (SYS_CPNT_ND == TRUE)
    NETCFG_MGR_ND_SignalRifCreate(&(rif_p->addr));
#endif

#if (SYS_CPNT_OSPF == TRUE)
    if(rif_p->addr.type == L_INET_ADDR_TYPE_IPV4)
    {
        UI32_T ip_addr, ip_mask;
        UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN];

        IP_LIB_ArraytoUI32(rif_p->addr.addr, &ip_addr);
        IP_LIB_CidrToMask(rif_p->addr.preflen, byte_mask);
        IP_LIB_ArraytoUI32(byte_mask, &ip_mask);

        NETCFG_MGR_OSPF_SignalRifCreate(rif_p->ifindex, ip_addr, ip_mask, rif_p->ipv4_role);
    }
#endif

    /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
    if (L_INET_IS_IPV6_ADDR_TYPE(rif_p->addr.type))
    {
        OSPF6_PMGR_SignalL3IfRifCreate(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, rif_p->ifindex, &(rif_p->addr), rif_p->ipv4_role);
    }
#endif /* SYS_CPNT_OSPF6 */

#if (SYS_CPNT_UDP_HELPER == TRUE)
    if( rif_p->ipv4_role == NETCFG_TYPE_MODE_PRIMARY )
        (void)UDPHELPER_PMGR_RifCreate(rif_p->ifindex, rif_p->addr);
#endif

    if(L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type || L_INET_ADDR_TYPE_IPV4Z == rif_p->addr.type)
    {
#if (SYS_CPNT_BGP == TRUE)
        BGP_PMGR_SignalL3IfRifCreate(rif_p->ifindex, &(rif_p->addr));
#endif
    }

    SYS_CALLBACK_MGR_RifCreatedCallBack(SYS_MODULE_IPCFG, rif_p->ifindex, &(rif_p->addr));
    return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolRifDelete
 * PURPOSE:
 *      Signal other CSCs that an IPv4 RIF is deleted.
 *
 * INPUT:
 *      rif_p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolRifDelete(NETCFG_TYPE_InetRifConfig_T *rif_p)
{
    if (NULL == rif_p)
        return;

#if (SYS_CPNT_ND == TRUE)
    NETCFG_MGR_ND_SignalRifDestroy(&rif_p->addr);
#endif

#if (SYS_CPNT_OSPF == TRUE)
    if(rif_p->addr.type == L_INET_ADDR_TYPE_IPV4)
    {
        UI32_T ip_addr, ip_mask;
        UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN];

        IP_LIB_ArraytoUI32(rif_p->addr.addr, &ip_addr);
        IP_LIB_CidrToMask(rif_p->addr.preflen, byte_mask);
        IP_LIB_ArraytoUI32(byte_mask, &ip_mask);

        NETCFG_MGR_OSPF_SignalRifDelete(rif_p->ifindex, ip_addr, ip_mask, rif_p->ipv4_role);
    }

#endif /* SYS_CPNT_OSPF */

        /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
    OSPF6_PMGR_SignalL3IfRifDestroy(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, rif_p->ifindex, &(rif_p->addr));
#endif /* SYS_CPNT_OSPF6 */


#if (SYS_CPNT_UDP_HELPER == TRUE)
    if( rif_p->ipv4_role == NETCFG_TYPE_MODE_PRIMARY )
        (void)UDPHELPER_PMGR_RifDelete(rif_p->ifindex, rif_p->addr);
#endif

    if(L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type || L_INET_ADDR_TYPE_IPV4Z == rif_p->addr.type)
    {
#if (SYS_CPNT_BGP == TRUE)
        BGP_PMGR_SignalL3IfRifDestroy(rif_p->ifindex, &(rif_p->addr));
#endif
    }

#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_PMGR_SignalL3IfRifDestroy(rif_p->ifindex, &(rif_p->addr));
#endif

    SYS_CALLBACK_MGR_RifDestroyedCallBack(SYS_MODULE_IPCFG, rif_p->ifindex, &(rif_p->addr));
    return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolRifActive
 * PURPOSE:
 *      Signal other CSCs that a rif is active.
 *
 * INPUT:
 *      rif_p       -- pointer to the rif.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolRifActive(NETCFG_TYPE_InetRifConfig_T *rif_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    BOOL_T autoconfig = FALSE;
    UI32_T ipv6_mtu = 0;

    /* BODY
     */

    if(debug_flag)
    {
        printf("rif is active.\n");
        DUMP_RIF_ENTRY(rif_p);
    }

    /* In current design, there is no NSM in L2 products. Default gateway is added directly
     * into kernel. If there is no corresponding rif, add route to kernel will fail.
     * So we must check whether there is default gateway needed to add into kernel again
     * when a rif is activated
     */
#if (SYS_CPNT_NSM != TRUE)
    NETCFG_MGR_ROUTE_SignalRifUp(&rif_p->addr);
#endif

#if (SYS_CPNT_ND == TRUE)
    NETCFG_MGR_ND_SignalRifUp(rif_p->ifindex, &rif_p->addr);
#endif

#if (SYS_CPNT_OSPF == TRUE)
    if(rif_p->ipv4_role == NETCFG_TYPE_MODE_PRIMARY)
    {
        NETCFG_MGR_OSPF_SignalRifUp(rif_p->ifindex);
    }
#endif

#if (SYS_CPNT_OSPF6 == TRUE)
    if(rif_p->addr.type == L_INET_ADDR_TYPE_IPV6Z)
        OSPF6_PMGR_SignalL3IfUp(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, rif_p->ifindex);
#endif

    if(L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type || L_INET_ADDR_TYPE_IPV4Z == rif_p->addr.type)
    {
        UI32_T ip_addr, ip_mask;
        UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN];

        IP_LIB_ArraytoUI32(rif_p->addr.addr, &ip_addr);
        IP_LIB_CidrToMask(rif_p->addr.preflen, byte_mask);
        IP_LIB_ArraytoUI32(byte_mask, &ip_mask);

#if (SYS_CPNT_RIP == TRUE)
        NETCFG_MGR_RIP_SignalRifUp(rif_p->ifindex, ip_addr, ip_mask);
#endif
#if (SYS_CPNT_BGP == TRUE)
        BGP_PMGR_SignalL3IfRifUp(rif_p->ifindex, &(rif_p->addr));
#endif
#if (SYS_CPNT_PBR == TRUE)
        NETCFG_MGR_PBR_SignalL3IfRifUp(rif_p->ifindex, &(rif_p->addr));
#endif
    }

#if (SYS_CPNT_PROXY_ARP == TRUE)
    /* Because only vlan with a ip address is up,proxy-arp can be enabled successfully in TCP/IP  stack.
           Set proxy-arp enable to TCP/IP stack --xiongyu 20081106*/
    {
        NETCFG_TYPE_L3_Interface_T intf;
        memset(&intf, 0, sizeof(intf));
        intf.ifindex = rif_p->ifindex;
        if(NETCFG_OM_IP_GetL3Interface(&intf) == NETCFG_TYPE_OK)
        {
            if(intf.u.physical_intf.proxy_arp_enable == TRUE)
            {
                IP_MGR_SetIpNetToMediaProxyStatus(intf.ifindex, intf.u.physical_intf.proxy_arp_enable);
            }
        }
    }
#endif

    /* to avoid configuraion lost, set into kernel again
     */
    if(rif_p->addr.type == L_INET_ADDR_TYPE_IPV6Z)
    {
        /* ipv6 address autoconfig */
        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus(rif_p->ifindex, &autoconfig))
        {
            if(autoconfig)
                IP_MGR_IPv6AddrAutoconfigEnable(rif_p->ifindex);
            else
                IP_MGR_IPv6AddrAutoconfigDisable(rif_p->ifindex);
        }

        /* ipv6 mtu */
        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6InterfaceMTU(rif_p->ifindex, &ipv6_mtu))
        {
            NETCFG_MGR_IP_SetIPv6InterfaceMTU(rif_p->ifindex, ipv6_mtu);
        }
    }
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    /* notify routecfg to update static routes on craft interface */
    if((rif_p->addr.type == L_INET_ADDR_TYPE_IPV4)||
       (rif_p->addr.type == L_INET_ADDR_TYPE_IPV4Z))
    {
        NETCFG_MGR_ROUTE_UpdateStaticRoute(rif_p->ifindex, &(rif_p->addr), TRUE);
    }
#endif
    /* signal sys_callback */
    /* Because all XXX_CFG are communicated with other CSC via NETCFG MGR queue,
       therefore, the CSC_ID is filled with SYS_MODULE_NETCFG */
    SYS_CALLBACK_MGR_RifActiveCallBack(SYS_MODULE_IPCFG, rif_p->ifindex, &(rif_p->addr));


     return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolRifDown
 * PURPOSE:
 *      Signal other CSCs that a rif is down.
 *
 * INPUT:
 *      rif_p       -- pointer to the rif.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolRifDown(NETCFG_TYPE_InetRifConfig_T *rif_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */

    /* BODY
     */

    if(debug_flag)
    {
        printf("rif is down.\n");
        DUMP_RIF_ENTRY(rif_p);
    }

#if (SYS_CPNT_ND == TRUE)
    NETCFG_MGR_ND_SignalRifDown(rif_p->ifindex, &rif_p->addr);
#endif

#if (SYS_CPNT_OSPF == TRUE)
    if(rif_p->ipv4_role == NETCFG_TYPE_MODE_PRIMARY)
    {
        NETCFG_MGR_OSPF_SignalRifDown(rif_p->ifindex);
    }
#endif

    if(L_INET_ADDR_TYPE_IPV4 == rif_p->addr.type || L_INET_ADDR_TYPE_IPV4Z == rif_p->addr.type)
    {
        UI32_T ip_addr, ip_mask;
        UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN];

        IP_LIB_ArraytoUI32(rif_p->addr.addr, &ip_addr);
        IP_LIB_CidrToMask(rif_p->addr.preflen, byte_mask);
        IP_LIB_ArraytoUI32(byte_mask, &ip_mask);

#if (SYS_CPNT_RIP == TRUE)
        NETCFG_MGR_RIP_SignalRifDown(rif_p->ifindex, ip_addr, ip_mask);
#endif
#if (SYS_CPNT_BGP == TRUE)
        BGP_PMGR_SignalL3IfRifDown(rif_p->ifindex, &(rif_p->addr));
#endif
#if (SYS_CPNT_PBR == TRUE)
        NETCFG_MGR_PBR_SignalL3IfRifDown(rif_p->ifindex, &(rif_p->addr));
#endif
    }

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    /* notify routecfg to update static routes on craft interface */
    if((rif_p->addr.type == L_INET_ADDR_TYPE_IPV4)||
       (rif_p->addr.type == L_INET_ADDR_TYPE_IPV4Z))
    {
        NETCFG_MGR_ROUTE_UpdateStaticRoute(rif_p->ifindex, &(rif_p->addr), FALSE);
    }
#endif

    /* signal sys_callback */
    /* Because all XXX_CFG are communicated with other CSC via NETCFG MGR queue,
       therefore, the CSC_ID is filled with SYS_MODULE_NETCFG */
     SYS_CALLBACK_MGR_RifDownCallBack(SYS_MODULE_IPCFG, rif_p->ifindex, &(rif_p->addr));
     return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_SignalInterfaceStatus
 * PURPOSE:
 *      Signal other CSC of NETCFG about interface's status.
 *
 * INPUT:
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 */
static void NETCFG_MGR_IP_SignalInterfaceStatus(UI32_T ifindex, BOOL_T status)
{
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    NETCFG_TYPE_InetRifConfig_T ipv4_rif;

    if (status == INTERFACE_STATUS_UP)
    {
        /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
        // peter, move to ipv6 rif active, OSPF6_PMGR_SignalL3IfUp(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, ifindex);
#endif
#if (SYS_CPNT_RIP == TRUE)
        NETCFG_MGR_RIP_SignalInterfaceUp(ifindex);
#endif
#if (SYS_CPNT_BGP == TRUE)
        BGP_PMGR_SignalL3IfUp(ifindex);
#endif
#if (SYS_CPNT_ND == TRUE)
        NETCFG_MGR_ND_SignalL3IfUp(ifindex);
#endif
    }
    else if (status == INTERFACE_STATUS_DOWN)
    {
        /* added by steven.gao for OSPFv3 */
#if (SYS_CPNT_OSPF6 == TRUE)
        OSPF6_PMGR_SignalL3IfDown(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, ifindex);
#endif
#if (SYS_CPNT_RIP == TRUE)
        NETCFG_MGR_RIP_SignalInterfaceDown(ifindex);
#endif
#if (SYS_CPNT_BGP == TRUE)
        BGP_PMGR_SignalL3IfDown(ifindex);
#endif
    }

    memset(&rif_om, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
    rif_om.ifindex = ifindex;
    while(NETCFG_OM_IP_GetNextInetRifByIfindex(&rif_om, rif_om.ifindex) == NETCFG_TYPE_OK)
    {
        if (rif_om.row_status != VAL_netConfigStatus_2_active)
            break;

        memset(&ipv4_rif, 0, sizeof (NETCFG_TYPE_InetRifConfig_T));

        memcpy(&ipv4_rif.addr, &rif_om.addr, sizeof(L_INET_AddrIp_T));

        ipv4_rif.ifindex = rif_om.ifindex;
        ipv4_rif.ipv4_role = rif_om.ipv4_role;

        if (status == INTERFACE_STATUS_DOWN)
            NETCFG_MGR_IP_SignalAllProtocolRifDown(&ipv4_rif);
        /* Don't call NETCFG_MGR_IP_SignalAllProtocolRifActive() here because the RIF is not
         * truly UP at this moment. It will be called in NETCFG_MGR_IP_IpalRifReflection_CallBack().
         */
        /*else
            NETCFG_MGR_IP_SignalAllProtocolRifActive(&ipv4_rif);*/
    }

    return;
}

/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceCreate
 * PURPOSE:
 *      Signal other CSCs that a loopback interface is created.
 *
 * INPUT:
 *      ifindex
 *      if_flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceCreate(UI32_T ifindex,
                            UI16_T if_flags)
{
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_MGR_RIP_SignalLoopbackInterfaceAdd(ifindex, if_flags);
#endif /* SYS_CPNT_RIP */

#if (SYS_CPNT_OSPF == TRUE)
    NETCFG_MGR_OSPF_SignalLoopbackInterfaceAdd(ifindex, if_flags);
#endif /* SYS_CPNT_OSPF */

    return;
}

/* FUNCTION NAME : NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceDelete
 * PURPOSE:
 *      Signal other CSCs that a loopback interface is deleted.
 *
 * INPUT:
 *      ifindex
 *      if_flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceDelete(UI32_T ifindex)
{
#if (SYS_CPNT_RIP == TRUE)
        NETCFG_MGR_RIP_SignalInterfaceDelete(ifindex);
#endif /* SYS_CPNT_RIP */

#if (SYS_CPNT_OSPF == TRUE)
        NETCFG_MGR_OSPF_SignalInterfaceDelete(ifindex);
#endif /* SYS_CPNT_OSPF */

    return;
}

/*fuzhimin,20090218*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
static void NETCFG_OM_IP_DeleteEth0IpAddr()
{
    char ifname [20];
    struct ifreq ifr;
    int sock;

    /* build interface name */
    bzero (ifname, sizeof (ifname));
    sprintf (ifname, "%s%d", "eth", 0);
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);

    sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        return;
    }

    ioctl (sock, SIOCGIFFLAGS, (caddr_t) &ifr);

    ifr.ifr_flags &= ~IFF_UP;
    ioctl (sock, SIOCSIFFLAGS, (caddr_t) &ifr);

    close (sock);
    return ;
}
#endif
/*fuzhimin,20090218,end*/

/* FUNCTION NAME : NETCFG_MGR_IP_SetSingleManagementVlan
 * PURPOSE:
 *      Set the single management vlan on a specific ifindex and delete others if exist
 *
 * INPUT:
 *      ifindex     -- ifindex of the new management Vlan
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *      1. For L2 switch, Only one L3If with ip address is allowed, which is the management Vlan.
 *         We will delete L3If on the old mgmt Vlan and change the Vlan type to L2.
 */
static UI32_T NETCFG_MGR_IP_SetSingleManagementVlan(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T  vid, vid_tmp;
    UI8_T   cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    NETCFG_TYPE_L3_Interface_T intf;
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T rc;
    AMTR_TYPE_Ret_T ret;
    /* BODY
     */
    VLAN_OM_ConvertFromIfindex(ifindex, &vid);

    memset(&intf, 0, sizeof(intf));
    memset(&rif_om, 0, sizeof(rif_om));

    /* Delete other L3If and leave mgmt vlan */
    while(NETCFG_OM_IP_GetNextL3Interface(&intf) == NETCFG_TYPE_OK)
    {
        if(intf.ifindex == ifindex)
            continue;

        if (FALSE == VLAN_OM_ConvertFromIfindex(intf.ifindex, &vid_tmp))
            continue;

        DEBUG_PRINTF("%s, %d, delete L3If on vlan %ld.\n", __FUNCTION__, __LINE__, (long)vid_tmp);

        NETCFG_MGR_IP_DeleteL3If(vid_tmp);

        /* Delete old Vlan MAC from chip */
        VLAN_PMGR_GetVlanMac(intf.ifindex, cpu_mac);
        AMTR_PMGR_DeleteCpuMac(vid_tmp, cpu_mac);

        /* Update old Vlan address mode to USER_DEFINE (for IPv4) */
        VLAN_PMGR_SetVlanAddressMethod(vid_tmp, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE);
        NETCFG_OM_IP_SetIpAddressMode(intf.ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE);
        SYS_CALLBACK_MGR_DHCPSetIfStatusCallback(SYS_MODULE_IPCFG, intf.ifindex, DHCP_MGR_CLIENT_DOWN);
    }

    /* Add new management Vlan */
#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
    if (VLAN_PMGR_SetIpInterface (vid)==FALSE)
        return(NETCFG_TYPE_INTERFACE_NOT_EXISTED);
#else
    if (VLAN_PMGR_SetManagementVlan(vid)==FALSE)
        return(NETCFG_TYPE_INTERFACE_NOT_EXISTED);
#endif
    /* Add new Vlan MAC into chip */
    VLAN_PMGR_GetVlanMac(ifindex, cpu_mac);

    /* this function is called only when SYS_CPNT_ROUTING != TRUE
     */
    ret = AMTR_PMGR_SetCpuMac(vid, cpu_mac, FALSE);

    switch(ret)
    {
        case AMTR_TYPE_RET_SUCCESS:
            rc = NETCFG_TYPE_OK;
            break;
        case AMTR_TYPE_RET_COLLISION:
            rc = NETCFG_TYPE_MAC_COLLISION;
            break;
        default:
            rc = NETCFG_TYPE_FAIL;
            break;
    }
    return rc;
}

/* FUNCTION NAME : NETCFG_MGR_IP_CreateDefaultLoopbackInterface
 * PURPOSE:
 *      Create the default loopback interface.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void NETCFG_MGR_IP_CreateDefaultLoopbackInterface()
{
    NETCFG_TYPE_L3_Interface_T lo_intf;

    memset(&lo_intf, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
    lo_intf.ifindex = SYS_ADPT_LOOPBACK_IF_INDEX_BASE;
    lo_intf.u.physical_intf.if_flags = (IFF_UP | IFF_RUNNING | IFF_LOOPBACK);
    lo_intf.u.physical_intf.ipv4_address_mode = NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE;
    lo_intf.iftype = VLAN_L3_IP_IFTYPE;
    NETCFG_OM_IP_CreateL3Interface(&lo_intf);

    /* Notify the loopback interface */
    NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceCreate(lo_intf.ifindex, lo_intf.u.physical_intf.if_flags);
}

/* FUNCTION NAME : NETCFG_MGR_IP_CreateLoopbackInterface
 * PURPOSE:
 *      Create a Loopback interface.
 *
 * INPUT:
 *      lo_id  -- loopback id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *      None.
 */
static UI32_T NETCFG_MGR_IP_CreateLoopbackInterface(UI32_T lo_id)
{
    UI32_T ifindex;
    NETCFG_TYPE_L3_Interface_T lo_intf;
    IP_MGR_Interface_T ip_intf;
    UI32_T rc;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Check if it's a created interface */
    memset(&lo_intf, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
    memset(&ip_intf, 0, sizeof (IP_MGR_Interface_T));

    lo_intf.ifindex = lo_id+SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER;

    if (NETCFG_OM_IP_GetL3Interface(&lo_intf) == NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_OK;
    }

    lo_intf.u.physical_intf.if_flags = (IFF_UP | IFF_RUNNING | IFF_LOOPBACK);
    lo_intf.u.physical_intf.ipv4_address_mode = NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE;
    lo_intf.u.physical_intf.mtu = SYS_ADPT_IF_MTU;
    lo_intf.iftype = VLAN_L3_IP_IFTYPE;

    ip_intf.ifindex = lo_intf.ifindex;
    ip_intf.u.physical_intf.mtu = lo_intf.u.physical_intf.mtu;
    ip_intf.u.physical_intf.if_flags = lo_intf.u.physical_intf.if_flags;

     if((rc = IP_MGR_CreateLoopbackInterface(&ip_intf)) != NETCFG_TYPE_OK)
    {
        DBGprintf("Fail to IP_MGR_CreateLoopbackInterface, rc=%lu", rc);
        return rc;
    }

    if (NETCFG_OM_IP_CreateL3Interface(&lo_intf) != NETCFG_TYPE_OK)
    {
        DBGprintf("Fail to NETCFG_OM_IP_CreateL3Interface");
        return NETCFG_TYPE_FAIL;
    }

    /* Notify the loopback interface */
    NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceCreate(lo_intf.ifindex, lo_intf.u.physical_intf.if_flags);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_IP_DeleteLoopbackInterface
 * PURPOSE:
 *      Delete a Loopback interface,
 *
 * INPUT:
 *      lo_id -- loopback id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      Should delete all rif entries of the interface.
 */
static UI32_T NETCFG_MGR_IP_DeleteLoopbackInterface(UI32_T lo_id)
{
    UI32_T ifindex;
    NETCFG_TYPE_L3_Interface_T lo_intf;
    UI8_T action_flags = NETCFG_MGR_IP_FLAG_IPV4 | NETCFG_MGR_IP_FLAG_IPV6;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* Check if it's a created interface */
    memset(&lo_intf, 0, sizeof (lo_intf));

    lo_intf.ifindex = lo_id+SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER;

    if (NETCFG_OM_IP_GetL3Interface(&lo_intf) != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_ENTRY_NOT_EXIST;
    }

    /* Delete all rif entries */
    if (NETCFG_MGR_IP_DeleteAllRifByIfindex(action_flags, lo_intf.ifindex) != NETCFG_TYPE_OK)
    {printf("%s, %d. X DeleteAllRifByIfindex\n", __FUNCTION__, __LINE__);
        return NETCFG_TYPE_FAIL;
    }

    if (IP_MGR_DeleteLoopbackInterface(lo_intf.ifindex) == FALSE)
    {printf("%s, %d. X DeleteLoopbackInterface\n", __FUNCTION__, __LINE__);
        return NETCFG_TYPE_FAIL;
    }

    if (NETCFG_OM_IP_DeleteL3Interface(&lo_intf) != NETCFG_TYPE_OK)
    {printf("%s, %d. X DeleteL3Interface\n", __FUNCTION__, __LINE__);
        return NETCFG_TYPE_FAIL;
    }

    NETCFG_MGR_IP_SignalAllProtocolLoopbackInterfaceDelete(lo_intf.ifindex);

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_IP_DeleteAllInterface
 * PURPOSE:
 *      Delete All l3ipvlan configuration.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 */
static void NETCFG_MGR_IP_DeleteAllInterface(void)
{
    NETCFG_TYPE_L3_Interface_T intf;

    memset(&intf, 0, sizeof(intf));

    while(NETCFG_OM_IP_GetNextL3Interface(&intf) == NETCFG_TYPE_OK)
    {
        /* Delete all rif entries */
        NETCFG_MGR_IP_DeleteAllRifByIfindex(NETCFG_MGR_IP_FLAG_IPV4V6, intf.ifindex);

        /* Delete interface information from low-level modules */
        IP_MGR_DeleteInterface(intf.ifindex);
        /* Delete interface information from other protocols that in netcfg */
        NETCFG_MGR_IP_SignalAllProtocolVlanInterfaceDelete(intf.ifindex);
    }

    /* Delete interface from database */
    NETCFG_OM_IP_DeleteAllInterface();

/*fuzhimin,20090218*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
    NETCFG_OM_IP_DeleteEth0IpAddr();
#endif
/*fuzhimin,20090218,end*/
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : NETCFG_MGR_IP_SetTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_IP_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_EnterTransitionMode
 * PURPOSE:
 *      Enter transition mode, releasing all allocateing resource in master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_IP_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();

    NETCFG_MGR_IP_DeleteAllInterface();

    /* seems kernel will not clear the ipv6 statistics,
     * so record the current value when entering transition mode
     */
#if (SYS_CPNT_IPV6 == TRUE)
    {
        IPAL_Ipv6Statistics_T       ip6_base = {0};
        IPAL_Icmpv6Statistics_T     icmp6_base = {0};
        IPAL_Udpv6Statistics_T      udp6_base = {0};

        IPAL_IF_GetAllIpv6Statistic(&ip6_base);
        IPAL_IF_GetAllIcmp6Statistic(&icmp6_base);
        IPAL_IF_GetAllUdp6Statistic(&udp6_base);

        NETCFG_OM_IP_SetIPv6StatBaseCntr(&ip6_base);
        NETCFG_OM_IP_SetICMPv6StatBaseCntr(&icmp6_base);
        NETCFG_OM_IP_SetUDPv6StatBaseCntr(&udp6_base);
    }
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    /* in AOS, CRAFT port's address could be decided by dhcp or static set in /etc/network/interfaces
     * so we should not delete it
     */
    /* NETCFG_MGR_IP_DeleteAllCraftInterfaceInetAddress(); */
#endif

}


/* FUNCTION NAME : NETCFG_MGR_IP_EnterMasterMode
 * PURPOSE:
 *      Enter master mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void NETCFG_MGR_IP_EnterMasterMode (void)
{
#if (SYS_CPNT_IPV6 == TRUE)
    IPAL_IF_SetIpv6DefaultAutoconfig(SYS_DFLT_IPV6_ADDR_AUTOCONFIG);
#endif
    SYSFUN_ENTER_MASTER_MODE();

    NETCFG_MGR_IP_CreateDefaultLoopbackInterface();
}


/* FUNCTION NAME : NETCFG_MGR_IP_EnterSlaveMode
 * PURPOSE:
 *      Enter slave mode.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_IP_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* FUNCTION NAME : NETCFG_MGR_IP_ProvisionComplete
 * PURPOSE:
 *      notify Provision Complete
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void NETCFG_MGR_IP_ProvisionComplete(void)
{
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    NETCFG_MGR_IP_SyncKernelCraftInterfaceInetAddress();
#endif
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_IP_HandleHotInsertionForL3If
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will invoke a new dut insertion in L3IF of NETCFG_MGR_IP.
 *
 * INPUT:
 *    starting_port_ifindex  -- starting port ifindex
 *    number_of_port         -- number of ports
 *    use_default            -- whether use default setting
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void NETCFG_MGR_IP_HandleHotInsertionForL3If(UI32_T starting_port_ifindex,
                                            UI32_T number_of_port,
                                            BOOL_T use_default)
{
#if (SYS_CPNT_ROUTING == TRUE) && (SYS_CPNT_SWDRVL3 == TRUE)
    NETCFG_TYPE_L3_Interface_T l3if;
    UI32_T  vid;

    memset(&l3if, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
    while(NETCFG_OM_IP_GetNextL3Interface(&l3if) == NETCFG_TYPE_OK)
    {
        if(FALSE == VLAN_OM_ConvertFromIfindex(l3if.ifindex, &vid))
            continue;

        SWDRVL3_HotInsertCreateL3Interface(SYS_ADPT_DEFAULT_FIB, vid, l3if.u.physical_intf.hw_addr, l3if.drv_l3_intf_index);
    }
#endif
}
#endif


/* FUNCTION NAME : NETCFG_MGR_IP_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void NETCFG_MGR_IP_Create_InterCSC_Relation(void)
{
#ifdef  BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ipcfg",
                                                      SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
                                                      NETCFG_MGR_IP_BackDoorMain);
#endif
}


 /* FUNCTION NAME : NETCFG_MGR_IP_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_IP used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_IP_InitiateProcessResources(void)
{
    NETCFG_OM_IP_InitateProcessResources();
    IP_MGR_InitiateProcessResources();

    return TRUE;
}


/* FUNCTION NAME : NETCFG_MGR_IP_L3InterfaceDestory_CallBack
 * PURPOSE:
 *     Handle the callback message for L3 interface is deleted.
 *
 * INPUT:
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 */
void NETCFG_MGR_IP_L3InterfaceDestory_CallBack(UI32_T ifindex)
{
    NETCFG_TYPE_L3_Interface_T vlan_intf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    /* Check if it's a created interface */
    memset(&vlan_intf, 0, sizeof (vlan_intf));
    vlan_intf.ifindex = ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&vlan_intf) != NETCFG_TYPE_OK)
        return;

    /* Delete all rif entries */
    NETCFG_MGR_IP_DeleteAllRifByIfindex(NETCFG_MGR_IP_FLAG_IPV4V6, vlan_intf.ifindex);

    IP_MGR_DeleteInterface(vlan_intf.ifindex);
#if (SYS_CPNT_IPV6 == TRUE)
    /* L3 interafce destroy call back notification, for DHCPv6
     */
    SYS_CALLBACK_MGR_NETCFG_L3IfDestroy(SYS_MODULE_IPCFG, ifindex);
#endif
    NETCFG_OM_IP_DeleteL3Interface(&vlan_intf);

    return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_L3IfOperStatusChanged_CallBack
 * PURPOSE:
 *      Handle the callback message for L3 interface operation status change.
 *
 * INPUT:
 *      vid_ifindex -- interface index
 *      status : VAL_ifOperStatus_up, interface up.
 *               VAL_ifOperStatus_down, interface down.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_MGR_IP_L3IfOperStatusChanged_CallBack(UI32_T ifindex, UI32_T oper_status)
{
    UI32_T vid;
    NETCFG_TYPE_L3_Interface_T vlan_intf;
    UI16_T flags = IFF_RUNNING;
#if (SYS_CPNT_IPV6 == TRUE)
    BOOL_T autoconfig = FALSE;
#endif
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return;

    VLAN_OM_ConvertFromIfindex (ifindex, &vid);
    if (FALSE == VLAN_OM_IsVlanExisted (vid))
        return;

    /* Check if it's a created interface */
    memset(&vlan_intf, 0, sizeof (vlan_intf));
    VLAN_OM_ConvertToIfindex(vid, &ifindex);
    vlan_intf.ifindex = ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&vlan_intf) != NETCFG_TYPE_OK)
        return;

    if (oper_status == VAL_ifOperStatus_up)
    {
        /* Notify other L3 protocols */
        if (IP_MGR_SetIfFlags(vlan_intf.ifindex, flags) == FALSE)
            return;

        if ((UI32_T)NETCFG_OM_IP_SetInterfaceFlags(&vlan_intf, flags) !=  NETCFG_TYPE_OK)
            return;

        /* Notify other CSC module of NETCFG */
        NETCFG_MGR_IP_SignalInterfaceStatus(vlan_intf.ifindex, INTERFACE_STATUS_UP);

#if (SYS_CPNT_IPV6 == TRUE)
        /* L3 interafce oper status up call back notification
         * and the status of IPv6AddrAutoconfig, for DHCPv6
         */
        if (NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus(ifindex,&autoconfig) == NETCFG_TYPE_OK)
        {
#if (SYS_CPNT_DHCPV6 == TRUE)
            SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusUp(SYS_MODULE_IPCFG, ifindex, autoconfig);
#endif
        }

#endif

        /* If NSM is not supported, we must take care the jobs done in nsm_if_addr_wakeup()
         * This function add all ipv4/v6 addresses back to kernel, because kernel will
         * remove all ipv6 addresses automatically when interface is down
         */
        {
            NETCFG_TYPE_InetRifConfig_T rif_config;

            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = ifindex;
            while (NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif_config))
            {
                IP_MGR_AddInetRif(&rif_config);
            }
        }
    }
    else
    {
        NETCFG_MGR_IP_SignalInterfaceStatus(vlan_intf.ifindex, INTERFACE_STATUS_DOWN);

        if (IP_MGR_UnsetIfFlags(vlan_intf.ifindex, flags) == FALSE)
            return;

        if (NETCFG_OM_IP_UnsetInterfaceFlags(&vlan_intf, flags) != NETCFG_TYPE_OK)
            return;
#if (SYS_CPNT_IPV6 == TRUE)
        /* L3 interafce oper status down call back notification, for DHCPv6
         */
#if (SYS_CPNT_DHCPV6 == TRUE)
        SYS_CALLBACK_MGR_NETCFG_L3IfOperStatusDown(SYS_MODULE_IPCFG, ifindex);
#endif
#endif
    }

    return;
}


/* FUNCTION NAME : NETCFG_MGR_IP_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_MGR_IP.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T NETCFG_MGR_IP_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    NETCFG_MGR_IP_IPCMsg_T *netcfg_mgr_ip_msg_p;
    BOOL_T need_respond=TRUE;

    if(ipcmsg_p==NULL)
        return FALSE;

    netcfg_mgr_ip_msg_p = (NETCFG_MGR_IP_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        netcfg_mgr_ip_msg_p->type.result_bool = FALSE;
        ipcmsg_p->msg_size = sizeof(netcfg_mgr_ip_msg_p->type);
        return TRUE;
    }

    switch(netcfg_mgr_ip_msg_p->type.cmd)
    {
        /* IP Configuration */
        case NETCFG_MGR_IP_IPCCMD_CREATEL3IF:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_CreateL3If(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_IP_IPCCMD_CREATELOOPBACKIF:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_CreateLoopbackInterface(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_IP_IPCCMD_DELETEL3IF:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_DeleteL3If(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_IP_IPCCMD_DELETELOOPBACKIF:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_DeleteLoopbackInterface(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_IP_IPCCMD_SETIPADDRESSMODE:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_SetIpAddressMode(
                netcfg_mgr_ip_msg_p->data.u32a1_ip_addr_mode.u32_a1,
                netcfg_mgr_ip_msg_p->data.u32a1_ip_addr_mode.ip_addr_mode);
            ipcmsg_p->msg_size=NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case NETCFG_MGR_IP_IPCCMD_SETINETRIF:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_SetInetRif(
                &(netcfg_mgr_ip_msg_p->data.inet_rif_config_u32a1.inet_rif_config),
                netcfg_mgr_ip_msg_p->data.inet_rif_config_u32a1.u32_a1);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#if (SYS_CPNT_PROXY_ARP == TRUE)
        case NETCFG_MGR_IP_IPCCMD_SETARPPROXYSTATUS:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_SetIpNetToMediaProxyStatus(
                netcfg_mgr_ip_msg_p->data.arp_proxy_status.ifindex,
                netcfg_mgr_ip_msg_p->data.arp_proxy_status.status);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETRUNNINGARPPROXYSTATUS:
            netcfg_mgr_ip_msg_p->type.result_running_cfg = NETCFG_MGR_IP_GetRunningIpNetToMediaProxyStatus(
                netcfg_mgr_ip_msg_p->data.arp_proxy_status.ifindex,
                &(netcfg_mgr_ip_msg_p->data.arp_proxy_status.status));
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(arp_proxy_status);
            need_respond = TRUE;
            break;
#endif

#if (SYS_CPNT_IPV6 == TRUE)
        case NETCFG_MGR_IP_IPCCMD_IPV6ENABLE:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_IPv6Enable(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_IPV6DISABLE:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_IPv6Disable(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_IPV6ADDRAUTOCONFIGENABLE:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_IPv6AddrAutoconfigEnable(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_IPV6ADDRAUTOCONFIGDISABLE:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_IPv6AddrAutoconfigDisable(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_IPV6SETINTERFACEMTU:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_SetIPv6InterfaceMTU(
                netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a1, netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_IPV6UNSETINTERFACEMTU:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_UnsetIPv6InterfaceMTU(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_IPV6GETINTERFACEMTU:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_GetIPv6InterfaceMTU(
                netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a1, &netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_a1_a2);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETNEXTIFJOINIPV6MCASTADDR:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_GetNextIfJoinIpv6McastAddr(netcfg_mgr_ip_msg_p->data.u32a1_addra2.u32_a1, &(netcfg_mgr_ip_msg_p->data.u32a1_addra2.addr_a2));
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_addra2);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETNEXTPMTUENTRY:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_GetNextPMTUEntry(&(netcfg_mgr_ip_msg_p->data.pmtu_entry_v));
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(pmtu_entry_v);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETIFIPV6ADDRINFO:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_GetIfIpv6AddrInfo(&(netcfg_mgr_ip_msg_p->data.ip_addr_info_v));
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ip_addr_info_v);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_DHCPRELEASECOMPLETE:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_DhcpReleaseComplete(netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a1, netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_a1_a2);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6ENABLESTATUS:
            netcfg_mgr_ip_msg_p->type.result_running_cfg = NETCFG_MGR_IP_GetRunningIPv6EnableStatus(netcfg_mgr_ip_msg_p->data.u32a1_bla2.u32_a1, &netcfg_mgr_ip_msg_p->data.u32a1_bla2.bl_a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6ADDRAUTOCONFIGENABLESTATUS:
            netcfg_mgr_ip_msg_p->type.result_running_cfg = NETCFG_MGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus(netcfg_mgr_ip_msg_p->data.u32a1_bla2.u32_a1, &netcfg_mgr_ip_msg_p->data.u32a1_bla2.bl_a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6INTERFACEMTU:
            netcfg_mgr_ip_msg_p->type.result_running_cfg = NETCFG_MGR_IP_GetRunningIPv6InterfaceMTU(netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a1, &netcfg_mgr_ip_msg_p->data.ui32_a1_a2.a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ui32_a1_a2);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETIPV6STATISTICS:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_GetIPv6Statistics(
                &netcfg_mgr_ip_msg_p->data.ip6_stat);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(ip6_stat);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETICMPV6STATISTICS:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_GetICMPv6Statistics(
                &netcfg_mgr_ip_msg_p->data.icmp6_stat);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(icmp6_stat);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_GETUDPV6STATISTICS:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_GetUDPv6Statistics(
                &netcfg_mgr_ip_msg_p->data.udp6_stat);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(udp6_stat);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_CLEARIPV6STATISTICSBYTYPE:
            netcfg_mgr_ip_msg_p->type.result_ui32 = NETCFG_MGR_IP_ClearIPv6StatisticsByType(
                netcfg_mgr_ip_msg_p->data.ui32_v);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
#if (SYS_CPNT_CLUSTER == TRUE)
        case NETCFG_MGR_IP_IPCCMD_CLUSTERVLANSETRIFROWSTATUS:
            netcfg_mgr_ip_msg_p->type.result_ui32 =
                NETCFG_MGR_IP_ClusterVlanSetRifRowStatus(netcfg_mgr_ip_msg_p->data.cluster_vlan_rif.ipAddress,
                                                         netcfg_mgr_ip_msg_p->data.cluster_vlan_rif.ipMask,
                                                         netcfg_mgr_ip_msg_p->data.cluster_vlan_rif.rowStatus,
                                                         netcfg_mgr_ip_msg_p->data.cluster_vlan_rif.incoming_type);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(cluster_vlan_rif);
            need_respond = TRUE;
            break;
#endif
#if (SYS_CPNT_CRAFT_PORT == TRUE)

        case NETCFG_MGR_IP_IPCCMD_SETCRAFTINTERFACEINETADDRESS:
            netcfg_mgr_ip_msg_p->type.result_ui32 =
                NETCFG_MGR_IP_SetCraftInterfaceInetAddress(&(netcfg_mgr_ip_msg_p->data.craft_addr), FALSE /* om_only */);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(craft_addr);
            need_respond = TRUE;
            break;

        case NETCFG_MGR_IP_IPCCMD_IPV6_IPV6ENABLE_CRAFT:
            netcfg_mgr_ip_msg_p->type.result_ui32 =
                NETCFG_MGR_IP_IPv6Enable_Craft(netcfg_mgr_ip_msg_p->data.u32a1_bla2.u32_a1, netcfg_mgr_ip_msg_p->data.u32a1_bla2.bl_a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
        case NETCFG_MGR_IP_IPCCMD_SET_DHCP_INFORM:
            netcfg_mgr_ip_msg_p->type.result_ui32 =
                NETCFG_MGR_IP_SetDhcpInform(netcfg_mgr_ip_msg_p->data.u32a1_bla2.u32_a1, netcfg_mgr_ip_msg_p->data.u32a1_bla2.bl_a2);
            ipcmsg_p->msg_size = NETCFG_MGR_IP_GET_MSG_SIZE(u32a1_bla2);
            need_respond = TRUE;
            break;
#endif
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            netcfg_mgr_ip_msg_p->type.result_bool = FALSE;
            ipcmsg_p->msg_size = NETCFG_MGR_IP_MSGBUF_TYPE_SIZE;
    }

    return need_respond;
}

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : NETCFG_MGR_IP_SetIpNetToMediaProxyStatus
 * PURPOSE:
 *      Set ARP proxy enable/disable.
 *
 * INPUT:
 *      ifindex -- the interface.
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_IP_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status)
{
    BOOL_T  old_status;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(ifindex > (SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1) || ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER) /*xiongyu 20081106*/
        return NETCFG_TYPE_INVALID_ARG;

    if(NETCFG_OM_IP_GetIpNetToMediaProxyStatus(ifindex,&old_status) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_CAN_NOT_SET;

    if (status == old_status)
        return NETCFG_TYPE_OK;

    if(IP_MGR_SetIpNetToMediaProxyStatus(ifindex, status) != NETCFG_TYPE_OK)
        return NETCFG_TYPE_CAN_NOT_SET;

    if(NETCFG_OM_IP_SetIpNetToMediaProxyStatus(ifindex, status) != NETCFG_TYPE_OK)
    {
        IP_MGR_SetIpNetToMediaProxyStatus(ifindex, old_status);
        return NETCFG_TYPE_CAN_NOT_SET;
    }
    else
        return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetRunningIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get running proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *     SYS_TYPE_GET_RUNNING_CFG_FAIL/
 *     SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_MGR_IP_GetRunningIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T *status)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(ifindex > (SYS_ADPT_MAX_VLAN_ID + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -1) || ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER) /*xiongyu 20081106*/
        return NETCFG_TYPE_INVALID_ARG;

    if (NETCFG_OM_IP_GetIpNetToMediaProxyStatus(ifindex,status) == NETCFG_TYPE_OK)
    {
        if (SYS_DFLT_ARP_PROXY_ARP_STATUS != *status)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

/* FUNCTION NAME : NETCFG_MGR_IP_IpalRifReflection_CallBack
 * PURPOSE:
 *      Handle the callback message for Rif reflection notification from kernel via IPAL.
 *
 * INPUT:
 *      rif_p -- pointer to NETCFG_TYPE_InetRifConfig_T
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. Here we assume that linux kernel has been fixed that
 *      there is no auto-generated ipv6 link-local address for a net device.
 *      Thus we don't need to handle it.
 */
void NETCFG_MGR_IP_IpalRifReflection_CallBack(NETCFG_TYPE_InetRifConfig_T *rif_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    UI32_T rc_om, rc;
    BOOL_T old_rif_reach_max = FALSE;
    BOOL_T new_rif_reach_max = FALSE;
    NETCFG_TYPE_L3_Interface_T l3_if;

    /* BODY
     */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    if(rif_p->ifindex == SYS_ADPT_CRAFT_INTERFACE_IFINDEX)
    {
        /* Currently, IPv4/IPv6 address on craft port are manually configured
         */
        NETCFG_TYPE_CraftInetAddress_T craft_rif;
        memset(&craft_rif, 0, sizeof(craft_rif));
        craft_rif.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
        craft_rif.addr.type = rif_p->addr.type;

        rc_om = NETCFG_OM_IP_GetCraftInterfaceInetAddress(&craft_rif);
#if (SYS_CPNT_NETCFG_IP_SYNC == TRUE)
        if(rif_p->row_status == VAL_netConfigStatus_2_createAndGo)
        {
            if (NETCFG_TYPE_OK != rc_om ||
               (NETCFG_TYPE_OK == rc_om && (rif_p->addr.preflen != craft_rif.addr.preflen ||
                    0 != memcmp(rif_p->addr.addr, craft_rif.addr.addr, SYS_ADPT_IPV6_ADDR_LEN))))
        {
                craft_rif.addr.preflen = rif_p->addr.preflen;
                memcpy(craft_rif.addr.addr, rif_p->addr.addr, SYS_ADPT_IPV6_ADDR_LEN);
                craft_rif.row_status = rif_p->row_status;
                craft_rif.by_user_mode_helper_func = 1; /* not set via AOS CLI */
                if(NETCFG_TYPE_OK != (rc=NETCFG_MGR_IP_SetCraftInterfaceInetAddress(&craft_rif, TRUE)))

                {

                    /* delete rif from kernel
                     */
                    if(IPAL_RESULT_OK != IPAL_IF_DeleteIpAddress(rif_p->ifindex,&(rif_p->addr)))
                        printf("%s:failed to delete rif from kernel.\n",__FUNCTION__);
                }
                else
                {
                    NETCFG_MGR_IP_SignalAllProtocolRifActive(rif_p);
                }
            }
            else
            {
                /* rif active notification
                 */
                NETCFG_MGR_IP_SignalAllProtocolRifActive(rif_p);
            }
        }
        else /* rowstatus is VAL_netConfigStatus_2_destroy */
        {
            if(NETCFG_TYPE_OK == rc_om)
            {

                NETCFG_MGR_IP_SignalAllProtocolRifDown(rif_p);
                craft_rif.addr.preflen = rif_p->addr.preflen;
                memcpy(craft_rif.addr.addr, rif_p->addr.addr, SYS_ADPT_IPV6_ADDR_LEN);
                craft_rif.row_status = rif_p->row_status;
                craft_rif.by_user_mode_helper_func = 1; /* not set via AOS CLI */
                NETCFG_MGR_IP_SetCraftInterfaceInetAddress(&craft_rif, TRUE);
            }
            else
            {
            }
        }/* if(rif_p->row_status == VAL_netConfigStatus_2_createAndGo) */
#else /* #if (SYS_CPNT_NETCFG_IP_SYNC == TRUE) */
        if (NETCFG_TYPE_OK != rc_om)
        {
#if (SYS_CPNT_NETCFG_IP_REDIRECT != TRUE)
            if(IPAL_RESULT_OK != IPAL_IF_DeleteIpAddress(rif_p->ifindex,&(rif_p->addr)))
                DEBUG_PRINTF("%s:failed to delete rif from kernel.\n",__FUNCTION__);
#endif
        }
        else
        {
            if(rif_p->row_status == VAL_netConfigStatus_2_createAndGo)
            {
                if (rif_p->addr.preflen != craft_rif.addr.preflen ||
                    0 != memcmp(rif_p->addr.addr, craft_rif.addr.addr, SYS_ADPT_IPV6_ADDR_LEN))
                {
#if (SYS_CPNT_NETCFG_IP_REDIRECT != TRUE)
                    if(IPAL_RESULT_OK != IPAL_IF_DeleteIpAddress(rif_p->ifindex,&(rif_p->addr)))
                        DEBUG_PRINTF("%s:failed to delete rif from kernel.\n",__FUNCTION__);
                    IPAL_IF_DisableIpv6AcceptRaPrefixInfo(rif_p->ifindex);
#endif
                }
                else
                {
                    /* rif active notification */
                    NETCFG_MGR_IP_SignalAllProtocolRifActive(rif_p);
                }
            }
            else /* rowstatus is VAL_netConfigStatus_2_destroy */
            {
                if (rif_p->addr.preflen == craft_rif.addr.preflen &&
                    0 == memcmp(rif_p->addr.addr, craft_rif.addr.addr, SYS_ADPT_IPV6_ADDR_LEN))
                {
                    /* impossible to come here while SYS_CPNT_NETCFG_IP_REDIRECT == TRUE
                     */
                    DEBUG_PRINTF("%s:unexpected rif delete in kernel.\n",__FUNCTION__);
                    if(IPAL_RESULT_OK != IPAL_IF_AddIpAddress(rif_p->ifindex,&(rif_p->addr)))
                        DEBUG_PRINTF("%s:failed to add rif to kernel.\n",__FUNCTION__);
                }
                else
                {
                    /* rif down notification */
                    NETCFG_MGR_IP_SignalAllProtocolRifDown(rif_p);
                }
            }
        }
#endif
        return;
    }/* if(rif_p->ifindex == SYS_ADPT_CRAFT_INTERFACE_IFINDEX) */
#endif

    memset(&rif_om, 0, sizeof(rif_om));
    memcpy(&rif_om.addr, &rif_p->addr, sizeof(L_INET_AddrIp_T));

    rc_om = NETCFG_OM_IP_GetInetRif(&rif_om);

    DEBUG_PRINTF("IpalRifReflection_CallBack, row_status is %ld.\n", (long)rif_p->row_status);

    old_rif_reach_max = NETCFG_MGR_IP_IsReachMaxNbrOfRif();

    if(rif_p->row_status == VAL_netConfigStatus_2_createAndGo)
    {
        if(NETCFG_TYPE_OK == rc_om)
        {
            /* rif already exists in om */
            /* 1. clear invalid flag */
            if(rif_om.flags & NETCFG_TYPE_RIF_FLAG_IPV6_INVALID)
            {
                NETCFG_OM_IP_UnSetRifFlags(&rif_om, NETCFG_TYPE_RIF_FLAG_IPV6_INVALID);
                DEBUG_PRINTF("%s: clear invalid flag for addr: \n", __FUNCTION__);
                /* 2. rif active notification */
                /* update the correct ipv4_role from ipcfg_om */
                rif_p->ipv4_role = rif_om.ipv4_role;
                NETCFG_MGR_IP_SignalAllProtocolRifActive(rif_p);

                /* 3. if it reaches maximum rif origianlly, unset accept_ra_pinfo */
                if(TRUE == old_rif_reach_max)
                {
                    DEBUG_PRINTF("%s: reach maximum number of rifs, unset accept_ra_pinfo.\n",__FUNCTION__);
                    memset(&l3_if, 0, sizeof(l3_if));
                    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextL3Interface(&l3_if))
                    {
                        if(l3_if.u.physical_intf.accept_ra_pinfo)
                        {
                            DEBUG_PRINTF("Disable acceptrapinfo on vlan %lu\r\n.", (long)l3_if.ifindex);
                            if(IPAL_RESULT_OK == IPAL_IF_DisableIpv6AcceptRaPrefixInfo(l3_if.ifindex))
                                NETCFG_OM_IP_SetInterfaceAcceptRaPrefixInfo(l3_if.ifindex, FALSE);
                        }
                    }
                }
            }
        }
        else if (!(rif_p->flags & NETCFG_TYPE_RIF_FLAG_IFA_F_PERMANENT)) /* skip user config addr */
        {
            /* rif isn't in om, create it */
            if(NETCFG_TYPE_OK != NETCFG_MGR_IP_SetInetRif(rif_p, NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC))
            {
                /* delete rif from kernel */
                if(IPAL_RESULT_OK != IPAL_IF_DeleteIpAddress(rif_p->ifindex,&(rif_p->addr)))
                    DEBUG_PRINTF("%s:failed to delete rif from kernel.\n",__FUNCTION__);
            }
            else
            {
                /* if it reaches the maximum number of rif after add rif to ipcfg,
                 * unset accept_ra_pinfo,
                 * this will let kernel stop to create stateless address from RA
                 */
                new_rif_reach_max = NETCFG_MGR_IP_IsReachMaxNbrOfRif();
                if((TRUE != old_rif_reach_max)&&
                   (TRUE == new_rif_reach_max))
                {
                    DEBUG_PRINTF("%s: reach maximum number of rifs, unset accept_ra_pinfo.\n",__FUNCTION__);
                    memset(&l3_if, 0, sizeof(l3_if));
                    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextL3Interface(&l3_if))
                    {
                        IPAL_IF_DisableIpv6AcceptRaPrefixInfo(l3_if.ifindex);
                    }
                }
            }
        }
    }
    else
    {
        /* rowstatus is VAL_netConfigStatus_2_destroy */
        if(NETCFG_TYPE_OK == rc_om)
        {
            NETCFG_MGR_IP_SignalAllProtocolRifDown(rif_p);
            BOOL_T reach_max_rif = FALSE;


#if (SYS_CPNT_IPV6 == TRUE)

            if((rif_om.ipv6_addr_config_type == NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATELESS)||
               (rif_om.ipv6_addr_config_type == NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATEFULL))
            {
                if(NETCFG_TYPE_OK == NETCFG_MGR_IP_SetInetRif(rif_p, NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC))
                {
                    new_rif_reach_max = NETCFG_MGR_IP_IsReachMaxNbrOfRif();
                    if((TRUE == old_rif_reach_max)&&
                       (TRUE != new_rif_reach_max))
                    {
                        /* if it's not reach the maximum number of rif after delete,
                         * set accept_ra_pinfo
                         */
                        DEBUG_PRINTF("%s: not reach maximum number of rifs, set accept_ra_pinfo.\n",__FUNCTION__);
                        memset(&l3_if, 0, sizeof(l3_if));
                        while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextL3Interface(&l3_if))
                        {
                            if(!l3_if.u.physical_intf.accept_ra_pinfo)
                            {
                                DEBUG_PRINTF("Disable acceptrapinfo on vlan %lu\r\n.", (unsigned long)l3_if.ifindex);
                                if(IPAL_RESULT_OK == IPAL_IF_EnableIpv6AcceptRaPrefixInfo(l3_if.ifindex))
                                    NETCFG_OM_IP_SetInterfaceAcceptRaPrefixInfo(l3_if.ifindex, TRUE);
                            }

                        }
                    }
                }


            }
            else
            {
                /* manually config, just set invalid flag */
                NETCFG_OM_IP_SetRifFlags(&rif_om, NETCFG_TYPE_RIF_FLAG_IPV6_INVALID);
            }
#endif
        }
        else
        {
#if (SYS_CPNT_IPV6 == TRUE)
            /* manually delete ip address */
            /* if it doesn't reaches maximum rif origianlly, set accept_ra_pinfo */
            if(TRUE != old_rif_reach_max)
            {
                DEBUG_PRINTF("%s: not reach maximum number of rifs, set accept_ra_pinfo.\n",__FUNCTION__);
                memset(&l3_if, 0, sizeof(l3_if));
                while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextL3Interface(&l3_if))
                {
                    IPAL_IF_EnableIpv6AcceptRaPrefixInfo(l3_if.ifindex);
                }
            }
#endif
        }
    }
}


/* FUNCTION NAME : NETCFG_MGR_IP_IsReachMaxNbrOfRif
 * PURPOSE:
 *      To check if current system rif number reach the maximum
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE.
 *
 * NOTES:
 *
 */
static BOOL_T NETCFG_MGR_IP_IsReachMaxNbrOfRif(void)
{
    UI32_T rif_num=0;
    NETCFG_OM_IP_GetSystemRifNbr(&rif_num);
    if(SYS_ADPT_MAX_NBR_OF_RIF == rif_num)
        return TRUE;
    else
        return FALSE;
}
#if (SYS_CPNT_IPV6 == TRUE)

/* FUNCTION NAME : NETCFG_MGR_IP_IPv6Enable
 * PURPOSE:
 *      To enable IPv6 processing on an interface that has not been configured
 *      with an explicit IPv6 address.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. It will automatically configures an IPv6 link-local unicast address
 *         on the interface while also enabling the interface for IPv6 processing.
 */
UI32_T NETCFG_MGR_IP_IPv6Enable(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    BOOL_T has_explicit_address = FALSE;
    NETCFG_TYPE_InetRifConfig_T rif;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* BODY
     */
#if (SYS_CPNT_ROUTING != TRUE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
    if( NETCFG_TYPE_OK != NETCFG_MGR_IP_SetSingleManagementVlan(ifindex))
    {
        return NETCFG_TYPE_FAIL;
    }
#endif


// ?? Need to verify enable bevavior here. -> ipv6 enable -> ipv6 address global -> if ipv6 enable config exists?


    /* the ipv6 is enabled explicitly
     */
    NETCFG_OM_IP_SetIPv6EnableStatus(ifindex, TRUE);

    /* if there is an explicit IPv6 address on the interface, then return.
     */
    memset(&rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
    rif.ifindex = ifindex;
    rif.addr.type = L_INET_ADDR_TYPE_IPV6;
    /* check if there is any global address */
    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif))
    {
        if(NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL == rif.ipv6_addr_config_type)
        {
            has_explicit_address = TRUE;
            break;
        }
    } /* while */

    if(!has_explicit_address)
    {
        /* set auto link-local address on the interface
         */

       return NETCFG_MGR_IP_IPv6_SetAutoLinkLocal(ifindex);
    }

   return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_IP_IPv6Disable
 * PURPOSE:
 *      To disable IPv6 processing on an interface that has not been configured
 *      with an explicit IPv6 address.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. It does not disable IPv6 processing on an interface that is configured
 *         with an explicit IPv6 address.
 */
UI32_T NETCFG_MGR_IP_IPv6Disable(UI32_T ifindex)
{

    /* LOCAL VARIABLES DECLARATIONS
     */
    BOOL_T has_explicit_address = FALSE;
    NETCFG_TYPE_InetRifConfig_T rif;

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;
// ?? Need to verify enable bevavior here. -> ipv6 enable -> ipv6 address global -> if ipv6 enable config exists?
// and can we disable the "ipv6 enable"? (not ipv6 global address part )

    /* the ipv6 is disabled explicitly
     */

    NETCFG_OM_IP_SetIPv6EnableStatus(ifindex, FALSE);

    /* if there is an explicit IPv6 address on the interface, then return.
     */
    memset(&rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
    rif.ifindex = ifindex;
    rif.addr.type = L_INET_ADDR_TYPE_IPV6;
    /* check if there is any global address */
    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif))
    {
        if(NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_MANUAL == rif.ipv6_addr_config_type)
        {
            has_explicit_address = TRUE;
            break;
        }
    } /* while */

    if(!has_explicit_address)
    {
        /* unset auto link-local address on the interface
         */
        return NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal(ifindex);
    }
    else
    {
        /* The no ipv6 enable command does not disable IPv6 processing on an interface
         * that is configured with an explicit IPv6 address.
         */
        return NETCFG_TYPE_HAS_EXPLICIT_IPV6_ADDR;
    }
}

/* FUNCTION NAME : NETCFG_MGR_IP_IPv6_SetAutoLinkLocal
 * PURPOSE:
 *      To add a auto link-local address for the interface if there was none on it.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      status      -- status
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *
 * NOTES:
 *      1. If there is already a link-local address, return NETCFG_TYPE_OK.
 */
static UI32_T NETCFG_MGR_IP_IPv6_SetAutoLinkLocal(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_TYPE_L3_Interface_T intf;
    NETCFG_TYPE_InetRifConfig_T rif;
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    char tmp_buf[64] = {0};
    /* BODY
     */
    memset(&intf, 0, sizeof(intf));
    intf.ifindex = ifindex;

    if (NETCFG_OM_IP_GetL3Interface(&intf) != NETCFG_TYPE_OK)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    memset(&rif, 0, sizeof(rif));

    rif.ifindex = ifindex;
    rif.addr.type = L_INET_ADDR_TYPE_IPV6Z;

    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetLinkLocalRifFromInterface(&rif))
    {
        /* there is already a link-local address */
        return NETCFG_TYPE_OK;
    }

    memset(&rif_om, 0, sizeof(rif_om));

    rif_om.ifindex = ifindex;
    rif_om.addr.type = L_INET_ADDR_TYPE_IPV6Z;
    rif_om.addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;

    rif_om.addr.preflen = 64;
    rif_om.addr.addr[0] = 0xFE;
    rif_om.addr.addr[1] = 0x80;
    /* zoneid */
    VLAN_OM_ConvertFromIfindex(ifindex, &rif_om.addr.zoneid);


    if(NETCFG_TYPE_OK == NETCFG_MGR_IP_IPv6_IPv6AddressEUI64(rif_om.addr.addr, 64, intf.u.physical_intf.hw_addr))
    {
        L_INET_InaddrToString((L_INET_Addr_T *) &rif_om.addr, tmp_buf, sizeof(tmp_buf));

        rif_om.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;
        rif_om.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER;
        rif_om.row_status = VAL_netConfigStatus_2_createAndGo;
        return NETCFG_MGR_IP_InternalSetInetRif(&rif_om); // or NETCFG_MGR_IP_SetInetRif ?
    }
    return NETCFG_TYPE_OK;
}

static UI32_T NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_TYPE_InetRifConfig_T rif;
//    NETCFG_OM_L3_Interface_T intf;
    NETCFG_OM_IP_InetRifConfig_T rif_om;

    /* BODY
     */

    memset(&rif, 0, sizeof(rif));

    rif.ifindex = ifindex;
    rif.addr.type = L_INET_ADDR_TYPE_IPV6Z;

    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetLinkLocalRifFromInterface(&rif)
        && (NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER == rif.ipv6_addr_config_type))
    {
        memset(&rif_om, 0, sizeof(rif_om));
        rif_om.ifindex = rif.ifindex;
        rif_om.ipv6_addr_config_type = rif.ipv6_addr_config_type;
        rif_om.ipv6_addr_type = rif.ipv6_addr_type;
        memcpy(&rif_om.addr, &rif.addr, sizeof(rif_om.addr));
        rif_om.row_status = VAL_netConfigStatus_2_destroy;
        return NETCFG_MGR_IP_InternalSetInetRif(&rif_om); // or NETCFG_MGR_IP_SetInetRif ?
    }

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_IP_IPv6_GetIntIdFromMAC
 * PURPOSE:
 *      Generate interface id based on mac address.
 *
 * INPUT:
 *      mac_addr    -- mac address for eui64 address
 *
 * OUTPUT:
 *      int_id      -- interface id is based on EUI64
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. mac_addr is 48 bits, int_id is 64 bits.
 *      2. An intermediate eui64 address is based on mac_addr with
 *         0xFFFE is inserted.
 *      3. the int_id is based on eui64 address with the complement th U/L bit.
 */
static UI32_T NETCFG_MGR_IP_IPv6_GetIntIdFromMAC(UI8_T* int_id, UI8_T* mac_addr)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
#define INTERFACE_ID_LEN        8

   UI8_T local_id[INTERFACE_ID_LEN];


    /* BODY
     */
    memset(local_id, 0, INTERFACE_ID_LEN);
    memcpy(local_id, mac_addr, 3);
    memcpy(local_id + 5, mac_addr + 3, 3);
    local_id[3] = 0xff;
    local_id[4] = 0xfe;

    /* complement the U/L bit */
    local_id[0] ^= 0x02;

    memcpy(int_id, local_id, INTERFACE_ID_LEN);
    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : NETCFG_MGR_IP_IPv6_IPv6AddressEUI64
 * PURPOSE:
 *      Generate IPv6 address based on the prefix and mac address.
 *
 * INPUT:
 *      addr        -- IPv6 address with prefix
 *      preflen     -- prefix length
 *      mac_addr    -- mac address for eui64 address
 *
 * OUTPUT:
 *      addr        -- IPv6 address whose interface id is based on EUI64.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1.
 */
static UI32_T NETCFG_MGR_IP_IPv6_IPv6AddressEUI64(UI8_T* addr, UI16_T preflen, UI8_T* mac_addr)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    static UI8_T local_addr[SYS_ADPT_IPV6_ADDR_LEN]; /* 128 bits */
    static UI8_T int_id[8]; /* 64 bits */
    UI32_T num_byte, num_bit;

    /* BODY
     */
    memset(local_addr, 0, SYS_ADPT_IPV6_ADDR_LEN);

    /* generate eui64 interface id */
    NETCFG_MGR_IP_IPv6_GetIntIdFromMAC(int_id, mac_addr);


    /* copy last 64 bits for interface id */
    memcpy(local_addr + 8, int_id, 8);

    /* copy prefix */
    num_byte = preflen / 8;
    num_bit = preflen % 8;

    memcpy(local_addr, addr, num_byte); /* copy the bytes part */
    if (num_bit != 0) /* copy the bits part */
    {
        local_addr[num_byte] &= 0xFF >> num_bit;
        local_addr[num_byte] |= addr[num_byte] & (0xFF<<(8-num_bit));
    }

    memcpy(addr, local_addr, SYS_ADPT_IPV6_ADDR_LEN);

    return NETCFG_TYPE_OK;
}

static BOOL_T NETCFG_MGR_IsPrivateNetwork(UI8_T* ipv4_addr)
{
    UI8_T subnet[4];
    memset(subnet,0,sizeof(subnet));
    subnet[0]=10; //10.0.0.0/8

    if(IP_LIB_IsIpBelongToSubnet(subnet,8,ipv4_addr))
        return TRUE;
    subnet[0]=172; //172.16.0.0/12
    subnet[1]=26;
    if(IP_LIB_IsIpBelongToSubnet(subnet,12,ipv4_addr))
        return TRUE;
    subnet[0]=192; //192.168.0.0/16
    subnet[1]=168;
    if(IP_LIB_IsIpBelongToSubnet(subnet,16,ipv4_addr))
        return TRUE;
    return FALSE;
}

#if 0
/* FUNCTION NAME : NETCFG_MGR_IP_IPv6_IPv6AddressModifiedEUI64
 * PURPOSE:
 *      Generate IPv6 address based on the prefix and IP address.
 *
 * INPUT:
 *      preflen     -- prefix length
 *      ipv4_addr    -- ip  address for eui64 address
 *
 * OUTPUT:
 *      addr        -- IPv6 address whose interface id is based on EUI64.
 *                         64bit prefixt:200:5EFE:w.x.y.z
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1.
 */
 static UI32_T NETCFG_MGR_IP_IPv6_IPv6AddressIsatapEUI64(UI8_T* addr, UI16_T preflen, UI8_T* ipv4_addr)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    static UI8_T local_addr[SYS_ADPT_IPV6_ADDR_LEN]; /* 128 bits */
    static UI8_T int_id[8]; /* 64 bits */
    UI32_T num_byte, num_bit;

    /* BODY
     */
    memset(local_addr, 0, SYS_ADPT_IPV6_ADDR_LEN);

    /* generate eui64 interface id */
    if(!NETCFG_MGR_IsPrivateNetwork(ipv4_addr))
    {//0200:5EFE
        int_id[0]=0x02;
    }
    int_id[2]=0x5E;
    int_id[3]=0xFE;
    memcpy(&int_id[4],ipv4_addr,SYS_ADPT_IPV4_ADDR_LEN);

    /* copy last 64 bits for interface id */
    memcpy(local_addr + 8, int_id, 8);

    /* copy prefix */
    num_byte = preflen / 8;
    num_bit = preflen % 8;

    memcpy(local_addr, addr, num_byte); /* copy the bytes part */
    if (num_bit != 0) /* copy the bits part */
    {
        local_addr[num_byte] &= 0xFF >> num_bit;
        local_addr[num_byte] |= addr[num_byte] & (0xFF<<(8-num_bit));
    }

    memcpy(addr, local_addr, SYS_ADPT_IPV6_ADDR_LEN);

    return NETCFG_TYPE_OK;
}
static UI32_T NETCFG_MGR_IP_IPv6_IPv6AddressModifiedEUI64(UI8_T* addr, UI16_T preflen, UI8_T*   ipv4_addr)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    static UI8_T local_addr[SYS_ADPT_IPV6_ADDR_LEN]; /* 128 bits */
    static UI8_T int_id[8]; /* 64 bits */
    UI32_T num_byte, num_bit;

    /* BODY
     */
    memset(local_addr, 0, SYS_ADPT_IPV6_ADDR_LEN);

    /* generate eui64 interface id */
    memcpy(&int_id[4],ipv4_addr,SYS_ADPT_IPV4_ADDR_LEN);

    /* copy last 64 bits for interface id */
    memcpy(local_addr + 8, int_id, 8);

    /* copy prefix */
    num_byte = preflen / 8;
    num_bit = preflen % 8;

    memcpy(local_addr, addr, num_byte); /* copy the bytes part */
    if (num_bit != 0) /* copy the bits part */
    {
        local_addr[num_byte] &= 0xFF >> num_bit;
        local_addr[num_byte] |= addr[num_byte] & (0xFF<<(8-num_bit));
    }

    memcpy(addr, local_addr, SYS_ADPT_IPV6_ADDR_LEN);

    return NETCFG_TYPE_OK;
}
#endif
/* FUNCTION NAME : NETCFG_MGR_IP_IPv6AddrAutoconfigEnable
 * PURPOSE:
 *      To enable automatic configuration of IPv6 addresses using stateless
 *      autoconfiguration on an interface and enable IPv6 processing on the interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. The autoconfiguration process specified here applies only to hosts, not routers.
 *      2. The configuration is passed into TCP/IP protocol stack, and after the process,
 *         the stack will call NETCFG_MGR_IP_SetInetRif to add into OM.
 */
UI32_T NETCFG_MGR_IP_IPv6AddrAutoconfigEnable(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
     UI32_T rc;
    UI32_T ipv6_forwarding_status;
    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* ipv6 autoconfig config is only for L2 device */
    if((NETCFG_MGR_ROUTE_GetIpForwardingStatus(0, L_INET_ADDR_TYPE_IPV6, &ipv6_forwarding_status) == NETCFG_TYPE_OK)
        && (ipv6_forwarding_status == 1))
    {
            return NETCFG_TYPE_FAIL;
    }

#if (SYS_CPNT_ROUTING != TRUE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
    if( NETCFG_TYPE_OK != NETCFG_MGR_IP_SetSingleManagementVlan(ifindex))
    {
        return NETCFG_TYPE_FAIL;
    }
#endif

    NETCFG_MGR_IP_IPv6_SetAutoLinkLocal(ifindex);

    /* the kernel support ipv6 address autoconfig, we can
     * set/unset to kernel.
     */
    if(NETCFG_TYPE_OK != IP_MGR_IPv6AddrAutoconfigEnable(ifindex))
        return NETCFG_TYPE_FAIL;

    rc = NETCFG_OM_IP_SetIPv6AddrAutoconfigEnableStatus(ifindex, TRUE);

    /* let cmgr to handle notifying dhcpv6 */
#if 0
    /* call back for DHCPv6
     */
    if(rc == NETCFG_TYPE_OK)
        SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfig(SYS_MODULE_IPCFG, ifindex, TRUE);
#endif
    return rc;
}


/* FUNCTION NAME : NETCFG_MGR_IP_IPv6AddrAutoconfigDisable
 * PURPOSE:
 *      To disable automatic configuration of IPv6 addresses.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. None.
 */
UI32_T NETCFG_MGR_IP_IPv6AddrAutoconfigDisable(UI32_T ifindex)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    NETCFG_OM_IP_InetRifConfig_T rif_om;
    BOOL_T has_ipv6_global = FALSE;
    BOOL_T ipv6_explicit_enable = FALSE;
    UI32_T ipv6_forwarding_status;

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    /* ipv6 autoconfig config is only for L2 device */
    if((NETCFG_MGR_ROUTE_GetIpForwardingStatus(0, L_INET_ADDR_TYPE_IPV6, &ipv6_forwarding_status) == NETCFG_TYPE_OK)
        && (ipv6_forwarding_status == 1))
    {
            return NETCFG_TYPE_FAIL;
    }

    /* the kernel support ipv6 address autoconfig, we can
     * set/unset to kernel.
     */
    if(NETCFG_TYPE_OK != IP_MGR_IPv6AddrAutoconfigDisable(ifindex))
        return NETCFG_TYPE_FAIL;

    /* delete autoconfig address */

    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.ifindex = ifindex;
    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif_config))
    {
        if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV6)
            && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV6Z))
        {
            continue;
        }
        /* skip link-local */
        if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif_config.addr.addr))
            continue;

        /* clear ipv6 stateless address */
        if(NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_STATELESS == rif_config.ipv6_addr_config_type)
        {
            memset(&rif_om, 0, sizeof(rif_om));
            rif_om.ifindex = rif_config.ifindex;
            memcpy(&rif_om.addr, &rif_config.addr, sizeof(L_INET_AddrIp_T));
            rif_om.row_status = VAL_netConfigStatus_2_destroy;
            NETCFG_MGR_IP_InternalSetInetRif(&rif_om);
        }
    }


    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
    rif_config.ifindex = ifindex;
    rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;
    /* check if there is any global address */
    while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif_config))
    {
        /* check ipv6 global address */
        if(!L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif_config.addr.addr))
        {
            has_ipv6_global = TRUE;
            break;
        }
    } /* while */

    if((!has_ipv6_global)
        && (NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6EnableStatus(ifindex, &ipv6_explicit_enable)
            && (!ipv6_explicit_enable)))
    {
        NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal(ifindex);
    }

    rc = NETCFG_OM_IP_SetIPv6AddrAutoconfigEnableStatus(ifindex, FALSE);

    /* let cmgr to handle notifying dhcpv6 */
#if 0
    /* call back for DHCPv6
     */
    if (rc == NETCFG_TYPE_OK)
        SYS_CALLBACK_MGR_NETCFG_IPv6AddrAutoConfig(SYS_MODULE_IPCFG, ifindex, FALSE);
#endif
    return rc;
}

/* FUNCTION NAME : NETCFG_MGR_IP_SetIPv6InterfaceMTU
 * PURPOSE:
 *      Set IPv6 interface MTU.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *      mtu     -- MTU
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_IPV6_MTU_EXCEED_IF_MTU
 *
 * NOTES:
 *      None.
 */
UI32_T  NETCFG_MGR_IP_SetIPv6InterfaceMTU(UI32_T ifindex, UI32_T mtu)
{

    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc, if_mtu, old_mtu;
#if (SYS_CPNT_SWDRVL3 == TRUE)
    UI32_T vid;
#endif

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (IPAL_RESULT_OK != IPAL_IF_GetIfMtu(ifindex, &if_mtu))
        return NETCFG_TYPE_FAIL;

    if (mtu > if_mtu)
        return NETCFG_TYPE_IPV6_MTU_EXCEED_IF_MTU;

    rc = NETCFG_OM_IP_SetIPv6InterfaceMTU(ifindex, mtu);
    if(NETCFG_TYPE_OK != rc)
        return rc;

    if (IPAL_RESULT_OK != IPAL_IF_GetIpv6Mtu(ifindex, &old_mtu))
        return NETCFG_TYPE_FAIL;

    if (old_mtu == mtu)
        return NETCFG_TYPE_OK;

    rc = ROUTE_MGR_Rt6MtuChange(ifindex, mtu);
    if (rc != NETCFG_TYPE_OK)
        return rc;

    if  (IPAL_RESULT_OK != IPAL_IF_SetIpv6Mtu(ifindex, mtu))
        return NETCFG_TYPE_FAIL;

#if (SYS_CPNT_SWDRVL3 == TRUE)
/* The following code is only a work around patch to pass TAHI IPv6 ready logo test
 * The TAHI test require the setting of IPv6 MTU on an interface take effect on
 * both unicast and multicast routing traffic (egress interface MTU must not smaller than
 * the packet size). Currently, Broadcom's Firebolt chipset doesn't support MTU setting
 * on L3 interface, so we adopt the following work around (when IPv6 MTU is set on L3if):
 * 1. For unicast: Set EGR_MTU_SIZE[port_num].L3_MTU_SIZE for every ports in the L3if.
 *                 This will be work fine if no member overlap for each VLAN.
 * 2. For multicast: Set IPMC_L3_MTU_0 which is the default IPMC MTU for all multicast entries.
 *                   This can be viewed as a system-wise setting for IPMC L3 MTU.
 * The implementation is located in DEV_SWDRVL3_SetL3InterfaceMtu()
 * Note: Here we didn't add SWDRVL3_SetL3InterfaceMtu() because it is only unformal work around
 *       code to pass TAHI test.
 */
    VLAN_OM_ConvertFromIfindex(ifindex, &vid);
    if (!DEV_SWDRVL3_PMGR_SetL3InterfaceMtu(vid, mtu))
    {
        return NETCFG_TYPE_FAIL;
    }
#endif

    return rc;
}

/* FUNCTION NAME : NETCFG_MGR_IP_UnsetIPv6InterfaceMTU
 * PURPOSE:
 *      Unset IPv6 interface MTU.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_IPV6_MTU_EXCEED_IF_MTU
 *
 * NOTES:
 *      None.
 */
UI32_T  NETCFG_MGR_IP_UnsetIPv6InterfaceMTU(UI32_T ifindex)
{

    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T rc, rc_om, if_mtu, old_mtu;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    rc = IPAL_IF_GetIfMtu(ifindex, &if_mtu);
    if(IPAL_RESULT_OK != rc)
        return NETCFG_TYPE_FAIL;

    if (SYS_DFLT_IPV6_INTERFACE_MTU > if_mtu)
        return NETCFG_TYPE_IPV6_MTU_EXCEED_IF_MTU;

    rc_om = NETCFG_OM_IP_UnsetIPv6InterfaceMTU(ifindex);

    if(NETCFG_TYPE_OK != rc_om)
        return rc_om;

    rc = IPAL_IF_GetIpv6Mtu(ifindex, &old_mtu);
    if(IPAL_RESULT_OK != rc)
        return NETCFG_TYPE_FAIL;

    if (old_mtu == SYS_DFLT_IPV6_INTERFACE_MTU)
        return NETCFG_TYPE_OK;
    rc = IPAL_IF_SetIpv6Mtu(ifindex, SYS_DFLT_IPV6_INTERFACE_MTU);
    if(IPAL_RESULT_OK != rc)
        return NETCFG_TYPE_FAIL;

    rc = ROUTE_MGR_Rt6MtuChange(ifindex, SYS_DFLT_IPV6_INTERFACE_MTU);
    return rc;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetIPv6InterfaceMTU
 * PURPOSE:
 *      Get IPv6 interface MTU from kernel.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      mtu_p   -- MTU
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
     UI32_T rc;
    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    rc = IPAL_IF_GetIpv6Mtu(ifindex, mtu_p);

    if(IPAL_RESULT_OK != rc)
        return NETCFG_TYPE_FAIL;

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetNextIfJoinIpv6McastAddr
 * PURPOSE:
 *      Get next joined multicast group for the interface.
 *
 * INPUT:
 *      ifindex     -- interface
 *
 * OUTPUT:
 *      mcaddr_p    -- pointer to multicast address
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
     UI32_T rc;
    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    rc = IP_MGR_GetNextIfJoinIpv6McastAddr(ifindex, mcaddr_p);

    return rc;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetNextPMTUEntry
 * PURPOSE:
 *      Get next path mtu entry.
 *
 * INPUT:
 *      entry_p     -- pointer to entry
 *
 * OUTPUT:
 *      entry_p     -- pointer to entry
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_GetNextPMTUEntry(NETCFG_TYPE_PMTU_Entry_T *entry_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
     UI32_T rc;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    rc = IP_MGR_GetNextPMTUEntry(entry_p);

    return rc;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetIfIpv6AddrInfo
 * PURPOSE:
 *      Get ipv6 address info
 *
 * INPUT:
 *      ifindex     -- interface
 *
 * OUTPUT:
 *      addr_info_p -- pointer to address info
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_GetIfIpv6AddrInfo(NETCFG_TYPE_IpAddressInfoEntry_T *addr_info_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
     UI32_T rc;
    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    rc = IP_MGR_GetIfIpv6AddrInfo(addr_info_p);

    return rc;
}

/* FUNCTION NAME : NETCFG_MGR_IP_DhcpReleaseComplete
 * PURPOSE:
 *      If DHCP or DHCPv6 has release the last address, it will use this
 *      to notify NETCFG.
 *
 * INPUT:
 *      ifindex     -- interface
 *      protocols   --
 *                  DHCP    0x01
 *                  DHCPv6  0x02
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1.For IPv6, NETCFG may delet the link-local address after this
 *        notification.
 */
UI32_T NETCFG_MGR_IP_DhcpReleaseComplete(UI32_T ifindex, UI32_T protocols)
{
#define DHCP    0x01
#define DHCPv6  0x02

    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_TYPE_InetRifConfig_T rif;
    BOOL_T has_ipv6_global = FALSE;
    BOOL_T ipv6_explicit_enable = FALSE;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;


    if(TRUE == (protocols & DHCPv6))
    {
        memset(&rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
        rif.ifindex = ifindex;
        rif.addr.type = L_INET_ADDR_TYPE_IPV6;
        /* check if there is any global address */
        while(NETCFG_TYPE_OK == NETCFG_OM_IP_GetNextInetRifOfInterface(&rif))
        {
            /* check ipv6 global address, is it right ? */
            if(!L_INET_ADDR_IS_IPV6_LINK_LOCAL(rif.addr.addr))
            {
                has_ipv6_global = TRUE;
                break;
            }
        } /* while */

        if((!has_ipv6_global)
            && (NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6EnableStatus(ifindex, &ipv6_explicit_enable)
                && (!ipv6_explicit_enable)))
        {
            NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal(ifindex);
        }
    }

    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_MGR_IP_GetRunningIPv6EnableStatus
 * PURPOSE:
 *      Get "ipv6 enable" configuration of the interface.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1.This function is for CLI running config.
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_MGR_IP_GetRunningIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(ifindex > SYS_ADPT_MAX_VLAN_ID || ifindex <= 0)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (NETCFG_OM_IP_GetIPv6EnableStatus(ifindex, status_p) == NETCFG_TYPE_OK)
    {
        if (SYS_DFLT_IPV6_ENABLE!= *status_p)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      To get the IPv6 address autoconfig enable status of the interface.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      status      -- status
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1.This function is for CLI running config.
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_MGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(ifindex > SYS_ADPT_MAX_VLAN_ID || ifindex <= 0)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus(ifindex, status_p) == NETCFG_TYPE_OK)
    {
        if (SYS_DFLT_IPV6_ADDR_AUTOCONFIG != *status_p)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetRunningIPv6InterfaceMTU
 * PURPOSE:
 *      Get IPv6 interface MTU.
 *
 * INPUT:
 *      ifindex -- the interface be assoicated.
 *
 * OUTPUT:
 *      mtu_p   -- the pointer to the mtu value.
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *
 * NOTES:
 *      1.This function is for CLI running config.
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_MGR_IP_GetRunningIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_TYPE_L3_Interface_T intf;
    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    memset(&intf, 0, sizeof(intf));
    intf.ifindex = ifindex;

    if (NETCFG_OM_IP_GetL3Interface(&intf) == NETCFG_TYPE_OK)
    {
        *mtu_p = intf.u.physical_intf.mtu6;
        if (SYS_DFLT_IPV6_INTERFACE_MTU != *mtu_p)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        else
            return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetIPv6Statistics
 * PURPOSE:
 *      Get IPv6 statistics.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      ip6stat_p -- IPv6 statistics.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_GetIPv6Statistics(IPAL_Ipv6Statistics_T *ip6stat_p)
{
    UI32_T                  ret = NETCFG_TYPE_FAIL;
    IPAL_Ipv6Statistics_T   ip6_stat_base, ip6_stat_curr;
    UI32_T                  *val_curr_p, *val_base_p, *val_res_p,
                            field_cnt, loop_idx;
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (  (NULL != ip6stat_p)
        &&(IPAL_RESULT_OK == IPAL_IF_GetAllIpv6Statistic(&ip6_stat_curr))
        &&(NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6StatBaseCntr(&ip6_stat_base))
       )
    {
        field_cnt = (sizeof(IPAL_Ipv6Statistics_T) / sizeof(UI32_T));
        val_curr_p  = (UI32_T *) &ip6_stat_curr;
        val_base_p  = (UI32_T *) &ip6_stat_base;
        val_res_p   = (UI32_T *) ip6stat_p;

        for (loop_idx = 0; loop_idx < field_cnt; loop_idx++)
        {
            NETCFG_MGR_IP_UpdateDiffCounter(*val_base_p, *val_curr_p, *val_res_p);
            val_curr_p ++;
            val_base_p ++;
            val_res_p  ++;
        }

        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetICMPv6Statistics
 * PURPOSE:
 *      Get ICMPv6 statistics.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp6stat_p -- ICMPv6 statistics.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_GetICMPv6Statistics(IPAL_Icmpv6Statistics_T *icmp6stat_p)
{
    UI32_T                      ret = NETCFG_TYPE_FAIL;
    IPAL_Icmpv6Statistics_T     icmp6_stat_base, icmp6_stat_curr;
    UI32_T                      *val_curr_p, *val_base_p, *val_res_p,
                                field_cnt, loop_idx;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (  (NULL != icmp6stat_p)
        &&(IPAL_RESULT_OK == IPAL_IF_GetAllIcmp6Statistic(&icmp6_stat_curr))
        &&(NETCFG_TYPE_OK == NETCFG_OM_IP_GetICMPv6StatBaseCntr(&icmp6_stat_base))
       )
    {
        field_cnt = (sizeof(IPAL_Icmpv6Statistics_T) / sizeof(UI32_T));
        val_curr_p  = (UI32_T *) &icmp6_stat_curr;
        val_base_p  = (UI32_T *) &icmp6_stat_base;
        val_res_p   = (UI32_T *) icmp6stat_p;

        for (loop_idx = 0; loop_idx < field_cnt; loop_idx++)
        {
            NETCFG_MGR_IP_UpdateDiffCounter(*val_base_p, *val_curr_p, *val_res_p);
            val_curr_p ++;
            val_base_p ++;
            val_res_p  ++;
        }

        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

/* FUNCTION NAME : NETCFG_MGR_IP_GetUDPv6Statistics
 * PURPOSE:
 *      Get UDPv6 statistics.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      udp6stat_p -- UDPv6 statistics.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_GetUDPv6Statistics(IPAL_Udpv6Statistics_T *udp6stat_p)
{
    UI32_T                  ret = NETCFG_TYPE_FAIL;
    IPAL_Udpv6Statistics_T  udp6_stat_base, udp6_stat_curr;
    UI32_T                  *val_curr_p, *val_base_p, *val_res_p,
                            field_cnt, loop_idx;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if (  (NULL != udp6stat_p)
        &&(IPAL_RESULT_OK == IPAL_IF_GetAllUdp6Statistic(&udp6_stat_curr))
        &&(NETCFG_TYPE_OK == NETCFG_OM_IP_GetUDPv6StatBaseCntr(&udp6_stat_base))
       )
    {
        field_cnt = (sizeof(IPAL_Udpv6Statistics_T) / sizeof(UI32_T));
        val_curr_p  = (UI32_T *) &udp6_stat_curr;
        val_base_p  = (UI32_T *) &udp6_stat_base;
        val_res_p   = (UI32_T *) udp6stat_p;

        for (loop_idx = 0; loop_idx < field_cnt; loop_idx++)
        {
            NETCFG_MGR_IP_UpdateDiffCounter(*val_base_p, *val_curr_p, *val_res_p);
            val_curr_p ++;
            val_base_p ++;
            val_res_p  ++;
        }

        ret = NETCFG_TYPE_OK;
    }

    return ret;
}

/* FUNCTION NAME : NETCFG_MGR_IP_ClearIPv6StatisticsByType
 * PURPOSE:
 *      Clear IPv6 statistics by specified type.
 *
 * INPUT:
 *      clear_type -- which type to clear.
 *                    refer to NETCFG_TYPE_IPV6_STAT_TYPE_E
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_MGR_IP_ClearIPv6StatisticsByType(UI32_T clear_type)
{
    UI32_T                  ret = NETCFG_TYPE_FAIL;
    IPAL_Ipv6Statistics_T   ip6_stat_curr;
    IPAL_Icmpv6Statistics_T icmp6_stat_curr;
    IPAL_Udpv6Statistics_T  udp6_stat_curr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    switch (clear_type)
    {
    case NETCFG_TYPE_IPV6_STAT_TYPE_ALL:
    case NETCFG_TYPE_IPV6_STAT_TYPE_IP6:
        if (IPAL_RESULT_OK == IPAL_IF_GetAllIpv6Statistic(&ip6_stat_curr))
        {
            ret = NETCFG_OM_IP_SetIPv6StatBaseCntr(&ip6_stat_curr);
        }
        if (NETCFG_TYPE_IPV6_STAT_TYPE_ALL != clear_type)
            break;

    case NETCFG_TYPE_IPV6_STAT_TYPE_ICMP6:
        if (IPAL_RESULT_OK == IPAL_IF_GetAllIcmp6Statistic(&icmp6_stat_curr))
        {
            ret = NETCFG_OM_IP_SetICMPv6StatBaseCntr(&icmp6_stat_curr);
        }
        if (NETCFG_TYPE_IPV6_STAT_TYPE_ALL != clear_type)
            break;

    case NETCFG_TYPE_IPV6_STAT_TYPE_UDP6:
        if (IPAL_RESULT_OK == IPAL_IF_GetAllUdp6Statistic(&udp6_stat_curr))
        {
            ret = NETCFG_OM_IP_SetUDPv6StatBaseCntr(&udp6_stat_curr);
        }
        break;

    default:
        break;
    }

    return ret;
}

#endif /* SYS_CPNT_IPV6 */


#if (SYS_CPNT_CLUSTER == TRUE)
/* FUNCTION NAME: NETCFG_MGR_IP_ClusterVlanSetRifRowStatus
 * PURPOSE  : Set and delete a IP address on specific Cluster VLAN
 * INPUT    : ipAddress -- for cluster VLAN internal IP
 *            rowStatus -- only allow VAL_netConfigStatus_2_createAndGo and VAL_netConfigStatus_2_destroy
 *            incoming_type -- from which UI (CLI/WEB/dynamic)
 * OUTPUT   : none
 * RETURN   : successful (NETCFG_TYPE_OK), failed (NETCFG_TYPE_FAIL)
 * NOTES    :
 *      1. Because we can set only 1 IP, if there is existing one, delete it first. Then we process rowStatus request by each
 *         case, and only validate IP in createAndGo case. If no IP existed, destroy case will return OK.
 *      1. This function is derived from NETCFG_SetRifRowStatus() and for cluster vlan only.
 *      2. Allow only 1 IP address on this VLAN as far.
 *      3. rowStatus only provides VAL_netConfigStatus_2_createAndGo and VAL_netConfigStatus_2_destroy
 *      4. Call NETCFG_MGR_SetIpAddressMode() here.
 *      hawk, 2006.3.1
 */
UI32_T NETCFG_MGR_IP_ClusterVlanSetRifRowStatus(UI8_T *ipAddress, UI8_T *ipMask, UI32_T rowStatus, UI32_T incoming_type)
{

    UI32_T  res = NETCFG_TYPE_OK;
    UI32_T  rif_num;
    NETCFG_TYPE_InetRifConfig_T   rif_conf;
    NETCFG_OM_IP_InetRifConfig_T  overlap_rif,internal_rif_conf;
    NETCFG_OM_L3_Interface_T      intf;
    UI32_T  b_addr;
    UI32_T  vid_ifindex = 0;
    UI32_T  overed_rif;
    UI32_T  ip = 0;
    UI32_T  mask = 0;
    UI32_T  ret = 0;
    L_INET_AddrIp_T  addr;



    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if ((rowStatus != VAL_netConfigStatus_2_createAndGo)
       &&(rowStatus != VAL_netConfigStatus_2_destroy))
        return NETCFG_TYPE_FAIL;

    ipAddress=L_STDLIB_Ntoh32(ipAddress);
    ipMask=L_STDLIB_Ntoh32(ipMask);



    VLAN_OM_ConvertToIfindex(SYS_DFLT_CLUSTER_DEFAULT_VLAN, &vid_ifindex);
    /* Normally, we call SetIpAddressMode() at CLI layer, not here, but this is a special function for setting IP
     * to cluster VLAN. We do all the neccessay thing for the convenience of usage.
     */
    NETCFG_MGR_IP_SetIpAddressMode(vid_ifindex,NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE);
    /* processing the request.
     * For current spec, there is only 1 RIF (the primary),remove the existing primary RIF. Hence, no matter we perform
     * CreateAndGo or Destroy, we need to delete the existed one.
     */
    memset(&rif_conf, 0 , sizeof(NETCFG_TYPE_InetRifConfig_T));
    rif_conf.ifindex = vid_ifindex;
    rif_conf.addr.type = L_INET_ADDR_TYPE_IPV4;

    memset(&internal_rif_conf, 0 ,sizeof(NETCFG_OM_IP_InetRifConfig_T));

    if (NETCFG_OM_IP_GetNextInetRifOfInterface(&rif_conf)==NETCFG_TYPE_OK)
    {

        internal_rif_conf.ifindex = rif_conf.ifindex;
        internal_rif_conf.ipv4_role = rif_conf.ipv4_role;
        internal_rif_conf.row_status = VAL_netConfigStatus_2_destroy;
        internal_rif_conf.flags = rif_conf.flags;
        memcpy(&(internal_rif_conf.addr), &(rif_conf.addr), sizeof(L_INET_AddrIp_T));
        res = NETCFG_MGR_IP_InternalSetInetRif(&internal_rif_conf);

    }

    memset(&addr, 0, sizeof(L_INET_AddrIp_T));
    addr.type = L_INET_ADDR_TYPE_IPV4;
    addr.preflen =  IP_LIB_MaskToCidr(ipMask);
    addr.addrlen =  SYS_ADPT_IPV4_ADDR_LEN;
    memcpy(addr.addr,ipAddress,SYS_ADPT_IPV4_ADDR_LEN);



    switch (rowStatus)
    {/* When no IP existed and perform deleting, it will return OK*/
        case VAL_netConfigStatus_2_destroy:
        /*We have deleted the RIF, exit*/
        /* Delete L3 interface */
        ret = NETCFG_MGR_IP_DeleteL3If(SYS_DFLT_CLUSTER_DEFAULT_VLAN);
        if((ret != NETCFG_TYPE_OK)&&(ret != NETCFG_TYPE_ENTRY_NOT_EXIST))
        {
            return NETCFG_TYPE_FAIL;
        }
        else
            return NETCFG_TYPE_OK;

        case VAL_netConfigStatus_2_createAndGo:

            /* if delete existed rif failed, return fail*/
            if ((NETCFG_TYPE_OK != res)&&(NETCFG_TYPE_ENTRY_NOT_EXIST != res))
                return NETCFG_TYPE_FAIL;

           /*
            * When we create IP interface ,it must fulfill the following conditions:
            * 1) must be either class A, class B, or class C
            * 2) can't be network b'cast ip or network id
            * 3) can't be loop back ip
            * 4) can't be b'cast ip
            * 5) can't be m'cast ip
            */
            memcpy(&ip, ipAddress, SYS_ADPT_IPV4_ADDR_LEN);
            memcpy(&mask, ipMask, SYS_ADPT_IPV4_ADDR_LEN);
           /* class A, class B or class C */
            if ((IP_LIB_IsIpInClassA(ipAddress) == FALSE) && (IP_LIB_IsIpInClassB(ipAddress) == FALSE) &&
                (IP_LIB_IsIpInClassC(ipAddress) == FALSE) && (ip & mask != 0))
                    return NETCFG_TYPE_FAIL;

            /*  network b'cast ip   */
            if (IP_LIB_GetSubnetBroadcastIp(ipAddress, ipMask, &b_addr)==IP_LIB_OK)
            {
                if (b_addr == ipAddress)
                    return NETCFG_TYPE_FAIL;
            }

            /*  network ID  */
            if (ipAddress != 0)
            {
                if ((ip & mask )== ipAddress )
                    return NETCFG_TYPE_FAIL;
            }

            /* loopback ip, b'cast ip, or m'cast ip */
             if ((IP_LIB_IsLoopBackIp (ipAddress) == TRUE) ||
                (IP_LIB_IsMulticastIp(ipAddress) == TRUE) ||
                (IP_LIB_IsBroadcastIp(ipAddress) == TRUE))
                    return NETCFG_TYPE_FAIL;

            /* Filters ipAddress (0.0.0.x)*/
            if (((ip >> 24) & 0xff) == 0)
            {
                if (ip != SYS_DFLT_DEFAULT_IP)
                {

                    return NETCFG_TYPE_FAIL;
                }
                /* Only accepts default mask defines by sys_dflt.h
                 */
                if (mask != SYS_DFLT_DEFAULT_MASK)
                {

                    return NETCFG_TYPE_FAIL;
                }
            }

            memset(&intf, 0 , sizeof(NETCFG_OM_L3_Interface_T));
            intf.ifindex = vid_ifindex;
            memset(&overlap_rif, 0 , sizeof(NETCFG_OM_IP_InetRifConfig_T));
            overlap_rif.ifindex = vid_ifindex;
            overlap_rif.ipv4_role = NETCFG_TYPE_MODE_PRIMARY;
            memcpy(&overlap_rif.addr, &addr, sizeof(L_INET_AddrIp_T));


            if (NETCFG_TYPE_OK !=NETCFG_OM_IP_CheckAddressOverlap(&intf, &overlap_rif))
            {

                return NETCFG_TYPE_FAIL;
            }

            memset(&internal_rif_conf, 0, sizeof(NETCFG_OM_IP_InetRifConfig_T));
            internal_rif_conf.ifindex = vid_ifindex;
            internal_rif_conf.ipv4_role = NETCFG_TYPE_MODE_PRIMARY;
            internal_rif_conf.row_status = VAL_netConfigStatus_2_createAndGo;
            memcpy(&(internal_rif_conf.addr), &addr, sizeof(L_INET_AddrIp_T));

            /* Create Cluster l3 interface first */
            if(NETCFG_MGR_IP_CreateL3If(SYS_DFLT_CLUSTER_DEFAULT_VLAN) != NETCFG_TYPE_OK)
            {

                return NETCFG_TYPE_FAIL;
            }

            /* Change row status to Create and Wait. */


            if(NETCFG_MGR_IP_InternalSetInetRif(&internal_rif_conf)!= NETCFG_TYPE_OK)
            {

                if(rowStatus == VAL_netConfigStatus_2_createAndGo)
                {
                    internal_rif_conf.row_status = VAL_netConfigStatus_2_destroy;
                    res = NETCFG_MGR_IP_InternalSetInetRif(&internal_rif_conf);

                    return NETCFG_TYPE_FAIL;
                }
            }
            else
                return NETCFG_TYPE_OK;
            break;
        default:
            return NETCFG_TYPE_FAIL;
    }


}   /*  end of NETCFG_ClusterVlanSetRifRowStatus   */
#endif


static void NETCFG_MGR_IP_BackDoorMain(void)
{
    int ch;
    char buf[64];
    char *terminal;
    L_THREADGRP_Handle_T tg_handle;
    UI32_T               backdoor_member_id;

    tg_handle      = NETCFG_PROC_COMM_GetNetcfgTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while (1)
    {
        BACKDOOR_MGR_Printf("\n");
        BACKDOOR_MGR_Printf("3.  Dump OM L3 Interface Table\n");
        BACKDOOR_MGR_Printf("4.  Dump OM IPv4 Rif list\n");
#if (SYS_CPNT_IPV6 == TRUE)
        BACKDOOR_MGR_Printf("5.  Dump OM IPv6 Rif list\n");
        BACKDOOR_MGR_Printf("6.  test NETCFG_OM_IP_GetIPv6RifConfig\n");
        BACKDOOR_MGR_Printf("7.  Show Inet Rif of the interface in OM\n");
#endif

        BACKDOOR_MGR_Printf("9.  Test NETCFG_MGR_IP_DeleteAllInterface()\n");
        BACKDOOR_MGR_Printf("99. Set debug flag\n");
        BACKDOOR_MGR_Printf("0.  Exit\n");
        BACKDOOR_MGR_Printf("    select =");

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int) strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf ("\n");

        switch(ch)
        {
            case 0:
                BACKDOOR_MGR_Printf("\r\n");
                return;
            case 3:
                {
                    NETCFG_TYPE_L3_Interface_T intf;
                    memset(&intf, 0, sizeof(intf));
                    intf.ifindex = 0; // get from the first one.
                    BACKDOOR_MGR_Printf
                            (
                            "\n"
                            "ifindex"           /* 7 */
                            " "
                            "iftype"            /* 6 */
                            " "
                            "if_flags"          /* 8 */
#if 0
                            "drv_l3_id"         /* 9 */
#endif
                            " "
                            "v6_enable"         /* 9 */
                            " "
                            "v6_autoconf"       /* 11 */
                            " "
                            "hw_addr          " /* 17 */
                            " "
                            "v4_a_mode"         /* 9 */
#if (SYS_CPNT_DHCP_INFORM == TRUE)
                            " "
                            "dhcp"              /* 4 */
#endif
#if 0
                            " "
                            "int mtu"           /* 7 */
#endif
                            "\n");

                    BACKDOOR_MGR_Printf("===============================================================================\r\n");

                    while(NETCFG_OM_IP_GetNextL3Interface(&intf) == NETCFG_TYPE_OK)
                    {
                        BACKDOOR_MGR_Printf
                                ("%7lu"             /* 7 */
                                " "
                                "%6u"               /* 6 */
                                " "
                                "%8lu"              /* 8 */
                                " "
                                "%9u"               /* 9 */
                                " "
                                "%11u"              /* 11 */
                                " "
                                "%02x:%02x:%02x:%02x:%02x:%02x" /* 17 */
                                " "
                                "%9lu"              /* 9 */
#if (SYS_CPNT_DHCP_INFORM == TRUE)
                                " "
                                "%4s"               /* 4 */
#endif
#if 0
                                " "
                                //"%7lu"              /* 7 */
#endif
                                "\n"
                                ,
                                (unsigned long)intf.ifindex,
                                intf.iftype,
                                //intf.drv_l3_intf_index,
                                (unsigned long)intf.u.physical_intf.if_flags,
                                intf.ipv6_enable,
                                intf.ipv6_autoconf_enable,
                                intf.u.physical_intf.hw_addr[0],
                                intf.u.physical_intf.hw_addr[1],
                                intf.u.physical_intf.hw_addr[2],
                                intf.u.physical_intf.hw_addr[3],
                                intf.u.physical_intf.hw_addr[4],
                                intf.u.physical_intf.hw_addr[5],
                                (unsigned long)intf.u.physical_intf.ipv4_address_mode
#if (SYS_CPNT_DHCP_INFORM == TRUE)
                                ,intf.dhcp_inform?"Y":"N"
#endif
                                //intf.u.physical_intf.mtu
                                );

                    }
                }
                break;
            case 4:
                {
                    NETCFG_TYPE_InetRifConfig_T rif_config;
                    char tmp_buf[45] = {0};

                    memset(&rif_config, 0, sizeof(rif_config));
                    BACKDOOR_MGR_Printf
                            ("ip             "  /* 15 */
                            " "
                            "masklen"           /* 7 */
                            " "
                            "ifindex"           /* 7 */
                            "\n"
                            );

                    while(NETCFG_OM_IP_GetNextIPv4RifConfig(&rif_config) == NETCFG_TYPE_OK)
                    {

                        L_INET_InaddrToString((L_INET_Addr_T *) &rif_config.addr, tmp_buf, sizeof(tmp_buf));

                        BACKDOOR_MGR_Printf
                            ("%15s"             /* 15 */
                            " "
                            "%7u"               /* 7 */
                            " "
                            "%7lu"              /* 7 */
                            "\n"
                            ,
                            tmp_buf,        /* IP */
                            rif_config.addr.preflen, /* masklen */
                            (unsigned long)rif_config.ifindex  /* ifindex */
                            );
                        DUMP_RIF_ENTRY(&rif_config);
                    }
                }
                break;
#if (SYS_CPNT_IPV6 == TRUE)
            case 5:
                {
                    NETCFG_TYPE_InetRifConfig_T rif_config;
                    char tmp_buf[L_INET_MAX_IP6ADDR_STR_LEN] = {};


                    memset(&rif_config, 0, sizeof(rif_config));
                    BACKDOOR_MGR_Printf
                            ("type"  /* 4 */
                            " "
                            "pref"           /* 4 */
                            " "
                            "z-id"           /* 4 */
                            " "
                            "if-id"           /* 5 */
                            " "
                            "add_t"           /* 5 */
                            " "
                            "cf_t"           /* 4 */
                            " "
                            "addr"
                            "\n"
                            );
                    rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;
                    while(NETCFG_OM_IP_GetNextIPv6RifConfig(&rif_config) == NETCFG_TYPE_OK)
                    {
                        memset(tmp_buf, 0, sizeof(tmp_buf));
                        sprintf(tmp_buf+strlen(tmp_buf),"%lx:%lx:%lx:%lx",L_INET_EXPAND_IPV6(rif_config.addr.addr));
                        BACKDOOR_MGR_Printf
                            ("%4.1u"                /* 4 */
                            " "
                            "%4.2u"               /* 4 */
                            " "
                            "%4lu"                /* 4 */
                            " "
                            "%5lu"              /* 5 */
                            " "
                            "%5lu"           /* 5 */
                            " "
                            "%4lu"           /* 4 */
                            " "
                            "%s"             /* 20 */
                            "\n"
                            ,
                            rif_config.addr.type, /* type */
                            rif_config.addr.preflen, /* prefixlen */
                            (unsigned long)rif_config.addr.zoneid, /* zoneid */
                            (unsigned long)rif_config.ifindex,  /* ifindex */
                            (unsigned long)rif_config.ipv6_addr_type,
                            (unsigned long)rif_config.ipv6_addr_config_type,
                            tmp_buf            /* address */
                            );
                    }
                }
                break;
            case 6:
                /* test NETCFG_OM_IP_GetIPv6RifConfig*/
                {
                    NETCFG_TYPE_InetRifConfig_T inet_rif;
                    char tmp_buf[64] = {0};
                    UI32_T rc;

                    //L_INET_AddrIp_T inet_addr;
                    //UI32_T ui32;

                    memset(&inet_rif, 0, sizeof(inet_rif));
                    //memset(&inet_addr, 0, sizeof(inet_addr));

                    BACKDOOR_MGR_Printf("\nInput addr: (len <=64)");
                    BACKDOOR_MGR_RequestKeyIn(buf, 64);
                    L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC, buf, (L_INET_Addr_T *)&inet_rif.addr, sizeof(inet_rif.addr));

                    BACKDOOR_MGR_Printf("\nInput masklen: ");
                    BACKDOOR_MGR_RequestKeyIn(buf, 15);
                    inet_rif.addr.preflen = (UI16_T) strtoul(buf, NULL, 10); /* base is 10 */

                    BACKDOOR_MGR_Printf("\nInput ifindex: ");
                    BACKDOOR_MGR_RequestKeyIn(buf, 15);

                    inet_rif.ifindex = strtoul(buf, NULL, 10);

                    //tmp_buf = {};
                    BACKDOOR_MGR_Printf("\nInput: ");
                    L_INET_InaddrToString((L_INET_Addr_T *) &inet_rif.addr, tmp_buf, sizeof(tmp_buf));
                    BACKDOOR_MGR_Printf("inet_rif.addr: %s ", tmp_buf);
                    BACKDOOR_MGR_Printf("masklen: %d ", inet_rif.addr.preflen);
                    BACKDOOR_MGR_Printf("ifindex: %lu\n", (unsigned long)inet_rif.ifindex);

                    rc = NETCFG_OM_IP_GetIPv6RifConfig(&inet_rif);

                    BACKDOOR_MGR_Printf("\nOutput: ");
                    if(NETCFG_TYPE_OK == rc)
                    {
                        L_INET_InaddrToString((L_INET_Addr_T *) &inet_rif.addr, tmp_buf, sizeof(tmp_buf));
                        BACKDOOR_MGR_Printf("inet_rif.addr: %s ", tmp_buf);
                        BACKDOOR_MGR_Printf("masklen: %d ", inet_rif.addr.preflen);
                        BACKDOOR_MGR_Printf("ifindex: %lu\n", (unsigned long)inet_rif.ifindex);
                    }
                    else
                        BACKDOOR_MGR_Printf("InetRifconfig not found.\n");
                }
                break;
            case 7:
                {
                    UI32_T ifindex;
                    BACKDOOR_MGR_Printf("\nInput ifindex: ");
                    BACKDOOR_MGR_RequestKeyIn(buf, 15);
                    ifindex = (UI16_T) strtoul(buf, NULL, 10); /* base is 10 */

                    NETCFG_OM_IP_Debug_ShowInetRifOfInterface(ifindex);
                }
                break;
#endif
            case 9:
                /* Test enter transition mode from backdoor */
                NETCFG_MGR_IP_DeleteAllInterface();
                break;
            case 99:
            {
                UI8_T  debug_index;
                BOOL_T exit = FALSE;
                while (1)
                {
                    BACKDOOR_MGR_Printf("1:Debug (%s)\r\n",(debug_flag&IPCFG_DEBUG_FLAG_BIT_DEBUG)?"ON":"OFF");
                    BACKDOOR_MGR_Printf("2:Info  (%s)\r\n",(debug_flag&IPCFG_DEBUG_FLAG_BIT_INFO)?"ON":"OFF");
                    BACKDOOR_MGR_Printf("3:Note  (%s)\r\n",(debug_flag&IPCFG_DEBUG_FLAG_BIT_NOTE)?"ON":"OFF");
                    BACKDOOR_MGR_Printf("4:Tunnel(%s)\r\n",(debug_flag&IPCFG_DEBUG_FLAG_BIT_TUNNEL)?"ON":"OFF");
                    BACKDOOR_MGR_Printf("0:Exit\r\n");
                    BACKDOOR_MGR_Printf("Enter debug type: ");
                    BACKDOOR_MGR_RequestKeyIn(buf, 15);
                    debug_index = (UI8_T) strtoul(buf, NULL, 10); /* base is 10 */
                    switch(debug_index)
                    {
                        case 1:
                            debug_flag = debug_flag^IPCFG_DEBUG_FLAG_BIT_DEBUG;
                        break;
                        case 2:
                            debug_flag = debug_flag^IPCFG_DEBUG_FLAG_BIT_INFO;
                        break;
                        case 3:
                            debug_flag = debug_flag^IPCFG_DEBUG_FLAG_BIT_NOTE;
                        break;
                        case 4:
                            debug_flag = debug_flag^IPCFG_DEBUG_FLAG_BIT_TUNNEL;
                        break;
                        case 0:
                            exit = TRUE;
                        break;
                    }

                    if(exit == TRUE)
                        break;

                /* also debug OM */
                    NETCFG_OM_IP_SetDebugFlag(debug_flag);
                    BACKDOOR_MGR_Printf("\r\n");
                }
            }
                break;

            default:
                break;
        }
    }

    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
}

/* FUNCTION NAME : NETCFG_MGR_IP_NsmRouteChange_CallBack
 * PURPOSE:
 *      when nsm has ipv4 route change, it will call back to netcfg_mgr_ip.
 *      If there's a route change of tunnel's next hop, we must change status of tunnel interface
 *
 *
 * INPUT:
 *      address_family  -- ipv4 or ipv6
 *
 * OUTPUT:
 *      NONE
 *
 * RETURN:
 *      NONE
 *
 *
 * NOTES:
 *      We don't define address family,
 *      this callback is only used when ipv4 route change in this moment.
 *
 *      If it needs add ipv6 route change call back in the future,
 *      it must define the address family to distinguish ipv4 and ipv6.
 */
void NETCFG_MGR_IP_NsmRouteChange_CallBack(UI32_T address_family)
{
    return;
}

static BOOL_T NETCFG_MGR_FSM(UI32_T action, UI32_T *state, BOOL_T (*active_check_fun)(void*rec), void *rec)
{

    TUNNELprintf("state=%s, action=%s",RAW_STATUS_TO_STRING(*state),RAW_STATUS_TO_STRING(action));
    switch (action)
    {
        case L_RSTATUS_CREATEANDGO:
            switch(*state)
            {
                case L_RSTATUS_NOT_EXIST:
                    *state = ((*active_check_fun)(rec))?L_RSTATUS_ACTIVE:L_RSTATUS_NOTREADY;
                    return TRUE;
                case L_RSTATUS_NOTREADY:
                case L_RSTATUS_NOTINSERVICE:
                case L_RSTATUS_ACTIVE:
                default:
                    return FALSE;
            }
            break;
        case L_RSTATUS_CREATEANDWAIT:
            switch(*state)
            {
                case L_RSTATUS_NOT_EXIST:
                    *state = ((*active_check_fun)(rec))?L_RSTATUS_NOTINSERVICE:L_RSTATUS_NOTREADY;
                    return TRUE;
                case L_RSTATUS_NOTREADY:
                case L_RSTATUS_NOTINSERVICE:
                case L_RSTATUS_ACTIVE:
                default:
                    return FALSE;
            }
            break;
        case L_RSTATUS_ACTIVE:
            switch(*state)
            {
                case L_RSTATUS_NOT_EXIST:
                    return FALSE;
                case L_RSTATUS_NOTREADY:
                case L_RSTATUS_NOTINSERVICE:
                    *state = ((*active_check_fun)(rec))?L_RSTATUS_ACTIVE:L_RSTATUS_NOTREADY;
                    return TRUE;
                case L_RSTATUS_ACTIVE:
                    return TRUE;
                default:
                    return FALSE;
            }
            break;
        case L_RSTATUS_NOTINSERVICE:
            switch(*state)
            {
                case L_RSTATUS_NOT_EXIST:
                    return FALSE;
                case L_RSTATUS_NOTREADY:
                case L_RSTATUS_NOTINSERVICE:
                case L_RSTATUS_ACTIVE:
                    *state = L_RSTATUS_NOTINSERVICE;
                    return TRUE;
                default:
                    return FALSE;
            }
            break;
        case L_RSTATUS_DESTROY:
            switch(*state)
            {
                case L_RSTATUS_NOT_EXIST:
                    return FALSE;
                case L_RSTATUS_NOTREADY:
                case L_RSTATUS_NOTINSERVICE:
                case L_RSTATUS_ACTIVE:
                    *state = L_RSTATUS_NOT_EXIST;
                    return TRUE;
                default:
                    return FALSE;
            }
            break;
        case L_RSTATUS_SET_OTHER_COLUMN:
            switch(*state)
            {
                case L_RSTATUS_NOT_EXIST:
                    return TRUE;
                case L_RSTATUS_NOTREADY:
                    *state = ((*active_check_fun)(rec))?L_RSTATUS_NOTINSERVICE:L_RSTATUS_NOTREADY;
                    return TRUE;
                case L_RSTATUS_NOTINSERVICE:
                    return TRUE;
                case L_RSTATUS_ACTIVE:
                    return FALSE;
                default:
                    return FALSE;
            }
            break;
        default:
            return FALSE;

    }

}

#if (SYS_CPNT_CRAFT_PORT == TRUE)
static UI32_T NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p, BOOL_T om_only)
{
    NETCFG_TYPE_CraftInetAddress_T local_addr;
    UI32_T rc = 1;

    switch(craft_addr_p->row_status)
    {
        case VAL_netConfigStatus_2_createAndGo:
            if (FALSE == om_only)
            {
            rc = IPAL_IF_AddIpAddress(craft_addr_p->ifindex, &(craft_addr_p->addr));
            if(rc)
                return NETCFG_TYPE_FAIL;
            }
            memcpy(&local_addr, craft_addr_p, sizeof(local_addr));
            local_addr.row_status = VAL_netConfigStatus_2_active;
            switch(local_addr.addr.type)
            {
                case L_INET_ADDR_TYPE_IPV4:
                case L_INET_ADDR_TYPE_IPV4Z:
                    NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(local_addr), &local_addr);
                    break;
                case L_INET_ADDR_TYPE_IPV6:
                    NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL, sizeof(local_addr), &local_addr);
                    break;
                case L_INET_ADDR_TYPE_IPV6Z:
                    NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(local_addr), &local_addr);
                    break;
                default:
                    return NETCFG_TYPE_FAIL;
            }
            {
                NETCFG_TYPE_InetRifConfig_T rif;
                memset(&rif, 0, sizeof(rif));
                rif.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
                memcpy(&(rif.addr), &(craft_addr_p->addr), sizeof(L_INET_AddrIp_T));
                NETCFG_MGR_IP_SignalAllProtocolRifCreate(&rif);
            }
            return NETCFG_TYPE_OK;
        case VAL_netConfigStatus_2_destroy:
        {
            NETCFG_TYPE_InetRifConfig_T rif;
            memset(&rif, 0, sizeof(rif));
            rif.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
            memcpy(&(rif.addr), &(craft_addr_p->addr), sizeof(L_INET_AddrIp_T));
            NETCFG_MGR_IP_SignalAllProtocolRifDown(&rif);

            if (FALSE == om_only)
            {
                rc = IPAL_IF_DeleteIpAddress(craft_addr_p->ifindex, &(craft_addr_p->addr));
                if(rc)
                {
                    return NETCFG_TYPE_FAIL;
                }
            }

            memset(&local_addr, 0, sizeof(local_addr));
            switch(craft_addr_p->addr.type)
            {
                case L_INET_ADDR_TYPE_IPV4:
                case L_INET_ADDR_TYPE_IPV4Z:
                    NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(local_addr), &local_addr);
                    break;
                case L_INET_ADDR_TYPE_IPV6:
                    NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL, sizeof(local_addr), &local_addr);
                    break;
                case L_INET_ADDR_TYPE_IPV6Z:
                    NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(local_addr), &local_addr);
                    break;
                default:
                    return NETCFG_TYPE_FAIL;
            }
            NETCFG_MGR_IP_SignalAllProtocolRifDelete(&rif);
            return NETCFG_TYPE_OK;
        }
        default: /* not support */
            return NETCFG_TYPE_FAIL;
    }
}

/* FUNCTION NAME : NETCFG_MGR_IP_IPv6_SetAutoLinkLocal_Craft
 * PURPOSE:
 *      To enable IPv6 processing on a craft interface that has not been configured
 *      with an explicit IPv6 address.
 *
 * INPUT:
 *      ifindex     -- interface index
 *      om_only     -- only set om, without setting to kernel
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. It will automatically configures an IPv6 link-local unicast address
 *         on the interface while also enabling the interface for IPv6 processing.
 */
UI32_T NETCFG_MGR_IP_IPv6_SetAutoLinkLocal_Craft(UI32_T ifindex, BOOL_T om_only)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    NETCFG_TYPE_CraftInetAddress_T link_local;
    NETCFG_TYPE_CraftInetAddress_T craft_addr;
    UI8_T cpu_mac[SYS_ADPT_MAC_ADDR_LEN] = {};
    UI32_T rc;
    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(link_local), (void*) &link_local))
    {
        return NETCFG_TYPE_OK;
    }

    STKTPLG_POM_GetLocalUnitBaseMac(cpu_mac);

    memset(&craft_addr, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
    craft_addr.ifindex = ifindex;
    craft_addr.addr.type = L_INET_ADDR_TYPE_IPV6Z;
    craft_addr.addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;

    craft_addr.addr.preflen = 64;
    craft_addr.addr.addr[0] = 0xFE;
    craft_addr.addr.addr[1] = 0x80;


    NETCFG_MGR_IP_IPv6_IPv6AddressEUI64(craft_addr.addr.addr, 64, cpu_mac);
    craft_addr.row_status = VAL_netConfigStatus_2_createAndGo;
    craft_addr.addr.type = L_INET_ADDR_TYPE_IPV6Z;
    craft_addr.addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    craft_addr.addr.preflen = 64;
    craft_addr.addr.zoneid = ifindex-SYS_ADPT_VLAN_1_IF_INDEX_NUMBER+1;

    craft_addr.ipv6_addr_config_type = NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER;
    craft_addr.ipv6_addr_type = NETCFG_TYPE_IPV6_ADDRESS_TYPE_LINK_LOCAL;

    rc = NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(&craft_addr, om_only);

    /* set interface up */
    if(NETCFG_TYPE_OK == rc && FALSE == om_only)
        IPAL_IF_SetIfFlags(ifindex, IFF_UP| IFF_RUNNING);

    return rc;

}
UI32_T NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal_Craft(UI32_T ifindex)
{
    NETCFG_TYPE_CraftInetAddress_T link_local, ipv4;
    UI32_T rc;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(link_local), (void*) &link_local)
        && link_local.ipv6_addr_config_type == NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER)
    {
        link_local.row_status = VAL_netConfigStatus_2_destroy;
        rc = NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(&link_local, FALSE /* om_only */);
        if(NETCFG_TYPE_OK != rc)
            return rc;
        /* set interface down, if there is no ipv4 address */
        if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(ipv4), (void*) &ipv4))
        {
            IPAL_IF_UnsetIfFlags(ifindex, IFF_UP| IFF_RUNNING);
        }
    }

    return NETCFG_TYPE_OK;
}
/* FUNCTION NAME : NETCFG_MGR_IP_SetCraftInterfaceInetAddress
 * PURPOSE:
 *      To add or delete ipv4/v6 address on craft interface
 * INPUT:
 *      rif_p               -- pointer to rif
 *      om_only             -- only set om, without setting to kernel
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_MGR_IP_SetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p, BOOL_T om_only)
{
    NETCFG_TYPE_CraftInetAddress_T link_local;
    NETCFG_TYPE_CraftInetAddress_T ipv4;
    UI32_T rc;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    switch(craft_addr_p->addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            switch (craft_addr_p->row_status)
            {
                case VAL_netConfigStatus_2_createAndGo:
                    /* check overlape with rif */
                    if(NETCFG_TYPE_OK != NETCFG_OM_IP_CheckAddressOverlap_Craft(craft_addr_p))
                        return NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED;
                    /* check static route's nexthop on craft int. conflicts with normal L3 int. */
                    {
                        ROUTE_MGR_IpCidrRouteEntry_T route_entry;
                        memset(&route_entry, 0, sizeof(route_entry));
                        route_entry.ip_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
                        while(NETCFG_TYPE_OK == NETCFG_OM_ROUTE_GetNextStaticIpCidrRoute(&route_entry))
                        {
                            if(IP_LIB_IsIpBelongToSubnet(craft_addr_p->addr.addr, craft_addr_p->addr.preflen, route_entry.ip_cidr_route_next_hop.addr))
                            {
                                UI32_T num_nh = 0;

                                NETCFG_OM_ROUTE_GetAllNextHopNumberOfRoute(&route_entry, &num_nh);
                                if(num_nh >1)
                                {
                                    return NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT;
                                }
                            }
                        }
                    }

                    /* there should be only one ipv4 address, delete old one */
                    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(ipv4), (void*) &ipv4))
                    {
                        if(!memcmp(&(craft_addr_p->addr), &(ipv4.addr), sizeof(ipv4.addr)))
                            return NETCFG_TYPE_OK;
                        else
                        {
                            ipv4.row_status = VAL_netConfigStatus_2_destroy;
                            NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(&ipv4, om_only);
                        }
                    }

                    if(NETCFG_TYPE_OK == NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(craft_addr_p, om_only))
                    {
                        /* set interface up */
                        if (FALSE == om_only)
                            IPAL_IF_SetIfFlags(craft_addr_p->ifindex, IFF_UP| IFF_RUNNING);

                        /* users config IP address -> stop dhclient from doing DHCP
                         */
                        if (craft_addr_p->by_user_mode_helper_func != 1)
                        {
                            NETCFG_MGR_IP_SetCraftRunDHCP(FALSE);
                        }
                        return NETCFG_TYPE_OK;

                    }
                    else
                        return NETCFG_TYPE_FAIL;
                case VAL_netConfigStatus_2_destroy:
                    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(ipv4), (void*) &ipv4)
                        && !memcmp(&(craft_addr_p->addr), &(ipv4.addr), sizeof(ipv4.addr)))
                    {
                        NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(craft_addr_p, om_only);

                        /* set interface down, if there is no ipv6 link-local address */
                        if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(link_local), (void*) &link_local))
                        {
                            if (FALSE == om_only)
                            IPAL_IF_UnsetIfFlags(craft_addr_p->ifindex, IFF_UP| IFF_RUNNING);
                        }

                        /* no IP address -> start dhclient to do DHCP
                         */
                        if (craft_addr_p->by_user_mode_helper_func != 1)
                        {
                            NETCFG_MGR_IP_SetCraftRunDHCP(TRUE);
                        }
                        return NETCFG_TYPE_OK;
                    }
                    else
                        return NETCFG_TYPE_FAIL;

                default:
                    return NETCFG_TYPE_FAIL;
            }
            break;
        case L_INET_ADDR_TYPE_IPV6Z: /* manual link-local */
        {
            NETCFG_TYPE_CraftInetAddress_T global;
            BOOL_T ipv6_enable;

            rc = NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(craft_addr_p, om_only);
            if(NETCFG_TYPE_OK != rc)
                return rc;
            switch (craft_addr_p->row_status)
            {
                case VAL_netConfigStatus_2_createAndGo:
                    /* set interface up */
                    if (FALSE == om_only)
                        IPAL_IF_SetIfFlags(craft_addr_p->ifindex, IFF_UP| IFF_RUNNING);
                    break;
                case VAL_netConfigStatus_2_destroy:
                    /* set interface down, if there is no ipv4 address */
                    if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(ipv4), (void*) &ipv4))
                    {
                        if (FALSE == om_only)
                            IPAL_IF_UnsetIfFlags(craft_addr_p->ifindex, IFF_UP| IFF_RUNNING);
                    }
                    break;
                default:
                    return NETCFG_TYPE_FAIL;
            }

            /* If we are going to destroy a link-local address, but
             * the interface is ipv6 enabled explicitly or autoconfig enabled or has global address
             * we need to add auto link-local addr back.
             */
            if(craft_addr_p->row_status == VAL_netConfigStatus_2_destroy)
            {
                if((NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL, sizeof(global), (void*) &global))
                || (NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ENABLE, sizeof(ipv6_enable), (void*) &ipv6_enable)&& ipv6_enable == TRUE))
                {
                    rc = NETCFG_MGR_IP_IPv6_SetAutoLinkLocal_Craft(craft_addr_p->ifindex, om_only);
                    /* set interface up */
                    if(NETCFG_TYPE_OK == rc &&  FALSE == om_only)
                        IPAL_IF_SetIfFlags(craft_addr_p->ifindex, IFF_UP| IFF_RUNNING);

                }
            }
#if (SYS_CPNT_CRAFT_DHCLIENT == TRUE)
            if (craft_addr_p->by_user_mode_helper_func != 1)
            {
                if (craft_addr_p->row_status == VAL_netConfigStatus_2_createAndGo)
                    NETCFG_MGR_IP_SetCraftRunDHCP(FALSE);
                else /* VAL_netConfigStatus_2_destroy */
                    NETCFG_MGR_IP_SetCraftRunDHCP(TRUE);
            }
#endif
            return NETCFG_TYPE_OK;
        }
            break;
        case L_INET_ADDR_TYPE_IPV6:
        {

            switch (craft_addr_p->row_status)
            {
                case VAL_netConfigStatus_2_createAndGo:
                {
                    NETCFG_TYPE_CraftInetAddress_T global;

                    /* check overlape with rif */
                    if(NETCFG_TYPE_OK != NETCFG_OM_IP_CheckAddressOverlap_Craft(craft_addr_p))
                        return NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED;

                    /* there should be only one ipv6 global address, delete old one */
                    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL, sizeof(global), (void*) &global))
                    {
                        if(!memcmp(&(craft_addr_p->addr), &(global.addr), sizeof(global.addr)))
                            return NETCFG_TYPE_OK;
                        else
                        {
                            global.row_status = VAL_netConfigStatus_2_destroy;
                            rc = NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(&global, om_only);
                            if (NETCFG_TYPE_OK != rc)
                                return NETCFG_TYPE_FAIL;
                        }
                    }

                    if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(link_local), (void*) &link_local))
                    {
                        /* there is no link-local yet, add one first. */
                        NETCFG_MGR_IP_IPv6_SetAutoLinkLocal_Craft(craft_addr_p->ifindex, om_only);
                        /* set interface up */
                        if (FALSE == om_only)
                            IPAL_IF_SetIfFlags(craft_addr_p->ifindex, IFF_UP| IFF_RUNNING);

                    }
                    rc = NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(craft_addr_p, om_only);
#if (SYS_CPNT_CRAFT_DHCLIENT == TRUE)
                    if (NETCFG_TYPE_OK == rc && craft_addr_p->by_user_mode_helper_func != 1)
                    {
                        NETCFG_MGR_IP_SetCraftRunDHCP(FALSE);
                    }
#endif
                    return rc;
                    break;
                }
                case VAL_netConfigStatus_2_destroy:
                {
                    BOOL_T ipv6_enable;

                    rc = NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(craft_addr_p, om_only);
                    if(NETCFG_TYPE_OK != rc)
                        return rc;
                    /* If the address is the LAST ipv6 global address and action is destroy,
                     * (there is no global address anymore)
                     * we need to check if the interface is not "ipv6 enable" explicitly
                     * and if the link-local is auto, then also destroy it.
                     */
                    if((NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(link_local), (void*) &link_local)
                            && link_local.ipv6_addr_config_type == NETCFG_TYPE_IPV6_ADDRESS_CONFIG_TYPE_AUTO_OTHER)
                        && (NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ENABLE, sizeof(ipv6_enable), (void*) &ipv6_enable)
                            && ipv6_enable == FALSE))
                    {
                        NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal_Craft(craft_addr_p->ifindex);
                        /* set interface down, if there is no ipv4 address */
                        if(NETCFG_TYPE_OK != NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(ipv4), (void*) &ipv4))
                        {
                            if (FALSE == om_only)
                            IPAL_IF_UnsetIfFlags(craft_addr_p->ifindex, IFF_UP| IFF_RUNNING);
                        }
                    }
                    if (craft_addr_p->by_user_mode_helper_func != 1)
                    {
                        NETCFG_MGR_IP_SetCraftRunDHCP(TRUE);
                    }
                    return NETCFG_TYPE_OK;
                }
                    break;
                default:
                    return NETCFG_TYPE_FAIL;
            }
            break;
        }
        default:
            return NETCFG_TYPE_FAIL;
    }
    return NETCFG_TYPE_FAIL;
}
/* FUNCTION NAME : NETCFG_MGR_IP_IPv6Enable_Craft
 * PURPOSE:
 *      To enable/disable ipv6 address on craft interface
 * INPUT:
 *      ifindex
 *      do_enable           -- enable/disable
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_MGR_IP_IPv6Enable_Craft(UI32_T ifindex, BOOL_T do_enable)
{
    NETCFG_TYPE_CraftInetAddress_T ipv6_global;
    UI32_T rc;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    if(do_enable)
    {
        rc = NETCFG_MGR_IP_IPv6_SetAutoLinkLocal_Craft(ifindex, FALSE /* om_only */);
        if(NETCFG_TYPE_OK != rc)
            return NETCFG_TYPE_FAIL;

        NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ENABLE, sizeof(do_enable), &do_enable);
        return NETCFG_TYPE_OK;
    }
    else
    {
        rc = NETCFG_OM_IP_SetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ENABLE, sizeof(do_enable), &do_enable);
        if(NETCFG_TYPE_OK != rc)
            return NETCFG_TYPE_FAIL;
        /* check ipv6 global address */
        memset(&ipv6_global, 0, sizeof(ipv6_global));
        if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL, sizeof(ipv6_global), (void*) &ipv6_global))
            return NETCFG_TYPE_HAS_EXPLICIT_IPV6_ADDR;
        else
            return NETCFG_MGR_IP_IPv6_UnsetAutoLinkLocal_Craft(ifindex);
    }
}

/* FUNCTION NAME : NETCFG_MGR_IP_DeleteAllCraftInterfaceInetAddress
 * PURPOSE:
 *      To delete all  ip/ipv6 address on craft interface
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 * NOTES:
 *      None
 */
static UI32_T NETCFG_MGR_IP_DeleteAllCraftInterfaceInetAddress()
{
    NETCFG_TYPE_CraftInetAddress_T ipv4;
    NETCFG_TYPE_CraftInetAddress_T ipv6_link;
    NETCFG_TYPE_CraftInetAddress_T ipv6_global;
    UI32_T rc;

    /* delete ipv4 address */
    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR, sizeof(ipv4), (void*) &ipv4))
    {
        ipv4.row_status = VAL_netConfigStatus_2_destroy;
        NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(&ipv4, FALSE /* om_only */);
    }

    /* delete ipv6 link-local address */
    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK, sizeof(ipv6_link), (void*) &ipv6_link))
    {
        ipv6_link.row_status = VAL_netConfigStatus_2_destroy;
        NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(&ipv6_link, FALSE /* om_only */);
    }

    /* delete ipv6 global address */
    if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetCraftInterfaceValue(NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL, sizeof(ipv6_global), (void*) &ipv6_global))
    {
        ipv6_global.row_status = VAL_netConfigStatus_2_destroy;
        NETCFG_MGR_IP_InternalSetCraftInterfaceInetAddress(&ipv6_global, FALSE /* om_only */);
    }
    return NETCFG_TYPE_OK;
}


/* to sync CRAFT port's IP address from kernel of OM
 */
static void NETCFG_MGR_IP_SyncKernelCraftInterfaceInetAddress()
{
    L_INET_AddrIp_T p;

    if (IPAL_RESULT_OK == IPAL_IF_GetIfIpv4Addr(SYS_ADPT_CRAFT_INTERFACE_IFINDEX, &p) ||
        IPAL_RESULT_OK == IPAL_IF_GetIfIpv6Addr(SYS_ADPT_CRAFT_INTERFACE_IFINDEX, &p)
        )
    {
        NETCFG_TYPE_CraftInetAddress_T craft_rif;
        NETCFG_TYPE_InetRifConfig_T rif;
        UI32_T rc_om, rc;

        memset(&craft_rif, 0, sizeof(craft_rif));
        memset(&rif, 0, sizeof(rif));

        rif.ifindex = craft_rif.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
        rif.addr.type = craft_rif.addr.type = p.type;
        rif.addr.preflen = p.preflen;
        memcpy(rif.addr.addr, p.addr, SYS_ADPT_IPV6_ADDR_LEN);
        rif.ipv4_role = NETCFG_TYPE_MODE_PRIMARY;
        rc_om = NETCFG_OM_IP_GetCraftInterfaceInetAddress(&craft_rif);

        if (NETCFG_TYPE_OK != rc_om ||
           (NETCFG_TYPE_OK == rc_om && (rif.addr.preflen != craft_rif.addr.preflen ||
                0 != memcmp(rif.addr.addr, craft_rif.addr.addr, SYS_ADPT_IPV6_ADDR_LEN))))
        {
            craft_rif.row_status = VAL_netConfigStatus_2_createAndGo;
            craft_rif.addr.preflen = rif.addr.preflen;
            memcpy(craft_rif.addr.addr, rif.addr.addr, SYS_ADPT_IPV6_ADDR_LEN);
            craft_rif.by_user_mode_helper_func = 1; /* not set via AOS CLI */
            if(NETCFG_TYPE_OK != (rc=NETCFG_MGR_IP_SetCraftInterfaceInetAddress(&craft_rif, TRUE)))
            {

                /* delete rif from kernel
                 */
                rc = IPAL_IF_DeleteIpAddress(SYS_ADPT_CRAFT_INTERFACE_IFINDEX, &p);
             //   if (IPAL_RESULT_OK != rc)
             //       printf("%s:failed to delete rif from kernel.\n",__FUNCTION__);
            }
            else
            {
                NETCFG_MGR_IP_SignalAllProtocolRifActive(&rif);
            }
        }
    }
}

#endif /* SYS_CPNT_CRAFT_PORT */



#if (SYS_CPNT_DHCP_INFORM == TRUE)
/* FUNCTION NAME : NETCFG_MGR_IP_SetDhcpInform
 * PURPOSE:
 *      To enable/disable dhcp inform on L3 interface
 * INPUT:
 *      ifindex
 *      do_enable           -- enable/disable
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_MGR_IP_SetDhcpInform(UI32_T ifindex, BOOL_T do_enable)
{
    BOOL_T dhcp_inform = FALSE;
    NETCFG_TYPE_L3_Interface_T intf;
    UI32_T rc = NETCFG_TYPE_FAIL;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return NETCFG_TYPE_NOT_MASTER_MODE;

    NETCFG_OM_IP_GetDhcpInform(ifindex, &dhcp_inform);

    /* the same setting with original, do nothing */
    if(dhcp_inform == do_enable)
    {
        return NETCFG_TYPE_OK;
    }
    else
    {
        rc = NETCFG_OM_IP_SetDhcpInform(ifindex, do_enable);
        if(NETCFG_TYPE_OK != rc)
            return NETCFG_TYPE_FAIL;

        memset(&intf, 0, sizeof(intf));
        intf.ifindex = ifindex;
        rc = NETCFG_OM_IP_GetL3Interface(&intf);
        if(NETCFG_TYPE_OK != rc)
            return NETCFG_TYPE_FAIL;

        if(NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE==intf.u.physical_intf.ipv4_address_mode)
        {
            SYS_CALLBACK_MGR_DHCPRestart3Callback(SYS_MODULE_IPCFG, DHCP_MGR_RESTART_CLIENT);
        }
    }
    return NETCFG_TYPE_OK;
}
#endif /* SYS_CPNT_DHCP_INFORM */

