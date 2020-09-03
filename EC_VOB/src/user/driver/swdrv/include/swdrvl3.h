/*******************************************************************
 *
 *    DESCRIPTION:
 *       Layer 3 Driver Layer API Specifications
 *
 *    AUTHOR:
 *
 *    HISTORY:
 *
 *   By              Date     Ver.   Modification Description
 *   --------------- -------- -----  ---------------------------------------
 *   Anderson                        Created
 *   Ted           01/24/2002        1. Clarify the API
 *                                   2. Change the IPMC related APIs
 *   Ted           07/09/2002        Fixed SWDRVL3_DeleteHostRoute() input parameter "dst_vid" data type
 *   Garfield      06/12/2003        1. Remove parameter next_hop_gateway_ip
 *                                      in api SWDRVL3_AddNetRoute()
 *                                   2. Add two apis SWDRVL3_EnableUnicastStormProtect()
 *                                      and SWDRVL3_DisableUnicastStormProtect()
 *   Garfield      05/24/2004        1. Add one new API SWDRVL3_AddNetRouteWithNextHopIp. This API is 
 *                                      for adding net route with next hop IP. It is needed by SDK4.2.3
 *   Garfield      06/17/2004        1. Add one new API:
 *                                      SWDRVL3_SetDefaultRouteWithNextHopIp()
 *                                      This API is for adding default route with next hop IP.
 *                                      The next hop IP is needed by SDK4.2.3
 *   Vai           05/21/2008        1. Change the L3Unicast related APIs
 *   Terry Liu	   07/29/2009	     1. Add Tunnel related structure and API
 *					SWDRVL3_TunnelInitiator_S
 *					SWDRVL3_TunnelTerminator_S
 *					SWDRVL3_AddTunnelInitiator()
 *					SWDRVL3_DeleteTunnelInitiator()
 *					SWDRVL3_AddTunnelTerminator()
 *					SWDRVL3_DeleteTunnelTerminator()
 *******************************************************************
 */

#ifndef SWDRVL3_H
#define SWDRVL3_H



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "l_inet.h"
#include "dev_swdrvl3.h"
#include "dev_swdrv.h"
#include "swdrvl3_type.h"

#if (SYS_CPNT_STACKING == TRUE)
#include "l_mm.h"
#include "isc.h"
#endif /* SYS_CPNT_STACKING == TRUE */

/* NAMING CONSTANT DECLARATIONS
 */
/* 1-30-2002
*/
/** \def SWDRVL3_ACTION_TRAP2CPU
 *  Define the route action is "TRAP 2 CPU" 
 * \def SWDRVL3_ACTION_ROUTE
 *  Define the route action is "Normal Forwarding" 
 * \def SWDRVL3_ACTION_DROP
 *  Define the route action is "Drop" 
 */
#define SWDRVL3_ACTION_TRAP2CPU         DEV_SWDRVL3_ACTION_TRAP2CPU
#define SWDRVL3_ACTION_ROUTE            DEV_SWDRVL3_ACTION_ROUTE
#define SWDRVL3_ACTION_DROP             DEV_SWDRVL3_ACTION_DROP

/** \def SWDRVL3_FLAG_IPV4
 *  Indicate the \c host or \c route type is IPv4
 *  \def SWDRVL3_FLAG_IPV6
 *  Indicate the \c host or \c route type is IPv6
 *  \def SWDRVL3_FLAG_ECMP
 *  Indicate the \c route is an ECMP entry
 *  \def SWDRVL3_FLAG_STATIC
 *  Indicate the \c host is a static ARP or \c route is a 
 *  static ROUTE
 *  \def SWDRVL3_FLAG_TRUNK_EGRESS_PORT
 *  Indicate the \c egress-port is a Trunk port
 *  \def SWDRVL3_FLAG_TAGGED_EGRESS_VLAN
 *  Indicate the \c egress-port is a tagged member of the \c egress-vlan
 *  \def SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE
 *  Indicate the \c route is a local-connected route
 */
#define SWDRVL3_FLAG_IPV4                       DEV_SWDRVL3_FLAG_IPV4             
#define SWDRVL3_FLAG_IPV6                       DEV_SWDRVL3_FLAG_IPV6
#define SWDRVL3_FLAG_ECMP                       DEV_SWDRVL3_FLAG_ECMP
#define SWDRVL3_FLAG_ECMP_ONE_PATH              DEV_SWDRVL3_FLAG_ECMP_ONE_PATH
#define SWDRVL3_FLAG_ECMP_ONE_PATH_ALONE        DEV_SWDRVL3_FLAG_ECMP_ONE_PATH_ALONE
#define SWDRVL3_FLAG_STATIC                     DEV_SWDRVL3_FLAG_STATIC
#define SWDRVL3_FLAG_TRUNK_EGRESS_PORT          DEV_SWDRVL3_FLAG_TRUNK_EGRESS_PORT
#define SWDRVL3_FLAG_TAGGED_EGRESS_VLAN         DEV_SWDRVL3_FLAG_TAGGED_EGRESS_VLAN
#define SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE      DEV_SWDRVL3_FLAG_LOCAL_CONNECTED_ROUTE
#define SWDRVL3_FLAG_PROCESS_L3_EGRESS_ONLY     DEV_SWDRVL3_FLAG_PROCESS_L3_EGRESS_ONLY    
#define SWDRVL3_FLAG_STATIC_TUNNEL              DEV_SWDRVL3_FLAG_STATIC_TUNNEL
#define SWDRVL3_FLAG_DYNAMIC_TUNNEL             DEV_SWDRVL3_FLAG_DYNAMIC_TUNNEL
/** \def SWDRVL3_L3_BUCKET_FULL
 *  Define the error code is "Bucket Full"
 *  \def SWDRVL3_L3_ECMP_BUCKET_FULL
 *  Define the error code is "ECMP Bucket Full"
 *  \def SWDRVL3_L3_NO_ERROR
 *  Define the error code is "No Error"
 *  \def SWDRVL3_L3_OTHERS
 *  Define the error code is "Other Misc. Reason"
 *  \def SWDRVL3_L3_EXISTS
 *  Define the error code is "Entry is Exist"
 */
#define    SWDRVL3_L3_BUCKET_FULL       DEV_SWDRVL3_L3_BUCKET_FULL
#define    SWDRVL3_L3_ECMP_BUCKET_FULL  DEV_SWDRVL3_L3_ECMP_BUCKET_FULL
#define    SWDRVL3_L3_NO_ERROR          DEV_SWDRVL3_L3_NO_ERROR
#define    SWDRVL3_L3_OTHERS            DEV_SWDRVL3_L3_OTHERS
#define    SWDRVL3_L3_EXISTS            DEV_SWDRVL3_L3_EXISTS
#define    SWDRVL3_L3_MAC_COLLISION     DEV_SWDRVL3_L3_MAC_COLLISION


/** \def SWDRVL3_HW_INFO_INVALID
 *  Define the default (invalide) information of hw_info
 */
#define SWDRVL3_HW_INFO_INVALID           DEV_SWDRVL3_HW_INFO_INVALID

#define SWDRVL3_TUNNELTYPE_MANUAL	DEV_SWDRVL3_TUNNELTYPE_MANUAL
#define SWDRVL3_TUNNELTYPE_6TO4	DEV_SWDRVL3_TUNNELTYPE_6TO4	    
#define SWDRVL3_TUNNELTYPE_ISATAP	DEV_SWDRVL3_TUNNELTYPE_ISATAP	    





/* DATA TYPE DECLARATIONS
 */
/** \struct SWDRVL3_Host_S
 *  \typedef SWDRVL3_Host_T
 *  Define the host entry structure
 */
typedef struct SWDRVL3_Host_S
{
    UI32_T flags;
    UI32_T fib_id;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }ip_addr;
    /* The MAC address of the host */
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    /* The MAC of the router */
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T vid;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    void*  hw_info;
}SWDRVL3_Host_T;


/** \struct SWDRVL3_Route_S
 *  \typedef SWDRVL3_Route_T
 *  Define the route entry structure
 */
typedef struct SWDRVL3_Route_S
{
    UI32_T flags;
    UI32_T fib_id;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }dst_ip;
    UI32_T prefix_length;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }next_hop_ip;
    /* The MAC address of the router */
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
    /* The MAC address of the target next hop */
    UI8_T  nexthop_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T dst_vid;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    void*  hw_info;
}SWDRVL3_Route_T;

#define  SWDRVL3_TunnelInitiator_T        TunnelInitiator_T
#define   SWDRVL3_TunnelTerminator_T   TunnelTerminator_T
typedef struct SWDRVL3_HostTunnel_S
{
    UI32_T flags;
    UI32_T fib_id;
    union{
        UI32_T ipv4_addr;
        UI8_T  ipv6_addr[SYS_ADPT_IPV6_ADDR_LEN];
    }ip_addr;
    /* The MAC address of the host */
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    /* The MAC of the router */
    UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T vid;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    void*  hw_info;
    TunnelInitiator_T  tnl_init;
    TunnelTerminator_T  tnl_term;
}SWDRVL3_HostTunnel_T;



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/** \fn void SWDRVL3_InitiateSystemResources(void)
 *  \brief Initialize L3 switch driver
 *  \return None
 */
/* FUNCTION NAME: SWDRVL3_Inititate_System_Resources
 *----------------------------------------------------------------------------------
 * PURPOSE: init L3 switch driver
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL3_InitiateSystemResources(void);


/** \fn void SWDRVL3_AttachSystemResources(void)
 *  \brief Initialize share memory semaphore
 *  \return None
 */
/* FUNCTION NAME: SWDRVL3_AttachSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: init share memory semaphore
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL3_AttachSystemResources(void);


/** \fn void SWDRVL3_Create_InterCSC_Relation(void)
 *  \brief This function initializes all function pointer registration operations
 *  \return None
 */
/* FUNCTION NAME: SWDRVL3_Create_InterCSC_Relation
 *----------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void SWDRVL3_Create_InterCSC_Relation(void);


/** \fn void SWDRVL3_EnterMasterMode(void)
 *  \brief This function will configurate the Layer 3 Switch Driver module to
 *  enter master mode after stacking
 *  \return None
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_EnterMasterMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           enter master mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_EnterMasterMode(void);


/** \fn void SWDRVL3_EnterSlaveMode(void)
 *  \brief This function will configurate the Layer 3 Switch Driver module to
 *  enter slave mode after stacking
 *  \return None
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_EnterSlaveMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           enter slave mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_EnterSlaveMode(void);


/** \fn void SWDRVL3_EnterTransitionMode(void)
 *  \brief This function will configurate the Layer 3 Switch Driver module to
 *  enter transition mode after stacking
 *  \return None
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_EnterTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           enter transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_EnterTransitionMode(void);


/** \fn void SWDRVL3_SetTransitionMode(void)
 *  \brief This function will configurate the Layer 3 Switch Driver module to
 *  set transition mode after stacking
 *  \return None
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_SetTransitionMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will configurate the Layer 3 Switch Driver module to
 *           set transition mode after stacking
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWDRVL3_SetTransitionMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_GetShMemInfo
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return the total size of shared memory.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : UI32_T Shared memory size
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T SWDRVL3_GetShMemInfo(void);


/** \fn void SWDRVL3_CreateTask(void)
 *  \brief This function will create swdrvl3 task
 *  \return None
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_CreateTask                                        
 *------------------------------------------------------------------------
 * FUNCTION: This function will create swdrvl3 task            
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWDRVL3_CreateTask(void);

/** \fn BOOL_T SWDRVL3_EnableIpForwarding(UI32_T flag, UI32_T vr_id)
 *  \brief This function will enable IPv4/IPv6 Forwarding Function
 *  \param[in] flag  (IN) Indicate either IPv4 or IPv6 forwarding function is enabled
 *  \param[in] vr_id (IN) Virtual Router ID
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_EnableIpForwarding
 *
 * Purpose: This function will enable IPv4/IPv6 Forwarding Function.
 * Inputs:
 *          flags   : Indicate either IPv4 or IPv6 forwarding function is enabled
 *	    vr_id   : Virtual Router ID
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T SWDRVL3_EnableIpForwarding(UI32_T flag, UI32_T vr_id);

/** \fn BOOL_T SWDRVL3_DisableIpForwarding(UI32_T flag, UI32_T vr_id)
 *  \brief This function will disable IPv4/IPv6 Forwarding Function
 *  \param[in] flag  (IN) Indicate either IPv4 or IPv6 forwarding function is disabled
 *  \param[in] vr_id (IN) Virtual Router ID
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_DisableIpForwarding
 *
 * Purpose: This function will disable IPv4/IPv6 Forwarding Function.
 * Inputs:
 *          flags   : Indicate either IPv4 or IPv6 forwarding function is disabled
 *	    vr_id   : Virtual Router ID
 * Outputs: None
 * Return: TRUE/FALSE
 *******************************************************************************
 */
BOOL_T SWDRVL3_DisableIpForwarding(UI32_T flag, UI32_T vr_id);

/** \fn BOOL_T SWDRVL3_CreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac)
 *  \brief This function will create an L3 interface
 *  \param[in] fib_id (IN) FIB Id
 *  \param[in] vid (IN) VLAN ID of the interface
 *  \param[in] vlan_mac (IN) MAC address of the interface
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 *  \note 
 *  <ul>
 *    <ol>
 *      <li>The MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN.</li>
 *      <li>The MAC address should be added to ARL with BCM solution.
 *    </ol>
 *  </ul>
 */
/*******************************************************************************
 * SWDRVL3_CreateL3Interface
 *
 * Purpose: This function will create an L3 interface.
 * Inputs:
 *          fib_id  : FIB ID
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs:
 *          hw_info : hw info (index) of the l3 interface
 * RETURN:  UI32_T    Success, or cause of fail.
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *******************************************************************************
 */
UI32_T SWDRVL3_CreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T *hw_info);


/*******************************************************************************
 * SWDRVL3_HotInsertCreateL3Interface
 *
 * Purpose: This function will create an L3 interface.
 * Inputs:
 *          fib_id  : FIB ID
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 *          hw_info : hw info (index) of the L3 interface
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *******************************************************************************
 */
BOOL_T SWDRVL3_HotInsertCreateL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T hw_info);


/** \fn BOOL_T SWDRVL3_DeleteL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac)
 *  \brief This function will delete an L3 interface
 *  \param[in] fib_id (IN) FIB Id
 *  \param[in] vid (IN) VLAN ID of the interface
 *  \param[in] vlan_mac (IN) MAC address of the interface
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 *  \note 
 *  <ul>
 *  <ol><li>The MAC address will be the Router MAC for any subnet/IP interface
 *      deployed on this VLAN.</li>
 *      <li>The MAC address should be delete from ARL with BCM solution.</li>
 *  </ol>
 *  </ul>
 */
/*******************************************************************************
 * SWDRVL3_DeleteL3Interface
 *
 * Purpose: This function will delete an L3 interface.
 * Inputs:
 *          fib_id  : FIB ID
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * Return: TRUE/FALSE
 * Note: 1. This MAC address will be the Router MAC for any subnet/IP interface
 *          deployed on this VLAN
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteL3Interface(UI32_T fib_id, UI32_T vid, const UI8_T *vlan_mac, UI32_T hw_info);

/*******************************************************************************
 * SWDRVL3_AddL3Mac
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
 *******************************************************************************
 */
UI32_T SWDRVL3_AddL3Mac(UI8_T *l3_mac, UI32_T vid);


/*******************************************************************************
 * SWDRVL3_DeleteL3Mac
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
BOOL_T SWDRVL3_DeleteL3Mac(UI8_T *l3_mac, UI32_T vid);


/*******************************************************************************
 * SWDRVL3_SetL3Bit
 *
 * Purpose: Turn on the L3 bit of a MAC on a vlan interface
 * Inputs:
 *			vlan_mac: MAC address of vlan interface
 *			vid		: VLAN ID of this MAC address
 * Outputs: None
 * RETURN:  UI32_T    Success, or cause of fail.
 * Note: None
 *******************************************************************************
 */
UI32_T SWDRVL3_SetL3Bit(UI8_T *l3_mac, UI32_T vid);


/*******************************************************************************
 * SWDRVL3_UnSetL3Bit
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
BOOL_T SWDRVL3_UnSetL3Bit(UI8_T *l3_mac, UI32_T vid);

/*******************************************************************************
 * SWDRVL3_SetInetHostRoute
 *
 * Purpose: This function will add or update a host entry.
 * Inputs:
 *          host  : host entry information
 * Outputs: 
 *          host->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_SetInetHostRoute(SWDRVL3_Host_T *host);
UI32_T SWDRVL3_AddInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host);
UI32_T SWDRVL3_DeleteInetHostTunnelRoute(SWDRVL3_HostTunnel_T *host);
UI32_T SWDRVL3_TunnelUpdateTtl(SWDRVL3_HostTunnel_T *tunnel_entry);
/*******************************************************************************
 * SWDRVL3_HotInsertAddInetHostRoute
 *
 * Purpose: This function will add or update a host entry.
 * Inputs:
 *          host  : host entry information
 * Outputs: 
 *          host->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetHostRoute(SWDRVL3_Host_T *host);


/*******************************************************************************
 * SWDRVL3_HotInsertAddInetHostTunnelRoute
 *
 * Purpose: This function will add or update a tunnel host entry.
 * Inputs:
 *          tunnel_host  : tunnel host entry information
 * Outputs: 
 *          host->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If hw_info is invalid, that means create a new host entry.
 *          2. If hw_info not invalid, do updating.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetHostTunnelRoute(SWDRVL3_HostTunnel_T *tunnel_host);

/*******************************************************************************
 * SWDRVL3_DeleteInetHostRoute
 *
 * Purpose: This function will delete a host entry.
 * Inputs:
 *          host  : host entry information
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetHostRoute(SWDRVL3_Host_T *host);


/** \fn BOOL_T SWDRVL3_ClearHostRouteHWInfo(SWDRVL3_Host_T *host)
 *  \brief This function will clear HW information of the host
 *  \param[in] host (IN) Host entry information
 *  \n The input KEY fields are:
 *  \arg \c host->unit
 *  \arg \c host->hw_info
 *  \param[out] host (IN/OUT) The host->hw_info should be SWDRVL3_HW_INFO_INVALID
 *                  if the return value is TRUE
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_ClearHostRouteHWInfo
 *
 * Purpose: This function will clear HW information of the host.
 * Inputs:
 *          host->hw_info  : Host HW information
 * Outputs: 
 *          host->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_ClearHostRouteHWInfo(SWDRVL3_Host_T *host);


/** \fn UI32_T SWDRVL3_AddInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info)
 *  \brief This function will add a route entry
 *  \param[in] route (IN) Route entry information
 *  \n The input KEY fields are:
 *  \arg \c route->flags
 *  \arg \c route->fib_id
 *  \arg \c route->dst_ip
 *  \arg \c route->prefix_length
 *  \arg \c route->next_hop_ip
 *  \arg \c route->nexthop_mac
 *  \arg \c route->dst_vid
 *  \arg \c route->unit
 *  \arg \c route->port OR \c route->trunk_id
 *  \param[in] nh_hw_info (IN) The HW information of the nexthop
 *  \param[out] route (IN/OUT) Output the route->hw_info
 *  \return The error code after execute this subroutine
 *  \retval SWDRVL3_L3_BUCKET_FULL HW entry bucket is full
 *  \retval SWDRVL3_L3_NO_ERROR Set host entry successfully
 *  \note 
 *  <ul>
 *    <ol>
 *      <li>The hw_info indicate the \c Egress-Object \c handler of nexthop in BCM SDK.</li>
 *    </ol>
 *  </ul>
 */
/*******************************************************************************
 * SWDRVL3_AddInetNetRoute
 *
 * Purpose: This function will add a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info);


/*******************************************************************************
 * SWDRVL3_HotInsertAddInetNetRoute
 *
 * Purpose: This function will add a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info);


/** \fn BOOL_T SWDRVL3_DeleteInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info)
 *  \brief This function will delete a route entry
 *  \param[in] route (IN) Route entry information
 *  \n The KEY fields are:
 *  \arg \c route->flags
 *  \arg \c route->fib_id
 *  \arg \c route->dst_ip
 *  \arg \c route->prefix_length
 *  \arg \c route->next_hop_ip
 *  \arg \c route->nexthop_mac
 *  \arg \c route->dst_vid
 *  \arg \c route->unit
 *  \arg \c route->port OR \c route->trunk_id
 *  \param[in] nh_hw_info (IN) The HW information of the nexthop
 *  \param[out] route (IN/OUT) The route->hw_info should be SWDRVL3_HW_INFO_INVALID
 *                  if the return value is TRUE
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_DeleteInetNetRoute
 *
 * Purpose: This function will delete a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : The HW information of the nexthop
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetNetRoute(SWDRVL3_Route_T *route, void *nh_hw_info);


/** \fn BOOL_T SWDRVL3_ClearNetRouteHWInfo(SWDRVL3_Route_T *route)
 *  \brief This function will clear HW information of the route
 *  \param[in] route (IN) Route entry information
 *  \n The input KEY fields are:
 *  \arg \c route->unit
 *  \arg \c route->hw_info
 *  \param[out] route (IN/OUT) The route->hw_info should be SWDRVL3_HW_INFO_INVALID
 *                  if the return value is TRUE
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_ClearNetRouteHWInfo
 *
 * Purpose: This function will clear HW information of the route.
 * Inputs:
 *          route->hw_info  : Host HW information
 * Outputs: 
 *          route->hw_info
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_ClearNetRouteHWInfo(SWDRVL3_Route_T *route);


/** \fn BOOL_T SWDRVL3_SetSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action)
 *  \brief This function will add a default route entry for special purpose
 *  \param[in] default_route (IN) Default Route entry information
 *  \n The KEY fields are:
 *  \arg \c default_route->flags
 *  \arg \c default_route->fib_id
 *  \arg \c default_route->unit
 *  \param[in] action (IN) Indicate the special purpose of this Default Route entry
 *  \n The "action" are:
 *  \arg \c SWDRVL3_ACTION_TRAP2CPU Trap packet to CPU if match
 *  \arg \c SWDRVL3_ACTION_DROP Drop packet if match
 *  \arg \c SWDRVL3_ACTION_ROUTE Do normal forwarding if match
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_SetSpecialDefaultRoute
 *
 * Purpose:  This function will add an default route entry for special purpose.
 * Inputs:
 *          default_route  : Default Route entry information
 *          action : Indicate the special purpose of this Default Route entry
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_SetSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action);


/** \fn BOOL_T SWDRVL3_DeleteSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action)
 *  \brief This function will delete the special default route entry
 *  \param[in] default_route (IN) Default Route entry information
 *  \n The KEY fields are:
 *  \arg \c default_route->flags
 *  \arg \c default_route->fib_id
 *  \arg \c default_route->unit
 *  \param[in] action (IN) Indicate the special purpose of this Default Route entry
 *  \n The "action" are:
 *  \arg \c SWDRVL3_ACTION_TRAP2CPU Trap packet to CPU if match
 *  \arg \c SWDRVL3_ACTION_DROP Drop packet if match
 *  \arg \c SWDRVL3_ACTION_ROUTE Do normal forwarding if match
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_DeleteSpecialDefaultRoute
 *
 * Purpose:  This function will delete the special default route entry.
 * Inputs:
 *          default_route  : Default Route entry information
 *          action : Indicate the special purpose of this Default Route entry
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteSpecialDefaultRoute(SWDRVL3_Route_T *default_route, UI32_T action);


/** \fn BOOL_T SWDRVL3_ReadAndClearHostRouteEntryHitBit(SWDRVL3_Host_T *host, UI32_T *hit)
 *  \brief This function will Read and Clear hit bit of a host entry
 *  \param[in] host (IN) Host entry information
 *  \n The KEY fields are:
 *  \arg \c host->flags
 *  \arg \c host->fib_id
 *  \arg \c host->ip_addr
 *  \arg \c host->unit
 *  \param[out] hit (IN/OUT) The read value of Hit bit
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 *  SWDRVL3_ReadAndClearHostRouteHitBit
 *
 * Purpose: Read and Clear hit bit of a host entry.
 * Inputs:
 *          host  : Host entry information
 * Outputs: 
 *          hit : The read Hit bit
 * Return:  TRUE/FALSE
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_ReadAndClearHostRouteEntryHitBit(SWDRVL3_Host_T *host, UI32_T *hit);


/** \fn UI32_T SWDRVL3_AddInetMyIpHostRoute(SWDRVL3_Host_T *host)
 *  \brief This function will add a "My IP" host entry
 *  \param[in] host (IN) My IP Host entry information
 *  \n The KEY fields are:
 *  \arg \c host->flags
 *  \arg \c host->fib_id
 *  \arg \c host->ip_addr
 *  \arg \c host->unit
 *  \return The error code after execute this subroutine.
 *  \retval SWDRVL3_L3_BUCKET_FULL HW entry bucket is full
 *  \retval SWDRVL3_L3_NO_ERROR Set host entry successfully
 */
/*******************************************************************************
 * SWDRVL3_AddInetMyIpHostRoute
 *
 * Purpose: This function will add a "My IP" host entry.
 * Inputs:
 *          host  : My IP host entry information
 * Outputs: None
 * Return:  
 *          SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetMyIpHostRoute(SWDRVL3_Host_T *host);


/*******************************************************************************
 * SWDRVL3_HotInsertAddInetMyIpHostRoute
 *
 * Purpose: This function will add a "My IP" host entry.
 * Inputs:
 *          host  : My IP host entry information
 * Outputs: None
 * Return:  
 *          SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetMyIpHostRoute(SWDRVL3_Host_T *host);


/** \fn UI32_T SWDRVL3_DeleteInetMyIpHostRoute(SWDRVL3_Host_T *host)
 *  \brief This function will delete a "My IP" host entry
 *  \param[in] host (IN) My IP Host entry information
 *  \n The KEY fields are:
 *  \arg \c host->flags
 *  \arg \c host->fib_id
 *  \arg \c host->ip_addr
 *  \arg \c host->unit
 *  \return The error code after execute this subroutine.
 *  \retval SWDRVL3_L3_BUCKET_FULL HW entry bucket is full
 *  \retval SWDRVL3_L3_NO_ERROR Set host entry successfully
 */
/*******************************************************************************
 * SWDRVL3_DeleteInetMyIpHostRoute
 *
 * Purpose: This function will delete a "My IP" host entry.
 * Inputs:
 *          host  : My IP host entry information
 * Outputs: None
 * Return:  
 *          SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetMyIpHostRoute(SWDRVL3_Host_T *host);


/** \fn UI32_T SWDRVL3_AddInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_first)
 *  \brief This function will add a single nexthop of an ECMP route
 *  \param[in] route (IN) Route entry information
 *  \n The KEY fields are:
 *  \arg \c route->flags
 *  \arg \c route->fib_id
 *  \arg \c route->dst_ip
 *  \arg \c route->prefix_length
 *  \arg \c route->unit
 *  \param[in] nh_hw_info The HW information of the nexthop
 *  \param[in] is_first Indicate the nexthop is the first one or not
 *  \param[out] route (IN/OUT) Ouput the route->hw_info if \c is_first is TRUE
 *  \return The error code after execute this subroutine.
 *  \retval SWDRVL3_L3_BUCKET_FULL HW entry bucket is full
 *  \retval SWDRVL3_L3_ECMP_BUCKET_FULL ecmp entry bucket is full
 *  \retval SWDRVL3_L3_NO_ERROR Set host entry successfully
 *  \note 
 *  <ul>
 *    <ol>
 *      <li>If \c is_first is TRUE, this function will create a multipath nexthop
            HW handler according to the input \c nh_hw_info and set to route->hw_info.</li>
 *    </ol>
 *  </ul>
 */
/*******************************************************************************
 * SWDRVL3_AddInetECMPRouteOnePath
 *
 * Purpose: This function will add a single nexthop of an ECMP route.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 *          is_first
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_ECMP_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    1. If is_first is TRUE, this function will create a multipath nexthop
 *             handler according to the input nh_hw_info and set to route->hw_info.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_first);


/** \fn BOOL_T SWDRVL3_DeleteInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_last)
 *  \brief This function will delete a single nexthop of an ECMP route
 *  \param[in] route (IN) Route entry information
 *  \n The KEY fields are:
 *  \arg \c route->flags
 *  \arg \c route->fib_id
 *  \arg \c route->dst_ip
 *  \arg \c route->prefix_length
 *  \arg \c route->unit
 *  \param[in] nh_hw_info (IN) The HW information of the nexthop
 *  \param[in] is_last (IN) Indicate the nexthop is the last one or not
 *  \param[out] route (IN/OUT) The route->hw_info should be SWDRVL3_HW_INFO_INVALID
 *                  if the return value is TRUE
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_DeleteInetECMPRouteOnePath
 *
 * Purpose: This function will add or update a route entry.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info : nexthop information
 *          is_last
 * Outputs: None
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetECMPRouteOnePath(SWDRVL3_Route_T *route, void *nh_hw_info, BOOL_T is_last);


/** \fn UI32_T SWDRVL3_AddInetECMPRouteMultiPath(SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count)
 *  \brief This function will add an ECMP route with multiple nexthop
 *  \param[in] route (IN) Route entry information
 *  \n The KEY fields are:
 *  \arg \c route->flags
 *  \arg \c route->fib_id
 *  \arg \c route->dst_ip
 *  \arg \c route->prefix_length
 *  \arg \c route->unit
 *  \param[in] nh_hw_info_array (IN) The array pointer of HW information of 
 *              the multiple nexthop
 *  \param[in] count (IN) Indicate number of the multiple nexthop
 *  \param[out] route (IN/OUT) Ouput the route->hw_info
 *  \return The error code after execute this subroutine.
 *  \retval SWDRVL3_L3_BUCKET_FULL HW entry bucket is full
 *  \retval SWDRVL3_L3_ECMP_BUCKET_FULL ecmp entry bucket is full
 *  \retval SWDRVL3_L3_NO_ERROR Set host entry successfully
 */
/*******************************************************************************
 * SWDRVL3_AddInetECMPRouteMultiPath
 *
 * Purpose: This function will add an ECMP route with multiple nexthop.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info_array : The array pointer of HW information of the multiple nexthop
 *          count : Number of nexthops
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_ECMP_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddInetECMPRouteMultiPath(SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count);


/*******************************************************************************
 * SWDRVL3_HotInsertAddInetECMPRouteMultiPath
 *
 * Purpose: This function will add an ECMP route with multiple nexthop to all slaves.
 * Inputs:
 *          route  : Route entry information
 *          nh_hw_info_array : The array pointer of HW information of the multiple nexthop
 *          count : Number of nexthops
 * Outputs: 
 *          route->hw_info
 * Return:  SWDRVL3_L3_BUCKET_FULL
 *          SWDRVL3_L3_ECMP_BUCKET_FULL
 *          SWDRVL3_L3_NO_ERROR
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_HotInsertAddInetECMPRouteMultiPath(SWDRVL3_Route_T *route, void *nh_hw_info_array, UI32_T count);


/** \fn BOOL_T SWDRVL3_DeleteInetECMPRoute(SWDRVL3_Route_T *route)
 *  \brief This function will delete an ECMP route
 *  \param[in] route (IN) Route entry information
 *  \n The KEY fields are:
 *  \arg \c route->flags
 *  \arg \c route->fib_id
 *  \arg \c route->dst_ip
 *  \arg \c route->prefix_length
 *  \arg \c route->unit
 *  \return Boolean
 *  \retval TRUE Success
 *  \retval FALSE Failed
 */
/*******************************************************************************
 * SWDRVL3_DeleteInetECMPRoute
 *
 * Purpose: This function will delete an ECMP route.
 * Inputs:
 *          route  : Route entry information
 * Outputs: None
 * Return:  TRUE/FALSE
 * Note:    None.
 *******************************************************************************
 */
BOOL_T SWDRVL3_DeleteInetECMPRoute(SWDRVL3_Route_T *route);

/*******************************************************************************
 * SWDRVL3_AddTunnelIntfL3
 *
 * Purpose: This function will add/del a tunnel l3 interface.
 * Inputs:
 *          tl3_p  : Tunnel Initiator information
 *          tl3_p->l3_intf_id for del
 *          tl3_p->vid        for add
 *          tl3_p->src_mac    for add
 *          is_add : TRUE if it's to add
 * Outputs: tl3_p->l3_intf_id : L3 Interface Id for add
 * Return:  SWDRVL3_L3_NO_ERROR/SWDRVL3_L3_OTHERS
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddTunnelIntfL3(
    SWDRVL3_TunnelIntfL3_T *tl3_p);

/** \fn UI32_T SWDRVL3_AddTunnelInitiator(SWDRVL3_TunnelInitiator_T *tunnel)
 *  \brief This function will add a tunnel initiator
 *  \param[in] tunnel (IN) Tunnel Initiator information
 *  \n The KEY fields are:
 *  \arg \c tunnel->tunnel_type
 *  \arg \c tunnel->sip
 *  \arg \c tunnel->dip
 *  \arg \c tunnel->ttl
 *  \arg \c tunnel->nexthop_mac
 *  \param[out] tunnel->l3_intf_id (OUT) L3 Interface ID associated with this tunnel
 *  \return The error code after execute this subroutine.
 *  \retval 
 */
/*******************************************************************************
 * SWDRVL3_AddTunnelInitiator
 *
 * Purpose: This function will add a tunnel initiator.
 * Inputs:
 *          tunnel : Tunnel Initiator information
 * Outputs: tunnel->l3_intf_id : L3 Interface Id associated with this tunnel 
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddTunnelInitiator(SWDRVL3_TunnelInitiator_T *tunnel);

/** \fn UI32_T SWDRVL3_DeleteTunnelInitiator(UI32_T l3_intf_id)
 *  \brief This function will delete a tunnel initiator attached on an L3 Interface ID
 *  \param[in] l3_intf_id (IN) L3 Interface ID
 *  \return The error code after execute this subroutine.
 *  \retval 
 */
/*******************************************************************************
 * SWDRVL3_DeleteTunnelInitiator
 *
 * Purpose: This function will delete a tunnel initiator.
 * Inputs:
 *          l3_intf_id: The L3 Interface ID associated with the tunnel initiator 
 * Outputs: None 
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_DeleteTunnelInitiator(UI32_T l3_intf_id);

/** \fn UI32_T SWDRVL3_AddTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel)
 *  \brief This function will add a tunnel terminator
 *  \param[in] tunnel (IN) Tunnel Terminator information
 *  \n The KEY fields are:
 *  \arg \c tunnel->fib_id
 *  \arg \c tunnel->tunnel_type
 *  \arg \c tunnel->sip
 *  \arg \c tunnel->dip
 *  \arg \c tunnel->port_bitmap
 *  \return The error code after execute this subroutine.
 *  \retval 
 */
/*******************************************************************************
 * SWDRVL3_AddTunnelTerminator
 *
 * Purpose: This function will add a tunnel terminator.
 * Inputs:
 *          tunnel : Tunnel Terminator information
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_AddTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel);

/** \fn UI32_T SWDRVL3_DeleteTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel)
 *  \brief This function will delete a tunnel terminator
 *  \param[in] tunnel (IN) Tunnel Terminator information
 *  \n The KEY fields are:
 *  \arg \c tunnel->fib_id
 *  \arg \c tunnel->tunnel_type
 *  \arg \c tunnel->sip
 *  \arg \c tunnel->dip
 *  \return The error code after execute this subroutine.
 *  \retval 
 */
/*******************************************************************************
 * SWDRVL3_DeleteTunnelTerminator
 *
 * Purpose: This function will delete a tunnel terminator.
 * Inputs:
 *          tunnel : Tunnel Terminator information
 * Outputs: None 
 * Return:  
 * Note:    None.
 *******************************************************************************
 */
UI32_T SWDRVL3_DeleteTunnelTerminator(SWDRVL3_TunnelTerminator_T *tunnel);



/* FUNCTION NAME : SWDRVL3_CompareTunnelNetRouteByIfindexAddr
 * PURPOSE:
 *      Provide function for L_SORT module to compare NetRoute_T elements
 *
 * INPUT:
 *      elm1_p        element1 for comparing
 *      elm2_p        element2 for comparing
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      if elm1_p is better than elm2_p, reture value is greater than zero.
 *      if elm1_p is same as elm2_p, return value is zero.
 *      if elm1_p is worse than elm2_p, return value is less than zero
 *
 * NOTES:
 *      The first key is dst_vid, the second key is dst_ip, the third key is prefix length
 */
int SWDRVL3_CompareTunnelNetRouteByIfindexAddr(void *elm1_p,void *elm2_p);

#if (SYS_CPNT_STACKING==TRUE)
/* -------------------------------------------------------------------------
 * Function : SWDRVL3_ISC_Handler
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulte all of SWDRVL3 via ISC
 * INPUT    : *key            -- key of ISC
 *            *mref_handle_p  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by ISC_Agent
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRVL3_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
#endif /* SYS_CPNT_STACKING==TRUE */

#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRVL3_SetEcmpBalanceMode
 * -------------------------------------------------------------------------
 * FUNCTION: To set ECMP balance mode
 * INPUT   : balance_mode bitmap - SWDRVL3_ECMP_DST_IP
 *                                 SWDRVL3_ECMP_L4_PORT
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T SWDRVL3_SetEcmpBalanceMode(UI32_T mode);
#endif

#endif /* End of SWDRVL3.H */

