/* Module Name: ip_mgr.c
 * Purpose:
 *      IP provides Layer-3 interface management function, including create routing interface,
 *      destroy routing interface, change ip address of the interface.
 *      IP_MGR is the porting interface which porting all access points of ZebOS, a Routing
 *      protocol package.
 *      The interface management will be done via NSM(the core of ZebOS) and it will then
 *      pass the configuration to Linux Kernel (TCP/IP stack).
 *      In whole system, the management hierarch is NETCFG --> IPCFG --> IP_MGR --> ZebOS -> Linux Kernel.
 *
 * Notes:
 *      1. Recordset definition :
 *         Circuit : { rif_no, phyAddress, if_MTU, if_speed }
 *                   Key : rif_no.
 *         Subnet  : { router_ip, netmask, rif_no }
 *                   Key : rif_no.
 *         Routing : { dst_ip, dst_mask, next_hop, rif_no, metric }
 *                   Key : (dst_ip, dst_mask)
 *         ARP     : { ip_addr, rif_no, phyAddress }
 *                   Key : (ip_addr, rif_no)
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2001.08.17  --  William,    Created
 *      2001.09.12  --  William,    replace all circuit_blk_id to rif_num
 *                                  in API parameter, the interface
 *                                  btw NetCfg and IP_MGR is rif_num, not circuit_blk.
 *      2001.10.06  --  William,    Clear the code.
 *  0.2 2002.02.28  --  William,    Re-organize for Layer-3 developing.
 *  1.0 2002.11.02  --  William,    add IP_MGR_ShowSocketUsage().
 *  1.1 2007.07.16  --  Max Chen,   Porting to Linux Platform
 *
 * Copyright(C)      Accton Corporation, 2007.
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "netcfg_type.h"
#include "sysfun.h"
#include "sys_module.h"
#include "l_mm.h"
#include "ip_mgr.h"
#include "ip_lib.h"
#include "string.h"
#include "stdlib.h"
#include "l_stdlib.h"
#include "l_threadgrp.h"
#include "l_prefix.h"
#include "ipal_types.h"
#include "ipal_neigh.h"

#include "ipal_if.h" /* for IPAL_IF_Enable/DisableIpv6Autoconfig */
#include "ipal_route.h"
#include "backdoor_mgr.h"
#include "vlan_lib.h" /* for VLAN_OM_ConvertFromIfindex */
#include "swdrvl3.h"
#include "netcfg_om_ip.h" /* for NETCFG_OM_L3_Interface_T */
#include "amtr_pmgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
/***************************************************************/
static unsigned long DEBUG_FLAG = 0;//DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/
#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?:printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)

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
/*END Simon's debug function*/
/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : IP_MGR_InitiateProcessResources
 * PURPOSE:
 *      Initialize IP_MGR used system resource.
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
void IP_MGR_InitiateProcessResources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    return;
}   /*  End of IP_MGR_InitiateProcessResources */



/* FUNCTION NAME : IP_MGR_Create_InterCSC_Relation
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
 *
 * NOTES:
 *
 */
void IP_MGR_Create_InterCSC_Relation(void)
{
    return;

}
/* FUNCTION NAME : IP_MGR_Enter_Master_Mode
 * PURPOSE:
 *      Enter Master Mode; could handle TCP/IP protocol stack management operation.
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
void IP_MGR_Enter_Master_Mode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    return;
}   /*  end of IP_MGR_Enter_Master_Mode */

/* FUNCTION NAME : IP_MGR_Enter_Slave_Mode
 * PURPOSE:
 *      Enter Slave Mode; ignore any request.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *
 * NOTES:
 *
 */
void IP_MGR_Enter_Slave_Mode (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    return;
}   /*  end of IP_MGR_Enter_Slave_Mode  */


/* FUNCTION NAME : IP_MGR_Enter_Transition_Mode
 * PURPOSE:
 *      Enter Transition Mode; release all resource of IP.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *
 * NOTES:
 *      1. Reset all modules, release all resources.
 */
void IP_MGR_Enter_Transition_Mode (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */


    /*2005.8.23 Willy Add the IP_MGR Entertransition mode
     *for EPR:ES4649-32-01451, we do not clear IP in Phase 2
     *originally.
     */
    /*IP_MGR_DestroyAlIPv4lRif();*/
    /*end willy add*/
    return;
}   /*  end of IP_MGR_Enter_Transition_Mode */

/*------------------------------------------------
 *  IP_MGR Configure API.
 *------------------------------------------------
 */

/* FUNCTION NAME : IP_MGR_CreateInterface
 * PURPOSE:
 *      Create an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      intf -- the interface entry.
 *      drv_l3_intf_index_p -- pointer to the L3If index (hwinfo) in chip
 *                             if L3If index != SWDRVL3_HW_INFO_INVALID -> use it to create L3If
 *                             otherwise -> create a new one and return it's L3If index
 *
 * OUTPUT:
 *      drv_l3_intf_index_p -- pointer to the drv_l3_intf_index
 *
 * RETURN:
 *      NETCFG_TYPE_OK
 *      NETCFG_TYPE_FAIL
 *      NETCFG_TYPE_MAC_COLLISION
 *
 * NOTES:
 *      None.
 *
 */
UI32_T IP_MGR_CreateInterface(IP_MGR_Interface_T *intf, UI32_T *drv_l3_intf_index_p)
{
    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T vid;
#if (SYS_CPNT_ROUTING == TRUE) && (SYS_CPNT_SWDRVL3 == TRUE)
    UI32_T fib_id;
    I32_T is_routing_v4 = 0, is_routing_v6 = 0;
#endif
    BOOL_T is_routing;
    UI32_T rc;
    UI32_T ret_ui32;
    AMTR_TYPE_Ret_T ret;

    UI16_T flags;

    /* BODY
     */
    if (NULL == intf)
        return NETCFG_TYPE_FAIL;

    if (IPAL_RESULT_OK != IPAL_IF_CreateInterface(intf->ifindex, intf->u.physical_intf.logical_mac))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_CreateInterface failed\n");
        return NETCFG_TYPE_FAIL;
    }

    /* According to the origninal design, we treated IFF_UP for admin status
     * and IFF_RUNNING for oper status, but this policy is not compatible
     * with Linux TCP/IP stack which only care IFF_UP (refer to dev_change_flags()
     * in dev.c). Therefore, a workaround is added here to handle IFF_RUNNING flags.
     */
    flags = intf->u.physical_intf.if_flags;
    if(FLAG_ISSET(flags, IFF_RUNNING))
        SET_FLAG(flags, IFF_UP);
    else
        UNSET_FLAG(flags, IFF_UP);

    if (IPAL_RESULT_OK != IPAL_IF_SetIfFlags(intf->ifindex, flags))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_SetIfFlags failed\n");
        return NETCFG_TYPE_FAIL;
    }

    VLAN_OM_ConvertFromIfindex(intf->ifindex, &vid);
#if (SYS_CPNT_ROUTING == TRUE) && (SYS_CPNT_SWDRVL3 == TRUE)
    /* TODO:
     * Should assign a proper FIB ID to create l3 interface.
     */
    fib_id = 0;
#if (SYS_CPNT_IPV6 == TRUE)
    IPAL_ROUTE_GetIpv6Forwarding(&is_routing_v6);
#endif
    IPAL_ROUTE_GetIpv4Forwarding(&is_routing_v4);

    is_routing = ((is_routing_v4 | is_routing_v6)!=0);

    ret_ui32 = SWDRVL3_CreateL3Interface(fib_id ,vid, intf->u.physical_intf.logical_mac, drv_l3_intf_index_p);
    switch(ret_ui32)
    {
        case SWDRVL3_L3_NO_ERROR:
            rc = NETCFG_TYPE_OK;
            break;
        case SWDRVL3_L3_MAC_COLLISION:
            rc = NETCFG_TYPE_MAC_COLLISION;
            break;
        default:
            rc = NETCFG_TYPE_FAIL;
            break;
    }
    if(rc != NETCFG_TYPE_OK)
        return rc;
#else
    *drv_l3_intf_index_p = 0;
    is_routing = FALSE;
#endif

    ret = AMTR_PMGR_SetCpuMac(vid, intf->u.physical_intf.logical_mac, is_routing);

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

/* FUNCTION NAME : IP_MGR_CreateLoopbackInterface
 * PURPOSE:
 *      Create a loopback interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      intf -- the interface entry.
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
 *
 */
UI32_T IP_MGR_CreateLoopbackInterface(IP_MGR_Interface_T *intf)
{
    if (NULL == intf)
        return NETCFG_TYPE_FAIL;

    if (IPAL_RESULT_OK != IPAL_IF_CreateLoopbackInterface(intf->ifindex))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_CreateLoopbackInterface failed\n");
        return NETCFG_TYPE_FAIL;
    }

    IPAL_IF_SetIfFlags(intf->ifindex, IFF_UP | IFF_RUNNING);

    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : IP_MGR_DeleteInterface
 * PURPOSE:
 *      Delete an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_DeleteInterface(UI32_T ifindex)
{
    UI32_T fib_id = 0;
    NETCFG_TYPE_L3_Interface_T l3_intf;
    UI32_T vid;

    if (ifindex == 0)
        return FALSE;

    /* TODO:
     * Should assign a proper FIB ID to create l3 interface.
     */
    fib_id = 0;

    VLAN_OM_ConvertFromIfindex(ifindex, &vid);

    memset(&l3_intf, 0, sizeof(l3_intf));
    l3_intf.ifindex = ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&l3_intf) != NETCFG_TYPE_OK)
        return FALSE;

#if (SYS_CPNT_SWDRVL3 == TRUE)
    if(FALSE == SWDRVL3_DeleteL3Interface(fib_id ,vid, l3_intf.u.physical_intf.logical_mac, l3_intf.drv_l3_intf_index))
        return FALSE;
#endif

    if (IPAL_RESULT_OK != IPAL_IF_UnsetIfFlags(ifindex, IFF_UP|IFF_RUNNING))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_UnsetIfFlags failed\n");
        return FALSE;
    }

    if (IPAL_RESULT_OK != IPAL_IF_DestroyInterface(ifindex))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_DestroyInterface failed\n");
        return FALSE;
    }

    if (!AMTR_PMGR_DeleteCpuMac(vid, l3_intf.u.physical_intf.logical_mac))
        return FALSE;

    return TRUE;
}

/* FUNCTION NAME : IP_MGR_DeleteLoopbackInterface
 * PURPOSE:
 *      Delete a loopback interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_DeleteLoopbackInterface(UI32_T ifindex)
{
    NETCFG_TYPE_L3_Interface_T l3_intf;
    UI32_T vid;

    if (ifindex == 0)
        return FALSE;

    memset(&l3_intf, 0, sizeof(l3_intf));
    l3_intf.ifindex = ifindex;
    if (NETCFG_OM_IP_GetL3Interface(&l3_intf) != NETCFG_TYPE_OK)
    {
        return FALSE;
    }

    if (IPAL_RESULT_OK != IPAL_IF_UnsetIfFlags(ifindex, IFF_UP|IFF_RUNNING))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_UnsetIfFlags failed\n");
        return FALSE;
    }

    if (IPAL_RESULT_OK != IPAL_IF_DestroyLoopbackInterface(ifindex))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_DestroyLoopbackInterface failed\n");
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME : IP_MGR_SetIfFlags
 * PURPOSE:
 *      Set flags of an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_SetIfFlags(UI32_T ifindex, UI16_T flags)
{
    if (ifindex == 0)
        return FALSE;

    /* According to the origninal design, we treated IFF_UP for admin status
     * and IFF_RUNNING for oper status, but this policy is not compatible
     * with Linux TCP/IP stack which only care IFF_UP (refer to dev_change_flags()
     * in dev.c). Therefore, a workaround is added here to handle IFF_RUNNING flags.
     */
    if(FLAG_ISSET(flags, IFF_RUNNING))
        SET_FLAG(flags, IFF_UP);
    else
        UNSET_FLAG(flags, IFF_UP);

    if (IPAL_RESULT_OK != IPAL_IF_SetIfFlags(ifindex, flags))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_SetIfFlags failed\n");
        return FALSE;
    }

    /* PMs will support later */
#if 0 /* VaiWang, Thursday, March 27, 2008 3:10:23 */
    ret = RIP_PMGR_SetIfFlags(ifindex, flags);
    if (ret != RIP_TYPE_RESULT_OK)
    {
        DBG_PrintText (" IP_MGR : IP_MGR_SetIfFlags failed\n");
        return FALSE;
    }
#endif /*  ACCTON_METRO */

    return TRUE;
}


/* FUNCTION NAME : IP_MGR_UnsetIfFlags
 * PURPOSE:
 *      Unset flags of an interface in PMs and NSM->TCP/IP stack.
 *
 * INPUT:
 *      ifindex -- the index of interface.
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_MGR_UnsetIfFlags(UI32_T ifindex, UI16_T flags)
{
    if (ifindex == 0)
        return FALSE;

    /* PMs will support later */
#if 0 /* VaiWang, Thursday, March 27, 2008 3:12:32 */
    ret = RIP_PMGR_UnsetIfFlags(ifindex, flags);
    if (ret != RIP_TYPE_RESULT_OK)
    {
        DBG_PrintText (" IP_MGR : IP_MGR_UnsetIfFlags failed\n");
        return FALSE;
    }
#endif /*  ACCTON_METRO */


    /* According to the origninal design, we treated IFF_UP for admin status
     * and IFF_RUNNING for oper status, but this policy is not compatible
     * with Linux TCP/IP stack which only care IFF_UP (refer to dev_change_flags()
     * in dev.c). Therefore, a workaround is added here to handle IFF_RUNNING flags.
     */
    if(FLAG_ISSET(flags, IFF_RUNNING))
        SET_FLAG(flags, IFF_UP);
    else
        UNSET_FLAG(flags, IFF_UP);

    if (IPAL_RESULT_OK != IPAL_IF_UnsetIfFlags(ifindex, flags))
    {
        DBG_PrintText (" IP_MGR : IPAL_IF_UnsetIfFlags failed\n");
        return FALSE;
    }

    return TRUE;
}


/* FUNCTION NAME : IP_MGR_AddInetRif
 * PURPOSE:
 *      Add Inet Rif.
 *
 * INPUT:
 *      rif->ifindex            --  the ifindex of interface (currently, equal to vid_ifindex)
 *      rif->addr.addr          --  subnet ip address, the ip associated with Router.
 *      rif->addr.mask          --  subnet mask.
 *      rif->ipv4_role          --  set as the primary or secondary interface
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_INVALID_ARG -- input argument is invalid
 *      NETCFG_TYPE_CAN_NOT_ADD -- Fail to Add the specified IPV4 Rif
 *      NETCFG_TYPE_OK          -- The Rif is added successfully
 *
 * NOTES:
 *      1. In Linux and ZebOS, each interface can bound with multiple IP address
 *         The interface is identified via name(retrieved from ifindex)
 *      2. For both IPv4 or IPv6.
 *
 */
UI32_T IP_MGR_AddInetRif(NETCFG_TYPE_InetRifConfig_T *rif)
{
    UI32_T result;

    if (rif == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    {
        UI16_T  l3_if_flags = 0;

        /* add address into kernel if IFF_UP to avoid IPv6 DAD failure */
        if(!IPAL_IF_GetIfFlags(rif->ifindex, &l3_if_flags))
        {
            if (l3_if_flags & IFF_UP)
            {
                result = IPAL_IF_AddIpAddress(rif->ifindex, &rif->addr);
                if (result != IPAL_RESULT_OK && result != IPAL_RESULT_ENTRY_EXIST)
                    return NETCFG_TYPE_CAN_NOT_ADD;
            }
        }
    }

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : IP_MGR_DeleteIPv4Rif
 * PURPOSE:
 *      Delete Inet Rif.
 *
 * INPUT:
 *      rif->ifindex            --  the ifindex of interface (currently, equal to vid_ifindex)
 *      rif->addr.addr          --  subnet ip address, the ip associated with Router.
 *      rif->addr.mask          --  subnet mask.
 *      rif->ipv4_role          --  figure out if the interface is primary or secondary
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      NETCFG_TYPE_OK           -- Successfully delete the ip address .
 *      NETCFG_TYPE_ENTRY_NOT_EXIST
 *      NETCFG_TYPE_INVALID_ARG
 *      NETCFG_TYPE_CAN_NOT_DELETE -- Can not delete
 *
 * NOTES:
 *      1. In Linux and ZebOS, each interface can bound with multiple IP address
 *         The interface is identified via name(retrieved from ifindex)
 *      2. For both IPv4 or IPv6.
 */
UI32_T IP_MGR_DeleteInetRif(NETCFG_TYPE_InetRifConfig_T *rif)
{
    if (rif == NULL)
    {
        return NETCFG_TYPE_INVALID_ARG;
    }

    if (IPAL_RESULT_OK != IPAL_IF_DeleteIpAddress(rif->ifindex, &rif->addr))
        return NETCFG_TYPE_CAN_NOT_DELETE;

    return NETCFG_TYPE_OK;
}


/* FUNCTION NAME : IP_MGR_IsEmbeddedUdpPort
 * PURPOSE:
 *      Check the udp-port is used in protocol engine or not.
 *
 * INPUT:
 *      udp_port -- the udp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      1. This function is used for udp-helper; a passive function.
 *      2. Because get only, do not enter critical section and
 *         do not check master mode.
 *      3. check for ipv4/ipv6 port
 */
BOOL_T IP_MGR_IsEmbeddedUdpPort (UI32_T udp_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
#define _PATH_PROCNET_UDP   "/proc/net/udp"
#if (SYS_CPNT_IPV6 == TRUE)
#define _PATH_PROCNET_UDP6   "/proc/net/udp6"
#endif

    /* LOCAL VARIABLES DEFINITION
     */
    FILE *procinfo;
    char buffer[512];

    /* BODY
     */
    procinfo = fopen(_PATH_PROCNET_UDP, "r");
    if (procinfo == NULL)
    {
        return FALSE;
    }

    do
    {
      if (fgets(buffer, sizeof(buffer), procinfo))
      {
        int num, d, local_port;
        char local_addr[128];


        num = sscanf(buffer,
        "%d: %64[0-9A-Fa-f]:%X %*s\n",
             &d, local_addr, &local_port);

        /* compare with local_port */
        if (udp_port == local_port)
        {
            /* this port has been used */
            fclose(procinfo);
            return TRUE;
        }
      }
    } while (!feof(procinfo));

    fclose(procinfo);

#if (SYS_CPNT_IPV6 == TRUE)

    procinfo = fopen(_PATH_PROCNET_UDP6, "r");
    if (procinfo == NULL)
    {
        return FALSE;
    }

    do
    {
      if (fgets(buffer, sizeof(buffer), procinfo))
      {
        int num, d, local_port;
        char local_addr[128];


        num = sscanf(buffer,
        "%d: %64[0-9A-Fa-f]:%X %*s\n",
             &d, local_addr, &local_port);

        /* compare with local_port */
        if (udp_port == local_port)
        {
            /* this port has been used */
            fclose(procinfo);
            return TRUE;
        }
      }
    } while (!feof(procinfo));

    fclose(procinfo);
#endif

    return FALSE;
}   /*  End of IP_MGR_IsEmbeddedUdpPort */

/* FUNCTION NAME : IP_MGR_IsEmbeddedTcpPort
 * PURPOSE:
 *      Check the tcp-port is used in protocol engine or not.
 *
 * INPUT:
 *      tcp_port -- the tcp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      1. This function is used for UI; a passive function.
 *      2. Ref. linux net-tools-1.60 netstat.c
 *      3. check for ipv4/ipv6 port
 */
BOOL_T IP_MGR_IsEmbeddedTcpPort(UI32_T tcp_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
#define _PATH_PROCNET_TCP   "/proc/net/tcp"
#if (SYS_CPNT_IPV6 == TRUE)
#define _PATH_PROCNET_TCP6   "/proc/net/tcp6"
#endif

    /* LOCAL VARIABLES DEFINITION
     */
    FILE *procinfo;
    char buffer[512];
    //int rc = 0;
    //int lnr = 0;

    /* BODY
     */
    procinfo = fopen(_PATH_PROCNET_TCP, "r");
    if (procinfo == NULL)
    {
        return FALSE;
    }

    do
    {
      if (fgets(buffer, sizeof(buffer), procinfo))
      {
        int num, d, local_port;
        char local_addr[128], state_str[16];


        num = sscanf(buffer,
        "%d: %64[0-9A-Fa-f]:%X %*s %s\n",
             &d, local_addr, &local_port, state_str);

        /* compare with local_port */
        /*jingyan zheng defect ES3628BT-FLF-ZZ-00478*/
        if (tcp_port == local_port && !strncmp(state_str, "0A", 2))
        {
            /* this port has been used */
            fclose(procinfo);
            return TRUE;
        }
      }
    } while (!feof(procinfo));

    fclose(procinfo);

#if (SYS_CPNT_IPV6 == TRUE)
    procinfo = fopen(_PATH_PROCNET_TCP6, "r");
    if (procinfo == NULL)
    {
        return FALSE;
    }

    do
    {
      if (fgets(buffer, sizeof(buffer), procinfo))
      {
        int num, d, local_port;
        char local_addr[128], state_str[16];


        num = sscanf(buffer,
        "%d: %64[0-9A-Fa-f]:%X %*s %s\n",
             &d, local_addr, &local_port, state_str);

        /* compare with local_port */
        /*jingyan zheng defect ES3628BT-FLF-ZZ-00478*/
        if (tcp_port == local_port && !strncmp(state_str, "0A", 2))
        {
            /* this port has been used */
            fclose(procinfo);
            return TRUE;
        }
      }
    } while (!feof(procinfo));

    fclose(procinfo);

#endif

    return (FALSE);
}   /* end of IP_MGR_IsEmbeddedTcpPort */

#if (SYS_CPNT_PROXY_ARP == TRUE)
/* FUNCTION NAME : IP_MGR_SetIpNetToMediaProxyStatus
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
UI32_T  IP_MGR_SetIpNetToMediaProxyStatus(UI32_T ifindex, BOOL_T status)
{
	if(ifindex == 0)
		return NETCFG_TYPE_INVALID_ARG;

    if (status == TRUE)
        IPAL_NEIGH_EnableIpv4ProxyArp(ifindex);
    else
        IPAL_NEIGH_DisableIpv4ProxyArp(ifindex);

    return NETCFG_TYPE_OK;
}
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

#if (SYS_CPNT_IPV6 == TRUE)
/* FUNCTION NAME : IP_MGR_IPv6AddrAutoconfigEnable
 * PURPOSE:
 *      To enable automatic configuration of IPv6 addresses using stateless
 *      autoconfiguration on an interface.
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
 *      1. The autoconfiguration process specified here applies only to hosts, not routers.
 *      2. The configuration is passed into TCP/IP protocol stack, and after the process,
 *         the stack will call NETCFG_MGR_IP_SetInetRif to add into OM.
 */
UI32_T IP_MGR_IPv6AddrAutoconfigEnable(UI32_T ifindex)
{

    IPAL_IF_EnableIpv6Autoconfig(ifindex);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : IP_MGR_IPv6AddrAutoconfigDisable
 * PURPOSE:
 *      To disable automatic configuration of IPv6 addresses using stateless
 *      autoconfiguration on an interface.
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
 *      1. The autoconfiguration process specified here applies only to hosts, not routers.
 *      2. The configuration is passed into TCP/IP protocol stack, and after the process,
 *         the stack will call NETCFG_MGR_IP_SetInetRif to add into OM.
 */
UI32_T IP_MGR_IPv6AddrAutoconfigDisable(UI32_T ifindex)
{
    IPAL_IF_DisableIpv6Autoconfig(ifindex);
    return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : IP_MGR_GetNextIfJoinIpv6McastAddr
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
UI32_T IP_MGR_GetNextIfJoinIpv6McastAddr(UI32_T ifindex, L_INET_AddrIp_T *mcaddr_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */

    if(IPAL_RESULT_OK != IPAL_IF_GetNextIfJoinIpv6McastAddr(ifindex, mcaddr_p))
        return NETCFG_TYPE_FAIL;

   return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : IP_MGR_GetNextPMTUEntry
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
UI32_T IP_MGR_GetNextPMTUEntry(NETCFG_TYPE_PMTU_Entry_T *entry_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    IPAL_PmtuEntry_T pmtu_entry;
    UI32_T entry_num;

    /* BODY */
    memcpy(&(pmtu_entry.destip), &(entry_p->destip), sizeof(L_INET_AddrIp_T));

    entry_num = 1;

    if(IPAL_RESULT_OK != IPAL_ROUTE_GetNextNPmtuEntry(&pmtu_entry, &entry_num))
        return NETCFG_TYPE_FAIL;

    entry_p->pmtu = pmtu_entry.pmtu;
    memcpy(&(entry_p->destip), &(pmtu_entry.destip), sizeof(L_INET_AddrIp_T));
    entry_p->since_time = pmtu_entry.since_time;

   return NETCFG_TYPE_OK;
}

/* FUNCTION NAME : IP_MGR_GetIfIpv6AddrInfo
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
UI32_T IP_MGR_GetIfIpv6AddrInfo(NETCFG_TYPE_IpAddressInfoEntry_T *addr_info_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    IPAL_IpAddressInfoEntry_T   ipal_addr_info;

    /* BODY */

    memset(&ipal_addr_info, 0, sizeof(ipal_addr_info));
    memcpy(&(ipal_addr_info.ipaddr), &(addr_info_p->ipaddr), sizeof(ipal_addr_info.ipaddr));

    if(IPAL_RESULT_OK != IPAL_IF_GetIfIpv6AddrInfo(addr_info_p->ifindex, &ipal_addr_info))
        return NETCFG_TYPE_FAIL;

    addr_info_p->ifindex = ipal_addr_info.ifindex;
    addr_info_p->valid_lft = ipal_addr_info.valid_lft;
    addr_info_p->preferred_lft = ipal_addr_info.preferred_lft;
    addr_info_p->scope = ipal_addr_info.scope;
    addr_info_p->tentative = ipal_addr_info.tentative;
    addr_info_p->deprecated = ipal_addr_info.deprecated;
    addr_info_p->permanent = ipal_addr_info.permanent;
    memcpy(&(addr_info_p->ipaddr), &(ipal_addr_info.ipaddr), sizeof(addr_info_p->ipaddr));

    return NETCFG_TYPE_OK;
}

#endif /* SYS_CPNT_IPV6 */
