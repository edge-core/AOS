#ifndef _DEV_SWDRVL3_PMGR_H_
#define _DEV_SWDRVL3_PMGR_H_

#include "sys_type.h"
#include "dev_swdrvl3.h"

enum
{
    DEV_SWDRVL3_PMGR_RESP_RESULT_FAIL,
};
#define DEV_SWDRVL3_PMGR_REQ_CMD_SIZE       sizeof(((DEV_SWDRVL3_PMGR_IPCMSG_T *)0)->type)
#define DEV_SWDRVL3_PMGR_RESP_RESULT_SIZE   sizeof(((DEV_SWDRVL3_PMGR_IPCMSG_T *)0)->type)

typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    }type;
    union
    {
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }enableunicaststormprotect;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }disableunicaststormprotect;
        union
        {
            struct
            {
                UI32_T flags;
                UI32_T fib_id;
            }req;
            struct
            {
            }resp;
        }enablerouting;
        union
        {
            struct
            {
                UI32_T flags;
                UI32_T fib_id;
            }req;
            struct
            {
            }resp;
        }disablerouting;
        union
        {
            struct
            {
                void* hw_info;
                UI32_T fib_id;
                UI32_T vid;
                UI8_T vlan_mac[SYS_ADPT_MAC_ADDR_LEN];
            }req;
            struct
            {
                void* hw_info;
            }resp;
        }createl3interface;
        union
        {
            struct
            {
                UI32_T fib_id;
                UI32_T vid;
                void* hw_info;
                UI8_T vlan_mac[SYS_ADPT_MAC_ADDR_LEN];
            }req;
        }deletel3interface;
        union
        {
            struct
            {
                UI32_T vid;
                UI32_T mtu;
            }req;
            struct
            {
            }resp;
        }setl3interfacemtu;        
        union
        {
            DEV_SWDRVL3_Host_T host;
        }setinethostroute;
        union
        {
            DEV_SWDRVL3_Host_T host;
        }deleteinethostroute;
        union
        {
            struct
            {
                DEV_SWDRVL3_Route_T route;
                void *nh_hw_info;
            }req;
            struct
            {
                DEV_SWDRVL3_Route_T route;
            }resp;
        }addinetnetroute;
        union
        {
            struct
            {
                DEV_SWDRVL3_Route_T route;
                void *nh_hw_info;
            }req;
            struct
            {
                DEV_SWDRVL3_Route_T route;
            }resp;
        }deleteinetnetroute;
        union
        {
            DEV_SWDRVL3_Host_T host;
        }addmyiphostroute;
        union
        {
            DEV_SWDRVL3_Host_T host;
        }deletemyiphostroute;
        union
        {
            struct
            {
                DEV_SWDRVL3_Route_T route;
                void* nh_hw_info[SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE];
                UI32_T count;
            }req;
            struct
            {
                DEV_SWDRVL3_Route_T route;
            }resp;
        }addinetecmproute;
        union
        {
            DEV_SWDRVL3_Route_T route;
        }deleteinetecmproute;
        union
        {
            struct
            {
                DEV_SWDRVL3_Route_T route;
                void *nh_hw_info;
                BOOL_T is_first;
            }req;
            struct
            {
                DEV_SWDRVL3_Route_T route;
            }resp;
        }addinetecmprouteonepath;
        union
        {
            struct
            {
                DEV_SWDRVL3_Route_T route;
                void *nh_hw_info;
                BOOL_T is_last;
            }req;
        }deleteinetecmprouteonepath;
        union
        {
            DEV_SWDRVL3_Route_T default_route;
            UI32_T action;
        }specialdefaultroute;
        struct
        {
            DEV_SWDRVL3_Host_T host;
            UI32_T hit;
        }readandclearhostroutehitbit;
        struct
        {
            DEV_SWDRVL3_Route_T route;
            UI32_T hit;
        }readandclearnetroutehitbit;
        union
        {
            DEV_SWDRVL3_Host_T host;
        }clearhostroutehwinfo;
        union
        {
            DEV_SWDRVL3_Route_T route;
        }clearnetroutehwinfo;
        union
        {
            struct
            {
                UI32_T   mcast_ip;
                UI32_T   src_ip;
                UI32_T   src_vid;
                UI32_T   src_unit;
                UI32_T   src_port;
                UI8_T    port_member_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
                UI32_T   dst_vid;
                UI8_T    port_member_tagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            }req;
            struct
            {
            }resp;
        }setipmcastroute;
        union
        {
            struct
            {
                UI32_T   mcast_ip;
                UI32_T   src_ip;
                UI8_T    src_mac[6];
                UI32_T   src_vid;
                UI32_T   src_unit;
                UI32_T   src_port;
                UI32_T   src_trunk_id;
                BOOL_T   is_src_trunk;
                BOOL_T   check_src_port;
            }req;
            struct
            {
            }resp;
        }createipmcastroute;
        union
        {
            struct
            {
                UI32_T   mcast_ip;
                UI32_T   src_ip;
                UI32_T   src_vid;
            }req;
            struct
            {
            }resp;
        }deleteipmcastroute;
        union
        {
            struct
            {
                UI32_T   mcast_ip;
                UI32_T   src_ip;
                UI32_T   src_vid;
                UI32_T   dst_vid;
                UI32_T   unit;
                UI32_T   port;
                UI32_T   trunk_id;
                BOOL_T   is_trunk;
                BOOL_T   is_untagged_vlan;
            }req;
            struct
            {
            }resp;
        }addipmcastportmember;
        union
        {
            struct
            {
                UI32_T   mcast_ip;
                UI32_T   src_ip;
                UI32_T   src_vid;
                UI32_T   dst_vid;
                UI32_T   unit;
                UI32_T   port;
                UI32_T   trunk_id;
                BOOL_T   is_trunk;
                BOOL_T   is_untagged_vlan;
            }req;
            struct
            {
            }resp;
        }addipmcastl2portmember;
        union
        {
            struct
            {
                UI32_T   mcast_ip;
                UI32_T   src_ip;
                UI32_T   src_vid;
                UI32_T   dst_vid;
                UI32_T   unit;
                UI32_T   port;
                UI32_T   trunk_id;
                BOOL_T   is_trunk;
            }req;
            struct
            {
            }resp;
        }deleteipmcastportmember;
        union
        {
            struct
            {
                UI32_T   mcast_ip;
                UI32_T   src_ip;
                UI32_T   src_vid;
                UI32_T   dst_vid;
                UI32_T   unit;
                UI32_T   port;
                UI32_T   trunk_id;
                BOOL_T   is_trunk;
            }req;
            struct
            {
            }resp;
        }deleteipmcastl2portmember;
		
        union
        {
            struct 
            {
                UI32_T vrf_id;
                UI8_T grp_addr_a[16];
                UI8_T src_addr_a[16];
                UI32_T vid;
                UI8_T l2port[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            }req;
			
            struct
            {
            }resp;
        }msl_l2mc_entry;

        union
        {
            struct 
            {
                UI32_T vrf_id;
                UI8_T grp_addr_a[16];
                UI8_T src_addr_a[16];
                UI32_T vid;
                UI8_T l2port[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
                UI8_T count;
                swdrv_dnintf_t dn_intf[SWDRV_MC_VIF_MAX];
            }req;
			
            struct
            {
            }resp;
        }msl_l3mc_entry;

        union
        {
            struct 
            {
                UI32_T vrf_id;
                UI8_T grp_addr_a[16];
                UI8_T src_addr_a[16];
                UI32_T vid;
                UI32_T port;
                UI32_T dn_vid;
                UI32_T status;
            }req;
			
            struct
            {
            }resp;
        }msl_entry;

        union
        {
            struct 
            {
                UI32_T unit;
                UI32_T up_port;
                UI32_T dn_port;
            }req;
			
            struct
            {
            }resp;
        }msl_drv_status;


        union
        {
            struct 
            {
                UI32_T vrf_id;
                UI32_T status;
            }req;
			
            struct
            {
            }resp;
        }msl_status;
		
		
        union
        {
            struct
            {
                UI32_T second;
            }req;
            struct
            {
            }resp;
        }setagetimer;
        union
        {
            struct
            {
                UI32_T subnet;
                UI32_T prefix_length;
            }req;
            struct
            {
            }resp;
        }deletehostroutebysubnet;
        union
        {
            struct
            {
                UI32_T unit;
                UI32_T port;
            }req;
            struct
            {
            }resp;
        }deletehostroutebyport;
        union
        {
            struct
            {
                UI32_T flags;
            }req;
            struct
            {
            }resp;
        }clearallhostroutetable;
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }clearipmcroutetable;
        union
        {
            struct
            {
                UI32_T src_ip;
                UI32_T grp_ip;
                UI32_T vid_index;
            }req;
            struct
            {
                UI32_T hit;
            }resp;
        }readandclearipmcentryhitbit;
        union
        {
            struct
            {
                DEV_SWDRVL3_TunnelInitiator_T tunnel;
            }req;
            struct
            {
		DEV_SWDRVL3_TunnelInitiator_T tunnel;
            }resp;
        }addtunnelinitiator;
        union
        {
            struct
            {
                UI32_T l3_intf_id;
            }req;
            struct
            {
            }resp;
        }deletetunnelinitiator;

        union
        {
            struct
            {
                UI32_T l3_intf_id;
                UI8_T  ttl;
            }req;
            struct
            {
            }resp;
        }updatetunnelttl;
        union
        {
            struct
            {
		DEV_SWDRVL3_TunnelTerminator_T tunnel;
            }req;
            struct
            {
            }resp;
        }addtunnelterminator;
        union
        {
            struct
            {
		DEV_SWDRVL3_TunnelTerminator_T tunnel;
            }req;
            struct
            {
            }resp;
        }deletetunnelterminator;
        union
        {
            struct
            {
		DEV_SWDRVL3_HostTunnel_T host;
            }req;
            struct
            {
		DEV_SWDRVL3_HostTunnel_T host;
            }resp;
        }addinethosttunnelroute;
        union
        {
            struct
            {
		DEV_SWDRVL3_HostTunnel_T host;
            }req;
            struct
            {
		DEV_SWDRVL3_HostTunnel_T host;
            }resp;
        }deleteinethosttunnelroute;

        union
        {
            struct
            {
                UI32_T    status;
            }req;
            struct
            {
            }resp;	
        }setrouteradditionalctrlreg;
        union
        {
            struct
            {
		        DEV_SWDRVL3_TunnelIntfL3_T  tl3;
                BOOL_T                      is_add;
            }req;
            struct
            {
		        DEV_SWDRVL3_TunnelIntfL3_T  tl3;
            }resp;
        }addtunnelintfl3;

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
        struct
        {
            UI32_T balance_mode;
        } setecmpmode;
#endif


    }data;
}DEV_SWDRVL3_PMGR_IPCMSG_T;

enum
{
    DEV_SWDRVL3_IPCCMD_ENABLEUNICASTSTORMPROTECT,
    DEV_SWDRVL3_IPCCMD_DISABLEUNICASTSTORMPROTECT,
    DEV_SWDRVL3_IPCCMD_ENABLEROUTING,
    DEV_SWDRVL3_IPCCMD_ENABLEIP4ROUTING,
    DEV_SWDRVL3_IPCCMD_ENABLEIP6ROUTING,
    DEV_SWDRVL3_IPCCMD_DISABLEROUTING,
    DEV_SWDRVL3_IPCCMD_DISABLEIP4ROUTING,
    DEV_SWDRVL3_IPCCMD_DISABLEIP6ROUTING,
    DEV_SWDRVL3_IPCCMD_CREATEL3INTERFACE,
    DEV_SWDRVL3_IPCCMD_DELETEL3INTERFACE,
    DEV_SWDRVL3_IPCCMD_ADDL3MAC,
    DEV_SWDRVL3_IPCCMD_DELETEL3MAC,
    DEV_SWDRVL3_IPCCMD_SETL3BIT,
    DEV_SWDRVL3_IPCCMD_UNSETL3BIT,    
    DEV_SWDRVL3_IPCCMD_SETROUTERADDITIONALCTRLREG,
    DEV_SWDRVL3_IPCCMD_SETL3INTERFACEMTU,
    DEV_SWDRVL3_IPCCMD_SETINETHOSTROUTE,
    DEV_SWDRVL3_IPCCMD_DELETEINETHOSTROUTE,
    DEV_SWDRVL3_IPCCMD_ADDINETNETROUTE,
    DEV_SWDRVL3_IPCCMD_DELETEINETNETROUTE,
    DEV_SWDRVL3_IPCCMD_ADDTUNNELINITIATOR,
    DEV_SWDRVL3_IPCCMD_DELETETUNNELINITIATOR,
    DEV_SWDRVL3_IPCCMD_UPDATETUNNELTTL,
    DEV_SWDRVL3_IPCCMD_ADDTUNNELTERMINATOR,
    DEV_SWDRVL3_IPCCMD_DELETETUNNELTERMINATOR,
    DEV_SWDRVL3_IPCCMD_ADDINETHOSTTUNNELROUTE,
    DEV_SWDRVL3_IPCCMD_DELETEINETHOSTTUNNELROUTE,
    DEV_SWDRVL3_IPCCMD_ADDINETMYIPHOSTROUTE,
    DEV_SWDRVL3_IPCCMD_DELETEINETMYIPHOSTROUTE,
    DEV_SWDRVL3_IPCCMD_ADDINETECMPROUTE,
    DEV_SWDRVL3_IPCCMD_DELETEINETECMPROUTE,
    DEV_SWDRVL3_IPCCMD_ADDINETECMPROUTEONEPATH,
    DEV_SWDRVL3_IPCCMD_DELETEINETECMPROUTEONEPATH,
    DEV_SWDRVL3_IPCCMD_SETSPECIALDEFAULTROUTE,
    DEV_SWDRVL3_IPCCMD_DELETESPECIALDEFAULTROUTE,
    DEV_SWDRVL3_IPCCMD_READANDCLEARHOSTROUTEHITBIT,
    DEV_SWDRVL3_IPCCMD_READANDCLEARNETROUTEHITBIT,
    DEV_SWDRVL3_IPCCMD_CLEARHOSTROUTEHWINFO,
    DEV_SWDRVL3_IPCCMD_CLEARNETROUTEHWINFO,
    DEV_SWDRVL3_IPCCMD_ADDOFL3FWDFLOW,
    DEV_SWDRVL3_IPCCMD_DELETEOFL3FWDFLOW,
    DEV_SWDRVL3_IPCCMD_READANDCLEAROFL3FWDFLOWHITBIT,
    DEV_SWDRVL3_IPCCMD_ADDTUNNELINTFL3,
    DEV_SWDRVL3_IPCCMD_SETECMPBALANCEMODE,
};

#define DEV_SWDRVL3_IPCCMD_MC_BASE (100)
enum
{
    DEV_SWDRVL3_IPCCMD_IPMC_ENABLE = DEV_SWDRVL3_IPCCMD_MC_BASE,      
    DEV_SWDRVL3_IPCCMD_IPMC_DISABLE, 		
    DEV_SWDRVL3_IPCCMD_IPMC_DEBUG_ENABLE,
    DEV_SWDRVL3_IPCCMD_IPMC_DEBUG_DISABLE,
    DEV_SWDRVL3_IPCCMD_L2MC_PORT_ADD,/*base+5*/
    DEV_SWDRVL3_IPCCMD_L2MC_PORT_DEL,    
    DEV_SWDRVL3_IPCCMD_L2MC_ENTRY_ADD,
    DEV_SWDRVL3_IPCCMD_L2MC_ENTRY_DEL,    
    DEV_SWDRVL3_IPCCMD_L3MC_PORT_ADD, 
    DEV_SWDRVL3_IPCCMD_L3MC_PORT_DEL,    /*base+10*/
    DEV_SWDRVL3_IPCCMD_L3MC_DNINTF_ADD,/*downstream interface*/    
    DEV_SWDRVL3_IPCCMD_L3MC_DNINTF_DEL,
    DEV_SWDRVL3_IPCCMD_L3MC_ENTRY_ADD,
    DEV_SWDRVL3_IPCCMD_L3MC_ENTRY_DEL,
    DEV_SWDRVL3_IPCCMD_L3MC_ENTRY_STAT,/*base+15*/ 
    DEV_SWDRVL3_IPCCMD_L3MC_CPU_PORT_ADD,
    DEV_SWDRVL3_IPCCMD_L3MC_CPU_PORT_DEL,
    DEV_SWDRVL3_IPCCMD_L3MC_REPL_ADD,
    DEV_SWDRVL3_IPCCMD_L3MC_REPL_DEL 
};

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_EnableUnicastStormProtect
 *
 * Purpose: to prevent the unicast storm to CPU
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 * Note :  (unit=0, port=0) means per system protect
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_EnableUnicastStormProtect(UI32_T unit, UI32_T port) ;

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DisableUnicastStormProtect
 * Purpose: to allow the unicast storm to CPU
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 * Note :  (unit=0, port=0) means per system protect
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DisableUnicastStormProtect(UI32_T unit, UI32_T port) ;

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_EnableRouting
 *
 * Purpose: This function enables the L3 Routing function.
 * Inputs: None.
 * Outputs: None.
 * Return: TRUE/FALSE
 * Notes: None.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_EnableRouting(UI32_T flags, UI32_T fib_id);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_EnableIPv4Routing
 *
 * Purpose: This function enables the L3 IPv4 Routing function.
 * Inputs: None.
 * Outputs: None.
 * Return: TRUE/FALSE
 * Notes: None.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_EnableIPv4Routing(UI32_T flags, UI32_T fib_id);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_EnableIPv6Routing
 *
 * Purpose: This function enables the L3 IPv6 Routing function.
 * Inputs: None.
 * Outputs: None.
 * Return: TRUE/FALSE
 * Notes: None.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_EnableIPv6Routing(UI32_T flags, UI32_T fib_id);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DisableRouting
 *
 * Purpose: Disable GalNet-3 routing function.
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DisableRouting(UI32_T flags, UI32_T fib_id) ;

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DisableIPv4Routing
 *
 * Purpose: Disable GalNet-3 routing function.
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DisableIPv4Routing(UI32_T flags, UI32_T fib_id);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DisableIPv6Routing
 *
 * Purpose: Disable GalNet-3 routing function.
 * Inputs: None
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DisableIPv6Routing(UI32_T flags, UI32_T fib_id);



/*******************************************************************************
 * DEV_SWDRVL3_PMGR_CreateL3Interface
 *
 * Purpose: This function will create an L3 interface.
 * Inputs: 
 *          fib_id - FIB Id
 *          vid - VLAN ID of the interface
 *          vlan_mac - MAC address of the interface
 * Outputs:
 *          hw_info  - bcm_l3_intf_t.l3a_intf_id(interface id of L3 table on chip)
 * RETURN:  UI32_T   - Error code
 * Notes:
 *          1. The MAC address will be the Router MAC for any subnet/IP interface
 *             deployed on this VLAN.
 *          2. The MAC address should be added to ARL with BCM solution.
 *          3. hw_info is required to delete the L3 interface on chip.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_CreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T *hw_info);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteL3Interface
 *
 * Purpose: This function will delete an L3 interface
 * Inputs: 
 *          fib_id - FIB Id
 *          vid - VLAN ID of the interface
 *          vlan_mac - MAC address of the interface
 *          hw_info  - bcm_l3_intf_t.l3a_intf_id which is output from
 *                     DEV_SWDRVL3_PMGR_CreateL3Interface()
 * Outputs: None
 * Return: TRUE/FALSE
 * Notes:
 *          1. The MAC address will be the Router MAC for any subnet/IP interface
 *             deployed on this VLAN.
 *          2. The MAC address should be delete from ARL with BCM solution.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DeleteL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T hw_info);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddL3Mac
 *
 * Purpose: Add one MAC address with L3 bit On(For vrrp HSRP)
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * RETURN:  UI32_T    Success, or cause of fail.
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *       3. If IP Routing is disabled, the router MAC should just delete L3 bit.
 *          And the behavior of CPU mac will be just the same as L2 Intervention.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddL3Mac(UI32_T vid, UI8_T *l3_mac);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteL3Mac
 *
 * Purpose: Remove one L3 MAC address that belonging to one vlan interface.
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *       2. For Intel solution, there is only ONE MAC for the whole routing system.
 *          This function is non-applicable for Intel solution
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DeleteL3Mac(UI32_T vid, UI8_T *l3_mac);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_SetL3Bit
 *
 * Purpose: Turn on the L3 bit of a MAC on a vlan interface
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_SetL3Bit(UI32_T vid, UI8_T *l3_mac);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_UnSetL3Bit
 *
 * Purpose: Turn off the L3 bit of a MAC on a vlan interface
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_UnSetL3Bit(UI32_T vid, UI8_T *l3_mac);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_SetL3InterfaceMtu
 *
 * Purpose: Set L3 MTU on vlan interface
 * Inputs:
 *          vid  : VLAN ID
 *          mtu  : L3 interface MTU 
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_SetL3InterfaceMtu(UI32_T vid, UI32_T mtu);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_SetInetHostTunnelRoute
 *
 * Purpose: This function will add or update a host entry.
 * Inputs:
 *          host  : host entry with tunneling information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *                  host->hw_info If the action is updating
 *		    host->tnl_init Tunnel initiator associated with this host entry
 *		    host->tnl_term Tunnel terminator associated with this host entry
 *
 * Outputs: 
 *          host->hw_info
 *	    host->tnl_init.l3a_intf_id
 *
 * Return:  DEV_SWDRVL3_L3_BUCKET_FULL
 *          DEV_SWDRVL3_L3_NO_ERROR
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. If hw_info is invalid, that means create a new host entry and BCM Egress Object.
 *          3. If hw_info not invalid, do updating of BCM Egress Object.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddInetHostTunnelRoute(DEV_SWDRVL3_HostTunnel_T *host);




/*******************************************************************************
 * DEV_SWDRVL3_PMGR_SetInetHostRoute
 *
 * Purpose: This function will add or update a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *                  host->hw_info If the action is updating
 *
 * Outputs: 
 *          host->hw_info
 * Return:  DEV_SWDRVL3_L3_BUCKET_FULL
 *          DEV_SWDRVL3_L3_NO_ERROR
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. If hw_info is invalid, that means create a new host entry and BCM Egress Object.
 *          3. If hw_info not invalid, do updating of BCM Egress Object.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_SetInetHostRoute(DEV_SWDRVL3_Host_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteInetHostTunnelRoute
 *
 * Purpose: This function deletes a host entry and assoicated l3_intf tunnel initiator/terminator.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *		    host->tnl_init
 *		    host->tnl_term
 *
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. This function should call bcm_l3_egress_destroy to clear the hw_info, 
 *             if this action failed, Core Layer (AMTRL3) should call 
 *             the "DEV_SWDRVL3_ClearHostRouteHWInfo" to clear the hw_info.
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_DeleteInetHostTunnelRoute(DEV_SWDRVL3_HostTunnel_T *host);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteInetHostRoute
 *
 * Purpose: This function will delete a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->mac
 *                  host->vid
 *                  host->unit
 *                  host->port OR host->trunk_id
 *
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    1. The hw_info indicate the Egress-Object handler in BCM SDK.
 *          2. This function should call bcm_l3_egress_destroy to clear the hw_info, 
 *             if this action failed, Core Layer (AMTRL3) should call 
 *             the "DEV_SWDRVL3_ClearHostRouteHWInfo" to clear the hw_info.
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DeleteInetHostRoute(DEV_SWDRVL3_Host_T *host);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddInetNetRoute
 *
 * Purpose: This function will add a route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *
 *          nh_hw_info : The HW information of the nexthop
 *
 * Outputs: 
 *          host->hw_info
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddInetNetRoute(DEV_SWDRVL3_Route_T *route, void *nh_hw_info);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteInetNetRoute
 *
 * Purpose: This function will delete a route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *
 *          nh_hw_info : The HW information of the nexthop
 *
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DeleteInetNetRoute(DEV_SWDRVL3_Route_T *route, void *nh_hw_info);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddTunnelIntfL3
 *
 * Purpose: This function will add/del a tunnel l3 intf.
 * Inputs:
 *          tl3_p : tunnel l3 intf information
 *                  l3_intf_id is key for delete
 *
 *          is_add: TRUE if it's to add
 * Outputs:
 *          tl3_p->l3_intf_id for add
 * Return:
 *  	    DEV_SWDRVL3_L3_NO_ERROR/DEV_SWDRVL3_L3_OTHERS
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddTunnelIntfL3(
    DEV_SWDRVL3_TunnelIntfL3_T  *tl3_p,
    BOOL_T                      is_add);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddTunnelInitiator
 *
 * Purpose: This function will add a tunnel initiator.
 * Inputs:
 *          tunnel : tunnel initiator information
 *              The input KEY fields are:
 *                  tunnel->tunnel_type
 *                  tunnel->sip
 *                  tunnel->dip
 *                  tunnel->ttl
 *                  tunnel->nexthop_mac
 *
 * Outputs: 
 *          tunnel->l3_intf_id
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddTunnelInitiator(DEV_SWDRVL3_TunnelInitiator_T *tunnel);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteTunnelInitiator
 *
 * Purpose: This function will delete a tunnel initiator associated with specific
 *          L3 interface ID.
 * Inputs:
 *          l3_intf_id : The L3 Interface ID associated with the tunnel initiator
 *
 * Outputs: None
 *          
 * Return:  DEV_SWDRVL3_L3_NO_ERROR 	Delete successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_DeleteTunnelInitiator(UI32_T l3_intf_id);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_UpdateTunnelTtl
 *
 * Purpose: This function will add a tunnel terminator.
 * Inputs:
 *          tunnel : tunnel initiator information
 *              The input KEY fields are:
 *                  tunnel->l3_intf_id
 *                  tunnel->ttl
 *
 * Outputs: N/A
 *          
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_UpdateTunnelTtl(DEV_SWDRVL3_TunnelInitiator_T *tunnel);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddTunnelTerminator
 *
 * Purpose: This function will add a tunnel terminator.
 * Inputs:
 *          tunnel : tunnel terminator information
 *              The input KEY fields are:
 *                  tunnel->fib_id
 *                  tunnel->tunnel_type
 *                  tunnel->sip
 *                  tunnel->dip
 *                  tunnel->port_bitmap
 *
 * Outputs: None
 *          
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddTunnelTerminator(DEV_SWDRVL3_TunnelTerminator_T *tunnel);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteTunnelTerminator
 *
 * Purpose: This function will delete a tunnel terminator.
 * Inputs:
 *          tunnel : tunnel terminator information
 *              The input KEY fields are:
 *                  tunnel->fib_id
 *                  tunnel->tunnel_type
 *                  tunnel->sip
 *                  tunnel->dip
 *                  tunnel->port_bitmap
 *
 * Outputs: None
 *          
 * Return:  
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_DeleteTunnelTerminator(DEV_SWDRVL3_TunnelTerminator_T *tunnel);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddInetMyIpHostRoute
 *
 * Purpose: This function will add a "My IP" host entry.
 * Inputs:
 *          host  : My IP Host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->unit
 *
 * Outputs: None
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Add net entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddInetMyIpHostRoute(DEV_SWDRVL3_Host_T *host);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteInetMyIpHostRoute
 *
 * Purpose: This function will delete a "My IP" host entry.
 * Inputs:
 *          host  : My IP Host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *                  host->unit
 *
 * Outputs: None
 *          DEV_SWDRVL3_L3_BUCKET_FULL 	HW entry bucket is full
 *  	    DEV_SWDRVL3_L3_NO_ERROR 	Delete host entry successfully
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_DeleteInetMyIpHostRoute(DEV_SWDRVL3_Host_T *host);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddInetECMPRouteMultiPath
 *
 * Purpose: This function will add an ECMP route with multiple nexthop.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *
 *          nh_hw_info_array : The array pointer of HW information of the multiple nexthop
 *          count : Indicate number of the multiple nexthop
 *
 * Outputs: None
 * Return:
 *          DEV_SWDRVL3_L3_ECMP_BUCKET_FULL
 *          DEV_SWDRVL3_L3_BUCKET_FULL
 *  	    DEV_SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddInetECMPRouteMultiPath(DEV_SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteInetECMPRoute
 *
 * Purpose: This function will delete an ECMP route.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *                  route->hw_info
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DeleteInetECMPRoute(DEV_SWDRVL3_Route_T *route);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_AddInetECMPRouteOnePath
 *
 * Purpose: his function will add a single nexthop of an ECMP route.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *                  route->hw_info
 *
 *          nh_hw_info 	The HW information of the nexthop
 *          is_first 	Indicate the nexthop is the first one or not
 *
 * Outputs: route->hw_info
 * Return:  
 *          DEV_SWDRVL3_L3_OTHERS
 *          DEV_SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T DEV_SWDRVL3_PMGR_AddInetECMPRouteOnePath(DEV_SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_first);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteInetECMPRouteOnePath
 *
 * Purpose: This function will delete a single nexthop of an ECMP route.
 * Inputs:
 *          route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *                  route->unit
 *                  route->hw_info
 *
 *          nh_hw_info 	The HW information of the nexthop
 *          is_first 	Indicate the nexthop is the first one or not
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DeleteInetECMPRouteOnePath(DEV_SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_last);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_SetSpecialDefaultRoute
 *
 * Purpose: This function will add a default route entry for special purpose.
 * Inputs:
 *          default_route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *
 *          action : Indicate the special purpose of this Default Route entry
 *              The "action" are:
 *                  SWDRVL3_ACTION_TRAP2CPU Trap packet to CPU if match
 *                  SWDRVL3_ACTION_DROP Drop packet if match
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    The implementation DOES NOT accept "SWDRVL3_ACTION_ROUTE".
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_SetSpecialDefaultRoute(DEV_SWDRVL3_Route_T *default_route, UI32_T action);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_DeleteSpecialDefaultRoute
 *
 * Purpose: This function will delete the special default route entry.
 * Inputs:
 *          default_route  : Route entry information 
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *
 *          action : Indicate the special purpose of this Default Route entry
 *              The "action" are:
 *                  SWDRVL3_ACTION_TRAP2CPU Trap packet to CPU if match
 *                  SWDRVL3_ACTION_DROP Drop packet if match
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    The implementation DOES NOT accept "SWDRVL3_ACTION_ROUTE".
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_DeleteSpecialDefaultRoute(DEV_SWDRVL3_Route_T *default_route, UI32_T action);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_ReadAndClearHostRouteEntryHitBit
 *
 * Purpose: This function will Read and Clear hit bit of a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->flags
 *                  host->fib_id
 *                  host->ip_addr
 *
 *          hit : The read value of Hit bit
 *
 * Outputs: route
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_ReadAndClearHostRouteEntryHitBit(DEV_SWDRVL3_Host_T *host, UI32_T *hit);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_ReadAndClearNetRouteEntryHitBit
 *
 * Purpose: This function will Read and Clear hit bit of a net route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->flags
 *                  route->fib_id
 *                  route->dst_ip
 *                  route->prefix_length
 *
 *          hit : The read value of Hit bit
 *
 * Outputs: hit
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_ReadAndClearNetRouteEntryHitBit(DEV_SWDRVL3_Route_T *route, UI32_T *hit);

/*******************************************************************************
 * DEV_SWDRVL3_PMGR_ClearHostRouteHWInfo
 *
 * Purpose: This function will Clear hw_info of a host entry.
 * Inputs:
 *          host  : host entry information
 *              The input KEY fields are:
 *                  host->hw_info
 *
 * Outputs: host->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_ClearHostRouteHWInfo(DEV_SWDRVL3_Host_T *host);


/*******************************************************************************
 * DEV_SWDRVL3_PMGR_ClearNetRouteHWInfo
 *
 * Purpose: This function will Clear hw_info of an ECMP route entry.
 * Inputs:
 *          route  : route entry information
 *              The input KEY fields are:
 *                  route->hw_info
 *
 * Outputs: route->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T DEV_SWDRVL3_PMGR_ClearNetRouteHWInfo(DEV_SWDRVL3_Route_T *route);

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRVL3_PMGR_SetEcmpBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set ECMP balance mode
 * INPUT   : balance_mode bitmap - DEV_SWDRVL3_ECMP_DST_IP
 *                                 DEV_SWDRVL3_ECMP_L4_PORT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : Call DEV_SWDRV_PMGR_SetHashSelectionForECMP for Hash-Selection
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRVL3_PMGR_SetEcmpBalanceMode(UI32_T balance_mode);
#endif

/* -------------------------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRVL3_PMGR_InitiateProcessResource
 * -------------------------------------------------------------------------------------------
 * PURPOSE : This function is used to initiate the information for IPC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------------------------
 */
void DEV_SWDRVL3_PMGR_InitiateProcessResource() ;


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  DEV_SWDRVL3_PMGR_Backdoor_Main
 *-------------------------------------------------------------------------
 * PURPOSE  : DEV_SWDRVL3_PMGR backdoor function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void DEV_SWDRVL3_PMGR_Backdoor_Main(void);

void DEV_SWDRVL3_PMGR_Create_InterCSC_Relation(void);



I32_T 
DEV_SWDRVL3_PMGR_ipmc_enable(UI32_T unit, UI32_T up_port, UI32_T dn_port);

I32_T 
DEV_SWDRVL3_PMGR_ipmc_disable(void);

I32_T 
DEV_SWDRVL3_PMGR_ipmc_debug_enable(void);

I32_T 
DEV_SWDRVL3_PMGR_ipmc_debug_disable(void);
#if 0 /* not used now for canceling copile warnning*/
I32_T 
DEV_SWDRVL3_PMGR_l2mc_port_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T port);

I32_T 
DEV_SWDRVL3_PMGR_l2mc_port_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T port);
#endif
I32_T 
DEV_SWDRVL3_PMGR_l2mc_entry_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI8_T *l2port);

I32_T 
DEV_SWDRVL3_PMGR_l2mc_entry_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[]);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_port_add(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T vid, UI32_T port);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_port_del(UI32_T vrf_id, UI32_T up_vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T vid, UI32_T port);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_dnintf_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], swdrv_dnintf_t *dn_intf);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_dnintf_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], swdrv_dnintf_t *dn_intf);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_repl_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], swdrv_dnintf_t *dn_intf);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_repl_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], swdrv_dnintf_t *dn_intf);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_entry_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI8_T *l2port, UI8_T count, swdrv_dnintf_t *dn_intf);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_entry_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[]);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_entry_stat(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[], UI32_T *count);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_cpu_port_del(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[]);

I32_T 
DEV_SWDRVL3_PMGR_l3mc_cpu_port_add(UI32_T vrf_id, UI32_T vid, UI8_T grp_addr_a[], UI8_T src_addr_a[]);

#endif /* #ifndef _DEV_SWDRVL3_PMGR_H_ */

