/* Module Name: NETCFG_OM_IP.H
 * Purpose: Keep user configured IP interface entry.
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       -- 	Modifier,  	Reason
 *      01/22/2008  --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

#ifndef NETCFG_OM_IP_H
#define NETCFG_OM_IP_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "l_inet.h"
#include "netcfg_type.h"
#include "ipal_types.h"

/* NAME CONSTANT DECLARATIONS
 */
#define NETCFG_OM_IP_MSGBUF_TYPE_SIZE sizeof(union NETCFG_OM_IP_IPCMsg_Type_U)

/* definitions of command in CSCA which will be used in POM operation
 */
enum
{
/* System Wise */

/* Ip Configuration   */
    NETCFG_OM_IP_IPCCMD_GETL3INTERFACE,
    NETCFG_OM_IP_IPCCMD_GETNEXTL3INTERFACE,
    NETCFG_OM_IP_IPCCMD_GETIPADDRESSMODE,
    NETCFG_OM_IP_IPCCMD_GETNEXTIPADDRESSMODE,
    NETCFG_OM_IP_IPCCMD_GETRIFCONFIG,
    NETCFG_OM_IP_IPCCMD_GETNEXTRIFCONFIG,
    NETCFG_OM_IP_IPCCMD_GETRIFFROMSUBNET,
    NETCFG_OM_IP_IPCCMD_GETRIFFROMIP,
    NETCFG_OM_IP_IPCCMD_GETRIFFROMEXACTIP,
    NETCFG_OM_IP_IPCCMD_GETRIFFROMINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETPRIMARYRIFFROMINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETNEXTSECONDARYRIFFROMINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETNEXTRIFFROMINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETNEXTINETRIFOFINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETIPADDRENTRY,
    NETCFG_OM_IP_IPCCMD_GETNEXTIPADDRENTRY,
    NETCFG_OM_IP_IPCCMD_GETRUNNINGIPADDRESSMODE,
    NETCFG_OM_IP_IPCCMD_GETARPPROXYSTATUS,
    NETCFG_OM_IP_IPCCMD_GETNEXTARPPROXYSTATUS,
#if (SYS_CPNT_IPV6 == TRUE)
    NETCFG_OM_IP_IPCCMD_GETIPV6RIFCONFIG,
    NETCFG_OM_IP_IPCCMD_GETNEXTIPV6RIFCONFIG,
    NETCFG_OM_IP_IPCCMD_GETINETRIFCONFIG,
    NETCFG_OM_IP_IPCCMD_GETNEXTINETRIFCONFIG,
    NETCFG_OM_IP_IPCCMD_GETIPV6RIFFROMINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETNEXTIPV6RIFFROMINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETIPV6ENABLESTATUS,
    NETCFG_OM_IP_IPCCMD_GETIPV6ADDRAUTOCONFIGENABLESTATUS,
    NETCFG_OM_IP_IPCCMD_GETNEXTIPV6ADDRAUTOCONFIGENABLESTATUS,
    NETCFG_OM_IP_IPCCMD_GETLINKLOCALRIFFROMINTERFACE,
    NETCFG_OM_IP_IPCCMD_GETIPV6INTERFACEMTU,
#endif /* SYS_CPNT_IPV6 */
#if (SYS_CPNT_CRAFT_PORT == TRUE)
    NETCFG_OM_IP_IPCCMD_GETCRAFTINTERFACEINETADDRESS,
    NETCFG_OM_IP_IPCCMD_GETIPV6ENABLESTATUS_CRAFT,
#endif /* SYS_CPNT_CRAFT_PORT */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    NETCFG_OM_IP_IPCCMD_GET_DHCP_INFORM,
    NETCFG_OM_IP_IPCCMD_GET_RUNNING_DHCP_INFORM,
#endif /* SYS_CPNT_DHCP_INFORM */
#if (SYS_CPNT_VIRTUAL_IP == TRUE)
    NETCFG_OM_IP_IPCCMD_GETNEXTVIRTUALRIFBYIFINDEX,
#endif


};

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in CSCA_OM_IPCMsg_T.data
 */
#define NETCFG_OM_IP_GET_MSG_SIZE(field_name)                       \
            (NETCFG_OM_IP_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_OM_IP_IPCMsg_T*)0)->data.field_name))

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))

#define NETCFG_OM_IP_CONFIG_CHECK(flag)

/* DATA TYPE DECLARATIONS
 */
/* structure for the request/response ipc message in csca pom and om
 */
typedef struct
{
    union NETCFG_OM_IP_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. CSCA_OM_IPC_CMD1 ... */
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
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

        NETCFG_TYPE_InetRifConfig_T inet_rif_config_v;
        NETCFG_TYPE_ipAddrEntry_T   ip_addr_entry_v;
        NETCFG_TYPE_L3_Interface_T l3_interface_v;

#if (SYS_CPNT_CRAFT_PORT == TRUE)
        NETCFG_TYPE_CraftInetAddress_T  craft_addr;
#endif /* SYS_CPNT_CRAFT_PORT */

        struct
        {
            UI32_T u32_a1;
            UI32_T u32_a2;
        } u32a1_u32a2;

        struct
        {
            UI32_T u32_a1;
            BOOL_T bl_a2;
        } u32a1_bla2;

    } data; /* contains the supplemntal data for the corresponding cmd */
} NETCFG_OM_IP_IPCMsg_T;

typedef struct NETCFG_OM_IP_InetRifConfig_S
{
    /* Key */
    L_INET_AddrIp_T addr;

    UI32_T ifindex;

    /* Type of Rif:
     * VAL_netConfigPrimaryInterface_primary
     * VAL_netConfigPrimaryInterface_secondary
     * NETCFG_TYPE_MODE_VIRTUAL
     */
    UI32_T ipv4_role;

    struct NETCFG_OM_IP_InetRifConfig_S *next;
#if (SYS_CPNT_IPV6 == TRUE)
    UI32_T ipv6_addr_type;
    UI32_T ipv6_addr_config_type;
#endif
    UI32_T flags;
    UI32_T row_status;
} NETCFG_OM_IP_InetRifConfig_T;

/* Key: ifindex
 */
typedef struct
{
    /* Lookup Key */
    UI32_T ifindex;

    /* interface type: physical /tunnel */
    UI8_T  iftype;

    /* Routing Interface Information list
     * This rif list arrange the NETCFG_OM_IP_InetRifConfig_T
     * by 1. prefix type; 2. prefixlen; 3. ip address
     */
    NETCFG_OM_IP_InetRifConfig_T *inet_rif_p;  // peter renamed.

    UI32_T  drv_l3_intf_index;  /* to save driver layer l3_intf_index here for distinguish physical/tunner int in driver layer */

#if (SYS_CPNT_IPV6 == TRUE)
    BOOL_T  ipv6_enable;
    BOOL_T  ipv6_autoconf_enable;
#endif
    UI16_T config;
#define NETCFG_OM_IP_CONFIG_MTU		(1 << 0)
#define NETCFG_OM_IP_CONFIG_MTU6	(1 << 1)

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    BOOL_T  dhcp_inform;    /* to enable dhcp inform */
#endif
    /* union */
    struct
    {
        struct
        {
    /* The interface flags, defined in TCP/IP stack,
     * IFF_UP, IFF_RUNNING
     */
    UI16_T if_flags;

    /* Vlan MAC
     */
    UI8_T hw_addr[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T logical_mac[SYS_ADPT_MAC_ADDR_LEN];
    /* For SNMP use, TRUE specifies a logical mac has set
     */
    BOOL_T logical_mac_set;

    /* Vlan MTU
     */
    UI32_T mtu;

#if (SYS_CPNT_IPV6 == TRUE)
    /* IPv6 MTU
     */
    UI32_T mtu6;
    BOOL_T accept_ra_pinfo; /* ipv6 accept ra prefix info */
#endif
    /* Vlan Bandwidth
     */
    UI32_T bandwidth;

#if (SYS_CPNT_PROXY_ARP == TRUE)
    /* Proxy ARP status
     */
    BOOL_T proxy_arp_enable;
#endif

    UI32_T ipv4_address_mode;


        } physical_intf;
#if (SYS_CPNT_IP_TUNNEL == TRUE)

        NETCFG_TYPE_Tunnel_Interface_T  tunnel_intf;
#endif
    } u;

}NETCFG_OM_L3_Interface_T;

#if (SYS_CPNT_CRAFT_PORT == TRUE)
typedef struct {
    NETCFG_TYPE_CraftInetAddress_T craft_if_ipv4_address;
    BOOL_T craft_if_ipv6_enable;
    NETCFG_TYPE_CraftInetAddress_T craft_if_ipv6_address_link; /* link-local */
    NETCFG_TYPE_CraftInetAddress_T craft_if_ipv6_address_global;
} NETCFG_OM_Craft_Interface_T;

#define NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV4_ADDR         1 << 0
#define NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ENABLE       1 << 1
#define NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_LINK    1 << 2
#define NETCFG_OM_CRAFT_INTERFACE_FIELD_IPV6_ADDR_GLOBAL  1 << 3
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_OM_IP_GetShMemInfo
 * PURPOSE:
 *          Get shared memory space information.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      segid_p
 *      seglen_p
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_OM_IP_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);


/* FUNCTION NAME : NETCFG_OM_IP_InitateSystemResources
 * PURPOSE:
 *          Initialize system resources.
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
void NETCFG_OM_IP_InitateSystemResources(void);

/* FUNCTION NAME : NETCFG_OM_IP_AttachSystemResources
 * PURPOSE:
 *          Attach system resources.
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
void NETCFG_OM_IP_AttachSystemResources(void);


/* FUNCTION NAME : NETCFG_OM_IP_InitateProcessResources
 * PURPOSE:
 *          Initialize semophore & create a list to store ip interface entry.
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
void NETCFG_OM_IP_InitateProcessResources(void);


/* FUNCTION NAME : NETCFG_OM_IP_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_OM_IP.
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
BOOL_T NETCFG_OM_IP_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/* FUNCTION NAME : NETCFG_OM_IP_IsL3InterfaceTableFull
 * PURPOSE:
 *          Check if L3 Interface table full in advance.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T NETCFG_OM_IP_IsL3InterfaceTableFull(void);

/* FUNCTION NAME : NETCFG_OM_IP_CreateL3Interface
 * PURPOSE:
 *          Store the new created L3 interface entry.
 *
 * INPUT:
 *      intf  -- the interface entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_CreateL3Interface(NETCFG_TYPE_L3_Interface_T *intf);


/* FUNCTION NAME : NETCFG_OM_IP_DeleteL3Interface
 * PURPOSE:
 *          Delete a L3 interface entry.
 *
 * INPUT:
 *      intf  -- the interface entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_DeleteL3Interface(NETCFG_TYPE_L3_Interface_T *intf);

/* FUNCTION NAME : NETCFG_OM_IP_GetL3Interface
 * PURPOSE:
 *          Get an interface entry by given ifindex.
 *
 * INPUT:
 *      intf->ifindex
 *
 * OUTPUT:
 *      intf
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetL3Interface(NETCFG_TYPE_L3_Interface_T *intf);

/* FUNCTION NAME : NETCFG_OM_IP_GetNextL3Interface
 * PURPOSE:
 *          Get Next L3 interface entry
 *
 * INPUT:
 *      intf->ifindex
 *
 * OUTPUT:
 *      intf
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *      For CLI show ip interface.
 */
UI32_T NETCFG_OM_IP_GetNextL3Interface(NETCFG_TYPE_L3_Interface_T *intf);

/* FUNCTION NAME : NETCFG_OM_IP_DeleteAllInterface
 * PURPOSE:
 *          Delete all interface entry from database.
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
void NETCFG_OM_IP_DeleteAllInterface(void);

/* FUNCTION NAME: NETCFG_OM_IP_SetInterfaceAcceptRaPrefixInfo
 * PURPOSE:
 *      Set/unset interface's accept_ra_pinfo.
 *
 * INPUT:
 *      ifindex     -- the interface ifindex
 *      status      -- enable/disable
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
UI32_T NETCFG_OM_IP_SetInterfaceAcceptRaPrefixInfo(UI32_T ifindex, BOOL_T status);

/* FUNCTION NAME : NETCFG_OM_IP_LookupPrimaryRif
 * PURPOSE:
 *      Get the primary rif of given interface.
 *
 * INPUT:
 *      primary_rif->addr
 *      ifindex
 *
 * OUTPUT:
 *      primary_rif
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_LookupPrimaryRif(NETCFG_OM_IP_InetRifConfig_T *primary_rif, UI32_T ifindex);


/* FUNCTION NAME : NETCFG_OM_IP_GetInterfaceRifCount
 * PURPOSE:
 *          Get the total RIF number of an interface by given ifindex.
 *
 * INPUT:
 *      ifindex
 *
 * OUTPUT:
 *      count    -- the total number of RIF entries
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_NO_SUCH_INTERFACE
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetInterfaceRifCount(UI32_T ifindex, int *count);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv4RifConfig
 * PURPOSE:
 *      Get the rif by the ip address.
 *
 * INPUT:
 *      rif_config_p        -- pointer to the rif_config
 *
 * OUTPUT:
 *      rif_config_p        -- pointer to the result rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 */
UI32_T NETCFG_OM_IP_GetIPv4RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetNextIPv4RifConfig
 * PURPOSE:
 *      Get the Next rif by the ip address.
 *
 * INPUT:
 *      rif_entry_p->ip_addr    -- ip address (key)
 *
 * OUTPUT:
 *      rif_config_p        -- pointer to the result rif_config
 *
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST;
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      The KEY is only ip address.
 */
UI32_T NETCFG_OM_IP_GetNextIPv4RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetInetRif
 * PURPOSE:
 *      Get an routing interface entry.
 *
 * INPUT:
 *      rif->addr    -- must provide
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      1.Giving rif-addr must include prefixlen.
 *      2.If all you need is checking if my rif own this ip, use NETCFG_OM_IP_GetRifFromExactIp
 */
UI32_T NETCFG_OM_IP_GetInetRif(NETCFG_OM_IP_InetRifConfig_T *rif);

#if 0
/* FUNCTION NAME : NETCFG_OM_IP_GetNextInetRif
 * PURPOSE:
 *      Get Next routing interface entry sorting by only the ip address.
 *
 * INPUT:
 *      rif->addr
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetNextInetRif(NETCFG_OM_IP_InetRifConfig_T *rif);

/* FUNCTION NAME : NETCFG_OM_IP_GetInetRifByIfindex
 * PURPOSE:
 *          Get a routing interface entry of a given interface.
 *
 * INPUT:
 *      ifindex
 *      rif->addr
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      1. The input ifindex must not be 0
 *      2. The sorting & searching KEYs are:
 *          (1) prefix type (IPv4 or IPv6)
 *          (2) prefix length
 *          (3) prefix address
 */
UI32_T NETCFG_OM_IP_GetInetRifByIfindex(NETCFG_OM_IP_InetRifConfig_T *rif, UI32_T ifindex);
#endif

/* FUNCTION NAME : NETCFG_OM_IP_GetNextInetRifByIfindex
 * PURPOSE:
 *          Get next routing interface entry of a given interface.
 *
 * INPUT:
 *      ifindex
 *      rif->addr
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      The input ifindex must not be 0 and this function will not go
 *      next interface.
 */
UI32_T NETCFG_OM_IP_GetNextInetRifByIfindex(NETCFG_OM_IP_InetRifConfig_T *rif, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_IP_GetRifFromSubnet
 * PURPOSE:
 *      Get the rif whose subnet covers the target IP address.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetRifFromSubnet(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetRifFromIp
 * PURPOSE:
 *      Get the rif whose subnet covers the target IP address.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. For both IPv4/IPv6.
 */
UI32_T NETCFG_OM_IP_GetRifFromIp(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetRifFromIpAndIfindex
 * PURPOSE:
 *      Get the rif whose subnet covers the target IP address with specified ifindex.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *      rif_config_p->ifindex   -- the target ifindex
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. For both IPv4/IPv6.
 *      2. For vrf use case.
 */
UI32_T NETCFG_OM_IP_GetRifFromIpAndIfindex(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetRifFromExactIp
 * PURPOSE:
 *      Find the interface which this ip on.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. For both IPv4/IPv6.
 */
UI32_T  NETCFG_OM_IP_GetRifFromExactIp (NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetRifFromExactIpAndIfindex
 * PURPOSE:
 *      Find the interface which this ip on.
 *
 * INPUT:
 *      rif_config_p->ip_addr   -- the target IP address for checking.
 *      rif_config_p->ifindex   -- the target ifindex for checking.
 *      chk_ifidx               -- TRUE to check ifindex
 * OUTPUT:
 *      rif_config_p            -- pointer to rif
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. For both IPv4/IPv6.
 *      2. Also search in the ipv4 rif of craft interface.
 *      3. For vrf use case.
 */
UI32_T NETCFG_OM_IP_GetRifFromExactIpAndIfindex (
    NETCFG_TYPE_InetRifConfig_T *rif_config_p,
    BOOL_T                      chk_ifidx);

/* FUNCTION NAME : NETCFG_OM_IP_SetInetRif4Rif
 * PURPOSE:
 *      Add or update rif for the specified L3 interface(by ifindex)
 *
 * INPUT:
 *      rif_config_p->ifindex               -- specified L3 interface.
 *      rif_config_p->primary_interface     -- role.
 *      rif_config_p->prefix               -- ip address & mask of rif.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_SetInetRif(NETCFG_OM_IP_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_CheckAddressOverlap
 * PURPOSE:
 *      Check the subnet of rif is overlapping with another one
 *
 * INPUT:
 *      rif->addr -- be checked.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NETCFG_TYPE_OK -- no overlap
 *		NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED
 *      NETCFG_TYPE_INVALID_ARG
 *
 * NOTES:
 *      The overlapping check applied to:
 *      1. primary(input) to secondary(exist);
 *      2. secondary(input) to secondary(exist);
 *      3. seconary(input) to primary(exist);
 */
UI32_T NETCFG_OM_IP_CheckAddressOverlap(NETCFG_OM_L3_Interface_T *intf, NETCFG_OM_IP_InetRifConfig_T *rif);


#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : NETCFG_OM_IP_GetIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_OM_IP_GetIpNetToMediaProxyStatus(UI32_T ifindex,BOOL_T *status);

/* FUNCTION NAME : NETCFG_OM_IP_GetNextIpNetToMediaProxyStatus
 * PURPOSE:
 *      Get next interface's proxy ARP status.
 *
 * INPUT:
 *      ifindex -- the interface.
 *
 * OUTPUT:
 *      status -- one of {TRUE(stand for enable) |FALSE(stand for disable)}
 *
 * RETURN:
 *     NETCFG_TYPE_OK/
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_OM_IP_GetNextIpNetToMediaProxyStatus(UI32_T *ifindex,BOOL_T *status);
#endif

/* FUNCTION NAME: NETCFG_OM_IP_GetNextRifOfInterface
 * PURPOSE:
 *      Get the next rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex   -- the beginning interface trying to search (key).
 *      rif_config_p->ip_addr   -- ip address (key).
 *
 * OUTPUT:
 *      rif_config_p            -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. This function WILL NOT iterate to the next L3 interface.
 *      2. For IPv4/IPv6.
 */
UI32_T NETCFG_OM_IP_GetNextInetRifOfInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME: NETCFG_OM_IP_GetRifFromInterface
 * PURPOSE:
 *      Get the IPv4 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->ip_addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv4 only.
 */
UI32_T NETCFG_OM_IP_GetRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME: NETCFG_OM_IP_GetNextRifFromInterface
 * PURPOSE:
 *      Get the next IPv4 rif from the L3 interface by ifindex.
 * INPUT:
 *      rif_config_p->ifindex   -- the beginning interface trying to search (key).
 *      rif_config_p->ip_addr   -- ip address (key).
 *      rif_config_p->mask      -- mask (key).
 *
 * OUTPUT:
 *      rif_config_p            -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. This function WILL iterate to the next L3 interface.
 *      2. CLI "show ip interface" and SNMP GetNext should call this function.
 *      3. For IPv4 only
 */
UI32_T NETCFG_OM_IP_GetNextRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

UI32_T NETCFG_OM_IP_SetDebugFlag(UI8_T is_enable);

/* FUNCTION NAME : NETCFG_OM_IP_SetRifFlags
 * PURPOSE:
 *          Set flags of an rif entry.
 *
 * INPUT:
 *      rif   -- the rif entry.
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_SetRifFlags(NETCFG_OM_IP_InetRifConfig_T *rif_p, UI32_T flags);
UI32_T NETCFG_OM_IP_UnSetRifFlags(NETCFG_OM_IP_InetRifConfig_T *rif_p, UI32_T flags);

UI32_T NETCFG_OM_IP_GetRifFlags(NETCFG_OM_IP_InetRifConfig_T *rif_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetSystemRifNbr
 * PURPOSE:
 *      Get current system IPv4/IPv6 rif number
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      rif_num   -- current system rif number
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *
 */
UI32_T NETCFG_OM_IP_GetSystemRifNbr(UI32_T *rif_num);
#if (SYS_CPNT_IPV6 == TRUE)

/* FUNCTION NAME: NETCFG_OM_IP_GetIPv6RifFromInterface
 * PURPOSE:
 *      Get the IPv6 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->ip_addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv6 only.
 */
UI32_T NETCFG_OM_IP_GetIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME: NETCFG_OM_IP_GetNextIPv6RifFromInterface
 * PURPOSE:
 *      Get the IPv6 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->ip_addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv6 only.
 */
UI32_T NETCFG_OM_IP_GetNextIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6RifConfig
 * PURPOSE:
 *      Get an routing interface entry.
 *
 * INPUT:
 *      rif->addr    -- must provide
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetNextIPv6RifConfig
 * PURPOSE:
 *      Get Next routing interface entry sorting by only the ip address.
 *
 * INPUT:
 *      rif->addr
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetNextIPv6RifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetInetRif
 * PURPOSE:
 *      Get an routing interface entry.
 *
 * INPUT:
 *      rif->addr    -- must provide
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetInetRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetNextInetRifConfig
 * PURPOSE:
 *      Get Next routing interface entry sorting by only the ip address.
 *
 * INPUT:
 *      rif->addr
 *
 * OUTPUT:
 *      rif
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_OK
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetNextInetRifConfig(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME: NETCFG_OM_IP_GetIPv6RifFromInterface
 * PURPOSE:
 *      Get the IPv6 rif from the L3 interface by ifindex, and ip_addr.
 * INPUT:
 *      rif_config_p->ifindex           -- ifindex of interface (must).
 *      rif_config_p->ip_addr           -- ip address (optional).
 *
 * OUTPUT:
 *      rif_config_p                    -- pointer to the rif_config.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 * NOTES:
 *      1. The search key priority is ifindex > ip_addr.
 *      2. If interface entry doesn't exist, return fail.
 *      3. If the ip address is specified, try to find it.
 *      4. For IPv6 only.
 */
UI32_T NETCFG_OM_IP_GetIPv6RifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_config_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6EnableStatus
 * PURPOSE:
 *      Get "ipv6 enable" configuration.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      1. There is no ipv6 enable configuration in linux kernel, thus we only get it in OM.
 */
UI32_T NETCFG_OM_IP_GetIPv6EnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get IPv6 address autoconfig status.
 *
 * INPUT:
 *      ifindex     -- interface index
 *
 * OUTPUT:
 *      status_p    -- pointer to status
 *                     TRUE: autoconfig enabled.
 *                     FALSE: autoconfig disabled.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetIPv6AddrAutoconfigEnableStatus(UI32_T ifindex, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetNextIPv6AddrAutoconfigEnableStatus
 * PURPOSE:
 *      Get next IPv6 address autoconfig status.
 *
 * INPUT:
 *      ifindex_p   -- interface index
 *
 * OUTPUT:
 *      status_p    -- pointer to status
 *                     TRUE: autoconfig enabled.
 *                     FALSE: autoconfig disabled.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetNextIPv6AddrAutoconfigEnableStatus(UI32_T *ifindex_p, BOOL_T *status_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6UnicastRouting
 * PURPOSE:
 *      Get global IPv6 unicast routing.
 *
 * INPUT:
 *      status_p    -- the pointer to the status. Enable(TRUE)/Disable(FALSE)
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
UI32_T NETCFG_OM_IP_GetIPv6UnicastRouting(BOOL_T *status_p);

UI32_T NETCFG_OM_IP_GetLinkLocalRifFromInterface(NETCFG_TYPE_InetRifConfig_T *rif_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6InterfaceMTU
 * PURPOSE:
 *      get IPv6 interface MTU from OM.
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
UI32_T NETCFG_OM_IP_GetIPv6InterfaceMTU(UI32_T ifindex, UI32_T *mtu_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6StatBaseCntr
 * PURPOSE:
 *      Get IPv6 statistics base counters from OM.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      ip6stat_base_p -- IPv6 statistics base counters.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetIPv6StatBaseCntr(IPAL_Ipv6Statistics_T *ip6stat_base_p);

/* FUNCTION NAME : NETCFG_OM_IP_SetIPv6StatBaseCntr
 * PURPOSE:
 *      Set IPv6 statistics base counters to OM.
 *
 * INPUT:
 *      ip6stat_base_p -- IPv6 statistics base counters.
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
UI32_T NETCFG_OM_IP_SetIPv6StatBaseCntr(IPAL_Ipv6Statistics_T *ip6stat_base_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetICMPv6StatBaseCntr
 * PURPOSE:
 *      Get ICMPv6 statistics base counters from OM.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      icmp6stat_base_p -- ICMPv6 statistics base counters.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetICMPv6StatBaseCntr(IPAL_Icmpv6Statistics_T *icmp6stat_base_p);

/* FUNCTION NAME : NETCFG_OM_IP_SetICMPv6StatBaseCntr
 * PURPOSE:
 *      Set ICMPv6 statistics base counters to OM.
 *
 * INPUT:
 *      icmp6stat_base_p -- ICMPv6 statistics base counters.
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
UI32_T NETCFG_OM_IP_SetICMPv6StatBaseCntr(IPAL_Icmpv6Statistics_T *icmp6stat_base_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetUDPv6StatBaseCntr
 * PURPOSE:
 *      Get UDPv6 statistics base counters from OM.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      udp6stat_base_p -- UDPv6 statistics base counters.
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      None.
 */
UI32_T NETCFG_OM_IP_GetUDPv6StatBaseCntr(IPAL_Udpv6Statistics_T *udp6stat_base_p);

/* FUNCTION NAME : NETCFG_OM_IP_SetUDPv6StatBaseCntr
 * PURPOSE:
 *      Set UDPv6 statistics base counters to OM.
 *
 * INPUT:
 *      udp6stat_base_p -- UDPv6 statistics base counters.
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
UI32_T NETCFG_OM_IP_SetUDPv6StatBaseCntr(IPAL_Udpv6Statistics_T *udp6stat_base_p);

#endif /* SYS_CPNT_IPV6 */

/* FUNCTION NAME : NETCFG_OM_IP_CompareRif
 * PURPOSE:
 *      Compare two rifs by inet addr.
 *
 * INPUT:
 *      rec1    -- the pointer to the first entry.
 *      rec2    -- the pointer to the second entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      =0
 *      >0
 *      <0
 * NOTES:
 *      Support IPv4/IPv6.
 */
int NETCFG_OM_IP_CompareRif(void* rec1, void* rec2);

#if (SYS_CPNT_CRAFT_PORT == TRUE)
/* FUNCTION NAME : NETCFG_OM_IP_GetCraftInterfaceInetAddress
 * PURPOSE:
 *      To get ipv4/v6 address on craft interface
 * INPUT:
 *      craft_addr_p        -- pointer to address
 *
 * OUTPUT:
 *      None.
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 * NOTES:
 *      None
 */
UI32_T NETCFG_OM_IP_GetCraftInterfaceInetAddress(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetIPv6EnableStatus_Craft
 * PURPOSE:
 *      Get "ipv6 enable" configuration.
 *
 * INPUT:
 *      ifindex     -- the interface.
 *
 * OUTPUT:
 *      status_p    -- status
 *                     TRUE
 *                     FALSE
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      NONE.
 */
UI32_T NETCFG_OM_IP_GetIPv6EnableStatus_Craft(UI32_T ifindex, BOOL_T *status_p);
UI32_T NETCFG_OM_IP_SetCraftInterfaceValue(UI32_T field, UI32_T len, void *value_p);
UI32_T NETCFG_OM_IP_GetCraftInterfaceValue(UI32_T field, UI32_T len, void *value_p);
UI32_T NETCFG_OM_IP_CheckAddressOverlap_Craft(NETCFG_TYPE_CraftInetAddress_T *craft_addr_p);

#endif /* SYS_CPNT_CRAFT_PORT */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
/* FUNCTION NAME : NETCFG_OM_IP_SetDhcpInform
 * PURPOSE:
 *      Set the IPv4 addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *      do_enable   -- enable/disable dhcp inform
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This is only for IPv4.
 */
UI32_T NETCFG_OM_IP_SetDhcpInform(UI32_T ifindex, BOOL_T do_enable);

/* FUNCTION NAME : NETCFG_OM_IP_GetDhcpInform
 * PURPOSE:
 *      Get the IPv4 addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      do_enable_p -- status of dhcp inform
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 * NOTES:
 *      1. This is only for IPv4.
 */
UI32_T NETCFG_OM_IP_GetDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p);

/* FUNCTION NAME : NETCFG_OM_IP_GetRunningDhcpInform
 * PURPOSE:
 *      Get the running IPv4 addr_mode of specified L3 interface.
 *
 * INPUT:
 *      ifindex     -- the ifindex of L3 interface.
 *
 * OUTPUT:
 *      do_enable_p -- status of dhcp inform
 *
 * RETURN:
 *      SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *      SYS_TYPE_GET_RUNNING_CFG_FAIL
 *      SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTES:
 *      1. This is only for IPv4.
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_OM_IP_GetRunningDhcpInform(UI32_T ifindex, BOOL_T *do_enable_p);

#endif /* SYS_CPNT_DHCP_INFORM*/

#if (SYS_CPNT_VIRTUAL_IP == TRUE)
UI32_T NETCFG_OM_IP_GetVirtualRifByIpaddr(NETCFG_TYPE_InetRifConfig_T *rif_config_p);
UI32_T NETCFG_OM_IP_GetNextVirtualRifByIfindex(NETCFG_TYPE_InetRifConfig_T *rif_config_p);
#endif

#endif /* NETCFG_OM_IP_H */

