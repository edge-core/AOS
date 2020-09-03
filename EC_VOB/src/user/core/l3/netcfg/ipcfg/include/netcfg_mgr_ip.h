/* Module Name: netcfg_mgr_ip.h
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

#ifndef NETCFG_MGR_IP_H
#define NETCFG_MGR_IP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "ipal_types.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NETCFG_MGR_IP_MSGBUF_TYPE_SIZE sizeof(union NETCFG_MGR_IP_IPCMsg_Type_U)

/* For Exceptional Handler */
enum NETCFG_MGR_IP_FUN_NO_E
{
    NETCFG_MGR_IP_CREATE_VLAN_INTERFACE_FUN_NO = 1,
    NETCFG_MGR_IP_DELETE_VLAN_INTERFACE_FUN_NO,
    NETCFG_MGR_IP_SET_IPV4_RIF_FUN_NO
};

/* definitions of command in CSCA which will be used in ipc message
 */
enum
{
    /* IP Configuration */
    NETCFG_MGR_IP_IPCCMD_CREATEL3IF,
    NETCFG_MGR_IP_IPCCMD_CREATELOOPBACKIF,
    NETCFG_MGR_IP_IPCCMD_DELETEL3IF,
    NETCFG_MGR_IP_IPCCMD_DELETELOOPBACKIF,
    NETCFG_MGR_IP_IPCCMD_SETIPADDRESSMODE,
    NETCFG_MGR_IP_IPCCMD_DELETEIPADDRESSMODE,
    NETCFG_MGR_IP_IPCCMD_SETINETRIF,
#if (SYS_CPNT_PROXY_ARP == TRUE)    
    NETCFG_MGR_IP_IPCCMD_SETARPPROXYSTATUS,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGARPPROXYSTATUS,
#endif    
#if (SYS_CPNT_IPV6 == TRUE)
    NETCFG_MGR_IP_IPCCMD_IPV6ENABLE,
    NETCFG_MGR_IP_IPCCMD_IPV6DISABLE,
    NETCFG_MGR_IP_IPCCMD_IPV6ADDRAUTOCONFIGENABLE,
    NETCFG_MGR_IP_IPCCMD_IPV6ADDRAUTOCONFIGDISABLE,
    NETCFG_MGR_IP_IPCCMD_IPV6SETINTERFACEMTU,
    NETCFG_MGR_IP_IPCCMD_IPV6UNSETINTERFACEMTU,
    NETCFG_MGR_IP_IPCCMD_IPV6GETINTERFACEMTU,
    NETCFG_MGR_IP_IPCCMD_IPV6SETUNICASTROUTING,
    NETCFG_MGR_IP_IPCCMD_GETNEXTIFJOINIPV6MCASTADDR,
    NETCFG_MGR_IP_IPCCMD_GETNEXTPMTUENTRY,
    NETCFG_MGR_IP_IPCCMD_GETIFIPV6ADDRINFO,
    NETCFG_MGR_IP_IPCCMD_DHCPRELEASECOMPLETE,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6ENABLESTATUS,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6ADDRAUTOCONFIGENABLESTATUS,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6UNICASTROUTING,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGIPV6INTERFACEMTU,
    NETCFG_MGR_IP_IPCCMD_GETIPV6STATISTICS,
    NETCFG_MGR_IP_IPCCMD_GETICMPV6STATISTICS,
    NETCFG_MGR_IP_IPCCMD_GETUDPV6STATISTICS,
    NETCFG_MGR_IP_IPCCMD_CLEARIPV6STATISTICSBYTYPE,
#endif /* SYS_CPNT_IPV6 */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
    NETCFG_MGR_IP_IPCCMD_CREATETUNNELINTERFACE,
    NETCFG_MGR_IP_IPCCMD_DELETETUNNELINTERFACE,
    NETCFG_MGR_IP_IPCCMD_SETTUNNELRIF,
    NETCFG_MGR_IP_IPCCMD_SETTUNNELSOURCEVLAN,
    NETCFG_MGR_IP_IPCCMD_DELETETUNNELSOURCEVLAN,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGTUNNELSOURCEVLAN,
    NETCFG_MGR_IP_IPCCMD_SETTUNNELDESTINATION,
    NETCFG_MGR_IP_IPCCMD_UNSETTUNNELDESTINATION,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGTUNNELDESTINATION,
    NETCFG_MGR_IP_IPCCMD_SETTUNNELMODE,
    NETCFG_MGR_IP_IPCCMD_UNSETTUNNELMODE,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGTUNNELMODE,
    NETCFG_MGR_IP_IPCCMD_SETTUNNELTTL,
    NETCFG_MGR_IP_IPCCMD_UNSETTUNNELTTL,
    NETCFG_MGR_IP_IPCCMD_GETRUNNINGTUNNELTTL,
    NETCFG_MGR_IP_IPCCMD_GETTUNNELIFINDEXBYSRCVIDIFINDEX,
    NETCFG_MGR_IP_IPCCMD_SET_TUNNEL_ROWSTATUS,
    NETCFG_MGR_IP_IPCCMD_SET_TUNNEL_INTERFACE_FROM_SNMP,
 #endif /* SYS_CPNT_IP_TUNNEL */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    NETCFG_MGR_IP_IPCCMD_SETCRAFTINTERFACEINETADDRESS,
    NETCFG_MGR_IP_IPCCMD_IPV6_IPV6ENABLE_CRAFT,
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    NETCFG_MGR_IP_IPCCMD_SET_DHCP_INFORM,
#endif

    NETCFG_MGR_IP_IPCCMD_CLUSTERVLANSETRIFROWSTATUS

};

/* Macro function for calculation of ipc msg_buf size based on structure name
 */
#define NETCFG_MGR_IP_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_IP_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_IP_IPCMsg_T*)0)->data.field_name))


/* DATA TYPE DECLARATIONS
 */
/* structure for the request/response ipc message in csca pmgr and mgr
 */
typedef struct
{
    union NETCFG_MGR_IP_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        I32_T result_i32;  /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type;

    union
    {
        BOOL_T  bool_v;
        UI8_T   ui8_v;
        I8_T    i8_v;
        UI32_T  ui32_v;
        UI16_T  ui16_v;
        I32_T   i32_v;
        I16_T   i16_v;
        UI8_T   ip4_v[SYS_ADPT_IPV4_ADDR_LEN];
        int     int_v;
        NETCFG_TYPE_IpAddressInfoEntry_T ip_addr_info_v;
        NETCFG_TYPE_PMTU_Entry_T pmtu_entry_v;
#if (SYS_CPNT_IPV6 == TRUE)
        IPAL_Ipv6Statistics_T       ip6_stat;
        IPAL_Icmpv6Statistics_T     icmp6_stat;
        IPAL_Udpv6Statistics_T      udp6_stat;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
#if (SYS_CPNT_CRAFT_PORT == TRUE)
        NETCFG_TYPE_CraftInetAddress_T  craft_addr;
#endif

        struct
        {
            UI32_T u32_a1;
            NETCFG_TYPE_IP_ADDRESS_MODE_T ip_addr_mode;
        } u32a1_ip_addr_mode;

        struct
        {
            NETCFG_TYPE_InetRifConfig_T inet_rif_config;
            UI32_T u32_a1;
        } inet_rif_config_u32a1;

        struct
        {
            UI32_T ifindex;
            UI8_T logical_mac[SYS_ADPT_MAC_ADDR_LEN];
        } ifindex_logical_mac;

        struct
        {
            UI32_T ifindex;
            UI16_T flags;
        } ifindex_flags;

        struct
        {
            UI32_T ifindex;
            BOOL_T status;
        }arp_proxy_status;/*Lin.Li, for ARP porting*/

        struct
        {
            UI32_T a1;
            UI32_T a2;
        }ui32_a1_a2;

        struct
        {
            UI32_T u32_a1;
            BOOL_T bl_a2;
        } u32a1_bla2;

        struct
        {
            UI32_T u32_a1;
            L_INET_AddrIp_T addr_a2;
        } u32a1_addra2;
#if (SYS_CPNT_CLUSTER == TRUE)
        struct
        {
            UI32_T rowStatus;
            UI32_T incoming_type;
            UI8_T  ipAddress[SYS_ADPT_IPV4_ADDR_LEN];
            UI8_T  ipMask[SYS_ADPT_IPV4_ADDR_LEN];
        } cluster_vlan_rif;
#endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
        struct
        {
            UI32_T src_vid_ifindex;
            NETCFG_TYPE_L3_Interface_T l3_intf;
        } tunnel_l3_intf;

        struct
        {
            UI32_T tunnel_id;
            UI16_T rowstatus;
        } tunnel_rowstatus;
#endif
    } data; /* contains the supplemntal data for the corresponding cmd */
} NETCFG_MGR_IP_IPCMsg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
void NETCFG_MGR_IP_SetTransitionMode(void);


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
void NETCFG_MGR_IP_EnterTransitionMode (void);


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
void NETCFG_MGR_IP_EnterMasterMode (void);


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
void NETCFG_MGR_IP_EnterSlaveMode (void);

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
void NETCFG_MGR_IP_ProvisionComplete(void);

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
                                             BOOL_T use_default);
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
void NETCFG_MGR_IP_Create_InterCSC_Relation(void);


/*---------------------------------------
 *  Initialization
 *---------------------------------------
 */
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
BOOL_T NETCFG_MGR_IP_InitiateProcessResources(void);


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
void NETCFG_MGR_IP_L3InterfaceDestory_CallBack(UI32_T ifindex);


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
void NETCFG_MGR_IP_L3IfOperStatusChanged_CallBack(UI32_T ifindex, UI32_T oper_status);


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
BOOL_T NETCFG_MGR_IP_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);
/*Lin.Li, for ARP porting, modify start*/
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
UI32_T NETCFG_MGR_IP_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status);

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
NETCFG_MGR_IP_GetRunningIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T *status);
/*Lin.Li, for ARP porting, modify end*/

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
                                        UI32_T incoming_type);

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
void NETCFG_MGR_IP_IpalRifReflection_CallBack(NETCFG_TYPE_InetRifConfig_T *rif_p);

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
UI32_T NETCFG_MGR_IP_IPv6Enable(UI32_T ifindex);

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
UI32_T NETCFG_MGR_IP_IPv6Disable(UI32_T ifindex);

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
UI32_T NETCFG_MGR_IP_IPv6AddrAutoconfigEnable(UI32_T ifindex);

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
UI32_T NETCFG_MGR_IP_IPv6AddrAutoconfigDisable(UI32_T ifindex);

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
 *      NETCFG_TYPE_OK -- keep this information in IPCFG_OM.
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED -- interface (ifindex do not exist)
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T  NETCFG_MGR_IP_SetIPv6InterfaceMTU(UI32_T ifindex, UI32_T mtu);

UI32_T  NETCFG_MGR_IP_UnsetIPv6InterfaceMTU(UI32_T ifindex);

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
UI32_T NETCFG_MGR_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p);

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
UI32_T  NETCFG_MGR_IP_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p);

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
UI32_T NETCFG_MGR_IP_GetNextPMTUEntry(NETCFG_TYPE_PMTU_Entry_T *entry_p);

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
UI32_T NETCFG_MGR_IP_GetIfIpv6AddrInfo(NETCFG_TYPE_IpAddressInfoEntry_T *addr_info_p);

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
UI32_T NETCFG_MGR_IP_DhcpReleaseComplete(UI32_T ifindex, UI32_T protocols);

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
NETCFG_MGR_IP_GetRunningIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p);

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
NETCFG_MGR_IP_GetRunningIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p);

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
NETCFG_MGR_IP_GetRunningIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p);

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
UI32_T NETCFG_MGR_IP_GetIPv6Statistics(IPAL_Ipv6Statistics_T *ip6stat_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetICMPv6Statistics
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
UI32_T NETCFG_MGR_IP_GetICMPv6Statistics(IPAL_Icmpv6Statistics_T *icmp6stat_p);

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
UI32_T NETCFG_MGR_IP_GetUDPv6Statistics(IPAL_Udpv6Statistics_T *udp6stat_p);

/* FUNCTION NAME : NETCFG_MGR_IP_ClearIPv6StatisticsByType
 * PURPOSE:
 *      Clear IPv6 statistics by specified type.
 *
 * INPUT:
 *      clear_type -- which type to clear.
 *                    refer to NETCFG_MGR_IPV6_STAT_TYPE_E
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
UI32_T NETCFG_MGR_IP_ClearIPv6StatisticsByType(UI32_T clear_type);

#endif /* SYS_CPNT_IPV6 */
#if (SYS_CPNT_IP_TUNNEL == TRUE)
/* FUNCTION NAME : NETCFG_MGR_IP_CreateTunnelInterface
 * PURPOSE:
 *      Create a new tunnel interface
 *
 * INPUT:
 *      tunnel_id -- the tunnel id.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_IP_CreateTunnelInterface(UI32_T tunnel_id);
/* FUNCTION NAME : NETCFG_MGR_IP_DeleteTunnelInterface
 * PURPOSE:
 *      delete existing tunnel interface
 *
 * INPUT:
 *      tunnel_id -- the tunnel id.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_EXIST
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_IP_DeleteTunnelInterface(UI32_T tunnel_id);


/* FUNCTION NAME : NETCFG_MGR_IP_SetTunnelIPv6Rif
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
UI32_T NETCFG_MGR_IP_SetTunnelIPv6Rif(NETCFG_TYPE_InetRifConfig_T *rif_p,UI32_T incoming_type);
/*
* FUNCTION NAME : NETCFG_MGR_IP_SetTunnelSourceVLAN
 * PURPOSE:
 *      set tunnel source vlan
 * INPUT:
 *      tunnel_ifindex: tunnel to be set
 *      source_vid: the source vid (NOT vid ifindex)
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_TUNNEL_SOURCE_VLAN_HAS_NO_IPV4_ADDRESS
 *
 * NOTES:
 *
 */
UI32_T NETCFG_MGR_IP_SetTunnelSourceVLAN(UI32_T tunnel_ifindex, UI32_T source_vid);
UI32_T NETCFG_MGR_IP_UnsetTunnelSourceVLAN(UI32_T tunnel_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_IP_GetRunningTunnelSourceVlan
 * ------------------------------------------------------------------------
 * PURPOSE  :  Get tunnel rif configuration
 * INPUT    :   ifindex : tunnel ifindex
 * OUTPUT   :   *src_vid_ifindex  -- tunnel source vid ifindex
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :  none
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_IP_GetRunningTunnelSourceVlan(UI32_T ifindex, UI32_T *src_vid_ifindex);

/*
* FUNCTION NAME : NETCFG_MGR_IP_SetTunnelDestinationAddress
 * PURPOSE:
 *      set tunnel destination address
 * INPUT:
 *      tunnel_ifindex: tunnel to be set
 *      dest: the tunnel destination IPv4 address
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *     when dynamic tunnel (ISATAP, 6to4), this command have to effect
 */
UI32_T NETCFG_MGR_IP_SetTunnelDestinationAddress(UI32_T tunnel_ifindex,L_INET_AddrIp_T* dest);

/* FUNCTION NAME : NETCFG_MGR_IP_UnetTunnelDestinationAddress
 * PURPOSE:
 *      unset tunnel destination address
 * INPUT:
 *      tunnel_ifindex: tunnel to be set
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *     when dynamic tunnel (ISATAP, 6to4), this command have to effect
 */
UI32_T NETCFG_MGR_IP_UnetTunnelDestinationAddress(UI32_T tunnel_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_IP_GetRunningTunnelDestination
 * ------------------------------------------------------------------------
 * PURPOSE  :  get tunnel destionation
 * INPUT    :   ifindex : tunnel ifindex
 * OUTPUT   :   *dip  -- tunnel destination address
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :  none
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_IP_GetRunningTunnelDestination(UI32_T ifindex, L_INET_AddrIp_T *dip);

/*
* FUNCTION NAME : NETCFG_MGR_IP_SetTunnelMode
 * PURPOSE:
 *      set tunnel mode
 * INPUT:
 *      tunnel_ifindex: tunnel to be set
 *      tunnel mode : configured(1), 6to4(2), isatap (3)
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED
 *
 * NOTES:
 *     when dynamic tunnel (ISATAP, 6to4), this command have to effect
 */
UI32_T NETCFG_MGR_IP_SetTunnelMode(UI32_T tunnel_ifindex, /*NETCFG_TYPE_TUNNEL_MODE_T*/UI32_T  tunnel_mode);
/*
* FUNCTION NAME : NETCFG_MGR_IP_UnsetTunnelMode
 * PURPOSE:
 *      restore tunnel mode to default mode
 * INPUT:
 *      tunnel_ifindex: tunnel to be unset
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED
 *
 * NOTES:
 *
 */

UI32_T NETCFG_MGR_IP_UnsetTunnelMode(UI32_T tunnel_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_IP_GetRunningTunnelMode
 * ------------------------------------------------------------------------
 * PURPOSE  :  get tunnel destionation
 * INPUT    :   ifindex : tunnel ifindex
 * OUTPUT   :   *tunnel_mode  -- tunnel mode
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :  none
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_IP_GetRunningTunnelMode(UI32_T ifindex, UI32_T *tunnel_mode);

/*
* FUNCTION NAME : NETCFG_MGR_IP_SetTunnelTtl
 * PURPOSE:
 *      set tunnel ttl
 * INPUT:
 *      tunnel_ifindex: tunnel to be unset
 *      tunnel_ttl  : 0~255
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED
 *
 * NOTES:
 *
 */
UI32_T NETCFG_MGR_IP_SetTunnelTtl(UI32_T tunnel_ifindex,  UI32_T  tunnel_ttl);



/* FUNCTION NAME : NETCFG_MGR_IP_UnsetTunnelTtl
 * PURPOSE:
 *      set tunnel ttl to default
 * INPUT:
 *      tunnel_ifindex: tunnel to be unset
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NOT_MASTER_MODE
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INTERFACE_NOT_EXISTED
 *
 * NOTES:
 *
 */
UI32_T NETCFG_MGR_IP_UnsetTunnelTtl(UI32_T tunnel_ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_IP_GetRunningTunnelTtl
 * ------------------------------------------------------------------------
 * PURPOSE  :  get tunnel ttl
 * INPUT    :   ifindex : tunnel ifindex
 * OUTPUT   :   *ttl  -- tunnel ttl
  * RETURN   :  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :  none
 * ------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_MGR_IP_GetRunningTunnelTtl(UI32_T ifindex, UI32_T *ttl);


/* FUNCTION NAME : NETCFG_MGR_IP_ReceiveIpv6Packet_CallBack
 * PURPOSE:
 *      receive ipv6 packet
 * INPUT:
 *      src_addr
 *      dst_addr
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void  NETCFG_MGR_IP_ReceiveIpv6Packet_CallBack(UI8_T* src_addr, UI8_T* dst_addr);

/* FUNCTION NAME: NETCFG_MGR_IP_GetTunnelIfindexBySrcVidIfindex
 * PURPOSE  : Get tunnel interface ifindex by its configuration source vlan ifindex
 * INPUT    : src_vid_ifindex -- Tunnel source vid ifindex
 *            
 * OUTPUT   : tunnel_l3_intf      -- Tunnel l3 interface
 * RETURN   : successful (NETCFG_TYPE_OK), failed (NETCFG_TYPE_FAIL)
 * NOTES    :
 *     
 */
UI32_T  NETCFG_MGR_IP_GetTunnelIfindexBySrcVidIfindex(UI32_T src_vid_ifindex, NETCFG_TYPE_L3_Interface_T *tunnel_l3_intf);


/* FUNCTION NAME: NETCFG_MGR_IP_SetTunnelRowStatus
 * PURPOSE  : set tunnel interface's row status
 * INPUT    : ifindex   -- Tunnel interface ifindex
 *            rowstatus -- action rowstatus(active/notReady/destroy)
 *            
 * OUTPUT   : none
 * RETURN   : successful (NETCFG_TYPE_OK), failed (NETCFG_TYPE_FAIL)
 * NOTES    : This api is for snmp to use.
 *     
 */
UI32_T NETCFG_MGR_IP_SetTunnelRowStatus(UI32_T ifindex,UI16_T rowstatus);

/* FUNCTION NAME: NETCFG_MGR_IP_SetTunnelInterfaceFromSnmp
 * PURPOSE  : configure tunnel interface
 * INPUT    : tunnel_if  --  tunnel interface information 
 *            
 * OUTPUT   : none
 * RETURN   : successful (NETCFG_TYPE_OK), failed (NETCFG_TYPE_FAIL)
 * NOTES    : This api is for snmp to use.
 *            we use this api to set (destination/source vlan/mode) 
 *     
 */
UI32_T NETCFG_MGR_IP_SetTunnelInterfaceFromSnmp(NETCFG_TYPE_L3_Interface_T *tunnel_if);
#endif /*SYS_CPNT_IP_TUNNEL*/



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

void NETCFG_MGR_IP_NsmRouteChange_CallBack(UI32_T address_family);

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
UI32_T NETCFG_MGR_IP_ClusterVlanSetRifRowStatus(UI8_T *ipAddress, UI8_T *ipMask, UI32_T rowStatus, UI32_T incoming_type);

#endif /* SYS_CPNT_CLUSTER */

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME : NETCFG_MGR_IP_SetCraftInterfaceInetAddress
 * PURPOSE:
 *      To add or delete ipv4/v6 address on craft interface
 * INPUT:
 *      rif_p               -- pointer to rif
 *      om_only             -- only set om, without setting to kernel
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_MGR_IP_SetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p, BOOL_T om_only);

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
UI32_T NETCFG_MGR_IP_IPv6Enable_Craft(UI32_T ifindex, BOOL_T do_enable);
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
UI32_T NETCFG_MGR_IP_SetDhcpInform(UI32_T ifindex, BOOL_T do_enable);
#endif /* SYS_CPNT_DHCP_INFORM*/

#endif /* NETCFG_MGR_IP_H */

